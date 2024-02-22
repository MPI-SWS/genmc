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

#include "PropagateAssumesPass.hpp"
#include "Error.hpp"
#include "InterpreterEnumAPI.hpp"
#include "LLVMUtils.hpp"
#include "config.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <numeric>

using namespace llvm;

void PropagateAssumesPass::getAnalysisUsage(AnalysisUsage &au) const { au.setPreservesAll(); }

bool isAssumeFalse(Instruction *i)
{
	auto *ci = dyn_cast<CallInst>(i);
	if (!ci || !isAssumeFunction(getCalledFunOrStripValName(*ci)))
		return false;

	auto *arg = dyn_cast<ConstantInt>(ci->getOperand(0));
	return arg && arg->isZero();
}

bool jumpsOnLoadResult(Value *cond)
{
	auto *sc = stripCastsConstOps(cond);
	if (isa<LoadInst>(sc))
		return true;

	if (auto *ci = dyn_cast<CmpInst>(sc)) {
		auto *op1 = dyn_cast<LoadInst>(stripCastsConstOps(ci->getOperand(0)));
		auto *op2 = dyn_cast<LoadInst>(stripCastsConstOps(ci->getOperand(1)));
		return op1 && op2;
	}
	return false;
}

Value *getOtherSuccCondition(BranchInst *bi, BasicBlock *succ)
{
	if (bi->getSuccessor(1) == succ)
		return bi->getCondition();
	return BinaryOperator::CreateNot(bi->getCondition(), "", bi);
}

bool propagateAssumeToPred(CallInst *assume, BasicBlock *pred)
{
	auto *bi = dyn_cast<BranchInst>(pred->getTerminator());
	if (!bi || bi->isUnconditional())
		return false;

	auto *bb = assume->getParent();
	auto assumeName = getCalledFunOrStripValName(*assume);
	auto *endFun = bb->getParent()->getParent()->getFunction(assumeName);
	BUG_ON(!endFun);

	/* Get the condition that we need to ensure; if there's a type
	 * mismatch, cast to the exposed type too */
	auto *cond = getOtherSuccCondition(bi, bb);
	auto *M = endFun->getParent();
	auto &DL = M->getDataLayout();
	auto *arg = &*endFun->arg_begin();
	if (DL.getTypeAllocSize(cond->getType()) != DL.getTypeAllocSize(arg->getType())) {
		BUG_ON(DL.getTypeAllocSize(cond->getType()) > DL.getTypeAllocSize(arg->getType()));
		auto bits = DL.getTypeAllocSizeInBits(arg->getType());
		auto *intNTy = Type::getIntNTy(M->getContext(), bits);
		cond = CastInst::CreateZExtOrBitCast(cond, intNTy, "", bi);
	}

	/* Ensure the condition */
	auto *ci = CallInst::Create(endFun, {cond}, "", bi);
	ci->setMetadata("dbg", bi->getMetadata("dbg"));
	return true;
}

bool propagateAssume(CallInst *assume)
{
	auto *bb = assume->getParent();
	return std::accumulate(pred_begin(bb), pred_end(bb), false,
			       [&](const bool &accum, BasicBlock *p) {
				       return accum || propagateAssumeToPred(assume, p);
			       });
}

bool PropagateAssumesPass::runOnFunction(llvm::Function &F)
{
	auto modified = false;

	for (auto &bb : F)
		if (isAssumeFalse(&*bb.begin()))
			modified |= propagateAssume(dyn_cast<CallInst>(&*bb.begin()));
	return modified;
}

Pass *createPropagateAssumesPass()
{
	auto *p = new PropagateAssumesPass();
	return p;
}

char PropagateAssumesPass::ID = 42;
static llvm::RegisterPass<PropagateAssumesPass> P("propagate-assumes",
						  "Propagates assume(0) upwards.");
