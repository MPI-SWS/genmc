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

#ifndef GENMC_LOAD_ANNOTATION_PASS_HPP
#define GENMC_LOAD_ANNOTATION_PASS_HPP

#include "passes/ModuleInfo.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

using LoadAnnotationAnalysisResult = AnnotationInfo<Instruction *, Value *>;

class LoadAnnotationAnalysis : public AnalysisInfoMixin<LoadAnnotationAnalysis> {
public:
	using Result = LoadAnnotationAnalysisResult;

	auto run(Function &F, FunctionAnalysisManager &FAM) -> Result;

private:
	friend AnalysisInfoMixin<LoadAnnotationAnalysis>;
	static inline AnalysisKey Key;

	Result result_;
};

class LoadAnnotationPass : public AnalysisInfoMixin<LoadAnnotationPass> {
public:
	LoadAnnotationPass(LoadAnnotationAnalysisResult &LAI) : LAI_(LAI) {}

	auto run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses;

private:
	LoadAnnotationAnalysisResult &LAI_;
};

#endif /* GENMC_LOAD_ANNOTATION_PASS_HPP_ */
