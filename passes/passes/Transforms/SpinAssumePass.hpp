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

#ifndef GENMC_SPIN_ASSUME_PASS_HPP
#define GENMC_SPIN_ASSUME_PASS_HPP

#include "genmc/ADT/VSet.hpp"

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

/** Convert spinloops to assume statements */
class SpinAssumePass : public PassInfoMixin<SpinAssumePass> {
public:
	SpinAssumePass(bool markStarts = false) : markStarts_(markStarts) {}

	auto run(Module &M, ModuleAnalysisManager &MAM) const -> PreservedAnalyses;

private:
	/** Whether we should mark spinloop starts */
	bool markStarts_{};
};

#endif /* GENMC_SPIN_ASSUME_PASS_HPP */
