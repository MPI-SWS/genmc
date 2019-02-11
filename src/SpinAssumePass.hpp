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

#include <llvm/Pass.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/IR/Instructions.h>

class SpinAssumePass : public llvm::LoopPass {

protected:
	bool isAssumeStatement(llvm::Instruction &i) const;
	bool isSpinLoop(const llvm::Loop *l) const;
	void addAssumeCallToBlock(llvm::BasicBlock *eb, llvm::BasicBlock *nb,
				  llvm::BranchInst *bi, bool exitOn);
	void removeDisconnectedBlocks(llvm::Loop *l);
	bool transformLoop(llvm::Loop *l, llvm::LPPassManager &lpm);
	
public:
	static char ID;
	
	SpinAssumePass() : llvm::LoopPass(ID) {};
	virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
	virtual bool runOnLoop(llvm::Loop *L, llvm::LPPassManager &LPM);
	
};

#endif /* __SPIN_ASSUME_PASS_HPP__ */
