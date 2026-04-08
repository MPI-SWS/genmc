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

#include "BarrierResultCheckerPass.hpp"
#include "genmc/Execution/LoadAnnotation.hpp"
#include "passes/LLVMUtils.hpp"
#include "passes/ModuleInfo.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>

/* Checks whether the results of barrier_wait() and co are used.
 * (Info relevant to BAM deployment.) */

using namespace llvm;

/* Needs to match our include/ */
#define PTHREAD_BARRIER_SERIAL_THREAD -1

static auto isBarrierAssume(Instruction *i) -> bool
{
	auto *ci = dyn_cast<CallInst>(i);
	if (!ci || !isAssumeFunction(getCalledFunOrStripValName(*ci)))
		return false;

	auto *arg = dyn_cast<ConstantInt>(ci->getOperand(1));
	return arg && arg->getLimitedValue() ==
			      static_cast<std::underlying_type_t<AssumeType>>(AssumeType::Barrier);
}

/* The only use we allow is jumping to a given block if the result of barrier_wait)
 * is neither 0 nor 1. */
static auto isAllowedResultUse(User *user) -> bool
{
	/* Allow non-instruction users, as well as instructions that... */
	auto *i = llvm::dyn_cast<Instruction>(user);
	if (!i || i->isLifetimeStartOrEnd() || i->isDebugOrPseudoInst())
		return true;

	/* are icmp ne... */
	auto *icmp = dyn_cast<ICmpInst>(i);
	if (!icmp || icmp->getPredicate() != ICmpInst::ICMP_NE ||
	    !isa<ConstantInt>(icmp->getOperand(1)))
		return false;

	/* compare with 0 and PTHREAD_BARRIER_SERIAL_THREAD... */
	auto arg = llvm::cast<ConstantInt>(icmp->getOperand(1))->getSExtValue();
	if (arg != 0 && arg != PTHREAD_BARRIER_SERIAL_THREAD)
		return false;

	/* and all subsequent users are branches leading to the same block */
	return std::ranges::all_of(icmp->users(), [](auto *u) {
		if (!llvm::isa<Instruction>(u))
			return true;
		auto *bi = llvm::dyn_cast<BranchInst>(u);
		if (!bi)
			return false;
		return !bi->isConditional() || bi->getSuccessor(0) == bi->getSuccessor(1);
	});
}

static auto isBarrierResultUsed(CallInst *ci) -> bool
{
	auto siIt = std::find_if(ci->getIterator(), ci->getParent()->end(),
				 [](auto &i) { return isa<SelectInst>(&i); });

	/* No select instruction found => ternary operator removed => result unused */
	if (siIt == ci->getParent()->end())
		return false;

	return std::ranges::any_of(siIt->users(), [](auto *u) { return !isAllowedResultUse(u); });
}

auto BarrierResultAnalysis::run(Module &M, ModuleAnalysisManager &MAM)
	-> BarrierResultAnalysis::Result
{
	auto result = false;
	for (auto &F : M) {
		for (auto &i : instructions(F)) {
			if (!isBarrierAssume(&i))
				continue;

			result |= isBarrierResultUsed(llvm::cast<CallInst>(&i));
		}
	}
	return result ? std::make_optional(BarrierRetResult::Used)
		      : std::make_optional(BarrierRetResult::Unused);
}

auto BarrierResultCheckerPass::run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses
{
	PMI.barrierResultsUsed = MAM.getResult<BarrierResultAnalysis>(M);
	return PreservedAnalyses::all();
}
