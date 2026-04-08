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

#ifndef GENMC_CALL_INFO_COLLECTION_PASS_HPP
#define GENMC_CALL_INFO_COLLECTION_PASS_HPP

#include "genmc/ADT/VSet.hpp"
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
