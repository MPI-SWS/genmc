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

#ifndef GENMC_BARRIER_RESULT_CHECKER_PASS_HPP
#define GENMC_BARRIER_RESULT_CHECKER_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class PassModuleInfo;
enum class BarrierRetResult : std::uint8_t;

class BarrierResultAnalysis : public AnalysisInfoMixin<BarrierResultAnalysis> {
public:
	using Result = std::optional<BarrierRetResult>;
	auto run(Module &M, ModuleAnalysisManager &AM) -> Result;

	auto calculate(Module &M) -> Result;

private:
	friend AnalysisInfoMixin<BarrierResultAnalysis>;
	static inline AnalysisKey Key;

	Result resultsUsed{};
};

class BarrierResultCheckerPass : public PassInfoMixin<BarrierResultCheckerPass> {
public:
	BarrierResultCheckerPass(PassModuleInfo &PMI) : PMI(PMI) {}

	auto run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses;

private:
	PassModuleInfo &PMI;
};

#endif /* GENMC_BARRIER_RETURN_UNUSED_PASS_HPP */
