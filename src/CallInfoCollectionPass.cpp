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

#include "CallInfoCollectionPass.hpp"
#include "Error.hpp"
#include "InterpreterEnumAPI.hpp"
#include "LLVMUtils.hpp"
#include "config.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace llvm;

void CallInfoCollectionPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	au.setPreservesAll();
}

bool hasSideEffects(Function *F, SmallVector<Function *, 4> &chain, VSet<Function *> &clean,
		    VSet<Function *> &dirty)
{
	if (!F || dirty.count(F))
		return true;
	if (clean.count(F) || std::find(chain.begin(), chain.end(), F) != chain.end())
		return false;
	if (F->empty())
		return !isCleanInternalFunction(F->getName().str());

	for (auto it = inst_begin(*F), ei = inst_end(*F); it != ei; ++it) {
		if (!hasSideEffects(&*it))
			continue;

		/* Found an instruction w/ side effects; check type */
		if (!isa<CallInst>(&*it))
			return true;

		/* If it is a CallInst, recurse */
		if (auto *ci = dyn_cast<CallInst>(&*it)) {
			chain.push_back(F);
			if (hasSideEffects(ci->getCalledFunction(), chain, clean, dirty))
				return true;
			chain.pop_back();
		}
	}

	if (chain.empty())
		clean.insert(F);
	return false;
}

bool hasSideEffects(Function *F, VSet<Function *> &clean, VSet<Function *> &dirty)
{
	SmallVector<Function *, 4> chain;
	return hasSideEffects(F, chain, clean, dirty);
}

bool isAllocating(Function *F)
{
	if (!F->getReturnType()->isPointerTy())
		return false;

	SmallVector<ReturnInst *, 4> rets;
	std::for_each(inst_begin(*F), inst_end(*F),
		      [&](Instruction &i) { /* transform-if */
					    if (auto *ri = dyn_cast<ReturnInst>(&i))
						    rets.push_back(ri);
		      });

	return std::all_of(rets.begin(), rets.end(), [](const ReturnInst *ri) {
		auto *rv = ri->getReturnValue();
		return isa<Instruction>(rv) && isAlloc(dyn_cast<Instruction>(rv));
	});
}

bool CallInfoCollectionPass::runOnModule(Module &M)
{
	VSet<Function *> dirty;

	clean.clear();
	alloc.clear();
	for (auto &F : M) {
		if (hasSideEffects(&F, clean, dirty))
			dirty.insert(&F);
		else
			clean.insert(&F);
		if (isAllocating(&F))
			alloc.insert(&F);
	}
	return false;
}

ModulePass *createCallInfoCollectionPass() { return new CallInfoCollectionPass(); }

char CallInfoCollectionPass::ID = 42;
static llvm::RegisterPass<CallInfoCollectionPass>
	P("call-info-collection", "Collects information about side-effect-free functions.");
