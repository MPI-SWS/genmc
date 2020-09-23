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
#include "Error.hpp"
#include "SpinAssumePass.hpp"
#include "DeclareAssumePass.hpp"
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/CallSite.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#ifdef LLVM_HAS_TERMINATORINST
 typedef llvm::TerminatorInst TerminatorInst;
#else
 typedef llvm::Instruction TerminatorInst;
#endif

void SpinAssumePass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	au.addRequired<llvm::DominatorTreeWrapperPass>();
	au.addRequired<DeclareAssumePass>();
	au.addPreserved<DeclareAssumePass>();
}

bool SpinAssumePass::isAssumeStatement(llvm::Instruction &i) const
{
	llvm::CallInst *ci = llvm::dyn_cast<llvm::CallInst>(&i);
	if (!ci)
		return false;

	llvm::CallSite cs(ci);
	llvm::Function *fun = cs.getCalledFunction();
	return fun && fun->getName().str() == "__VERIFIER_assume";
}

bool SpinAssumePass::isSpinLoop(const llvm::Loop *l) const
{
	for (auto bb = l->block_begin(); bb != l->block_end(); ++bb) {
		for (auto i = (*bb)->begin(); i != (*bb)->end(); ++i) {
			if (i->mayHaveSideEffects() &&
			    !llvm::isa<llvm::LoadInst>(*i))
				return false;
			if (llvm::isa<llvm::AllocaInst>(*i))
				return false;
			if (llvm::isa<llvm::PHINode>(*i))
				return false;
		}
	}
	return true;
}

void SpinAssumePass::addAssumeCallToBlock(llvm::BasicBlock *eb, llvm::BasicBlock *nb,
					  llvm::BranchInst *bi, bool exitOn)
{
	llvm::Value *cond;
	llvm::MDNode *mdata, *extraMdata;
	llvm::BinaryOperator *bop;
	llvm::Function *assumeFun;
	llvm::Type *assumeArgTyp;
	llvm::ZExtInst *ei;
	llvm::Instruction *ci, *newBI;

	cond = bi->getCondition();
	mdata = bi->getMetadata("dbg");
	extraMdata = llvm::MDNode::get(bi->getContext(), llvm::MDString::get(bi->getContext(), "spinloop"));
	eb->getInstList().pop_back();
	if (!exitOn) {
		bop = llvm::BinaryOperator::CreateNot(cond, "notcond", eb);
		bop->setMetadata("dbg", mdata);
		cond = bop;
	}

	assumeFun = eb->getParent()->getParent()->getFunction("__VERIFIER_assume");
	assumeArgTyp = assumeFun->arg_begin()->getType();
	BUG_ON(!assumeArgTyp->isIntegerTy());
	if (assumeArgTyp->getIntegerBitWidth() != 1) {
		ei = new llvm::ZExtInst(cond, assumeArgTyp, "", eb);
		ei->setMetadata("dbg", mdata);
		cond = ei;
	}
	ci = llvm::CallInst::Create(assumeFun, {cond}, "", eb);
	ci->setMetadata("dbg", mdata);
	ci->setMetadata("assume.kind", extraMdata);
	newBI = llvm::BranchInst::Create(nb, eb);
	newBI->setMetadata("dbg", mdata);
	return;
}

void SpinAssumePass::removeDisconnectedBlocks(llvm::Loop *l)
{
	bool done;

	while (l) {
		done = false;
		while (!done) {
			done = true;
			VSet<llvm::BasicBlock*> hasPredecessor;

			/*
			 * We collect blocks with predecessors in l, and we also
			 * search for BBs without successors in l
			 */
			for (auto it = l->block_begin(); done && it != l->block_end(); ++it) {
				TerminatorInst *T = (*it)->getTerminator();
				bool hasLoopSuccessor = false;

				for(auto i = 0u; i < T->getNumSuccessors(); ++i) {
					if(l->contains(T->getSuccessor(i))){
						hasLoopSuccessor = true;
						hasPredecessor.insert(T->getSuccessor(i));
					}
				}

				if (!hasLoopSuccessor) {
					done = false;
					l->removeBlockFromLoop(*it);
				}
			}

			/* Find BBs without predecessors in l */
			for(auto it = l->block_begin(); done && it != l->block_end(); ++it){
				if(hasPredecessor.count(*it) == 0) {
					done = false;
					l->removeBlockFromLoop(*it);
				}
			}
		}
		l = l->getParentLoop();
	}
}

bool SpinAssumePass::transformLoop(llvm::Loop *l, llvm::LPPassManager &lpm)
{
	llvm::BasicBlock *eb, *nb;
	TerminatorInst *ei;
	llvm::BranchInst *bi;
	bool exitOn = false;

	/* Has the loop more than one exiting blocks? */
	eb = l->getExitingBlock();
	if (!eb)
		return false;

	/* Is the last instruction of the loop a branch? */
	ei = eb->getTerminator();
	BUG_ON(!ei);
	bi = llvm::dyn_cast<llvm::BranchInst>(ei);
	if (!bi || !bi->isConditional())
		return false;

	nb = l->getExitBlock();
	BUG_ON(!nb || bi->getNumSuccessors() != 2);
	if (bi->getSuccessor(0) == nb)
		exitOn = true;
	addAssumeCallToBlock(eb, nb, bi, exitOn);
	removeDisconnectedBlocks(l);
	return true;
}

bool SpinAssumePass::runOnLoop(llvm::Loop *l, llvm::LPPassManager &lpm)
{
	bool modified = false;

	if (isSpinLoop(l))
		if (transformLoop(l, lpm)) {
#ifdef LLVM_HAVE_LOOPINFO_ERASE
			lpm.getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo().erase(l);
			lpm.markLoopAsDeleted(*l);
#elif  LLVM_HAVE_LOOPINFO_MARK_AS_REMOVED
			lpm.getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo().markAsRemoved(l);
#else
			lpm.deleteLoopFromQueue(l);
#endif
			modified = true;
		}

	return modified;
}

char SpinAssumePass::ID = 42;
static llvm::RegisterPass<SpinAssumePass> P("spin-assume",
					    "Replaces spin-loops with __VERIFIER_assume().");
