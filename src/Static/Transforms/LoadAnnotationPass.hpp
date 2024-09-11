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

#ifndef GENMC_LOAD_ANNOTATION_PASS_HPP
#define GENMC_LOAD_ANNOTATION_PASS_HPP

#include "Static/ModuleInfo.hpp"
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
