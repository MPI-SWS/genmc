// For the parts of the code originating from LLVM-3.5:
//===-- Interpreter.h ------------------------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LLVMLICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This header file defines the interpreter structure
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


#include "config.h"

#ifndef LLI_INTERPRETER_H
#define LLI_INTERPRETER_H

#include "Library.hpp"
#include "View.hpp"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "llvm/IR/CallSite.h"
#include <llvm/IR/DebugInfo.h>
#if defined(HAVE_LLVM_IR_DATALAYOUT_H)
#include <llvm/IR/DataLayout.h>
#elif defined(HAVE_LLVM_DATALAYOUT_H)
#include <llvm/DataLayout.h>
#endif
#if defined(HAVE_LLVM_IR_FUNCTION_H)
#include <llvm/IR/Function.h>
#elif defined(HAVE_LLVM_FUNCTION_H)
#include <llvm/Function.h>
#endif
#if defined(HAVE_LLVM_INSTVISITOR_H)
#include <llvm/InstVisitor.h>
#elif defined(HAVE_LLVM_IR_INSTVISITOR_H)
#include <llvm/IR/InstVisitor.h>
#elif defined(HAVE_LLVM_SUPPORT_INSTVISITOR_H)
#include <llvm/Support/InstVisitor.h>
#endif
#include <llvm/Support/DataTypes.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

#include <random>
#include <unordered_map>

class GenMCDriver;

namespace llvm {

class IntrinsicLowering;
struct FunctionInfo;
template<typename T> class generic_gep_type_iterator;
class ConstantExpr;
typedef generic_gep_type_iterator<User::const_op_iterator> gep_type_iterator;


typedef std::vector<GenericValue> ValuePlaneTy;

// ExecutionContext struct - This struct represents one stack frame currently
// executing.
//
struct ExecutionContext {
  Function             *CurFunction;// The currently executing function
  BasicBlock           *CurBB;      // The currently executing BB
  BasicBlock::iterator  CurInst;    // The next instruction to execute
  CallSite             Caller;     // Holds the call that called subframes.
                                   // NULL if main func or debugger invoked fn
  std::map<Value *, GenericValue> Values; // LLVM values used in this invocation
  std::vector<GenericValue>  VarArgs; // Values passed through an ellipsis
};

class Thread {

public:
	using MyRNG  = std::minstd_rand;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;
	static constexpr int seed = 1821;

	int id;
	int parentId;
	llvm::Function *threadFun;
	std::vector<llvm::ExecutionContext> ECStack;
	llvm::ExecutionContext initSF;
	std::unordered_map<void *, llvm::GenericValue> tls;
	unsigned int globalInstructions;
	bool isBlocked;
	MyRNG rng;
	std::vector<std::pair<int, std::string> > prefixLOC;

	void block() { isBlocked = true; };
	void unblock() { isBlocked = false; };

	Thread(llvm::Function *F, int id)
		: id(id), parentId(-1), threadFun(F), globalInstructions(0),
		  isBlocked(false), rng(seed) {}

	Thread(llvm::Function *F, int id, int pid, const llvm::ExecutionContext &SF)
		: id(id), parentId(pid), threadFun(F), initSF(SF), globalInstructions(0),
		  isBlocked(false), rng(seed) {}
};

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const Thread &thr);

// Interpreter - This class represents the entirety of the interpreter.
//
class Interpreter : public ExecutionEngine, public InstVisitor<Interpreter> {
  GenericValue ExitValue;          // The return value of the called function
  DataLayout TD;
  IntrinsicLowering *IL;

  /* Composition pointers */
  GenMCDriver *driver;

  // The runtime stack of executing code.  The top of the stack is the current
  // function record.
  std::vector<ExecutionContext> mainECStack;

  // AtExitHandlers - List of functions to call when the program exits,
  // registered with the atexit() library function.
  std::vector<Function*> AtExitHandlers;

public:
  explicit Interpreter(Module *M, GenMCDriver *driver);
  virtual ~Interpreter();

  /* Enum to inform the driver about possible special attributes
   * of the instruction being interpreted */
  enum InstAttr {
	  IA_None,
	  IA_Fai,
	  IA_Cas,
	  IA_Lock,
	  IA_Unlock,
  };

  /* Resets the interpreter at the beginning of a new execution */
  void reset();

  /* Stores the addresses of global values into globalVars and threadLocalVars */
  void storeGlobals(Module *M);

  std::vector<ExecutionContext> &ECStack();

  /* Checks whether an address is the address of a global variable */
  bool isGlobal(const void *);
  bool isHeapAlloca(const void *);
  bool isStackAlloca(const void *);
  std::string getGlobalName(const void *addr);
  void freeRegion(const void *addr, int size);

  /// runAtExitHandlers - Run any functions registered by the program's calls to
  /// atexit(3), which we intercept and store in AtExitHandlers.
  ///
  void runAtExitHandlers();

  /// create - Create an interpreter ExecutionEngine. This can never fail.
  ///
  static ExecutionEngine *create(Module *M, GenMCDriver *driver,
				 std::string *ErrorStr = nullptr);

  /// run - Start execution with the specified function and arguments.
  ///
#ifdef LLVM_EXECUTION_ENGINE_RUN_FUNCTION_VECTOR
  virtual GenericValue runFunction(Function *F,
				   const std::vector<GenericValue> &ArgValues);
#else
  virtual GenericValue runFunction(Function *F,
				   llvm::ArrayRef<GenericValue> ArgValues);
#endif

  void *getPointerToNamedFunction(const std::string &Name,
                                  bool AbortOnFailure = true) {
    // FIXME: not implemented.
    return nullptr;
  }

  void *getPointerToNamedFunction(llvm::StringRef Name,
                                  bool AbortOnFailure = true) {
    // FIXME: not implemented.
    return nullptr;
  };

  /// recompileAndRelinkFunction - For the interpreter, functions are always
  /// up-to-date.
  ///
  void *recompileAndRelinkFunction(Function *F) {
    return getPointerToFunction(F);
  }

  /// freeMachineCodeForFunction - The interpreter does not generate any code.
  ///
  void freeMachineCodeForFunction(Function *F) { }

  std::vector<Thread> threads;
  int currentThread = 0;

  Thread& getCurThr() { return threads[currentThread]; };
  Thread& getThrById(int id) { return threads[id]; };

  /* List of global and thread-local variables */
  llvm::SmallVector<void *, 1021> globalVars;
  std::vector<std::pair<void *, std::string > > globalVarNames;
  std::unordered_map<void *, llvm::GenericValue> threadLocalVars;

  std::vector<void *> stackAllocas;
  std::vector<void *> heapAllocas;
  std::vector<void *> stackMem;
  std::vector<void *> freedMem;

  /* Helper functions */
#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
  void collectGVNames(Module *M, char *ptr, Type *typ,
		      DIType *md, std::string nameBuilder);
#else
  void collectGVNames(const GlobalVariable &v, char *ptr, unsigned int typeSize);
#endif
  void replayExecutionBefore(const View &before);
  bool compareValues(const llvm::Type *typ, const GenericValue &val1, const GenericValue &val2);
  GenericValue getLocInitVal(GenericValue *ptr, Type *typ);
  void executeAtomicRMWOperation(GenericValue &result, const GenericValue &oldVal,
				 const GenericValue &val, AtomicRMWInst::BinOp op);

  /* Custom Opcode Implementations */ // TODO: Remove call* from the class?
  void callAssertFail(Function *F, const std::vector<GenericValue> &ArgVals);
  void callEndLoop(Function *F, const std::vector<GenericValue> &ArgVals);
  void callVerifierAssume(Function *F, const std::vector<GenericValue> &ArgVals);
  void callVerifierNondetInt(Function *F, const std::vector<GenericValue> &ArgVals);
  void callMalloc(Function *F, const std::vector<GenericValue> &ArgVals);
  void callFree(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadSelf(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadCreate(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadJoin(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadExit(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadMutexInit(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadMutexLock(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadMutexUnlock(Function *F, const std::vector<GenericValue> &ArgVals);
  void callPthreadMutexTrylock(Function *F, const std::vector<GenericValue> &ArgVals);
  void callReadFunction(const Library &lib, const LibMem &m, Function *F,
			const std::vector<GenericValue> &ArgVals);
  void callWriteFunction(const Library &lib, const LibMem &m, Function *F,
			 const std::vector<GenericValue> &ArgVals);


  // Methods used to execute code:
  // Place a call on the stack
  void callFunction(Function *F, const std::vector<GenericValue> &ArgVals);
  void run();                // Execute instructions until nothing left to do

  // Opcode Implementations
  void visitReturnInst(ReturnInst &I);
  void visitBranchInst(BranchInst &I);
  void visitSwitchInst(SwitchInst &I);
  void visitIndirectBrInst(IndirectBrInst &I);

  void visitBinaryOperator(BinaryOperator &I);
  void visitICmpInst(ICmpInst &I);
  void visitFCmpInst(FCmpInst &I);
  void visitAllocaInst(AllocaInst &I);
  void visitLoadInst(LoadInst &I);
  void visitStoreInst(StoreInst &I);
  void visitGetElementPtrInst(GetElementPtrInst &I);
  void visitPHINode(PHINode &PN) {
    llvm_unreachable("PHI nodes already handled!");
  }
  void visitTruncInst(TruncInst &I);
  void visitZExtInst(ZExtInst &I);
  void visitSExtInst(SExtInst &I);
  void visitFPTruncInst(FPTruncInst &I);
  void visitFPExtInst(FPExtInst &I);
  void visitUIToFPInst(UIToFPInst &I);
  void visitSIToFPInst(SIToFPInst &I);
  void visitFPToUIInst(FPToUIInst &I);
  void visitFPToSIInst(FPToSIInst &I);
  void visitPtrToIntInst(PtrToIntInst &I);
  void visitIntToPtrInst(IntToPtrInst &I);
  void visitBitCastInst(BitCastInst &I);
  void visitSelectInst(SelectInst &I);


  void visitCallSite(CallSite CS);
  void visitCallInst(CallInst &I) { visitCallSite (CallSite (&I)); }
  void visitInvokeInst(InvokeInst &I) { visitCallSite (CallSite (&I)); }
  void visitUnreachableInst(UnreachableInst &I);

  void visitShl(BinaryOperator &I);
  void visitLShr(BinaryOperator &I);
  void visitAShr(BinaryOperator &I);

  void visitVAArgInst(VAArgInst &I);
  void visitExtractElementInst(ExtractElementInst &I);
  void visitInsertElementInst(InsertElementInst &I);
  void visitShuffleVectorInst(ShuffleVectorInst &I);

  void visitExtractValueInst(ExtractValueInst &I);
  void visitInsertValueInst(InsertValueInst &I);

  void visitAtomicCmpXchgInst(AtomicCmpXchgInst &I);
  void visitAtomicRMWInst(AtomicRMWInst &I);
  void visitFenceInst(FenceInst &I);

  bool isInlineAsm(CallSite &CS, std::string *asmStr);
  void visitInlineAsm(CallSite &CS, const std::string &asmString);

  void visitInstruction(Instruction &I) {
    errs() << I << "\n";
    llvm_unreachable("Instruction not interpretable yet!");
  }

  GenericValue callExternalFunction(Function *F,
                                    const std::vector<GenericValue> &ArgVals);
  void exitCalled(GenericValue GV);

  void addAtExitHandler(Function *F) {
    AtExitHandlers.push_back(F);
  }

  GenericValue *getFirstVarArg () {
    return &(ECStack().back ().VarArgs[0]);
  }

private:  // Helper functions
  GenericValue executeGEPOperation(Value *Ptr, gep_type_iterator I,
                                   gep_type_iterator E, ExecutionContext &SF);

  // SwitchToNewBasicBlock - Start execution in a new basic block and run any
  // PHI nodes in the top of the block.  This is used for intraprocedural
  // control flow.
  //
  void SwitchToNewBasicBlock(BasicBlock *Dest, ExecutionContext &SF);

  void *getPointerToFunction(Function *F) { return (void*)F; }
  void *getPointerToBasicBlock(BasicBlock *BB) { return (void*)BB; }

  void initializeExecutionEngine() { }
  void initializeExternalFunctions();
  GenericValue getConstantExprValue(ConstantExpr *CE, ExecutionContext &SF);
  GenericValue getOperandValue(Value *V, ExecutionContext &SF);
  GenericValue executeTruncInst(Value *SrcVal, Type *DstTy,
                                ExecutionContext &SF);
  GenericValue executeSExtInst(Value *SrcVal, Type *DstTy,
                               ExecutionContext &SF);
  GenericValue executeZExtInst(Value *SrcVal, Type *DstTy,
                               ExecutionContext &SF);
  GenericValue executeFPTruncInst(Value *SrcVal, Type *DstTy,
                                  ExecutionContext &SF);
  GenericValue executeFPExtInst(Value *SrcVal, Type *DstTy,
                                ExecutionContext &SF);
  GenericValue executeFPToUIInst(Value *SrcVal, Type *DstTy,
                                 ExecutionContext &SF);
  GenericValue executeFPToSIInst(Value *SrcVal, Type *DstTy,
                                 ExecutionContext &SF);
  GenericValue executeUIToFPInst(Value *SrcVal, Type *DstTy,
                                 ExecutionContext &SF);
  GenericValue executeSIToFPInst(Value *SrcVal, Type *DstTy,
                                 ExecutionContext &SF);
  GenericValue executePtrToIntInst(Value *SrcVal, Type *DstTy,
                                   ExecutionContext &SF);
  GenericValue executeIntToPtrInst(Value *SrcVal, Type *DstTy,
                                   ExecutionContext &SF);
  GenericValue executeBitCastInst(Value *SrcVal, Type *DstTy,
                                  ExecutionContext &SF);
  GenericValue executeCastOperation(Instruction::CastOps opcode, Value *SrcVal,
                                    Type *Ty, ExecutionContext &SF);
  void returnValueToCaller(Type *RetTy, GenericValue Result);
  void popStackAndReturnValueToCaller(Type *RetTy, GenericValue Result);

};

} // End llvm namespace

#endif
