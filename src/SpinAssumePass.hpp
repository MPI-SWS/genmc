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

#ifndef __SPIN_ASSUME_PASS_HPP__
#define __SPIN_ASSUME_PASS_HPP__

#include "VSet.hpp"
#include <llvm/Analysis/LoopPass.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Pass.h>

class SpinAssumePass : public llvm::LoopPass {

protected:
	bool isPathToHeaderEffectFree(llvm::BasicBlock *latch, llvm::Loop *l,
				      bool &checkDynamically);
	bool isPathToHeaderFAIZNE(llvm::BasicBlock *latch, llvm::Loop *l,
				  llvm::Instruction *&lastEffect);
	bool isPathToHeaderLockZNE(llvm::BasicBlock *latch, llvm::Loop *l,
				   llvm::Instruction *&lastEffect);

public:
	static char ID;

	SpinAssumePass() : llvm::LoopPass(ID){};

	void markSpinloopStarts(bool mark) { markStarts = mark; }

	virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
	virtual bool runOnLoop(llvm::Loop *L, llvm::LPPassManager &LPM);

private:
	/* Whether we should mark spinloop starts */
	bool markStarts = false;
};

#endif /* __SPIN_ASSUME_PASS_HPP__ */
