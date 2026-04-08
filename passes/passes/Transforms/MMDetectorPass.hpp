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

#ifndef GENMC_MM_DETECTOR_PASS_HPP
#define GENMC_MM_DETECTOR_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

#include <optional>

using namespace llvm;

class PassModuleInfo;
enum class ModelType : std::uint8_t;

class MMAnalysis : public AnalysisInfoMixin<MMAnalysis> {
public:
	using Result = std::optional<ModelType>;
	auto run(Module &M, ModuleAnalysisManager &AM) -> Result;

	auto calculate(Module &M) -> Result;

private:
	friend AnalysisInfoMixin<MMAnalysis>;
	static inline AnalysisKey Key;

	ModelType model{};
};

class MMDetectorPass : public AnalysisInfoMixin<MMDetectorPass> {
public:
	MMDetectorPass(PassModuleInfo &PMI) : PMI(PMI) {}

	auto run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses;

private:
	PassModuleInfo &PMI;
};

#endif /* GENMC_MM_DETECTOR_PASS_HPP */
