/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
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
