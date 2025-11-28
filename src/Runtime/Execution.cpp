// For the parts of the code originating from LLVM-3.5:
//===-- Execution.cpp - Implement code to simulate the program ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LLVMLICENSE for details.
//
//===----------------------------------------------------------------------===//
//
//  This file contains the actual instruction interpreter.
//
//===----------------------------------------------------------------------===//

/*
 * (For the parts of the code modified from LLVM-3.5)
 *
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

#include "ExecutionGraph/Event.hpp"
#include "Runtime/Interpreter.h"
#include "Static/LLVMUtils.hpp"
#include "Support/Error.hpp"
#include "Support/SExprVisitor.hpp"
#include "Verification/GenMCDriver.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <cmath>

using namespace llvm;

#define DEBUG_TYPE "interpreter"

#if LLVM_VERSION_MAJOR < 11
#define LLVM_VECTOR_TYPEID_CASES case llvm::Type::VectorTyID:
#else
#define LLVM_VECTOR_TYPEID_CASES                                                                   \
	case llvm::Type::FixedVectorTyID:                                                          \
	case llvm::Type::ScalableVectorTyID:
#endif

// static cl::opt<bool> PrintVolatile("interpreter-print-volatile", cl::Hidden,
//           cl::desc("make the interpreter print every volatile load and store"));

//===----------------------------------------------------------------------===//
//                     Various Helper Functions
//===----------------------------------------------------------------------===//

#define GET_DEPS(deps) (deps ? *deps : EventDeps())

#define SVAL_TO_GV(val, typ)                                                                       \
	({                                                                                         \
		llvm::GenericValue __result;                                                       \
		if (auto *iTyp = llvm::dyn_cast<IntegerType>(typ))                                 \
			__result.IntVal = APInt(iTyp->getBitWidth(), (val).get(),                  \
						iTyp->getSignBit() APINT_NOCHECK);                 \
		else                                                                               \
			__result.PointerVal = (void *)(val).get();                                 \
		__result;                                                                          \
	})

#define GV_TO_SVAL(val, typ)                                                                       \
	({                                                                                         \
		SVal __result;                                                                     \
		if (auto *iTyp = llvm::dyn_cast<IntegerType>(typ))                                 \
			__result = SVal((val).IntVal.getLimitedValue());                           \
		else                                                                               \
			__result = SVal((uintptr_t)(val).PointerVal);                              \
		__result;                                                                          \
	})

#define TYPE_TO_ATYPE(typ)                                                                         \
	({                                                                                         \
		AType __result;                                                                    \
		if (auto *iTyp = llvm::dyn_cast<IntegerType>(typ))                                 \
			__result = (iTyp->getSignBit() ? AType::Signed : AType::Unsigned);         \
		else                                                                               \
			__result = AType::Pointer;                                                 \
		__result;                                                                          \
	})

/** Translates an LLVM ordering to our internal one; assumes the
 * ordering is one we support (i.e., currently not Unordered)*/
inline auto toGenMCOrdering(llvm::AtomicOrdering ord) -> MemOrdering
{
	static const MemOrdering lookup[8] = {
		/* NotAtomic */ MemOrdering::NotAtomic,
		/* Unordered */ MemOrdering::Relaxed,
		/* monotonic */ MemOrdering::Relaxed,
		/* consume   */ MemOrdering::Acquire,
		/* acquire   */ MemOrdering::Acquire,
		/* release   */ MemOrdering::Release,
		/* acq_rel   */ MemOrdering::AcquireRelease,
		/* seq_cst   */ MemOrdering::SequentiallyConsistent,
	};
	return lookup[static_cast<size_t>(ord)];
}

/* Translates an LLVM opreation to our internal one; assumes the
 * operation is one we support (i.e., no fops,udecwrap,etc)*/
inline auto toGenMCBinOp(llvm::AtomicRMWInst::BinOp op) -> RMWBinOp
{
	static const RMWBinOp lookup[11] = {
		/* xchg */ RMWBinOp::Xchg,
		/* add  */ RMWBinOp::Add,
		/* sub  */ RMWBinOp::Sub,
		/* and  */ RMWBinOp::And,
		/* nand */ RMWBinOp::Nand,
		/* or   */ RMWBinOp::Or,
		/* xor  */ RMWBinOp::Xor,
		/* max  */ RMWBinOp::Max,
		/* min  */ RMWBinOp::Min,
		/* umax */ RMWBinOp::UMax,
		/* umin */ RMWBinOp::UMin,
	};
	BUG_ON(!isValidRMWBinOp(
		static_cast<std::underlying_type_t<llvm::AtomicRMWInst::BinOp>>(op)));
	return lookup[static_cast<size_t>(op)];
}

static void SetValue(Value *V, GenericValue Val, ExecutionContext &SF) { SF.Values[V] = Val; }

bool Interpreter::isStaticallyAllocated(SAddr addr) const
{
	auto p = std::make_pair(addr, addr);
	auto it = std::lower_bound(
		staticAllocas.begin(), staticAllocas.end(), p,
		[](const decltype(p) &itV, const decltype(p) &v) { return itV.second < v.first; });
	return it == staticAllocas.end() ? false : (it->first <= addr && addr <= it->second);
}

SAddr getStaticAllocBegin(const VSet<std::pair<SAddr, SAddr>> &allocMap, SAddr addr)
{
	auto p = std::make_pair(addr, addr);
	auto it = std::lower_bound(
		allocMap.begin(), allocMap.end(), p,
		[](const decltype(p) &itV, const decltype(p) &v) { return itV.second < v.first; });
	BUG_ON(it == allocMap.end() || addr < it->first || addr > it->second);
	return it->first;
}

const NameInfo *
Interpreter::getVarNameInfo(Value *v, StorageDuration sd, AddressSpace spc,
			    const VariableInfo<ModuleID::ID>::InternalKey &key /* = {} */)
{
	if (!dynState.collectDbgInfo)
		return nullptr;

	if (spc == AddressSpace::AS_Internal)
		return MI->varInfo.internalInfo[key].get();

	switch (sd) {
	case StorageDuration::SD_Static:
		return MI->varInfo.globalInfo[MI->idInfo.VID[v]].get();
	case StorageDuration::SD_Automatic:
		return MI->varInfo.localInfo[MI->idInfo.VID[v]].get();
	case StorageDuration::SD_Heap:
		return nullptr;
	default:
		BUG();
	}
	return nullptr;
}

std::string Interpreter::getStaticName(SAddr addr) const
{
	/* Don't complain if it's not allocated so that we can safely use it during error reporting
	 */
	if (!isStaticallyAllocated(addr))
		return "";

	auto sBeg = getStaticAllocBegin(staticAllocas, addr);
	BUG_ON(!staticNames.count(sBeg));
	auto gv = staticNames.at(sBeg);
	auto gvID = MI->idInfo.VID[gv];
	BUG_ON(!MI->varInfo.globalInfo.count(gvID));
	auto &gi = *MI->varInfo.globalInfo.at(gvID);
	return gv->getName().str() + gi.getNameAtOffset(addr - sBeg);
}

void *Interpreter::getStaticAddr(SAddr addr) const
{
	/* If the address is not statically allocated, something went
	 * wrong with the access validity checks; don't bother recovering */
	BUG_ON(!isStaticallyAllocated(addr));
	auto sBeg = getStaticAllocBegin(staticAllocas, addr);
	BUG_ON(!staticValueMap.count(sBeg));
	return (char *)staticValueMap.at(sBeg) + (addr.get() - sBeg.get());
}

std::optional<Annotation> Interpreter::getCurrentAnnotConcretized()
{
	auto *l = ECStack().back().CurInst->getPrevNode();
	auto id = MI->idInfo.VID[l];
	if (!MI->annotInfo.annotMap.count(id))
		return {};

	Annotation result;
	auto info = MI->annotInfo.annotMap.at(id);

	using Concretizer = SExprConcretizer<AnnotID>;
	auto &stackVals = ECStack().back().Values;
	Concretizer::ReplaceMap vMap;

	for (auto &kv : stackVals) {
		/* (1) Check against NULL due to possibly empty thread parameter list
		 * (2) Ensure that the load itself will not be concretized */
		if (kv.first && kv.first != l) {
			vMap.insert({(MI->idInfo.VID.at(kv.first)),
				     std::make_pair(GV_TO_SVAL(kv.second, kv.first->getType()),
						    ASize(getTypeSize(kv.first->getType()) * 8))});
		}
	}

	result.type = info.first;
	result.expr = Concretizer().concretize(info.second.get(), vMap);
	return result;
}

EventLabel::EventLabelKind getReadKind(LoadInst &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return Kind::Read;

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	if (GENMC_KIND(flags) == GENMC_KIND_SPECUL)
		return Kind::SpeculativeRead;
	else if (GENMC_KIND(flags) == GENMC_KIND_CONFIRM)
		return Kind::ConfirmingRead;
	else if (GENMC_KIND(flags) == GENMC_KIND_BARRIER)
		return Kind::BWaitRead;
	BUG();
}

EventLabel::EventLabelKind getUnlockKind(Instruction &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return Kind::UnlockWrite;

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	BUG_ON(GENMC_KIND(flags) != GENMC_KIND_PLOCK);
	return Kind::AbstractUnlockWrite;
}

constexpr unsigned int switchPair(EventLabel::EventLabelKind a, EventLabel::EventLabelKind b)
{
	return (unsigned(a) << 16) + b;
}

constexpr unsigned int
switchPair(std::pair<EventLabel::EventLabelKind, EventLabel::EventLabelKind> p)
{
	return switchPair(p.first, p.second);
}

std::pair<EventLabel::EventLabelKind, EventLabel::EventLabelKind> getCasKinds(AtomicCmpXchgInst &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return std::make_pair(Kind::CasRead, Kind::CasWrite);

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	if (!GENMC_KIND(flags))
		return std::make_pair(Kind::CasRead, Kind::CasWrite);

	if (GENMC_KIND(flags) == GENMC_KIND_HELPED)
		return std::make_pair(Kind::HelpedCasRead, Kind::HelpedCasWrite);
	if (GENMC_KIND(flags) == GENMC_KIND_HELPING)
		return std::make_pair(Kind::HelpingCas, Kind::HelpingCas);
	BUG_ON(GENMC_KIND(flags) != GENMC_KIND_CONFIRM);
	return std::make_pair(Kind::ConfirmingCasRead, Kind::ConfirmingCasWrite);
}

std::pair<EventLabel::EventLabelKind, EventLabel::EventLabelKind> getLockKinds(Instruction &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return std::make_pair(Kind::LockCasRead, Kind::LockCasWrite);

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	if (!GENMC_KIND(flags))
		return std::make_pair(Kind::LockCasRead, Kind::LockCasWrite);
	BUG_ON(GENMC_KIND(flags) != GENMC_KIND_PLOCK);
	return std::make_pair(Kind::AbstractLockCasRead, Kind::AbstractLockCasWrite);
}

std::pair<EventLabel::EventLabelKind, EventLabel::EventLabelKind> getFaiKinds(AtomicRMWInst &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return std::make_pair(Kind::FaiRead, Kind::FaiWrite);

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	if (GENMC_KIND(flags) == GENMC_KIND_NONVR)
		return std::make_pair(Kind::NoRetFaiRead, Kind::NoRetFaiWrite);
	else if (GENMC_KIND(flags) == GENMC_KIND_BARRIER)
		return std::make_pair(Kind::BIncFaiRead, Kind::BIncFaiWrite);
	BUG();
}

/* Returns the size (in bytes) for a given type */
unsigned int Interpreter::getTypeSize(Type *typ) const
{
	return (size_t)getDataLayout().getTypeAllocSize(typ);
}

/* Should match include/pthread.h (or barrier/mutex/thread decls) */
#define GENMC_PTHREAD_BARRIER_SERIAL_THREAD -1
#define GENMC_ASSUME_USER 0
#define GENMC_ASSUME_BARRIER 1
#define GENMC_ASSUME_SPINLOOP 2

//===----------------------------------------------------------------------===//
//                    Binary Instruction Implementations
//===----------------------------------------------------------------------===//

#define IMPLEMENT_BINARY_OPERATOR(OP, TY)                                                          \
	case Type::TY##TyID:                                                                       \
		Dest.TY##Val = Src1.TY##Val OP Src2.TY##Val;                                       \
		break

static void executeFAddInst(GenericValue &Dest, GenericValue Src1, GenericValue Src2, Type *Ty)
{
	switch (Ty->getTypeID()) {
		IMPLEMENT_BINARY_OPERATOR(+, Float);
		IMPLEMENT_BINARY_OPERATOR(+, Double);
	default:
		dbgs() << "Unhandled type for FAdd instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
}

static void executeFSubInst(GenericValue &Dest, GenericValue Src1, GenericValue Src2, Type *Ty)
{
	switch (Ty->getTypeID()) {
		IMPLEMENT_BINARY_OPERATOR(-, Float);
		IMPLEMENT_BINARY_OPERATOR(-, Double);
	default:
		dbgs() << "Unhandled type for FSub instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
}

static void executeFMulInst(GenericValue &Dest, GenericValue Src1, GenericValue Src2, Type *Ty)
{
	switch (Ty->getTypeID()) {
		IMPLEMENT_BINARY_OPERATOR(*, Float);
		IMPLEMENT_BINARY_OPERATOR(*, Double);
	default:
		dbgs() << "Unhandled type for FMul instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
}

static void executeFDivInst(GenericValue &Dest, GenericValue Src1, GenericValue Src2, Type *Ty)
{
	switch (Ty->getTypeID()) {
		IMPLEMENT_BINARY_OPERATOR(/, Float);
		IMPLEMENT_BINARY_OPERATOR(/, Double);
	default:
		dbgs() << "Unhandled type for FDiv instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
}

static void executeFRemInst(GenericValue &Dest, GenericValue Src1, GenericValue Src2, Type *Ty)
{
	switch (Ty->getTypeID()) {
	case Type::FloatTyID:
		Dest.FloatVal = fmod(Src1.FloatVal, Src2.FloatVal);
		break;
	case Type::DoubleTyID:
		Dest.DoubleVal = fmod(Src1.DoubleVal, Src2.DoubleVal);
		break;
	default:
		dbgs() << "Unhandled type for Rem instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
}

#define IMPLEMENT_INTEGER_ICMP(OP, TY)                                                             \
	case Type::IntegerTyID:                                                                    \
		Dest.IntVal = APInt(1, Src1.IntVal.OP(Src2.IntVal));                               \
		break;

#define IMPLEMENT_VECTOR_INTEGER_ICMP(OP, TY)                                                      \
	LLVM_VECTOR_TYPEID_CASES                                                                   \
	{                                                                                          \
		assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());                      \
		Dest.AggregateVal.resize(Src1.AggregateVal.size());                                \
		for (uint32_t _i = 0; _i < Src1.AggregateVal.size(); _i++)                         \
			Dest.AggregateVal[_i].IntVal = APInt(                                      \
				1, Src1.AggregateVal[_i].IntVal.OP(Src2.AggregateVal[_i].IntVal)); \
	}                                                                                          \
	break;

// Handle pointers specially because they must be compared with only as much
// width as the host has.  We _do not_ want to be comparing 64 bit values when
// running on a 32-bit target, otherwise the upper 32 bits might mess up
// comparisons if they contain garbage.
#define IMPLEMENT_POINTER_ICMP(OP)                                                                 \
	case Type::PointerTyID:                                                                    \
		Dest.IntVal = APInt(1, (void *)(intptr_t)Src1.PointerVal OP(void *)(intptr_t)      \
					       Src2.PointerVal);                                   \
		break;

static GenericValue executeICMP_EQ(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(eq, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(eq, Ty);
		IMPLEMENT_POINTER_ICMP(==);
	default:
		dbgs() << "Unhandled type for ICMP_EQ predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_NE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(ne, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(ne, Ty);
		IMPLEMENT_POINTER_ICMP(!=);
	default:
		dbgs() << "Unhandled type for ICMP_NE predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_ULT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(ult, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(ult, Ty);
		IMPLEMENT_POINTER_ICMP(<);
	default:
		dbgs() << "Unhandled type for ICMP_ULT predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_SLT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(slt, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(slt, Ty);
		IMPLEMENT_POINTER_ICMP(<);
	default:
		dbgs() << "Unhandled type for ICMP_SLT predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_UGT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(ugt, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(ugt, Ty);
		IMPLEMENT_POINTER_ICMP(>);
	default:
		dbgs() << "Unhandled type for ICMP_UGT predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_SGT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(sgt, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(sgt, Ty);
		IMPLEMENT_POINTER_ICMP(>);
	default:
		dbgs() << "Unhandled type for ICMP_SGT predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_ULE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(ule, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(ule, Ty);
		IMPLEMENT_POINTER_ICMP(<=);
	default:
		dbgs() << "Unhandled type for ICMP_ULE predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_SLE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(sle, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(sle, Ty);
		IMPLEMENT_POINTER_ICMP(<=);
	default:
		dbgs() << "Unhandled type for ICMP_SLE predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_UGE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(uge, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(uge, Ty);
		IMPLEMENT_POINTER_ICMP(>=);
	default:
		dbgs() << "Unhandled type for ICMP_UGE predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeICMP_SGE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_INTEGER_ICMP(sge, Ty);
		IMPLEMENT_VECTOR_INTEGER_ICMP(sge, Ty);
		IMPLEMENT_POINTER_ICMP(>=);
	default:
		dbgs() << "Unhandled type for ICMP_SGE predicate: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

void Interpreter::visitICmpInst(ICmpInst &I)
{
	ExecutionContext &SF = ECStack().back();
	Type *Ty = I.getOperand(0)->getType();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue R; // Result

	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	updateDataDeps(getCurThr().id, &I, I.getOperand(1));

	switch (I.getPredicate()) {
	case ICmpInst::ICMP_EQ:
		R = executeICMP_EQ(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_NE:
		R = executeICMP_NE(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_ULT:
		R = executeICMP_ULT(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_SLT:
		R = executeICMP_SLT(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_UGT:
		R = executeICMP_UGT(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_SGT:
		R = executeICMP_SGT(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_ULE:
		R = executeICMP_ULE(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_SLE:
		R = executeICMP_SLE(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_UGE:
		R = executeICMP_UGE(Src1, Src2, Ty);
		break;
	case ICmpInst::ICMP_SGE:
		R = executeICMP_SGE(Src1, Src2, Ty);
		break;
	default:
		dbgs() << "Don't know how to handle this ICmp predicate!\n-->" << I;
		llvm_unreachable(nullptr);
	}

	SetValue(&I, R, SF);
}

#define IMPLEMENT_FCMP(OP, TY)                                                                     \
	case Type::TY##TyID:                                                                       \
		Dest.IntVal = APInt(1, Src1.TY##Val OP Src2.TY##Val);                              \
		break

#define IMPLEMENT_VECTOR_FCMP_T(OP, TY)                                                            \
	assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());                              \
	Dest.AggregateVal.resize(Src1.AggregateVal.size());                                        \
	for (uint32_t _i = 0; _i < Src1.AggregateVal.size(); _i++)                                 \
		Dest.AggregateVal[_i].IntVal =                                                     \
			APInt(1, Src1.AggregateVal[_i].TY##Val OP Src2.AggregateVal[_i].TY##Val);  \
	break;

#define IMPLEMENT_VECTOR_FCMP(OP)                                                                  \
	LLVM_VECTOR_TYPEID_CASES                                                                   \
	if (dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy()) {                             \
		IMPLEMENT_VECTOR_FCMP_T(OP, Float);                                                \
	} else {                                                                                   \
		IMPLEMENT_VECTOR_FCMP_T(OP, Double);                                               \
	}

static GenericValue executeFCMP_OEQ(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_FCMP(==, Float);
		IMPLEMENT_FCMP(==, Double);
		IMPLEMENT_VECTOR_FCMP(==);
	default:
		dbgs() << "Unhandled type for FCmp EQ instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

#define IMPLEMENT_SCALAR_NANS(TY, X, Y)                                                            \
	if (TY->isFloatTy()) {                                                                     \
		if (X.FloatVal != X.FloatVal || Y.FloatVal != Y.FloatVal) {                        \
			Dest.IntVal = APInt(1, false);                                             \
			return Dest;                                                               \
		}                                                                                  \
	} else {                                                                                   \
		if (X.DoubleVal != X.DoubleVal || Y.DoubleVal != Y.DoubleVal) {                    \
			Dest.IntVal = APInt(1, false);                                             \
			return Dest;                                                               \
		}                                                                                  \
	}

#define MASK_VECTOR_NANS_T(X, Y, TZ, FLAG)                                                         \
	assert(X.AggregateVal.size() == Y.AggregateVal.size());                                    \
	Dest.AggregateVal.resize(X.AggregateVal.size());                                           \
	for (uint32_t _i = 0; _i < X.AggregateVal.size(); _i++) {                                  \
		if (X.AggregateVal[_i].TZ##Val != X.AggregateVal[_i].TZ##Val ||                    \
		    Y.AggregateVal[_i].TZ##Val != Y.AggregateVal[_i].TZ##Val)                      \
			Dest.AggregateVal[_i].IntVal = APInt(1, FLAG);                             \
		else {                                                                             \
			Dest.AggregateVal[_i].IntVal = APInt(1, !FLAG);                            \
		}                                                                                  \
	}

#define MASK_VECTOR_NANS(TY, X, Y, FLAG)                                                           \
	if (TY->isVectorTy()) {                                                                    \
		if (dyn_cast<VectorType>(TY)->getElementType()->isFloatTy()) {                     \
			MASK_VECTOR_NANS_T(X, Y, Float, FLAG)                                      \
		} else {                                                                           \
			MASK_VECTOR_NANS_T(X, Y, Double, FLAG)                                     \
		}                                                                                  \
	}

static GenericValue executeFCMP_ONE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	// if input is scalar value and Src1 or Src2 is NaN return false
	IMPLEMENT_SCALAR_NANS(Ty, Src1, Src2)
	// if vector input detect NaNs and fill mask
	MASK_VECTOR_NANS(Ty, Src1, Src2, false)
	GenericValue DestMask = Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_FCMP(!=, Float);
		IMPLEMENT_FCMP(!=, Double);
		IMPLEMENT_VECTOR_FCMP(!=);
	default:
		dbgs() << "Unhandled type for FCmp NE instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	// in vector case mask out NaN elements
	if (Ty->isVectorTy())
		for (size_t _i = 0; _i < Src1.AggregateVal.size(); _i++)
			if (DestMask.AggregateVal[_i].IntVal == false)
				Dest.AggregateVal[_i].IntVal = APInt(1, false);

	return Dest;
}

static GenericValue executeFCMP_OLE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_FCMP(<=, Float);
		IMPLEMENT_FCMP(<=, Double);
		IMPLEMENT_VECTOR_FCMP(<=);
	default:
		dbgs() << "Unhandled type for FCmp LE instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeFCMP_OGE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_FCMP(>=, Float);
		IMPLEMENT_FCMP(>=, Double);
		IMPLEMENT_VECTOR_FCMP(>=);
	default:
		dbgs() << "Unhandled type for FCmp GE instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeFCMP_OLT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_FCMP(<, Float);
		IMPLEMENT_FCMP(<, Double);
		IMPLEMENT_VECTOR_FCMP(<);
	default:
		dbgs() << "Unhandled type for FCmp LT instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

static GenericValue executeFCMP_OGT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	switch (Ty->getTypeID()) {
		IMPLEMENT_FCMP(>, Float);
		IMPLEMENT_FCMP(>, Double);
		IMPLEMENT_VECTOR_FCMP(>);
	default:
		dbgs() << "Unhandled type for FCmp GT instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}
	return Dest;
}

#define IMPLEMENT_UNORDERED(TY, X, Y)                                                              \
	if (TY->isFloatTy()) {                                                                     \
		if (X.FloatVal != X.FloatVal || Y.FloatVal != Y.FloatVal) {                        \
			Dest.IntVal = APInt(1, true);                                              \
			return Dest;                                                               \
		}                                                                                  \
	} else if (X.DoubleVal != X.DoubleVal || Y.DoubleVal != Y.DoubleVal) {                     \
		Dest.IntVal = APInt(1, true);                                                      \
		return Dest;                                                                       \
	}

#define IMPLEMENT_VECTOR_UNORDERED(TY, X, Y, _FUNC)                                                \
	if (TY->isVectorTy()) {                                                                    \
		GenericValue DestMask = Dest;                                                      \
		Dest = _FUNC(Src1, Src2, Ty);                                                      \
		for (size_t _i = 0; _i < Src1.AggregateVal.size(); _i++)                           \
			if (DestMask.AggregateVal[_i].IntVal == true)                              \
				Dest.AggregateVal[_i].IntVal = APInt(1, true);                     \
		return Dest;                                                                       \
	}

static GenericValue executeFCMP_UEQ(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	IMPLEMENT_UNORDERED(Ty, Src1, Src2)
	MASK_VECTOR_NANS(Ty, Src1, Src2, true)
	IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OEQ)
	return executeFCMP_OEQ(Src1, Src2, Ty);
}

static GenericValue executeFCMP_UNE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	IMPLEMENT_UNORDERED(Ty, Src1, Src2)
	MASK_VECTOR_NANS(Ty, Src1, Src2, true)
	IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_ONE)
	return executeFCMP_ONE(Src1, Src2, Ty);
}

static GenericValue executeFCMP_ULE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	IMPLEMENT_UNORDERED(Ty, Src1, Src2)
	MASK_VECTOR_NANS(Ty, Src1, Src2, true)
	IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OLE)
	return executeFCMP_OLE(Src1, Src2, Ty);
}

static GenericValue executeFCMP_UGE(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	IMPLEMENT_UNORDERED(Ty, Src1, Src2)
	MASK_VECTOR_NANS(Ty, Src1, Src2, true)
	IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OGE)
	return executeFCMP_OGE(Src1, Src2, Ty);
}

static GenericValue executeFCMP_ULT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	IMPLEMENT_UNORDERED(Ty, Src1, Src2)
	MASK_VECTOR_NANS(Ty, Src1, Src2, true)
	IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OLT)
	return executeFCMP_OLT(Src1, Src2, Ty);
}

static GenericValue executeFCMP_UGT(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	IMPLEMENT_UNORDERED(Ty, Src1, Src2)
	MASK_VECTOR_NANS(Ty, Src1, Src2, true)
	IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OGT)
	return executeFCMP_OGT(Src1, Src2, Ty);
}

static GenericValue executeFCMP_ORD(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	if (Ty->isVectorTy()) {
		assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
		Dest.AggregateVal.resize(Src1.AggregateVal.size());
		if (dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy()) {
			for (size_t _i = 0; _i < Src1.AggregateVal.size(); _i++)
				Dest.AggregateVal[_i].IntVal =
					APInt(1, ((Src1.AggregateVal[_i].FloatVal ==
						   Src1.AggregateVal[_i].FloatVal) &&
						  (Src2.AggregateVal[_i].FloatVal ==
						   Src2.AggregateVal[_i].FloatVal)));
		} else {
			for (size_t _i = 0; _i < Src1.AggregateVal.size(); _i++)
				Dest.AggregateVal[_i].IntVal =
					APInt(1, ((Src1.AggregateVal[_i].DoubleVal ==
						   Src1.AggregateVal[_i].DoubleVal) &&
						  (Src2.AggregateVal[_i].DoubleVal ==
						   Src2.AggregateVal[_i].DoubleVal)));
		}
	} else if (Ty->isFloatTy())
		Dest.IntVal = APInt(
			1, (Src1.FloatVal == Src1.FloatVal && Src2.FloatVal == Src2.FloatVal));
	else {
		Dest.IntVal = APInt(
			1, (Src1.DoubleVal == Src1.DoubleVal && Src2.DoubleVal == Src2.DoubleVal));
	}
	return Dest;
}

static GenericValue executeFCMP_UNO(GenericValue Src1, GenericValue Src2, Type *Ty)
{
	GenericValue Dest;
	if (Ty->isVectorTy()) {
		assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
		Dest.AggregateVal.resize(Src1.AggregateVal.size());
		if (dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy()) {
			for (size_t _i = 0; _i < Src1.AggregateVal.size(); _i++)
				Dest.AggregateVal[_i].IntVal =
					APInt(1, ((Src1.AggregateVal[_i].FloatVal !=
						   Src1.AggregateVal[_i].FloatVal) ||
						  (Src2.AggregateVal[_i].FloatVal !=
						   Src2.AggregateVal[_i].FloatVal)));
		} else {
			for (size_t _i = 0; _i < Src1.AggregateVal.size(); _i++)
				Dest.AggregateVal[_i].IntVal =
					APInt(1, ((Src1.AggregateVal[_i].DoubleVal !=
						   Src1.AggregateVal[_i].DoubleVal) ||
						  (Src2.AggregateVal[_i].DoubleVal !=
						   Src2.AggregateVal[_i].DoubleVal)));
		}
	} else if (Ty->isFloatTy())
		Dest.IntVal = APInt(
			1, (Src1.FloatVal != Src1.FloatVal || Src2.FloatVal != Src2.FloatVal));
	else {
		Dest.IntVal = APInt(
			1, (Src1.DoubleVal != Src1.DoubleVal || Src2.DoubleVal != Src2.DoubleVal));
	}
	return Dest;
}

static GenericValue executeFCMP_BOOL(GenericValue Src1, GenericValue Src2, const Type *Ty,
				     const bool val)
{
	GenericValue Dest;
	if (Ty->isVectorTy()) {
		assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
		Dest.AggregateVal.resize(Src1.AggregateVal.size());
		for (size_t _i = 0; _i < Src1.AggregateVal.size(); _i++)
			Dest.AggregateVal[_i].IntVal = APInt(1, val);
	} else {
		Dest.IntVal = APInt(1, val);
	}

	return Dest;
}

void Interpreter::visitFCmpInst(FCmpInst &I)
{
	ExecutionContext &SF = ECStack().back();
	Type *Ty = I.getOperand(0)->getType();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue R; // Result

	switch (I.getPredicate()) {
	default:
		dbgs() << "Don't know how to handle this FCmp predicate!\n-->" << I;
		llvm_unreachable(nullptr);
		break;
	case FCmpInst::FCMP_FALSE:
		R = executeFCMP_BOOL(Src1, Src2, Ty, false);
		break;
	case FCmpInst::FCMP_TRUE:
		R = executeFCMP_BOOL(Src1, Src2, Ty, true);
		break;
	case FCmpInst::FCMP_ORD:
		R = executeFCMP_ORD(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_UNO:
		R = executeFCMP_UNO(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_UEQ:
		R = executeFCMP_UEQ(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_OEQ:
		R = executeFCMP_OEQ(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_UNE:
		R = executeFCMP_UNE(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_ONE:
		R = executeFCMP_ONE(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_ULT:
		R = executeFCMP_ULT(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_OLT:
		R = executeFCMP_OLT(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_UGT:
		R = executeFCMP_UGT(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_OGT:
		R = executeFCMP_OGT(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_ULE:
		R = executeFCMP_ULE(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_OLE:
		R = executeFCMP_OLE(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_UGE:
		R = executeFCMP_UGE(Src1, Src2, Ty);
		break;
	case FCmpInst::FCMP_OGE:
		R = executeFCMP_OGE(Src1, Src2, Ty);
		break;
	}

	SetValue(&I, R, SF);
}

#if LLVM_VERSION_MAJOR < 19
static GenericValue executeCmpInst(unsigned predicate, GenericValue Src1, GenericValue Src2,
				   Type *Ty)
{
	GenericValue Result;
	switch (predicate) {
	case ICmpInst::ICMP_EQ:
		return executeICMP_EQ(Src1, Src2, Ty);
	case ICmpInst::ICMP_NE:
		return executeICMP_NE(Src1, Src2, Ty);
	case ICmpInst::ICMP_UGT:
		return executeICMP_UGT(Src1, Src2, Ty);
	case ICmpInst::ICMP_SGT:
		return executeICMP_SGT(Src1, Src2, Ty);
	case ICmpInst::ICMP_ULT:
		return executeICMP_ULT(Src1, Src2, Ty);
	case ICmpInst::ICMP_SLT:
		return executeICMP_SLT(Src1, Src2, Ty);
	case ICmpInst::ICMP_UGE:
		return executeICMP_UGE(Src1, Src2, Ty);
	case ICmpInst::ICMP_SGE:
		return executeICMP_SGE(Src1, Src2, Ty);
	case ICmpInst::ICMP_ULE:
		return executeICMP_ULE(Src1, Src2, Ty);
	case ICmpInst::ICMP_SLE:
		return executeICMP_SLE(Src1, Src2, Ty);
	case FCmpInst::FCMP_ORD:
		return executeFCMP_ORD(Src1, Src2, Ty);
	case FCmpInst::FCMP_UNO:
		return executeFCMP_UNO(Src1, Src2, Ty);
	case FCmpInst::FCMP_OEQ:
		return executeFCMP_OEQ(Src1, Src2, Ty);
	case FCmpInst::FCMP_UEQ:
		return executeFCMP_UEQ(Src1, Src2, Ty);
	case FCmpInst::FCMP_ONE:
		return executeFCMP_ONE(Src1, Src2, Ty);
	case FCmpInst::FCMP_UNE:
		return executeFCMP_UNE(Src1, Src2, Ty);
	case FCmpInst::FCMP_OLT:
		return executeFCMP_OLT(Src1, Src2, Ty);
	case FCmpInst::FCMP_ULT:
		return executeFCMP_ULT(Src1, Src2, Ty);
	case FCmpInst::FCMP_OGT:
		return executeFCMP_OGT(Src1, Src2, Ty);
	case FCmpInst::FCMP_UGT:
		return executeFCMP_UGT(Src1, Src2, Ty);
	case FCmpInst::FCMP_OLE:
		return executeFCMP_OLE(Src1, Src2, Ty);
	case FCmpInst::FCMP_ULE:
		return executeFCMP_ULE(Src1, Src2, Ty);
	case FCmpInst::FCMP_OGE:
		return executeFCMP_OGE(Src1, Src2, Ty);
	case FCmpInst::FCMP_UGE:
		return executeFCMP_UGE(Src1, Src2, Ty);
	case FCmpInst::FCMP_FALSE:
		return executeFCMP_BOOL(Src1, Src2, Ty, false);
	case FCmpInst::FCMP_TRUE:
		return executeFCMP_BOOL(Src1, Src2, Ty, true);
	default:
		dbgs() << "Unhandled Cmp predicate\n";
		llvm_unreachable(nullptr);
	}
}
#endif

void Interpreter::visitBinaryOperator(BinaryOperator &I)
{
	ExecutionContext &SF = ECStack().back();
	Type *Ty = I.getOperand(0)->getType();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue R; // Result

	/* Update dependencies */
	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	updateDataDeps(getCurThr().id, &I, I.getOperand(1));

	// First process vector operation
	if (Ty->isVectorTy()) {
		assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
		R.AggregateVal.resize(Src1.AggregateVal.size());

		// Macros to execute binary operation 'OP' over integer vectors
#define INTEGER_VECTOR_OPERATION(OP)                                                               \
	for (unsigned i = 0; i < R.AggregateVal.size(); ++i)                                       \
		R.AggregateVal[i].IntVal =                                                         \
			Src1.AggregateVal[i].IntVal OP Src2.AggregateVal[i].IntVal;

		// Additional macros to execute binary operations udiv/sdiv/urem/srem since
		// they have different notation.
#define INTEGER_VECTOR_FUNCTION(OP)                                                                \
	for (unsigned i = 0; i < R.AggregateVal.size(); ++i)                                       \
		R.AggregateVal[i].IntVal =                                                         \
			Src1.AggregateVal[i].IntVal.OP(Src2.AggregateVal[i].IntVal);

		// Macros to execute binary operation 'OP' over floating point type TY
		// (float or double) vectors
#define FLOAT_VECTOR_FUNCTION(OP, TY)                                                              \
	for (unsigned i = 0; i < R.AggregateVal.size(); ++i)                                       \
		R.AggregateVal[i].TY = Src1.AggregateVal[i].TY OP Src2.AggregateVal[i].TY;

		// Macros to choose appropriate TY: float or double and run operation
		// execution
#define FLOAT_VECTOR_OP(OP)                                                                        \
	{                                                                                          \
		if (dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy())                       \
			FLOAT_VECTOR_FUNCTION(OP, FloatVal)                                        \
		else {                                                                             \
			if (dyn_cast<VectorType>(Ty)->getElementType()->isDoubleTy())              \
				FLOAT_VECTOR_FUNCTION(OP, DoubleVal)                               \
			else {                                                                     \
				dbgs() << "Unhandled type for OP instruction: " << *Ty << "\n";    \
				llvm_unreachable(0);                                               \
			}                                                                          \
		}                                                                                  \
	}

		switch (I.getOpcode()) {
		default:
			dbgs() << "Don't know how to handle this binary operator!\n-->" << I;
			llvm_unreachable(nullptr);
			break;
		case Instruction::Add:
			INTEGER_VECTOR_OPERATION(+) break;
		case Instruction::Sub:
			INTEGER_VECTOR_OPERATION(-) break;
		case Instruction::Mul:
			INTEGER_VECTOR_OPERATION(*) break;
		case Instruction::UDiv:
			INTEGER_VECTOR_FUNCTION(udiv) break;
		case Instruction::SDiv:
			INTEGER_VECTOR_FUNCTION(sdiv) break;
		case Instruction::URem:
			INTEGER_VECTOR_FUNCTION(urem) break;
		case Instruction::SRem:
			INTEGER_VECTOR_FUNCTION(srem) break;
		case Instruction::And:
			INTEGER_VECTOR_OPERATION(&) break;
		case Instruction::Or:
			INTEGER_VECTOR_OPERATION(|) break;
		case Instruction::Xor:
			INTEGER_VECTOR_OPERATION(^) break;
		case Instruction::FAdd:
			FLOAT_VECTOR_OP(+) break;
		case Instruction::FSub:
			FLOAT_VECTOR_OP(-) break;
		case Instruction::FMul:
			FLOAT_VECTOR_OP(*) break;
		case Instruction::FDiv:
			FLOAT_VECTOR_OP(/) break;
		case Instruction::FRem:
			if (dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy())
				for (unsigned i = 0; i < R.AggregateVal.size(); ++i)
					R.AggregateVal[i].FloatVal =
						fmod(Src1.AggregateVal[i].FloatVal,
						     Src2.AggregateVal[i].FloatVal);
			else {
				if (dyn_cast<VectorType>(Ty)->getElementType()->isDoubleTy())
					for (unsigned i = 0; i < R.AggregateVal.size(); ++i)
						R.AggregateVal[i].DoubleVal =
							fmod(Src1.AggregateVal[i].DoubleVal,
							     Src2.AggregateVal[i].DoubleVal);
				else {
					dbgs() << "Unhandled type for Rem instruction: " << *Ty
					       << "\n";
					llvm_unreachable(nullptr);
				}
			}
			break;
		}
	} else {
		switch (I.getOpcode()) {
		default:
			dbgs() << "Don't know how to handle this binary operator!\n-->" << I;
			llvm_unreachable(nullptr);
			break;
		case Instruction::Add:
			R.IntVal = Src1.IntVal + Src2.IntVal;
			break;
		case Instruction::Sub:
			R.IntVal = Src1.IntVal - Src2.IntVal;
			break;
		case Instruction::Mul:
			R.IntVal = Src1.IntVal * Src2.IntVal;
			break;
		case Instruction::FAdd:
			executeFAddInst(R, Src1, Src2, Ty);
			break;
		case Instruction::FSub:
			executeFSubInst(R, Src1, Src2, Ty);
			break;
		case Instruction::FMul:
			executeFMulInst(R, Src1, Src2, Ty);
			break;
		case Instruction::FDiv:
			executeFDivInst(R, Src1, Src2, Ty);
			break;
		case Instruction::FRem:
			executeFRemInst(R, Src1, Src2, Ty);
			break;
		case Instruction::UDiv:
			R.IntVal = Src1.IntVal.udiv(Src2.IntVal);
			break;
		case Instruction::SDiv:
			R.IntVal = Src1.IntVal.sdiv(Src2.IntVal);
			break;
		case Instruction::URem:
			R.IntVal = Src1.IntVal.urem(Src2.IntVal);
			break;
		case Instruction::SRem:
			R.IntVal = Src1.IntVal.srem(Src2.IntVal);
			break;
		case Instruction::And:
			R.IntVal = Src1.IntVal & Src2.IntVal;
			break;
		case Instruction::Or:
			R.IntVal = Src1.IntVal | Src2.IntVal;
			break;
		case Instruction::Xor:
			R.IntVal = Src1.IntVal ^ Src2.IntVal;
			break;
		}
	}
	SetValue(&I, R, SF);
}

static GenericValue executeSelectInst(GenericValue Src1, GenericValue Src2, GenericValue Src3,
				      const Type *Ty)
{
	GenericValue Dest;
	if (Ty->isVectorTy()) {
		assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
		assert(Src2.AggregateVal.size() == Src3.AggregateVal.size());
		Dest.AggregateVal.resize(Src1.AggregateVal.size());
		for (size_t i = 0; i < Src1.AggregateVal.size(); ++i)
			Dest.AggregateVal[i] = (Src1.AggregateVal[i].IntVal == 0)
						       ? Src3.AggregateVal[i]
						       : Src2.AggregateVal[i];
	} else {
		Dest = (Src1.IntVal == 0) ? Src3 : Src2;
	}
	return Dest;
}

void Interpreter::visitSelectInst(SelectInst &I)
{
	ExecutionContext &SF = ECStack().back();
	const Type *Ty = I.getOperand(0)->getType();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Src3 = getOperandValue(I.getOperand(2), SF);
	GenericValue R = executeSelectInst(Src1, Src2, Src3, Ty);
	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	updateDataDeps(getCurThr().id, &I, I.getOperand(1));
	updateDataDeps(getCurThr().id, &I, I.getOperand(2));
	SetValue(&I, R, SF);
}

//===----------------------------------------------------------------------===//
//                     Terminator Instruction Implementations
//===----------------------------------------------------------------------===//

void Interpreter::freeAllocas(const AllocaHolder &allocas)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	for (auto it = allocas.get().begin(), ie = allocas.get().end(); it != ie; ++it)
		CALL_DRIVER(handleFree, currDbgInfo(), currPos(), *it, GET_DEPS(deps));
}

void Interpreter::exitCalled(GenericValue GV)
{
	// runAtExitHandlers() assumes there are no stack frames, but
	// if exit() was called, then it had a stack frame. Blow away
	// the stack before interpreting atexit handlers.
	WARN_ONCE("exit-called", "Usage of exit() is not thread-safe!\n");
	while (ECStack().size() > 0) {
		freeAllocas(ECStack().back().Allocas);
		ECStack().pop_back(); /* FIXME: Now assumes the user has properly used it */
	}
	runAtExitHandlers();
	// exit(GV.IntVal.zextOrTrunc(32).getZExtValue());
}

/// Pop the last stack frame off of ECStack and then copy the result
/// back into the result variable if we are not returning void. The
/// result variable may be the ExitValue, or the Value of the calling
/// CallInst if there was a previous stack frame. This method may
/// invalidate any ECStack iterators you have. This method also takes
/// care of switching to the normal destination BB, if we are returning
/// from an invoke.
///
void Interpreter::popStackAndReturnValueToCaller(Type *RetTy, GenericValue Result,
						 ReturnInst *retI /* = nullptr */)
{
	// Keep track of the ret inst to update deps if necessary:
	// We check whether there _was_ an instruction which caused the
	// stack frame to be popped, and that the instruction was a ret inst
	// (That's why we have an extra parameter.)

	// Pop the current stack frame.
	freeAllocas(ECStack().back().Allocas);
	ECStack().pop_back();

	// if (ECStack.empty()) {  // Finished main.  Put result into exit code...
	//   if (RetTy && !RetTy->isVoidTy()) {          // Nonvoid return type?
	//     ExitValue = Result;   // Capture the exit value of the program
	//   } else {
	//     memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
	//   }
	if (ECStack().empty()) {
		if (getCurThr().isMain() && getProgramState() == ProgramState::Main)
			runAtExitHandlers();
		if (getProgramState() != ProgramState::Dtors)
			CALL_DRIVER(handleThreadFinish, currDbgInfo(), currPos(),
				    GV_TO_SVAL(Result, RetTy));
	} else {
		// If we have a previous stack frame, and we have a previous call,
		// fill in the return value...
		ExecutionContext &CallingSF = ECStack().back();
		if (Instruction *I = &CallingSF.Caller) {
			// Save result...
			if (!(&CallingSF.Caller)->getType()->isVoidTy()) {
				if (retI) // if we are coming from a ret inst, update deps
					updateDataDeps(getCurThr().id, I, retI->getReturnValue());
				SetValue(I, Result, CallingSF);
			}
			if (InvokeInst *II = dyn_cast<InvokeInst>(I))
				SwitchToNewBasicBlock(II->getNormalDest(), CallingSF);
			CallingSF.Caller = CallInstWrapper(); // We returned from the call...
		}
	}
}

void Interpreter::returnValueToCaller(Type *RetTy, GenericValue Result)
{
	assert(!ECStack().empty());
	// fill in the return value...
	ExecutionContext &CallingSF = ECStack().back();
	if (Instruction *I = &CallingSF.Caller) {
		// Save result...
		if (!(&CallingSF.Caller)->getType()->isVoidTy())
			SetValue(I, Result, CallingSF);
		if (InvokeInst *II = dyn_cast<InvokeInst>(I))
			SwitchToNewBasicBlock(II->getNormalDest(), CallingSF);
		CallingSF.Caller = CallInstWrapper(); // We returned from the call...
	}
}

void Interpreter::visitReturnInst(ReturnInst &I)
{
	ExecutionContext &SF = ECStack().back();
	Type *RetTy = Type::getVoidTy(I.getContext());
	GenericValue Result;

	// Save away the return value... (if we are not 'ret void')
	if (I.getNumOperands()) {
		RetTy = I.getReturnValue()->getType();
		Result = getOperandValue(I.getReturnValue(), SF);
	}

	popStackAndReturnValueToCaller(RetTy, Result, &I);
}

void Interpreter::visitUnreachableInst(UnreachableInst &I)
{
	report_fatal_error("Program executed an 'unreachable' instruction!");
}

void Interpreter::visitBranchInst(BranchInst &I)
{
	ExecutionContext &SF = ECStack().back();
	BasicBlock *Dest;

	Dest = I.getSuccessor(0); // Uncond branches have a fixed dest...
	if (!I.isUnconditional()) {
		Value *Cond = I.getCondition();
		if (getOperandValue(Cond, SF).IntVal == 0) // If false cond...
			Dest = I.getSuccessor(1);
		updateCtrlDeps(getCurThr().id, Cond);
	}
	SwitchToNewBasicBlock(Dest, SF);
}

void Interpreter::visitSwitchInst(SwitchInst &I)
{
	ExecutionContext &SF = ECStack().back();
	Value *Cond = I.getCondition();
	Type *ElTy = Cond->getType();
	GenericValue CondVal = getOperandValue(Cond, SF);

	// Check to see if any of the cases match...
	BasicBlock *Dest = nullptr;
	for (SwitchInst::CaseIt i = I.case_begin(), e = I.case_end(); i != e; ++i) {
		GenericValue CaseVal = getOperandValue(i->getCaseValue(), SF);
		if (executeICMP_EQ(CondVal, CaseVal, ElTy).IntVal != 0) {
			Dest = cast<BasicBlock>(i->getCaseSuccessor());
			break;
		}
	}
	if (!Dest)
		Dest = I.getDefaultDest(); // No cases matched: use default
	updateCtrlDeps(getCurThr().id, Cond);
	SwitchToNewBasicBlock(Dest, SF);
}

void Interpreter::visitIndirectBrInst(IndirectBrInst &I)
{
	ExecutionContext &SF = ECStack().back();
	void *Dest = GVTOP(getOperandValue(I.getAddress(), SF));
	updateCtrlDeps(getCurThr().id, I.getAddress());
	SwitchToNewBasicBlock((BasicBlock *)Dest, SF);
}

// SwitchToNewBasicBlock - This method is used to jump to a new basic block.
// This function handles the actual updating of block and instruction iterators
// as well as execution of all of the PHI nodes in the destination block.
//
// This method does this because all of the PHI nodes must be executed
// atomically, reading their inputs before any of the results are updated.  Not
// doing this can cause problems if the PHI nodes depend on other PHI nodes for
// their inputs.  If the input PHI node is updated before it is read, incorrect
// results can happen.  Thus we use a two phase approach.
//
void Interpreter::SwitchToNewBasicBlock(BasicBlock *Dest, ExecutionContext &SF)
{
	BasicBlock *PrevBB = SF.CurBB;	// Remember where we came from...
	SF.CurBB = Dest;		// Update CurBB to branch destination
	SF.CurInst = SF.CurBB->begin(); // Update new instruction ptr...

	if (!isa<PHINode>(SF.CurInst))
		return; // Nothing fancy to do

	// Loop over all of the PHI nodes in the current block, reading their inputs.
	std::vector<GenericValue> ResultValues;

	for (; PHINode *PN = dyn_cast<PHINode>(SF.CurInst); ++SF.CurInst) {
		// Search for the value corresponding to this previous bb...
		int i = PN->getBasicBlockIndex(PrevBB);
		assert(i != -1 && "PHINode doesn't contain entry for predecessor??");
		Value *IncomingValue = PN->getIncomingValue(i);

		// Save the incoming value for this PHI node...
		ResultValues.push_back(getOperandValue(IncomingValue, SF));
		updateDataDeps(getCurThr().id, PN, IncomingValue);
	}

	// Now loop over all of the PHI nodes setting their values...
	SF.CurInst = SF.CurBB->begin();
	for (unsigned i = 0; isa<PHINode>(SF.CurInst); ++SF.CurInst, ++i) {
		PHINode *PN = cast<PHINode>(SF.CurInst);
		SetValue(PN, ResultValues[i], SF);
	}
}

//===----------------------------------------------------------------------===//
//                     Memory Instruction Implementations
//===----------------------------------------------------------------------===//

void Interpreter::visitAllocaInst(AllocaInst &I)
{
	ExecutionContext &SF = ECStack().back();

	Type *Ty = I.getAllocatedType(); // Type to be allocated

	// Get the number of elements being allocated by the array...
	unsigned NumElements = getOperandValue(I.getOperand(0), SF).IntVal.getZExtValue();

	unsigned TypeSize = (size_t)getDataLayout().getTypeAllocSize(Ty);

	// Avoid malloc-ing zero bytes, use max()...
	unsigned MemToAlloc = std::max(1U, NumElements * TypeSize);

	/* The driver will provide the address this alloca returns */
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	auto *info = getVarNameInfo(&I, StorageDuration::SD_Automatic, AddressSpace::AS_User);
	SVal result = CALL_DRIVER(handleMalloc, currDbgInfo(), currPos(), MemToAlloc,
				  I.getAlign().value(), StorageDuration::SD_Automatic,
				  StorageType::ST_Volatile, AddressSpace::AS_User, info,
				  I.getName().str(), GET_DEPS(deps));

	if (dynState.collectDbgInfo && info)
		dynState.nameInfo.insert(SAddr(result.get()), SAddr(result.get()) + MemToAlloc,
					 std::make_pair(I.getName().str(), info));
	ECStack().back().Allocas.add((void *)result.get());

	updateDataDeps(getCurThr().id, &I, currPos());
	SetValue(&I, SVAL_TO_GV(result, I.getType()), SF);
}

// getElementOffset - The workhorse for getelementptr.
//
GenericValue Interpreter::executeGEPOperation(Value *Ptr, gep_type_iterator I, gep_type_iterator E,
					      ExecutionContext &SF)
{
	assert(Ptr->getType()->isPointerTy() && "Cannot getElementOffset of a nonpointer type!");

	updateDataDeps(getCurThr().id, SF.CurInst->getPrevNode(), Ptr);

	uint64_t Total = 0;
	for (; I != E; ++I) {
		updateDataDeps(getCurThr().id, SF.CurInst->getPrevNode(), I.getOperand());
		if (StructType *STy = I.getStructTypeOrNull()) {
			const StructLayout *SLO = getDataLayout().getStructLayout(STy);

			const ConstantInt *CPU = cast<ConstantInt>(I.getOperand());
			unsigned Index = unsigned(CPU->getZExtValue());

			Total += SLO->getElementOffset(Index);
		} else {
			// Get the index number for the array... which must be long type...
			GenericValue IdxGV = getOperandValue(I.getOperand(), SF);

			int64_t Idx;
			unsigned BitWidth =
				cast<IntegerType>(I.getOperand()->getType())->getBitWidth();
			if (BitWidth == 32)
				Idx = (int64_t)(int32_t)IdxGV.IntVal.getZExtValue();
			else {
				assert(BitWidth == 64 && "Invalid index type for getelementptr");
				Idx = (int64_t)IdxGV.IntVal.getZExtValue();
			}
			Total += getDataLayout().getTypeAllocSize(I.getIndexedType()) * Idx;
		}
	}

	GenericValue Result;
	Result.PointerVal = ((char *)getOperandValue(Ptr, SF).PointerVal) + Total;
	// DEBUG(dbgs() << "GEP Index " << Total << " bytes.\n");
	return Result;
}

void Interpreter::visitGetElementPtrInst(GetElementPtrInst &I)
{
	ExecutionContext &SF = ECStack().back();
	SetValue(&I,
		 executeGEPOperation(I.getPointerOperand(), gep_type_begin(I), gep_type_end(I), SF),
		 SF);
}

void Interpreter::visitLoadInst(LoadInst &I)
{
	Thread &thr = getCurThr();
	GenericValue src = getOperandValue(I.getPointerOperand(), ECStack().back());
	GenericValue *ptr = (GenericValue *)GVTOP(src);
	Type *typ = I.getType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);
	auto ord = toGenMCOrdering(I.getOrdering());

	/* If this is a thread-local access it is not recorded in the graph,
	 * so just perform the load. */
	if (thr.tls.count(ptr)) {
		SetValue(&I, thr.tls[ptr], ECStack().back());
		return;
	}

	/* Otherwise, set the dependencies for this instruction.. */
	auto deps = makeEventDeps(getDataDeps(thr.id, I.getPointerOperand()), nullptr,
				  getCtrlDeps(thr.id), getAddrPoDeps(thr.id), nullptr);

	/* ... and then the driver will provide the appropriate value */
#define IMPLEMENT_READ_VISIT(__kind)                                                               \
	case EventLabel::EventLabelKind::__kind: {                                                 \
		val = CALL_DRIVER_RESET_IF_NONE(                                                   \
			handleLoad<EventLabel::EventLabelKind::__kind>, currDbgInfo(ptr),          \
			currPos(), std::nullopt, toGenMCOrdering(I.getOrdering()), ptr, size,      \
			atyp, nullptr, getCurrentAnnotConcretized(), GET_DEPS(deps));              \
		break;                                                                             \
	}

	GenMCDriver::HandleResult<SVal> val;
	switch (getReadKind(I)) {
		IMPLEMENT_READ_VISIT(Read);
		IMPLEMENT_READ_VISIT(SpeculativeRead);
		IMPLEMENT_READ_VISIT(ConfirmingRead);
		IMPLEMENT_READ_VISIT(BWaitRead);
	default:
		BUG();
	}

	auto retVal = std::get_if<SVal>(&val);
	if (!retVal)
		return;

	updateDataDeps(thr.id, &I, currPos());
	updateAddrPoDeps(thr.id, I.getPointerOperand());

	/* Last, set the return value for this instruction */
	SetValue(&I, SVAL_TO_GV(*retVal, typ), ECStack().back());
	return;
}

void Interpreter::visitStoreInst(StoreInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	Type *typ = I.getOperand(0)->getType();
	GenericValue val = getOperandValue(I.getOperand(0), SF);
	auto src = getOperandValue(I.getPointerOperand(), SF);
	auto *ptr = (GenericValue *)GVTOP(src);
	auto asize = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);
	auto ord = toGenMCOrdering(I.getOrdering());

	/* Do not bother with thread-local accesses */
	if (thr.tls.count(ptr)) {
		thr.tls[ptr] = val;
		return;
	}

	/* First, record the dependencies for this instruction */
	auto deps = makeEventDeps(getDataDeps(thr.id, I.getPointerOperand()),
				  getDataDeps(thr.id, I.getOperand(0)), getCtrlDeps(thr.id),
				  getAddrPoDeps(thr.id), nullptr);

	/* Inform the Driver about the newly interpreter store */
	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::Write>, currDbgInfo(ptr), currPos(),
		    std::nullopt, ord, ptr, asize, atyp, GV_TO_SVAL(val, typ), getWriteAttr(I),
		    GET_DEPS(deps));

	updateAddrPoDeps(getCurThr().id, I.getPointerOperand());
	return;
}

void Interpreter::visitFenceInst(FenceInst &I)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id), nullptr, nullptr);
	CALL_DRIVER(handleFence, currDbgInfo(), currPos(), toGenMCOrdering(I.getOrdering()),
		    GET_DEPS(deps));
}

void Interpreter::visitAtomicCmpXchgInst(AtomicCmpXchgInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	Type *typ = I.getCompareOperand()->getType();
	GenericValue cmpVal = getOperandValue(I.getCompareOperand(), SF);
	GenericValue newVal = getOperandValue(I.getNewValOperand(), SF);
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	auto *ptr = (GenericValue *)GVTOP(src);
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);
	GenericValue result;

	if (thr.tls.count(ptr)) {
		GenericValue oldVal = thr.tls[ptr];
		GenericValue cmpRes = executeICMP_EQ(oldVal, cmpVal, typ);
		if (cmpRes.IntVal.getBoolValue())
			thr.tls[ptr] = newVal;
		result.AggregateVal.push_back(oldVal);
		result.AggregateVal.push_back(cmpRes);
		SetValue(&I, result, SF);
		return;
	}

	auto lDeps = makeEventDeps(getDataDeps(thr.id, I.getPointerOperand()),
				   getDataDeps(thr.id, I.getNewValOperand()), getCtrlDeps(thr.id),
				   getAddrPoDeps(thr.id),
				   getDataDeps(thr.id, I.getCompareOperand()));

	GenMCDriver::HandleResult<SVal> ret;
	SVal *retVal = nullptr;
	int cmpRes{};

#define IMPLEMENT_CAS_VISIT(__kindR, __kindW)                                                      \
	case switchPair(__kindR, __kindW): {                                                       \
		ret = CALL_DRIVER_RESET_IF_NONE(                                                   \
			handleLoad<__kindR>, currDbgInfo(ptr), currPos(), std::nullopt,            \
			toGenMCOrdering(I.getSuccessOrdering()), ptr, size, atyp,                  \
			GV_TO_SVAL(cmpVal, typ), GV_TO_SVAL(newVal, typ), getWriteAttr(I),         \
			nullptr, getCurrentAnnotConcretized(), GET_DEPS(lDeps));                   \
                                                                                                   \
		retVal = std::get_if<SVal>(&ret);                                                  \
		if (!retVal)                                                                       \
			return;                                                                    \
		cmpRes = *retVal == GV_TO_SVAL(cmpVal, typ);                                       \
		updateDataDeps(getCurThr().id, &I, currPos());                                     \
		updateAddrPoDeps(getCurThr().id, I.getPointerOperand());                           \
		if (!cmpRes)                                                                       \
			break;                                                                     \
		auto sDeps = makeEventDeps(getDataDeps(getCurThr().id, I.getPointerOperand()),     \
					   getDataDeps(getCurThr().id, I.getNewValOperand()),      \
					   getCtrlDeps(getCurThr().id), getAddrPoDeps(thr.id),     \
					   nullptr);                                               \
		CALL_DRIVER(handleStore<__kindW>, currDbgInfo(ptr), currPos(), std::nullopt,       \
			    toGenMCOrdering(I.getSuccessOrdering()), ptr, size, atyp,              \
			    GV_TO_SVAL(newVal, typ), getWriteAttr(I), GET_DEPS(sDeps));            \
		break;                                                                             \
	}

	switch (switchPair(getCasKinds(I))) {
		IMPLEMENT_CAS_VISIT(EventLabel::EventLabelKind::CasRead,
				    EventLabel::EventLabelKind::CasWrite);
		IMPLEMENT_CAS_VISIT(EventLabel::EventLabelKind::HelpedCasRead,
				    EventLabel::EventLabelKind::HelpedCasWrite);
		IMPLEMENT_CAS_VISIT(EventLabel::EventLabelKind::ConfirmingCasRead,
				    EventLabel::EventLabelKind::ConfirmingCasWrite);
	case switchPair(EventLabel::EventLabelKind::HelpingCas,
			EventLabel::EventLabelKind::HelpingCas): {
		if (!CALL_DRIVER_RESET_IF_FALSE(handleHelpingCas, currDbgInfo(ptr), currPos(),
						toGenMCOrdering(I.getSuccessOrdering()), ptr, size,
						atyp, GV_TO_SVAL(cmpVal, typ),
						GV_TO_SVAL(newVal, typ), GET_DEPS(lDeps)))
			return;
		ret = {SVal(0)}; // Dummy value, the result is not used
		retVal = std::get_if<SVal>(&ret);
		break;
	}
	default:
		BUG();
	}

	result.AggregateVal.push_back(SVAL_TO_GV(*retVal, typ));
	result.AggregateVal.push_back(INT_TO_GV(Type::getInt1Ty(I.getContext()), cmpRes));
	SetValue(&I, result, ECStack().back());
}

void Interpreter::visitAtomicRMWInst(AtomicRMWInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	auto *ptr = (GenericValue *)GVTOP(src);
	Type *typ = I.getType();
	auto val = GV_TO_SVAL(getOperandValue(I.getValOperand(), SF), typ);
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	BUG_ON(!typ->isIntegerTy());

	WARN_ON_ONCE(I.getOperation() == AtomicRMWInst::BinOp::Xchg && getDepTracker(),
		     "unsupported-xchg-deps",
		     "Atomic xchg support is experimental under dependency-tracking models!");

	if (thr.tls.count(ptr)) {
		GenericValue oldVal = thr.tls[ptr];
		auto newVal = executeRMWBinOp(GV_TO_SVAL(oldVal, typ), val, size,
					      toGenMCBinOp(I.getOperation()));
		thr.tls[ptr] = SVAL_TO_GV(newVal, typ);
		SetValue(&I, oldVal, SF);
		return;
	}

	auto deps = makeEventDeps(getDataDeps(thr.id, I.getPointerOperand()),
				  getDataDeps(thr.id, I.getValOperand()), getCtrlDeps(thr.id),
				  getAddrPoDeps(thr.id), nullptr);

#define IMPLEMENT_FAI_VISIT(__kindR, __kindW)                                                      \
	case switchPair(__kindR, __kindW): {                                                       \
		ret = CALL_DRIVER_RESET_IF_NONE(handleLoad<__kindR>, currDbgInfo(ptr), currPos(),  \
						std::nullopt, toGenMCOrdering(I.getOrdering()),    \
						ptr, size, atyp, toGenMCBinOp(I.getOperation()),   \
						val, getWriteAttr(I), GET_DEPS(deps));             \
                                                                                                   \
		retVal = std::get_if<SVal>(&ret);                                                  \
		if (!retVal)                                                                       \
			return;                                                                    \
		updateDataDeps(getCurThr().id, &I, currPos());                                     \
		updateAddrPoDeps(getCurThr().id, I.getPointerOperand());                           \
		auto newVal = executeRMWBinOp(*retVal, val, size, toGenMCBinOp(I.getOperation())); \
		CALL_DRIVER(handleStore<__kindW>, currDbgInfo(ptr), currPos(), std::nullopt,       \
			    toGenMCOrdering(I.getOrdering()), ptr, size, atyp, newVal,             \
			    getWriteAttr(I), GET_DEPS(deps));                                      \
		break;                                                                             \
	}

	GenMCDriver::HandleResult<SVal> ret;
	SVal *retVal = nullptr;
	switch (switchPair(getFaiKinds(I))) {
		IMPLEMENT_FAI_VISIT(EventLabel::EventLabelKind::FaiRead,
				    EventLabel::EventLabelKind::FaiWrite);
		IMPLEMENT_FAI_VISIT(EventLabel::EventLabelKind::NoRetFaiRead,
				    EventLabel::EventLabelKind::NoRetFaiWrite);
		IMPLEMENT_FAI_VISIT(EventLabel::EventLabelKind::BIncFaiRead,
				    EventLabel::EventLabelKind::BIncFaiWrite);
	default:
		BUG();
	}

	SetValue(&I, SVAL_TO_GV(*retVal, typ), ECStack().back());
}

bool Interpreter::isInlineAsm(CallInstWrapper CIW, std::string *asmStr)
{
	llvm::CallInst *CI = dyn_cast<CallInst>(&CIW);
	if (!CI || !CI->isInlineAsm())
		return false;

	llvm::InlineAsm *IA = llvm::dyn_cast<llvm::InlineAsm>(CIW.getCalledOperand());
	*asmStr = IA->getAsmString();
	asmStr->erase(asmStr->begin(), std::find_if(asmStr->begin(), asmStr->end(),
						    [](int c) { return !std::isspace(c); }));
	asmStr->erase(std::find_if(asmStr->rbegin(), asmStr->rend(),
				   [](int c) { return !std::isspace(c); })
			      .base(),
		      asmStr->end());
	return true;
}

void Interpreter::visitInlineAsm(CallInstWrapper CIW, const std::string &asmStr)
{
	if (asmStr == "")
		; /* Plain compiler fence */
	else
		WARN_ONCE("invalid-inline-asm",
			  "Arbitrary inline assembly not supported: {}! Skipping...\n", asmStr);
	return;
}

//===----------------------------------------------------------------------===//
//                 Miscellaneous Instruction Implementations
//===----------------------------------------------------------------------===//

void Interpreter::visitCallInstWrapper(CallInstWrapper CS)
{

	std::string asmStr;
	if (isInlineAsm(CS, &asmStr)) {
		visitInlineAsm(CS, asmStr);
		return;
	}

	ExecutionContext &SF = ECStack().back();

	// Check to see if this is an intrinsic function call...
	Function *F = CS.getCalledFunction();
	if (F && F->isDeclaration())
		switch (F->getIntrinsicID()) {
		case Intrinsic::not_intrinsic:
			break;
		case Intrinsic::vastart: { // va_start
			GenericValue ArgIndex;
			ArgIndex.UIntPairVal.first = ECStack().size() - 1;
			ArgIndex.UIntPairVal.second = 0;
			SetValue(&CS, ArgIndex, SF);
			return;
		}
		case Intrinsic::vaend: // va_end is a noop for the interpreter
			return;
		case Intrinsic::vacopy: // va_copy: dest = src
			SetValue(&CS, getOperandValue(*CS.arg_begin(), SF), SF);
			return;
		default:
			WARN_ONCE("unknown-intrinsic", "Unknown intrinstic function"
						       "encountered. Attempting to lower it...\n");
			// If it is an unknown intrinsic function, use the intrinsic lowering
			// class to transform it into hopefully tasty LLVM code.
			//
			BasicBlock::iterator me(&CS);
			BasicBlock *Parent = (&CS)->getParent();
			bool atBegin(Parent->begin() == me);
			if (!atBegin)
				--me;
			IL->LowerIntrinsicCall(cast<CallInst>(&CS));

			// Restore the CurInst pointer to the first instruction newly inserted, if
			// any.
			if (atBegin) {
				SF.CurInst = Parent->begin();
			} else {
				SF.CurInst = me;
				++SF.CurInst;
			}
			return;
		}

	SF.Caller = CS;
	std::vector<GenericValue> ArgVals;
	const unsigned NumArgs = SF.Caller.arg_size();
	ArgVals.reserve(NumArgs);
	uint16_t pNum = 1;
	for (auto i = SF.Caller.arg_begin(), e = SF.Caller.arg_end(); i != e; ++i, ++pNum) {
		Value *V = *i;
		ArgVals.push_back(getOperandValue(V, SF));
	}

	// To handle indirect calls, we must get the pointer value from the argument
	// and treat it as a function pointer.
	GenericValue SRC = getOperandValue(SF.Caller.getCalledOperand(), SF);
	auto specialDeps = updateFunArgDeps(getCurThr().id, (Function *)GVTOP(SRC));
	callFunction((Function *)GVTOP(SRC), ArgVals, specialDeps);
	updateInternalFunRetDeps(getCurThr().id, (Function *)GVTOP(SRC), &CS);
}

// auxiliary function for shift operations
static unsigned getShiftAmount(uint64_t orgShiftAmount, llvm::APInt valueToShift)
{
	unsigned valueWidth = valueToShift.getBitWidth();
	if (orgShiftAmount < (uint64_t)valueWidth)
		return orgShiftAmount;
	// according to the llvm documentation, if orgShiftAmount > valueWidth,
	// the result is undfeined. but we do shift by this rule:
	return (NextPowerOf2(valueWidth - 1) - 1) & orgShiftAmount;
}

void Interpreter::visitShl(BinaryOperator &I)
{
	ExecutionContext &SF = ECStack().back();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Dest;
	const Type *Ty = I.getType();

	if (Ty->isVectorTy()) {
		uint32_t src1Size = uint32_t(Src1.AggregateVal.size());
		assert(src1Size == Src2.AggregateVal.size());
		for (unsigned i = 0; i < src1Size; i++) {
			GenericValue Result;
			uint64_t shiftAmount = Src2.AggregateVal[i].IntVal.getZExtValue();
			llvm::APInt valueToShift = Src1.AggregateVal[i].IntVal;
			Result.IntVal = valueToShift.shl(getShiftAmount(shiftAmount, valueToShift));
			Dest.AggregateVal.push_back(Result);
		}
	} else {
		// scalar
		uint64_t shiftAmount = Src2.IntVal.getZExtValue();
		llvm::APInt valueToShift = Src1.IntVal;
		Dest.IntVal = valueToShift.shl(getShiftAmount(shiftAmount, valueToShift));
	}

	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	updateDataDeps(getCurThr().id, &I, I.getOperand(1));
	SetValue(&I, Dest, SF);
}

void Interpreter::visitLShr(BinaryOperator &I)
{
	ExecutionContext &SF = ECStack().back();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Dest;
	const Type *Ty = I.getType();

	if (Ty->isVectorTy()) {
		uint32_t src1Size = uint32_t(Src1.AggregateVal.size());
		assert(src1Size == Src2.AggregateVal.size());
		for (unsigned i = 0; i < src1Size; i++) {
			GenericValue Result;
			uint64_t shiftAmount = Src2.AggregateVal[i].IntVal.getZExtValue();
			llvm::APInt valueToShift = Src1.AggregateVal[i].IntVal;
			Result.IntVal =
				valueToShift.lshr(getShiftAmount(shiftAmount, valueToShift));
			Dest.AggregateVal.push_back(Result);
		}
	} else {
		// scalar
		uint64_t shiftAmount = Src2.IntVal.getZExtValue();
		llvm::APInt valueToShift = Src1.IntVal;
		Dest.IntVal = valueToShift.lshr(getShiftAmount(shiftAmount, valueToShift));
	}

	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	updateDataDeps(getCurThr().id, &I, I.getOperand(1));
	SetValue(&I, Dest, SF);
}

void Interpreter::visitAShr(BinaryOperator &I)
{
	ExecutionContext &SF = ECStack().back();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Dest;
	const Type *Ty = I.getType();

	if (Ty->isVectorTy()) {
		size_t src1Size = Src1.AggregateVal.size();
		assert(src1Size == Src2.AggregateVal.size());
		for (unsigned i = 0; i < src1Size; i++) {
			GenericValue Result;
			uint64_t shiftAmount = Src2.AggregateVal[i].IntVal.getZExtValue();
			llvm::APInt valueToShift = Src1.AggregateVal[i].IntVal;
			Result.IntVal =
				valueToShift.ashr(getShiftAmount(shiftAmount, valueToShift));
			Dest.AggregateVal.push_back(Result);
		}
	} else {
		// scalar
		uint64_t shiftAmount = Src2.IntVal.getZExtValue();
		llvm::APInt valueToShift = Src1.IntVal;
		Dest.IntVal = valueToShift.ashr(getShiftAmount(shiftAmount, valueToShift));
	}

	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	updateDataDeps(getCurThr().id, &I, I.getOperand(1));
	SetValue(&I, Dest, SF);
}

GenericValue Interpreter::executeTruncInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);
	Type *SrcTy = SrcVal->getType();
	if (SrcTy->isVectorTy()) {
		Type *DstVecTy = DstTy->getScalarType();
		unsigned DBitWidth = cast<IntegerType>(DstVecTy)->getBitWidth();
		unsigned NumElts = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal
		Dest.AggregateVal.resize(NumElts);
		for (unsigned i = 0; i < NumElts; i++)
			Dest.AggregateVal[i].IntVal = Src.AggregateVal[i].IntVal.trunc(DBitWidth);
	} else {
		IntegerType *DITy = cast<IntegerType>(DstTy);
		unsigned DBitWidth = DITy->getBitWidth();
		Dest.IntVal = Src.IntVal.trunc(DBitWidth);
	}
	return Dest;
}

GenericValue Interpreter::executeSExtInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	const Type *SrcTy = SrcVal->getType();
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);
	if (SrcTy->isVectorTy()) {
		const Type *DstVecTy = DstTy->getScalarType();
		unsigned DBitWidth = cast<IntegerType>(DstVecTy)->getBitWidth();
		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal.
		Dest.AggregateVal.resize(size);
		for (unsigned i = 0; i < size; i++)
			Dest.AggregateVal[i].IntVal = Src.AggregateVal[i].IntVal.sext(DBitWidth);
	} else {
		const IntegerType *DITy = cast<IntegerType>(DstTy);
		unsigned DBitWidth = DITy->getBitWidth();
		Dest.IntVal = Src.IntVal.sext(DBitWidth);
	}
	return Dest;
}

GenericValue Interpreter::executeZExtInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	const Type *SrcTy = SrcVal->getType();
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);
	if (SrcTy->isVectorTy()) {
		const Type *DstVecTy = DstTy->getScalarType();
		unsigned DBitWidth = cast<IntegerType>(DstVecTy)->getBitWidth();

		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal.
		Dest.AggregateVal.resize(size);
		for (unsigned i = 0; i < size; i++)
			Dest.AggregateVal[i].IntVal = Src.AggregateVal[i].IntVal.zext(DBitWidth);
	} else {
		const IntegerType *DITy = cast<IntegerType>(DstTy);
		unsigned DBitWidth = DITy->getBitWidth();
		Dest.IntVal = Src.IntVal.zext(DBitWidth);
	}
	return Dest;
}

GenericValue Interpreter::executeFPTruncInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);

	if (isa<VectorType>(SrcVal->getType())) {
		assert(SrcVal->getType()->getScalarType()->isDoubleTy() &&
		       DstTy->getScalarType()->isFloatTy() && "Invalid FPTrunc instruction");

		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal.
		Dest.AggregateVal.resize(size);
		for (unsigned i = 0; i < size; i++)
			Dest.AggregateVal[i].FloatVal = (float)Src.AggregateVal[i].DoubleVal;
	} else {
		assert(SrcVal->getType()->isDoubleTy() && DstTy->isFloatTy() &&
		       "Invalid FPTrunc instruction");
		Dest.FloatVal = (float)Src.DoubleVal;
	}

	return Dest;
}

GenericValue Interpreter::executeFPExtInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);

	if (isa<VectorType>(SrcVal->getType())) {
		assert(SrcVal->getType()->getScalarType()->isFloatTy() &&
		       DstTy->getScalarType()->isDoubleTy() && "Invalid FPExt instruction");

		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal.
		Dest.AggregateVal.resize(size);
		for (unsigned i = 0; i < size; i++)
			Dest.AggregateVal[i].DoubleVal = (double)Src.AggregateVal[i].FloatVal;
	} else {
		assert(SrcVal->getType()->isFloatTy() && DstTy->isDoubleTy() &&
		       "Invalid FPExt instruction");
		Dest.DoubleVal = (double)Src.FloatVal;
	}

	return Dest;
}

GenericValue Interpreter::executeFPToUIInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	Type *SrcTy = SrcVal->getType();
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);

	if (isa<VectorType>(SrcTy)) {
		const Type *DstVecTy = DstTy->getScalarType();
		const Type *SrcVecTy = SrcTy->getScalarType();
		uint32_t DBitWidth = cast<IntegerType>(DstVecTy)->getBitWidth();
		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal.
		Dest.AggregateVal.resize(size);

		if (SrcVecTy->getTypeID() == Type::FloatTyID) {
			assert(SrcVecTy->isFloatingPointTy() && "Invalid FPToUI instruction");
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].IntVal = APIntOps::RoundFloatToAPInt(
					Src.AggregateVal[i].FloatVal, DBitWidth);
		} else {
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].IntVal = APIntOps::RoundDoubleToAPInt(
					Src.AggregateVal[i].DoubleVal, DBitWidth);
		}
	} else {
		// scalar
		uint32_t DBitWidth = cast<IntegerType>(DstTy)->getBitWidth();
		assert(SrcTy->isFloatingPointTy() && "Invalid FPToUI instruction");

		if (SrcTy->getTypeID() == Type::FloatTyID)
			Dest.IntVal = APIntOps::RoundFloatToAPInt(Src.FloatVal, DBitWidth);
		else {
			Dest.IntVal = APIntOps::RoundDoubleToAPInt(Src.DoubleVal, DBitWidth);
		}
	}

	return Dest;
}

GenericValue Interpreter::executeFPToSIInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	Type *SrcTy = SrcVal->getType();
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);

	if (isa<VectorType>(SrcTy)) {
		const Type *DstVecTy = DstTy->getScalarType();
		const Type *SrcVecTy = SrcTy->getScalarType();
		uint32_t DBitWidth = cast<IntegerType>(DstVecTy)->getBitWidth();
		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal
		Dest.AggregateVal.resize(size);

		if (SrcVecTy->getTypeID() == Type::FloatTyID) {
			assert(SrcVecTy->isFloatingPointTy() && "Invalid FPToSI instruction");
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].IntVal = APIntOps::RoundFloatToAPInt(
					Src.AggregateVal[i].FloatVal, DBitWidth);
		} else {
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].IntVal = APIntOps::RoundDoubleToAPInt(
					Src.AggregateVal[i].DoubleVal, DBitWidth);
		}
	} else {
		// scalar
		unsigned DBitWidth = cast<IntegerType>(DstTy)->getBitWidth();
		assert(SrcTy->isFloatingPointTy() && "Invalid FPToSI instruction");

		if (SrcTy->getTypeID() == Type::FloatTyID)
			Dest.IntVal = APIntOps::RoundFloatToAPInt(Src.FloatVal, DBitWidth);
		else {
			Dest.IntVal = APIntOps::RoundDoubleToAPInt(Src.DoubleVal, DBitWidth);
		}
	}
	return Dest;
}

GenericValue Interpreter::executeUIToFPInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);

	if (isa<VectorType>(SrcVal->getType())) {
		const Type *DstVecTy = DstTy->getScalarType();
		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal
		Dest.AggregateVal.resize(size);

		if (DstVecTy->getTypeID() == Type::FloatTyID) {
			assert(DstVecTy->isFloatingPointTy() && "Invalid UIToFP instruction");
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].FloatVal =
					APIntOps::RoundAPIntToFloat(Src.AggregateVal[i].IntVal);
		} else {
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].DoubleVal =
					APIntOps::RoundAPIntToDouble(Src.AggregateVal[i].IntVal);
		}
	} else {
		// scalar
		assert(DstTy->isFloatingPointTy() && "Invalid UIToFP instruction");
		if (DstTy->getTypeID() == Type::FloatTyID)
			Dest.FloatVal = APIntOps::RoundAPIntToFloat(Src.IntVal);
		else {
			Dest.DoubleVal = APIntOps::RoundAPIntToDouble(Src.IntVal);
		}
	}
	return Dest;
}

GenericValue Interpreter::executeSIToFPInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);

	if (isa<VectorType>(SrcVal->getType())) {
		const Type *DstVecTy = DstTy->getScalarType();
		unsigned size = Src.AggregateVal.size();
		// the sizes of src and dst vectors must be equal
		Dest.AggregateVal.resize(size);

		if (DstVecTy->getTypeID() == Type::FloatTyID) {
			assert(DstVecTy->isFloatingPointTy() && "Invalid SIToFP instruction");
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].FloatVal = APIntOps::RoundSignedAPIntToFloat(
					Src.AggregateVal[i].IntVal);
		} else {
			for (unsigned i = 0; i < size; i++)
				Dest.AggregateVal[i].DoubleVal = APIntOps::RoundSignedAPIntToDouble(
					Src.AggregateVal[i].IntVal);
		}
	} else {
		// scalar
		assert(DstTy->isFloatingPointTy() && "Invalid SIToFP instruction");

		if (DstTy->getTypeID() == Type::FloatTyID)
			Dest.FloatVal = APIntOps::RoundSignedAPIntToFloat(Src.IntVal);
		else {
			Dest.DoubleVal = APIntOps::RoundSignedAPIntToDouble(Src.IntVal);
		}
	}

	return Dest;
}

GenericValue Interpreter::executePtrToIntInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	uint32_t DBitWidth = cast<IntegerType>(DstTy)->getBitWidth();
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);
	assert(SrcVal->getType()->isPointerTy() && "Invalid PtrToInt instruction");

	Dest.IntVal = APInt(DBitWidth, (intptr_t)Src.PointerVal, false APINT_NOCHECK);
	return Dest;
}

GenericValue Interpreter::executeIntToPtrInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);
	assert(DstTy->isPointerTy() && "Invalid PtrToInt instruction");

	uint32_t PtrSize = getDataLayout().getPointerSizeInBits();
	if (PtrSize != Src.IntVal.getBitWidth())
		Src.IntVal = Src.IntVal.zextOrTrunc(PtrSize);

	Dest.PointerVal = PointerTy(intptr_t(Src.IntVal.getZExtValue()));
	return Dest;
}

GenericValue Interpreter::executeBitCastInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF)
{

	// This instruction supports bitwise conversion of vectors to integers and
	// to vectors of other types (as long as they have the same size)
	Type *SrcTy = SrcVal->getType();
	GenericValue Dest, Src = getOperandValue(SrcVal, SF);

	if (isa<VectorType>(SrcTy) || isa<VectorType>(DstTy)) {
		// vector src bitcast to vector dst or vector src bitcast to scalar dst or
		// scalar src bitcast to vector dst
		bool isLittleEndian = getDataLayout().isLittleEndian();
		GenericValue TempDst, TempSrc, SrcVec;
		const Type *SrcElemTy;
		const Type *DstElemTy;
		unsigned SrcBitSize;
		unsigned DstBitSize;
		unsigned SrcNum;
		unsigned DstNum;

		if (isa<VectorType>(SrcTy)) {
			SrcElemTy = SrcTy->getScalarType();
			SrcBitSize = SrcTy->getScalarSizeInBits();
			SrcNum = Src.AggregateVal.size();
			SrcVec = Src;
		} else {
			// if src is scalar value, make it vector <1 x type>
			SrcElemTy = SrcTy;
			SrcBitSize = SrcTy->getPrimitiveSizeInBits();
			SrcNum = 1;
			SrcVec.AggregateVal.push_back(Src);
		}

		if (isa<VectorType>(DstTy)) {
			DstElemTy = DstTy->getScalarType();
			DstBitSize = DstTy->getScalarSizeInBits();
			DstNum = (SrcNum * SrcBitSize) / DstBitSize;
		} else {
			DstElemTy = DstTy;
			DstBitSize = DstTy->getPrimitiveSizeInBits();
			DstNum = 1;
		}

		if (SrcNum * SrcBitSize != DstNum * DstBitSize)
			llvm_unreachable("Invalid BitCast");

		// If src is floating point, cast to integer first.
		TempSrc.AggregateVal.resize(SrcNum);
		if (SrcElemTy->isFloatTy()) {
			for (unsigned i = 0; i < SrcNum; i++)
				TempSrc.AggregateVal[i].IntVal =
					APInt::floatToBits(SrcVec.AggregateVal[i].FloatVal);

		} else if (SrcElemTy->isDoubleTy()) {
			for (unsigned i = 0; i < SrcNum; i++)
				TempSrc.AggregateVal[i].IntVal =
					APInt::doubleToBits(SrcVec.AggregateVal[i].DoubleVal);
		} else if (SrcElemTy->isIntegerTy()) {
			for (unsigned i = 0; i < SrcNum; i++)
				TempSrc.AggregateVal[i].IntVal = SrcVec.AggregateVal[i].IntVal;
		} else {
			// Pointers are not allowed as the element type of vector.
			llvm_unreachable("Invalid Bitcast");
		}

		// now TempSrc is integer type vector
		if (DstNum < SrcNum) {
			// Example: bitcast <4 x i32> <i32 0, i32 1, i32 2, i32 3> to <2 x i64>
			unsigned Ratio = SrcNum / DstNum;
			unsigned SrcElt = 0;
			for (unsigned i = 0; i < DstNum; i++) {
				GenericValue Elt;
				Elt.IntVal = 0;
				Elt.IntVal = Elt.IntVal.zext(DstBitSize);
				unsigned ShiftAmt = isLittleEndian ? 0 : SrcBitSize * (Ratio - 1);
				for (unsigned j = 0; j < Ratio; j++) {
					APInt Tmp;
					Tmp = Tmp.zext(SrcBitSize);
					Tmp = TempSrc.AggregateVal[SrcElt++].IntVal;
					Tmp = Tmp.zext(DstBitSize);
					Tmp = Tmp.shl(ShiftAmt);
					ShiftAmt += isLittleEndian ? SrcBitSize : -SrcBitSize;
					Elt.IntVal |= Tmp;
				}
				TempDst.AggregateVal.push_back(Elt);
			}
		} else {
			// Example: bitcast <2 x i64> <i64 0, i64 1> to <4 x i32>
			unsigned Ratio = DstNum / SrcNum;
			for (unsigned i = 0; i < SrcNum; i++) {
				unsigned ShiftAmt = isLittleEndian ? 0 : DstBitSize * (Ratio - 1);
				for (unsigned j = 0; j < Ratio; j++) {
					GenericValue Elt;
					Elt.IntVal = Elt.IntVal.zext(SrcBitSize);
					Elt.IntVal = TempSrc.AggregateVal[i].IntVal;
					Elt.IntVal = Elt.IntVal.lshr(ShiftAmt);
					// it could be DstBitSize == SrcBitSize, so check it
					if (DstBitSize < SrcBitSize)
						Elt.IntVal = Elt.IntVal.trunc(DstBitSize);
					ShiftAmt += isLittleEndian ? DstBitSize : -DstBitSize;
					TempDst.AggregateVal.push_back(Elt);
				}
			}
		}

		// convert result from integer to specified type
		if (isa<VectorType>(DstTy)) {
			if (DstElemTy->isDoubleTy()) {
				Dest.AggregateVal.resize(DstNum);
				for (unsigned i = 0; i < DstNum; i++)
					Dest.AggregateVal[i].DoubleVal =
						TempDst.AggregateVal[i].IntVal.bitsToDouble();
			} else if (DstElemTy->isFloatTy()) {
				Dest.AggregateVal.resize(DstNum);
				for (unsigned i = 0; i < DstNum; i++)
					Dest.AggregateVal[i].FloatVal =
						TempDst.AggregateVal[i].IntVal.bitsToFloat();
			} else {
				Dest = TempDst;
			}
		} else {
			if (DstElemTy->isDoubleTy())
				Dest.DoubleVal = TempDst.AggregateVal[0].IntVal.bitsToDouble();
			else if (DstElemTy->isFloatTy()) {
				Dest.FloatVal = TempDst.AggregateVal[0].IntVal.bitsToFloat();
			} else {
				Dest.IntVal = TempDst.AggregateVal[0].IntVal;
			}
		}
	} else { //  if ((SrcTy->getTypeID() == Type::VectorTyID) ||
		 //     (DstTy->getTypeID() == Type::VectorTyID))

		// scalar src bitcast to scalar dst
		if (DstTy->isPointerTy()) {
			assert(SrcTy->isPointerTy() && "Invalid BitCast");
			Dest.PointerVal = Src.PointerVal;
		} else if (DstTy->isIntegerTy()) {
			if (SrcTy->isFloatTy())
				Dest.IntVal = APInt::floatToBits(Src.FloatVal);
			else if (SrcTy->isDoubleTy()) {
				Dest.IntVal = APInt::doubleToBits(Src.DoubleVal);
			} else if (SrcTy->isIntegerTy()) {
				Dest.IntVal = Src.IntVal;
			} else {
				llvm_unreachable("Invalid BitCast");
			}
		} else if (DstTy->isFloatTy()) {
			if (SrcTy->isIntegerTy())
				Dest.FloatVal = Src.IntVal.bitsToFloat();
			else {
				Dest.FloatVal = Src.FloatVal;
			}
		} else if (DstTy->isDoubleTy()) {
			if (SrcTy->isIntegerTy())
				Dest.DoubleVal = Src.IntVal.bitsToDouble();
			else {
				Dest.DoubleVal = Src.DoubleVal;
			}
		} else {
			llvm_unreachable("Invalid Bitcast");
		}
	}

	return Dest;
}

void Interpreter::visitTruncInst(TruncInst &I)
{
	ExecutionContext &SF = ECStack().back();
	updateDataDeps(getCurThr().id, &I, I.getOperand(0)),
		SetValue(&I, executeTruncInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitSExtInst(SExtInst &I)
{
	ExecutionContext &SF = ECStack().back();
	updateDataDeps(getCurThr().id, &I, I.getOperand(0)),
		SetValue(&I, executeSExtInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitZExtInst(ZExtInst &I)
{
	ExecutionContext &SF = ECStack().back();
	updateDataDeps(getCurThr().id, &I, I.getOperand(0)),
		SetValue(&I, executeZExtInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPTruncInst(FPTruncInst &I)
{
	ExecutionContext &SF = ECStack().back();
	SetValue(&I, executeFPTruncInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPExtInst(FPExtInst &I)
{
	ExecutionContext &SF = ECStack().back();
	SetValue(&I, executeFPExtInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitUIToFPInst(UIToFPInst &I)
{
	ExecutionContext &SF = ECStack().back();
	SetValue(&I, executeUIToFPInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitSIToFPInst(SIToFPInst &I)
{
	ExecutionContext &SF = ECStack().back();
	SetValue(&I, executeSIToFPInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPToUIInst(FPToUIInst &I)
{
	ExecutionContext &SF = ECStack().back();
	SetValue(&I, executeFPToUIInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPToSIInst(FPToSIInst &I)
{
	ExecutionContext &SF = ECStack().back();
	SetValue(&I, executeFPToSIInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitPtrToIntInst(PtrToIntInst &I)
{
	ExecutionContext &SF = ECStack().back();
	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	SetValue(&I, executePtrToIntInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitIntToPtrInst(IntToPtrInst &I)
{
	ExecutionContext &SF = ECStack().back();
	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	SetValue(&I, executeIntToPtrInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitBitCastInst(BitCastInst &I)
{
	ExecutionContext &SF = ECStack().back();
	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	SetValue(&I, executeBitCastInst(I.getOperand(0), I.getType(), SF), SF);
}

#define IMPLEMENT_VAARG(TY)                                                                        \
	case Type::TY##TyID:                                                                       \
		Dest.TY##Val = Src.TY##Val;                                                        \
		break

void Interpreter::visitVAArgInst(VAArgInst &I)
{
	ExecutionContext &SF = ECStack().back();

	// Get the incoming valist parameter.  LLI treats the valist as a
	// (ec-stack-depth var-arg-index) pair.
	GenericValue VAList = getOperandValue(I.getOperand(0), SF);
	GenericValue Dest;
	GenericValue Src = ECStack()[VAList.UIntPairVal.first].VarArgs[VAList.UIntPairVal.second];
	Type *Ty = I.getType();
	switch (Ty->getTypeID()) {
	case Type::IntegerTyID:
		Dest.IntVal = Src.IntVal;
		break;
		IMPLEMENT_VAARG(Pointer);
		IMPLEMENT_VAARG(Float);
		IMPLEMENT_VAARG(Double);
	default:
		dbgs() << "Unhandled dest type for vaarg instruction: " << *Ty << "\n";
		llvm_unreachable(nullptr);
	}

	// Set the Value of this Instruction.
	updateDataDeps(getCurThr().id, &I, I.getOperand(0));
	SetValue(&I, Dest, SF);

	// Move the pointer to the next vararg.
	++VAList.UIntPairVal.second;
}

void Interpreter::visitExtractElementInst(ExtractElementInst &I)
{
	ExecutionContext &SF = ECStack().back();
	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Dest;

	Type *Ty = I.getType();
	const unsigned indx = unsigned(Src2.IntVal.getZExtValue());

	if (Src1.AggregateVal.size() > indx) {
		switch (Ty->getTypeID()) {
		default:
			dbgs() << "Unhandled destination type for extractelement instruction: "
			       << *Ty << "\n";
			llvm_unreachable(nullptr);
			break;
		case Type::IntegerTyID:
			Dest.IntVal = Src1.AggregateVal[indx].IntVal;
			break;
		case Type::FloatTyID:
			Dest.FloatVal = Src1.AggregateVal[indx].FloatVal;
			break;
		case Type::DoubleTyID:
			Dest.DoubleVal = Src1.AggregateVal[indx].DoubleVal;
			break;
		}
	} else {
		dbgs() << "Invalid index in extractelement instruction\n";
	}

	SetValue(&I, Dest, SF);
}

void Interpreter::visitInsertElementInst(InsertElementInst &I)
{
	ExecutionContext &SF = ECStack().back();
	Type *Ty = I.getType();

	if (!(Ty->isVectorTy()))
		llvm_unreachable("Unhandled dest type for insertelement instruction");

	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Src3 = getOperandValue(I.getOperand(2), SF);
	GenericValue Dest;

	Type *TyContained = Ty->getContainedType(0);

	const unsigned indx = unsigned(Src3.IntVal.getZExtValue());
	Dest.AggregateVal = Src1.AggregateVal;

	if (Src1.AggregateVal.size() <= indx)
		llvm_unreachable("Invalid index in insertelement instruction");
	switch (TyContained->getTypeID()) {
	default:
		llvm_unreachable("Unhandled dest type for insertelement instruction");
	case Type::IntegerTyID:
		Dest.AggregateVal[indx].IntVal = Src2.IntVal;
		break;
	case Type::FloatTyID:
		Dest.AggregateVal[indx].FloatVal = Src2.FloatVal;
		break;
	case Type::DoubleTyID:
		Dest.AggregateVal[indx].DoubleVal = Src2.DoubleVal;
		break;
	}
	SetValue(&I, Dest, SF);
}

void Interpreter::visitShuffleVectorInst(ShuffleVectorInst &I)
{
	ExecutionContext &SF = ECStack().back();

	Type *Ty = I.getType();
	if (!(Ty->isVectorTy()))
		llvm_unreachable("Unhandled dest type for shufflevector instruction");

	GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Src3 = getOperandValue(I.getOperand(2), SF);
	GenericValue Dest;

	// There is no need to check types of src1 and src2, because the compiled
	// bytecode can't contain different types for src1 and src2 for a
	// shufflevector instruction.

	Type *TyContained = Ty->getContainedType(0);
	unsigned src1Size = (unsigned)Src1.AggregateVal.size();
	unsigned src2Size = (unsigned)Src2.AggregateVal.size();
	unsigned src3Size = (unsigned)Src3.AggregateVal.size();

	Dest.AggregateVal.resize(src3Size);

	switch (TyContained->getTypeID()) {
	default:
		llvm_unreachable("Unhandled dest type for insertelement instruction");
		break;
	case Type::IntegerTyID:
		for (unsigned i = 0; i < src3Size; i++) {
			unsigned j = Src3.AggregateVal[i].IntVal.getZExtValue();
			if (j < src1Size)
				Dest.AggregateVal[i].IntVal = Src1.AggregateVal[j].IntVal;
			else if (j < src1Size + src2Size)
				Dest.AggregateVal[i].IntVal =
					Src2.AggregateVal[j - src1Size].IntVal;
			else
				// The selector may not be greater than sum of lengths of first and
				// second operands and llasm should not allow situation like
				// %tmp = shufflevector <2 x i32> <i32 3, i32 4>, <2 x i32> undef,
				//                      <2 x i32> < i32 0, i32 5 >,
				// where i32 5 is invalid, but let it be additional check here:
				llvm_unreachable("Invalid mask in shufflevector instruction");
		}
		break;
	case Type::FloatTyID:
		for (unsigned i = 0; i < src3Size; i++) {
			unsigned j = Src3.AggregateVal[i].IntVal.getZExtValue();
			if (j < src1Size)
				Dest.AggregateVal[i].FloatVal = Src1.AggregateVal[j].FloatVal;
			else if (j < src1Size + src2Size)
				Dest.AggregateVal[i].FloatVal =
					Src2.AggregateVal[j - src1Size].FloatVal;
			else
				llvm_unreachable("Invalid mask in shufflevector instruction");
		}
		break;
	case Type::DoubleTyID:
		for (unsigned i = 0; i < src3Size; i++) {
			unsigned j = Src3.AggregateVal[i].IntVal.getZExtValue();
			if (j < src1Size)
				Dest.AggregateVal[i].DoubleVal = Src1.AggregateVal[j].DoubleVal;
			else if (j < src1Size + src2Size)
				Dest.AggregateVal[i].DoubleVal =
					Src2.AggregateVal[j - src1Size].DoubleVal;
			else
				llvm_unreachable("Invalid mask in shufflevector instruction");
		}
		break;
	}
	SetValue(&I, Dest, SF);
}

void Interpreter::visitExtractValueInst(ExtractValueInst &I)
{
	ExecutionContext &SF = ECStack().back();
	Value *Agg = I.getAggregateOperand();
	GenericValue Dest;
	GenericValue Src = getOperandValue(Agg, SF);

	ExtractValueInst::idx_iterator IdxBegin = I.idx_begin();
	unsigned Num = I.getNumIndices();
	GenericValue *pSrc = &Src;

	for (unsigned i = 0; i < Num; ++i) {
		pSrc = &pSrc->AggregateVal[*IdxBegin];
		++IdxBegin;
	}

	updateDataDeps(getCurThr().id, &I, getDataDeps(getCurThr().id, Agg));

	Type *IndexedType = ExtractValueInst::getIndexedType(Agg->getType(), I.getIndices());
	switch (IndexedType->getTypeID()) {
	default:
		llvm_unreachable("Unhandled dest type for extractelement instruction");
		break;
	case Type::IntegerTyID:
		Dest.IntVal = pSrc->IntVal;
		break;
	case Type::FloatTyID:
		Dest.FloatVal = pSrc->FloatVal;
		break;
	case Type::DoubleTyID:
		Dest.DoubleVal = pSrc->DoubleVal;
		break;
	case Type::ArrayTyID:
	case Type::StructTyID:
		LLVM_VECTOR_TYPEID_CASES
		Dest.AggregateVal = pSrc->AggregateVal;
		break;
	case Type::PointerTyID:
		Dest.PointerVal = pSrc->PointerVal;
		break;
	}

	SetValue(&I, Dest, SF);
}

void Interpreter::visitInsertValueInst(InsertValueInst &I)
{

	ExecutionContext &SF = ECStack().back();
	Value *Agg = I.getAggregateOperand();

	GenericValue Src1 = getOperandValue(Agg, SF);
	GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
	GenericValue Dest = Src1; // Dest is a slightly changed Src1

	ExtractValueInst::idx_iterator IdxBegin = I.idx_begin();
	unsigned Num = I.getNumIndices();

	GenericValue *pDest = &Dest;
	for (unsigned i = 0; i < Num; ++i) {
		pDest = &pDest->AggregateVal[*IdxBegin];
		++IdxBegin;
	}
	// pDest points to the target value in the Dest now

	updateDataDeps(getCurThr().id, &I, getDataDeps(getCurThr().id, Agg));

	Type *IndexedType = ExtractValueInst::getIndexedType(Agg->getType(), I.getIndices());

	switch (IndexedType->getTypeID()) {
	default:
		llvm_unreachable("Unhandled dest type for insertelement instruction");
		break;
	case Type::IntegerTyID:
		pDest->IntVal = Src2.IntVal;
		break;
	case Type::FloatTyID:
		pDest->FloatVal = Src2.FloatVal;
		break;
	case Type::DoubleTyID:
		pDest->DoubleVal = Src2.DoubleVal;
		break;
	case Type::ArrayTyID:
	case Type::StructTyID:
		LLVM_VECTOR_TYPEID_CASES
		pDest->AggregateVal = Src2.AggregateVal;
		break;
	case Type::PointerTyID:
		pDest->PointerVal = Src2.PointerVal;
		break;
	}

	SetValue(&I, Dest, SF);
}

GenericValue Interpreter::getConstantExprValue(ConstantExpr *CE, ExecutionContext &SF)
{
	switch (CE->getOpcode()) {
	case Instruction::Trunc:
		return executeTruncInst(CE->getOperand(0), CE->getType(), SF);
#if LLVM_VERSION_MAJOR < 19
	case Instruction::ZExt:
		return executeZExtInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::SExt:
		return executeSExtInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::FPTrunc:
		return executeFPTruncInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::FPExt:
		return executeFPExtInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::UIToFP:
		return executeUIToFPInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::SIToFP:
		return executeSIToFPInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::FPToUI:
		return executeFPToUIInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::FPToSI:
		return executeFPToSIInst(CE->getOperand(0), CE->getType(), SF);
#endif
	case Instruction::PtrToInt:
		return executePtrToIntInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::IntToPtr:
		return executeIntToPtrInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::BitCast:
		return executeBitCastInst(CE->getOperand(0), CE->getType(), SF);
	case Instruction::GetElementPtr:
		return executeGEPOperation(CE->getOperand(0), gep_type_begin(CE), gep_type_end(CE),
					   SF);
#if LLVM_VERSION_MAJOR < 19
	case Instruction::FCmp:
	case Instruction::ICmp:
		return executeCmpInst(CE->getPredicate(), getOperandValue(CE->getOperand(0), SF),
				      getOperandValue(CE->getOperand(1), SF),
				      CE->getOperand(0)->getType());
	case Instruction::Select:
		return executeSelectInst(getOperandValue(CE->getOperand(0), SF),
					 getOperandValue(CE->getOperand(1), SF),
					 getOperandValue(CE->getOperand(2), SF),
					 CE->getOperand(0)->getType());
#endif
	default:
		break;
	}

	// The cases below here require a GenericValue parameter for the result
	// so we initialize one, compute it and then return it.
	GenericValue Op0 = getOperandValue(CE->getOperand(0), SF);
	GenericValue Op1 = getOperandValue(CE->getOperand(1), SF);
	GenericValue Dest;
#if LLVM_VERSION_MAJOR < 19
	Type *Ty = CE->getOperand(0)->getType();
#endif
	switch (CE->getOpcode()) {
	case Instruction::Add:
		Dest.IntVal = Op0.IntVal + Op1.IntVal;
		break;
	case Instruction::Sub:
		Dest.IntVal = Op0.IntVal - Op1.IntVal;
		break;
	case Instruction::Mul:
		Dest.IntVal = Op0.IntVal * Op1.IntVal;
		break;
#if LLVM_VERSION_MAJOR < 19
	case Instruction::FAdd:
		executeFAddInst(Dest, Op0, Op1, Ty);
		break;
	case Instruction::FSub:
		executeFSubInst(Dest, Op0, Op1, Ty);
		break;
	case Instruction::FMul:
		executeFMulInst(Dest, Op0, Op1, Ty);
		break;
	case Instruction::FDiv:
		executeFDivInst(Dest, Op0, Op1, Ty);
		break;
	case Instruction::FRem:
		executeFRemInst(Dest, Op0, Op1, Ty);
		break;
	case Instruction::SDiv:
		Dest.IntVal = Op0.IntVal.sdiv(Op1.IntVal);
		break;
	case Instruction::UDiv:
		Dest.IntVal = Op0.IntVal.udiv(Op1.IntVal);
		break;
	case Instruction::URem:
		Dest.IntVal = Op0.IntVal.urem(Op1.IntVal);
		break;
	case Instruction::SRem:
		Dest.IntVal = Op0.IntVal.srem(Op1.IntVal);
		break;
	case Instruction::And:
		Dest.IntVal = Op0.IntVal & Op1.IntVal;
		break;
	case Instruction::Or:
		Dest.IntVal = Op0.IntVal | Op1.IntVal;
		break;
#endif
	case Instruction::Xor:
		Dest.IntVal = Op0.IntVal ^ Op1.IntVal;
		break;
	case Instruction::Shl:
		Dest.IntVal = Op0.IntVal.shl(Op1.IntVal.getZExtValue());
		break;
#if LLVM_VERSION_MAJOR < 19
	case Instruction::LShr:
		Dest.IntVal = Op0.IntVal.lshr(Op1.IntVal.getZExtValue());
		break;
	case Instruction::AShr:
		Dest.IntVal = Op0.IntVal.ashr(Op1.IntVal.getZExtValue());
		break;
#endif
	default:
		dbgs() << "Unhandled ConstantExpr: " << *CE << "\n";
		llvm_unreachable("Unhandled ConstantExpr");
	}
	return Dest;
}

GenericValue Interpreter::getOperandValue(Value *V, ExecutionContext &SF)
{
	if (ConstantExpr *CE = dyn_cast<ConstantExpr>(V)) {
		return getConstantExprValue(CE, SF);
	} else if (Constant *CPV = dyn_cast<Constant>(V)) {
		return getConstantValue(CPV);
	} else if (GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
		return PTOGV(getPointerToGlobal(GV));
	} else {
		return SF.Values[V];
	}
}

//===----------------------------------------------------------------------===//
//                        Dispatch and Execution Code
//===----------------------------------------------------------------------===//

void Interpreter::handleLock(SAddr addr, ASize size, const EventDeps *deps)
{

	auto *I = ECStack().back().CurInst->getPrevNode();
	auto annot = std::move(Annotation(
		AssumeType::Spinloop,
		Annotation::ExprVP(
			NeExpr<AnnotID>::create(
				RegisterExpr<AnnotID>::create(size.getBits(), MI->idInfo.VID.at(I)),
				ConcreteExpr<AnnotID>::create(size.getBits(), SVal(1)))
				.release())));

	GenMCDriver::HandleResult<SVal> ret;
	SVal *retVal = nullptr;

#define IMPLEMENT_LOCK_VISIT(__kindR, __kindW)                                                     \
	case switchPair(__kindR, __kindW): {                                                       \
		ret = CALL_DRIVER_RESET_IF_NONE(handleLoad<__kindR>, currDbgInfo(addr), currPos(), \
						std::nullopt, addr, size, std::move(annot),        \
						GET_DEPS(deps));                                   \
		retVal = std::get_if<SVal>(&ret);                                                  \
		if (!retVal)                                                                       \
			return;                                                                    \
		if (*retVal == SVal(0))                                                            \
			CALL_DRIVER(handleStore<__kindW>, currDbgInfo(addr), currPos(),            \
				    std::nullopt, addr, size, GET_DEPS(deps));                     \
		else                                                                               \
			CALL_DRIVER(handleAssume, currDbgInfo(), currPos(), AssumeType::Spinloop); \
		break;                                                                             \
	}

	switch (switchPair(getLockKinds(*I))) {
		IMPLEMENT_LOCK_VISIT(EventLabel::EventLabelKind::LockCasRead,
				     EventLabel::EventLabelKind::LockCasWrite);
		IMPLEMENT_LOCK_VISIT(EventLabel::EventLabelKind::AbstractLockCasRead,
				     EventLabel::EventLabelKind::AbstractLockCasWrite);
	default:
		BUG();
	}
}

void Interpreter::handleUnlock(SAddr addr, ASize size, const EventDeps *deps)
{
	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::UnlockWrite>, currDbgInfo(addr),
		    currPos(), std::nullopt, MemOrdering::Release, addr, size, AType::Signed,
		    SVal(0), GET_DEPS(deps));
}

void Interpreter::callAssertFail(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	auto errMsg = (ArgVals.size())
			      ? std::string("Assertion violation: ") +
					std::string((char *)getStaticAddr(GVTOP(ArgVals[0])))
			      : "Unknown";

	CALL_DRIVER(handleError, currDbgInfo(), currPos(), errMsg);
}

void Interpreter::callOptBegin(Function *F, const std::vector<GenericValue> &ArgVals,
			       const std::unique_ptr<EventDeps> &specialDeps)
{
	auto expand = CALL_DRIVER(handleOptional, currDbgInfo(), currPos());
	updateCtrlDeps(getCurThr().id, currPos()); // add a ctrl dep on optionals

	GenericValue result;
	result.IntVal = APInt(F->getReturnType()->getIntegerBitWidth(), expand,
			      true APINT_NOCHECK); // signed
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callLoopBegin(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleLoopBegin, currDbgInfo(), currPos());
}

void Interpreter::callSpinStart(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleSpinStart, currDbgInfo(), currPos());
}

void Interpreter::callFaiZNESpinEnd(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleFaiZNESpinEnd, currDbgInfo(), currPos());
}

void Interpreter::callLockZNESpinEnd(Function *F, const std::vector<GenericValue> &ArgVals,
				     const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleLockZNESpinEnd, currDbgInfo(), currPos());
}

void Interpreter::callKillThread(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	if (ArgVals[0].IntVal.getBoolValue()) {
		CALL_DRIVER(handleThreadKill, currDbgInfo(), currPos());
		ECStack().clear();
	}
}

void Interpreter::callAssume(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	if (!ArgVals[0].IntVal.getBoolValue()) {
		CALL_DRIVER(handleAssume, currDbgInfo(), currPos(),
			    AssumeType(ArgVals[1].IntVal.getLimitedValue()));
	}

	/* Handle invoke-instruction */
	returnValueToCaller(F->getReturnType() /* void */, PTOGV(nullptr));
}

void Interpreter::callNondetInt(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	Thread::MyDist dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	GenericValue result;
	result.IntVal = APInt(F->getReturnType()->getIntegerBitWidth(), dist(getCurThr().rng),
			      true); // signed
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callMalloc(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	auto size = ArgVals[0].IntVal.getLimitedValue();

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc, currDbgInfo(), currPos(), size,
				   alignof(std::max_align_t), StorageDuration::SD_Heap,
				   StorageType::ST_Volatile, AddressSpace::AS_User, GET_DEPS(deps));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
	return;
}

void Interpreter::callMallocAligned(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	auto align = ArgVals[0].IntVal.getLimitedValue();
	auto size = ArgVals[1].IntVal.getLimitedValue();

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc, currDbgInfo(), currPos(), size, align,
				   StorageDuration::SD_Heap, StorageType::ST_Volatile,
				   AddressSpace::AS_User, GET_DEPS(deps));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
	return;
}

void Interpreter::callPMalloc(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	BUG_ON("Unsupported");

	auto size = ArgVals[0].IntVal.getLimitedValue();

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc, currDbgInfo(), currPos(), size,
				   alignof(std::max_align_t), StorageDuration::SD_Heap,
				   StorageType::ST_Durable, AddressSpace::AS_User, GET_DEPS(deps));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
	return;
}

void Interpreter::callFree(Function *F, const std::vector<GenericValue> &ArgVals,
			   const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *ptr = (GenericValue *)GVTOP(ArgVals[0]);

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* When attempting to free a NULL pointer, don't increase counters */
	if (ptr)
		CALL_DRIVER(handleFree, currDbgInfo(), currPos(), ptr, GET_DEPS(deps));
	return;
}

void Interpreter::callThreadSelf(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	llvm::Type *typ = F->getReturnType();

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id), nullptr, nullptr);
	auto result = SVal(getCurThr().id);
	returnValueToCaller(typ, SVAL_TO_GV(result, typ));
	return;
}

void Interpreter::callThreadCreate(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	Function *calledFun = (Function *)GVTOP(ArgVals[1]);
	ExecutionContext SF;

	/* First, set up the stack frame for the new function.
	 * Calling function needs to take only one argument ... */
	SF.CurFunction = calledFun;
	SF.CurBB = &calledFun->front();
	SF.CurInst = SF.CurBB->begin();

	SetValue(&*calledFun->arg_begin(), ArgVals[2], SF);

	/* Then, inform the driver about the thread creation */
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	int symm = ArgVals.size() > 3 ? ArgVals[3].IntVal.getLimitedValue() : -1;
	auto info = ThreadInfo(-1, currPos().thread, MI->idInfo.VID.at(calledFun),
			       SVal((uintptr_t)ArgVals[2].PointerVal),
			       getCurThr().threadFun->getName().str(), symm);
	auto tid = CALL_DRIVER(handleThreadCreate, currDbgInfo(), currPos(), info, GET_DEPS(deps));

	/* Prepare the execution context for the new thread */
	info.id = tid;
	constructAddThreadFromInfo(info);

	/* ... and return the TID of the created thread to the caller */
	Type *typ = F->getReturnType();
	returnValueToCaller(typ, INT_TO_GV(typ, tid));
}

void Interpreter::callThreadCreateSymmetric(Function *F, const std::vector<GenericValue> &ArgVals,
					    const std::unique_ptr<EventDeps> &specialDeps)
{
	callThreadCreate(F, ArgVals, specialDeps);
}

/* callPthreadJoin - Call to pthread_join() function */
void Interpreter::callThreadJoin(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto result = CALL_DRIVER_RESET_IF_NONE(handleThreadJoin, currDbgInfo(), currPos(),
						ArgVals[0].IntVal.getLimitedValue(),
						GET_DEPS(deps));
	auto retVal = std::get_if<SVal>(&result);
	if (!retVal)
		return;
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(*retVal, F->getReturnType()));
}

void Interpreter::callThreadExit(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	while (ECStack().size() > 1) {
		freeAllocas(ECStack().back().Allocas);
		ECStack().pop_back();
	}
	popStackAndReturnValueToCaller(F->getReturnType(), ArgVals[0]);
}

void Interpreter::callAtExit(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	addAtExitHandler((Function *)GVTOP(ArgVals[0]));
	returnValueToCaller(F->getReturnType(), INT_TO_GV(F->getReturnType(), 0));
}

void Interpreter::callMutexInit(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *lock = (GenericValue *)GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *)GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	if (attr)
		WARN_ONCE("pthread-mutex-init-arg",
			  "Ignoring non-null argument given to pthread_mutex_init.\n");

	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::Write>, currDbgInfo(lock), currPos(),
		    std::nullopt, MemOrdering::NotAtomic, lock, size, atyp, SVal(0),
		    GET_DEPS(specialDeps));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
}

void Interpreter::callMutexLock(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *ptr = (GenericValue *)GVTOP(ArgVals[0]);
	// return type carries type of the mutex location (workaround of opaque ptr)
	Type *typ = F->getReturnType();
	GenericValue result;

	handleLock(ptr, getTypeSize(typ), &*specialDeps);

	/*
	 * We need to return a result anyway, because even if the current thread
	 * blocked, it might become unblocked at some point in the future by another
	 * thread
	 */
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0); /* Success */
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callMutexUnlock(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *ptr = (GenericValue *)GVTOP(ArgVals[0]);
	// return type carries type of the mutex location (workaround of opaque ptr)
	Type *typ = F->getReturnType();
	GenericValue result;

	handleUnlock(ptr, getTypeSize(typ), &*specialDeps);

	result.IntVal = APInt(typ->getIntegerBitWidth(), 0); /* Success */
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callMutexTrylock(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *ptr = (GenericValue *)GVTOP(ArgVals[0]);
	Type *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);
	GenericValue result;

	auto ret = std::get<SVal>(CALL_DRIVER(
		handleLoad<EventLabel::EventLabelKind::TrylockCasRead>, currDbgInfo(ptr), currPos(),
		std::nullopt, ptr, size, getCurrentAnnotConcretized(), GET_DEPS(specialDeps)));

	auto cmpRes = ret == SVal(0);
	if (cmpRes)
		CALL_DRIVER(handleStore<EventLabel::EventLabelKind::TrylockCasWrite>,
			    currDbgInfo(ptr), currPos(), std::nullopt, ptr, size,
			    GET_DEPS(specialDeps));

	result.IntVal = APInt(typ->getIntegerBitWidth(), !cmpRes);
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callMutexDestroy(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *lock = (GenericValue *)GVTOP(ArgVals[0]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::Write>, currDbgInfo(lock), currPos(),
		    std::nullopt, MemOrdering::NotAtomic, lock, size, atyp, SVal(-1),
		    GET_DEPS(specialDeps));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
	return;
}

void Interpreter::callCondVarInit(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *cvar = (GenericValue *)GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *)GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	if (attr)
		WARN_ONCE("pthread-cvar-init-arg",
			  "Ignoring non-null argument given to pthread_cond_init.\n");

	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::Write>, currDbgInfo(cvar), currPos(),
		    std::nullopt, MemOrdering::NotAtomic, cvar, size, atyp, SVal(0),
		    GET_DEPS(specialDeps));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
}

void Interpreter::callCondVarWait(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *cvar = (GenericValue *)GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *)GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	auto *I = ECStack().back().CurInst->getPrevNode();
	auto annot = std::move(Annotation(
		AssumeType::Spinloop,
		Annotation::ExprVP(
			SgtExpr<AnnotID>::create(
				RegisterExpr<AnnotID>::create(ASize(size).getBits(),
							      MI->idInfo.VID.at(I)),
				ConcreteExpr<AnnotID>::create(ASize(size).getBits(), SVal(0)))
				.release())));
	auto val = std::get<SVal>(
		CALL_DRIVER(handleLoad<EventLabel::EventLabelKind::CondVarWaitRead>,
			    currDbgInfo(cvar), currPos(), std::nullopt, MemOrdering::Relaxed, cvar,
			    size, atyp, GET_DEPS(specialDeps)));

	returnValueToCaller(typ, SVAL_TO_GV(val, typ));
}

void Interpreter::callCondVarSignal(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *cvar = (GenericValue *)GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *)GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::CondVarSignalWrite>, currDbgInfo(cvar),
		    currPos(), std::nullopt, MemOrdering::Relaxed, cvar, size, atyp, SVal(1),
		    GET_DEPS(specialDeps));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
}

void Interpreter::callCondVarBcast(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *cvar = (GenericValue *)GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *)GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::CondVarBcastWrite>, currDbgInfo(cvar),
		    currPos(), std::nullopt, MemOrdering::Relaxed, cvar, size, atyp, SVal(1),
		    GET_DEPS(specialDeps));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
}

void Interpreter::callCondVarDestroy(Function *F, const std::vector<GenericValue> &ArgVals,
				     const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *cvar = (GenericValue *)GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *)GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::CondVarDestroyWrite>, currDbgInfo(cvar),
		    currPos(), std::nullopt, MemOrdering::Relaxed, cvar, size, atyp, SVal(-1),
		    GET_DEPS(specialDeps));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
}

void Interpreter::callHazptrAlloc(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc, currDbgInfo(), currPos(),
				   getTypeSize(F->getReturnType()), alignof(std::max_align_t),
				   StorageDuration::SD_Heap, StorageType::ST_Volatile,
				   AddressSpace::AS_Internal, GET_DEPS(deps));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
}

void Interpreter::callHazptrProtect(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	auto *hp = GVTOP(ArgVals[0]);
	auto *ptr = GVTOP(ArgVals[1]);

	CALL_DRIVER(handleHpProtect, currDbgInfo(), currPos(), hp, ptr);

	/* Handle invoke-instruction */
	returnValueToCaller(F->getReturnType() /* void */, PTOGV(nullptr));
}

void Interpreter::callHazptrClear(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	auto *typ = PointerType::getUnqual(F->getParent()->getContext());
	auto asize = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);
	auto *hp = GVTOP(ArgVals[0]);

	/* FIXME: Should this be an internal null? */
	CALL_DRIVER(handleStore<EventLabel::EventLabelKind::Write>, currDbgInfo(hp), currPos(),
		    std::nullopt, MemOrdering::Release, hp, asize, atyp, SVal(),
		    GET_DEPS(specialDeps));

	/* Handle invoke-instruction */
	returnValueToCaller(F->getReturnType() /* void */, PTOGV(nullptr));
}

void Interpreter::callHazptrFree(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	CALL_DRIVER(handleFree, currDbgInfo(), currPos(), GVTOP(ArgVals[0]), GET_DEPS(deps));

	/* Handle invoke-instruction */
	returnValueToCaller(F->getReturnType() /* void */, PTOGV(nullptr));
}

void Interpreter::callHazptrRetire(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	CALL_DRIVER(handleRetire, currDbgInfo(), currPos(), GVTOP(ArgVals[0]), GET_DEPS(deps));

	/* Handle invoke-instruction */
	returnValueToCaller(F->getReturnType() /* void */, PTOGV(nullptr));
}

void Interpreter::callMethodBegin(Function * /*F*/, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> & /*specialDeps*/)
{
	auto methodName = static_cast<const char *>(getStaticAddr(GVTOP(ArgVals[0])));
	int32_t argVal = ArgVals.size() > 1 ? ArgVals[1].IntVal.getLimitedValue() : 0;
	CALL_DRIVER(handleMethodBegin, currDbgInfo(), currPos(), methodName, argVal);
}

void Interpreter::callMethodEnd(Function * /*F*/, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> & /*specialDeps*/)
{
	auto methodName = static_cast<const char *>(getStaticAddr(GVTOP(ArgVals[0])));
	int32_t retVal = ArgVals.size() > 1 ? ArgVals[1].IntVal.getLimitedValue() : 0;
	CALL_DRIVER(handleMethodEnd, currDbgInfo(), currPos(), methodName, retVal);
}

/* These should be eliminated before they are called */
void Interpreter::callAnnotateBegin(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{}

void Interpreter::callAnnotateEnd(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{}

void Interpreter::callOutput(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	const char *msg = static_cast<const char *>(getStaticAddr(GVTOP(ArgVals[0])));

	CALL_DRIVER(handleOutput, currDbgInfo(), currPos(), msg);
}

bool isInternalCall(Function *F) { return internalFunNames.count(F->getName().str()); }

void Interpreter::callInternalFunction(Function *F, const std::vector<GenericValue> &ArgVals,
				       const std::unique_ptr<EventDeps> &specialDeps)
{
	auto fCode = internalFunNames.at(F->getName().str());

	switch (fCode) {
#define HANDLE_FUNCTION(NUM, FUN, NAME)                                                            \
	case InternalFunctions::NAME:                                                              \
		call##NAME(F, ArgVals, specialDeps);                                               \
		break;
#include "Runtime/InternalFunction.def"
	default:
		BUG();
		break;
	}
	return;
}

std::vector<GenericValue>
Interpreter::translateExternalCallArgs(Function *F, const std::vector<GenericValue> &ArgVals) const
{
	std::vector<GenericValue> result;

	for (auto i = 0u; i < ArgVals.size(); ++i) {
		/* Unfortunately, we cannot check for the F->arg_begin() type:
		 * the signature for printf might contain var_args (and thus it might be
		 * distance(arg_begin(), arg_end()) < ArgVals.size()), so we just
		 * check whether .PointerVal is non-null and hope for the best */
		if (GVTOP(ArgVals[i]) && SAddr(GVTOP(ArgVals[i])).isStatic()) {
			auto transAddr = PTOGV(getStaticAddr(GVTOP(ArgVals[i])));
			result.push_back(transAddr);
		} else {
			result.push_back(ArgVals[i]);
		}
	}
	return result;
}

//===----------------------------------------------------------------------===//
// callFunction - Execute the specified function...
//
void Interpreter::callFunction(Function *F, const std::vector<GenericValue> &ArgVals,
			       const std::unique_ptr<EventDeps> &specialDeps)
{
	/* Special handling for internal calls */
	if (isInternalCall(F)) {
		callInternalFunction(F, ArgVals, specialDeps);
		return;
	}

	assert(!specialDeps);
	assert((ECStack().empty() || !&ECStack().back().Caller ||
		ECStack().back().Caller.arg_size() == ArgVals.size()) &&
	       "Incorrect number of arguments passed into function call!");
	// Make a new stack frame... and fill it in.
	ECStack().push_back(ExecutionContext());

	ExecutionContext &StackFrame = ECStack().back();
	StackFrame.CurFunction = F;

	// Special handling for external functions.
	if (F->isDeclaration()) {
		auto translated = translateExternalCallArgs(F, ArgVals);
		auto Result = callExternalFunction(F, translated);
		// Simulate a 'ret' instruction of the appropriate type.
		popStackAndReturnValueToCaller(F->getReturnType(), Result);
		return;
	}

	// Get pointers to first LLVM BB & Instruction in function.
	StackFrame.CurBB = &F->front();
	StackFrame.CurInst = StackFrame.CurBB->begin();

	// Run through the function arguments and initialize their values...
	assert((ArgVals.size() == F->arg_size() ||
		(ArgVals.size() > F->arg_size() && F->getFunctionType()->isVarArg())) &&
	       "Invalid number of values passed to function invocation!");

	// Handle non-varargs arguments...
	unsigned i = 0;
	for (Function::arg_iterator AI = F->arg_begin(), E = F->arg_end(); AI != E; ++AI, ++i)
		SetValue(&*AI, ArgVals[i], StackFrame);

	// Handle varargs arguments...
	StackFrame.VarArgs.assign(ArgVals.begin() + i, ArgVals.end());
}

void Interpreter::runAtExitHandlers()
{
	auto oldState = getProgramState();
	setProgramState(ProgramState::Dtors);
	while (!dynState.AtExitHandlers.empty()) {
		scheduleThread(0);
		callFunction(dynState.AtExitHandlers.back(), std::vector<GenericValue>(), nullptr);
		dynState.AtExitHandlers.pop_back();

		// Don't call run; just run until the exit handler function returns...
		auto size = ECStack().size();
		while (ECStack().size() >= size) {
			llvm::ExecutionContext &SF = ECStack().back();
			llvm::Instruction &I = *SF.CurInst++;
			visit(I);
			if (!ECStack().empty()) {
				dynState.globalInstructions[currPos().thread].kind =
					getInstKind(&*ECStack().back().CurInst);
			}
		}
		// run();
	}
	setProgramState(oldState);
}

void Interpreter::run()
{
	auto tid = driver->scheduleNext(dynState.globalInstructions);
	while (std::holds_alternative<int>(tid)) {
		scheduleThread(std::get<int>(tid));
		llvm::ExecutionContext &SF = ECStack().back();
		llvm::Instruction &I = *SF.CurInst++;
		visit(I);
		if (!ECStack().empty()) {
			dynState.globalInstructions[currPos().thread].kind =
				getInstKind(&*ECStack().back().CurInst);
		}
		tid = driver->scheduleNext(dynState.globalInstructions);
	}
}

int Interpreter::runMain(bool collectDbg)
{
	setupStaticCtorsDtors(true);
	setupMain(mainFun, {"prog"}, nullptr);
	setupStaticCtorsDtors(false);

	mainECStack = getThrById(0).initEC = ECStack();
	setProgramState(llvm::ProgramState::Main);
	dynState.globalInstructions[currPos().thread].kind =
		getInstKind(&*ECStack().back().CurInst);
	dynState.collectDbgInfo = collectDbg;

	run();
	return dynState.ExitValue.IntVal.getZExtValue();
}
