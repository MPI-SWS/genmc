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

#include "IntrinsicLoweringPass.hpp"
#include "genmc/Support/Error.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Analysis/MemoryBuiltins.h>
#include <llvm/CodeGen/IntrinsicLowering.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Casting.h>

#include <memory>

using namespace llvm;

#define WITH_OVERFLOW_INST(I, EXT_OP, OP_BIN)                                                      \
	{                                                                                          \
		auto *OI = dyn_cast<WithOverflowInst>(I);                                          \
		IRBuilder<> builder(OI);                                                           \
                                                                                                   \
		auto *opA = OI->getArgOperand(0);                                                  \
		auto *opB = OI->getArgOperand(1);                                                  \
                                                                                                   \
		/* Get the type with twice the bitlength from our operands (i32 -> i64 etc.) */    \
		auto bitwidth = opA->getType()->getPrimitiveSizeInBits();                          \
		auto *typeExt = IntegerType::get(M.getContext(), bitwidth * 2);                    \
                                                                                                   \
		/* Extend the operands type to fit a possible overflow */                          \
		auto *aExt = builder.EXT_OP(opA, typeExt);                                         \
		auto *bExt = builder.EXT_OP(opB, typeExt);                                         \
                                                                                                   \
		/* Perform the operation to get the result */                                      \
		auto *result = builder.OP_BIN(opA, opB);                                           \
                                                                                                   \
		/* Perform same with extended bitwidth operands to check for overflows */          \
		auto *resultFromExtOps = builder.OP_BIN(aExt, bExt);                               \
		auto *resultExt = builder.EXT_OP(result, typeExt);                                 \
		auto *overflow = builder.CreateICmpNE(resultFromExtOps, resultExt);                \
                                                                                                   \
		/* Skip building the result-struct if it's only used by extractvals */             \
		if (replaceExtractValUsers(OI, result, overflow, toRemove)) {                      \
			/* Result struct is used somehow besides just extracting values */         \
			auto *resStruct = builder.CreateInsertValue(                               \
				UndefValue::get(OI->getType()), result, 0);                        \
			resStruct = builder.CreateInsertValue(resStruct, overflow, 1);             \
			OI->replaceAllUsesWith(resStruct);                                         \
		}                                                                                  \
		OI->eraseFromParent();                                                             \
                                                                                                   \
		modified = true;                                                                   \
		break;                                                                             \
	}

/* Given an overflowing instruction OI, replaces its extractval users with REPLACE or OVERFLOW.
 * Returns whether OI has any non-extractval users */
static auto replaceExtractValUsers(WithOverflowInst *oi, Value *replace, Value *overflow,
				   SmallVector<Instruction *, 8> &toRemove) -> bool
{
	auto result = false;
	for (auto *usr : oi->users()) {
		auto *evi = dyn_cast<ExtractValueInst>(usr);
		if (!evi) {
			result = true;
			continue;
		}

		auto indices = evi->getIndices();
		VERIFY(indices[0] == 0 || indices[0] == 1);
		if (indices[0] == 0)
			evi->replaceAllUsesWith(replace);
		if (indices[0] == 1)
			evi->replaceAllUsesWith(overflow);
		toRemove.push_back(evi);
	}
	return result;
}

static auto runOnBasicBlock(BasicBlock &bb, IntrinsicLowering *IL) -> bool
{
	auto &M = *bb.getParent()->getParent();
	auto modified = false;
	SmallVector<Instruction *, 8> toRemove;

	for (auto it = bb.begin(); it != bb.end();) {
		auto *I = llvm::dyn_cast<IntrinsicInst>(&*it);
		/* Iterator is incremented in order for it not to be invalidated */
		++it;
		/* If the instruction is not an intrinsic call, skip it */
		if (!I)
			continue;
		switch (I->getIntrinsicID()) {
		/* In case thread-local variables are not accessed directly, make them */
		case llvm::Intrinsic::threadlocal_address:
			I->replaceAllUsesWith(I->getOperand(0));
			break;
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
			/*
			 * Check for calls to @llvm.trap. Such calls may occur if LLVM
			 * detects a NULL pointer dereference in the CFG and simplify
			 * it to a trap call. In order for this to happen, the program
			 * has to be compiled with -O1 or -O2.
			 * We lower such calls to abort() (@trap could also be erased from M)
			 */
			auto FC = M.getOrInsertFunction("abort",
							llvm::Type::getVoidTy(M.getContext()));
			if (auto *F = llvm::dyn_cast<llvm::Function>(FC.getCallee())) {
				F->setDoesNotReturn();
				F->setDoesNotThrow();
				llvm::CallInst::Create(F, llvm::Twine(), I->getIterator());
			}
			I->eraseFromParent();
			modified = true;
			break;
		}
		case llvm::Intrinsic::objectsize: {
			auto *objSize = llvm::lowerObjectSizeCall(I,
								  I->getModule()->getDataLayout(),
								  nullptr, /* must succeed */ true);
			I->replaceAllUsesWith(objSize);
			I->eraseFromParent();
			modified = true;
			break;
		}
		// https://llvm.org/docs/LangRef.html#llvm-sadd-with-overflow-intrinsics
		case llvm::Intrinsic::sadd_with_overflow:
			WITH_OVERFLOW_INST(I, CreateSExt, CreateAdd)
		case llvm::Intrinsic::ssub_with_overflow:
			WITH_OVERFLOW_INST(I, CreateSExt, CreateSub)
		case llvm::Intrinsic::smul_with_overflow:
			WITH_OVERFLOW_INST(I, CreateSExt, CreateMul)
		case llvm::Intrinsic::uadd_with_overflow:
			WITH_OVERFLOW_INST(I, CreateZExt, CreateAdd)
		case llvm::Intrinsic::usub_with_overflow:
			WITH_OVERFLOW_INST(I, CreateZExt, CreateSub)
		case llvm::Intrinsic::umul_with_overflow:
			WITH_OVERFLOW_INST(I, CreateZExt, CreateMul)
		default:
			IL->LowerIntrinsicCall(I);
			modified = true;
			break;
		}
	}

	/* As removing other instructions while lowering *.with.overflows is unsafe while
	 * iterating over those same instructions, we do deletions separately */
	for (auto *inst : toRemove)
		inst->eraseFromParent();
	return modified;
}

auto IntrinsicLoweringPass::run(Function &F, FunctionAnalysisManager & /*FAM*/) -> PreservedAnalyses
{
	auto ILUP = std::make_unique<IntrinsicLowering>(F.getParent()->getDataLayout());

	/* Scan through the instructions and lower intrinsic calls */
	auto modified = false;
	for (auto &bb : F)
		modified |= runOnBasicBlock(bb, &*ILUP);

	return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
