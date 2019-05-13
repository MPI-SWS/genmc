/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#include "config.h"

#include "VSet.hpp"
#include "LoopUnrollPass.hpp"
#include "DeclareAssumePass.hpp"
#include "DeclareEndLoopPass.hpp"
#include "SpinAssumePass.hpp"
#include "Error.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <sstream>

#ifdef LLVM_HAS_TERMINATORINST
 typedef llvm::TerminatorInst TerminatorInst;
#else
 typedef llvm::Instruction TerminatorInst;
#endif

#ifdef LLVM_HAVE_LOOP_GET_NAME
# define LOOP_NAME(l) (l->getName())
#else
# define LOOP_NAME(l) ((*l->block_begin())->getName())
#endif

void LoopUnrollPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	llvm::LoopPass::getAnalysisUsage(au);
	au.addRequired<DeclareEndLoopPass>();
	au.addPreserved<DeclareEndLoopPass>();
}

/*
   This function creates a block that only calls the special __end_loop() function.
   The Interpreter should drop the execution when such a call is encountered.
*/
llvm::BasicBlock *LoopUnrollPass::createLoopDivergeBlock(llvm::Loop *l)
{
	llvm::Function *parentFun = (*l->block_begin())->getParent();
	llvm::Function *endLoopFun = parentFun->getParent()->getFunction("__end_loop");

	BUG_ON(!endLoopFun);

	llvm::BasicBlock *divergeBlock = llvm::BasicBlock::Create(parentFun->getContext(), "diverge", parentFun);
	llvm::CallInst::Create(endLoopFun, {}, "", divergeBlock);
	llvm::BranchInst::Create(divergeBlock, divergeBlock);

	return divergeBlock;
}

/*
 * Creates an allocation for a variable that will be used for bounding this loop.
 * The location return by the alloca() is initialized with the loop's bound.
 */
llvm::Value *LoopUnrollPass::createBoundAlloca(llvm::Loop *l)
{
	llvm::Function *parentFun = (*l->block_begin())->getParent();
	llvm::BasicBlock::iterator entryInst;

	/*
	 * LLVM expects all allocas() to be in the beginning of each function.
	 * We first locate the position in which this alloca() will be inserted.
	 */
	for (entryInst = parentFun->begin()->begin();
	     llvm::dyn_cast<llvm::AllocaInst>(&*entryInst); ++entryInst)
		;

	/* Create the alloca()), initialize the memory location, and return it */
	llvm::Value *loopBound = llvm::ConstantInt::get(llvm::Type::getInt32Ty(parentFun->getContext()),
							unrollDepth);
	llvm::Value *alloca = new llvm::AllocaInst(llvm::Type::getInt32Ty(parentFun->getContext()),
						   0, NULL, LOOP_NAME(l) + ".max", &*entryInst);
	llvm::StoreInst *ptr = new llvm::StoreInst(loopBound, alloca, &*entryInst);
	return alloca;
}

/*
 * Returns a basic block the only purpose of which is to initialize the bound variable
 * for this loop to the maximum bound. This block needs to be executed before entering a
 * loop for the first time.
 */
llvm::BasicBlock *LoopUnrollPass::createBoundInitBlock(llvm::Loop *l, llvm::Value *boundAlloca)
{
	llvm::Function *parentFun = (*l->block_begin())->getParent();
	llvm::BasicBlock *initBlock = llvm::BasicBlock::Create(parentFun->getContext(),
							       "init." + LOOP_NAME(l) + ".bound",
							       parentFun);

	llvm::Value *initBound = llvm::ConstantInt::get(llvm::Type::getInt32Ty(parentFun->getContext()),
							unrollDepth);
	llvm::StoreInst *ptr = new llvm::StoreInst(initBound, boundAlloca, initBlock);
	llvm::BranchInst::Create(l->getHeader(), initBlock); /* Branch to loop header */

	return initBlock;
}

/*
 * Returns a block which decrements the bound variable for this loop (boundAlloca).
 * If the new value of the bound variable is 0, the returned block redirects to a
 * divergence block. The returned block needs to be executed after each iteration of the loop.
 */
llvm::BasicBlock *LoopUnrollPass::createBoundDecrBlock(llvm::Loop *l, llvm::Value *boundAlloca)
{
	llvm::Function *parentFun = (*l->block_begin())->getParent();

	/*
	 * Create a divergence block for when the loop bound has been reached, and a
	 * block that decreases the bound counter.
	 */
	llvm::BasicBlock *divergeBlock = createLoopDivergeBlock(l);
	llvm::BasicBlock *boundBlock = llvm::BasicBlock::Create(parentFun->getContext(),
								"dec." + LOOP_NAME(l) + ".bound",
								parentFun);

	/*
	 * The form of boundBlock is the following:
	 *
	 * %bound.val = load i32, i32* LOOP_BOUND_VAR
	 * %dec = add nsw i32 %bound.val, -1
	 * store i32 %dec, i32* LOOP_BOUND_VAR
	 * %cmp = icmp eq i32 %dec, 0
	 * br i1 %cmp, label %diverge, label LOOP_HEAD
	 */
	llvm::Type *int32Typ = llvm::Type::getInt32Ty(parentFun->getContext());
	llvm::Value *zero = llvm::ConstantInt::get(int32Typ, 0);
	llvm::Value *minusOne = llvm::ConstantInt::get(int32Typ, -1, true);

	llvm::Value *val = new llvm::LoadInst(boundAlloca, "bound.val", boundBlock);
	llvm::Value *newVal = llvm::BinaryOperator::CreateNSW(llvm::Instruction::Add, val, minusOne,
							      LOOP_NAME(l) + ".bound.dec", boundBlock);
	llvm::StoreInst *ptr = new llvm::StoreInst(newVal, boundAlloca, boundBlock);
	llvm::Value *cmp = new llvm::ICmpInst(*boundBlock, llvm::ICmpInst::ICMP_EQ, newVal, zero,
					      LOOP_NAME(l) + ".bound.cmp");
	llvm::BranchInst::Create(divergeBlock, l->getHeader(), cmp, boundBlock);

	return boundBlock;
}

/* Redirects all predecessors of l that fulfill the condition f to the block toBlock. */
template<typename Func>
void LoopUnrollPass::redirectLoopPreds(llvm::Loop *l, llvm::BasicBlock *toBlock, Func &&f)
{
	llvm::BasicBlock *lh = l->getHeader();

	for (auto bb = pred_begin(lh); bb != pred_end(lh); ++bb) {
		if (!f(*bb))
			continue;

		TerminatorInst *ti = (*bb)->getTerminator();

		/* Find the successor of bb that redirects to the header */
		for (auto i = 0u; i < ti->getNumSuccessors(); i++) {
			if (*bb != toBlock && ti->getSuccessor(i) == lh)
				ti->setSuccessor(i, toBlock);
		}

		/* Fix any PHI nodes that had bb as incoming in the loop header */
		for (auto it = lh->begin(); auto phi = llvm::dyn_cast<llvm::PHINode>(it); ++it) {
			for (auto i = 0u; i < phi->getNumIncomingValues(); i++) {
				llvm::BasicBlock *ib = phi->getIncomingBlock(i);
				if (ib == *bb)
					phi->setIncomingBlock(i, toBlock);
			}
		}

	}
	return;
}

bool LoopUnrollPass::runOnLoop(llvm::Loop *l, llvm::LPPassManager &lpm)
{
	llvm::Value *boundAlloca = createBoundAlloca(l);
	llvm::BasicBlock *initBlock = createBoundInitBlock(l, boundAlloca);
	llvm::BasicBlock *boundBlock = createBoundDecrBlock(l, boundAlloca);

#ifdef LLVM_GET_ANALYSIS_LOOP_INFO
	l->addBasicBlockToLoop(boundBlock, lpm.getAnalysis<llvm::LoopInfo>().getBase());
#else
	l->addBasicBlockToLoop(boundBlock, lpm.getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo());
#endif

	redirectLoopPreds(l, boundBlock, [&](llvm::BasicBlock *bb){ return l->contains(bb); });
	redirectLoopPreds(l, initBlock, [&](llvm::BasicBlock *bb){ return !l->contains(bb); });

	return true;
}

char LoopUnrollPass::ID = 42;
// static llvm::RegisterPass<LoopUnrollPass> P("loop-unroll",
// 					    "Unrolls all loops at LLVM-IR level.");
