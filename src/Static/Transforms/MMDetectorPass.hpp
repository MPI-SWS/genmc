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
