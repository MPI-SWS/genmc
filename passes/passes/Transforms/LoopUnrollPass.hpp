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

#ifndef GENMC_LOOP_UNROLL_PASS_HPP
#define GENMC_LOOP_UNROLL_PASS_HPP

#include "genmc/ADT/VSet.hpp"
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class LoopUnrollPass : public PassInfoMixin<LoopUnrollPass> {
public:
	LoopUnrollPass(unsigned int depth, const VSet<std::string> &noUnrollFuns = {})
		: unrollDepth_(depth), noUnroll_(noUnrollFuns)
	{}

	auto run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR, LPMUpdater &U)
		-> PreservedAnalyses;

private:
	auto shouldUnroll(llvm::Loop *l) const -> bool
	{
		return !noUnroll_.contains((*l->block_begin())->getParent()->getName().str());
	}

	unsigned int unrollDepth_{};
	VSet<std::string> noUnroll_;
};

#endif /* GENMC_LOOP_UNROLL_PASS_HPP */
