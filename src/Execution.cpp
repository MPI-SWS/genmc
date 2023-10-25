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
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "Error.hpp"
#include "Event.hpp"
#include "GenMCDriver.hpp"
#include "Interpreter.h"
#include "LLVMUtils.hpp"
#include "SExprVisitor.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include <algorithm>
#include <cmath>
#include <sstream>
using namespace llvm;

#define DEBUG_TYPE "interpreter"

#if LLVM_VERSION_MAJOR < 11
# define LLVM_VECTOR_TYPEID_CASES case llvm::Type::VectorTyID:
# else
# define LLVM_VECTOR_TYPEID_CASES case llvm::Type::FixedVectorTyID: case llvm::Type::ScalableVectorTyID:
#endif

// static cl::opt<bool> PrintVolatile("interpreter-print-volatile", cl::Hidden,
//           cl::desc("make the interpreter print every volatile load and store"));


//===----------------------------------------------------------------------===//
//                     Various Helper Functions
//===----------------------------------------------------------------------===//

#define GET_DEPS(deps) (deps ? *deps : EventDeps())

#define SVAL_TO_GV(val, typ)						\
({									\
	llvm::GenericValue __result;					\
	if (auto *iTyp = llvm::dyn_cast<IntegerType>(typ))		\
		__result.IntVal = APInt(iTyp->getBitWidth(), (val).get(), iTyp->getSignBit()); \
	else								\
		__result.PointerVal = (void *) (val).get();		\
	__result;							\
})

#define GV_TO_SVAL(val, typ)						\
({									\
	SVal __result;							\
	if (auto *iTyp = llvm::dyn_cast<IntegerType>(typ))		\
		__result = (val).IntVal.getLimitedValue();		\
	else								\
		__result = (uintptr_t) (val).PointerVal;		\
	__result;							\
})

#define TYPE_TO_ATYPE(typ)						\
({									\
		AType __result;						\
if (auto *iTyp = llvm::dyn_cast<IntegerType>(typ))			\
		__result = (iTyp->getSignBit() ? AType::Signed : AType::Unsigned); \
	else								\
		__result = AType::Pointer;				\
	__result;							\
})

static void SetValue(Value *V, GenericValue Val, ExecutionContext &SF) {
  SF.Values[V] = Val;
}

bool Interpreter::isStaticallyAllocated(SAddr addr) const
{
	auto p = std::make_pair(addr, addr);
	auto it = std::lower_bound(staticAllocas.begin(), staticAllocas.end(), p,
				   [](const decltype(p) &itV, const decltype(p) &v){
					   return itV.second < v.first;
				   });
	return it == staticAllocas.end() ? false : (it->first <= addr && addr <= it->second);
}

SAddr getStaticAllocBegin(const VSet<std::pair<SAddr, SAddr>> &allocMap, SAddr addr)
{
	auto p = std::make_pair(addr, addr);
	auto it = std::lower_bound(allocMap.begin(), allocMap.end(), p,
				   [](const decltype(p) &itV, const decltype(p) &v){
					   return itV.second < v.first;
				   });
	BUG_ON(it == allocMap.end() || addr < it->first || addr > it->second);
	return it->first;
}

const NameInfo *Interpreter::getVarNameInfo(Value *v, StorageDuration sd, AddressSpace spc,
					    const VariableInfo<ModuleID::ID>::InternalKey &key /* = {} */)
{
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
	/* Don't complain if it's not allocated so that we can safely use it during error reporting */
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
	return (char *) staticValueMap.at(sBeg) + (addr.get() - sBeg.get());
}

std::unique_ptr<SExpr<unsigned int>> Interpreter::getCurrentAnnotConcretized()
{
	auto *l = ECStack().back().CurInst->getPrevNode();
	auto *annot = getAnnotation(l);
	if (!annot)
		return nullptr;

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
	return Concretizer().concretize(annot, vMap);
}

EventLabel::EventLabelKind
getReadKind(LoadInst &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return Kind::EL_Read;

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	if (GENMC_KIND(flags) == GENMC_KIND_SPECUL)
		return Kind::EL_SpeculativeRead;
	else if (GENMC_KIND(flags) == GENMC_KIND_CONFIRM)
		return Kind::EL_ConfirmingRead;
	BUG();
}

constexpr unsigned int switchPair(EventLabel::EventLabelKind a, EventLabel::EventLabelKind b)
{
	return (unsigned(a) << 16) + b;
}

constexpr unsigned int switchPair(std::pair<EventLabel::EventLabelKind, EventLabel::EventLabelKind> p)
{
	return switchPair(p.first, p.second);
}

std::pair<EventLabel::EventLabelKind, EventLabel::EventLabelKind>
getCasKinds(AtomicCmpXchgInst &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return std::make_pair(Kind::EL_CasRead, Kind::EL_CasWrite);

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	if (!GENMC_KIND(flags))
		return std::make_pair(Kind::EL_CasRead, Kind::EL_CasWrite);

	if (GENMC_KIND(flags) == GENMC_KIND_HELPED)
		return std::make_pair(Kind::EL_HelpedCasRead, Kind::EL_HelpedCasWrite);
	else if (GENMC_KIND(flags) == GENMC_KIND_HELPING)
		return std::make_pair(Kind::EL_HelpingCas, Kind::EL_HelpingCas);
	BUG_ON(GENMC_KIND(flags) != GENMC_KIND_CONFIRM);
	return std::make_pair(Kind::EL_ConfirmingCasRead, Kind::EL_ConfirmingCasWrite);
}

std::pair<EventLabel::EventLabelKind, EventLabel::EventLabelKind>
getFaiKinds(AtomicRMWInst &I)
{
	using Kind = EventLabel::EventLabelKind;

	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return std::make_pair(Kind::EL_FaiRead, Kind::EL_FaiWrite);

	auto *op = dyn_cast<ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = dyn_cast<ConstantInt>(op->getValue())->getZExtValue();
	if (GENMC_KIND(flags) == GENMC_KIND_NONVR)
		return std::make_pair(Kind::EL_NoRetFaiRead, Kind::EL_NoRetFaiWrite);
	BUG();
}

/* Returns the size (in bytes) for a given type */
unsigned int Interpreter::getTypeSize(Type *typ) const
{
	return (size_t) getDataLayout().getTypeAllocSize(typ);
}

void *Interpreter::getFileFromFd(int fd) const
{
	if (!dynState.fdToFile.inBounds(fd))
		return nullptr;
	return dynState.fdToFile[fd];
}

void Interpreter::setFdToFile(int fd, void *fileAddr)
{
	if (fd >= dynState.fdToFile.size())
		dynState.fdToFile.grow(fd);
	dynState.fdToFile[fd] = fileAddr;
}

void *Interpreter::getDirInode() const
{
	return MI->fsInfo.dirInode;
}

void *Interpreter::getInodeAddrFromName(const std::string &filename) const
{
	return dynState.nameToInodeAddr.at(filename);
}

/* Should match include/pthread.h (or barrier/mutex/thread decls) */
#define GENMC_PTHREAD_BARRIER_SERIAL_THREAD -1

/* Should match the definitions in include/unistd.h */
#define GENMC_SEEK_SET	0	/* Seek from beginning of file.  */
#define GENMC_SEEK_CUR	1	/* Seek from current position.  */
#define GENMC_SEEK_END	2	/* Seek from end of file.  */

/* Should match those in include/fcntl.h (and be in the valid list below) */
#define GENMC_O_RDONLY	00000000
#define GENMC_O_WRONLY	00000001
#define GENMC_O_RDWR	00000002
#define GENMC_O_CREAT	00000100
#define GENMC_O_TRUNC	00001000
#define GENMC_O_APPEND	00002000
#define GENMC_O_SYNC    04010000
#define GENMC_O_DSYNC	00010000

/* List of valid flags for the open flags argument -- FIXME: We do not support all flags */
#define GENMC_VALID_OPEN_FLAGS \
	(GENMC_O_RDONLY | GENMC_O_WRONLY | GENMC_O_RDWR | GENMC_O_CREAT | \
	 GENMC_O_TRUNC | GENMC_O_APPEND | GENMC_O_SYNC | GENMC_O_DSYNC	\
	 )

#define GENMC_O_ACCMODE 00000003
#define GENMC_ACC_MODE(x) ("\004\002\006\006"[(x)&GENMC_O_ACCMODE])
#define GENMC_OPEN_FMODE(flag) (((flag + 1) & GENMC_O_ACCMODE))

#define GENMC_FMODE_READ  0x1
#define GENMC_FMODE_WRITE 0x2

/* Fetching different fields of a file description (model @ include/unistd.h) */
#define GET_FILE_OFFSET_ADDR(file)					\
({								        \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.fileTyp);	\
        auto __off = (char *) file + SL->getElementOffset(4);		\
	__off;								\
})
#define GET_FILE_POS_LOCK_ADDR(file)				\
({							        \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.fileTyp);	\
	auto __lock = (char *) file + SL->getElementOffset(3);	\
	__lock;							\
})
#define GET_FILE_FLAGS_ADDR(file)					\
({								        \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.fileTyp);	\
        auto __flags = (char *) file + SL->getElementOffset(2);		\
	__flags;							\
})
#define GET_FILE_COUNT_ADDR(file)					\
({								        \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.fileTyp);	\
        auto __count = (char *) file + SL->getElementOffset(1);		\
	__count;							\
})
#define GET_FILE_INODE_ADDR(file)				\
({							        \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.fileTyp);	\
	auto __inode = (char *) file + SL->getElementOffset(0); \
	__inode;						\
})

/* Fetching different offsets of an inode */
#define GET_INODE_DATA_ADDR(inode)				\
({							        \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.inodeTyp);\
	auto __data = (char *) inode + SL->getElementOffset(4);	\
	__data;							\
})
#define GET_INODE_IDISKSIZE_ADDR(inode)					\
({							                \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.inodeTyp);\
	auto __disksize = (char *) inode + SL->getElementOffset(3);	\
	__disksize;							\
})
#define GET_INODE_ITRANSACTION_ADDR(inode)				\
({							                \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.inodeTyp);\
	auto __trans = (char *) inode + SL->getElementOffset(2);	\
	__trans;							\
})
#define GET_INODE_ISIZE_ADDR(inode)					\
({							                \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.inodeTyp);\
	auto __isize = (char *) inode + SL->getElementOffset(1);	\
	__isize;							\
})
#define GET_INODE_LOCK_ADDR(inode)					\
({								        \
	auto *SL = getDataLayout().getStructLayout(MI->fsInfo.inodeTyp);\
	auto __lock = (char *) inode + SL->getElementOffset(0);		\
	__lock;								\
})
#define GET_METADATA_MAPPING(inode)  (inode)
#define GET_DATA_MAPPING(inode)	     (inode)
#define GET_JOURNAL_MAPPING(inode)   (nullptr)

//===----------------------------------------------------------------------===//
//                    Binary Instruction Implementations
//===----------------------------------------------------------------------===//

#define IMPLEMENT_BINARY_OPERATOR(OP, TY) \
   case Type::TY##TyID: \
     Dest.TY##Val = Src1.TY##Val OP Src2.TY##Val; \
     break

static void executeFAddInst(GenericValue &Dest, GenericValue Src1,
                            GenericValue Src2, Type *Ty) {
  switch (Ty->getTypeID()) {
    IMPLEMENT_BINARY_OPERATOR(+, Float);
    IMPLEMENT_BINARY_OPERATOR(+, Double);
  default:
    dbgs() << "Unhandled type for FAdd instruction: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
}

static void executeFSubInst(GenericValue &Dest, GenericValue Src1,
                            GenericValue Src2, Type *Ty) {
  switch (Ty->getTypeID()) {
    IMPLEMENT_BINARY_OPERATOR(-, Float);
    IMPLEMENT_BINARY_OPERATOR(-, Double);
  default:
    dbgs() << "Unhandled type for FSub instruction: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
}

static void executeFMulInst(GenericValue &Dest, GenericValue Src1,
                            GenericValue Src2, Type *Ty) {
  switch (Ty->getTypeID()) {
    IMPLEMENT_BINARY_OPERATOR(*, Float);
    IMPLEMENT_BINARY_OPERATOR(*, Double);
  default:
    dbgs() << "Unhandled type for FMul instruction: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
}

static void executeFDivInst(GenericValue &Dest, GenericValue Src1,
                            GenericValue Src2, Type *Ty) {
  switch (Ty->getTypeID()) {
    IMPLEMENT_BINARY_OPERATOR(/, Float);
    IMPLEMENT_BINARY_OPERATOR(/, Double);
  default:
    dbgs() << "Unhandled type for FDiv instruction: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
}

static void executeFRemInst(GenericValue &Dest, GenericValue Src1,
                            GenericValue Src2, Type *Ty) {
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

#define IMPLEMENT_INTEGER_ICMP(OP, TY) \
   case Type::IntegerTyID:  \
      Dest.IntVal = APInt(1,Src1.IntVal.OP(Src2.IntVal)); \
      break;

#define IMPLEMENT_VECTOR_INTEGER_ICMP(OP, TY)                        \
  LLVM_VECTOR_TYPEID_CASES {					     \
    assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());    \
    Dest.AggregateVal.resize( Src1.AggregateVal.size() );            \
    for( uint32_t _i=0;_i<Src1.AggregateVal.size();_i++)             \
      Dest.AggregateVal[_i].IntVal = APInt(1,                        \
      Src1.AggregateVal[_i].IntVal.OP(Src2.AggregateVal[_i].IntVal));\
  } break;

// Handle pointers specially because they must be compared with only as much
// width as the host has.  We _do not_ want to be comparing 64 bit values when
// running on a 32-bit target, otherwise the upper 32 bits might mess up
// comparisons if they contain garbage.
#define IMPLEMENT_POINTER_ICMP(OP) \
   case Type::PointerTyID: \
      Dest.IntVal = APInt(1,(void*)(intptr_t)Src1.PointerVal OP \
                            (void*)(intptr_t)Src2.PointerVal); \
      break;

static GenericValue executeICMP_EQ(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(eq,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(eq,Ty);
    IMPLEMENT_POINTER_ICMP(==);
  default:
    dbgs() << "Unhandled type for ICMP_EQ predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_NE(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(ne,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(ne,Ty);
    IMPLEMENT_POINTER_ICMP(!=);
  default:
    dbgs() << "Unhandled type for ICMP_NE predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_ULT(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(ult,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(ult,Ty);
    IMPLEMENT_POINTER_ICMP(<);
  default:
    dbgs() << "Unhandled type for ICMP_ULT predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_SLT(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(slt,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(slt,Ty);
    IMPLEMENT_POINTER_ICMP(<);
  default:
    dbgs() << "Unhandled type for ICMP_SLT predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_UGT(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(ugt,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(ugt,Ty);
    IMPLEMENT_POINTER_ICMP(>);
  default:
    dbgs() << "Unhandled type for ICMP_UGT predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_SGT(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(sgt,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(sgt,Ty);
    IMPLEMENT_POINTER_ICMP(>);
  default:
    dbgs() << "Unhandled type for ICMP_SGT predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_ULE(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(ule,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(ule,Ty);
    IMPLEMENT_POINTER_ICMP(<=);
  default:
    dbgs() << "Unhandled type for ICMP_ULE predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_SLE(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(sle,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(sle,Ty);
    IMPLEMENT_POINTER_ICMP(<=);
  default:
    dbgs() << "Unhandled type for ICMP_SLE predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_UGE(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(uge,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(uge,Ty);
    IMPLEMENT_POINTER_ICMP(>=);
  default:
    dbgs() << "Unhandled type for ICMP_UGE predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

static GenericValue executeICMP_SGE(GenericValue Src1, GenericValue Src2,
                                    Type *Ty) {
  GenericValue Dest;
  switch (Ty->getTypeID()) {
    IMPLEMENT_INTEGER_ICMP(sge,Ty);
    IMPLEMENT_VECTOR_INTEGER_ICMP(sge,Ty);
    IMPLEMENT_POINTER_ICMP(>=);
  default:
    dbgs() << "Unhandled type for ICMP_SGE predicate: " << *Ty << "\n";
    llvm_unreachable(nullptr);
  }
  return Dest;
}

void Interpreter::visitICmpInst(ICmpInst &I) {
  ExecutionContext &SF = ECStack().back();
  Type *Ty    = I.getOperand(0)->getType();
  GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue R;   // Result

  updateDataDeps(getCurThr().id, &I, I.getOperand(0));
  updateDataDeps(getCurThr().id, &I, I.getOperand(1));

  switch (I.getPredicate()) {
  case ICmpInst::ICMP_EQ:  R = executeICMP_EQ(Src1,  Src2, Ty); break;
  case ICmpInst::ICMP_NE:  R = executeICMP_NE(Src1,  Src2, Ty); break;
  case ICmpInst::ICMP_ULT: R = executeICMP_ULT(Src1, Src2, Ty); break;
  case ICmpInst::ICMP_SLT: R = executeICMP_SLT(Src1, Src2, Ty); break;
  case ICmpInst::ICMP_UGT: R = executeICMP_UGT(Src1, Src2, Ty); break;
  case ICmpInst::ICMP_SGT: R = executeICMP_SGT(Src1, Src2, Ty); break;
  case ICmpInst::ICMP_ULE: R = executeICMP_ULE(Src1, Src2, Ty); break;
  case ICmpInst::ICMP_SLE: R = executeICMP_SLE(Src1, Src2, Ty); break;
  case ICmpInst::ICMP_UGE: R = executeICMP_UGE(Src1, Src2, Ty); break;
  case ICmpInst::ICMP_SGE: R = executeICMP_SGE(Src1, Src2, Ty); break;
  default:
    dbgs() << "Don't know how to handle this ICmp predicate!\n-->" << I;
    llvm_unreachable(nullptr);
  }

  SetValue(&I, R, SF);
}

#define IMPLEMENT_FCMP(OP, TY) \
   case Type::TY##TyID: \
     Dest.IntVal = APInt(1,Src1.TY##Val OP Src2.TY##Val); \
     break

#define IMPLEMENT_VECTOR_FCMP_T(OP, TY)                             \
  assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());     \
  Dest.AggregateVal.resize( Src1.AggregateVal.size() );             \
  for( uint32_t _i=0;_i<Src1.AggregateVal.size();_i++)              \
    Dest.AggregateVal[_i].IntVal = APInt(1,                         \
    Src1.AggregateVal[_i].TY##Val OP Src2.AggregateVal[_i].TY##Val);\
  break;

#define IMPLEMENT_VECTOR_FCMP(OP)                                   \
  LLVM_VECTOR_TYPEID_CASES					    \
    if(dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy()) {   \
      IMPLEMENT_VECTOR_FCMP_T(OP, Float);                           \
    } else {                                                        \
        IMPLEMENT_VECTOR_FCMP_T(OP, Double);                        \
    }

static GenericValue executeFCMP_OEQ(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
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

#define IMPLEMENT_SCALAR_NANS(TY, X,Y)                                      \
  if (TY->isFloatTy()) {                                                    \
    if (X.FloatVal != X.FloatVal || Y.FloatVal != Y.FloatVal) {             \
      Dest.IntVal = APInt(1,false);                                         \
      return Dest;                                                          \
    }                                                                       \
  } else {                                                                  \
    if (X.DoubleVal != X.DoubleVal || Y.DoubleVal != Y.DoubleVal) {         \
      Dest.IntVal = APInt(1,false);                                         \
      return Dest;                                                          \
    }                                                                       \
  }

#define MASK_VECTOR_NANS_T(X,Y, TZ, FLAG)                                   \
  assert(X.AggregateVal.size() == Y.AggregateVal.size());                   \
  Dest.AggregateVal.resize( X.AggregateVal.size() );                        \
  for( uint32_t _i=0;_i<X.AggregateVal.size();_i++) {                       \
    if (X.AggregateVal[_i].TZ##Val != X.AggregateVal[_i].TZ##Val ||         \
        Y.AggregateVal[_i].TZ##Val != Y.AggregateVal[_i].TZ##Val)           \
      Dest.AggregateVal[_i].IntVal = APInt(1,FLAG);                         \
    else  {                                                                 \
      Dest.AggregateVal[_i].IntVal = APInt(1,!FLAG);                        \
    }                                                                       \
  }

#define MASK_VECTOR_NANS(TY, X,Y, FLAG)                                     \
  if (TY->isVectorTy()) {                                                   \
    if (dyn_cast<VectorType>(TY)->getElementType()->isFloatTy()) {          \
      MASK_VECTOR_NANS_T(X, Y, Float, FLAG)                                 \
    } else {                                                                \
      MASK_VECTOR_NANS_T(X, Y, Double, FLAG)                                \
    }                                                                       \
  }                                                                         \



static GenericValue executeFCMP_ONE(GenericValue Src1, GenericValue Src2,
                                    Type *Ty)
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
    for( size_t _i=0; _i<Src1.AggregateVal.size(); _i++)
      if (DestMask.AggregateVal[_i].IntVal == false)
        Dest.AggregateVal[_i].IntVal = APInt(1,false);

  return Dest;
}

static GenericValue executeFCMP_OLE(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
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

static GenericValue executeFCMP_OGE(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
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

static GenericValue executeFCMP_OLT(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
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

static GenericValue executeFCMP_OGT(GenericValue Src1, GenericValue Src2,
                                     Type *Ty) {
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

#define IMPLEMENT_UNORDERED(TY, X,Y)                                     \
  if (TY->isFloatTy()) {                                                 \
    if (X.FloatVal != X.FloatVal || Y.FloatVal != Y.FloatVal) {          \
      Dest.IntVal = APInt(1,true);                                       \
      return Dest;                                                       \
    }                                                                    \
  } else if (X.DoubleVal != X.DoubleVal || Y.DoubleVal != Y.DoubleVal) { \
    Dest.IntVal = APInt(1,true);                                         \
    return Dest;                                                         \
  }

#define IMPLEMENT_VECTOR_UNORDERED(TY, X,Y, _FUNC)                       \
  if (TY->isVectorTy()) {                                                \
    GenericValue DestMask = Dest;                                        \
    Dest = _FUNC(Src1, Src2, Ty);                                        \
      for( size_t _i=0; _i<Src1.AggregateVal.size(); _i++)               \
        if (DestMask.AggregateVal[_i].IntVal == true)                    \
          Dest.AggregateVal[_i].IntVal = APInt(1,true);                  \
      return Dest;                                                       \
  }

static GenericValue executeFCMP_UEQ(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
  GenericValue Dest;
  IMPLEMENT_UNORDERED(Ty, Src1, Src2)
  MASK_VECTOR_NANS(Ty, Src1, Src2, true)
  IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OEQ)
  return executeFCMP_OEQ(Src1, Src2, Ty);

}

static GenericValue executeFCMP_UNE(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
  GenericValue Dest;
  IMPLEMENT_UNORDERED(Ty, Src1, Src2)
  MASK_VECTOR_NANS(Ty, Src1, Src2, true)
  IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_ONE)
  return executeFCMP_ONE(Src1, Src2, Ty);
}

static GenericValue executeFCMP_ULE(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
  GenericValue Dest;
  IMPLEMENT_UNORDERED(Ty, Src1, Src2)
  MASK_VECTOR_NANS(Ty, Src1, Src2, true)
  IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OLE)
  return executeFCMP_OLE(Src1, Src2, Ty);
}

static GenericValue executeFCMP_UGE(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
  GenericValue Dest;
  IMPLEMENT_UNORDERED(Ty, Src1, Src2)
  MASK_VECTOR_NANS(Ty, Src1, Src2, true)
  IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OGE)
  return executeFCMP_OGE(Src1, Src2, Ty);
}

static GenericValue executeFCMP_ULT(GenericValue Src1, GenericValue Src2,
                                   Type *Ty) {
  GenericValue Dest;
  IMPLEMENT_UNORDERED(Ty, Src1, Src2)
  MASK_VECTOR_NANS(Ty, Src1, Src2, true)
  IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OLT)
  return executeFCMP_OLT(Src1, Src2, Ty);
}

static GenericValue executeFCMP_UGT(GenericValue Src1, GenericValue Src2,
                                     Type *Ty) {
  GenericValue Dest;
  IMPLEMENT_UNORDERED(Ty, Src1, Src2)
  MASK_VECTOR_NANS(Ty, Src1, Src2, true)
  IMPLEMENT_VECTOR_UNORDERED(Ty, Src1, Src2, executeFCMP_OGT)
  return executeFCMP_OGT(Src1, Src2, Ty);
}

static GenericValue executeFCMP_ORD(GenericValue Src1, GenericValue Src2,
                                     Type *Ty) {
  GenericValue Dest;
  if(Ty->isVectorTy()) {
    assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
    Dest.AggregateVal.resize( Src1.AggregateVal.size() );
    if(dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy()) {
      for( size_t _i=0;_i<Src1.AggregateVal.size();_i++)
        Dest.AggregateVal[_i].IntVal = APInt(1,
        ( (Src1.AggregateVal[_i].FloatVal ==
        Src1.AggregateVal[_i].FloatVal) &&
        (Src2.AggregateVal[_i].FloatVal ==
        Src2.AggregateVal[_i].FloatVal)));
    } else {
      for( size_t _i=0;_i<Src1.AggregateVal.size();_i++)
        Dest.AggregateVal[_i].IntVal = APInt(1,
        ( (Src1.AggregateVal[_i].DoubleVal ==
        Src1.AggregateVal[_i].DoubleVal) &&
        (Src2.AggregateVal[_i].DoubleVal ==
        Src2.AggregateVal[_i].DoubleVal)));
    }
  } else if (Ty->isFloatTy())
    Dest.IntVal = APInt(1,(Src1.FloatVal == Src1.FloatVal &&
                           Src2.FloatVal == Src2.FloatVal));
  else {
    Dest.IntVal = APInt(1,(Src1.DoubleVal == Src1.DoubleVal &&
                           Src2.DoubleVal == Src2.DoubleVal));
  }
  return Dest;
}

static GenericValue executeFCMP_UNO(GenericValue Src1, GenericValue Src2,
                                     Type *Ty) {
  GenericValue Dest;
  if(Ty->isVectorTy()) {
    assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
    Dest.AggregateVal.resize( Src1.AggregateVal.size() );
    if(dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy()) {
      for( size_t _i=0;_i<Src1.AggregateVal.size();_i++)
        Dest.AggregateVal[_i].IntVal = APInt(1,
        ( (Src1.AggregateVal[_i].FloatVal !=
           Src1.AggregateVal[_i].FloatVal) ||
          (Src2.AggregateVal[_i].FloatVal !=
           Src2.AggregateVal[_i].FloatVal)));
      } else {
        for( size_t _i=0;_i<Src1.AggregateVal.size();_i++)
          Dest.AggregateVal[_i].IntVal = APInt(1,
          ( (Src1.AggregateVal[_i].DoubleVal !=
             Src1.AggregateVal[_i].DoubleVal) ||
            (Src2.AggregateVal[_i].DoubleVal !=
             Src2.AggregateVal[_i].DoubleVal)));
      }
  } else if (Ty->isFloatTy())
    Dest.IntVal = APInt(1,(Src1.FloatVal != Src1.FloatVal ||
                           Src2.FloatVal != Src2.FloatVal));
  else {
    Dest.IntVal = APInt(1,(Src1.DoubleVal != Src1.DoubleVal ||
                           Src2.DoubleVal != Src2.DoubleVal));
  }
  return Dest;
}

static GenericValue executeFCMP_BOOL(GenericValue Src1, GenericValue Src2,
                                    const Type *Ty, const bool val) {
  GenericValue Dest;
    if(Ty->isVectorTy()) {
      assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
      Dest.AggregateVal.resize( Src1.AggregateVal.size() );
      for( size_t _i=0; _i<Src1.AggregateVal.size(); _i++)
        Dest.AggregateVal[_i].IntVal = APInt(1,val);
    } else {
      Dest.IntVal = APInt(1, val);
    }

    return Dest;
}

void Interpreter::visitFCmpInst(FCmpInst &I) {
  ExecutionContext &SF = ECStack().back();
  Type *Ty    = I.getOperand(0)->getType();
  GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue R;   // Result

  switch (I.getPredicate()) {
  default:
    dbgs() << "Don't know how to handle this FCmp predicate!\n-->" << I;
    llvm_unreachable(nullptr);
  break;
  case FCmpInst::FCMP_FALSE: R = executeFCMP_BOOL(Src1, Src2, Ty, false);
  break;
  case FCmpInst::FCMP_TRUE:  R = executeFCMP_BOOL(Src1, Src2, Ty, true);
  break;
  case FCmpInst::FCMP_ORD:   R = executeFCMP_ORD(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_UNO:   R = executeFCMP_UNO(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_UEQ:   R = executeFCMP_UEQ(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_OEQ:   R = executeFCMP_OEQ(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_UNE:   R = executeFCMP_UNE(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_ONE:   R = executeFCMP_ONE(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_ULT:   R = executeFCMP_ULT(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_OLT:   R = executeFCMP_OLT(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_UGT:   R = executeFCMP_UGT(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_OGT:   R = executeFCMP_OGT(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_ULE:   R = executeFCMP_ULE(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_OLE:   R = executeFCMP_OLE(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_UGE:   R = executeFCMP_UGE(Src1, Src2, Ty); break;
  case FCmpInst::FCMP_OGE:   R = executeFCMP_OGE(Src1, Src2, Ty); break;
  }

  SetValue(&I, R, SF);
}

static GenericValue executeCmpInst(unsigned predicate, GenericValue Src1,
                                   GenericValue Src2, Type *Ty) {
  GenericValue Result;
  switch (predicate) {
  case ICmpInst::ICMP_EQ:    return executeICMP_EQ(Src1, Src2, Ty);
  case ICmpInst::ICMP_NE:    return executeICMP_NE(Src1, Src2, Ty);
  case ICmpInst::ICMP_UGT:   return executeICMP_UGT(Src1, Src2, Ty);
  case ICmpInst::ICMP_SGT:   return executeICMP_SGT(Src1, Src2, Ty);
  case ICmpInst::ICMP_ULT:   return executeICMP_ULT(Src1, Src2, Ty);
  case ICmpInst::ICMP_SLT:   return executeICMP_SLT(Src1, Src2, Ty);
  case ICmpInst::ICMP_UGE:   return executeICMP_UGE(Src1, Src2, Ty);
  case ICmpInst::ICMP_SGE:   return executeICMP_SGE(Src1, Src2, Ty);
  case ICmpInst::ICMP_ULE:   return executeICMP_ULE(Src1, Src2, Ty);
  case ICmpInst::ICMP_SLE:   return executeICMP_SLE(Src1, Src2, Ty);
  case FCmpInst::FCMP_ORD:   return executeFCMP_ORD(Src1, Src2, Ty);
  case FCmpInst::FCMP_UNO:   return executeFCMP_UNO(Src1, Src2, Ty);
  case FCmpInst::FCMP_OEQ:   return executeFCMP_OEQ(Src1, Src2, Ty);
  case FCmpInst::FCMP_UEQ:   return executeFCMP_UEQ(Src1, Src2, Ty);
  case FCmpInst::FCMP_ONE:   return executeFCMP_ONE(Src1, Src2, Ty);
  case FCmpInst::FCMP_UNE:   return executeFCMP_UNE(Src1, Src2, Ty);
  case FCmpInst::FCMP_OLT:   return executeFCMP_OLT(Src1, Src2, Ty);
  case FCmpInst::FCMP_ULT:   return executeFCMP_ULT(Src1, Src2, Ty);
  case FCmpInst::FCMP_OGT:   return executeFCMP_OGT(Src1, Src2, Ty);
  case FCmpInst::FCMP_UGT:   return executeFCMP_UGT(Src1, Src2, Ty);
  case FCmpInst::FCMP_OLE:   return executeFCMP_OLE(Src1, Src2, Ty);
  case FCmpInst::FCMP_ULE:   return executeFCMP_ULE(Src1, Src2, Ty);
  case FCmpInst::FCMP_OGE:   return executeFCMP_OGE(Src1, Src2, Ty);
  case FCmpInst::FCMP_UGE:   return executeFCMP_UGE(Src1, Src2, Ty);
  case FCmpInst::FCMP_FALSE: return executeFCMP_BOOL(Src1, Src2, Ty, false);
  case FCmpInst::FCMP_TRUE:  return executeFCMP_BOOL(Src1, Src2, Ty, true);
  default:
    dbgs() << "Unhandled Cmp predicate\n";
    llvm_unreachable(nullptr);
  }
}

void Interpreter::visitBinaryOperator(BinaryOperator &I) {
  ExecutionContext &SF = ECStack().back();
  Type *Ty    = I.getOperand(0)->getType();
  GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue R;   // Result

  /* Update dependencies */
  updateDataDeps(getCurThr().id, &I, I.getOperand(0));
  updateDataDeps(getCurThr().id, &I, I.getOperand(1));

  // First process vector operation
  if (Ty->isVectorTy()) {
    assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
    R.AggregateVal.resize(Src1.AggregateVal.size());

    // Macros to execute binary operation 'OP' over integer vectors
#define INTEGER_VECTOR_OPERATION(OP)                               \
    for (unsigned i = 0; i < R.AggregateVal.size(); ++i)           \
      R.AggregateVal[i].IntVal =                                   \
      Src1.AggregateVal[i].IntVal OP Src2.AggregateVal[i].IntVal;

    // Additional macros to execute binary operations udiv/sdiv/urem/srem since
    // they have different notation.
#define INTEGER_VECTOR_FUNCTION(OP)                                \
    for (unsigned i = 0; i < R.AggregateVal.size(); ++i)           \
      R.AggregateVal[i].IntVal =                                   \
      Src1.AggregateVal[i].IntVal.OP(Src2.AggregateVal[i].IntVal);

    // Macros to execute binary operation 'OP' over floating point type TY
    // (float or double) vectors
#define FLOAT_VECTOR_FUNCTION(OP, TY)                               \
      for (unsigned i = 0; i < R.AggregateVal.size(); ++i)          \
        R.AggregateVal[i].TY =                                      \
        Src1.AggregateVal[i].TY OP Src2.AggregateVal[i].TY;

    // Macros to choose appropriate TY: float or double and run operation
    // execution
#define FLOAT_VECTOR_OP(OP) {                                         \
  if (dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy())        \
    FLOAT_VECTOR_FUNCTION(OP, FloatVal)                               \
  else {                                                              \
    if (dyn_cast<VectorType>(Ty)->getElementType()->isDoubleTy())     \
      FLOAT_VECTOR_FUNCTION(OP, DoubleVal)                            \
    else {                                                            \
      dbgs() << "Unhandled type for OP instruction: " << *Ty << "\n"; \
      llvm_unreachable(0);                                            \
    }                                                                 \
  }                                                                   \
}

    switch(I.getOpcode()){
    default:
      dbgs() << "Don't know how to handle this binary operator!\n-->" << I;
      llvm_unreachable(nullptr);
      break;
    case Instruction::Add:   INTEGER_VECTOR_OPERATION(+) break;
    case Instruction::Sub:   INTEGER_VECTOR_OPERATION(-) break;
    case Instruction::Mul:   INTEGER_VECTOR_OPERATION(*) break;
    case Instruction::UDiv:  INTEGER_VECTOR_FUNCTION(udiv) break;
    case Instruction::SDiv:  INTEGER_VECTOR_FUNCTION(sdiv) break;
    case Instruction::URem:  INTEGER_VECTOR_FUNCTION(urem) break;
    case Instruction::SRem:  INTEGER_VECTOR_FUNCTION(srem) break;
    case Instruction::And:   INTEGER_VECTOR_OPERATION(&) break;
    case Instruction::Or:    INTEGER_VECTOR_OPERATION(|) break;
    case Instruction::Xor:   INTEGER_VECTOR_OPERATION(^) break;
    case Instruction::FAdd:  FLOAT_VECTOR_OP(+) break;
    case Instruction::FSub:  FLOAT_VECTOR_OP(-) break;
    case Instruction::FMul:  FLOAT_VECTOR_OP(*) break;
    case Instruction::FDiv:  FLOAT_VECTOR_OP(/) break;
    case Instruction::FRem:
      if (dyn_cast<VectorType>(Ty)->getElementType()->isFloatTy())
        for (unsigned i = 0; i < R.AggregateVal.size(); ++i)
          R.AggregateVal[i].FloatVal =
          fmod(Src1.AggregateVal[i].FloatVal, Src2.AggregateVal[i].FloatVal);
      else {
        if (dyn_cast<VectorType>(Ty)->getElementType()->isDoubleTy())
          for (unsigned i = 0; i < R.AggregateVal.size(); ++i)
            R.AggregateVal[i].DoubleVal =
            fmod(Src1.AggregateVal[i].DoubleVal, Src2.AggregateVal[i].DoubleVal);
        else {
          dbgs() << "Unhandled type for Rem instruction: " << *Ty << "\n";
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
    case Instruction::Add:   R.IntVal = Src1.IntVal + Src2.IntVal; break;
    case Instruction::Sub:   R.IntVal = Src1.IntVal - Src2.IntVal; break;
    case Instruction::Mul:   R.IntVal = Src1.IntVal * Src2.IntVal; break;
    case Instruction::FAdd:  executeFAddInst(R, Src1, Src2, Ty); break;
    case Instruction::FSub:  executeFSubInst(R, Src1, Src2, Ty); break;
    case Instruction::FMul:  executeFMulInst(R, Src1, Src2, Ty); break;
    case Instruction::FDiv:  executeFDivInst(R, Src1, Src2, Ty); break;
    case Instruction::FRem:  executeFRemInst(R, Src1, Src2, Ty); break;
    case Instruction::UDiv:  R.IntVal = Src1.IntVal.udiv(Src2.IntVal); break;
    case Instruction::SDiv:  R.IntVal = Src1.IntVal.sdiv(Src2.IntVal); break;
    case Instruction::URem:  R.IntVal = Src1.IntVal.urem(Src2.IntVal); break;
    case Instruction::SRem:  R.IntVal = Src1.IntVal.srem(Src2.IntVal); break;
    case Instruction::And:   R.IntVal = Src1.IntVal & Src2.IntVal; break;
    case Instruction::Or:    R.IntVal = Src1.IntVal | Src2.IntVal; break;
    case Instruction::Xor:   R.IntVal = Src1.IntVal ^ Src2.IntVal; break;
    }
  }
  SetValue(&I, R, SF);
}

static GenericValue executeSelectInst(GenericValue Src1, GenericValue Src2,
                                      GenericValue Src3, const Type *Ty) {
    GenericValue Dest;
    if(Ty->isVectorTy()) {
      assert(Src1.AggregateVal.size() == Src2.AggregateVal.size());
      assert(Src2.AggregateVal.size() == Src3.AggregateVal.size());
      Dest.AggregateVal.resize( Src1.AggregateVal.size() );
      for (size_t i = 0; i < Src1.AggregateVal.size(); ++i)
        Dest.AggregateVal[i] = (Src1.AggregateVal[i].IntVal == 0) ?
          Src3.AggregateVal[i] : Src2.AggregateVal[i];
    } else {
      Dest = (Src1.IntVal == 0) ? Src3 : Src2;
    }
    return Dest;
}

void Interpreter::visitSelectInst(SelectInst &I) {
  ExecutionContext &SF = ECStack().back();
  const Type * Ty = I.getOperand(0)->getType();
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
		CALL_DRIVER(handleFree, FreeLabel::create(currPos(), *it, GET_DEPS(deps)));
}

void Interpreter::exitCalled(GenericValue GV) {
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
void Interpreter::popStackAndReturnValueToCaller(Type *RetTy,
                                                 GenericValue Result,
						 ReturnInst *retI /* = nullptr */) {
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
	    CALL_DRIVER(handleThreadFinish, ThreadFinishLabel::create(
				currPos(), GV_TO_SVAL(Result, RetTy)));
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
      if (InvokeInst *II = dyn_cast<InvokeInst> (I))
        SwitchToNewBasicBlock (II->getNormalDest (), CallingSF);
      CallingSF.Caller = CallInstWrapper();          // We returned from the call...
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
		if (InvokeInst *II = dyn_cast<InvokeInst> (I))
			SwitchToNewBasicBlock (II->getNormalDest (), CallingSF);
		CallingSF.Caller = CallInstWrapper();          // We returned from the call...
	}
}

void Interpreter::visitReturnInst(ReturnInst &I) {
  ExecutionContext &SF = ECStack().back();
  Type *RetTy = Type::getVoidTy(I.getContext());
  GenericValue Result;

  // Save away the return value... (if we are not 'ret void')
  if (I.getNumOperands()) {
    RetTy  = I.getReturnValue()->getType();
    Result = getOperandValue(I.getReturnValue(), SF);
  }

  popStackAndReturnValueToCaller(RetTy, Result, &I);
}

void Interpreter::visitUnreachableInst(UnreachableInst &I) {
  report_fatal_error("Program executed an 'unreachable' instruction!");
}

void Interpreter::visitBranchInst(BranchInst &I) {
  ExecutionContext &SF = ECStack().back();
  BasicBlock *Dest;

  Dest = I.getSuccessor(0);          // Uncond branches have a fixed dest...
  if (!I.isUnconditional()) {
    Value *Cond = I.getCondition();
    if (getOperandValue(Cond, SF).IntVal == 0) // If false cond...
      Dest = I.getSuccessor(1);
    updateCtrlDeps(getCurThr().id, Cond);
  }
  SwitchToNewBasicBlock(Dest, SF);
}

void Interpreter::visitSwitchInst(SwitchInst &I) {
  ExecutionContext &SF = ECStack().back();
  Value* Cond = I.getCondition();
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
  if (!Dest) Dest = I.getDefaultDest();   // No cases matched: use default
  updateCtrlDeps(getCurThr().id, Cond);
  SwitchToNewBasicBlock(Dest, SF);
}

void Interpreter::visitIndirectBrInst(IndirectBrInst &I) {
  ExecutionContext &SF = ECStack().back();
  void *Dest = GVTOP(getOperandValue(I.getAddress(), SF));
  updateCtrlDeps(getCurThr().id, I.getAddress());
  SwitchToNewBasicBlock((BasicBlock*)Dest, SF);
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
void Interpreter::SwitchToNewBasicBlock(BasicBlock *Dest, ExecutionContext &SF){
  BasicBlock *PrevBB = SF.CurBB;      // Remember where we came from...
  SF.CurBB   = Dest;                  // Update CurBB to branch destination
  SF.CurInst = SF.CurBB->begin();     // Update new instruction ptr...

  if (!isa<PHINode>(SF.CurInst)) return;  // Nothing fancy to do

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

void Interpreter::visitAllocaInst(AllocaInst &I) {
  ExecutionContext &SF = ECStack().back();

  Type *Ty = I.getAllocatedType();  // Type to be allocated

  // Get the number of elements being allocated by the array...
  unsigned NumElements =
    getOperandValue(I.getOperand(0), SF).IntVal.getZExtValue();

  unsigned TypeSize = (size_t)getDataLayout().getTypeAllocSize(Ty);

  // Avoid malloc-ing zero bytes, use max()...
  unsigned MemToAlloc = std::max(1U, NumElements * TypeSize);

  /* The driver will provide the address this alloca returns */
  auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
			    getAddrPoDeps(getCurThr().id), nullptr);

  auto *info = getVarNameInfo(&I, StorageDuration::SD_Automatic, AddressSpace::AS_User);
  SVal result = CALL_DRIVER(handleMalloc,
			   MallocLabel::create(currPos(), MemToAlloc,
#if LLVM_VERSION_MAJOR >= 11
					       I.getAlign().value(),
#else
					       I.getAlignment(),
#endif
					       StorageDuration::SD_Automatic,
					       StorageType::ST_Volatile, AddressSpace::AS_User,
					       info, I.getName().str(), GET_DEPS(deps)));

  ECStack().back().Allocas.add((void *) result.get());

  updateDataDeps(getCurThr().id, &I, Event(getCurThr().id, getCurThr().globalInstructions));
  SetValue(&I, SVAL_TO_GV(result, I.getType()), SF);
}

// getElementOffset - The workhorse for getelementptr.
//
GenericValue Interpreter::executeGEPOperation(Value *Ptr, gep_type_iterator I,
                                              gep_type_iterator E,
                                              ExecutionContext &SF) {
  assert(Ptr->getType()->isPointerTy() &&
         "Cannot getElementOffset of a nonpointer type!");

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
  Result.PointerVal = ((char*)getOperandValue(Ptr, SF).PointerVal) + Total;
  // DEBUG(dbgs() << "GEP Index " << Total << " bytes.\n");
  return Result;
}

void Interpreter::visitGetElementPtrInst(GetElementPtrInst &I) {
  ExecutionContext &SF = ECStack().back();
  SetValue(&I, executeGEPOperation(I.getPointerOperand(),
				    gep_type_begin(I), gep_type_end(I), SF), SF);
}

void Interpreter::visitLoadInst(LoadInst &I)
{
	Thread &thr = getCurThr();
	GenericValue src = getOperandValue(I.getPointerOperand(), ECStack().back());
	GenericValue *ptr = (GenericValue *) GVTOP(src);
	Type *typ = I.getType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

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
#define IMPLEMENT_READ_VISIT(__kind)					\
	case EventLabel::EventLabelKind::EL_ ## __kind: {		\
		val = CALL_DRIVER_RESET_IF_NONE(handleLoad,		\
				 __kind ## Label::create(		\
					 currPos(), I.getOrdering(), ptr, \
					 size, atyp, GET_DEPS(deps)));		\
		break;							\
	}

	std::optional<SVal> val;
	switch (getReadKind(I)) {
		IMPLEMENT_READ_VISIT(Read);
		IMPLEMENT_READ_VISIT(SpeculativeRead);
		IMPLEMENT_READ_VISIT(ConfirmingRead);
	default:
		BUG();
	}
	if (!val.has_value())
		return;

	updateDataDeps(thr.id, &I, currPos());
	updateAddrPoDeps(thr.id, I.getPointerOperand());

	/* Last, set the return value for this instruction */
	SetValue(&I, SVAL_TO_GV(val.value(), typ), ECStack().back());
	return;
}

void Interpreter::visitStoreInst(StoreInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	Type *typ = I.getOperand(0)->getType();
	GenericValue val = getOperandValue(I.getOperand(0), SF);
	auto src = getOperandValue(I.getPointerOperand(), SF);
	auto *ptr = (GenericValue *) GVTOP(src);
	auto asize = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

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
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), I.getOrdering(), ptr, asize, atyp,
				       GV_TO_SVAL(val, typ), getWriteAttr(I), GET_DEPS(deps)));
	updateAddrPoDeps(getCurThr().id, I.getPointerOperand());
	return;
}

void Interpreter::visitFenceInst(FenceInst &I)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id), nullptr, nullptr);
	CALL_DRIVER(handleFence, FenceLabel::create(currPos(), I.getOrdering(), GET_DEPS(deps)));
	return;
}

void Interpreter::visitAtomicCmpXchgInst(AtomicCmpXchgInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	Type *typ = I.getCompareOperand()->getType();
	GenericValue cmpVal = getOperandValue(I.getCompareOperand(), SF);
	GenericValue newVal = getOperandValue(I.getNewValOperand(), SF);
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	auto *ptr = (GenericValue *) GVTOP(src);
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
				  getDataDeps(thr.id, I.getNewValOperand()),
				  getCtrlDeps(thr.id), getAddrPoDeps(thr.id),
				  getDataDeps(thr.id, I.getCompareOperand()));

#define IMPLEMENT_CAS_VISIT(nameR, nameW)				\
	case switchPair(EventLabel::EventLabelKind::EL_ ## nameR, EventLabel::EventLabelKind::EL_ ## nameW): { \
		ret = CALL_DRIVER_RESET_IF_NONE(handleLoad, nameR ## Label::create( \
			currPos(), I.getSuccessOrdering(), ptr, 	\
			size, atyp, GV_TO_SVAL(cmpVal, typ),		\
			GV_TO_SVAL(newVal, typ), getWriteAttr(I), GET_DEPS(lDeps))); \
		if (!ret.has_value())					\
			return;						\
		cmpRes = *ret == GV_TO_SVAL(cmpVal, typ);		\
		updateDataDeps(getCurThr().id, &I, currPos());		\
		updateAddrPoDeps(getCurThr().id, I.getPointerOperand());\
		if (!getCurThr().isBlocked() && cmpRes) {	\
			auto sDeps = makeEventDeps(getDataDeps(getCurThr().id, I.getPointerOperand()), \
						   getDataDeps(getCurThr().id, I.getNewValOperand()), \
						   getCtrlDeps(getCurThr().id), getAddrPoDeps(thr.id), nullptr); \
			CALL_DRIVER(handleStore, nameW ## Label::create( \
				currPos(), I.getSuccessOrdering(), ptr, size, \
				atyp, GV_TO_SVAL(newVal, typ), getWriteAttr(I), GET_DEPS(sDeps))); \
		}							\
		break;							\
	}

	/* Check whether this is some special CAS; in such cases we also need a snapshot */
        std::optional<SVal> ret;
	int cmpRes;
	switch (switchPair(getCasKinds(I))) {
		IMPLEMENT_CAS_VISIT(CasRead, CasWrite);
		IMPLEMENT_CAS_VISIT(HelpedCasRead, HelpedCasWrite);
		IMPLEMENT_CAS_VISIT(ConfirmingCasRead, ConfirmingCasWrite);
		case switchPair(EventLabel::EL_HelpingCas, EventLabel::EL_HelpingCas):
			if (!CALL_DRIVER_RESET_IF_FALSE(handleHelpingCas, HelpingCasLabel::create(
								currPos(), I.getSuccessOrdering(), ptr,
								size, atyp, GV_TO_SVAL(cmpVal, typ),
								GV_TO_SVAL(newVal, typ), GET_DEPS(lDeps))))
				return;
			break;
	default:
		BUG();
	}

	result.AggregateVal.push_back(SVAL_TO_GV(*ret, typ));
	result.AggregateVal.push_back(INT_TO_GV(Type::getInt1Ty(I.getContext()), cmpRes));
	SetValue(&I, result, ECStack().back());
	return;
}

SVal Interpreter::executeAtomicRMWOperation(SVal oldVal, SVal val, ASize size, AtomicRMWInst::BinOp op)
{
	switch (op) {
	case AtomicRMWInst::Xchg:
		WARN_ON_ONCE(getDepTracker(), "unsupported-xchg-deps",
			     "Atomic xchg support is experimental under dependency-tracking models!\n");
		return val;
	case AtomicRMWInst::Add:
		return (oldVal + val);
	case AtomicRMWInst::Sub:
		return (oldVal - val);
	case AtomicRMWInst::And:
		return oldVal & val;
	case AtomicRMWInst::Nand:
		return ~(oldVal & val);
	case AtomicRMWInst::Or:
		return oldVal | val;
	case AtomicRMWInst::Xor:
		return oldVal ^ val;
	case AtomicRMWInst::Max:
		return SVal(oldVal).signExtendBottom(size.getBits()).sgt(SVal(val).signExtendBottom(size.getBits())) ? oldVal : val;
	case AtomicRMWInst::Min:
		return SVal(oldVal).signExtendBottom(size.getBits()).slt(SVal(val).signExtendBottom(size.getBits())) ? oldVal : val;
	case AtomicRMWInst::UMax:
		return oldVal.ugt(val) ? oldVal : val;
	case AtomicRMWInst::UMin:
		return oldVal.ult(val) ? oldVal : val;
	default:
		WARN_ONCE("invalid-rmw-op",
			  "Unsupported operation in RMW instruction!\n");
		return val;
	}
}

void Interpreter::visitAtomicRMWInst(AtomicRMWInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	auto *ptr = (GenericValue *) GVTOP(src);
	Type *typ = I.getType();
	auto val = GV_TO_SVAL(getOperandValue(I.getValOperand(), SF), typ);
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	BUG_ON(!typ->isIntegerTy());

	if (thr.tls.count(ptr)) {
		GenericValue oldVal = thr.tls[ptr];
		auto newVal = executeAtomicRMWOperation(GV_TO_SVAL(oldVal, typ),
							val, size, I.getOperation());
		thr.tls[ptr] = SVAL_TO_GV(newVal, typ);
		SetValue(&I, oldVal, SF);
		return;
	}

	auto deps = makeEventDeps(getDataDeps(thr.id, I.getPointerOperand()),
				  getDataDeps(thr.id, I.getValOperand()),
				  getCtrlDeps(thr.id), getAddrPoDeps(thr.id), nullptr);

#define IMPLEMENT_FAI_VISIT(nameR, nameW)				\
	case switchPair(EventLabel::EventLabelKind::EL_ ## nameR, EventLabel::EventLabelKind::EL_ ## nameW): { \
		ret = CALL_DRIVER_RESET_IF_NONE(handleLoad, nameR ## Label::create( \
					  currPos(), I.getOrdering(), ptr, size, atyp, \
					  I.getOperation(), val, getWriteAttr(I), GET_DEPS(deps))); \
		if (!ret.has_value())					\
			return;						\
		updateDataDeps(getCurThr().id, &I, currPos());		\
		updateAddrPoDeps(getCurThr().id, I.getPointerOperand()); \
		auto newVal = executeAtomicRMWOperation(*ret, val, size, I.getOperation()); \
		if (!getCurThr().isBlocked())	{			\
			CALL_DRIVER(handleStore, 		\
				    nameW ## Label::create(currPos(), I.getOrdering(), ptr, size, \
							   atyp, newVal, getWriteAttr(I), GET_DEPS(deps))); \
		}							\
		break;							\
	}

	/* Check whether this is a special FAI */
	std::optional<SVal> ret;
	switch (switchPair(getFaiKinds(I))) {
		IMPLEMENT_FAI_VISIT(FaiRead, FaiWrite);
		IMPLEMENT_FAI_VISIT(NoRetFaiRead, NoRetFaiWrite);
	default:
		BUG();
	}

	SetValue(&I, SVAL_TO_GV(*ret, typ), ECStack().back());
	return;
}

bool Interpreter::isInlineAsm(CallInstWrapper CIW, std::string *asmStr)
{
	llvm::CallInst *CI = dyn_cast<CallInst>(&CIW);
	if (!CI || !CI->isInlineAsm())
		return false;

	llvm::InlineAsm *IA = llvm::dyn_cast<llvm::InlineAsm>(CIW.getCalledOperand());
	*asmStr = IA->getAsmString();
	asmStr->erase(asmStr->begin(), std::find_if(asmStr->begin(), asmStr->end(),
						    [](int c){ return !std::isspace(c); }));
	asmStr->erase(std::find_if(asmStr->rbegin(), asmStr->rend(),
				   [](int c){ return !std::isspace(c); }).base(), asmStr->end());
	return true;
}

void Interpreter::visitInlineAsm(CallInstWrapper CIW, const std::string &asmStr)
{
	if (asmStr == "")
		; /* Plain compiler fence */
	else
		WARN_ONCE("invalid-inline-asm",
			  "Arbitrary inline assembly not supported: " +
			  asmStr + "! Skipping...\n");
	return;
}

//===----------------------------------------------------------------------===//
//                 Miscellaneous Instruction Implementations
//===----------------------------------------------------------------------===//

void Interpreter::visitCallInstWrapper(CallInstWrapper CS) {

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
    case Intrinsic::vaend:    // va_end is a noop for the interpreter
      return;
    case Intrinsic::vacopy:   // va_copy: dest = src
      SetValue(&CS, getOperandValue(*CS.arg_begin(), SF), SF);
      return;
    default:
	    WARN_ONCE("unknown-intrinsic", "Unknown intrinstic function" \
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
  for (auto i = SF.Caller.arg_begin(),
       e = SF.Caller.arg_end(); i != e; ++i, ++pNum) {
    Value *V = *i;
    ArgVals.push_back(getOperandValue(V, SF));
  }

  // To handle indirect calls, we must get the pointer value from the argument
  // and treat it as a function pointer.
  GenericValue SRC = getOperandValue(SF.Caller.getCalledOperand(), SF);
  auto specialDeps = updateFunArgDeps(getCurThr().id, (Function *) GVTOP(SRC));
  callFunction((Function*)GVTOP(SRC), ArgVals, specialDeps);
  updateInternalFunRetDeps(getCurThr().id, (Function *) GVTOP(SRC), &CS);
}

// auxiliary function for shift operations
static unsigned getShiftAmount(uint64_t orgShiftAmount,
                               llvm::APInt valueToShift) {
  unsigned valueWidth = valueToShift.getBitWidth();
  if (orgShiftAmount < (uint64_t)valueWidth)
    return orgShiftAmount;
  // according to the llvm documentation, if orgShiftAmount > valueWidth,
  // the result is undfeined. but we do shift by this rule:
  return (NextPowerOf2(valueWidth-1) - 1) & orgShiftAmount;
}


void Interpreter::visitShl(BinaryOperator &I) {
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

void Interpreter::visitLShr(BinaryOperator &I) {
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
      Result.IntVal = valueToShift.lshr(getShiftAmount(shiftAmount, valueToShift));
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

void Interpreter::visitAShr(BinaryOperator &I) {
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
      Result.IntVal = valueToShift.ashr(getShiftAmount(shiftAmount, valueToShift));
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

GenericValue Interpreter::executeTruncInst(Value *SrcVal, Type *DstTy,
                                           ExecutionContext &SF) {
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

GenericValue Interpreter::executeSExtInst(Value *SrcVal, Type *DstTy,
                                          ExecutionContext &SF) {
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

GenericValue Interpreter::executeZExtInst(Value *SrcVal, Type *DstTy,
                                          ExecutionContext &SF) {
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

GenericValue Interpreter::executeFPTruncInst(Value *SrcVal, Type *DstTy,
                                             ExecutionContext &SF) {
  GenericValue Dest, Src = getOperandValue(SrcVal, SF);

  if (isa<VectorType>(SrcVal->getType())) {
    assert(SrcVal->getType()->getScalarType()->isDoubleTy() &&
           DstTy->getScalarType()->isFloatTy() &&
           "Invalid FPTrunc instruction");

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

GenericValue Interpreter::executeFPExtInst(Value *SrcVal, Type *DstTy,
                                           ExecutionContext &SF) {
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

GenericValue Interpreter::executeFPToUIInst(Value *SrcVal, Type *DstTy,
                                            ExecutionContext &SF) {
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

GenericValue Interpreter::executeFPToSIInst(Value *SrcVal, Type *DstTy,
                                            ExecutionContext &SF) {
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

GenericValue Interpreter::executeUIToFPInst(Value *SrcVal, Type *DstTy,
                                            ExecutionContext &SF) {
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

GenericValue Interpreter::executeSIToFPInst(Value *SrcVal, Type *DstTy,
                                            ExecutionContext &SF) {
  GenericValue Dest, Src = getOperandValue(SrcVal, SF);

  if (isa<VectorType>(SrcVal->getType())) {
    const Type *DstVecTy = DstTy->getScalarType();
    unsigned size = Src.AggregateVal.size();
    // the sizes of src and dst vectors must be equal
    Dest.AggregateVal.resize(size);

    if (DstVecTy->getTypeID() == Type::FloatTyID) {
      assert(DstVecTy->isFloatingPointTy() && "Invalid SIToFP instruction");
      for (unsigned i = 0; i < size; i++)
        Dest.AggregateVal[i].FloatVal =
            APIntOps::RoundSignedAPIntToFloat(Src.AggregateVal[i].IntVal);
    } else {
      for (unsigned i = 0; i < size; i++)
        Dest.AggregateVal[i].DoubleVal =
            APIntOps::RoundSignedAPIntToDouble(Src.AggregateVal[i].IntVal);
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

GenericValue Interpreter::executePtrToIntInst(Value *SrcVal, Type *DstTy,
                                              ExecutionContext &SF) {
  uint32_t DBitWidth = cast<IntegerType>(DstTy)->getBitWidth();
  GenericValue Dest, Src = getOperandValue(SrcVal, SF);
  assert(SrcVal->getType()->isPointerTy() && "Invalid PtrToInt instruction");

  Dest.IntVal = APInt(DBitWidth, (intptr_t) Src.PointerVal);
  return Dest;
}

GenericValue Interpreter::executeIntToPtrInst(Value *SrcVal, Type *DstTy,
                                              ExecutionContext &SF) {
  GenericValue Dest, Src = getOperandValue(SrcVal, SF);
  assert(DstTy->isPointerTy() && "Invalid PtrToInt instruction");

  uint32_t PtrSize = getDataLayout().getPointerSizeInBits();
  if (PtrSize != Src.IntVal.getBitWidth())
    Src.IntVal = Src.IntVal.zextOrTrunc(PtrSize);

  Dest.PointerVal = PointerTy(intptr_t(Src.IntVal.getZExtValue()));
  return Dest;
}

GenericValue Interpreter::executeBitCastInst(Value *SrcVal, Type *DstTy,
                                             ExecutionContext &SF) {

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

void Interpreter::visitTruncInst(TruncInst &I) {
  ExecutionContext &SF = ECStack().back();
  updateDataDeps(getCurThr().id, &I, I.getOperand(0)),
  SetValue(&I, executeTruncInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitSExtInst(SExtInst &I) {
  ExecutionContext &SF = ECStack().back();
  updateDataDeps(getCurThr().id, &I, I.getOperand(0)),
  SetValue(&I, executeSExtInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitZExtInst(ZExtInst &I) {
  ExecutionContext &SF = ECStack().back();
  updateDataDeps(getCurThr().id, &I, I.getOperand(0)),
  SetValue(&I, executeZExtInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPTruncInst(FPTruncInst &I) {
  ExecutionContext &SF = ECStack().back();
  SetValue(&I, executeFPTruncInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPExtInst(FPExtInst &I) {
  ExecutionContext &SF = ECStack().back();
  SetValue(&I, executeFPExtInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitUIToFPInst(UIToFPInst &I) {
  ExecutionContext &SF = ECStack().back();
  SetValue(&I, executeUIToFPInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitSIToFPInst(SIToFPInst &I) {
  ExecutionContext &SF = ECStack().back();
  SetValue(&I, executeSIToFPInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPToUIInst(FPToUIInst &I) {
  ExecutionContext &SF = ECStack().back();
  SetValue(&I, executeFPToUIInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitFPToSIInst(FPToSIInst &I) {
  ExecutionContext &SF = ECStack().back();
  SetValue(&I, executeFPToSIInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitPtrToIntInst(PtrToIntInst &I) {
  ExecutionContext &SF = ECStack().back();
  updateDataDeps(getCurThr().id, &I, I.getOperand(0));
  SetValue(&I, executePtrToIntInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitIntToPtrInst(IntToPtrInst &I) {
  ExecutionContext &SF = ECStack().back();
  updateDataDeps(getCurThr().id, &I, I.getOperand(0));
  SetValue(&I, executeIntToPtrInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitBitCastInst(BitCastInst &I) {
  ExecutionContext &SF = ECStack().back();
  updateDataDeps(getCurThr().id, &I, I.getOperand(0));
  SetValue(&I, executeBitCastInst(I.getOperand(0), I.getType(), SF), SF);
}

#define IMPLEMENT_VAARG(TY) \
   case Type::TY##TyID: Dest.TY##Val = Src.TY##Val; break

void Interpreter::visitVAArgInst(VAArgInst &I) {
  ExecutionContext &SF = ECStack().back();

  // Get the incoming valist parameter.  LLI treats the valist as a
  // (ec-stack-depth var-arg-index) pair.
  GenericValue VAList = getOperandValue(I.getOperand(0), SF);
  GenericValue Dest;
  GenericValue Src = ECStack()[VAList.UIntPairVal.first]
                      .VarArgs[VAList.UIntPairVal.second];
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

void Interpreter::visitExtractElementInst(ExtractElementInst &I) {
  ExecutionContext &SF = ECStack().back();
  GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue Dest;

  Type *Ty = I.getType();
  const unsigned indx = unsigned(Src2.IntVal.getZExtValue());

  if(Src1.AggregateVal.size() > indx) {
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

void Interpreter::visitInsertElementInst(InsertElementInst &I) {
  ExecutionContext &SF = ECStack().back();
  Type *Ty = I.getType();

  if(!(Ty->isVectorTy()) )
    llvm_unreachable("Unhandled dest type for insertelement instruction");

  GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue Src3 = getOperandValue(I.getOperand(2), SF);
  GenericValue Dest;

  Type *TyContained = Ty->getContainedType(0);

  const unsigned indx = unsigned(Src3.IntVal.getZExtValue());
  Dest.AggregateVal = Src1.AggregateVal;

  if(Src1.AggregateVal.size() <= indx)
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

void Interpreter::visitShuffleVectorInst(ShuffleVectorInst &I){
  ExecutionContext &SF = ECStack().back();

  Type *Ty = I.getType();
  if(!(Ty->isVectorTy()))
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
      for( unsigned i=0; i<src3Size; i++) {
        unsigned j = Src3.AggregateVal[i].IntVal.getZExtValue();
        if(j < src1Size)
          Dest.AggregateVal[i].IntVal = Src1.AggregateVal[j].IntVal;
        else if(j < src1Size + src2Size)
          Dest.AggregateVal[i].IntVal = Src2.AggregateVal[j-src1Size].IntVal;
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
      for( unsigned i=0; i<src3Size; i++) {
        unsigned j = Src3.AggregateVal[i].IntVal.getZExtValue();
        if(j < src1Size)
          Dest.AggregateVal[i].FloatVal = Src1.AggregateVal[j].FloatVal;
        else if(j < src1Size + src2Size)
          Dest.AggregateVal[i].FloatVal = Src2.AggregateVal[j-src1Size].FloatVal;
        else
          llvm_unreachable("Invalid mask in shufflevector instruction");
        }
      break;
    case Type::DoubleTyID:
      for( unsigned i=0; i<src3Size; i++) {
        unsigned j = Src3.AggregateVal[i].IntVal.getZExtValue();
        if(j < src1Size)
          Dest.AggregateVal[i].DoubleVal = Src1.AggregateVal[j].DoubleVal;
        else if(j < src1Size + src2Size)
          Dest.AggregateVal[i].DoubleVal =
            Src2.AggregateVal[j-src1Size].DoubleVal;
        else
          llvm_unreachable("Invalid mask in shufflevector instruction");
      }
      break;
  }
  SetValue(&I, Dest, SF);
}

void Interpreter::visitExtractValueInst(ExtractValueInst &I) {
  ExecutionContext &SF =  ECStack().back();
  Value *Agg = I.getAggregateOperand();
  GenericValue Dest;
  GenericValue Src = getOperandValue(Agg, SF);

  ExtractValueInst::idx_iterator IdxBegin = I.idx_begin();
  unsigned Num = I.getNumIndices();
  GenericValue *pSrc = &Src;

  for (unsigned i = 0 ; i < Num; ++i) {
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

void Interpreter::visitInsertValueInst(InsertValueInst &I) {

  ExecutionContext &SF = ECStack().back();
  Value *Agg = I.getAggregateOperand();

  GenericValue Src1 = getOperandValue(Agg, SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue Dest = Src1; // Dest is a slightly changed Src1

  ExtractValueInst::idx_iterator IdxBegin = I.idx_begin();
  unsigned Num = I.getNumIndices();

  GenericValue *pDest = &Dest;
  for (unsigned i = 0 ; i < Num; ++i) {
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

GenericValue Interpreter::getConstantExprValue (ConstantExpr *CE,
                                                ExecutionContext &SF) {
  switch (CE->getOpcode()) {
  case Instruction::Trunc:
      return executeTruncInst(CE->getOperand(0), CE->getType(), SF);
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
  case Instruction::PtrToInt:
      return executePtrToIntInst(CE->getOperand(0), CE->getType(), SF);
  case Instruction::IntToPtr:
      return executeIntToPtrInst(CE->getOperand(0), CE->getType(), SF);
  case Instruction::BitCast:
      return executeBitCastInst(CE->getOperand(0), CE->getType(), SF);
  case Instruction::GetElementPtr:
    return executeGEPOperation(CE->getOperand(0), gep_type_begin(CE),
                               gep_type_end(CE), SF);
  case Instruction::FCmp:
  case Instruction::ICmp:
    return executeCmpInst(CE->getPredicate(),
                          getOperandValue(CE->getOperand(0), SF),
                          getOperandValue(CE->getOperand(1), SF),
                          CE->getOperand(0)->getType());
  case Instruction::Select:
    return executeSelectInst(getOperandValue(CE->getOperand(0), SF),
                             getOperandValue(CE->getOperand(1), SF),
                             getOperandValue(CE->getOperand(2), SF),
                             CE->getOperand(0)->getType());
  default :
    break;
  }

  // The cases below here require a GenericValue parameter for the result
  // so we initialize one, compute it and then return it.
  GenericValue Op0 = getOperandValue(CE->getOperand(0), SF);
  GenericValue Op1 = getOperandValue(CE->getOperand(1), SF);
  GenericValue Dest;
  Type * Ty = CE->getOperand(0)->getType();
  switch (CE->getOpcode()) {
  case Instruction::Add:  Dest.IntVal = Op0.IntVal + Op1.IntVal; break;
  case Instruction::Sub:  Dest.IntVal = Op0.IntVal - Op1.IntVal; break;
  case Instruction::Mul:  Dest.IntVal = Op0.IntVal * Op1.IntVal; break;
  case Instruction::FAdd: executeFAddInst(Dest, Op0, Op1, Ty); break;
  case Instruction::FSub: executeFSubInst(Dest, Op0, Op1, Ty); break;
  case Instruction::FMul: executeFMulInst(Dest, Op0, Op1, Ty); break;
  case Instruction::FDiv: executeFDivInst(Dest, Op0, Op1, Ty); break;
  case Instruction::FRem: executeFRemInst(Dest, Op0, Op1, Ty); break;
  case Instruction::SDiv: Dest.IntVal = Op0.IntVal.sdiv(Op1.IntVal); break;
  case Instruction::UDiv: Dest.IntVal = Op0.IntVal.udiv(Op1.IntVal); break;
  case Instruction::URem: Dest.IntVal = Op0.IntVal.urem(Op1.IntVal); break;
  case Instruction::SRem: Dest.IntVal = Op0.IntVal.srem(Op1.IntVal); break;
  case Instruction::And:  Dest.IntVal = Op0.IntVal & Op1.IntVal; break;
  case Instruction::Or:   Dest.IntVal = Op0.IntVal | Op1.IntVal; break;
  case Instruction::Xor:  Dest.IntVal = Op0.IntVal ^ Op1.IntVal; break;
  case Instruction::Shl:
    Dest.IntVal = Op0.IntVal.shl(Op1.IntVal.getZExtValue());
    break;
  case Instruction::LShr:
    Dest.IntVal = Op0.IntVal.lshr(Op1.IntVal.getZExtValue());
    break;
  case Instruction::AShr:
    Dest.IntVal = Op0.IntVal.ashr(Op1.IntVal.getZExtValue());
    break;
  default:
    dbgs() << "Unhandled ConstantExpr: " << *CE << "\n";
    llvm_unreachable("Unhandled ConstantExpr");
  }
  return Dest;
}

GenericValue Interpreter::getOperandValue(Value *V, ExecutionContext &SF) {
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

void Interpreter::handleSystemError(SystemError code, const std::string &msg)
{
	if (stopOnSystemErrors) {
		systemErrorNumber = code;
		driver->reportError(currPos(), VerificationError::VE_SystemError, msg);
	} else {
		WARN_ONCE(errorList.at(code), msg + "\n");
		CALL_DRIVER(handleStore,
			    WriteLabel::create(currPos(), AtomicOrdering::Monotonic,
					       errnoAddr, getTypeSize(errnoTyp),
					       AType::Signed, static_cast<int>(code)));
	}
}

void Interpreter::handleLock(SAddr addr, ASize size, const EventDeps *deps)
{
	/* No locking when running the recovery routine */
	if (getProgramState() == ProgramState::Recovery)
		return;

	// /* Treatment of locks based on whether LAPOR is enabled */
	// if (getConf()->LAPOR) {
	// 	handleLockLAPOR(LockLabelLAPOR::create(pos, addr), deps);
	// 	return;
	// }

	auto ret = CALL_DRIVER_RESET_IF_NONE(handleLoad,
					     LockCasReadLabel::create(currPos(), addr, size, GET_DEPS(deps)));
	if (!ret.has_value())
		return;

	if (!getCurThr().isBlocked()) {
		if (ret == SVal(0))
			CALL_DRIVER(handleStore,
				    LockCasWriteLabel::create(currPos(), addr, size, GET_DEPS(deps)));
		else
			CALL_DRIVER(handleBlock,
				    BlockLabel::create(currPos(), BlockageType::LockNotAcq));
	}
}

void Interpreter::handleUnlock(SAddr addr, ASize size, const EventDeps *deps)
{
	/* No locking when running the recovery routine */
	if (getProgramState() == ProgramState::Recovery)
		return;

	// /* Treatment of unlocks based on whether LAPOR is enabled */
	// if (getConf()->LAPOR) {
	// 	handleUnlockLAPOR(UnlockLabelLAPOR::create(pos, addr), deps);
	// 	return;
	// }

	CALL_DRIVER(handleStore, UnlockWriteLabel::create(currPos(), addr, size, GET_DEPS(deps)));
	return;
}

void Interpreter::callAssertFail(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	auto errT = (getProgramState() == ProgramState::Recovery) ?
		VerificationError::VE_Recovery : VerificationError::VE_Safety;
	std::string err = (ArgVals.size()) ? std::string("Assertion violation: ") +
		std::string((char *) getStaticAddr(GVTOP(ArgVals[0])))	: "Unknown";

	driver->reportError(currPos(), errT, err);
}

void Interpreter::callOptBegin(Function *F, const std::vector<GenericValue> &ArgVals,
			       const std::unique_ptr<EventDeps> &specialDeps)
{
	auto expand = CALL_DRIVER(handleOptional, OptionalLabel::create(currPos()));

	GenericValue result;
	result.IntVal = APInt(F->getReturnType()->getIntegerBitWidth(), expand, true); // signed
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callLoopBegin(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleLoopBegin, LoopBeginLabel::create(currPos()));
}

void Interpreter::callSpinStart(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleSpinStart, SpinStartLabel::create(currPos()));
}

void Interpreter::callSpinEnd(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	/* XXX: If we ever remove EE blocking, account for blocked events in liveness */
	if (!ArgVals[0].IntVal.getBoolValue())
		CALL_DRIVER(handleBlock, BlockLabel::create(currPos(), BlockageType::Spinloop));
}

void Interpreter::callFaiZNESpinEnd(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleFaiZNESpinEnd, FaiZNESpinEndLabel::create(currPos()));
}

void Interpreter::callLockZNESpinEnd(Function *F, const std::vector<GenericValue> &ArgVals,
				     const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleLockZNESpinEnd, LockZNESpinEndLabel::create(currPos()));
}

void Interpreter::callKillThread(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	if (ArgVals[0].IntVal.getBoolValue())
		ECStack().clear();
}

void Interpreter::callAssume(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	if (!ArgVals[0].IntVal.getBoolValue())
		CALL_DRIVER(handleBlock, BlockLabel::create(currPos(), BlockageType::User));
}

void Interpreter::callNondetInt(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	Thread::MyDist dist(std::numeric_limits<int>::min(),
			    std::numeric_limits<int>::max());

	GenericValue result;
	result.IntVal = APInt(F->getReturnType()->getIntegerBitWidth(),
			      dist(getCurThr().rng), true); // signed
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callMalloc(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	if (!ArgVals[0].IntVal.isStrictlyPositive()) {
		driver->reportError(currPos(),
			   VerificationError::VE_Allocation, "Invalid size in malloc()");
		return;
	}

	auto size = ArgVals[0].IntVal.getLimitedValue();

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc,
				  MallocLabel::create(currPos(), size,
						      alignof(std::max_align_t),
						      StorageDuration::SD_Heap,
						      StorageType::ST_Volatile, AddressSpace::AS_User,
						      GET_DEPS(deps)));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
	return;
}

void Interpreter::callMallocAligned(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	auto align = ArgVals[0].IntVal.getLimitedValue();
	auto size = ArgVals[1].IntVal.getLimitedValue();

	if (!ArgVals[0].IntVal.isStrictlyPositive() || (align & (align - 1))) {
		driver->reportError(currPos(), VerificationError::VE_Allocation,
				    "Invalid alignment in aligned_alloc()");
		return;
	}
	if (!ArgVals[1].IntVal.isStrictlyPositive() || (size % align)) {
		driver->reportError(currPos(), VerificationError::VE_Allocation,
				    "Invalid size in aligned_alloc()");
		return;
	}

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc,
				  MallocLabel::create(currPos(), size, align, StorageDuration::SD_Heap,
						      StorageType::ST_Volatile, AddressSpace::AS_User,
						      GET_DEPS(deps)));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
	return;
}

void Interpreter::callPMalloc(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	if (!ArgVals[0].IntVal.isStrictlyPositive()) {
		driver->reportError(currPos(), VerificationError::VE_Allocation,
			   "Invalid size in malloc()");
		return;
	}

	auto size = ArgVals[0].IntVal.getLimitedValue();

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc,
				  MallocLabel::create(currPos(), size, alignof(std::max_align_t),
						      StorageDuration::SD_Heap,
						      StorageType::ST_Durable, AddressSpace::AS_User,
						      GET_DEPS(deps)));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
	return;
}

void Interpreter::callFree(Function *F, const std::vector<GenericValue> &ArgVals,
			   const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);

	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* When attempting to free a NULL pointer, don't increase counters */
	if (ptr)
		CALL_DRIVER(handleFree, FreeLabel::create(currPos(), ptr, GET_DEPS(deps)));
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
	Function *calledFun = (Function*) GVTOP(ArgVals[1]);
	ExecutionContext SF;
	GenericValue val, result;

	if (!calledFun) {
		driver->reportError(currPos(), VerificationError::VE_InvalidCreate,
				    "Invalid argument in pthread_create(): NULL pointer");
		return;
	}

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
			       (uintptr_t) ArgVals[2].PointerVal, symm);
	auto tid = CALL_DRIVER(handleThreadCreate,
			       ThreadCreateLabel::create(currPos(), info, GET_DEPS(deps)));

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
	auto result = CALL_DRIVER_RESET_IF_NONE(
		handleThreadJoin,
		ThreadJoinLabel::create(currPos(), ArgVals[0].IntVal.getLimitedValue(), GET_DEPS(deps)));
	if (!result.has_value())
		return;
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(result.value(), F->getReturnType()));
}

void Interpreter::callThreadExit(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	while (ECStack().size() > 1) {
		freeAllocas(ECStack().back().Allocas);
		ECStack().pop_back();
	}
	popStackAndReturnValueToCaller(Type::getInt8PtrTy(F->getContext()), ArgVals[0]);
}

void Interpreter::callAtExit(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	addAtExitHandler((Function*)GVTOP(ArgVals[0]));
	returnValueToCaller(F->getReturnType(), INT_TO_GV(F->getReturnType(), 0));
}

void Interpreter::callMutexInit(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *lock = (GenericValue *) GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *) GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	if (attr)
		WARN_ONCE("pthread-mutex-init-arg",
			  "Ignoring non-null argument given to pthread_mutex_init.\n");

	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), AtomicOrdering::NotAtomic,
				       lock, size, atyp, SVal(0), GET_DEPS(specialDeps)));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
}

void Interpreter::callMutexLock(Function *F, const std::vector<GenericValue> &ArgVals,
				const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
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
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
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
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
	Type *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);
	GenericValue result;

	/* Dependencies already set by the EE */
	auto ret = CALL_DRIVER(handleLoad,
			       TrylockCasReadLabel::create(currPos(), ptr, size, GET_DEPS(specialDeps))).value();

	auto cmpRes = ret == SVal(0);
	if (cmpRes)
		CALL_DRIVER(handleStore,
			    TrylockCasWriteLabel::create(currPos(), ptr, size, GET_DEPS(specialDeps)));

	result.IntVal = APInt(typ->getIntegerBitWidth(), !cmpRes);
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callMutexDestroy(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue *lock = (GenericValue *) GVTOP(ArgVals[0]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), AtomicOrdering::NotAtomic,
				       lock, size, atyp, SVal(-1), GET_DEPS(specialDeps)));

	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
	return;
}

void Interpreter::callBarrierInit(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	auto *barrier = (GenericValue *) GVTOP(ArgVals[0]);
	auto *attr = (GenericValue *) GVTOP(ArgVals[1]);
	auto *typ = F->getReturnType();
	auto value = GV_TO_SVAL(ArgVals[2], typ);
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	if (attr)
		WARN_ONCE("pthread-barrier-init-arg",
			  "Ignoring non-null argument given to pthread_barrier_init.\n");
	CALL_DRIVER(handleStore,
		    BInitWriteLabel::create(currPos(), AtomicOrdering::NotAtomic,
					    barrier, size, atyp, value, GET_DEPS(specialDeps)));

	/* Just return 0 */
	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
	return;
}

void Interpreter::callBarrierWait(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	auto *barrier = (GenericValue *) GVTOP(ArgVals[0]);
	auto *typ = F->getReturnType();
	auto asize = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	auto oldVal = CALL_DRIVER_RESET_IF_NONE(handleLoad,
				  BIncFaiReadLabel::create(currPos(), AtomicOrdering::AcquireRelease,
							   barrier, asize, atyp, AtomicRMWInst::BinOp::Sub,
							   SVal(1), GET_DEPS(specialDeps)));

	/* If the barrier was uninitialized and we blocked, abort */
	if (!oldVal.has_value() || oldVal->getSigned() <= 0 || getCurThr().isBlocked())
		return;

	auto newVal = executeAtomicRMWOperation(*oldVal, SVal(1), asize, AtomicRMWInst::BinOp::Sub);

	CALL_DRIVER(handleStore,
		    BIncFaiWriteLabel::create(currPos(), AtomicOrdering::AcquireRelease,
					      barrier, asize, atyp, newVal, GET_DEPS(specialDeps)));

	CALL_DRIVER(handleLoad,
		    BWaitReadLabel::create(currPos(), AtomicOrdering::Acquire,
					   barrier, asize, atyp, GET_DEPS(specialDeps)));

	auto result = (newVal != SVal(0)) ? INT_TO_GV(typ, 0)
		: INT_TO_GV(typ, GENMC_PTHREAD_BARRIER_SERIAL_THREAD);
	returnValueToCaller(typ, result);
	return;
}

void Interpreter::callBarrierDestroy(Function *F, const std::vector<GenericValue> &ArgVals,
				     const std::unique_ptr<EventDeps> &specialDeps)
{
	auto *barrier = (GenericValue *) GVTOP(ArgVals[0]);
	auto *typ = F->getReturnType();
	auto size = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);

	CALL_DRIVER(handleStore,
		    BDestroyWriteLabel::create(currPos(), AtomicOrdering::NotAtomic,
					       barrier, size, atyp, SVal(0), GET_DEPS(specialDeps)));

	/* Just return 0 */
	GenericValue result;
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	returnValueToCaller(typ, result);
	return;
}

void Interpreter::callHazptrAlloc(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	auto address = CALL_DRIVER(handleMalloc,
				  MallocLabel::create(currPos(), getTypeSize(F->getReturnType()),
						      alignof(std::max_align_t), StorageDuration::SD_Heap,
						      StorageType::ST_Volatile, AddressSpace::AS_Internal,
						      GET_DEPS(deps)));
	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(address, F->getReturnType()));
	return;
}

void Interpreter::callHazptrProtect(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	auto *hp = GVTOP(ArgVals[0]);
	auto *ptr = GVTOP(ArgVals[1]);

	CALL_DRIVER(handleHpProtect, HpProtectLabel::create(currPos(), hp, ptr));
	return;
}

void Interpreter::callHazptrClear(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &specialDeps)
{
	auto *typ = Type::getVoidTy(F->getParent()->getContext())->getPointerTo();
	auto asize = getTypeSize(typ);
	auto atyp = TYPE_TO_ATYPE(typ);
	auto *hp = GVTOP(ArgVals[0]);

	/* FIXME: Should this be an internal null? */
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), AtomicOrdering::Release, hp, asize, atyp, SVal()));
	return;
}

void Interpreter::callHazptrFree(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	CALL_DRIVER(handleFree,
		    FreeLabel::create(currPos(), GVTOP(ArgVals[0]), GET_DEPS(deps)));
}

void Interpreter::callHazptrRetire(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	CALL_DRIVER(handleFree,
		    HpRetireLabel::create(currPos(), GVTOP(ArgVals[0]), GET_DEPS(deps)));
}

static const std::unordered_map<std::string, SmpFenceType> smpFenceTypes = {
	{"mb", SmpFenceType::MB},
	{"rmb", SmpFenceType::RMB},
	{"wmb", SmpFenceType::WMB},
	{"ba", SmpFenceType::MBBA},
	{"aa", SmpFenceType::MBAA},
	{"as", SmpFenceType::MBAS},
	{"aul", SmpFenceType::MBAUL},
};

void Interpreter::callSmpFenceLKMM(Function *F, const std::vector<GenericValue> &ArgVals,
				   const std::unique_ptr<EventDeps> &specialDeps)
{
	auto ft = smpFenceTypes.at((const char *) getStaticAddr(GVTOP(ArgVals[0])));
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id), nullptr, nullptr);
	CALL_DRIVER(handleFence,
		    SmpFenceLabelLKMM::create(currPos(), llvm::AtomicOrdering::Monotonic, ft, GET_DEPS(deps)));
	return;
}

void Interpreter::callRCUReadLockLKMM(Function *F, const std::vector<GenericValue> &ArgVals,
				      const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleRCULockLKMM,
		   RCULockLabelLKMM::create(currPos()));
	return;
}

void Interpreter::callRCUReadUnlockLKMM(Function *F, const std::vector<GenericValue> &ArgVals,
					const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleRCUUnlockLKMM, RCUUnlockLabelLKMM::create(currPos()));
	return;
}

void Interpreter::callSynchronizeRCULKMM(Function *F, const std::vector<GenericValue> &ArgVals,
					 const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleRCUSyncLKMM, RCUSyncLabelLKMM::create(currPos()));
	return;
}

void Interpreter::callCLFlush(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);
	CALL_DRIVER(handleCLFlush, CLFlushLabel::create(currPos(), GVTOP(ArgVals[0]), GET_DEPS(deps)));
	return;
}

SVal Interpreter::getInodeTransStatus(void *inode, Type *intTyp)
{
	auto inodeItrans = GET_INODE_ITRANSACTION_ADDR(inode);
	return CALL_DRIVER(handleDskRead,
			  DskReadLabel::create(currPos(), inodeItrans, getTypeSize(intTyp),
					       TYPE_TO_ATYPE(intTyp)));
}

void Interpreter::setInodeTransStatus(void *inode, Type *intTyp, SVal val)
{
	/* Transaction status modifications do not have any mapping (journal only) */
	auto inodeItrans = GET_INODE_ITRANSACTION_ADDR(inode);
	CALL_DRIVER(handleDskWrite,
		   DskJnlWriteLabel::create(currPos(), inodeItrans, getTypeSize(intTyp),
					    TYPE_TO_ATYPE(intTyp), val, GET_JOURNAL_MAPPING(inode), inode));
	return;
}

SVal Interpreter::readInodeSizeFS(void *inode, Type *intTyp, const std::unique_ptr<EventDeps> &deps)
{
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);

	if (getProgramState() == ProgramState::Recovery) {
		auto inodeIdisksize = GET_INODE_IDISKSIZE_ADDR(inode);
		return CALL_DRIVER(handleDskRead,
				  DskReadLabel::create(currPos(), inodeIdisksize, asize, atyp));
	}

	auto inodeIsize = GET_INODE_ISIZE_ADDR(inode);
	return CALL_DRIVER(handleLoad,
			   ReadLabel::create(currPos(), AtomicOrdering::Acquire, inodeIsize,
					     asize, atyp, GET_DEPS(deps))).value();
}

void Interpreter::updateInodeSizeFS(void *inode, Type *intTyp, SVal newSize,
				    const std::unique_ptr<EventDeps> &deps)
{
	auto inodeIsize = GET_INODE_ISIZE_ADDR(inode);
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), AtomicOrdering::Release, inodeIsize,
				       getTypeSize(intTyp), TYPE_TO_ATYPE(intTyp), newSize, GET_DEPS(deps)));
	return;
}

void Interpreter::updateInodeDisksizeFS(void *inode, Type *intTyp, SVal newSize,
					SVal ordDataBegin, SVal ordDataEnd)
{
	auto inodeIdisksize = GET_INODE_IDISKSIZE_ADDR(inode);
	auto ordDataRange = std::make_pair((void *) nullptr, (void *) nullptr);

	/* In data=ordered mode, metadata are journaled after data are written */
	if (MI->fsInfo.journalData == JournalDataFS::ordered) {
		auto *inodeData = (char *) GET_INODE_DATA_ADDR(inode);
		ordDataRange.first = inodeData + ordDataBegin.get();
		ordDataRange.second = inodeData + ordDataEnd.get();
	}

	/* Update disksize*/
	CALL_DRIVER(handleDskWrite,
		   DskMdWriteLabel::create(currPos(), inodeIdisksize, getTypeSize(intTyp),
					   TYPE_TO_ATYPE(intTyp), newSize,
					   GET_METADATA_MAPPING(inode), ordDataRange));

	/* If there is no delayed allocation, we actually _wait_ for the write */
	if (MI->fsInfo.journalData == JournalDataFS::ordered && !MI->fsInfo.delalloc)
		executeFsyncFS(inode, intTyp);
	return;
}

void Interpreter::writeDataToDisk(void *buf, int bufOffset, void *inode, int inodeOffset, int count,
				  Type *dataTyp, const std::unique_ptr<EventDeps> &deps)
{
	auto *inodeData = GET_INODE_DATA_ADDR(inode);
	auto size = getTypeSize(dataTyp);
	auto atyp = TYPE_TO_ATYPE(dataTyp);

	for (auto i = 0u; i < count; i++) {
		auto loadAddr = (char *) buf + bufOffset + i;
		auto val = CALL_DRIVER(handleLoad,
				       ReadLabel::create(currPos(), AtomicOrdering::NotAtomic,
							 loadAddr, size, atyp, GET_DEPS(deps))).value();

		auto writeAddr = (char *) inodeData + inodeOffset + i;
		CALL_DRIVER(handleDskWrite,
			   DskWriteLabel::create(currPos(), writeAddr, size, atyp, val,
						 GET_DATA_MAPPING(inode)));
	}
	return;
}

void Interpreter::readDataFromDisk(void *inode, int inodeOffset, void *buf, int bufOffset, int count,
				   Type *dataTyp, const std::unique_ptr<EventDeps> &deps)
{
	auto *inodeData = GET_INODE_DATA_ADDR(inode);
	auto asize = getTypeSize(dataTyp);
	auto atyp = TYPE_TO_ATYPE(dataTyp);

	for (auto i = 0u; i < count; i++) {
		auto readAddr = (char *) inodeData + inodeOffset + i;
		auto val = CALL_DRIVER(handleDskRead,
				      DskReadLabel::create(currPos(), readAddr, asize, atyp));

		auto storeAddr = (char *) buf + bufOffset + i;
		CALL_DRIVER(handleStore,
			    WriteLabel::create(currPos(), AtomicOrdering::NotAtomic,
					       storeAddr, asize, atyp, val, GET_DEPS(deps)));
	}
	return;
}

void Interpreter::updateDirNameInode(const std::string &name, Type *intTyp, SVal inode)
{
	auto *dirInode = getDirInode();
	auto inodeAddr = getInodeAddrFromName(name);

	executeFsyncFS(dirInode, intTyp); //   \/ relies on inode layout
	CALL_DRIVER(handleDskWrite,
		   DskDirWriteLabel::create(currPos(), inodeAddr, getTypeSize(intTyp->getPointerTo()),
					    AType::Pointer, inode, GET_DATA_MAPPING(dirInode)));
	if (MI->fsInfo.journalData == JournalDataFS::journal)
		executeFsyncFS(dirInode, intTyp);
	return;
}

/* We may have to manipulate the flags at some point (depending on the model) */
SVal Interpreter::checkOpenFlagsFS(SVal &flags, Type *intTyp)
{
	if (flags.get() & ~GENMC_VALID_OPEN_FLAGS) {
		handleSystemError(SystemError::SE_EINVAL, "Invalid flags used for open()");
		return SVal(-1);
	}

	if (flags.get() & GENMC_O_DSYNC)
		WARN_ONCE("open-dsync-osync", "O_DSYNC encountered. Assuming O_SYNC...\n");

	return SVal(0);
}

SVal Interpreter::executeInodeLookupFS(const std::string &filename, Type *intTyp)
{
	auto inTrans = getInodeTransStatus(getDirInode(), intTyp);
	if (inTrans == SVal(1)) {
		getCurThr().rollToSnapshot();
		getCurThr().block(BlockageType::Cons);
		return SVal(42); /* propagate block */
	}

	/* Fetch the address where the inode should be and read the contents */
	auto inodeAddr = getInodeAddrFromName(filename);
	return CALL_DRIVER(handleDskRead,
			  DskReadLabel::create(currPos(), inodeAddr, getTypeSize(intTyp->getPointerTo()),
					       AType::Pointer));
}

SVal Interpreter::executeInodeCreateFS(const std::string &filename, Type *intTyp,
				       const std::unique_ptr<EventDeps> &deps)
{
	/* Allocate enough space for the inode... */
	unsigned int inodeSize = getTypeSize(MI->fsInfo.inodeTyp);
	auto *info = getVarNameInfo(nullptr, StorageDuration::SD_Heap, AddressSpace::AS_Internal, "inode");
	auto *inode = (void *) CALL_DRIVER(handleMalloc,
					  MallocLabel::create(currPos(), inodeSize, alignof(std::max_align_t),
							      StorageDuration::SD_Heap, StorageType::ST_Durable,
							      AddressSpace::AS_Internal, info,
							      std::string("__inode_") + filename, GET_DEPS(deps))).get();

	/* ... properly initialize its fields... */
	auto inodeLock = GET_INODE_LOCK_ADDR(inode);
	auto inodeIsize = GET_INODE_ISIZE_ADDR(inode);

	auto zero = SVal(0);
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), AtomicOrdering::Release, inodeLock,
				       asize, atyp, zero, GET_DEPS(deps)));
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), AtomicOrdering::Release, inodeIsize,
				       asize, atyp, zero, GET_DEPS(deps)));
	setInodeTransStatus(inode, intTyp, zero);
	updateInodeDisksizeFS(inode, intTyp, zero, zero, zero);

	/* ... set the newly allocated inode to the appropriate address */
	updateDirNameInode(filename, intTyp, SVal((uintptr_t) inode));

	return SVal((uintptr_t) inode);
}

/* On success, returns a non-null pointer value that corresponds
 * to the address of FILE's inode. Success: either if the inode was
 * already created, or flags contain O_CREAT and the inode was
 * created. */
SVal Interpreter::executeLookupOpenFS(const std::string &file, SVal &flags, Type *intTyp,
				      const std::unique_ptr<EventDeps> &deps)
{
	/* If O_CREAT was not specified, just do the lookup */
	if (!(flags.get() & GENMC_O_CREAT))
		return executeInodeLookupFS(file, intTyp);

	/* Otherwise, we need to take the dir inode's lock */
	auto dirLock = GET_INODE_LOCK_ADDR(getDirInode());
	handleLock(dirLock, getTypeSize(intTyp), &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return SVal(42);
	}

	/* Check if the corresponding inode already exists */
	auto inode = executeInodeLookupFS(file, intTyp);
	if (getCurThr().isBlocked())
		return inode; /* propagate the block */

	/* Return the inode, if it already exists */
	if ((void *) inode.get() != nullptr)
		goto unlock;

	/* Otherwise, we create an inode */
	inode = executeInodeCreateFS(file, intTyp, deps);

	/* If we created the inode, we will not truncate it
	 * (This should not happen here, but since we only model ext4 it doesn't matter) */
	flags &= SVal(~GENMC_O_TRUNC); /* Compatible with LLVM <= 4 */

unlock:
	handleUnlock(dirLock, getTypeSize(intTyp), &*deps);
	return inode;
}

SVal Interpreter::executeOpenFS(const std::string &filename, SVal flags, SVal inode, Type *intTyp,
				const std::unique_ptr<EventDeps> &deps)
{
	Type *intPtrType = intTyp->getPointerTo();

	/* Get a fresh fd */
	auto fd = CALL_DRIVER(handleDskOpen, DskOpenLabel::create(currPos(), filename, getTypeSize(intTyp)));

	/* Also a name for the description */
	std::string varname("__file_");
	raw_string_ostream sname(varname);
	sname << filename << "_" << getCurThr().id << "_" << getCurThr().globalInstructions;

	/* We allocate space for the file description... */
	auto fileSize = getTypeSize(MI->fsInfo.fileTyp);
	auto *info = getVarNameInfo(nullptr, StorageDuration::SD_Heap, AddressSpace::AS_Internal, "file");
	auto *file = (void *) CALL_DRIVER(handleMalloc,
					 MallocLabel::create(currPos(), fileSize, alignof(std::max_align_t),
							     StorageDuration::SD_Heap, StorageType::ST_Volatile,
							     AddressSpace::AS_Internal, info, sname.str(),
							     GET_DEPS(deps))).get();

	/* ... and initialize its fields */
	auto fileInode = GET_FILE_INODE_ADDR(file);
	auto fileCount = GET_FILE_COUNT_ADDR(file);
	auto fileFlags = GET_FILE_FLAGS_ADDR(file);
	auto filePosLock = GET_FILE_POS_LOCK_ADDR(file);
	auto fileOffset = GET_FILE_OFFSET_ADDR(file);
	auto atyp = TYPE_TO_ATYPE(intTyp);

	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
				       fileInode, getTypeSize(intPtrType), AType::Pointer, inode,
				       GET_DEPS(deps)));
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
				       fileFlags, getTypeSize(intTyp), atyp, flags,
				       GET_DEPS(deps)));
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
				       fileOffset, getTypeSize(intTyp), atyp, SVal(0),
				       GET_DEPS(deps)));
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), llvm::AtomicOrdering::Release,
				       filePosLock, getTypeSize(intTyp), atyp, SVal(0),
				       GET_DEPS(deps)));
	CALL_DRIVER(handleStore,
		    WriteLabel::create(currPos(), llvm::AtomicOrdering::Release,
				       fileCount, getTypeSize(intTyp), atyp, SVal(1),
				       GET_DEPS(deps)));
	setFdToFile(fd.get(), file);
	return fd;
}

SVal Interpreter::executeTruncateFS(SVal inode, SVal length, Type *intTyp,
				    const std::unique_ptr<EventDeps> &deps)
{
	/* length is a signed integer -- careful because it's long */
	if (length.getSigned() < 0) {
		handleSystemError(SystemError::SE_EINVAL, "Invalid length for truncate()");
		return SVal(-1);
	}

	/* Get inode's lock (as in do_truncate()) */
	auto inodeLock = GET_INODE_LOCK_ADDR((void *) inode.get());
	handleLock(inodeLock, getTypeSize(intTyp), &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return SVal(42);
	}

	SVal ret = SVal(0);
	SVal ordRangeBegin = SVal(0), ordRangeEnd = SVal(0);

	/* Check if we are actually extending the file */
	auto oldIsize = readInodeSizeFS((void *) inode.get(), intTyp, deps);
	if (length.getSigned() > oldIsize.getSigned()) {
		if (length.getSigned() >= MI->fsInfo.maxFileSize) {
			handleSystemError(SystemError::SE_EFBIG, "Length too big in truncate()");
			ret = SVal(-1);
			goto out;
		}
		zeroDskRangeFS((void *) inode.get(), oldIsize, length, Type::getInt8Ty(intTyp->getContext()));
		ordRangeBegin = oldIsize;
		ordRangeEnd = length;
	}

	/* Update inode's size (ext4_setattr()) */
	updateInodeDisksizeFS((void *) inode.get(), intTyp, length, ordRangeBegin, ordRangeEnd);
	updateInodeSizeFS((void *) inode.get(), intTyp, length, deps);

	if (MI->fsInfo.journalData >= JournalDataFS::ordered)
		executeFsyncFS((void *) inode.get(), intTyp);

out:
	/* Release inode's lock */
	handleUnlock(inodeLock, getTypeSize(intTyp), &*deps);
	return ret;
}

void Interpreter::callOpenFS(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	std::string filename = (const char *) getStaticAddr(GVTOP(ArgVals[0]));
	SVal flags = ArgVals[1].IntVal.getLimitedValue();
	Type *intTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* Check the flags passed -- we ignore mode_t for the time being */
	auto retO = checkOpenFlagsFS(flags, intTyp);
	if (retO == SVal(-1)) {
		returnValueToCaller(F->getReturnType(), SVAL_TO_GV(retO, F->getReturnType()));
		return;
	}

	/* Try and find the requested inode */
	auto inode = executeLookupOpenFS(filename, flags, intTyp, deps);
	if (getCurThr().isBlocked())
		return;

	/* Inode not found -- cannot open file */
	if ((void *) inode.get() == nullptr) {
		handleSystemError(SystemError::SE_ENOENT, "File does not exist in open()");
		returnValueToCaller(F->getReturnType(), INT_TO_GV(intTyp, -1));
		return;
	}

	/* We allocate a new file description pointing to the file's inode... */
	auto fd = executeOpenFS(filename, flags, inode, intTyp, deps);

	/* ... and truncate if necessary */
	if (flags.get() & GENMC_O_TRUNC) {
		if (!(GENMC_OPEN_FMODE(flags.get()) & GENMC_FMODE_WRITE)) {
			driver->reportError(currPos(), VerificationError::VE_InvalidTruncate,
					   "File is not open for writing");
			returnValueToCaller(F->getReturnType(), INT_TO_GV(F->getReturnType(), -1));
			return;
		}
		auto ret = executeTruncateFS(inode, SVal(0), intTyp, deps);
		if (ret == SVal(42))
			return; /* Failed to acquire inode's lock... */
		if (ret == SVal(-1)) {
			returnValueToCaller(F->getReturnType(), INT_TO_GV(F->getReturnType(), -1));
			return; /* Something went really wrong (negative length?) */
		}
	}

	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(fd, F->getReturnType()));
	return;
}

void Interpreter::callCreatFS(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	Type *intTyp = F->getReturnType();
	auto flags = INT_TO_GV(intTyp, GENMC_O_CREAT|GENMC_O_WRONLY|GENMC_O_TRUNC);
	callOpenFS(F, {ArgVals[0], flags, ArgVals[1]}, specialDeps);
	return;
}

void Interpreter::executeReleaseFileFS(void *fileDesc, Type *intTyp,
				       const std::unique_ptr<EventDeps> &deps)
{
	/* Nothing for auto_da_alloc_close */

	/* Free file description */
	CALL_DRIVER(handleFree, FreeLabel::create(currPos(), fileDesc, GET_DEPS(deps)));
	return;
}

SVal Interpreter::executeCloseFS(SVal fd, Type *intTyp, const std::unique_ptr<EventDeps> &deps)
{
	/* If it's not a valid open fd, report the error */
	auto *fileDesc = getFileFromFd(fd.get());
	if (!fileDesc) {
		handleSystemError(SystemError::SE_EBADF, "Invalid fd used in close()");
		return SVal(-1);
	}

	/* Decrease the count for this file */
	auto fileCount = GET_FILE_COUNT_ADDR(fileDesc);
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);
	auto ret = CALL_DRIVER(handleLoad,
			       FaiReadLabel::create(currPos(), AtomicOrdering::AcquireRelease,
						    fileCount, asize, atyp, AtomicRMWInst::Sub, SVal(1),
						    GET_DEPS(deps))).value();

	auto newVal = executeAtomicRMWOperation(ret, SVal(1), asize, AtomicRMWInst::Sub);

	CALL_DRIVER(handleStore,
		    FaiWriteLabel::create(currPos(), AtomicOrdering::AcquireRelease,
					  fileCount, asize, atyp, newVal, GET_DEPS(deps)));

	/* Check if it is the last reference to a file description */
	if (newVal == SVal(0))
		executeReleaseFileFS(fileDesc, intTyp, deps);

	return SVal(0);
}

void Interpreter::callCloseFS(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	SVal fd = ArgVals[0].IntVal.getLimitedValue();
	Type *intTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* Close the file and return result to user */
	auto result = executeCloseFS(fd, intTyp, deps);
	returnValueToCaller(intTyp, SVAL_TO_GV(result, intTyp));
	return;
}

SVal Interpreter::executeLinkFS(const std::string &newpath, SVal oldInode, Type *intTyp)
{
	updateDirNameInode(newpath, intTyp, oldInode);
	return SVal(0);
}

void Interpreter::callLinkFS(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	std::string oldpath = (const char *) getStaticAddr(GVTOP(ArgVals[0]));
	std::string newpath = (const char *) getStaticAddr(GVTOP(ArgVals[1]));
	Type *intTyp = F->getReturnType();
	GenericValue result;

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	auto dirLock = GET_INODE_LOCK_ADDR(getDirInode());
	SVal source, target;

	/* Since we have a single-directory structure, link boils down to a simple cs */
	handleLock(dirLock, getTypeSize(intTyp), &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return;
	}

	source = executeInodeLookupFS(oldpath, intTyp);
	if (getCurThr().isBlocked())
		return;

	/* If no such entry found, exit */
	if ((void *) source.get() == nullptr) {
		handleSystemError(SystemError::SE_ENOENT, "No entry found for oldpath at link()");
		result = INT_TO_GV(intTyp, -1);
		goto exit;
	}

	/* Otherwise, check if newpath exists */
	target = executeInodeLookupFS(newpath, intTyp);
	if (target.get()) {
		handleSystemError(SystemError::SE_EEXIST, "The entry for newpath exists at link()");
		result = INT_TO_GV(intTyp, -1);
		goto exit;
	}

	result = SVAL_TO_GV(executeLinkFS(newpath, source, intTyp), F->getReturnType());

exit:
	handleUnlock(dirLock, getTypeSize(intTyp), &*deps);
	returnValueToCaller(F->getReturnType(), result);
	return;
}

SVal Interpreter::executeUnlinkFS(const std::string &pathname, Type *intTyp)
{
	/* Unlink inode */
	updateDirNameInode(pathname, intTyp, SVal(0));
	return SVal(0);
}

void Interpreter::callUnlinkFS(Function *F, const std::vector<GenericValue> &ArgVals,
			       const std::unique_ptr<EventDeps> &specialDeps)
{
	std::string pathname = (const char *) getStaticAddr(GVTOP(ArgVals[0]));
	Type *intTyp = F->getReturnType();
	GenericValue result;

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	auto dirLock = GET_INODE_LOCK_ADDR(getDirInode());
	handleLock(dirLock, getTypeSize(intTyp), &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return;
	}

	auto inode = executeInodeLookupFS(pathname, intTyp);
	if (getCurThr().isBlocked())
		return;

	/* Check if component exists */
	if ((void *) inode.get() == nullptr) {
		handleSystemError(SystemError::SE_ENOENT, "Component does not exist for unlink()");
		result = INT_TO_GV(intTyp, -1);
		goto exit;
	}

	result = SVAL_TO_GV(executeUnlinkFS(pathname, intTyp), F->getReturnType());

exit:
	handleUnlock(dirLock, getTypeSize(intTyp), &*deps);

	returnValueToCaller(F->getReturnType(), result);
	return;
}

SVal Interpreter::executeRenameFS(const std::string &oldpath, SVal oldInode,
				  const std::string &newpath, SVal newInode,
				  Type *intTyp)
{
	Type *intPtrTyp = intTyp->getPointerTo();

	/* If hard links referring on the same file */
	if (oldInode == newInode)
		return SVal(0);

	/*
	 * Do the actual rename operation within a transaction
	 * We don't have to do actual locking, since i_transaction
	 * is protected by the inode's lock
	 */
	setInodeTransStatus(getDirInode(), intTyp, SVal(1));

	/* Make new name point to old name's inode and delete old name */
	auto result = executeLinkFS(newpath, oldInode, intTyp);
	BUG_ON(result.getSigned() == -1);
	result = executeUnlinkFS(oldpath, intTyp);

	setInodeTransStatus(getDirInode(), intTyp, SVal(0));

	return result;
}

void Interpreter::callRenameFS(Function *F, const std::vector<GenericValue> &ArgVals,
			       const std::unique_ptr<EventDeps> &specialDeps)
{
	std::string oldpath = (const char *) getStaticAddr(GVTOP(ArgVals[0]));
	std::string newpath = (const char *) getStaticAddr(GVTOP(ArgVals[1]));
	Type *intTyp = F->getReturnType();
	GenericValue result;

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	auto dirLock = GET_INODE_LOCK_ADDR(getDirInode());
	handleLock(dirLock, getTypeSize(intTyp), &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return;
	}

	/* Try to find source inode */
	SVal source, target;
	source = executeInodeLookupFS(oldpath, intTyp);
	if (getCurThr().isBlocked())
		return;
	if ((void *) source.get() == nullptr) {
		handleSystemError(SystemError::SE_ENOENT, "Oldpath does not exist for rename()");
		result = INT_TO_GV(intTyp, -1);
		goto exit; /* Use gotos since we might add support for more flags/error checks */
	}

	/* Try to find target inode */
	target = executeInodeLookupFS(newpath, intTyp);
	if (getCurThr().isBlocked())
		return;

	result = SVAL_TO_GV(executeRenameFS(oldpath, source, newpath, target, intTyp), F->getReturnType());

exit:
	handleUnlock(dirLock, getTypeSize(intTyp), &*deps);

	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callTruncateFS(Function *F, const std::vector<GenericValue> &ArgVals,
				 const std::unique_ptr<EventDeps> &specialDeps)
{
	std::string filename = (const char *) getStaticAddr(GVTOP(ArgVals[0]));
	auto length  = ArgVals[1].IntVal.getLimitedValue();
	Type *intTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	auto dirLock = GET_INODE_LOCK_ADDR(getDirInode());
	handleLock(dirLock, getTypeSize(intTyp), &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return;
	}

	/* Try and find the requested inode */
	auto inode = executeInodeLookupFS(filename, intTyp);
	if (getCurThr().isBlocked())
		return;

	handleUnlock(dirLock, getTypeSize(intTyp), &*deps);

	/* Inode not found -- cannot truncate file */
	if ((void *) inode.get() == nullptr) {
		handleSystemError(SystemError::SE_ENOENT, "File does not exist in truncate()");
		returnValueToCaller(F->getReturnType(), INT_TO_GV(intTyp, -1));
		return;
	}

	auto ret = executeTruncateFS(inode, length, intTyp, deps);
	if (ret == SVal(42))
		return; /* Failed to acquire inode's lock... */

	returnValueToCaller(F->getReturnType(), SVAL_TO_GV(ret, F->getReturnType()));
	return;
}

bool Interpreter::shouldUpdateInodeDisksizeFS(void *inode, Type *intTyp, SVal iSize,
					      SVal wOffset, SVal countVal, SVal &dSize)
{
	/* ugly hack for comparing different length APInts... */
	auto size = iSize.get();
	auto offset = wOffset.get();
	auto count = countVal.get();

	/* We should update if we are appending in the same (pre-allocated) block  */
	auto bi = size % MI->fsInfo.blockSize;
	auto rb = MI->fsInfo.blockSize - bi;
	auto be = size + rb;
	bool shouldUpdate = ((offset <= size && size < offset + count) ||
			     (offset < be   && be <= offset + count)  ||
			     (size <= offset  && offset + count <= be)) && bi > 0;
	if (!shouldUpdate)
		return false;

	/* Calculate the new disksize */
	dSize = SVal(std::min(be, offset + count));
	return true;
}

SVal Interpreter::executeReadFS(void *file, Type *intTyp, void *buf, Type *bufElemTyp, SVal offset,
				SVal count, const std::unique_ptr<EventDeps> &deps)
{
	Type *intPtrType = intTyp->getPointerTo();
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);
	SVal nr = SVal(-1);

	/* Check if we can read from the file */
	auto flagsOffset = GET_FILE_FLAGS_ADDR(file);
	auto flags = CALL_DRIVER(handleLoad,
				 ReadLabel::create(currPos(), AtomicOrdering::NotAtomic,
						   flagsOffset, asize, atyp, GET_DEPS(deps))).value();
	if (!(GENMC_OPEN_FMODE(flags.get()) & GENMC_FMODE_READ)) {
		handleSystemError(SystemError::SE_EBADF, "File not opened for reading in read()");
		return nr;
	}

	/* Fetch the address of the inode */
	auto fileInode = GET_FILE_INODE_ADDR(file);
	auto *inode = (void *) CALL_DRIVER(handleLoad,
					  ReadLabel::create(currPos(), llvm::AtomicOrdering::Monotonic,
							    fileInode, getTypeSize(intPtrType),
							    AType::Pointer, GET_DEPS(deps))).value().get();

	/* Read the inode size and check whether we are reading past EOF */
	auto iSize = readInodeSizeFS(inode, intTyp, deps);
	if (offset.sge(iSize)) {
		nr = SVal(0);
		return nr;
	}

	/* Calculate how many bytes we can actually read from the file... */
	nr = iSize - offset;
	nr = (nr.getSigned() >= count.getSigned()) ? count : nr;

	/* ... and go ahead and read them one by one. (Here we cheat and not make
	 * the read buffered, since it doesn't make a difference.)  */
	readDataFromDisk(inode, offset.getSigned(), buf, 0, nr.getSigned(), bufElemTyp, deps);

	if (MI->fsInfo.journalData == JournalDataFS::journal) {
		auto inTrans = getInodeTransStatus(inode, intTyp);
		if (inTrans == SVal(1)) {
			getCurThr().rollToSnapshot();
			getCurThr().block(BlockageType::Cons);
			return SVal(42); /* propagate block */
		}
	}
	return nr;
}

void Interpreter::callReadFS(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue fd = ArgVals[0];
	GenericValue *buf = (GenericValue *) GVTOP(ArgVals[1]);
	SVal count = ArgVals[2].IntVal.getLimitedValue();
	Type *bufElemTyp = Type::getInt8Ty(F->getContext());
	Type *intTyp = F->getFunctionType()->getParamType(0);
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);
	Type *retTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* We get the address of the file description, from which we will get
	 * the reading offset, as well as the address of the inode. */
	auto *file = getFileFromFd(fd.IntVal.getLimitedValue());
	if (!file) {
		handleSystemError(SystemError::SE_EBADF, "File is not open in read()");
		returnValueToCaller(retTyp, INT_TO_GV(intTyp, -1));
		return;
	}

	/* First, we must get the file's lock. If we fail to acquire the lock,
	 * we reset the EE to this instruction */
	auto fileLock = GET_FILE_POS_LOCK_ADDR(file);
	handleLock(fileLock, asize, &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return;
	}

	/* Now we can read the offset at which we will try to read... */
	auto fileOffset = GET_FILE_OFFSET_ADDR(file);
	auto offset = CALL_DRIVER(handleLoad,
				 ReadLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
						   fileOffset, asize, atyp, GET_DEPS(deps))).value();

	auto nr = executeReadFS(file, intTyp, buf, bufElemTyp, offset, count, deps);
	if (getCurThr().isBlocked())
		return;

	/* If the read succeeded, update the offset in the file description... */
	if (nr.getSigned() >= 0) {
		offset += nr;
		CALL_DRIVER(handleStore,
			   WriteLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
					      fileOffset, asize, atyp, offset, GET_DEPS(deps)));
	}

	/* ...and then release the description's lock */
	handleUnlock(fileLock, asize, &*deps);

	/* Return #bytes read -- if successful, fullfills the read request in full */
	returnValueToCaller(retTyp, SVAL_TO_GV(nr, retTyp));
	return;
}

void Interpreter::zeroDskRangeFS(void *inode, SVal start, SVal end, Type *writeIntTyp)
{
	auto *dataOffset = (char *) GET_INODE_DATA_ADDR(inode);
	auto asize = getTypeSize(writeIntTyp);
	auto atyp = TYPE_TO_ATYPE(writeIntTyp);
	for (auto i = start.get(); i < end.get(); i++) {
		auto addr = dataOffset + i;
		CALL_DRIVER(handleDskWrite,
			   DskWriteLabel::create(currPos(), addr, asize, atyp, SVal(0),
						 GET_DATA_MAPPING(inode)));
	}
	return;
}

SVal Interpreter::executeWriteChecksFS(void *inode, Type *intTyp, SVal flags, SVal offset, SVal count,
				       SVal &wOffset, const std::unique_ptr<EventDeps> &deps)
{
	if (count.getSigned() <= 0)
		return count;

	/* Non-POSIX-compliant behavior for pwrite() -- see ext4_write_checks() */
	wOffset = offset;
	if (flags.get() & GENMC_O_APPEND)
		wOffset = readInodeSizeFS(inode, intTyp, deps);

	/* Check if the maximum file size is going to be exceeded */
	if (wOffset.getSigned() >= MI->fsInfo.maxFileSize) {
		handleSystemError(SystemError::SE_EFBIG, "Offset too big in write()");
		return SVal(-1);
	}
	return SVal(std::min(count.getSigned(), (int64_t) MI->fsInfo.maxFileSize - wOffset.getSigned()));
}

SVal Interpreter::executeBufferedWriteFS(void *inode, Type *intTyp, void *buf, Type *bufElemTyp,
					 SVal wOffset, SVal count, const std::unique_ptr<EventDeps> &deps)
{
	auto *inodeData = GET_INODE_DATA_ADDR(inode);

	/* Special care for appends and past-EOF writes (we zero ranges lazily) */
	SVal dSize, ordRangeBegin;
	auto iSize = readInodeSizeFS(inode, intTyp, deps);
	if (MI->fsInfo.delalloc && shouldUpdateInodeDisksizeFS(inode, intTyp, iSize, wOffset, count, dSize)) {
		zeroDskRangeFS(inode, iSize, dSize, bufElemTyp);
		updateInodeDisksizeFS(inode, intTyp, dSize, iSize, dSize);
	}
	if (wOffset.getSigned() > iSize.getSigned())
		zeroDskRangeFS(inode, iSize, wOffset, bufElemTyp);

	/* Block-wise write (we know that count > 0 at this point) */
	ordRangeBegin = 0;
	auto dataCount = count.get();
	auto bufOffset = 0;
	auto inodeOffset = wOffset.get();
	do {
		/* Calculate amount to write and align write */
		auto bytes = std::min<unsigned long>(MI->fsInfo.blockSize, dataCount); /* should be small enough */
		auto blockIndex = inodeOffset % MI->fsInfo.blockSize;
		auto blockRem = MI->fsInfo.blockSize - blockIndex;
		if (blockIndex != 0 && bytes > blockRem)
			bytes = blockRem;

		if (MI->fsInfo.journalData == JournalDataFS::journal)
			setInodeTransStatus(inode, intTyp, SVal(1));

		/* Write data */
		writeDataToDisk(buf, bufOffset, inode, inodeOffset, bytes, bufElemTyp, deps);

		/* Update the inode's size, if necessary */
		SVal newSize = SVal(inodeOffset + bytes);
		if (newSize.getSigned() > readInodeSizeFS(inode, intTyp, deps).getSigned()) {
			updateInodeDisksizeFS(inode, intTyp, newSize, ordRangeBegin, newSize);
			updateInodeSizeFS(inode, intTyp, newSize, deps);
			ordRangeBegin = newSize;
		}

		if (MI->fsInfo.journalData == JournalDataFS::journal) {
			setInodeTransStatus(inode, intTyp, SVal(0));
			executeFsyncFS(inode, intTyp);
		}

		bufOffset += bytes;
		inodeOffset += bytes;
		dataCount -= bytes;
	} while (dataCount);

	return count;
}

SVal Interpreter::executeWriteFS(void *file, Type *intTyp, void *buf, Type *bufElemTyp,
				 SVal offset, SVal count, const std::unique_ptr<EventDeps> &deps)
{
	Type *intPtrType = intTyp->getPointerTo();
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);

	/* Check if we can write to the file */
	auto flagsOffset = GET_FILE_FLAGS_ADDR(file);
	auto flags = CALL_DRIVER(handleLoad,
				 ReadLabel::create(currPos(), AtomicOrdering::NotAtomic,
						   flagsOffset, asize, atyp, GET_DEPS(deps))).value();
	if (!(GENMC_OPEN_FMODE(flags.get()) & GENMC_FMODE_WRITE)) {
		handleSystemError(SystemError::SE_EBADF, "File not open for writing in write()");
		return SVal(-1);
	}

	/* Fetch the inode */
	auto fileInode = GET_FILE_INODE_ADDR(file);
	auto *inode = (void *) CALL_DRIVER(handleLoad,
					  ReadLabel::create(currPos(), llvm::AtomicOrdering::Monotonic,
							    fileInode, getTypeSize(intPtrType),
							    AType::Pointer, GET_DEPS(deps))).value().get();

	/* Since we are writing, we need to lock of the inode */
	auto inodeLock = GET_INODE_LOCK_ADDR(inode);
	handleLock(inodeLock, asize, &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return SVal(42); // lock acquisition failed
	}

	SVal ret = SVal(0);
	SVal wOffset, wCount;

	wCount = executeWriteChecksFS(inode, intTyp, flags, offset, count, wOffset, deps);
	if (wCount.getSigned() <= 0)
		goto out;

	ret = executeBufferedWriteFS(inode, intTyp, buf, bufElemTyp, wOffset, wCount, deps);

out:
	/* Release inode's lock */
	handleUnlock(inodeLock, asize, &*deps);

	/* Check for O_DSYNC/O_SYNC if write was performed */
	if (ret.getSigned() > 0) {
		if (flags.get() & GENMC_O_SYNC)
			executeFsyncFS(inode, intTyp);
	}
	return ret;
}

void Interpreter::callWriteFS(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue fd = ArgVals[0];
	GenericValue *buf = (GenericValue *) GVTOP(ArgVals[1]);
	SVal count = ArgVals[2].IntVal.getLimitedValue();
	Type *bufElemTyp = Type::getInt8Ty(F->getContext());
	Type *intTyp = F->getFunctionType()->getParamType(0);
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);
	Type *retTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* We get the address of the file description, from which we will get
	 * the writing offset, as well as the address of the inode. */
	auto *file = getFileFromFd(fd.IntVal.getLimitedValue());
	if (!file) {
		handleSystemError(SystemError::SE_EBADF, "File is not open in write()");
		returnValueToCaller(retTyp, INT_TO_GV(retTyp, -1));
		return;
	}

	/* First, we must get the file's lock. If we fail to acquire the lock,
	 * we reset the EE to this instruction */
	auto fileLock = GET_FILE_POS_LOCK_ADDR(file);
	handleLock(fileLock, asize, &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return;
	}

	/* Now we can read the offset at which we will try to write... */
	auto fileOffset = GET_FILE_OFFSET_ADDR(file);
	auto offset = CALL_DRIVER(handleLoad,
				 ReadLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
						   fileOffset, asize, atyp, GET_DEPS(deps))).value();

	/* Perform the disk write operation -- may not succeed
	 * (If we failed to acquire the node's lock, block without returning anything) */
	auto nw = executeWriteFS(file, intTyp, buf, bufElemTyp, offset, count, deps);
	if (nw.getSigned() == 42)
		return; // SPECIAL CASE: lock acquisition failed

	/* If the write succeeded, we update the offset in the file description... */
	if (nw.getSigned() >= 0) {
		offset += nw;
		CALL_DRIVER(handleStore,
			   WriteLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
					      fileOffset, asize, atyp, offset, GET_DEPS(deps)));
	}

	/* We release the file description's lock */
	handleUnlock(fileLock, asize, &*deps);

	/* Return #bytes written -- if successful, fulfills the request in full  */
	returnValueToCaller(retTyp, SVAL_TO_GV(nw, retTyp));
	return;
}

void Interpreter::executeFsyncFS(void *inode, Type *intTyp)
{
	/* No need to clear reserved blocks */

	/* do the fsync() */
	CALL_DRIVER(handleDskFsync, DskFsyncLabel::create(currPos(), inode, getTypeSize(MI->fsInfo.inodeTyp)));
	return;
}

void Interpreter::callFsyncFS(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue fd = ArgVals[0];
	Type *retTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	auto *file = getFileFromFd(fd.IntVal.getLimitedValue());
	if (!file) {
		handleSystemError(SystemError::SE_EBADF, "File is not open in fsync()");
		returnValueToCaller(retTyp, INT_TO_GV(retTyp, -1));
		return;
	}

	auto fileInode = GET_FILE_INODE_ADDR(file);
	auto *inode = (void *) CALL_DRIVER(handleLoad,
					  ReadLabel::create(currPos(), llvm::AtomicOrdering::Monotonic, fileInode,
							    getTypeSize(retTyp->getPointerTo()),
							    AType::Pointer, GET_DEPS(deps))).value().get();
	executeFsyncFS(inode, retTyp);

	returnValueToCaller(retTyp, INT_TO_GV(retTyp, 0));
	return;
}

void Interpreter::callSyncFS(Function *F, const std::vector<GenericValue> &ArgVals,
			     const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleDskSync, DskSyncLabel::create(currPos()));
	return;
}

void Interpreter::callPreadFS(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue fd = ArgVals[0];
	GenericValue *buf = (GenericValue *) GVTOP(ArgVals[1]);
	SVal count = ArgVals[2].IntVal.getLimitedValue();
	SVal offset = ArgVals[3].IntVal.getLimitedValue();
	Type *bufElemTyp = Type::getInt8Ty(F->getContext());
	Type *intTyp = F->getFunctionType()->getParamType(0);
	Type *retTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* Check if the given offset is valid */
	if (offset.getSigned() < 0) {
		handleSystemError(SystemError::SE_EINVAL, "Invalid offset in pread()");
		returnValueToCaller(retTyp, INT_TO_GV(retTyp, -1));
		return;
	}

	/* We get the address of the file description, from which we will get
	 * the reading offset. In contrast to read(), we do not get the offset's lock */
	auto *file = getFileFromFd(fd.IntVal.getLimitedValue());
	if (!file) {
		handleSystemError(SystemError::SE_EBADF, "File is not open in pread()");
		returnValueToCaller(retTyp, INT_TO_GV(retTyp, -1));
		return;
	}

	/* Execute the read in the specified offset */
	auto nr = executeReadFS(file, intTyp, buf, bufElemTyp, offset, count, deps);
	if (getCurThr().isBlocked())
		return;

	/* Return the number of bytes read (similar to read()) */
	returnValueToCaller(retTyp, SVAL_TO_GV(nr, retTyp));
	return;
}

void Interpreter::callPwriteFS(Function *F, const std::vector<GenericValue> &ArgVals,
			       const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue fd = ArgVals[0];
	GenericValue *buf = (GenericValue *) GVTOP(ArgVals[1]);
	SVal count = ArgVals[2].IntVal.getLimitedValue();
	SVal offset = ArgVals[3].IntVal.getLimitedValue();
	Type *bufElemTyp = Type::getInt8Ty(F->getContext());
	Type *intTyp = F->getFunctionType()->getParamType(0);
	Type *retTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* Check if the given offset is valid */
	if (offset.getSigned() < 0) {
		handleSystemError(SystemError::SE_EINVAL, "Invalid offset in pwrite()");
		returnValueToCaller(retTyp, INT_TO_GV(retTyp, -1));
		return;
	}

	/* We get the address of the file description, from which we will get
	 * the reading offset. In contrast to write(), we do not get the offset's lock */
	auto *file = getFileFromFd(fd.IntVal.getLimitedValue());
	if (!file) {
		handleSystemError(SystemError::SE_EBADF, "File is not open in pwrite()");
		returnValueToCaller(retTyp, INT_TO_GV(retTyp, -1));
		return;
	}

	/* Execute the write in the specified offset */
	auto nw = executeWriteFS(file, intTyp, buf, bufElemTyp, offset, count, deps);

	/* Return the number of bytes written */
	returnValueToCaller(retTyp, SVAL_TO_GV(nw, retTyp));
	return;
}

SVal Interpreter::executeLseekFS(void *file, Type *intTyp, SVal offset, SVal whence,
				 const std::unique_ptr<EventDeps> &deps)
{
	Type *intPtrType = intTyp->getPointerTo();
	auto asize = getTypeSize(intTyp);
	auto atyp = TYPE_TO_ATYPE(intTyp);

	/* We get the address of the inode (to read isize, as in ext4_llseek) */
	auto fileInode = GET_FILE_INODE_ADDR(file);
	auto *inode = (void*) CALL_DRIVER(handleLoad,
					 ReadLabel::create(currPos(), llvm::AtomicOrdering::Monotonic, fileInode,
							   getTypeSize(intPtrType), AType::Pointer,
							   GET_DEPS(deps))).value().get();

	/* We read the inode size before switching on WHENCE */
	auto fileSize = readInodeSizeFS(inode, intTyp, deps);

	/* Then, we calculate the new offset before updating it */
	SVal newOffset;
	auto fileOffset = GET_FILE_OFFSET_ADDR(file);
	switch (whence.get()) {
	case GENMC_SEEK_SET:
		newOffset = offset;
		break;
	case GENMC_SEEK_END:
		newOffset = offset;
		newOffset += fileSize;
		break;
	case GENMC_SEEK_CUR: {
		if (offset.get() == 0) {
			/* Special case: Position-querying operation */
			return CALL_DRIVER(handleLoad,
					  ReadLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
							    fileOffset, asize, atyp, GET_DEPS(deps))).value();
		}

		/* This case is weird in the kernel: The offset is updated while holding
		 * f_lock (as opposed to f_pos_lock), but I think this is a leftover from
		 * some previous version, where only SEEK_CURs would synchronize. Now,
		 * since we hold f_pos_lock anyway, getting f_lock as well seems
		 * unnecessary, at least for our purposes.
		 *
		 * In any case, the update is performed within the switch statement,
		 * as is done in the kernel. */
		newOffset = CALL_DRIVER(handleLoad,
				       ReadLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
							 fileOffset, asize, atyp, GET_DEPS(deps))).value();
		newOffset += offset;
		CALL_DRIVER(handleStore,
			   WriteLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic, fileOffset,
					      asize, atyp, newOffset.get(), GET_DEPS(deps)));
		return newOffset;
	}
	default:
		handleSystemError(SystemError::SE_EINVAL, "whence is not valid in lseek()");
		newOffset = SVal(-1); // signed
		return newOffset;
	}
	/* Update the offset (if lseek has not already returned) */
	CALL_DRIVER(handleStore,
		   WriteLabel::create(currPos(), llvm::AtomicOrdering::NotAtomic,
				      fileOffset, asize, atyp, newOffset.get(), GET_DEPS(deps)));
	return newOffset;
}

void Interpreter::callLseekFS(Function *F, const std::vector<GenericValue> &ArgVals,
			      const std::unique_ptr<EventDeps> &specialDeps)
{
	GenericValue fd = ArgVals[0];
	SVal offset = ArgVals[1].IntVal.getLimitedValue();
	SVal whence = ArgVals[2].IntVal.getLimitedValue();
	Type *intTyp = F->getFunctionType()->getParamType(0);
	Type *retTyp = F->getReturnType();

	getCurThr().takeSnapshot();
	auto deps = makeEventDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
				  getAddrPoDeps(getCurThr().id), nullptr);

	/* We get the address of the file description, from which we will get
	 * the offset to modify */
	auto *file = getFileFromFd(fd.IntVal.getLimitedValue());
	if (!file) {
		handleSystemError(SystemError::SE_EBADF, "File is not open in lseek()");
		returnValueToCaller(retTyp, INT_TO_GV(retTyp, -1));
		return;
	}

	/* First, we must get the file's lock. If we fail to acquire the lock,
	 * we reset the EE to this instruction */
	auto *fileLock = GET_FILE_POS_LOCK_ADDR(file);
	handleLock(fileLock, getTypeSize(intTyp), &*deps);
	if (getCurThr().isBlocked()) {
		getCurThr().rollToSnapshot();
		return;
	}

	auto newOffset = executeLseekFS(file, intTyp, offset, whence, deps);

	/* We release the file description's lock */
	handleUnlock(fileLock, getTypeSize(intTyp), &*deps);
	returnValueToCaller(retTyp, SVAL_TO_GV(newOffset, retTyp));
	return;
}

void Interpreter::callPersBarrierFS(Function *F, const std::vector<GenericValue> &ArgVals,
				    const std::unique_ptr<EventDeps> &specialDeps)
{
	CALL_DRIVER(handleDskPbarrier, DskPbarrierLabel::create(currPos()));
	return;
}

bool isInternalCall(Function *F)
{
	return internalFunNames.count(F->getName().str());
}

bool isInvalidRecCall(InternalFunctions fCode, const std::vector<GenericValue> &ArgVals)
{
	return isFsInvalidRecCode(fCode) ||
		(fCode == InternalFunctions::FN_OpenFS &&
		 (ArgVals[1].IntVal.getLimitedValue() & GENMC_O_CREAT));
}

#define CALL_INTERNAL_FUNCTION(NAME)			\
	case InternalFunctions::FN_##NAME:		\
		call##NAME(F, ArgVals, specialDeps);	\
		break

void Interpreter::callInternalFunction(Function *F, const std::vector<GenericValue> &ArgVals,
				       const std::unique_ptr<EventDeps> &specialDeps)
{
	auto fCode = internalFunNames.at(F->getName().str());

	/* Make sure we are not trying to make an invalid call during recovery */
	if (getProgramState() == ProgramState::Recovery && isInvalidRecCall(fCode, ArgVals)) {
		driver->reportError(currPos(), VerificationError::VE_InvalidRecoveryCall,
				    F->getName().str() + "() cannot be called during recovery");
		return;
	}

	switch (fCode) {
		CALL_INTERNAL_FUNCTION(AssertFail);
		CALL_INTERNAL_FUNCTION(OptBegin);
		CALL_INTERNAL_FUNCTION(LoopBegin);
		CALL_INTERNAL_FUNCTION(SpinStart);
		CALL_INTERNAL_FUNCTION(SpinEnd);
		CALL_INTERNAL_FUNCTION(FaiZNESpinEnd);
		CALL_INTERNAL_FUNCTION(LockZNESpinEnd);
		CALL_INTERNAL_FUNCTION(KillThread);
		CALL_INTERNAL_FUNCTION(Assume);
		CALL_INTERNAL_FUNCTION(NondetInt);
		CALL_INTERNAL_FUNCTION(Malloc);
		CALL_INTERNAL_FUNCTION(MallocAligned);
		CALL_INTERNAL_FUNCTION(PMalloc);
		CALL_INTERNAL_FUNCTION(Free);
		CALL_INTERNAL_FUNCTION(ThreadSelf);
		CALL_INTERNAL_FUNCTION(ThreadCreate);
		CALL_INTERNAL_FUNCTION(ThreadCreateSymmetric);
		CALL_INTERNAL_FUNCTION(ThreadJoin);
		CALL_INTERNAL_FUNCTION(ThreadExit);
		CALL_INTERNAL_FUNCTION(AtExit);
		CALL_INTERNAL_FUNCTION(MutexInit);
		CALL_INTERNAL_FUNCTION(MutexLock);
		CALL_INTERNAL_FUNCTION(MutexUnlock);
		CALL_INTERNAL_FUNCTION(MutexTrylock);
		CALL_INTERNAL_FUNCTION(MutexDestroy);
		CALL_INTERNAL_FUNCTION(BarrierInit);
		CALL_INTERNAL_FUNCTION(BarrierWait);
		CALL_INTERNAL_FUNCTION(BarrierDestroy);
		CALL_INTERNAL_FUNCTION(HazptrAlloc);
		CALL_INTERNAL_FUNCTION(HazptrProtect);
		CALL_INTERNAL_FUNCTION(HazptrClear);
		CALL_INTERNAL_FUNCTION(HazptrFree);
		CALL_INTERNAL_FUNCTION(HazptrRetire);
		CALL_INTERNAL_FUNCTION(OpenFS);
		CALL_INTERNAL_FUNCTION(CreatFS);
		CALL_INTERNAL_FUNCTION(CloseFS);
		CALL_INTERNAL_FUNCTION(RenameFS);
		CALL_INTERNAL_FUNCTION(LinkFS);
		CALL_INTERNAL_FUNCTION(UnlinkFS);
		CALL_INTERNAL_FUNCTION(TruncateFS);
		CALL_INTERNAL_FUNCTION(ReadFS);
		CALL_INTERNAL_FUNCTION(WriteFS);
		CALL_INTERNAL_FUNCTION(FsyncFS);
		CALL_INTERNAL_FUNCTION(SyncFS);
		CALL_INTERNAL_FUNCTION(PreadFS);
		CALL_INTERNAL_FUNCTION(PwriteFS);
		CALL_INTERNAL_FUNCTION(LseekFS);
		CALL_INTERNAL_FUNCTION(PersBarrierFS);
		CALL_INTERNAL_FUNCTION(SmpFenceLKMM);
		CALL_INTERNAL_FUNCTION(RCUReadLockLKMM);
		CALL_INTERNAL_FUNCTION(RCUReadUnlockLKMM);
		CALL_INTERNAL_FUNCTION(SynchronizeRCULKMM);
		CALL_INTERNAL_FUNCTION(CLFlush);
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
    auto Result = callExternalFunction (F, translated);
    // Simulate a 'ret' instruction of the appropriate type.
    popStackAndReturnValueToCaller (F->getReturnType (), Result);
    return;
  }

  // Get pointers to first LLVM BB & Instruction in function.
  StackFrame.CurBB     = &F->front();
  StackFrame.CurInst   = StackFrame.CurBB->begin();

  // Run through the function arguments and initialize their values...
  assert((ArgVals.size() == F->arg_size() ||
         (ArgVals.size() > F->arg_size() && F->getFunctionType()->isVarArg()))&&
         "Invalid number of values passed to function invocation!");

  // Handle non-varargs arguments...
  unsigned i = 0;
  for (Function::arg_iterator AI = F->arg_begin(), E = F->arg_end();
       AI != E; ++AI, ++i)
    SetValue(&*AI, ArgVals[i], StackFrame);

  // Handle varargs arguments...
  StackFrame.VarArgs.assign(ArgVals.begin()+i, ArgVals.end());
}

std::string getFilenameFromMData(MDNode *node)
{
	const llvm::DILocation &loc = static_cast<const llvm::DILocation&>(*node);
	llvm::StringRef file = loc.getFilename();
	llvm::StringRef dir = loc.getDirectory();

	BUG_ON(!file.size());

	std::string absPath;
	if (file.front() == '/') {
		absPath = file.str();
	} else {
		BUG_ON(!dir.size());
		absPath = dir.str();
		if (absPath.back() != '/')
			absPath += "/";
		absPath += file;
	}
	return absPath;
}

void Interpreter::replayExecutionBefore(const VectorClock &before)
{
	reset();
	setExecState(ExecutionState::Replay);
	setProgramState(ProgramState::Main);

	/* We have to replay all threads in order to get debug metadata */
	for (auto i = 0u; i < before.size(); i++) {
		auto &thr = getThrById(i);
		if (thr.isMain())
			thr.ECStack = mainECStack;
		else
			thr.ECStack = thr.initEC;
		thr.prefixLOC.clear();
		thr.prefixLOC.resize(before.getMax(i) + 2); /* Grow since it can be accessed */
		scheduleThread(i);
		if (thr.threadFun == recoveryRoutine)
			setProgramState(ProgramState::Recovery);
		/* Make sure to refetch references within the loop (invalidation danger) */
		while ((int) getCurThr().globalInstructions < before.getMax(i)) {
			int snap = getCurThr().globalInstructions;
			ExecutionContext &SF = ECStack().back();
			Instruction &I = *SF.CurInst++;
			visit(I);

			/* Collect metadata only for global instructions */
			if (getCurThr().globalInstructions == snap)
				continue;
			/* If there are no metadata for this instruction, skip */
			if (!I.getMetadata("dbg"))
				continue;

			/* Store relevant trace information in the appropriate spot */
			int line = I.getDebugLoc().getLine();
			std::string file = getFilenameFromMData(I.getMetadata("dbg"));
			getCurThr().prefixLOC[snap + 1] = std::make_pair(line, file);

			/* If the instruction maps to more than one events, we have to fill more spots */
			for (auto i = snap + 2; i <= std::min((int) getCurThr().globalInstructions, before.getMax(i)); i++)
				getCurThr().prefixLOC[i] = std::make_pair(line, file);
		}
	}
}

void Interpreter::run()
{
	while (driver->scheduleNext()) {
		if (driver->tryOptimizeScheduling(currPos()))
			continue;
		llvm::ExecutionContext &SF = ECStack().back();
		llvm::Instruction &I = *SF.CurInst++;
		visit(I);
	}
	return;
}

int Interpreter::runAsMain(const std::string &main)
{
	setupStaticCtorsDtors(true);
	setupMain(FindFunctionNamed(main), {"prog"}, nullptr);
	setupStaticCtorsDtors(false);

	mainECStack = getThrById(0).initEC = ECStack();
	setProgramState(llvm::ProgramState::Main);
	driver->handleExecutionStart();
	run();
	driver->handleExecutionEnd();
	return dynState.ExitValue.IntVal.getZExtValue();
}

void Interpreter::runRecovery()
{
	setProgramState(llvm::ProgramState::Recovery);
	driver->handleRecoveryStart();
	run();
	driver->handleRecoveryEnd();
}
