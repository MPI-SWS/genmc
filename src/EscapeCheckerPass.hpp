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

#ifndef __ESCAPE_CHECKER_PASS_HPP__
#define __ESCAPE_CHECKER_PASS_HPP__

#include "VSet.hpp"
#include "llvm/Analysis/AliasAnalysis.h"
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

#include <string>
#include <unordered_map>
#include <vector>


/*
 * This class is responsible for identifying allocations that escape
 * their enclosing functions through writes to "global" memory.
 * (For the purposes of this pass, global memory is mem2reg could not
 * deem local. We may have to make it smarter in the future. )
 */
class EscapeInfo {

public:
	EscapeInfo() = default;

	/* (Re)-calculates the esacape points for each instruction of F */
	void calculate(llvm::Function &F, const VSet<llvm::Function *> &allocFuns,
		       llvm::AliasAnalysis &AA);

	/* Returns true if V escapes in F */
	bool escapes(const llvm::Value *v) const;

	/* Returns true if all escape points of A are dominated by B.
	 * If there are no escape points, returns true. */
	bool escapesAfter(const llvm::Value *a, const llvm::Instruction *b,
			  llvm::DominatorTree &DT) const;

	/* If VAL represents local memory, returns the respective allocating instructions */
	llvm::Instruction *writesDynamicMemory(llvm::Value *val /*, llvm::AliasAnalysis &AA */);

	VSet<llvm::Instruction *>::const_iterator alloc_begin() const { return allocs.begin(); }
	VSet<llvm::Instruction *>::const_iterator alloc_end() const { return allocs.end(); }

	/* For debugging */
	void print(llvm::raw_ostream &s) const;

private:
	using EPT = std::unordered_map<const llvm::Value *, std::vector<const llvm::Instruction *>>;

	EPT escapePoints;
	VSet<llvm::Instruction *> allocs;
};


/*
 * Populates an EscapeInfo object for the function the pass runs on.
 */
class EscapeCheckerPass : public llvm::FunctionPass {

public:
	EscapeCheckerPass() : llvm::FunctionPass(ID) {}

	bool runOnFunction(llvm::Function &F) override;
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

	EscapeInfo &getEscapeInfo() { return EI; }
	const EscapeInfo &getEscapeInfo() const { return EI; }

	static char ID;

private:
	EscapeInfo EI;
};

#endif /* __ESCAPE_CHECKER_PASS_HPP__ */
