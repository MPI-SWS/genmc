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

#include "DeclareInternalsPass.hpp"
#include "Support/Error.hpp"

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

auto declareInternal(Module &M, const std::string &name, Type *retTyp,
		     const ArrayRef<Type *> &argTyps) -> bool
{
	auto *fun = M.getFunction(name);
	if (fun)
		return false;

	auto *funTyp = FunctionType::get(retTyp, argTyps, false);
	auto funAttrs = AttributeList::get(M.getContext(), AttributeList::FunctionIndex,
					   std::vector<Attribute::AttrKind>({Attribute::NoUnwind}));
	M.getOrInsertFunction(name, funTyp, funAttrs);
	return true;
}

PreservedAnalyses DeclareInternalsPass::run(Module &M, ModuleAnalysisManager &AM)
{
	bool modified = false;

	modified |= declareInternal(M, "__VERIFIER_assume", Type::getVoidTy(M.getContext()),
				    {Type::getInt1Ty(M.getContext())});
	modified |= declareInternal(M, "__VERIFIER_kill_thread", Type::getVoidTy(M.getContext()),
				    {Type::getInt1Ty(M.getContext())});
	modified |= declareInternal(M, "__VERIFIER_opt_begin", Type::getInt1Ty(M.getContext()), {});
	modified |=
		declareInternal(M, "__VERIFIER_loop_begin", Type::getVoidTy(M.getContext()), {});
	modified |=
		declareInternal(M, "__VERIFIER_spin_start", Type::getVoidTy(M.getContext()), {});
	modified |= declareInternal(M, "__VERIFIER_spin_end", Type::getVoidTy(M.getContext()),
				    {Type::getInt1Ty(M.getContext())});
	modified |= declareInternal(M, "__VERIFIER_faiZNE_spin_end",
				    Type::getVoidTy(M.getContext()), {});
	modified |= declareInternal(M, "__VERIFIER_lockZNE_spin_end",
				    Type::getVoidTy(M.getContext()), {});
	return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
auto getDeclareInternalsPluginInfo() -> PassPluginLibraryInfo
{
	return {LLVM_PLUGIN_API_VERSION, "DeclareInternals", LLVM_VERSION_STRING,
		[](PassBuilder &PB) {
			PB.registerPipelineParsingCallback(
				[](StringRef Name, ModulePassManager &MPM,
				   ArrayRef<PassBuilder::PipelineElement>) {
					if (Name == "declare-internals") {
						MPM.addPass(DeclareInternalsPass());
						return true;
					}
					return false;
				});
		}};
}
