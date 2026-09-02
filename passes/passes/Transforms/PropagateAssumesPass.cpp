/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#include "PropagateAssumesPass.hpp"
#include "genmc/Support/Error.hpp"
#include "passes/InternalFunctions.hpp"
#include "passes/LLVMUtils.hpp"

#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/Casting.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <numeric>

using namespace llvm;

static auto isAssumeFalse(Instruction *i) -> bool
{
	auto *ci = dyn_cast<CallInst>(i);
	if (!ci || !isAssumeFunction(getCalledFunOrStripValName(*ci)))
		return false;

	auto *arg = dyn_cast<ConstantInt>(ci->getOperand(0));
	return arg && arg->isZero();
}

[[maybe_unused]] static auto jumpsOnLoadResult(Value *cond) -> bool
{
	auto *stripped = stripCastsConstOps(cond);
	if (isa<LoadInst>(stripped))
		return true;

	if (auto *ci = dyn_cast<CmpInst>(stripped)) {
		auto *op1 = dyn_cast<LoadInst>(stripCastsConstOps(ci->getOperand(0)));
		auto *op2 = dyn_cast<LoadInst>(stripCastsConstOps(ci->getOperand(1)));
		return op1 && op2;
	}
	return false;
}

static auto getOtherSuccCondition(BranchInst *bi, BasicBlock *succ) -> Value *
{
	if (bi->getSuccessor(1) == succ)
		return bi->getCondition();
	return BinaryOperator::CreateNot(bi->getCondition(), "", bi->getIterator());
}

static auto propagateAssumeToPred(CallInst *assume, BasicBlock *pred) -> bool
{
	auto *bi = dyn_cast<BranchInst>(pred->getTerminator());
	if (!bi || bi->isUnconditional())
		return false;

	auto *bb = assume->getParent();
	auto assumeName = getCalledFunOrStripValName(*assume);
	auto *endFun = bb->getParent()->getParent()->getFunction(assumeName);
	VERIFY(endFun);

	/* Get the condition that we need to ensure; if there's a type
	 * mismatch, cast to the exposed type too */
	auto *cond = getOtherSuccCondition(bi, bb);
	auto *M = endFun->getParent();
	const auto &DL = M->getDataLayout();
	auto *arg = &*endFun->arg_begin();
	if (DL.getTypeAllocSize(cond->getType()) != DL.getTypeAllocSize(arg->getType())) {
		VERIFY(DL.getTypeAllocSize(cond->getType()) <= DL.getTypeAllocSize(arg->getType()));
		auto bits = DL.getTypeAllocSizeInBits(arg->getType());
		auto *intNTy = Type::getIntNTy(M->getContext(), bits);
		cond = CastInst::CreateZExtOrBitCast(cond, intNTy, "", bi->getIterator());
	}

	/* Ensure the condition */
	auto *ci =
		CallInst::Create(endFun, {cond, assume->getArgOperand(1)}, "", bi->getIterator());
	ci->setMetadata("dbg", bi->getMetadata("dbg"));
	return true;
}

static auto propagateAssume(CallInst *assume) -> bool
{
	auto *bb = assume->getParent();
	return std::accumulate(pred_begin(bb), pred_end(bb), false,
			       [&](const bool &accum, BasicBlock *pred) {
				       return accum || propagateAssumeToPred(assume, pred);
			       });
}

auto PropagateAssumesPass::run(Function &F, FunctionAnalysisManager & /*FAM*/) -> PreservedAnalyses
{
	auto modified = false;

	for (auto &bb : F)
		if (isAssumeFalse(&*bb.begin()))
			modified |= propagateAssume(cast<CallInst>(&*bb.begin()));
	return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
