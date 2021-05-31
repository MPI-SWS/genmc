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

#include "DefineLibcFunsPass.hpp"
#include "Error.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

void DefineLibcFunsPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const
{
}

void DefineLibcFunsPass::replaceFunWithNop(llvm::Module &M, std::string name)
{
	llvm::Function *F = M.getFunction(name);
	if (!F || !F->isDeclaration())
		return;

	llvm::Value *res = 0;
	llvm::Type *retTy = F->getReturnType();
	if (retTy->isIntegerTy())
		res = llvm::ConstantInt::get(retTy, 0);
	else if (retTy->isVoidTy())
		res = 0;
	else if (retTy->isPointerTy())
		res = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType *>(retTy));
	else
		WARN("Could not add definition for " + name + "!\n");

	llvm::BasicBlock *BB = llvm::BasicBlock::Create(F->getContext(), "", F);
	llvm::ReturnInst::Create(F->getContext(), res, BB);
	return;
}

bool DefineLibcFunsPass::runOnModule(llvm::Module &M)
{
	replaceFunWithNop(M, "fclose");
	replaceFunWithNop(M, "fopen");
	replaceFunWithNop(M, "fflush");
	replaceFunWithNop(M, "fprintf");
	return true;
}

llvm::ModulePass *createDefineLibcFunsPass()
{
	return new DefineLibcFunsPass();
}

char DefineLibcFunsPass::ID = 42;
static llvm::RegisterPass<DefineLibcFunsPass> X("define-libc",
						"Define some standard libc functions.");
