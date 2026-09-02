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

#include "CallInfoCollectionPass.hpp"
#include "genmc/ADT/VSet.hpp"
#include "passes/InternalFunctions.hpp"
#include "passes/LLVMUtils.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Casting.h>

#include <algorithm>

using namespace llvm;

static auto hasSideEffects(Function *F, SmallVector<Function *, 4> &chain, VSet<Function *> &clean,
			   VSet<Function *> &dirty) -> bool
{
	if (!F || dirty.contains(F))
		return true;
	if (clean.contains(F) || std::ranges::find(chain, F) != chain.end())
		return false;
	if (F->empty())
		return !isCleanInternalFunction(F->getName().str());

	for (auto it = inst_begin(*F), ei = inst_end(*F); it != ei; ++it) {
		if (!hasSideEffects(&*it))
			continue;

		/* Found an instruction w/ side effects; check type */
		if (!isa<CallInst>(&*it))
			return true;

		/* If it is a CallInst, recurse */
		if (auto *ci = dyn_cast<CallInst>(&*it)) {
			chain.push_back(F);
			if (hasSideEffects(ci->getCalledFunction(), chain, clean, dirty))
				return true;
			chain.pop_back();
		}
	}

	if (chain.empty())
		clean.insert(F);
	return false;
}

static auto hasSideEffects(Function *F, VSet<Function *> &clean, VSet<Function *> &dirty) -> bool
{
	SmallVector<Function *, 4> chain;
	return hasSideEffects(F, chain, clean, dirty);
}

static auto isAllocating(Function *F) -> bool
{
	if (!F->getReturnType()->isPointerTy())
		return false;

	SmallVector<ReturnInst *, 4> rets;
	std::for_each(inst_begin(*F), inst_end(*F),
		      [&](Instruction &i) { /* transform-if */
					    if (auto *ri = dyn_cast<ReturnInst>(&i))
						    rets.push_back(ri);
		      });

	return std::ranges::all_of(rets, [](const ReturnInst *ri) {
		auto *rv = ri->getReturnValue();
		return isa<Instruction>(rv) && isAlloc(dyn_cast<Instruction>(rv));
	});
}

auto CallAnalysis::run(Module &M, ModuleAnalysisManager & /*MAM*/) -> Result
{
	VSet<Function *> dirty;

	result_.clean.clear();
	result_.alloc.clear();
	for (auto &F : M) {
		if (hasSideEffects(&F, result_.clean, dirty))
			dirty.insert(&F);
		else
			result_.clean.insert(&F);
		if (isAllocating(&F))
			result_.alloc.insert(&F);
	}
	return result_;
}

auto CallAnalysisPass::run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses
{
	AR_ = MAM.getResult<CallAnalysis>(M);
	return PreservedAnalyses::all();
}
