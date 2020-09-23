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

#ifndef __PROMOTE_MEMINTRINSIC_PASS_HPP__
#define __PROMOTE_MEMINTRINSIC_PASS_HPP__

#include <llvm/Pass.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>

class PromoteMemIntrinsicPass : public llvm::ModulePass {

public:
	static char ID;

	PromoteMemIntrinsicPass() : llvm::ModulePass(ID), hasPromoted(false) {}

	virtual bool runOnModule(llvm::Module &M);

protected:
	/* Promoters for specific intrinsics */
	bool tryPromoteMemCpy(llvm::MemCpyInst *MI, llvm::Module &M);
	bool tryPromoteMemSet(llvm::MemSetInst *MS, llvm::Module &M);

	/* Called to remove promoted intrinsics from the code */
	void removePromoted();

	/* Intrinsics we need to promote to load/store pairs */
	llvm::SmallVector<llvm::MemIntrinsic *, 8> promoted;

	/* Whether we have promoted all intrinsics */
	bool hasPromoted;
};

#endif /* __PROMOTE_MEMINTRINSIC_PASS_HPP__ */
