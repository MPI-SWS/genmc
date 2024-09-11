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
#include "Support/Error.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Passes/PassPlugin.h>

using namespace llvm;

void replaceFunWithNop(Module &M, std::string name)
{
	auto *F = M.getFunction(name);
	if (!F || !F->isDeclaration())
		return;

	Value *res = nullptr;
	Type *retTy = F->getReturnType();
	if (retTy->isIntegerTy())
		res = ConstantInt::get(retTy, 0);
	else if (retTy->isVoidTy())
		res = nullptr;
	else if (retTy->isPointerTy())
		res = ConstantPointerNull::get(dyn_cast<PointerType>(retTy));
	else
		WARN("Could not add definition for " + name + "!\n");

	auto *BB = BasicBlock::Create(F->getContext(), "", F);
	ReturnInst::Create(F->getContext(), res, BB);
}

auto DefineLibcFunsPass::run(Module &M, ModuleAnalysisManager &AM) -> PreservedAnalyses
{
	replaceFunWithNop(M, "fclose");
	replaceFunWithNop(M, "fopen");
	replaceFunWithNop(M, "fflush");
	replaceFunWithNop(M, "fprintf");
	return PreservedAnalyses::all();
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
auto getDefineLibcFunsPluginInfo() -> PassPluginLibraryInfo
{
	return {LLVM_PLUGIN_API_VERSION, "DefineLibcFuns", LLVM_VERSION_STRING,
		[](PassBuilder &PB) {
			PB.registerPipelineParsingCallback(
				[](StringRef Name, ModulePassManager &MPM,
				   ArrayRef<PassBuilder::PipelineElement>) {
					if (Name == "define-libc-funs") {
						MPM.addPass(DefineLibcFunsPass());
						return true;
					}
					return false;
				});
		}};
}
