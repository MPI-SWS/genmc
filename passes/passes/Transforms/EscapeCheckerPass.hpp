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

#ifndef GENMC_ESCAPE_CHECKER_PASS_HPP
#define GENMC_ESCAPE_CHECKER_PASS_HPP

#include "genmc/ADT/VSet.hpp"
#include "CallInfoCollectionPass.hpp"

#include <llvm/Passes/PassBuilder.h>

#include <ranges>
#include <unordered_map>
#include <vector>

using namespace llvm;

class EscapeAnalysis;

/*
 * This class is responsible for identifying allocations that escape
 * their enclosing functions through writes to "global" memory.
 * (For the purposes of this pass, global memory is mem2reg could not
 * deem local. We may have to make it smarter in the future. )
 */
class EscapeAnalysisResult {

public:
	EscapeAnalysisResult(bool loadsEscape = true) : loadsEscape_(loadsEscape) {}

	/* Whether we consider loads as escape points */
	auto canLoadsEscape() const -> bool { return loadsEscape_; }

	/* (Re)-calculates the esacape points for each instruction of F */
	void calculate(Function &F, const CallAnalysisResult &CAR);

	/* Returns true if V escapes in F */
	auto escapes(const Value *v) const -> bool;

	/* Returns true if all escape points of A are dominated by B.
	 * If there are no escape points, returns true. */
	auto escapesAfter(const Value *a, const Instruction *b, DominatorTree &DT) const -> bool;

	/* If VAL represents local memory, returns the respective allocating instructions */
	auto writesDynamicMemory(Value *val /*, AliasAnalysis &AA */) const -> Instruction *;

	auto alloc_begin() const -> VSet<Instruction *>::const_iterator { return allocs_.begin(); }
	auto alloc_end() const -> VSet<Instruction *>::const_iterator { return allocs_.end(); }

	/* For debugging */
	void print(raw_ostream &s) const;

private:
	using EPT = std::unordered_map<const Value *, std::vector<const Instruction *>>;

	bool loadsEscape_ = true;
	EPT escapePoints;
	VSet<Instruction *> allocs_;
};

/*
 * Populates an EscapeInfo object for the function the pass runs on.
 */
class EscapeAnalysis : public AnalysisInfoMixin<EscapeAnalysis> {
public:
	using Result = std::unordered_map<Function *, EscapeAnalysisResult>;

	auto run(Module &M, ModuleAnalysisManager &MAM) -> Result;

private:
	friend AnalysisInfoMixin<EscapeAnalysis>;
	static inline AnalysisKey Key;

	Result result_;
};

class EscapeCheckerPass : public AnalysisInfoMixin<EscapeCheckerPass> {
public:
	EscapeCheckerPass(EscapeAnalysis::Result &EAR) : EAR_(EAR) {}

	auto run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses;

private:
	EscapeAnalysis::Result &EAR_;
};

#endif /* GENMC_ESCAPE_CHECKER_PASS_HPP_ */
