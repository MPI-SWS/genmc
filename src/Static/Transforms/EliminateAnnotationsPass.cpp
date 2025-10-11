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

#include "EliminateAnnotationsPass.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "Static/LLVMUtils.hpp"
#include "Support/Error.hpp"
#include "Verification/Config.hpp"

#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm;
using AnnotationOptions = EliminateAnnotationsPass::AnnotationOptions;

#define POSTDOM_PASS PostDominatorTreeWrapperPass
#define GET_POSTDOM_PASS() getAnalysis<POSTDOM_PASS>().getPostDomTree();

static auto isAnnotationBegin(Instruction *i) -> bool
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::AnnotateBegin;
}

static auto isAnnotationEnd(Instruction *i) -> bool
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::AnnotateEnd;
}

static auto isMutexCall(Instruction *i) -> bool
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) && isMutexCode(internalFunNames.at(name));
}

/* We only annotate atomic instructions and selected intrinsics (e.g., locks) */
static auto isAnnotatable(Instruction *i) -> bool
{
	return (i->isAtomic() && !isa<FenceInst>(i)) || isMutexCall(i);
}

static auto getAnnotationValue(CallInst *ci) -> uint64_t
{
	auto *funArg = llvm::dyn_cast<ConstantInt>(ci->getOperand(0));
	BUG_ON(!funArg);
	return funArg->getValue().getLimitedValue();
}

static auto shouldAnnotate(const AnnotationOptions &options, uint64_t annotType) -> bool
{
	auto isHelperAnnot = [](uint64_t annotType) {
		return annotType == GENMC_KIND_HELPED || annotType == GENMC_KIND_HELPING;
	};
	auto isConfAnnot = [](uint64_t annotType) {
		return annotType == GENMC_KIND_CONFIRM || annotType == GENMC_KIND_SPECUL;
	};

	if (isHelperAnnot(annotType) && !options.annotHelper)
		return false;
	if (isConfAnnot(annotType) && !options.annotConf)
		return false;
	if (annotType == GENMC_ATTR_FINAL && !options.annotFinal)
		return false;
	return true;
}

static auto annotateInstructions(CallInst *begin, CallInst *end, const AnnotationOptions &options)
	-> bool
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
		/* check until we find the begin; we only annotate atomics */
		if (endFound && !beginFound && isAnnotatable(&i)) {
			if (!opcode)
				opcode = i.getOpcode();
			BUG_ON(opcode != i.getOpcode()); /* annotations across paths must match */
			if (shouldAnnotate(options, annotType))
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

static auto findMatchingEnd(CallInst *begin, const std::vector<CallInst *> &ends, DominatorTree &DT,
			    PostDominatorTree &PDT) -> CallInst *
{
	auto it = std::find_if(ends.begin(), ends.end(), [&](auto *ei) {
		return getAnnotationValue(begin) == getAnnotationValue(ei) &&
		       DT.dominates(begin, ei) &&
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
		changed |= annotateInstructions(bi, ei, getOptions());
		toDelete.insert(bi);
		toDelete.insert(ei);
	}
	for (auto *i : toDelete) {
		i->eraseFromParent();
		changed = true;
	}
	return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
