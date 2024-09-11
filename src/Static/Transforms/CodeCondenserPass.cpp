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
#include "Static/LLVMUtils.hpp"
#include "Static/Transforms/BisimilarityCheckerPass.hpp"
#include "Support/Error.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/InstIterator.h>
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
