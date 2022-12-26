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

#ifndef __INTRINSIC_LOWERING_PASS_HPP__
#define __INTRINSIC_LOWERING_PASS_HPP__

#include <llvm/Analysis/LoopPass.h>
#include <llvm/CodeGen/IntrinsicLowering.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Pass.h>

class IntrinsicLoweringPass : public llvm::ModulePass {

public:
	static char ID;
	const llvm::DataLayout &dataLayout;
	llvm::IntrinsicLowering *IL;

	IntrinsicLoweringPass(const llvm::DataLayout &TD)
		: llvm::ModulePass(ID), dataLayout(TD),
		  IL(new llvm::IntrinsicLowering(TD)) {}
	~IntrinsicLoweringPass() { delete IL; }
	virtual bool runOnModule(llvm::Module &M);

protected:
	bool runOnBasicBlock(llvm::BasicBlock &BB, llvm::Module &M);
};

#endif /* __INTRINSIC_LOWERING_PASS_HPP__ */
