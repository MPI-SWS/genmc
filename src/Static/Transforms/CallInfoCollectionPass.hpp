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

#ifndef GENMC_CALL_INFO_COLLECTION_PASS_HPP
#define GENMC_CALL_INFO_COLLECTION_PASS_HPP

#include "ADT/VSet.hpp"
#include <llvm/Passes/PassBuilder.h>

namespace llvm {
class Function;
} // namespace llvm

using namespace llvm;

class CallAnalysis;

struct CallAnalysisResult {
	using CallSet = VSet<Function *>;

	CallSet clean;
	CallSet alloc;
};

class CallAnalysis : public AnalysisInfoMixin<CallAnalysis> {
public:
	using Result = CallAnalysisResult;

	auto run(Module &M, ModuleAnalysisManager &MAM) -> Result;

private:
	friend AnalysisInfoMixin<CallAnalysis>;
	static inline AnalysisKey Key;

	CallAnalysisResult result_;
};

class CallAnalysisPass : public AnalysisInfoMixin<CallAnalysisPass> {
public:
	CallAnalysisPass(CallAnalysisResult &AR) : AR_(AR) {}

	auto run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses;

private:
	CallAnalysisResult &AR_;
};

#endif /* GENMC_CALL_INFO_COLLECTION_PASS_HPP */
