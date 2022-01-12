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

#include "CodeCondenserPass.hpp"
#include "BisimilarityCheckerPass.hpp"
#include "Error.hpp"
#ifdef LLVM_HAVE_ELIMINATE_UNCREACHABLE_BLOCKS
#include <llvm/Analysis/DomTreeUpdater.h>
#endif
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <unordered_map>

using namespace llvm;
using BsPoint = BisimilarityCheckerPass::BisimilarityPoint;

void CodeCondenserPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	au.addRequired<BisimilarityCheckerPass>();
}

/* Copy portions of LLVM code for older LLVM versions */
#ifndef LLVM_HAVE_ELIMINATE_UNREACHABLE_BLOCKS

#include <llvm/IR/Dominators.h>

#ifndef LLVM_HAVE_DF_ITERATOR_DEFAULT_SET
template <typename NodeRef, unsigned SmallSize=8>
struct df_iterator_default_set : public SmallPtrSet<NodeRef, SmallSize> {
  typedef SmallPtrSet<NodeRef, SmallSize>  BaseSet;
  typedef typename BaseSet::iterator iterator;
  std::pair<iterator,bool> insert(NodeRef N) { return BaseSet::insert(N) ; }
  template <typename IterT>
  void insert(IterT Begin, IterT End) { BaseSet::insert(Begin,End); }

  void completed(NodeRef) { }
};
#endif

namespace llvm {
	class DomTreeUpdater;
}

void DetatchDeadBlocks(
	ArrayRef<BasicBlock *> BBs,
	// SmallVectorImpl<DominatorTree::UpdateType> *Updates,
	bool KeepOneInputPHIs)
{
	for (auto *BB : BBs) {
		// Loop through all of our successors and make sure they know that one
		// of their predecessors is going away.
		SmallPtrSet<BasicBlock *, 4> UniqueSuccessors;
		for (BasicBlock *Succ : successors(BB)) {
			Succ->removePredecessor(BB, KeepOneInputPHIs);
			// if (Updates && UniqueSuccessors.insert(Succ).second)
			//   Updates->push_back({DominatorTree::Delete, BB, Succ});
		}

		// Zap all the instructions in the block.
		while (!BB->empty()) {
			Instruction &I = BB->back();
			// If this instruction is used, replace uses with an arbitrary value.
			// Because control flow can't get here, we don't care what we replace the
			// value with.  Note that since this block is unreachable, and all values
			// contained within it must dominate their uses, that all uses will
			// eventually be removed (they are themselves dead).
			if (!I.use_empty())
				I.replaceAllUsesWith(UndefValue::get(I.getType()));
			BB->getInstList().pop_back();
		}
		new UnreachableInst(BB->getContext(), BB);
		assert(BB->getInstList().size() == 1 &&
		       isa<UnreachableInst>(BB->getTerminator()) &&
		       "The successor list of BB isn't empty before "
		       "applying corresponding DTU updates.");
	}
}

void DeleteDeadBlocks(ArrayRef <BasicBlock *> BBs, DomTreeUpdater *DTU,
		      bool KeepOneInputPHIs)
{
// #ifndef NDEBUG
// 	// Make sure that all predecessors of each dead block is also dead.
// 	SmallPtrSet<BasicBlock *, 4> Dead(BBs.begin(), BBs.end());
// 	assert(Dead.size() == BBs.size() && "Duplicating blocks?");
// 	for (auto *BB : Dead)
// 		for (BasicBlock *Pred : predecessors(BB))
// 			assert(Dead.count(Pred) && "All predecessors must be dead!");
// #endif

	// SmallVector<DominatorTree::UpdateType, 4> Updates;
	DetatchDeadBlocks(BBs, // DTU ? &Updates : nullptr,
			  KeepOneInputPHIs);

	// if (DTU)
	//   DTU->applyUpdatesPermissive(Updates);

	for (BasicBlock *BB : BBs)
		// if (DTU)
		//   DTU->deleteBB(BB);
		// else
		BB->eraseFromParent();
}

void DeleteDeadBlock(BasicBlock *BB, DomTreeUpdater *DTU = nullptr,
		     bool KeepOneInputPHIs = false)
{
	DeleteDeadBlocks({BB}, DTU, KeepOneInputPHIs);
}

bool EliminateUnreachableBlocks(Function &F, DomTreeUpdater *DTU = nullptr,
				bool KeepOneInputPHIs = false)
{
	df_iterator_default_set<BasicBlock*> Reachable;

	// Mark all reachable blocks.
	for (BasicBlock *BB : depth_first_ext(&F, Reachable))
		(void)BB/* Mark all reachable blocks */;

	// Collect all dead blocks.
	std::vector<BasicBlock*> DeadBlocks;
	for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I)
		if (!Reachable.count(&*I)) {
			BasicBlock *BB = &*I;
			DeadBlocks.push_back(BB);
		}

	// Delete the dead blocks.
	DeleteDeadBlocks(DeadBlocks, DTU, KeepOneInputPHIs);

	return !DeadBlocks.empty();
}
#endif /* !LLVM_HAVE_ELIMINATE_UNREACHABLE_BLOCKS */

bool CodeCondenserPass::runOnFunction(Function &F)
{
	auto &bsps = getAnalysis<BisimilarityCheckerPass>().getFuncBsPoints(&F);

	/* Map blocks to bisimilarity points */
	std::unordered_map<BasicBlock *, std::vector<BsPoint> > bbsToBsps;
	for (auto &p : bsps)
		bbsToBsps[p.first->getParent()].push_back(p);

	/* Condense the code: */
	for (auto &bbP : bbsToBsps) {
		/* find the first bisimilar point within each block (should not be TerminatorInst)... */
		for (auto &i : *bbP.first) {
			auto pIt = std::find_if(bbP.second.begin(), bbP.second.end(),
					       [&](const BsPoint &p){ return p.first == &i; });
			if (pIt == bbP.second.end())
				continue;
			if (pIt->first->isTerminator())
				continue;

			/* ...and do the actual condensing */
			auto aIt = pIt->first->getIterator();
			auto bIt = pIt->second->getIterator();
			auto *aBB = aIt->getParent();
			auto *bBB = bIt->getParent();

			auto *aNewBB = (aBB->begin() != aIt || &*F.begin() == aBB) ? aBB->splitBasicBlock(aIt) : aBB;
			auto *bNewBB = (bBB->begin() != bIt || &*F.begin() == bBB) ? bBB->splitBasicBlock(bIt) : bBB;
			auto predIt = pred_begin(aNewBB);
			while (predIt != pred_end(aNewBB)) {
				auto tmpIt = predIt++;
				auto pred = *tmpIt;
				auto *term = dyn_cast<BranchInst>(pred->getTerminator());
				if (!term)
					continue;

				for (auto i = 0u; i < term->getNumSuccessors(); i++)
					if (term->getSuccessor(i) == aNewBB)
						term->setSuccessor(i, bNewBB);
			}
			break;
		}
	}
	/* Clear unnecessary blocks */
	EliminateUnreachableBlocks(F);
	return true;
}

FunctionPass *createCodeCondenserPass()
{
	return new CodeCondenserPass();
}

char CodeCondenserPass::ID = 42;
static llvm::RegisterPass<CodeCondenserPass> P("code-condenser",
					       "Reduces the size of the code by leveraging bisimilarity information.");
