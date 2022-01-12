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
#include "Error.hpp"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace llvm;

void promoteMemCpy(IRBuilder<> &builder, Value *dst, Value *src,
		   const std::vector<Value *> &args, Type *typ)
{
	Value *srcGEP = builder.CreateInBoundsGEP(dyn_cast<PointerType>(src->getType())->getElementType(),
						  src, args, "memcpy.src.gep");
	Value *dstGEP = builder.CreateInBoundsGEP(dyn_cast<PointerType>(dst->getType())->getElementType(),
						  dst, args, "memcpy.dst.gep");
	Value *srcLoad = builder.CreateLoad(typ, srcGEP, "memcpy.src.load");
	Value *dstStore = builder.CreateStore(srcLoad, dstGEP);
	return;
}

void promoteMemSet(IRBuilder<> &builder, Value *dst, Value *argVal,
		   const std::vector<Value *> &args, Type *typ)
{
	BUG_ON(!typ->isIntegerTy());
	BUG_ON(!isa<ConstantInt>(argVal));

	long int ival = dyn_cast<ConstantInt>(argVal)->getSExtValue();
	Value *val = Constant::getIntegerValue(typ, APInt(typ->getIntegerBitWidth(), ival));

	Value *dstGEP = builder.CreateInBoundsGEP(dyn_cast<PointerType>(dst->getType())->getElementType(),
						  dst, args, "memset.dst.gep");
	Value *dstStore = builder.CreateStore(val, dstGEP);
	return;
}

template<typename F>
void promoteMemIntrinsic(Type *typ, std::vector<Value *> &args, F&& promoteFun)
{
	auto *i32Ty = IntegerType::getInt32Ty(typ->getContext());

	if(!isa<StructType>(typ) && !isa<ArrayType>(typ) && !isa<VectorType>(typ)) {
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

bool canPromoteMemIntrinsic(MemIntrinsic *MI)
{
	/* Skip if length is not a constant */
	ConstantInt *length = dyn_cast<ConstantInt>(MI->getLength());
	if (!length) {
		WARN_ONCE("memintr-length", "Cannot promote non-constant-length mem intrinsic!" \
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
		WARN_ONCE("memintr-zero-length", "Cannot promote zero-length mem intrinsic!" \
			  "Skipping...\n");
		return false;
	}

	/*
	 * For memcpy(), make sure source and dest live in the same address space
	 * (This also makes sure we are not copying from genmc's space to userspace)
	 */
	if (auto *MC = dyn_cast<MemCpyInst>(MI))
		BUG_ON(MC->getSourceAddressSpace() != MC->getDestAddressSpace());

	/*
	 * Finally, check whether this is one of the cases we can currently handle
	 * We produce a warning anyway because, e.g., if a small struct has no atomic
	 * fields, clang might initialize it with memcpy(), and then read it with a
	 * 64bit access, and mixed-size accesses are __bad__ news
	 */
	WARN_ONCE("promote-memintrinsic", "Memory intrinsic found! Attempting to promote it...\n");
	return true;
}

bool PromoteMemIntrinsicPass::tryPromoteMemCpy(MemCpyInst *MI, Module &M)
{
	if (!canPromoteMemIntrinsic(MI))
		return false;

	auto *dst = MI->getDest();
	auto *src = MI->getSource();

	auto *i64Ty = IntegerType::getInt64Ty(MI->getContext());
	auto *nullInt = Constant::getNullValue(i64Ty);
	auto *dstTyp = dyn_cast<PointerType>(dst->getType());
	BUG_ON(!dstTyp);

	IRBuilder<> builder(MI);
	std::vector<Value *> args = { nullInt };

	promoteMemIntrinsic(dstTyp->getElementType(), args,
			    [&](Type *typ, const std::vector<Value *> &args)
			    { promoteMemCpy(builder, dst, src, args, typ); });
	promoted.push_back(MI);
	return true;
}

bool PromoteMemIntrinsicPass::tryPromoteMemSet(MemSetInst *MS, Module &M)
{
	if (!canPromoteMemIntrinsic(MS))
		return false;

	auto *dst = MS->getDest();
	auto *val = MS->getValue();

	auto *i64Ty = IntegerType::getInt64Ty(MS->getContext());
	auto *nullInt = Constant::getNullValue(i64Ty);
	auto *dstTyp = dyn_cast<PointerType>(MS->getDest()->getType());
	BUG_ON(!dstTyp);

	IRBuilder<> builder(MS);
	std::vector<Value *> args = { nullInt };

	promoteMemIntrinsic(dstTyp->getElementType(), args,
			    [&](Type *typ, const std::vector<Value *> &args)
			    { promoteMemSet(builder, dst, val, args, typ); });
	promoted.push_back(MS);
	return true;
}

void PromoteMemIntrinsicPass::removePromoted()
{
	for (auto MI : promoted) {

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

	promoted.clear(); // fix canpromote functions that produce warnings
}

bool PromoteMemIntrinsicPass::runOnModule(Module &M)
{
	/* We assume that no new intrinsics are going to be generated */
	if (hasPromoted)
		return false;

	bool modified = false;

	/* Locate mem intrinsics of interest */
	for (auto &F : M) {
		for (auto it = inst_iterator(F), ei = inst_end(F); it != ei; ++it) {
			if (auto *MI = dyn_cast<MemCpyInst>(&*it))
				modified |= tryPromoteMemCpy(MI, M);
			if (auto *MS = dyn_cast<MemSetInst>(&*it))
				modified |= tryPromoteMemSet(MS, M);
		}
	}

	/* Erase promoted intrinsics from the code */
	removePromoted();
	hasPromoted = true;

	return modified;
}

ModulePass *createPromoteMemIntrinsicPass()
{
	return new PromoteMemIntrinsicPass();
}

char PromoteMemIntrinsicPass::ID = 42;
