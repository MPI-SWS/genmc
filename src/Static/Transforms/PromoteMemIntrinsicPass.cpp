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

#include "PromoteMemIntrinsicPass.hpp"
#include "Support/Error.hpp"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <ranges>

using namespace llvm;

auto isPromotableMemIntrinsicOperand(Value *op) -> bool
{
	// Constant to capture MemSet too
	return isa<Constant>(op) || isa<AllocaInst>(op) || isa<GetElementPtrInst>(op);
}

auto getPromotionGEPType(Value *op) -> Type *
{
	BUG_ON(!isPromotableMemIntrinsicOperand(op));
	if (auto *v = dyn_cast<GlobalVariable>(op))
		return v->getValueType();
	if (auto *ai = dyn_cast<AllocaInst>(op))
		return ai->getAllocatedType();
	if (auto *gepi = dyn_cast<GetElementPtrInst>(op))
		return gepi->getResultElementType();
	BUG();
}

void promoteMemCpy(IRBuilder<> &builder, Value *dst, Value *src, const std::vector<Value *> &args,
		   Type *typ)
{
	Value *srcGEP =
		builder.CreateInBoundsGEP(getPromotionGEPType(src), src, args, "memcpy.src.gep");
	Value *dstGEP =
		builder.CreateInBoundsGEP(getPromotionGEPType(dst), dst, args, "memcpy.dst.gep");
	Value *srcLoad = builder.CreateLoad(typ, srcGEP, "memcpy.src.load");
	Value *dstStore = builder.CreateStore(srcLoad, dstGEP);
}

void promoteMemSet(IRBuilder<> &builder, Value *dst, Value *argVal,
		   const std::vector<Value *> &args, Type *typ)
{
	BUG_ON(!typ->isIntegerTy() && !typ->isPointerTy());
	BUG_ON(!isa<ConstantInt>(argVal));

	auto &DL = builder.GetInsertBlock()->getParent()->getParent()->getDataLayout();
	auto sizeInBits = typ->isIntegerTy() ? typ->getIntegerBitWidth()
					     : DL.getPointerTypeSizeInBits(typ);
	long int ival = dyn_cast<ConstantInt>(argVal)->getSExtValue();
	Value *val = Constant::getIntegerValue(typ, APInt(sizeInBits, ival));

	Value *dstGEP =
		builder.CreateInBoundsGEP(getPromotionGEPType(dst), dst, args, "memset.dst.gep");
	Value *dstStore = builder.CreateStore(val, dstGEP);
}

template <typename F>
void promoteMemIntrinsic(Type *typ, std::vector<Value *> &args, F &&promoteFun)
{
	auto *i32Ty = IntegerType::getInt32Ty(typ->getContext());

	if (!isa<StructType>(typ) && !isa<ArrayType>(typ) && !isa<VectorType>(typ)) {
		promoteFun(typ, args);
		return;
	}

	if (ArrayType *AT = dyn_cast<ArrayType>(typ)) {
#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
		auto n = AT->getNumElements();
#else
		auto n = AT->getArrayNumElements();
#endif
		for (auto i = 0u; i < n; i++) {
			args.push_back(Constant::getIntegerValue(i32Ty, APInt(32, i)));
			promoteMemIntrinsic(AT->getElementType(), args, promoteFun);
			args.pop_back();
		}
	} else if (StructType *ST = dyn_cast<StructType>(typ)) {
		auto i = 0u;
		for (auto it = ST->element_begin(); i < ST->getNumElements(); ++it, ++i) {
			args.push_back(Constant::getIntegerValue(i32Ty, APInt(32, i)));
			promoteMemIntrinsic(*it, args, promoteFun);
			args.pop_back();
		}
	} else {
		BUG();
	}
	return;
}

auto canPromoteMemIntrinsic(MemIntrinsic *MI) -> bool
{
	/* Skip if length is not a constant */
	ConstantInt *length = dyn_cast<ConstantInt>(MI->getLength());
	if (!length) {
		WARN_ONCE("memintr-length", "Cannot promote non-constant-length mem intrinsic!"
					    "Skipping...\n");
		return false;
	}

	/*
	 * Check whether the size is 0
	 * This should not normally be produced by clang, so simply produce a warning.
	 * If we do find such a case, we might have to just remove the memcpy() call
	 */
	uint64_t size = length->getLimitedValue();
	if (size == 0) {
		WARN_ONCE("memintr-zero-length", "Cannot promote zero-length mem intrinsic!"
						 "Skipping...\n");
		return false;
	}

	/*
	 * For memcpy(), make sure source and dest live in the same address space
	 * (This also makes sure we are not copying from genmc's space to userspace)
	 */
	if (auto *MC = dyn_cast<MemCpyInst>(MI))
		BUG_ON(MC->getSourceAddressSpace() != MC->getDestAddressSpace());

	if (!isPromotableMemIntrinsicOperand(MI->getDest()) ||
	    (isa<MemCpyInst>(MI) &&
	     !isPromotableMemIntrinsicOperand(dyn_cast<MemCpyInst>(MI)->getSource()))) {
		WARN_ONCE("memintr-dst",
			  "Cannot promote intrinsic due to opaque operand type pointer!"
			  "Skipping...\n");
		return false;
	}

	/*
	 * Finally, check whether this is one of the cases we can currently handle
	 * We produce a warning anyway because, e.g., if a small struct has no atomic
	 * fields, clang might initialize it with memcpy(), and then read it with a
	 * 64bit access, and mixed-size accesses are __bad__ news
	 */
	WARN_ONCE("promote-memintrinsic", "Memory intrinsic found! Attempting to promote it...\n");
	return true;
}

auto tryPromoteMemCpy(MemCpyInst *MI, SmallVector<llvm::MemIntrinsic *, 8> &promoted) -> bool
{
	if (!canPromoteMemIntrinsic(MI))
		return false;

	auto *dst = MI->getDest();
	auto *src = MI->getSource();

	auto *i64Ty = IntegerType::getInt64Ty(MI->getContext());
	auto *nullInt = Constant::getNullValue(i64Ty);
	auto *dstTyp = getPromotionGEPType(dst);
	BUG_ON(!dstTyp);

	IRBuilder<> builder(MI);
	std::vector<Value *> args = {nullInt};

	promoteMemIntrinsic(dstTyp, args, [&](Type *typ, const std::vector<Value *> &args) {
		promoteMemCpy(builder, dst, src, args, typ);
	});
	promoted.push_back(MI);
	return true;
}

auto tryPromoteMemSet(MemSetInst *MS, SmallVector<MemIntrinsic *, 8> &promoted) -> bool
{
	if (!canPromoteMemIntrinsic(MS))
		return false;

	auto *dst = MS->getDest();
	auto *val = MS->getValue();

	auto *i64Ty = IntegerType::getInt64Ty(MS->getContext());
	auto *nullInt = Constant::getNullValue(i64Ty);
	auto *dstTyp = getPromotionGEPType(dst);
	BUG_ON(!dstTyp);

	IRBuilder<> builder(MS);
	std::vector<Value *> args = {nullInt};

	promoteMemIntrinsic(dstTyp, args, [&](Type *typ, const std::vector<Value *> &args) {
		promoteMemSet(builder, dst, val, args, typ);
	});
	promoted.push_back(MS);
	return true;
}

void removePromoted(std::ranges::input_range auto &&promoted)
{
	for (auto *MI : promoted) {

		/* Are MI's operands used anywhere else? */
		BitCastInst *dst = dyn_cast<BitCastInst>(MI->getRawDest());
		BitCastInst *src = nullptr;
		if (auto *MC = dyn_cast<MemCpyInst>(MI))
			src = dyn_cast<BitCastInst>(MC->getRawSource());

		MI->eraseFromParent();
		if (dst && dst->hasNUses(0))
			dst->eraseFromParent();
		if (src && src->hasNUses(0))
			src->eraseFromParent();
	}
}

auto PromoteMemIntrinsicPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	/* Locate mem intrinsics of interest */
	SmallVector<llvm::MemIntrinsic *, 8> promoted;
	auto modified = false;
	for (auto &I : instructions(F)) {
		if (auto *MI = dyn_cast<MemCpyInst>(&I))
			modified |= tryPromoteMemCpy(MI, promoted);
		if (auto *MS = dyn_cast<MemSetInst>(&I))
			modified |= tryPromoteMemSet(MS, promoted);
	}

	/* Erase promoted intrinsics from the code */
	removePromoted(promoted);
	return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
