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

#ifndef GENMC_LOOP_THREAD_JUMPING_PASS_HPP
#define GENMC_LOOP_THREAD_JUMPING_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class LoopJumpThreadingPass : public PassInfoMixin<LoopJumpThreadingPass> {
public:
	auto run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR, LPMUpdater &U)
		-> PreservedAnalyses;
};

#endif /* GENMC_LOOP_THREAD_JUMPING_PASS_HPP */
