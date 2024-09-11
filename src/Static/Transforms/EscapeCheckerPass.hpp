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

#ifndef GENMC_ESCAPE_CHECKER_PASS_HPP
#define GENMC_ESCAPE_CHECKER_PASS_HPP

#include "ADT/VSet.hpp"
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
	auto writesDynamicMemory(Value *val /*, AliasAnalysis &AA */) -> Instruction *;

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
