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

#include "LocalSimplifyCFGPass.hpp"
#include "Static/LLVMUtils.hpp"

#include <llvm/ADT/STLExtras.h>
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
