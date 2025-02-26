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

#include "ADT/View.hpp"
#include "ADT/value_ptr.hpp"
#include "Config/Config.hpp"
#include "Runtime/DepTracker.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "Static/LLVMUtils.hpp"
#include "Static/ModuleInfo.hpp"
#include "Support/MemAccess.hpp"
#include "Support/SAddr.hpp"
#include "Support/SAddrAllocator.hpp"
#include "Support/SVal.hpp"
#include "Support/ThreadInfo.hpp"
#include "Verification/VerificationError.hpp"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/DataTypes.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>

/* Some helpers for GenericValues */
#define INT_TO_GV(typ, val)                                                                        \
	({                                                                                         \
		llvm::GenericValue __ret;                                                          \
		__ret.IntVal = llvm::APInt((typ)->getIntegerBitWidth(), (val), true);              \
		__ret;                                                                             \
	})

#define PTR_TO_GV(ptr)                                                                             \
	({                                                                                         \
		llvm::GenericValue __ret;                                                          \
		__ret.PointerVal = (void *)(ptr);                                                  \
		__ret;                                                                             \
	})

#define GET_ZERO_GV(typ)                                                                           \
	({                                                                                         \
		llvm::GenericValue __ret;                                                          \
		if (typ->isPointerTy())                                                            \
			__ret = PTR_TO_GV(nullptr);                                                \
		else                                                                               \
			__ret = INT_TO_GV(typ, 0);                                                 \
		__ret;                                                                             \
	})

class GenMCDriver;

#define INT_TO_GV(typ, val)                                                                        \
	({                                                                                         \
		llvm::GenericValue __ret;                                                          \
		__ret.IntVal = llvm::APInt((typ)->getIntegerBitWidth(), (val), true);              \
		__ret;                                                                             \
	})

#define PTR_TO_GV(ptr)                                                                             \
	({                                                                                         \
		llvm::GenericValue __ret;                                                          \
		__ret.PointerVal = (void *)(ptr);                                                  \
		__ret;                                                                             \
	})

namespace llvm {

class IntrinsicLowering;
struct FunctionInfo;
template <typename T> class generic_gep_type_iterator;
class ConstantExpr;
typedef generic_gep_type_iterator<User::const_op_iterator> gep_type_iterator;

typedef std::vector<GenericValue> ValuePlaneTy;

// AllocaHolder - Object to track all of the blocks of memory allocated by
// allocas in a particular stack frame. Since the driver needs to be made
// aware of the deallocs, special care needs to be taken to inform the driver
// when stack frames are popped
//
class AllocaHolder {
	std::vector<void *> allocas;

public:
	using Allocas = std::vector<void *>;

	AllocaHolder() {}

	void add(void *mem) { allocas.push_back(mem); }
	const Allocas &get() const { return allocas; }
};

// ExecutionContext struct - This struct represents one stack frame currently
// executing.
//
struct ExecutionContext {
	Function *CurFunction;			// The currently executing function
	BasicBlock *CurBB;			// The currently executing BB
	BasicBlock::iterator CurInst;		// The next instruction to execute
	CallInstWrapper Caller;			// Holds the call that called subframes.
						// NULL if main func or debugger invoked fn
	std::map<Value *, GenericValue> Values; // LLVM values used in this invocation
	std::vector<GenericValue> VarArgs;	// Values passed through an ellipsis
	AllocaHolder Allocas;			// Track memory allocated by alloca

	ExecutionContext() : CurFunction(nullptr), CurBB(nullptr), CurInst(nullptr) {}
};

/* The different reasons a thread might block */
enum class BlockageType {
	NotBlocked,
	ThreadJoin,
	Spinloop,
	FaiZNESpinloop,
	LockZNESpinloop,
	HelpedCas,
	Confirmation,
	ReadOptBlock,
	LockNotAcq,
	LockNotRel,
	Barrier,
	Cons,
	Error,
	User,
};

/*
 * Thread class -- Contains information specific to each thread.
 */
class Thread {

public:
	using MyRNG = std::minstd_rand;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;
	static constexpr int seed = 1995;

	int id;
	int parentId;
	llvm::Function *threadFun;
	SVal threadArg;
	std::vector<llvm::ExecutionContext> ECStack;
	std::vector<llvm::ExecutionContext> initEC;
	std::unordered_map<const void *, llvm::GenericValue> tls;
	unsigned int globalInstructions;
	unsigned int globalInstSnap;
	BasicBlock::iterator curInstSnap;
	BlockageType blocked;
	MyRNG rng;
	std::vector<std::pair<int, std::string>> prefixLOC;

	bool isMain() const { return id == 0; }

	void block(BlockageType t) { blocked = t; }
	void unblock() { blocked = BlockageType::NotBlocked; }
	bool isBlocked() const { return blocked != BlockageType::NotBlocked; }
	BlockageType getBlockageType() const { return blocked; }

	/* Useful for one-to-many instr->events correspondence */
	void takeSnapshot()
	{
		globalInstSnap = globalInstructions;
		curInstSnap = --ECStack.back().CurInst;
		++ECStack.back().CurInst;
	}
	void rollToSnapshot()
	{
		globalInstructions = globalInstSnap;
		ECStack.back().CurInst = curInstSnap;
	}

protected:
	friend class Interpreter;

	Thread(llvm::Function *F, int id)
		: id(id), parentId(-1), threadFun(F), initEC(), globalInstructions(0),
		  blocked(BlockageType::NotBlocked), rng(seed)
	{}

	Thread(llvm::Function *F, SVal arg, int id, int pid, const llvm::ExecutionContext &SF)
		: id(id), parentId(pid), threadFun(F), threadArg(arg), initEC({SF}),
		  globalInstructions(0), blocked(BlockageType::NotBlocked), rng(seed)
	{}
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const Thread &thr);

/* Pers: The state of the program -- i.e., part of the program being interpreted */
enum class ProgramState { Ctors, Main, Dtors, Recovery };

/* The state of the current execution */
enum class ExecutionState { Normal, Replay };

struct DynamicComponents {

	/* Information about threads as well as the currently executing thread */
	std::vector<Thread> threads;
	int currentThread = 0;

	/* Pointer to the dependency tracker */
	value_ptr<DepTracker, DepTrackerCloner> depTracker = nullptr;

	/* Information about the interpreter's state */
	ExecutionState execState = ExecutionState::Normal;
	ProgramState programState = ProgramState::Main; /* Pers */

	/* Pers: A map from file descriptors to file descriptions */
	llvm::IndexedMap<void *> fdToFile;

	/* Pers: Maps a filename to the address of the contents of the directory's inode for
	 * said name (the contents should have the address of the file's inode) */
	std::unordered_map<std::string, void *> nameToInodeAddr;

	GenericValue ExitValue; // The return value of the called function

	// AtExitHandlers - List of functions to call when the program exits,
	// registered with the atexit() library function.
	std::vector<Function *> AtExitHandlers;
};

using InterpreterState = DynamicComponents;

// Interpreter - This class represents the entirety of the interpreter.
//
class Interpreter : public ExecutionEngine, public InstVisitor<Interpreter> {

public:
	using AnnotID = ModuleID::ID;
	using AnnotT = SExpr<AnnotID>;

protected:
	/*** Static components (once set, do not change) ***/

	/* Information about the module under test */
	std::unique_ptr<ModuleInfo> MI;

	/* List of thread-local variables, with their initializing values */
	std::unordered_map<const void *, llvm::GenericValue> threadLocalVars;

	/* Mapping between static allocation beginning and the actual addresses of
	 * global variables (where their contents are stored) */
	std::unordered_map<SAddr, void *> staticValueMap;

	/* Keep all static ranges that have been allocated */
	VSet<std::pair<SAddr, SAddr>> staticAllocas;

	/* Maintain the relationship between SAddr and global variables,
	 * so that we can get naming information */
	std::unordered_map<SAddr, Value *> staticNames;

	/* (Composition) pointer to the driver */
	GenMCDriver *driver;

	Function *mainFun = nullptr;

	/* Whether the driver should be called on system errors */
	bool stopOnSystemErrors;

	/* Where system errors return values should be stored (if required) */
	SAddr errnoAddr;
	Type *errnoTyp;

	/* Pers: The recovery routine to run */
	Function *recoveryRoutine = nullptr;

	/* This is not exactly static but is reset to the same value each time*/
	std::vector<ExecutionContext> mainECStack;

	IntrinsicLowering *IL;

	/*** Dynamic components (change during verification) ***/

	DynamicComponents dynState;

public:
	explicit Interpreter(std::unique_ptr<Module> M, std::unique_ptr<ModuleInfo> MI,
			     GenMCDriver *driver, const Config *userConf, SAddrAllocator &alloctor);
	virtual ~Interpreter();

	std::unique_ptr<InterpreterState> saveState();
	void restoreState(std::unique_ptr<InterpreterState>);

	Thread &constructAddThreadFromInfo(const ThreadInfo &ti)
	{
		auto *calledFun =
			dyn_cast<Function>(const_cast<Value *>(MI->idInfo.IDV.at(ti.funId)));
		BUG_ON(!calledFun);
		ExecutionContext SF;

		SF.CurFunction = calledFun;
		SF.CurBB = &calledFun->front();
		SF.CurInst = SF.CurBB->begin();

		SF.Values[&*calledFun->arg_begin()] = PTR_TO_GV(ti.arg.get());
		return createAddNewThread(calledFun, ti.arg, ti.id, ti.parentId, SF);
	}
	void setExecutionContext(const std::vector<ThreadInfo> &tis)
	{
		dynState.threads.clear();
		createAddMainThread();
		for (auto &ti : tis)
			constructAddThreadFromInfo(ti);
	}

#define CALL_DRIVER(method, ...)                                                                   \
	({                                                                                         \
		if (getProgramState() != ProgramState::Recovery ||                                 \
		    strcmp(#method, "handleLoad")) {                                               \
			incPos();                                                                  \
		}                                                                                  \
		driver->method(__VA_ARGS__);                                                       \
	})

/* When replaying, we don't decrease the pos so we don't get into an
 * infinite loop, e.g., if the last instruction reads bottom.
 * (Besides, the amount of replaying should be well-defined.)
 */
#define CALL_DRIVER_RESET_IF_NONE(method, ...)                                                     \
	({                                                                                         \
		incPos();                                                                          \
		auto ret = driver->method(__VA_ARGS__);                                            \
		if (!ret.has_value() && getExecState() != ExecutionState::Replay) {                \
			decPos();                                                                  \
			--ECStack().back().CurInst;                                                \
		} else if (getProgramState() == ProgramState::Recovery &&                          \
			   (strcmp(#method, "handleLoad") == 0)) {                                 \
			decPos();                                                                  \
		}                                                                                  \
		ret;                                                                               \
	})

#define CALL_DRIVER_RESET_IF_FALSE(method, ...)                                                    \
	({                                                                                         \
		incPos();                                                                          \
		auto ret = driver->method(__VA_ARGS__);                                            \
		if (!ret) {                                                                        \
			decPos();                                                                  \
			--ECStack().back().CurInst;                                                \
		}                                                                                  \
		ret;                                                                               \
	})

	/* Blocks the current execution */
	void block(BlockageType t = BlockageType::Error)
	{
		std::for_each(threads_begin(), threads_end(), [&](Thread &thr) { thr.block(t); });
	}

	/* Resets all thread info to their initial values */
	void resetThread(unsigned int id);

	/* Resets the interpreter at the beginning of a new execution */
	void reset();

	/* Pers: Setups the execution context for the recovery routine
	 * in thread TID. Assumes that the thread has already been added
	 * to the thread list. */
	void setupRecoveryRoutine(int tid);

	/* Pers: Does cleanups after the recovery routine has run */
	void cleanupRecoveryRoutine(int tid);

	/* Creates a new thread and adds it to the thread list */
	Thread &createAddNewThread(llvm::Function *F, SVal arg, int tid, int pid,
				   const llvm::ExecutionContext &SF);

	/* Pers: Creates a thread for the recovery routine and adds it to
	 * the thread list */
	Thread &createAddRecoveryThread(int tid);

	/* Sets-up the specified thread for execution */
	void scheduleThread(int tid) { dynState.currentThread = tid; }

	/* Returns the currently executing thread */
	Thread &getCurThr() { return dynState.threads[dynState.currentThread]; }
	const Thread &getCurThr() const { return dynState.threads.at(dynState.currentThread); }

	/* Returns the thread with the specified ID (taken from the graph) */
	Thread &getThrById(int id) { return dynState.threads[id]; };
	const Thread &getThrById(int id) const { return dynState.threads[id]; };

	unsigned int getNumThreads() const { return dynState.threads.size(); }

	using thread_iterator = std::vector<Thread>::iterator;
	using const_thread_iterator = std::vector<Thread>::const_iterator;
	using thread_range = iterator_range<thread_iterator>;
	using const_thread_range = iterator_range<const_thread_iterator>;

	const_thread_iterator threads_begin() const { return dynState.threads.begin(); }
	const_thread_iterator threads_end() const { return dynState.threads.end(); }
	thread_iterator threads_begin() { return dynState.threads.begin(); }
	thread_iterator threads_end() { return dynState.threads.end(); }
	const_thread_range threads() const
	{
		return const_thread_range(threads_begin(), threads_end());
	}
	thread_range threads() { return thread_range(threads_begin(), threads_end()); }

	/* Returns the stack frame of the currently executing thread */
	std::vector<ExecutionContext> &ECStack() { return getCurThr().ECStack; }

	/* Returns the current (global) position (thread, index) interpreted */
	Event currPos() const { return Event(getCurThr().id, getCurThr().globalInstructions); };
	Event nextPos() const { return currPos().next(); };
	Event incPos()
	{
		auto &thr = getCurThr();
		return Event(thr.id, ++thr.globalInstructions);
	};
	Event decPos()
	{
		auto &thr = getCurThr();
		return Event(thr.id, --thr.globalInstructions);
	};

	/* Query interpreter's state */
	ProgramState getProgramState() const { return dynState.programState; }
	ExecutionState getExecState() const { return dynState.execState; }

	/* Annotation information */

	/* Returns annotation information for the instruction I */
	const AnnotT *getAnnotation(Instruction *I) const
	{
		auto id = MI->idInfo.VID[I];
		return MI->annotInfo.annotMap.count(id) ? MI->annotInfo.annotMap.at(id).get()
							: nullptr;
	}

	/* Returns (concretized) annotation information for the
	 * current instruction (assuming we're executing it) */
	std::unique_ptr<AnnotT> getCurrentAnnotConcretized();

	/* Memory pools checks */

	/* Returns true if the interpreter has allocated space for the specified static */
	bool isStaticallyAllocated(SAddr addr) const;
	void *getStaticAddr(SAddr addr) const;
	std::string getStaticName(SAddr addr) const;

	/// runAtExitHandlers - Run any functions registered by the program's calls to
	/// atexit(3), which we intercept and store in AtExitHandlers.
	///
	void runAtExitHandlers();

	/// create - Create an interpreter ExecutionEngine. This can never fail.
	///
	static std::unique_ptr<Interpreter>
	create(std::unique_ptr<Module> M, std::unique_ptr<ModuleInfo> MI, GenMCDriver *driver,
	       const Config *userConf, SAddrAllocator &alloctor, std::string *ErrorStr = nullptr);

	/// run - Start execution with the specified function and arguments.
	///
#ifdef LLVM_EXECUTION_ENGINE_RUN_FUNCTION_VECTOR
	virtual GenericValue runFunction(Function *F, const std::vector<GenericValue> &ArgValues);
#else
	virtual GenericValue runFunction(Function *F, llvm::ArrayRef<GenericValue> ArgValues);
#endif

	void *getPointerToNamedFunction(const std::string &Name, bool AbortOnFailure = true)
	{
		// FIXME: not implemented.
		return nullptr;
	}

	void *getPointerToNamedFunction(llvm::StringRef Name, bool AbortOnFailure = true)
	{
		// FIXME: not implemented.
		return nullptr;
	};

	/// recompileAndRelinkFunction - For the interpreter, functions are always
	/// up-to-date.
	///
	void *recompileAndRelinkFunction(Function *F) { return getPointerToFunction(F); }

	/// freeMachineCodeForFunction - The interpreter does not generate any code.
	///
	void freeMachineCodeForFunction(Function *F) {}

	/* Helper functions */
	void replayExecutionBefore(const VectorClock &before);

	SVal getLocInitVal(const AAccess &access)
	{
		GenericValue result;

		LoadValueFromMemory(
			result, (llvm::GenericValue *)getStaticAddr(access.getAddr()),
			IntegerType::get(Modules.back()->getContext(), access.getSize().get() * 8));
		return SVal(result.IntVal.getLimitedValue());
	}

	unsigned int getTypeSize(Type *typ) const;
	SVal executeAtomicRMWOperation(SVal oldVal, SVal val, ASize size, AtomicRMWInst::BinOp op);

	// Methods used to execute code:
	// Place a call on the stack
	void callFunction(Function *F, const std::vector<GenericValue> &ArgVals,
			  const std::unique_ptr<EventDeps> &specialDeps);

	/* callFunction() wrappers to be called before running a function */
	void setupFunctionCall(Function *F, ArrayRef<GenericValue> ArgValues);
	void setupStaticCtorsDtors(Module &M, bool isDtors);
	void setupStaticCtorsDtors(bool isDtors);
	void setupMain(Function *Fn, const std::vector<std::string> &argv, const char *const *envp);

	void run(); // Execute instructions until nothing left to do

	/* run() wrappers */
	int runAsMain(const std::string &main);
	void runRecovery();

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
	void visitPHINode(PHINode &PN) { llvm_unreachable("PHI nodes already handled!"); }
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

	void visitCallInstWrapper(CallInstWrapper CIW);
#if LLVM_VERSION_MAJOR < 11
	void visitCallSite(CallSite CS) { visitCallInstWrapper(CallInstWrapper(CS)); }
#else
	void visitCallBase(CallBase &CB) { visitCallInstWrapper(CallInstWrapper(CB)); }
#endif
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

	bool isInlineAsm(CallInstWrapper CIW, std::string *asmStr);
	void visitInlineAsm(CallInstWrapper CIW, const std::string &asmString);

	void visitInstruction(Instruction &I)
	{
		errs() << I << "\n";
		llvm_unreachable("Instruction not interpretable yet!");
	}

	GenericValue callExternalFunction(Function *F, const std::vector<GenericValue> &ArgVals);
	void exitCalled(GenericValue GV);

	void addAtExitHandler(Function *F) { dynState.AtExitHandlers.push_back(F); }

	GenericValue *getFirstVarArg() { return &(ECStack().back().VarArgs[0]); }

private: // Helper functions
	GenericValue executeGEPOperation(Value *Ptr, gep_type_iterator I, gep_type_iterator E,
					 ExecutionContext &SF);

	// SwitchToNewBasicBlock - Start execution in a new basic block and run any
	// PHI nodes in the top of the block.  This is used for intraprocedural
	// control flow.
	//
	void SwitchToNewBasicBlock(BasicBlock *Dest, ExecutionContext &SF);

	void *getPointerToFunction(Function *F) { return (void *)F; }
	void *getPointerToBasicBlock(BasicBlock *BB) { return (void *)BB; }

	void initializeExecutionEngine() {}
	void initializeExternalFunctions();
	GenericValue getConstantExprValue(ConstantExpr *CE, ExecutionContext &SF);
	GenericValue getOperandValue(Value *V, ExecutionContext &SF);
	GenericValue executeTruncInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeSExtInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeZExtInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeFPTruncInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeFPExtInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeFPToUIInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeFPToSIInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeUIToFPInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeSIToFPInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executePtrToIntInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeIntToPtrInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeBitCastInst(Value *SrcVal, Type *DstTy, ExecutionContext &SF);
	GenericValue executeCastOperation(Instruction::CastOps opcode, Value *SrcVal, Type *Ty,
					  ExecutionContext &SF);
	std::vector<GenericValue>
	translateExternalCallArgs(Function *F, const std::vector<GenericValue> &Args) const;
	void returnValueToCaller(Type *RetTy, GenericValue Result);
	void popStackAndReturnValueToCaller(Type *RetTy, GenericValue Result,
					    ReturnInst *retI = nullptr);

	void handleSystemError(SystemError code, const std::string &msg);

	SVal getInodeTransStatus(void *inode, Type *intTyp);
	void setInodeTransStatus(void *inode, Type *intTyp, SVal status);
	SVal readInodeSizeFS(void *inode, Type *intTyp, const std::unique_ptr<EventDeps> &deps);
	void updateInodeSizeFS(void *inode, Type *intTyp, SVal newSize,
			       const std::unique_ptr<EventDeps> &deps);
	void updateInodeDisksizeFS(void *inode, Type *intTyp, SVal newSize, SVal ordDataBegin,
				   SVal ordDataEnd);
	void writeDataToDisk(void *buf, int bufOffset, void *inode, int inodeOffset, int count,
			     Type *dataTyp, const std::unique_ptr<EventDeps> &deps);
	void readDataFromDisk(void *inode, int inodeOffset, void *buf, int bufOffset, int count,
			      Type *dataTyp, const std::unique_ptr<EventDeps> &deps);
	void updateDirNameInode(const std::string &name, Type *intTyp, SVal inode);

	SVal checkOpenFlagsFS(SVal &flags, Type *intTyp);
	SVal executeInodeLookupFS(const std::string &name, Type *intTyp);
	SVal executeInodeCreateFS(const std::string &name, Type *intTyp,
				  const std::unique_ptr<EventDeps> &deps);
	SVal executeLookupOpenFS(const std::string &filename, SVal &flags, Type *intTyp,
				 const std::unique_ptr<EventDeps> &deps);
	SVal executeOpenFS(const std::string &filename, SVal flags, SVal inode, Type *intTyp,
			   const std::unique_ptr<EventDeps> &deps);

	void executeReleaseFileFS(void *fileDesc, Type *intTyp,
				  const std::unique_ptr<EventDeps> &deps);
	SVal executeCloseFS(SVal fd, Type *intTyp, const std::unique_ptr<EventDeps> &deps);
	SVal executeRenameFS(const std::string &oldpath, SVal oldInode, const std::string &newpath,
			     SVal newInode, Type *intTyp);
	SVal executeLinkFS(const std::string &newpath, SVal oldInode, Type *intTyp);
	SVal executeUnlinkFS(const std::string &pathname, Type *intTyp);

	SVal executeTruncateFS(SVal inode, SVal length, Type *intTyp,
			       const std::unique_ptr<EventDeps> &deps);
	SVal executeReadFS(void *file, Type *intTyp, void *buf, Type *bufElemTyp, SVal offset,
			   SVal count, const std::unique_ptr<EventDeps> &deps);
	void zeroDskRangeFS(void *inode, SVal start, SVal end, Type *writeIntTyp);
	SVal executeWriteChecksFS(void *inode, Type *intTyp, SVal flags, SVal offset, SVal count,
				  SVal &wOffset, const std::unique_ptr<EventDeps> &deps);
	bool shouldUpdateInodeDisksizeFS(void *inode, Type *intTyp, SVal size, SVal offset,
					 SVal count, SVal &dSize);
	SVal executeBufferedWriteFS(void *inode, Type *intTyp, void *buf, Type *bufElemTyp,
				    SVal wOffset, SVal count,
				    const std::unique_ptr<EventDeps> &deps);
	SVal executeWriteFS(void *file, Type *intTyp, void *buf, Type *bufElemTyp, SVal offset,
			    SVal count, const std::unique_ptr<EventDeps> &deps);
	SVal executeLseekFS(void *file, Type *intTyp, SVal offset, SVal whence,
			    const std::unique_ptr<EventDeps> &deps);
	void executeFsyncFS(void *inode, Type *intTyp);

	void setProgramState(ProgramState s) { dynState.programState = s; }
	void setExecState(ExecutionState s) { dynState.execState = s; }

	void handleLock(SAddr addr, ASize size, const EventDeps *deps);
	void handleUnlock(SAddr addr, ASize size, const EventDeps *deps);

	/* Custom Opcode Implementations */
#define HANDLE_FUNCTION(NUM, FUN, NAME)                                                            \
	void call##NAME(Function *F, const std::vector<GenericValue> &ArgVals,                     \
			const std::unique_ptr<EventDeps> &specialDeps);
#include "Runtime/InternalFunction.def"

	void callInternalFunction(Function *F, const std::vector<GenericValue> &ArgVals,
				  const std::unique_ptr<EventDeps> &deps);

	void freeAllocas(const AllocaHolder &allocas);

	/* Collects the addresses (and some naming information) for all variables with
	 * static storage. Also calculates the starting address of the allocation pool */
	void collectStaticAddresses(SAddrAllocator &alloctor);

	/* Sets up how some errors will be reported to the user */
	void setupErrorPolicy(Module *M, const Config *userConf);

	/* Pers: Sets up information about the modeled filesystem */
	void setupFsInfo(Module *M, const Config *userConf);

	/* Adds the specified thread to the list */
	Thread &addNewThread(Thread &&thread);

	/* Creates an entry for the main() function. More information are
	 * filled from the execution engine when the exploration starts */
	Thread &createAddMainThread();

	/* Dependency tracking */

	DepTracker *getDepTracker() { return &*dynState.depTracker; }
	const DepTracker *getDepTracker() const { return &*dynState.depTracker; }

	std::unique_ptr<EventDeps> makeEventDeps(const DepInfo *addr, const DepInfo *data,
						 const DepInfo *ctrl, const DepInfo *addrPo,
						 const DepInfo *cas);

	const DepInfo *getDataDeps(unsigned int tid, Value *i)
	{
		return getDepTracker() ? getDepTracker()->getDataDeps(tid, i) : nullptr;
	}
	const DepInfo *getAddrPoDeps(unsigned int tid)
	{
		return getDepTracker() ? getDepTracker()->getAddrPoDeps(tid) : nullptr;
	}
	const DepInfo *getCtrlDeps(unsigned int tid)
	{
		return getDepTracker() ? getDepTracker()->getCtrlDeps(tid) : nullptr;
	}

	void updateDataDeps(unsigned int tid, Value *dst, Value *src)
	{
		if (getDepTracker())
			getDepTracker()->updateDataDeps(tid, dst, src);
	}
	void updateDataDeps(unsigned int tid, Value *dst, const DepInfo *e)
	{
		if (getDepTracker())
			getDepTracker()->updateDataDeps(tid, dst, *e);
	}
	void updateDataDeps(unsigned int tid, Value *dst, Event e)
	{
		if (getDepTracker())
			getDepTracker()->updateDataDeps(tid, dst, e);
	}
	void updateAddrPoDeps(unsigned int tid, Value *src)
	{
		if (getDepTracker())
			getDepTracker()->updateAddrPoDeps(tid, src);
	}
	void updateCtrlDeps(unsigned int tid, Value *src)
	{
		if (getDepTracker())
			getDepTracker()->updateCtrlDeps(tid, src);
	}
	void updateCtrlDeps(unsigned int tid, Event e)
	{
		if (getDepTracker())
			getDepTracker()->updateCtrlDeps(tid, e);
	}

	std::unique_ptr<EventDeps> updateFunArgDeps(unsigned int tid, Function *F);
	void updateInternalFunRetDeps(unsigned int tid, Function *F, Instruction *CS);

	void clearDeps(unsigned int tid)
	{
		if (getDepTracker())
			getDepTracker()->clearDeps(tid);
	}

	/* Gets naming information for value V (or value with key KEY), if it is
	 * an internal variable with no value correspondence */
	const NameInfo *getVarNameInfo(Value *v, StorageDuration sd, AddressSpace spc,
				       const VariableInfo<ModuleID::ID>::InternalKey &key = {});

	/* Pers: Returns the address of the file description referenced by FD */
	void *getFileFromFd(int fd) const;

	/* Pers: Tracks that the address of the file description of FD is FILEADDR */
	void setFdToFile(int fd, void *fileAddr);

	/* Pers: Directory operations */
	void *getDirInode() const;
	void *getInodeAddrFromName(const std::string &filename) const;
};

} // namespace llvm

#endif
