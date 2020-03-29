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

// static cl::opt<bool> PrintVolatile("interpreter-print-volatile", cl::Hidden,
//           cl::desc("make the interpreter print every volatile load and store"));

//===----------------------------------------------------------------------===//
//                     Various Helper Functions
//===----------------------------------------------------------------------===//

static GenericValue executeICMP_EQ(GenericValue Src1, GenericValue Src2, Type *typ);

static void SetValue(Value *V, GenericValue Val, ExecutionContext &SF) {
  SF.Values[V] = Val;
}

bool Interpreter::compareValues(const llvm::Type *typ, const GenericValue &val1, const GenericValue &val2)
{
	return executeICMP_EQ(val1, val2, (Type *)typ).IntVal.getBoolValue();
}

/* Returns the initial value for the specified memory location */
GenericValue Interpreter::getLocInitVal(GenericValue *ptr, Type *typ)
{
	GenericValue result;
	LoadValueFromMemory(result, ptr, typ);
	return result;
}

/* Returns the size (in bytes) for a given type */
unsigned int Interpreter::getTypeSize(Type *typ)
{
	return (size_t) TD.getTypeAllocSize(typ);
}


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
  case Type::VectorTyID: {                                           \
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
  Thread &thr = getCurThr();
  Type *Ty    = I.getOperand(0)->getType();
  GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue R;   // Result

  updateDataDeps(thr.id, &I, I.getOperand(0));
  updateDataDeps(thr.id, &I, I.getOperand(1));
  // thr.dataDeps[&I] = thr.dataDeps[I.getOperand(0)].
  // 	  depUnion(thr.dataDeps[I.getOperand(1)]);

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
  case Type::VectorTyID:                                            \
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
  Thread &thr = getCurThr();
  Type *Ty    = I.getOperand(0)->getType();
  GenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  GenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  GenericValue R;   // Result

  /* Update dependencies */
  updateDataDeps(thr.id, &I, I.getOperand(0));
  updateDataDeps(thr.id, &I, I.getOperand(1));
  // thr.dataDeps[&I] = thr.dataDeps[I.getOperand(0)].
  // 	  depUnion(thr.dataDeps[I.getOperand(1)]);

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

void Interpreter::exitCalled(GenericValue GV) {
  // runAtExitHandlers() assumes there are no stack frames, but
  // if exit() was called, then it had a stack frame. Blow away
  // the stack before interpreting atexit handlers.
  ECStack().clear();
  runAtExitHandlers();
  exit(GV.IntVal.zextOrTrunc(32).getZExtValue());
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
                                                 GenericValue Result) {
  // Keep track of the ret inst to update deps if necessary...
  auto *retI = dyn_cast<ReturnInst>(ECStack().back().CurInst->getPrevNode());

  // Pop the current stack frame.
  ECStack().pop_back();

  // if (ECStack.empty()) {  // Finished main.  Put result into exit code...
  //   if (RetTy && !RetTy->isVoidTy()) {          // Nonvoid return type?
  //     ExitValue = Result;   // Capture the exit value of the program
  //   } else {
  //     memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
  //   }
  if (ECStack().empty()) {
    setCurrentDeps(nullptr, nullptr, nullptr, nullptr, nullptr);
    driver->visitThreadFinish();
  } else {
    // If we have a previous stack frame, and we have a previous call,
    // fill in the return value...
    ExecutionContext &CallingSF = ECStack().back();
    if (Instruction *I = CallingSF.Caller.getInstruction()) {
      // Save result...
      if (!CallingSF.Caller.getType()->isVoidTy()) {
	if (retI) // if we are coming from a ret inst, update deps
	  updateDataDeps(getCurThr().id, I, retI->getReturnValue());
        SetValue(I, Result, CallingSF);
      }
      if (InvokeInst *II = dyn_cast<InvokeInst> (I))
        SwitchToNewBasicBlock (II->getNormalDest (), CallingSF);
      CallingSF.Caller = CallSite();          // We returned from the call...
    }
  }
}

void Interpreter::returnValueToCaller(Type *RetTy, GenericValue Result)
{
	assert(!ECStack().empty());
	// fill in the return value...
	ExecutionContext &CallingSF = ECStack().back();
	if (Instruction *I = CallingSF.Caller.getInstruction()) {
		// Save result...
		if (!CallingSF.Caller.getType()->isVoidTy())
			SetValue(I, Result, CallingSF);
		if (InvokeInst *II = dyn_cast<InvokeInst> (I))
			SwitchToNewBasicBlock (II->getNormalDest (), CallingSF);
		CallingSF.Caller = CallSite();          // We returned from the call...
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

  popStackAndReturnValueToCaller(RetTy, Result);
}

void Interpreter::visitUnreachableInst(UnreachableInst &I) {
  report_fatal_error("Program executed an 'unreachable' instruction!");
}

void Interpreter::visitBranchInst(BranchInst &I) {
  ExecutionContext &SF = ECStack().back();
  Thread &thr = getCurThr();
  BasicBlock *Dest;

  Dest = I.getSuccessor(0);          // Uncond branches have a fixed dest...
  if (!I.isUnconditional()) {
    Value *Cond = I.getCondition();
    if (getOperandValue(Cond, SF).IntVal == 0) // If false cond...
      Dest = I.getSuccessor(1);
    updateCtrlDeps(thr.id, Cond);
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
#ifdef LLVM_SWITCHINST_CASEIT_NEEDS_DEREF
    auto &it = *i;
#else
    auto &it = i;
#endif
    GenericValue CaseVal = getOperandValue(it.getCaseValue(), SF);
    if (executeICMP_EQ(CondVal, CaseVal, ElTy).IntVal != 0) {
      Dest = cast<BasicBlock>(it.getCaseSuccessor());
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
    // getCurThr().dataDeps[PN] = getCurThr().dataDeps[IncomingValue];
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

  Type *Ty = I.getType()->getElementType();  // Type to be allocated

  // Get the number of elements being allocated by the array...
  unsigned NumElements =
    getOperandValue(I.getOperand(0), SF).IntVal.getZExtValue();

  unsigned TypeSize = (size_t)TD.getTypeAllocSize(Ty);

  // Avoid malloc-ing zero bytes, use max()...
  unsigned MemToAlloc = std::max(1U, NumElements * TypeSize);

  /* The driver will provide the address this alloca returns */
  setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		 getAddrPoDeps(getCurThr().id), nullptr);

  GenericValue Result = driver->visitMalloc(MemToAlloc, true);

  updateDataDeps(getCurThr().id, &I, Event(getCurThr().id, getCurThr().globalInstructions));

  /* If this is not a replay, update naming information for this variable
   * based on previously collected information */
  if (!stackVars.count(Result.PointerVal))
	  updateVarNameInfo(&I, (char *) Result.PointerVal, MemToAlloc, true);

  SetValue(&I, Result, SF);
}

// getElementOffset - The workhorse for getelementptr.
//
GenericValue Interpreter::executeGEPOperation(Value *Ptr, gep_type_iterator I,
                                              gep_type_iterator E,
                                              ExecutionContext &SF) {
  assert(Ptr->getType()->isPointerTy() &&
         "Cannot getElementOffset of a nonpointer type!");

  Thread &thr = getCurThr();
  updateDataDeps(thr.id, SF.CurInst->getPrevNode(), Ptr);

  uint64_t Total = 0;
  for (; I != E; ++I) {
    updateDataDeps(thr.id, SF.CurInst->getPrevNode(), I.getOperand());
#ifdef LLVM_NEW_GEP_TYPE_ITERATOR_API
    if (StructType *STy = I.getStructTypeOrNull()) {
#else
    if (StructType *STy = dyn_cast<StructType>(*I)) {
#endif
      const StructLayout *SLO = TD.getStructLayout(STy);

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
#ifdef LLVM_NEW_GEP_TYPE_ITERATOR_API
      Total += TD.getTypeAllocSize(I.getIndexedType()) * Idx;
#else
      Total += TD.getTypeAllocSize(cast<SequentialType>(*I)->getElementType()) * Idx;
#endif
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
	ExecutionContext &SF = ECStack().back();
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	GenericValue *ptr = (GenericValue *) GVTOP(src);
	Type *typ = I.getType();

	/* If this is a thread-local access it is not recorded in the graph,
	 * so just perform the load. */
	if (thr.tls.count(ptr)) {
		SetValue(&I, thr.tls[ptr], SF);
		return;
	}

	/* Otherwise, set the dependencies for this instruction.. */
	setCurrentDeps(getDataDeps(thr.id, I.getPointerOperand()), nullptr,
		       getCtrlDeps(thr.id), getAddrPoDeps(thr.id), nullptr);

	/* ... and then the driver will provide the appropriate value */
	auto val = driver->visitLoad(IA_None, I.getOrdering(), ptr, typ);

	updateDataDeps(thr.id, &I, getCurrentPosition());
	updateAddrPoDeps(thr.id, I.getPointerOperand());

	/* Last, set the return value for this instruction */
	SetValue(&I, val, SF);
	return;
}

void Interpreter::visitStoreInst(StoreInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	GenericValue val = getOperandValue(I.getOperand(0), SF);
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	GenericValue *ptr = (GenericValue *) GVTOP(src);
	Type *typ = I.getOperand(0)->getType();

	/* Do not bother with thread-local accesses */
	if (thr.tls.count(ptr)) {
		thr.tls[ptr] = val;
		return;
	}

	/* First, record the dependencies for this instruction */
	setCurrentDeps(getDataDeps(thr.id, I.getPointerOperand()),
		       getDataDeps(thr.id, I.getOperand(0)), getCtrlDeps(thr.id),
		       getAddrPoDeps(thr.id), nullptr);

	/* Inform the Driver about the newly interpreter store */
	driver->visitStore(IA_None, I.getOrdering(), ptr, typ, val);

	updateAddrPoDeps(thr.id, I.getPointerOperand());
	// thr.addrPoDeps.update(thr.dataDeps[I.getPointerOperand()]);
	return;
}

void Interpreter::visitFenceInst(FenceInst &I)
{
	setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		       nullptr, nullptr);
	driver->visitFence(I.getOrdering());
	return;
}

void Interpreter::visitAtomicCmpXchgInst(AtomicCmpXchgInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	GenericValue cmpVal = getOperandValue(I.getCompareOperand(), SF);
	GenericValue newVal = getOperandValue(I.getNewValOperand(), SF);
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	GenericValue *ptr = (GenericValue *) GVTOP(src);
	Type *typ = I.getCompareOperand()->getType();
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

	setCurrentDeps(getDataDeps(thr.id, I.getPointerOperand()),
		       getDataDeps(thr.id, I.getNewValOperand()),
		       getCtrlDeps(thr.id), getAddrPoDeps(thr.id),
		       getDataDeps(thr.id, I.getCompareOperand()));

	auto ret = driver->visitLoad(IA_Cas, I.getSuccessOrdering(), ptr, typ,
				     cmpVal, newVal);

	auto cmpRes = executeICMP_EQ(ret, cmpVal, typ);
	if (cmpRes.IntVal.getBoolValue()) {
		setCurrentDeps(getDataDeps(thr.id, I.getPointerOperand()),
			       getDataDeps(thr.id, I.getNewValOperand()),
			       getCtrlDeps(thr.id), getAddrPoDeps(thr.id), nullptr);
		driver->visitStore(IA_Cas, I.getSuccessOrdering(), ptr, typ, newVal);
	}

	/* After the RMW operation is done, update dependencies */
	updateDataDeps(thr.id, &I, getCurrentPosition());
	updateAddrPoDeps(thr.id, I.getPointerOperand());

	result.AggregateVal.push_back(ret);
	result.AggregateVal.push_back(cmpRes);
	SetValue(&I, result, SF);
	return;
}

void Interpreter::executeAtomicRMWOperation(GenericValue &result, const GenericValue &oldVal,
					    const GenericValue &val, AtomicRMWInst::BinOp op)
{
	switch (op) {
	case AtomicRMWInst::Xchg:
		result = val;
		break;
	case AtomicRMWInst::Add:
		result.IntVal = oldVal.IntVal + val.IntVal;
		break;
	case AtomicRMWInst::Sub:
		result.IntVal = oldVal.IntVal - val.IntVal;
		break;
	case AtomicRMWInst::Or:
		result.IntVal = oldVal.IntVal | val.IntVal;
		break;
	case AtomicRMWInst::Xor:
		result.IntVal = oldVal.IntVal ^ val.IntVal;
		break;
	case AtomicRMWInst::And:
		result.IntVal = oldVal.IntVal & val.IntVal;
		break;
	default:
		WARN_ONCE("invalid-rmw-op",
			  "Unsupported operation in RMW instruction!\n");
		result.IntVal = oldVal.IntVal;
	}
}

void Interpreter::visitAtomicRMWInst(AtomicRMWInst &I)
{
	Thread &thr = getCurThr();
	ExecutionContext &SF = ECStack().back();
	GenericValue src = getOperandValue(I.getPointerOperand(), SF);
	GenericValue *ptr = (GenericValue *) GVTOP(src);
	GenericValue val = getOperandValue(I.getValOperand(), SF);
	Type *typ = I.getType();
	GenericValue newVal;

	BUG_ON(!typ->isIntegerTy());

	if (thr.tls.count(ptr)) {
		GenericValue oldVal = thr.tls[ptr];
		executeAtomicRMWOperation(newVal, oldVal, val, I.getOperation());
		thr.tls[ptr] = newVal;
		SetValue(&I, oldVal, SF);
		return;
	}

	setCurrentDeps(getDataDeps(thr.id, I.getPointerOperand()),
		       getDataDeps(thr.id, I.getValOperand()),
		       getCtrlDeps(thr.id), getAddrPoDeps(thr.id), nullptr);

	auto ret = driver->visitLoad(IA_Fai, I.getOrdering(), ptr, typ,
				     GenericValue(), val, I.getOperation());

	executeAtomicRMWOperation(newVal, ret, val, I.getOperation());

	setCurrentDeps(getDataDeps(thr.id, I.getPointerOperand()),
		       getDataDeps(thr.id, I.getValOperand()),
		       getCtrlDeps(thr.id), getAddrPoDeps(thr.id), nullptr);

	driver->visitStore(IA_Fai, I.getOrdering(), ptr, typ, newVal);

	/* After the RMW operation is done, update dependencies */
	updateDataDeps(thr.id, &I, getCurrentPosition());
	updateAddrPoDeps(thr.id, I.getPointerOperand());

	SetValue(&I, ret, SF);
	return;
}

bool Interpreter::isInlineAsm(CallSite &CS, std::string *asmStr)
{
	if (!CS.isCall())
		return false;

	llvm::CallInst *CI = cast<llvm::CallInst>(CS.getInstruction());
	if (!CI || !CI->isInlineAsm())
		return false;

	llvm::InlineAsm *IA = llvm::dyn_cast<llvm::InlineAsm>(CI->getCalledValue());
	*asmStr = IA->getAsmString();
	asmStr->erase(asmStr->begin(), std::find_if(asmStr->begin(), asmStr->end(),
	        std::not1(std::ptr_fun<int, int>(std::isspace))));
	asmStr->erase(std::find_if(asmStr->rbegin(), asmStr->rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), asmStr->end());
	return true;
}

void Interpreter::visitInlineAsm(CallSite &CS, const std::string &asmStr)
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

void Interpreter::visitCallSite(CallSite CS) {

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
      SetValue(CS.getInstruction(), ArgIndex, SF);
      return;
    }
    case Intrinsic::vaend:    // va_end is a noop for the interpreter
      return;
    case Intrinsic::vacopy:   // va_copy: dest = src
      SetValue(CS.getInstruction(), getOperandValue(*CS.arg_begin(), SF), SF);
      return;
    default:
	    WARN_ONCE("unknown-intrinsic", "Unknown intrinstic function" \
		      "encountered. Attempting to lower it...\n");
      // If it is an unknown intrinsic function, use the intrinsic lowering
      // class to transform it into hopefully tasty LLVM code.
      //
      BasicBlock::iterator me(CS.getInstruction());
      BasicBlock *Parent = CS.getInstruction()->getParent();
      bool atBegin(Parent->begin() == me);
      if (!atBegin)
        --me;
      IL->LowerIntrinsicCall(cast<CallInst>(CS.getInstruction()));

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
  for (CallSite::arg_iterator i = SF.Caller.arg_begin(),
         e = SF.Caller.arg_end(); i != e; ++i, ++pNum) {
    Value *V = *i;
    ArgVals.push_back(getOperandValue(V, SF));
  }

  Thread &thr = getCurThr();
  StringRef name = CS.getCalledFunction()->getName();
  if (name == "__VERIFIER_assume") {
	  /* We have ctrl dependency on the argument of an assume() */
	  for (CallSite::arg_iterator i = SF.Caller.arg_begin(),
		       e = SF.Caller.arg_end(); i != e; ++i, ++pNum) {
		  updateCtrlDeps(thr.id, *i);
	  }
  } else if (name == "pthread_mutex_lock" ||
	     name == "pthread_mutex_unlock" ||
	     name == "pthread_mutex_trylock") {
	  /* We have addr dependency on the argument of mutex calls */
	  setCurrentDeps(getDataDeps(thr.id, *SF.Caller.arg_begin()),
			 nullptr, getCtrlDeps(thr.id),
			 getAddrPoDeps(thr.id), nullptr);
  } else {
	  Function *F = SF.Caller.getCalledFunction();
	  auto ai = F->arg_begin();
	  /* The parameters of the function called get the data
	   * dependencies of the actual arguments */
	  for (CallSite::arg_iterator ci = SF.Caller.arg_begin(),
		       ce = SF.Caller.arg_end(); ci != ce; ++ci, ++ai) {
		  updateDataDeps(getCurThr().id, &*ai, &*ci->get());
	  }
  }

  // To handle indirect calls, we must get the pointer value from the argument
  // and treat it as a function pointer.
  GenericValue SRC = getOperandValue(SF.Caller.getCalledValue(), SF);
  callFunction((Function*)GVTOP(SRC), ArgVals);
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

  if (SrcVal->getType()->getTypeID() == Type::VectorTyID) {
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

  if (SrcVal->getType()->getTypeID() == Type::VectorTyID) {
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

  if (SrcTy->getTypeID() == Type::VectorTyID) {
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

  if (SrcTy->getTypeID() == Type::VectorTyID) {
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

  if (SrcVal->getType()->getTypeID() == Type::VectorTyID) {
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

  if (SrcVal->getType()->getTypeID() == Type::VectorTyID) {
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

  uint32_t PtrSize = TD.getPointerSizeInBits();
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

  if ((SrcTy->getTypeID() == Type::VectorTyID) ||
      (DstTy->getTypeID() == Type::VectorTyID)) {
    // vector src bitcast to vector dst or vector src bitcast to scalar dst or
    // scalar src bitcast to vector dst
    bool isLittleEndian = TD.isLittleEndian();
    GenericValue TempDst, TempSrc, SrcVec;
    const Type *SrcElemTy;
    const Type *DstElemTy;
    unsigned SrcBitSize;
    unsigned DstBitSize;
    unsigned SrcNum;
    unsigned DstNum;

    if (SrcTy->getTypeID() == Type::VectorTyID) {
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

    if (DstTy->getTypeID() == Type::VectorTyID) {
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
    if (DstTy->getTypeID() == Type::VectorTyID) {
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
  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, I.getOperand(0)),
  SetValue(&I, executeTruncInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitSExtInst(SExtInst &I) {
  ExecutionContext &SF = ECStack().back();
  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, I.getOperand(0)),
  SetValue(&I, executeSExtInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitZExtInst(ZExtInst &I) {
  ExecutionContext &SF = ECStack().back();
  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, I.getOperand(0)),
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
  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, I.getOperand(0));
  SetValue(&I, executePtrToIntInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitIntToPtrInst(IntToPtrInst &I) {
  ExecutionContext &SF = ECStack().back();
  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, I.getOperand(0));
  SetValue(&I, executeIntToPtrInst(I.getOperand(0), I.getType(), SF), SF);
}

void Interpreter::visitBitCastInst(BitCastInst &I) {
  ExecutionContext &SF = ECStack().back();
  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, I.getOperand(0));
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

  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, getDataDeps(thr.id, Agg));
  // thr.dataDeps[&I] = thr.dataDeps[&I].depUnion(thr.dataDeps[Agg]);

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
    case Type::VectorTyID:
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

  Thread &thr = getCurThr();
  updateDataDeps(thr.id, &I, getDataDeps(thr.id, Agg));

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
    case Type::VectorTyID:
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

std::string getFilenameFromMData(MDNode *node)
{
#ifdef LLVM_DILOCATION_IS_MDNODE
	const llvm::DILocation &loc = static_cast<const llvm::DILocation&>(*node);
#else
	llvm::DILocation loc(node);
#endif
	std::string file = loc.getFilename();
	std::string dir = loc.getDirectory();

	BUG_ON(!file.size() || !dir.size());

	std::string absPath;
	if (file.front() == '/') {
		absPath = file;
	} else {
		absPath = dir;
		if (absPath.back() != '/')
			absPath += "/";
		absPath += file;
	}
	return absPath;
}

void Interpreter::replayExecutionBefore(const VectorClock &before)
{
	threads[0].initSF = mainECStack.back();
	for (auto i = 0u; i < before.size(); i++) {
		auto &thr = getThrById(i);
		thr.ECStack.clear();
		thr.ECStack.push_back(thr.initSF);
		thr.globalInstructions = 0;
		thr.prefixLOC.clear();
		thr.prefixLOC.resize(before[i] + 2); /* Grow since it can be accessed */
		currentThread = i;
		while ((int) thr.globalInstructions < before[i]) {
			int snap = thr.globalInstructions;
			ExecutionContext &SF = thr.ECStack.back();
			Instruction &I = *SF.CurInst++;
			visit(I);

			/* Collect metadata only for global instructions */
			if (thr.globalInstructions == snap)
				continue;
			/* If there are no metadata for this instruction, skip */
			if (!I.getMetadata("dbg"))
				continue;

			/* Store relevant trace information in the appropriate spot */
			int line = I.getDebugLoc().getLine();
			std::string file = getFilenameFromMData(I.getMetadata("dbg"));
			thr.prefixLOC[snap + 1] = std::make_pair(line, file);

			/* If I was an RMW or a TCreate, we have to fill two spots */
			if (thr.globalInstructions == snap + 2)
				thr.prefixLOC[snap + 2] = std::make_pair(line, file);
		}
	}
}

/* callAssertFail - Call to assert() macro */
void Interpreter::callAssertFail(Function *F,
				 const std::vector<GenericValue> &ArgVals)
{
	std::string err = (ArgVals.size()) ? std::string("Assertion violation: ") +
		std::string((char *) GVTOP(ArgVals[0]))	: "Unknown";

	driver->visitError(err, Event::getInitializer());
}

void Interpreter::callEndLoop(Function *F, const std::vector<GenericValue> &ArgVals)
{
	ECStack().clear();
}

void Interpreter::callVerifierAssume(Function *F,
				     const std::vector<GenericValue> &ArgVals)
{
	bool cond = ArgVals[0].IntVal.getBoolValue();

	/* TODO: When support for nested functions is added, rewrite this */
	if (!cond)
		getCurThr().block();
}

void Interpreter::callVerifierNondetInt(Function *F,
					const std::vector<GenericValue> &ArgVals)
{
	Thread::MyDist dist(std::numeric_limits<int>::min(),
			    std::numeric_limits<int>::max());

	GenericValue result;
	result.IntVal = APInt(F->getReturnType()->getIntegerBitWidth(),
			      dist(getCurThr().rng));
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callMalloc(Function *F, const std::vector<GenericValue> &ArgVals)
{
	setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		       getAddrPoDeps(getCurThr().id), nullptr);
	GenericValue address = driver->visitMalloc(ArgVals[0].IntVal.getLimitedValue());
	returnValueToCaller(F->getReturnType(), address);
	return;
}

void Interpreter::callFree(Function *F, const std::vector<GenericValue> &ArgVals)
{
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);

	setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		       getAddrPoDeps(getCurThr().id), nullptr);
	driver->visitFree(ptr);
	return;
}

void Interpreter::callPthreadSelf(Function *F,
				  const std::vector<GenericValue> &ArgVals)
{
	llvm::Type *typ = F->getReturnType();

	GenericValue result = driver->visitThreadSelf(typ);
	returnValueToCaller(typ, result);
	return;
}

/* callPthreadCreate - Call to pthread_create() function */
void Interpreter::callPthreadCreate(Function *F,
				    const std::vector<GenericValue> &ArgVals)
{
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]); /* tid */
	Function *calledFun = (Function*) GVTOP(ArgVals[2]);
	ExecutionContext SF;
	GenericValue val, result;

	WARN_ON(ptr == nullptr, "NULL argument to pthread_create()!\n");

	/* First, set up the stack frame for the new function.
	 * Calling function needs to take only one argument ... */
	SF.CurFunction = calledFun;
	SF.CurBB = &calledFun->front();
	SF.CurInst = SF.CurBB->begin();

	SetValue(&*calledFun->arg_begin(), ArgVals[3], SF);

	/* Then, inform the driver about the thread creation */
	setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		       getAddrPoDeps(getCurThr().id), nullptr);
	auto tid = driver->visitThreadCreate(calledFun, SF);

	/* ...and save the TID in the location pointed by the 1st arg */
	Type *typ = static_cast<PointerType *>(F->arg_begin()->getType())->getElementType();
	val.IntVal = APInt(typ->getIntegerBitWidth(), tid);

	driver->visitStore(IA_None, AtomicOrdering::NotAtomic, ptr, typ, val);

	/* Finally, return a value indicating that pthread_create() succeeded */
	result.IntVal = APInt(F->getReturnType()->getIntegerBitWidth(), 0);
	returnValueToCaller(F->getReturnType(), result);
}

/* callPthreadJoin - Call to pthread_join() function */
void Interpreter::callPthreadJoin(Function *F,
				  const std::vector<GenericValue> &ArgVals)
{
	setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		       getAddrPoDeps(getCurThr().id), nullptr);
	auto result = driver->visitThreadJoin(F, ArgVals[0]);
	returnValueToCaller(F->getReturnType(), result);
}

/* callPthreadExit - Call to pthread_exit() function */
void Interpreter::callPthreadExit(Function *F,
				  const std::vector<GenericValue> &ArgVals)
{
	while (ECStack().size() > 1)
		ECStack().pop_back();
	popStackAndReturnValueToCaller(Type::getInt8PtrTy(F->getContext()), ArgVals[0]);
}

void Interpreter::callPthreadMutexInit(Function *F,
				       const std::vector<GenericValue> &ArgVals)
{
	GenericValue *lock = (GenericValue *) GVTOP(ArgVals[0]);
	GenericValue *attr = (GenericValue *) GVTOP(ArgVals[1]);

	if (attr)
		WARN_ONCE("pthread-mutex-init-arg",
			  "Ignoring non-null argument given to pthread_mutex_init.\n");

	if (lock == nullptr) {
		WARN("pthread_mutex_init called with NULL pointer as first argument!");
		abort();
	}

	/* Just return 0 */
	GenericValue result;
	result.IntVal = APInt(F->getReturnType()->getIntegerBitWidth(), 0);
	returnValueToCaller(F->getReturnType(), result);
}

void Interpreter::callPthreadMutexLock(Function *F,
				       const std::vector<GenericValue> &ArgVals)
{
	Thread &thr = getCurThr();
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
	Type *typ = F->getReturnType();
	GenericValue cmpVal, newVal, result;

	if (ptr == nullptr) {
		WARN("pthread_mutex_lock called with NULL pointer!");
		abort();
	}

	cmpVal.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	newVal.IntVal = APInt(typ->getIntegerBitWidth(), 1);

	driver->visitLock(ptr, typ, cmpVal, newVal);

	/*
	 * We need to return a result anyway, because even if the current thread
	 * blocked, it might become unblocked at some point in the future by another
	 * thread
	 */
	result.IntVal = APInt(typ->getIntegerBitWidth(), 0); /* Success */
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callPthreadMutexUnlock(Function *F,
					 const std::vector<GenericValue> &ArgVals)
{
	Thread &thr = getCurThr();
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
	Type *typ = F->getReturnType();
	GenericValue val, result;

	if (ptr == nullptr) {
		WARN("ERROR: pthread_mutex_lock called with NULL pointer!");
		abort();
	}

	val.IntVal = APInt(typ->getIntegerBitWidth(), 0);

	setCurrentDeps(nullptr, nullptr, getCtrlDeps(thr.id),
		       getAddrPoDeps(thr.id), nullptr);

	driver->visitUnlock(ptr, typ, val);

	result.IntVal = APInt(typ->getIntegerBitWidth(), 0); /* Success */
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callPthreadMutexTrylock(Function *F,
					 const std::vector<GenericValue> &ArgVals)
{
	Thread &thr = getCurThr();
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
	Type *typ = F->getReturnType();
	GenericValue cmpVal, newVal, result;

	if (ptr == nullptr) {
		WARN("ERROR: pthread_mutex_lock called with NULL pointer!");
		abort();
	}

	cmpVal.IntVal = APInt(typ->getIntegerBitWidth(), 0);
	newVal.IntVal = APInt(typ->getIntegerBitWidth(), 1);

	setCurrentDeps(nullptr, nullptr, getCtrlDeps(thr.id),
		       getAddrPoDeps(thr.id), nullptr);

	auto ret = driver->visitLoad(IA_Cas, AtomicOrdering::Acquire, ptr,
				     typ, cmpVal, newVal);

	auto cmpRes = executeICMP_EQ(ret, cmpVal, typ);
	if (cmpRes.IntVal.getBoolValue()) {
		setCurrentDeps(nullptr, nullptr, getCtrlDeps(thr.id),
			       getAddrPoDeps(thr.id), nullptr);

		driver->visitStore(IA_Cas, AtomicOrdering::Acquire,
				   ptr, typ, newVal);
	}

	result.IntVal = APInt(typ->getIntegerBitWidth(), !cmpRes.IntVal.getBoolValue());
	returnValueToCaller(F->getReturnType(), result);
	return;
}

void Interpreter::callReadFunction(const Library &lib, const LibMem &mem, Function *F,
				   const std::vector<GenericValue> &ArgVals)
{
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
	Type *typ = F->getReturnType();

	setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		       getAddrPoDeps(getCurThr().id), nullptr);
	auto res = driver->visitLibLoad(IA_None, mem.getOrdering(), ptr,
					typ, F->getName().str());
	auto &val = res.first;
	auto shouldBlock = res.second;

	if (shouldBlock) {
		getCurThr().block();
		return;
	}
	returnValueToCaller(F->getReturnType(), val);
	return;
}

void Interpreter::callWriteFunction(const Library &lib, const LibMem &mem, Function *F,
				    const std::vector<GenericValue> &ArgVals)
{
	GenericValue *ptr = (GenericValue *) GVTOP(ArgVals[0]);
	GenericValue val = ArgVals[1];
	Type *typ = F->getFunctionType()->getParamType(1);
	GenericValue result;

	if (ptr == nullptr) {
		WARN("ERROR: " + F->getName().str() + " called with NULL pointer!");
		abort();
	}

	setCurrentDeps(nullptr, nullptr, getCtrlDeps(getCurThr().id),
		       getAddrPoDeps(getCurThr().id), nullptr);
	driver->visitLibStore(IA_None, mem.getOrdering(), ptr, typ, val,
			      F->getName().str(), mem.isLibInit());
	return;
}

//===----------------------------------------------------------------------===//
// callFunction - Execute the specified function...
//
void Interpreter::callFunction(Function *F,
                               const std::vector<GenericValue> &ArgVals)
{
  /* Custom function support */
  std::string functionName = F->getName().str();

  auto granted = driver->getGrantedLibs();
  auto lib = Library::getLibByMemberName(granted, functionName);
  if (lib) {
	  auto m = lib->getMember(functionName);
	  BUG_ON(!m);
	  if (m->hasReadSemantics()) {
		  callReadFunction(*lib, *m, F, ArgVals);
		  return;
	  } else {
		  callWriteFunction(*lib, *m, F, ArgVals);
		  return;
	  }
  }

  if (functionName == "__assert_fail") {
	  callAssertFail(F, ArgVals);
	  return;
  } else if (functionName == "__end_loop") {
	  callEndLoop(F, ArgVals);
	  return;
  } else if (functionName == "__VERIFIER_assume") {
	  callVerifierAssume(F, ArgVals);
	  return;
  } else if (functionName == "__VERIFIER_nondet_int") {
	  callVerifierNondetInt(F, ArgVals);
	  return;
  } else if (functionName == "malloc") {
	  callMalloc(F, ArgVals);
	  return;
  } else if (functionName == "free") {
	  callFree(F, ArgVals);
	  return;
  } else if (functionName == "pthread_self") {
	  callPthreadSelf(F, ArgVals);
	  return;
  } else if (functionName == "pthread_create") {
	  callPthreadCreate(F, ArgVals);
	  return;
  } else if (functionName == "pthread_join" ||
	     functionName == "_pthread_join") { /* Mac OS X name mangling */
	  callPthreadJoin(F, ArgVals);
	  return;
  } else if (functionName == "pthread_exit") {
	  callPthreadExit(F, ArgVals);
	  return;
  } else if (functionName == "pthread_mutex_init") {
	  callPthreadMutexInit(F, ArgVals);
	  return;
  } else if (functionName == "pthread_mutex_lock") {
	  callPthreadMutexLock(F, ArgVals);
	  return;
  } else if (functionName == "pthread_mutex_unlock") {
	  callPthreadMutexUnlock(F, ArgVals);
	  return;
  } else if (functionName == "pthread_mutex_trylock") {
	  callPthreadMutexTrylock(F, ArgVals);
	  return;
  }

  assert((ECStack().empty() || !ECStack().back().Caller.getInstruction() ||
	  ECStack().back().Caller.arg_size() == ArgVals.size()) &&
	 "Incorrect number of arguments passed into function call!");
  // Make a new stack frame... and fill it in.
  ECStack().push_back(ExecutionContext());

  ExecutionContext &StackFrame = ECStack().back();
  StackFrame.CurFunction = F;

  // Special handling for external functions.
  if (F->isDeclaration()) {
    GenericValue Result = callExternalFunction (F, ArgVals);
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

void Interpreter::run()
{
	mainECStack = ECStack();

	driver->handleExecutionBeginning();
	while (driver->scheduleNext()) {
		driver->handleExecutionInProgress();
		llvm::ExecutionContext &SF = ECStack().back();
		llvm::Instruction &I = *SF.CurInst++;
		visit(I);
	}
	driver->handleFinishedExecution();
	return;
}
