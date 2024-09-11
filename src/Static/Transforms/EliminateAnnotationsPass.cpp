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

#include "EliminateAnnotationsPass.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "Static/LLVMUtils.hpp"
#include "Support/Error.hpp"
#include "config.h"

#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm;

#define POSTDOM_PASS PostDominatorTreeWrapperPass
#define GET_POSTDOM_PASS() getAnalysis<POSTDOM_PASS>().getPostDomTree();

auto isAnnotationBegin(Instruction *i) -> bool
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::AnnotateBegin;
}

auto isAnnotationEnd(Instruction *i) -> bool
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::AnnotateEnd;
}

auto getAnnotationValue(CallInst *ci) -> uint64_t
{
	auto *funArg = llvm::dyn_cast<ConstantInt>(ci->getOperand(0));
	BUG_ON(!funArg);
	return funArg->getValue().getLimitedValue();
}

auto annotateInstructions(CallInst *begin, CallInst *end) -> bool
{
	if (!begin || !end)
		return false;

	auto annotType = getAnnotationValue(begin);
	auto beginFound = false;
	auto endFound = false;
	unsigned opcode = 0; /* no opcode == 0 in LLVM */
	foreachInBackPathTo(end->getParent(), begin->getParent(), [&](Instruction &i) {
		/* wait until we find the end (e.g., if in same block) */
		if (!endFound) {
			endFound |= (dyn_cast<CallInst>(&i) == end);
			return;
		}
		/* check until we find the begin; only deal with atomic insts */
		if (endFound && !beginFound && i.isAtomic() && !isa<FenceInst>(i)) {
			if (!opcode)
				opcode = i.getOpcode();
			BUG_ON(opcode != i.getOpcode()); /* annotations across paths must match */
			annotateInstruction(&i, "genmc.attr", annotType);
		}
		/* stop when the begin is found; reset vars for next path */
		if (!beginFound) {
			beginFound |= (dyn_cast<CallInst>(&i) == begin);
			if (beginFound)
				beginFound = endFound = false;
		}
	});
	return true;
}

auto findMatchingEnd(CallInst *begin, const std::vector<CallInst *> &ends, DominatorTree &DT,
		     PostDominatorTree &PDT) -> CallInst *
{
	auto it = std::find_if(ends.begin(), ends.end(), [&](auto *ei) {
		return getAnnotationValue(begin) == getAnnotationValue(ei) &&
		       DT.dominates(begin, ei) &&
		       PDT.dominates(ei->getParent(), begin->getParent()) &&
		       std::none_of(ends.begin(), ends.end(), [&](auto *ei2) {
			       return ei != ei2 && DT.dominates(begin, ei2) &&
				      PDT.dominates(ei2->getParent(), begin->getParent()) &&
				      DT.dominates(ei2, ei);
		       });
	});
	BUG_ON(it == ends.end());
	return *it;
}

auto EliminateAnnotationsPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	std::vector<CallInst *> begins;
	std::vector<CallInst *> ends;

	for (auto &i : instructions(F)) {
		if (isAnnotationBegin(&i))
			begins.push_back(dyn_cast<CallInst>(&i));
		else if (isAnnotationEnd(&i))
			ends.push_back(dyn_cast<CallInst>(&i));
	}

	auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
	auto &PDT = FAM.getResult<PostDominatorTreeAnalysis>(F);
	VSet<Instruction *> toDelete;

	auto changed = false;
	for (auto *bi : begins) {
		auto *ei = findMatchingEnd(bi, ends, DT, PDT);
		BUG_ON(!ei);
		changed |= annotateInstructions(bi, ei);
		toDelete.insert(bi);
		toDelete.insert(ei);
	}
	for (auto *i : toDelete) {
		i->eraseFromParent();
		changed = true;
	}
	return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
