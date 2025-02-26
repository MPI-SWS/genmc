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

#include "LocalSimplifyCFGPass.hpp"
#include "Static/LLVMUtils.hpp"
#include "config.h"

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/Local.h>

using namespace llvm;

static auto foldSuccessors(BasicBlock *bb) -> bool
{
	auto *bi = dyn_cast<BranchInst>(bb->getTerminator());
	if (!bi || !bi->isConditional())
		return false;

	return tryThreadSuccessor(bi, bi->getSuccessor(0)) ||
	       (tryThreadSuccessor(bi, bi->getSuccessor(1)) != nullptr);
}

static auto localSimplifyCFG(Function &F) -> bool
{
	auto modified = false;

	// /* Unfortunately, we cannot do much here: try some simple jump
	//  * threading to simplify the code a bit */
	// auto bbIt = F.begin();
	// while (bbIt != F.end()) {
	// 	auto tmpIt = bbIt++;
	// // for (auto &BB : F) {
	// 	if (foldSuccessors(&*tmpIt))
	// 		;
	// }
	// EliminateUnreachableBlocks(F);

	/* And then see if we can merge any blocks without invalidating ctrl deps */
	auto bbIt = F.begin();
	while (bbIt != F.end()) {
		/* Advance the iterator so that it does get invalidated.
		 * The advanced iterator will not get invalidated (due to ilist) */
		auto tmpIt = bbIt++;

		auto *pred = tmpIt->getSinglePredecessor();
		if (!pred)
			continue;
		/* Ensure it's not a conditional branch always jumping to the same block */
		auto *predTerm = dyn_cast<BranchInst>(pred->getTerminator());
		if (predTerm && predTerm->isUnconditional())
			modified |= MergeBlockIntoPredecessor(&*tmpIt);
	}

	return modified;
}

auto LocalSimplifyCFGPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	return localSimplifyCFG(F) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
