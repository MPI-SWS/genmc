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

#ifndef GENMC_BISIMILARITY_CHECKER_PASS_HPP
#define GENMC_BISIMILARITY_CHECKER_PASS_HPP

#include <llvm/IR/Function.h>
#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class BisimilarityAnalysis : public AnalysisInfoMixin<BisimilarityAnalysis> {
public:
	using BisimilarityPoint = std::pair<Instruction *, Instruction *>;
	using Result = std::vector<BisimilarityPoint>;

	auto run(Function &F, FunctionAnalysisManager &FAM) -> Result;

	/* Returns all bisimilar points in a given function */
	auto getFuncBsPoints(Function &F) -> Result { return funcBsPoints_; }

private:
	friend AnalysisInfoMixin<BisimilarityAnalysis>;
	static inline AnalysisKey Key;

	/* Bisimilar points for a function */
	std::vector<BisimilarityPoint> funcBsPoints_;
};

class BisimilarityCheckerPass : public AnalysisInfoMixin<BisimilarityCheckerPass> {
public:
	BisimilarityCheckerPass(std::vector<BisimilarityAnalysis::BisimilarityPoint> &bsps)
		: bsps_(bsps)
	{}

	auto run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses;

private:
	std::vector<BisimilarityAnalysis::BisimilarityPoint> &bsps_;
};

#endif /* GENMC_BISIMILARITY_CHECKER_PASS_HPP */
