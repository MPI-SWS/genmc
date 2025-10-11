// For the parts of the code originating from LLVM-3.5:
//===- Interpreter.cpp - Top-Level LLVM Interpreter Implementation --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LLVMLICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the top-level functionality for the LLVM interpreter.
// This interpreter is designed to be a very simple, portable, inefficient
// interpreter.
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

#include "Runtime/Interpreter.h"
#include "Runtime/LLIConfig.hpp"
#include "Static/LLVMUtils.hpp"
#include "Support/Error.hpp"
#include <cstring>
#include <llvm/CodeGen/IntrinsicLowering.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <optional>

using namespace llvm;

extern "C" void LLVMLinkInInterpreter() {}

/// create - Create a new interpreter object.  This can never fail.
///
std::unique_ptr<Interpreter> Interpreter::create(std::unique_ptr<Module> M,
						 std::unique_ptr<ModuleInfo> MI,
						 GenMCDriver *driver, const LLIConfig *userConf,
						 SAddrAllocator &alloctor, std::string *ErrStr)
{
	// Tell this Module to materialize everything and release the GVMaterializer.
	if (Error Err = M->materializeAll()) {
		std::string Msg;
		handleAllErrors(std::move(Err), [&](ErrorInfoBase &EIB) { Msg = EIB.message(); });
		if (ErrStr)
			*ErrStr = Msg;
		// We got an error, just return 0
		return nullptr;
	}
	return std::make_unique<Interpreter>(std::move(M), std::move(MI), driver, userConf,
					     alloctor);
}

llvm::raw_ostream &llvm::operator<<(llvm::raw_ostream &s, const Thread &thr)
{
	return s << "<" << thr.parentId << ", " << thr.id << ">"
		 << " " << thr.threadFun->getName().str();
}

std::unique_ptr<InterpreterState> Interpreter::saveState()
{
	/* This function may be called during `GenMCDriver::scheduleNext` during the optimized
	 * scheduling part, when we hold a reference to the memory of `globalInstructions`. If a
	 * warning is triggered during this, we will save the state with this function, then later
	 * restore it. We need to ensure that the reference to `globalInstructions` stays valid, so
	 * we swap the copy and the original here.
	 */
	auto tmp = std::make_unique<InterpreterState>(dynState);
	std::swap(tmp->globalInstructions, dynState.globalInstructions);
	return std::move(tmp);
}

void Interpreter::restoreState(std::unique_ptr<InterpreterState> s) { dynState = std::move(*s); }

void Interpreter::resetThread(unsigned int id)
{
	auto &thr = getThrById(id);
	thr.ECStack = {};
	thr.tls = threadLocalVars;
	thr.blocked = BlockageType::NotBlocked;
	thr.rng.seed(Thread::seed);

	clearDeps(id);

	/* We don't _have to_ figure out the initial action for each thread here (since this is
	 * done by addNewThread() which is called at the beginning of each execution), but
	 * we can do it just in case this is not the case in the future */
	BUG_ON(thr.initEC.empty() && thr.id != 0);
	auto kind = thr.initEC.empty() ? ActionKind::Load
				       : getInstKind(&*thr.initEC.back().CurInst);
	dynState.globalInstructions[thr.id] = {kind, Event(thr.id, 0)};
}

void Interpreter::reset()
{
	/*
	 * Make sure that all execution stacks are empty since there may
	 * have been a failed assume on some thread and a join waiting on
	 * that thread (joins do not empty ECStacks)
	 */
	dynState.currentThread = 0;
	dynState.AtExitHandlers.clear();
	std::for_each(threads_begin(), threads_end(), [this](Thread &thr) { resetThread(thr.id); });
}

Thread &Interpreter::addNewThread(Thread &&thread)
{
	/* In case of a replay, do nothing (so that we don't have to replay in porf) */
	if (thread.id < getNumThreads()) {
		auto &exstThr = dynState.threads[thread.id];
		BUG_ON(exstThr.id != thread.id);
		BUG_ON(exstThr.parentId != thread.parentId);
		BUG_ON(exstThr.threadFun != thread.threadFun);
		return exstThr;
	}

	BUG_ON(thread.id != getNumThreads());
	dynState.threads.push_back(std::move(thread));

	auto &thr = dynState.threads.back();
	BUG_ON(thr.initEC.empty() && thr.id != 0);
	auto kind = thr.initEC.empty() ? ActionKind::Load
				       : getInstKind(&*thr.initEC.back().CurInst);
	dynState.globalInstructions.emplace_back(kind, Event(thr.id, 0));
	return thr;
}

/* Creates an entry for the main() function */
Thread &Interpreter::createAddMainThread()
{
	Thread main(mainFun, 0);
	main.tls = threadLocalVars;
	dynState.threads.clear(); /* make sure its empty */
	dynState.globalInstructions.clear();
	return addNewThread(std::move(main));
}

/* Creates an entry for another thread */
Thread &Interpreter::createAddNewThread(llvm::Function *F, SVal arg, int tid, int pid,
					const llvm::ExecutionContext &SF)
{
	Thread thr(F, arg, tid, pid, SF);
	thr.ECStack.push_back(SF);
	thr.tls = threadLocalVars;
	return addNewThread(std::move(thr));
}

void Interpreter::collectStaticAddresses(SAddrAllocator &alloctor)
{
	auto *M = Modules.back().get();
	std::vector<std::pair<const GlobalVariable *, void *>> toReinitialize;

	for (auto &v : GLOBALS(*M)) {
		char *ptr = static_cast<char *>(GVTOP(getConstantValue(&v)));
		unsigned int typeSize = getDataLayout().getTypeAllocSize(v.getValueType());

		/* Record whether this is a thread local variable or not */
		if (v.isThreadLocal()) {
			for (auto i = 0u; i < typeSize; i++)
				threadLocalVars[ptr + i] = getConstantValue(v.getInitializer());
			continue;
		}

		/* "Allocate" an address for this global variable... */
		auto addr = alloctor.allocStatic(0, typeSize, v.getAlignment(),
						 v.getSection() == "__genmc_persist",
						 v.getAddressSpace() == 42);
		staticAllocas.insert(std::make_pair(addr, addr + ASize(typeSize - 1)));
		staticValueMap[addr] = ptr;

		/* ... and use that in the EE instead. Make sure to re-initialize it too;
		 * it might contain the address of another global */
		updateGlobalMapping(&v, (void *)addr.get());
		toReinitialize.push_back(std::make_pair(&v, ptr));

		/* Update naming information */
		staticNames[addr] = &v;
		if (!v.hasPrivateLinkage() &&
		    (!MI->idInfo.VID.count(&v) ||
		     !MI->varInfo.globalInfo.count(MI->idInfo.VID.at(&v)))) {
			WARN_ONCE("name-info",
				  ("Inadequate naming info for variable " + v.getName() +
				   ".\nPlease submit a bug report to " PACKAGE_BUGREPORT "\n"));
		}
	}

	/* Now that we've updated all mappings, go ahead and re-initialize */
	for (auto &p : toReinitialize)
		InitializeMemory(p.first->getInitializer(), p.second);
}

void Interpreter::setupErrorPolicy(Module *M, const LLIConfig *userConf)
{
	stopOnSystemErrors = !userConf->disableStopOnSystemError;

	auto *errnoVar = M->getGlobalVariable("errno");
	if (!errnoVar)
		return;

	errnoAddr = GVTOP(getConstantValue(errnoVar));
	errnoTyp = errnoVar->getValueType();
	return;
}

std::unique_ptr<EventDeps> Interpreter::makeEventDeps(const DepInfo *addr, const DepInfo *data,
						      const DepInfo *ctrl, const DepInfo *addrPo,
						      const DepInfo *cas)
{
	if (!getDepTracker())
		return nullptr;

	auto result = std::make_unique<EventDeps>();

	if (addr)
		result->addr = *addr;
	if (data)
		result->data = *data;
	if (ctrl)
		result->ctrl = *ctrl;
	if (addrPo)
		result->addrPo = *addrPo;
	if (cas)
		result->cas = *cas;
	return result;
}

std::unique_ptr<EventDeps> Interpreter::updateFunArgDeps(unsigned int tid, Function *fun)
{
	if (!getDepTracker())
		return nullptr;

	ExecutionContext &SF = ECStack().back();
	auto name = fun->getName().str();

	/* Handling non-internals is straightforward: the parameters
	 * of the function called get the data dependencies of the
	 * actual arguments */
	bool isInternal = internalFunNames.count(name);
	if (!isInternal) {
		auto ai = fun->arg_begin();
		for (auto ci = SF.Caller.arg_begin(), ce = SF.Caller.arg_end(); ci != ce;
		     ++ci, ++ai) {
			updateDataDeps(tid, &*ai, &*ci->get());
		}
		return nullptr;
	}

	/* We have ctrl dependency on the argument of an assume() */
	if (isAssumeFunction(name)) {
		for (auto i = SF.Caller.arg_begin(), e = SF.Caller.arg_end(); i != e; ++i) {
			updateCtrlDeps(tid, *i);
		}
		return nullptr;
	}
	/* We have addr dependency on the argument of mutex/barrier/condvar calls */
	auto iFunCode = internalFunNames.at(name);
	if (isMutexCode(iFunCode) || isCondVarCode(iFunCode)) {
		return makeEventDeps(getDataDeps(tid, *SF.Caller.arg_begin()), nullptr,
				     getCtrlDeps(tid), getAddrPoDeps(tid), nullptr);
	}
	return nullptr;
}

void Interpreter::updateInternalFunRetDeps(unsigned int tid, Function *F, Instruction *CS)
{
	auto name = F->getName().str();
	if (!internalFunNames.count(name))
		return;

	auto iFunCode = internalFunNames.at(name);
	if (isAllocFunction(name))
		updateDataDeps(tid, CS, threadPos(tid));
}

//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(std::unique_ptr<Module> M, std::unique_ptr<ModuleInfo> MI,
			 GenMCDriver *driver, const LLIConfig *userConf, SAddrAllocator &alloctor)
	: ExecutionEngine(std::move(M)), MI(std::move(MI)), driver(driver)
{

	memset(&dynState.ExitValue.Untyped, 0, sizeof(dynState.ExitValue.Untyped));

	// Initialize the "backend"
	initializeExecutionEngine();
	initializeExternalFunctions();
	emitGlobals();

	IL = new IntrinsicLowering(getDataLayout());

	auto mod = Modules.back().get();

	/* Set up a dependency tracker if the model requires it */
	if (userConf->isDepTrackingModel)
		dynState.depTracker = std::make_unique<DepTracker>();

	collectStaticAddresses(alloctor);

	/* Set up the system error policy */
	setupErrorPolicy(mod, userConf);

	/* Setup the interpreter for the exploration */
	mainFun = FindFunctionNamed(userConf->programEntryFun);
	ERROR_ON(!mainFun, "Could not find program's entry point function!\n");

	createAddMainThread();
}

Interpreter::~Interpreter() { delete IL; }

namespace {
class ArgvArray {
	std::unique_ptr<char[]> Array;
	std::vector<std::unique_ptr<char[]>> Values;

public:
	/// Turn a vector of strings into a nice argv style array of pointers to null
	/// terminated strings.
	void *reset(LLVMContext &C, ExecutionEngine *EE, const std::vector<std::string> &InputArgv);
};
} // anonymous namespace
void *ArgvArray::reset(LLVMContext &C, ExecutionEngine *EE,
		       const std::vector<std::string> &InputArgv)
{
	Values.clear(); // Free the old contents.
	Values.reserve(InputArgv.size());
	unsigned PtrSize = EE->getDataLayout().getPointerSize();
	Array = std::make_unique<char[]>((InputArgv.size() + 1) * PtrSize);

	// LLVM_DEBUG(dbgs() << "JIT: ARGV = " << (void *)Array.get() << "\n");
#if LLVM_VERSION_MAJOR < 18
	Type *SBytePtr = Type::getInt8PtrTy(C);
#else
	Type *SBytePtr = PointerType::getUnqual(C);
#endif

	for (unsigned i = 0; i != InputArgv.size(); ++i) {
		unsigned Size = InputArgv[i].size() + 1;
		auto Dest = std::make_unique<char[]>(Size);
		// LLVM_DEBUG(dbgs() << "JIT: ARGV[" << i << "] = " << (void *)Dest.get()
		//                   << "\n");

		std::copy(InputArgv[i].begin(), InputArgv[i].end(), Dest.get());
		Dest[Size - 1] = 0;

		// Endian safe: Array[i] = (PointerTy)Dest;
		EE->StoreValueToMemory(PTOGV(Dest.get()), (GenericValue *)(&Array[i * PtrSize]),
				       SBytePtr);
		Values.push_back(std::move(Dest));
	}

	// Null terminate it
	EE->StoreValueToMemory(PTOGV(nullptr), (GenericValue *)(&Array[InputArgv.size() * PtrSize]),
			       SBytePtr);
	return Array.get();
}

void Interpreter::setupFunctionCall(Function *F, ArrayRef<GenericValue> ArgValues)
{
	// Try extra hard not to pass extra args to a function that isn't
	// expecting them.  C programmers frequently bend the rules and
	// declare main() with fewer parameters than it actually gets
	// passed, and the interpreter barfs if you pass a function more
	// parameters than it is declared to take. This does not attempt to
	// take into account gratuitous differences in declared types,
	// though.
	std::vector<GenericValue> ActualArgs;
	const unsigned ArgCount = F->getFunctionType()->getNumParams();
	for (unsigned i = 0; i < ArgCount; ++i)
		ActualArgs.push_back(ArgValues[i]);

	callFunction(F, ActualArgs, nullptr);
}

/// run - Start execution with the specified function and arguments.
///
GenericValue Interpreter::runFunction(Function *F, ArrayRef<GenericValue> ArgValues)
{
	assert(F && "Function *F was null at entry to run()");

	// Set up the function call.
	setupFunctionCall(F, ArgValues);

	// Start executing the function.
	run();

	return dynState.ExitValue;
}

void Interpreter::setupStaticCtorsDtors(Module &module, bool isDtors)
{
	StringRef Name(isDtors ? "llvm.global_dtors" : "llvm.global_ctors");
	GlobalVariable *GV = module.getNamedGlobal(Name);

	// If this global has internal linkage, or if it has a use, then it must be
	// an old-style (llvmgcc3) static ctor with __main linked in and in use.  If
	// this is the case, don't execute any of the global ctors, __main will do
	// it.
	if (!GV || GV->isDeclaration() || GV->hasLocalLinkage())
		return;

	// Should be an array of '{ i32, void ()* }' structs.  The first value is
	// the init priority, which we ignore.
	ConstantArray *InitList = dyn_cast<ConstantArray>(GV->getInitializer());
	if (!InitList)
		return;
	for (unsigned i = 0, e = InitList->getNumOperands(); i != e; ++i) {
		ConstantStruct *CS = dyn_cast<ConstantStruct>(InitList->getOperand(i));
		if (!CS)
			continue;

		Constant *FP = CS->getOperand(1);
		if (FP->isNullValue())
			continue; // Found a sentinal value, ignore.

		// Strip off constant expression casts.
		if (ConstantExpr *CE = dyn_cast<ConstantExpr>(FP))
			if (CE->isCast())
				FP = CE->getOperand(0);

		// Setup the ctor/dtor SF and quit
		if (Function *F = dyn_cast<Function>(FP))
			setupFunctionCall(F,
#if LLVM_VERSION_MAJOR >= 16
					  std::nullopt
#else
					  None
#endif
			);

		// FIXME: It is marginally lame that we just do nothing here if we see an
		// entry we don't recognize. It might not be unreasonable for the verifier
		// to not even allow this and just assert here.
	}
}

void Interpreter::setupStaticCtorsDtors(bool isDtors)
{
	// Execute global ctors/dtors for each module in the program.
	for (std::unique_ptr<Module> &M : Modules)
		setupStaticCtorsDtors(*M, isDtors);
}

#ifndef NDEBUG
/// isTargetNullPtr - Return whether the target pointer stored at Loc is null.
static bool isTargetNullPtr(ExecutionEngine *EE, void *Loc)
{
	unsigned PtrSize = EE->getDataLayout().getPointerSize();
	for (unsigned i = 0; i < PtrSize; ++i)
		if (*(i + (uint8_t *)Loc))
			return false;
	return true;
}
#endif

void Interpreter::setupMain(Function *Fn, const std::vector<std::string> &argv,
			    const char *const *envp)
{
	std::vector<GenericValue> GVArgs;
	GenericValue GVArgc;
	GVArgc.IntVal = APInt(32, argv.size());

	// Check main() type
	unsigned NumArgs = Fn->getFunctionType()->getNumParams();
	FunctionType *FTy = Fn->getFunctionType();
#if LLVM_VERSION_MAJOR < 18
	Type *PPInt8Ty = Type::getInt8PtrTy(Fn->getContext())->getPointerTo();
#else
	Type *PPInt8Ty = PointerType::get(Fn->getContext(), 0);
#endif

	// Check the argument types.
	if (NumArgs > 3)
		report_fatal_error("Invalid number of arguments of main() supplied");
	if (NumArgs >= 3 && FTy->getParamType(2) != PPInt8Ty)
		report_fatal_error("Invalid type for third argument of main() supplied");
	if (NumArgs >= 2 && FTy->getParamType(1) != PPInt8Ty)
		report_fatal_error("Invalid type for second argument of main() supplied");
	if (NumArgs >= 1 && !FTy->getParamType(0)->isIntegerTy(32))
		report_fatal_error("Invalid type for first argument of main() supplied");
	if (!FTy->getReturnType()->isIntegerTy() && !FTy->getReturnType()->isVoidTy())
		report_fatal_error("Invalid return type of main() supplied");

	ArgvArray CArgv;
	ArgvArray CEnv;
	if (NumArgs) {
		GVArgs.push_back(GVArgc); // Arg #0 = argc.
		if (NumArgs > 1) {
			// Arg #1 = argv.
			GVArgs.push_back(PTOGV(CArgv.reset(Fn->getContext(), this, argv)));
			assert(!isTargetNullPtr(this, GVTOP(GVArgs[1])) &&
			       "argv[0] was null after CreateArgv");
			if (NumArgs > 2) {
				std::vector<std::string> EnvVars;
				for (unsigned i = 0; envp[i]; ++i)
					EnvVars.emplace_back(envp[i]);
				// Arg #2 = envp.
				GVArgs.push_back(
					PTOGV(CEnv.reset(Fn->getContext(), this, EnvVars)));
			}
		}
	}

	setupFunctionCall(Fn, GVArgs);
}
