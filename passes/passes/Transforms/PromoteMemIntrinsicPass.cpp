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

#include "PromoteMemIntrinsicPass.hpp"
#include "genmc/Support/Error.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/Casting.h>

#include <cstdint>
#include <ranges>
#include <utility>
#include <vector>

/* With opaque pointers, no ptr->int->ptr cast is needed */
#define CAST_PTR_TO_TYPE_THROUGH_INT(builder, ptr, dstTy, dataLayout) ptr

#define CONSTEXPR_GET_FIELD_TYPE(MI, Field) MI->get##Field()->getType()

/* Helper macro that moves constant expressions into their own instructions */
#define LOWER_CONSTEXPR(builder, MI, Field)                                                        \
	if (auto *constExpr = dyn_cast<ConstantExpr>(MI->get##Field())) {                          \
		Value *newGEP = builder.Insert(constExpr->getAsInstruction());                     \
		[[maybe_unused]] Type *destType = CONSTEXPR_GET_FIELD_TYPE(MI, Field);             \
		Value *castedGEP = CAST_PTR_TO_TYPE_THROUGH_INT(                                   \
			builder, newGEP, destType,                                                 \
			MI->getParent()->getParent()->getParent()->getDataLayout());               \
		MI->set##Field(castedGEP);                                                         \
	}

using namespace llvm;

/**
 * Lower a call to: __memcpy_chk(void * dest, const void * src, size_t len, size_t destlen);
 *
 * Reference implementation (see __memcpy_chk source):
 * 	if dstlen < len
 * 		__chk_fail ();
 * 	return memcpy(dest, src, len);
 *
 * __memcpy_chk spec:
 * http://refspecs.linux-foundation.org/LSB_4.0.0/LSB-Core-generic/LSB-Core-generic/libc---memcpy-chk-1.html
 * __memcpy_chk source:
 * https://sourceware.org/git/?p=glibc.git;a=blob;f=debug/memcpy_chk.c;h=6f8628ab945c5e8d2738f7382238c1e1006afe41;hb=HEAD
 * memcpy spec: https://pubs.opengroup.org/onlinepubs/9699919799/functions/memcpy.html
 */
static void lowerFortifiedMemCpy(CallInst *CI, Function &F)
{
	/* Get arguments */
	auto *dst_ptr = CI->getArgOperand(0);
	auto *src_ptr = CI->getArgOperand(1);
	auto *len = CI->getArgOperand(2);
	auto *dst_len = CI->getArgOperand(3);

	/* Split the basic block at the call to __memcpy_chk in bb (before call-instr.) and contBB
	 * (rest of the block) */
	auto *bb = CI->getParent();
	auto *contBB =
		bb->splitBasicBlock(CI); /* Creates a dummy terminator for bb -> remove later */

	/* Insert the llvm.memcpy intrinsic at the start of contBB */
	IRBuilder<> contBuilder(contBB, contBB->begin());
	contBuilder.CreateMemCpy(dst_ptr, Align(1), src_ptr, Align(1), len);
	CI->replaceAllUsesWith(
		dst_ptr); /* memcpy(...) returns the dst_ptr back, see: memcpy Spec */

	/* Create a new basic block "memcpy__chk_fail" for inside the IF => aborts using an
	 * unreachable-instruction */
	auto *failBB = BasicBlock::Create(F.getContext(), "memcpy__chk_fail", &F);
	auto *assertFailFun = F.getParent()->getFunction("__VERIFIER_assert_fail");
	CallInst::Create(assertFailFun, {}, "", failBB);
	new UnreachableInst(F.getContext(), failBB);

	/* Compare arguments: dstlen < len */
	auto *dummyTerminator = bb->getTerminator();
	IRBuilder<> termBuilder(dummyTerminator);
	auto *cmp = termBuilder.CreateICmpULT(dst_len, len);

	/* Replace the dummy terminator: We jump to failBB if the condition holds */
	dummyTerminator->eraseFromParent();
	termBuilder.SetInsertPoint(bb);
	termBuilder.CreateCondBr(cmp, /* True */ failBB, /* False */ contBB);
}

/**
 * We collect and lower all calls to the fortified version of memcpy: __memcpy_chk
 * The fortified version checks for buffer overflows before performing the memcpy.
 * @see lowerFortifiedMemCpy for implementation details
 */
static auto lowerFortifiedCalls(Function &F) -> bool
{
	auto modified = false;
	SmallVector<CallInst *, 8> fortifiedCalls;

	/* Collect call-instructions to __memcpy_chk */
	for (auto &I : instructions(F)) {
		auto *ci = dyn_cast<CallInst>(&I);
		if (!ci)
			continue;

		auto *calledFun = dyn_cast<Function>(ci->getCalledOperand());
		if (!calledFun || calledFun->getName() != "__memcpy_chk")
			continue;

		fortifiedCalls.push_back(ci);
		modified = true;
	}

	for (auto *ci : fortifiedCalls)
		lowerFortifiedMemCpy(ci, F);
	for (auto *ci : fortifiedCalls)
		ci->eraseFromParent();
	return modified;
}

static auto isPromotableMemIntrinsicOperand(Value *op) -> bool
{
	/* Constant to capture MemSet too */
	return isa<Constant>(op) || isa<AllocaInst>(op) || isa<GetElementPtrInst>(op);
}

static auto getPromotionGEPType(Value *op) -> Type *
{
	VERIFY(isPromotableMemIntrinsicOperand(op));
	if (auto *v = dyn_cast<GlobalVariable>(op))
		return v->getValueType();
	if (auto *ai = dyn_cast<AllocaInst>(op))
		return ai->getAllocatedType();
	if (auto *gepi = dyn_cast<GetElementPtrInst>(op))
		return gepi->getResultElementType();
	UNREACHABLE();
}

static void promoteMemCpy(IRBuilder<> &builder, Value *dst, Value *src,
			  const std::vector<Value *> &args, Type *typ, uint64_t &remainingLen)
{
	if (remainingLen == 0)
		return;

	auto *srcGEP =
		builder.CreateInBoundsGEP(getPromotionGEPType(src), src, args, "memcpy.src.gep");
	auto *dstGEP =
		builder.CreateInBoundsGEP(getPromotionGEPType(dst), dst, args, "memcpy.dst.gep");

	auto len = builder.GetInsertBlock()->getModule()->getDataLayout().getTypeStoreSize(typ);
	VERIFY(len <= remainingLen);

	remainingLen -= len;
	auto *srcLoad = builder.CreateLoad(typ, srcGEP, "memcpy.src.load");
	builder.CreateStore(srcLoad, dstGEP);
}

static void promoteMemSet(IRBuilder<> &builder, Value *dst, Value *argVal,
			  const std::vector<Value *> &args, Type *typ)
{
	VERIFY(typ->isIntegerTy() || typ->isPointerTy());
	VERIFY(isa<ConstantInt>(argVal));

	const auto &DL = builder.GetInsertBlock()->getParent()->getParent()->getDataLayout();
	auto sizeInBits = typ->isIntegerTy() ? typ->getIntegerBitWidth()
					     : DL.getPointerTypeSizeInBits(typ);
	const long int ival = cast<ConstantInt>(argVal)->getSExtValue();
	Value *val = Constant::getIntegerValue(typ, APInt(sizeInBits, ival));

	Value *dstGEP =
		builder.CreateInBoundsGEP(getPromotionGEPType(dst), dst, args, "memset.dst.gep");
	builder.CreateStore(val, dstGEP);
}

template <typename F>
static void promoteMemIntrinsic(Type *typ, std::vector<Value *> &args, F &&promoteFun)
{
	auto *i32Ty = IntegerType::getInt32Ty(typ->getContext());

	if (!isa<StructType>(typ) && !isa<ArrayType>(typ) && !isa<VectorType>(typ)) {
		std::forward<F>(promoteFun)(typ, args);
		return;
	}

	if (auto *arrayType = dyn_cast<ArrayType>(typ)) {
#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
		auto numElems = arrayType->getNumElements();
#else
		auto numElems = arrayType->getArrayNumElements();
#endif
		for (auto i = 0U; i < numElems; i++) {
			args.push_back(Constant::getIntegerValue(i32Ty, APInt(32, i)));
			promoteMemIntrinsic(arrayType->getElementType(), args, promoteFun);
			args.pop_back();
		}
	} else if (auto *structType = dyn_cast<StructType>(typ)) {
		for (auto i = 0U; i < structType->getNumElements(); ++i) {
			args.push_back(Constant::getIntegerValue(i32Ty, APInt(32, i)));
			promoteMemIntrinsic(structType->getElementType(i), args, promoteFun);
			args.pop_back();
		}
	} else {
		UNREACHABLE();
	}
}

static auto canPromoteMemIntrinsic(MemIntrinsic *MI) -> bool
{
	/* Skip if length is not a constant */
	auto *length = dyn_cast<ConstantInt>(MI->getLength());
	if (!length) {
		WARN_ONCE("memintr-length",
			  "Cannot promote non-constant-length mem intrinsic! Skipping...\n");
		return false;
	}

	/*
	 * For memcpy(), make sure source and dest live in the same address space
	 * (This also makes sure we are not copying from genmc's space to userspace)
	 */
	auto *MCI = dyn_cast<MemCpyInst>(MI);
	VERIFY(!MCI || MCI->getSourceAddressSpace() == MCI->getDestAddressSpace());
	if (MCI && !(isPromotableMemIntrinsicOperand(MCI->getDest()) ||
		     isPromotableMemIntrinsicOperand(MCI->getSource()))) {
		WARN_ONCE("memintr-opaque", "Cannot promote memcpy() due to both src and dst being "
					    "opaque! Skipping...\n");
		return false;
	}

	auto *MSI = dyn_cast<MemSetInst>(MI);
	if (MSI && !isPromotableMemIntrinsicOperand(MSI->getDest())) {
		WARN_ONCE("memintr-dst",
			  "Cannot promote memset() due to dst being opaque! Skipping...\n");
		return false;
	}

	/*
	 * Finally, this is one of the cases we can currently handle.
	 * We produce a warning anyway because, e.g., if a small struct has no atomic
	 * fields, clang might initialize it with memcpy(), and then read it with a
	 * 64bit access, and mixed-size accesses are __bad__ news
	 */
	WARN_ONCE("promote-memintrinsic", "Memory intrinsic found! Attempting to promote it...\n");
	return true;
}

/**
 * Due to LLVM's use of opaque pointers, we infer the types based on the instruction defining the
 * value (Alloca, GEP, GlobalVal => @see getPromotionGEPType()). To allow copying between different
 * (compatible) data-types, we perform a cast either src->dst or dst->src.
 *
 * Note: We assume that the compiler only generates memcpy()s between "compatible" types:
 *  - compatible: struct {i32, i32, i32} and [3 x i32]
 *  - non-compatible:  struct {i32, i64, i32} and [4 x i32]
 * Promotion in the second case would lead to "mixed-sized accesses".
 */
static auto getRecastedOperands(MemCpyInst *MI, IRBuilder<> &builder) -> std::pair<Value *, Value *>
{
	auto *dst = MI->getDest();
	auto *src = MI->getSource();

	auto dataLayout = MI->getParent()->getParent()->getParent()->getDataLayout();
	auto indexSize = dataLayout.getIndexSizeInBits(dst->getType()->getPointerAddressSpace());
	const std::vector<Value *> gepArgs = {Constant::getIntegerValue(
		IntegerType::get(builder.getContext(), indexSize),
		APInt(indexSize, 0))}; // Ex: getelementptr %type, %ptr, i32 0

	/* Case src->dst */
	if (isPromotableMemIntrinsicOperand(dst)) {
		src = CAST_PTR_TO_TYPE_THROUGH_INT(builder, src, dst->getType(), dataLayout);
		return {builder.CreateGEP(getPromotionGEPType(dst), src, ArrayRef(gepArgs)), dst};
	}

	/* Case dst->src */
	VERIFY(isPromotableMemIntrinsicOperand(src));
	dst = CAST_PTR_TO_TYPE_THROUGH_INT(builder, dst, src->getType(), dataLayout);
	return {src, builder.CreateGEP(getPromotionGEPType(src), dst, ArrayRef(gepArgs))};
}

/* Tries to promote a memcpy() instruction.
 * We also allow the promotion of memcpy()s w/ constant arguments:
 *
 * call void @llvm.memcpy.p0.p0.i64(
 *      ptr align 8 getelementptr inbounds (%struct.queue_t, ptr @queue, i64 0, i32 0, i32 1),
 *      ptr align 8 %_3,
 *      i64 8,
 *      i1 false)
 *
 * by moving constant expressions into their own instruction and proceeding as normal. */
static auto tryPromoteMemCpy(MemCpyInst *MI, SmallVector<llvm::MemIntrinsic *, 8> &promoted) -> bool
{
	if (!canPromoteMemIntrinsic(MI))
		return false;

	/* We only copy "len" bytes (3rd arg in llvm.memcpy) */
	auto len = cast<ConstantInt>(MI->getLength())->getZExtValue();

	/* Remove memcpy's with len=0 completely as no-ops (generated by rustc) */
	if (len == 0) {
		WARN_ONCE("memintr-zero-length",
			  "Cannot promote zero-length mem intrinsic! Removing instruction...\n");
		promoted.push_back(MI);
		return true;
	}

	IRBuilder<> builder(MI);
	auto *i64Ty = IntegerType::getInt64Ty(MI->getContext());
	auto *nullInt = Constant::getNullValue(i64Ty);

	/* Check for constexpr arguments */
	LOWER_CONSTEXPR(builder, MI, Source); // NOLINT(misc-const-correctness)
	LOWER_CONSTEXPR(builder, MI, Dest);   // NOLINT(misc-const-correctness)

	/* Recast args to same types for GEP indexing later */
	auto [src, dst] = getRecastedOperands(MI, builder);
	auto *dstTyp = getPromotionGEPType(dst);
	VERIFY(dstTyp);

	/* To ensure we only copy "len" bytes from total */
	auto typeSizeDst = MI->getParent()->getModule()->getDataLayout().getTypeStoreSize(dstTyp);
	VERIFY(typeSizeDst >= len);

	std::vector<Value *> args = {nullInt};
	promoteMemIntrinsic(dstTyp, args, [&](Type *typ, const std::vector<Value *> &args) {
		promoteMemCpy(builder, dst, src, args, typ, len);
	});
	promoted.push_back(MI);
	return true;
}

static auto tryPromoteMemSet(MemSetInst *MS, SmallVector<MemIntrinsic *, 8> &promoted) -> bool
{
	if (!canPromoteMemIntrinsic(MS))
		return false;

	auto *dst = MS->getDest();
	auto *val = MS->getValue();

	auto *i64Ty = IntegerType::getInt64Ty(MS->getContext());
	auto *nullInt = Constant::getNullValue(i64Ty);
	auto *dstTyp = getPromotionGEPType(dst);
	VERIFY(dstTyp);

	IRBuilder<> builder(MS);
	std::vector<Value *> args = {nullInt};

	promoteMemIntrinsic(dstTyp, args, [&](Type *typ, const std::vector<Value *> &args) {
		promoteMemSet(builder, dst, val, args, typ);
	});
	promoted.push_back(MS);
	return true;
}

static void removePromoted(std::ranges::input_range auto &&promoted)
{
	for (auto *MI : promoted) {
		/* Are MI's operands used anywhere else? */
		auto *dst = dyn_cast<BitCastInst>(MI->getRawDest());
		CastInst *src = nullptr;
		if (auto *MC = dyn_cast<MemCpyInst>(MI))
			src = dyn_cast<BitCastInst>(MC->getRawSource());

		MI->eraseFromParent();
		if (dst && dst->hasNUses(0))
			dst->eraseFromParent();
		if (src && src->hasNUses(0))
			src->eraseFromParent();
	}
}

auto PromoteMemIntrinsicPass::run(Function &F, FunctionAnalysisManager & /*FAM*/)
	-> PreservedAnalyses
{
	/* Locate mem intrinsics of interest */
	SmallVector<llvm::MemIntrinsic *, 8> promoted;
	auto modified = false;

	modified |= lowerFortifiedCalls(F);
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
