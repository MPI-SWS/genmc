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

#include "FunctionInlinerPass.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "config.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace llvm;

static auto isInlinable(CallGraph &CG, CallGraphNode *node, SmallVector<CallGraphNode *, 4> &chain)
	-> bool
{
	/* Base cases: indirect/empty/recursive calls */
	auto *F = node->getFunction();
	if (node == CG.getCallsExternalNode())
		return false;
	if (F->empty())
		return isInternalFunction(F->getName().str());
	if (std::find(chain.begin(), chain.end(), node) != chain.end())
		return false;

	chain.push_back(node);
	for (auto &c : *node) {
		if (!isInlinable(CG, c.second, chain))
			return false;
	}
	chain.pop_back();
	return true;
}

static auto isInlinable(CallGraph &CG, Function &F) -> bool
{
	SmallVector<CallGraphNode *, 4> chain;
	return isInlinable(CG, CG[&F], chain);
}

static auto inlineCall(CallInst *ci) -> bool
{
	llvm::InlineFunctionInfo ifi;

#if LLVM_VERSION_MAJOR >= 11
	return InlineFunction(*ci, ifi).isSuccess();
#else
	return InlineFunction(ci, ifi);
#endif
}

static auto inlineFunction(Module &M, Function *toInline) -> bool
{
	std::vector<CallInst *> calls;
	for (auto &F : M) {
		for (auto &iit : instructions(F)) {
			auto *ci = dyn_cast<CallInst>(&iit);
			if (!ci)
				continue;

			if (ci->getCalledFunction() == toInline)
				calls.push_back(ci);
		}
	}

	auto changed = false;
	for (auto *ci : calls) {
		changed |= inlineCall(ci);
	}
	return changed;
}

auto FunctionInlinerPass::run(Module &M, ModuleAnalysisManager &AM) -> PreservedAnalyses
{
	CallGraph CG(M);

	auto changed = false;
	for (auto &F : M) {
		if (!F.empty() && isInlinable(CG, F)) {
			changed |= inlineFunction(M, &F);
		}
	}
	return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
