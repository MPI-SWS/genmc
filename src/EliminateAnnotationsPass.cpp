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

#include "EliminateAnnotationsPass.hpp"
#include "Error.hpp"
#include "InterpreterEnumAPI.hpp"
#include "LLVMUtils.hpp"
#include "config.h"

#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm;

#define POSTDOM_PASS PostDominatorTreeWrapperPass
#define GET_POSTDOM_PASS() getAnalysis<POSTDOM_PASS>().getPostDomTree();

void EliminateAnnotationsPass::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.addRequired<DominatorTreeWrapperPass>();
	AU.addRequired<POSTDOM_PASS>();
	AU.setPreservesAll();
}

bool isAnnotationBegin(Instruction *i)
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::FN_AnnotateBegin;
}

bool isAnnotationEnd(Instruction *i)
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::FN_AnnotateEnd;
}

uint64_t getAnnotationValue(CallInst *ci)
{
	auto *funArg = llvm::dyn_cast<ConstantInt>(ci->getOperand(0));
	BUG_ON(!funArg);
	return funArg->getValue().getLimitedValue();
}

bool annotateInstructions(CallInst *begin, CallInst *end)
{
	if (!begin || !end)
		return false;

	auto annotType = getAnnotationValue(begin);
	auto beginFound = false;
	auto endFound = false;
	unsigned opcode = 0; /* no opcode == 0 in LLVM */
	foreachInBackPathTo(end->getParent(), begin->getParent(), [&](Instruction &i) {
		/* wait until we find the end (e.g., if in same block) */
		if (!endFound) {
			endFound |= (dyn_cast<CallInst>(&i) == end);
			return;
		}
		/* check until we find the begin; only deal with atomic insts */
		if (endFound && !beginFound && i.isAtomic() && !isa<FenceInst>(i)) {
			if (!opcode)
				opcode = i.getOpcode();
			BUG_ON(opcode != i.getOpcode()); /* annotations across paths must match */
			annotateInstruction(&i, "genmc.attr", annotType);
		}
		/* stop when the begin is found; reset vars for next path */
		if (!beginFound) {
			beginFound |= (dyn_cast<CallInst>(&i) == begin);
			if (beginFound)
				beginFound = endFound = false;
		}
	});
	return true;
}

CallInst *findMatchingEnd(CallInst *begin, const std::vector<CallInst *> &ends, DominatorTree &DT,
			  PostDominatorTree &PDT)
{
	auto it = std::find_if(ends.begin(), ends.end(), [&](auto *ei) {
		return getAnnotationValue(begin) == getAnnotationValue(ei) &&
		       DT.dominates(begin, ei) &&
		       PDT.dominates(ei->getParent(), begin->getParent()) &&
		       std::none_of(ends.begin(), ends.end(), [&](auto *ei2) {
			       return ei != ei2 && DT.dominates(begin, ei2) &&
				      PDT.dominates(ei2->getParent(), begin->getParent()) &&
				      DT.dominates(ei2, ei);
		       });
	});
	BUG_ON(it == ends.end());
	return *it;
}

bool EliminateAnnotationsPass::runOnFunction(Function &F)
{
	std::vector<CallInst *> begins, ends;

	std::for_each(inst_begin(F), inst_end(F), [&](auto &i) {
		if (isAnnotationBegin(&i))
			begins.push_back(dyn_cast<CallInst>(&i));
		else if (isAnnotationEnd(&i))
			ends.push_back(dyn_cast<CallInst>(&i));
	});

	auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
	auto &PDT = GET_POSTDOM_PASS();
	VSet<Instruction *> toDelete;

	auto changed = false;
	std::for_each(begins.begin(), begins.end(), [&](auto *bi) {
		auto *ei = findMatchingEnd(bi, ends, DT, PDT);
		BUG_ON(!ei);
		changed |= annotateInstructions(bi, ei);
		toDelete.insert(bi);
		toDelete.insert(ei);
	});

	for (auto *i : toDelete) {
		i->eraseFromParent();
		changed = true;
	}
	return changed;
}

Pass *createEliminateAnnotationsPass()
{
	auto *p = new EliminateAnnotationsPass();
	return p;
}

char EliminateAnnotationsPass::ID = 42;
static llvm::RegisterPass<EliminateAnnotationsPass> P("elim-annots",
						      "Eliminates intrinsic annotations.");
