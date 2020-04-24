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

#include <config.h>
#include "DeclareEndLoopPass.hpp"
#include "Error.hpp"
#include <llvm/Pass.h>
#include <llvm/PassSupport.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;

#ifdef LLVM_HAS_ATTRIBUTELIST
# define AttributeList AttributeList
#else
# define AttributeList AttributeSet
#endif

bool DeclareEndLoopPass::runOnModule(Module &M)
{
	Function *endLoopFun;
	FunctionType *endLoopTyp;
	AttributeList endLoopAtt;
	bool modified = false;

	endLoopFun = M.getFunction("__end_loop");
	if (!endLoopFun) {
		Type *retTyp = Type::getVoidTy(M.getContext());
		endLoopTyp = FunctionType::get(retTyp, {}, false);

		AttributeList::get(M.getContext(), AttributeList::FunctionIndex,
				  std::vector<Attribute::AttrKind>({Attribute::NoUnwind}));
		M.getOrInsertFunction("__end_loop", endLoopTyp, endLoopAtt);
		modified = true;
	}
	return modified;
}

char DeclareEndLoopPass::ID = 42;
static llvm::RegisterPass<DeclareEndLoopPass> P("declare-end-loop",
						"Declares the __end_loop function.");
