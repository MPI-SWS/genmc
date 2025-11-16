/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */


#include "CodeCondenserPass.hpp"
#include "Static/Transforms/BisimilarityCheckerPass.hpp"
#include "Support/Error.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <unordered_map>

using namespace llvm;
using BsPoint = BisimilarityAnalysis::BisimilarityPoint;

auto CodeCondenserPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	auto bsps = FAM.getResult<BisimilarityAnalysis>(F);

	/* Map blocks to bisimilarity points */
	std::unordered_map<BasicBlock *, std::vector<BsPoint>> bbsToBsps;
	for (const auto &p : bsps)
		bbsToBsps[p.first->getParent()].push_back(p);

	/* Condense the code: */
	for (auto &bbP : bbsToBsps) {
		/* find the first bisimilar point within each block (should not be
		 * TerminatorInst)... */
		for (auto &i : *bbP.first) {
			auto pIt = std::find_if(bbP.second.begin(), bbP.second.end(),
						[&](const BsPoint &p) { return p.first == &i; });
			if (pIt == bbP.second.end())
				continue;
			if (pIt->first->isTerminator())
				continue;

			/* ...and do the actual condensing */
			auto aIt = pIt->first->getIterator();
			auto bIt = pIt->second->getIterator();
			auto *aBB = aIt->getParent();
			auto *bBB = bIt->getParent();

			auto *aNewBB = (aBB->begin() != aIt || &*F.begin() == aBB)
					       ? aBB->splitBasicBlock(aIt)
					       : aBB;
			auto *bNewBB = (bBB->begin() != bIt || &*F.begin() == bBB)
					       ? bBB->splitBasicBlock(bIt)
					       : bBB;
			auto predIt = pred_begin(aNewBB);
			while (predIt != pred_end(aNewBB)) {
				auto tmpIt = predIt++;
				auto pred = *tmpIt;
				auto *term = dyn_cast<BranchInst>(pred->getTerminator());
				if (!term)
					continue;

				for (auto i = 0U; i < term->getNumSuccessors(); i++)
					if (term->getSuccessor(i) == aNewBB)
						term->setSuccessor(i, bNewBB);
			}
			break;
		}
	}
	/* Clear unnecessary blocks */
	EliminateUnreachableBlocks(F);
	return PreservedAnalyses::none();
}
