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

#include "config.h"
#include "IntrinsicLoweringPass.hpp"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

bool IntrinsicLoweringPass::runOnBasicBlock(llvm::BasicBlock &BB, llvm::Module &M)
{
	bool modified = false;
	for (auto it = BB.begin(); it != BB.end();  ) {
		llvm::IntrinsicInst *I = llvm::dyn_cast<llvm::IntrinsicInst>(&*it);
		/* Iterator is incremented in order for it not to be invalidated */
		++it;
		/* If the instruction is not an intrinsic call, skip it */
		if (!I)
			continue;
		switch (I->getIntrinsicID()) {
		case llvm::Intrinsic::vastart:
		case llvm::Intrinsic::vaend:
		case llvm::Intrinsic::vacopy:
			break;
		case llvm::Intrinsic::dbg_value:
		case llvm::Intrinsic::dbg_declare:
			/* Remove useless calls to @llvm.debug.* */
			I->eraseFromParent();
			modified = true;
			break;
		case llvm::Intrinsic::trap: {
			/* Lower calls to @llvm.trap to abort() calls */
			auto FC = M.getOrInsertFunction("abort", llvm::Type::getVoidTy(M.getContext()));
#ifdef LLVM_GETORINSERTFUNCTION_RET_FUNCTION
			if (auto *F = llvm::dyn_cast<llvm::Function>(FC)) {
#else
			if (auto *F = llvm::dyn_cast<llvm::Function>(FC.getCallee())) {
#endif
				F->setDoesNotReturn();
				F->setDoesNotThrow();
				llvm::CallInst::Create(F, llvm::Twine(), I);
			}
			new llvm::UnreachableInst(M.getContext(), I);
			I->eraseFromParent();
			modified = true;
			break;
		}
		default:
			IL->LowerIntrinsicCall(I);
			modified = true;
			break;
		}
	}
	return modified;
}

bool IntrinsicLoweringPass::runOnModule(llvm::Module &M)
{
	bool modified = false;
	/* Scan through the instructions and lower intrinsic calls */
	for (auto &F : M)
		for (auto &BB : F)
			modified |= runOnBasicBlock(BB, M);

	/*
	 * Check for calls to @llvm.trap. Such calls may occur if LLVM
	 * detects a NULL pointer dereference in the CFG and simplify
	 * it to a trap call. In order for this to happen, the program
	 * has to be compiled with -O1 or -O2.
	 */
	if (llvm::Function *trapDecl = M.getFunction("llvm.trap")) {
		trapDecl->eraseFromParent();
		modified = true;
	}
	return modified;
}

llvm::ModulePass *createIntrinsicLoweringPass(llvm::Module &M)
{
#ifdef LLVM_EXECUTIONENGINE_DATALAYOUT_PTR
	return new IntrinsicLoweringPass(*M.getDataLayout());
#else
	return new IntrinsicLoweringPass(M.getDataLayout());
#endif
}

char IntrinsicLoweringPass::ID = 42;
