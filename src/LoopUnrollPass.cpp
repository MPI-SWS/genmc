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
#include "DeclareInternalsPass.hpp"
#include "Error.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#ifdef LLVM_ALLOCAINST_TAKES_ALIGN
# include <llvm/Support/Alignment.h>
#endif
#include <llvm/Transforms/Utils/Cloning.h>

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

#ifdef LLVM_LOADINST_VALUE_ONLY
# define GET_LOADINST_ARG(val)
#else
# define GET_LOADINST_ARG(val) (val)->getType()->getPointerElementType(),
#endif

#ifdef LLVM_ALLOCAINST_TAKES_ADDRSPACE
# define GET_BOUND_ALLOCA_DUMMY_ARGS() 0, NULL
#else
# define GET_BOUND_ALLOCA_DUMMY_ARGS() NULL
#endif

#ifdef LLVM_HAS_ALIGN
# define GET_BOUND_ALLOCA_ALIGN_ARG(val) llvm::Align(val)
#else
# define GET_BOUND_ALLOCA_ALIGN_ARG(val) val
#endif

using namespace llvm;

void LoopUnrollPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	LoopPass::getAnalysisUsage(au);
	au.addRequired<DeclareInternalsPass>();
	au.addPreserved<DeclareInternalsPass>();
}

/*
 * Returns a PHI node the only purpose of which is to serve as the
 * bound variable for this loop.  It does not set up the arguments for
 * the PHI node.
 */
PHINode *createBoundInit(Loop *l)
{
	Function *parentFun = (*l->block_begin())->getParent();
	Type *int32Typ = Type::getInt32Ty(parentFun->getContext());

	PHINode *bound = PHINode::Create(int32Typ, 0, "bound.val", &*l->getHeader()->begin());
	return bound;
}

/*
 * Returns an instruction which decrements the bound variable for this loop (BOUNDVAL).
 * The returned value should be later checked in order to bound the loop.
 */
BinaryOperator *createBoundDecrement(Loop *l, PHINode *boundVal)
{
	Function *parentFun = (*l->block_begin())->getParent();
	Type *int32Typ = Type::getInt32Ty(parentFun->getContext());

	Value *minusOne = ConstantInt::get(int32Typ, -1, true);
	Instruction *pt = &*l->getHeader()->getFirstInsertionPt();
	return BinaryOperator::CreateNSW(Instruction::Add, boundVal, minusOne,
					 LOOP_NAME(l) + ".bound.dec", pt);
}

void addBoundCmpAndSpinEndBefore(Loop *l, PHINode *val, BinaryOperator *decVal)
{
	Function *parentFun = (*l->block_begin())->getParent();
	Function *endLoopFun = parentFun->getParent()->getFunction("__VERIFIER_kill_thread");
	Type *int32Typ = Type::getInt32Ty(parentFun->getContext());


	Value *zero = ConstantInt::get(int32Typ, 0);
	Value *cmp = new ICmpInst(decVal, ICmpInst::ICMP_EQ, val, zero,
				  LOOP_NAME(l) + ".bound.cmp");

	BUG_ON(!endLoopFun);
	CallInst::Create(endLoopFun, {cmp}, "", decVal);
	return;
}

bool LoopUnrollPass::runOnLoop(Loop *l, LPPassManager &lpm)
{
	if (!shouldUnroll(l))
		return false;

	PHINode *val = createBoundInit(l);
	BinaryOperator *dec = createBoundDecrement(l, val);
	Type *int32Typ = Type::getInt32Ty((*l->block_begin())->getParent()->getContext());

	/* Adjust incoming values for the bound variable */
	for (BasicBlock *bb : predecessors(l->getHeader()))
		val->addIncoming(l->contains(bb) ? (Value *) dec : (Value *)
				 ConstantInt::get(int32Typ, unrollDepth), bb);

	/* Finally, compare the bound and block if it reaches zero */
	addBoundCmpAndSpinEndBefore(l, val, dec);
	return true;
}

Pass *createLoopUnrollPass(int depth, const VSet<std::string> &noUnrollFuns /* = {} */)
{
	return new LoopUnrollPass(depth, noUnrollFuns);
}

char LoopUnrollPass::ID = 42;
// static RegisterPass<LoopUnrollPass> P("loop-unroll",
// 					    "Unrolls all loops at LLVM-IR level.");
