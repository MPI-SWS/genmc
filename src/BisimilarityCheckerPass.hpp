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

#ifndef __BISIMILARITY_CHECKER_PASS_HPP__
#define __BISIMILARITY_CHECKER_PASS_HPP__

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>

#include <unordered_map>

using namespace llvm;

class BisimilarityCheckerPass : public FunctionPass {

public:
	using BisimilarityPoint = std::pair<Instruction *, Instruction *>;

	static char ID;

	BisimilarityCheckerPass() : FunctionPass(ID) {}
	virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
	virtual bool doInitialization(Module &M) override;
	virtual bool runOnFunction(Function &F) override;

	/* Returns all bisimilar points in a given function */
	const std::vector<BisimilarityPoint> &getFuncBsPoints(Function *F)
	{
		return funcBsPoints[F];
	}

private:
	/* Bisimilar points for all functions of the module */
	std::unordered_map<Function *, std::vector<BisimilarityPoint>> funcBsPoints;
};

#endif /* __BISIMILARITY_CHECKER_PASS_HPP__ */
