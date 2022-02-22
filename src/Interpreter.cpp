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

#include "Config.hpp"
#include "Error.hpp"
#include "Interpreter.h"
#include <llvm/CodeGen/IntrinsicLowering.h>
#if defined(HAVE_LLVM_IR_DERIVEDTYPES_H)
#include <llvm/IR/DerivedTypes.h>
#elif defined(HAVE_LLVM_DERIVEDTYPES_H)
#include <llvm/DerivedTypes.h>
#endif
#if defined(HAVE_LLVM_IR_MODULE_H)
#include <llvm/IR/Module.h>
#elif defined(HAVE_LLVM_MODULE_H)
#include <llvm/Module.h>
#endif
#include <cstring>

using namespace llvm;

extern "C" void LLVMLinkInInterpreter() { }

/// create - Create a new interpreter object.  This can never fail.
///
ExecutionEngine *Interpreter::create(std::unique_ptr<Module> M, std::unique_ptr<ModuleInfo> MI,
				     GenMCDriver *driver, const Config *userConf, std::string* ErrStr) {
  // Tell this Module to materialize everything and release the GVMaterializer.
#ifdef LLVM_MODULE_MATERIALIZE_ALL_PERMANENTLY_ERRORCODE_BOOL
  if (std::error_code EC = M->materializeAllPermanently()) {
    if (ErrStr)
      *ErrStr = EC.message();
    // We got an error, just return 0
    return nullptr;
  }
#elif defined LLVM_MODULE_MATERIALIZE_ALL_PERMANENTLY_BOOL_STRPTR
  if (M->MaterializeAllPermanently(ErrStr)){
    // We got an error, just return 0
    return nullptr;
  }
#elif defined LLVM_MODULE_MATERIALIZE_ALL_LLVM_ERROR
  if (Error Err = M->materializeAll()) {
    std::string Msg;
    handleAllErrors(std::move(Err), [&](ErrorInfoBase &EIB) {
      Msg = EIB.message();
    });
    if (ErrStr)
      *ErrStr = Msg;
    // We got an error, just return 0
    return nullptr;
  }
#else
  if(std::error_code EC = M->materializeAll()){
    if(ErrStr)
      *ErrStr = EC.message();
    // We got an error, just return 0
    return nullptr;
  }
#endif

  return new Interpreter(std::move(M), std::move(MI), driver, userConf);
}

/* Thread::seed is ODR-used -- we need to provide a definition (C++14) */
constexpr int Thread::seed;

llvm::raw_ostream& llvm::operator<<(llvm::raw_ostream &s, const Thread &thr)
{
	return s << "<" << thr.parentId << ", " << thr.id << ">"
		 << " " << thr.threadFun->getName().str();
}

EELocalState::EELocalState(const SAddrAllocator &alloctor,
			   const std::unique_ptr<DepTracker> &depTr,
			   const ExecutionState &execState,
			   const ProgramState &programState,
			   const llvm::BitVector &fds,
			   const llvm::IndexedMap<void *> &fdToFile,
			   const std::unordered_map<std::string, void *> &nameToInodeAddr,
			   const std::vector<Thread> &ts,
			   int current)
	: alloctor(alloctor),
	  depTracker(depTr ? LLVM_MAKE_UNIQUE<DepTracker>(*depTr) : nullptr),
	  execState(execState),
	  programState(programState), fds(fds), fdToFile(fdToFile),
	  nameToInodeAddr(nameToInodeAddr), threads(ts), currentThread(current) {}

std::unique_ptr<EELocalState> Interpreter::releaseLocalState()
{
	return LLVM_MAKE_UNIQUE<EELocalState>(alloctor, depTracker, execState, programState,
					      fds, fdToFile, nameToInodeAddr, threads, currentThread);
}

void Interpreter::restoreLocalState(std::unique_ptr<EELocalState> state)
{
	alloctor = std::move(state->alloctor);
	depTracker = std::move(state->depTracker);
	execState = state->execState;
	programState = state->programState;
	fds = std::move(state->fds);
	fdToFile = std::move(state->fdToFile);
	nameToInodeAddr = std::move(state->nameToInodeAddr);
	threads = std::move(state->threads);
	currentThread = state->currentThread;
}

void Interpreter::resetThread(unsigned int id)
{
	auto &thr = getThrById(id);
	thr.ECStack = {};
	thr.tls = threadLocalVars;
	thr.blocked = BlockageType::NotBlocked;
	thr.globalInstructions = 0;
	thr.rng.seed(Thread::seed);
	clearDeps(id);
}

void Interpreter::reset()
{
	/*
	 * Make sure that all execution stacks are empty since there may
	 * have been a failed assume on some thread and a join waiting on
	 * that thread (joins do not empty ECStacks)
	 */
	currentThread = 0;
	std::for_each(threads_begin(), threads_end(), [this](Thread &thr){ resetThread(thr.id); });
}

void Interpreter::setupRecoveryRoutine(int tid)
{
	BUG_ON(tid >= threads.size());
	currentThread = tid;

	/* Only fill the stack if a recovery routine actually exists... */
	ERROR_ON(!recoveryRoutine, "No recovery routine specified!\n");
	callFunction(recoveryRoutine, {}, nullptr);

	/* Also set up initSF, if it is the first invocation */
	if (!getThrById(tid).initSF.CurFunction)
		getThrById(tid).initSF = ECStack().back();
	return;
}

void Interpreter::cleanupRecoveryRoutine(int tid)
{
	/* Nothing to do -- yet */
	currentThread = 0;
	return;
}

Thread &Interpreter::addNewThread(Thread &&thread)
{
	if (thread.id == threads.size()) {
	        threads.push_back(std::move(thread));
		return threads.back();
	}
	BUG_ON(threads[thread.id].threadFun != thread.threadFun || threads[thread.id].id != thread.id);
	return threads[thread.id] = std::move(thread);
}

/* Creates an entry for the main() function */
Thread &Interpreter::createAddMainThread(llvm::Function *F)
{
	Thread main(F, 0);
	main.tls = threadLocalVars;
	threads.clear(); /* make sure its empty */
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

Thread &Interpreter::createAddRecoveryThread(int tid)
{
	Thread rec(recoveryRoutine, tid);
	rec.tls = threadLocalVars;
	return addNewThread(std::move(rec));
}

#ifndef LLVM_BITVECTOR_HAS_FIND_FIRST_UNSET
int my_find_first_unset(const llvm::BitVector &bv)
{
	for (auto i = 0u; i < bv.size(); i++)
		if (bv[i] == 0)
			return i;
	return -1;
}
#endif /* LLVM_BITVECTOR_HAS_FIND_FIRST_UNSET */

int Interpreter::getFreshFd()
{
#ifndef LLVM_BITVECTOR_HAS_FIND_FIRST_UNSET
	int fd = my_find_first_unset(fds);
#else
	int fd = fds.find_first_unset();
#endif

	/* If no available descriptor found, grow fds and try again */
	if (fd == -1) {
		fds.resize(2 * fds.size() + 1);
		fdToFile.grow(fds.size());
		return getFreshFd();
	}

	/* Otherwise, mark the file descriptor as used */
	markFdAsUsed(fd);
	return fd;
}

void Interpreter::markFdAsUsed(int fd)
{
	fds.set(fd);
}

void Interpreter::reclaimUnusedFd(int fd)
{
	fds.reset(fd);
}

#ifdef LLVM_GLOBALVALUE_HAS_GET_ADDRESS_SPACE
# define GET_GV_ADDRESS_SPACE(v) (v).getAddressSpace()
#else
# define GET_GV_ADDRESS_SPACE(v)			\
({						        \
	llvm::PointerType *pTy = v.getType();		\
	pTy->getAddressSpace();				\
})
#endif

void Interpreter::collectStaticAddresses(Module *M)
{
	std::vector<std::pair<const GlobalVariable *, void *> > toReinitialize;

	for (auto &v : M->getGlobalList()) {
		char *ptr = static_cast<char *>(GVTOP(getConstantValue(&v)));
		unsigned int typeSize =
		        getDataLayout().getTypeAllocSize(v.getType()->getElementType());

		/* Record whether this is a thread local variable or not */
		if (v.isThreadLocal()) {
			for (auto i = 0u; i < typeSize; i++)
				threadLocalVars[ptr + i] = getConstantValue(v.getInitializer());
			continue;
		}

		/* "Allocate" an address for this global variable... */
		auto addr = alloctor.allocStatic(typeSize, v.getAlignment(),
						 GET_GV_ADDRESS_SPACE(v) == 42);
		staticAllocas.insert(std::make_pair(addr, addr + typeSize - 1));
		staticValueMap[addr] = ptr;

		/* ... and use that in the EE instead. Make sure to re-initialize it too;
		 * it might contain the address of another global */
		updateGlobalMapping(&v, (void *) addr.get());
		toReinitialize.push_back(std::make_pair(&v, ptr));

		/* Update naming information */
		staticNames[addr] = &v;
		if (!v.hasPrivateLinkage() && (!MI->idInfo.VID.count(&v) ||
					       !MI->varInfo.globalInfo.count(MI->idInfo.VID.at(&v)))) {
			WARN_ONCE("name-info", ("Inadequate naming info for variable " + v.getName() +
						".\nPlease submit a bug report to " PACKAGE_BUGREPORT "\n"));
		}
	}

	/* Now that we've updated all mappings, go ahead and re-initialize */
	for (auto &p : toReinitialize)
		InitializeMemory(p.first->getInitializer(), p.second);
}

void Interpreter::setupErrorPolicy(Module *M, const Config *userConf)
{
	stopOnSystemErrors = !userConf->disableStopOnSystemError;

	auto *errnoVar = M->getGlobalVariable("errno");
	if (!errnoVar)
		return;

	errnoAddr = GVTOP(getConstantValue(errnoVar));
	errnoTyp = errnoVar->getType()->getElementType();
	return;
}

void Interpreter::setupFsInfo(Module *M, const Config *userConf)
{
	auto &FI = MI->fsInfo;

	/* Setup config options first */
	FI.blockSize = userConf->blockSize;
	FI.maxFileSize = userConf->maxFileSize;
	FI.journalData = userConf->journalData;
	FI.delalloc = FI.journalData == JournalDataFS::ordered && !userConf->disableDelalloc;

	auto *inodeVar = M->getGlobalVariable("__genmc_dir_inode");
	auto *fileVar = M->getGlobalVariable("__genmc_dummy_file");

	/* unistd.h not included -- not dealing with fs stuff */
	if (!inodeVar || !fileVar)
		return;

	fds = llvm::BitVector(20);
	fdToFile.grow(fds.size());

	FI.inodeTyp = dyn_cast<StructType>(inodeVar->getType()->getElementType());
	FI.fileTyp = dyn_cast<StructType>(fileVar->getType()->getElementType());
	BUG_ON(!FI.inodeTyp || !FI.fileTyp);

	/* Initialize the directory's inode -- assume that the first field is int
	 * We track this here to have custom naming info */
	unsigned int inodeSize = getTypeSize(FI.inodeTyp);
	FI.dirInode = static_cast<char *>(GVTOP(getConstantValue(inodeVar)));

	Type *intTyp = FI.inodeTyp->getElementType(0);
	unsigned int intSize = getTypeSize(intTyp);

	unsigned int count = 0;
	unsigned int intPtrSize = getTypeSize(intTyp->getPointerTo());
	auto *SL = getDataLayout().getStructLayout(FI.inodeTyp);
	for (auto &fname : FI.filenames) {
		auto *addr = (char *) FI.dirInode + SL->getElementOffset(4) + count * intPtrSize;
		nameToInodeAddr[fname] = addr;
		++count;
	}
	return;
}

std::unique_ptr<EventDeps>
Interpreter::makeEventDeps(const DepInfo *addr, const DepInfo *data,
			   const DepInfo *ctrl, const DepInfo *addrPo,
			   const DepInfo *cas)
{
	if (!depTracker)
		return nullptr;

	auto result = LLVM_MAKE_UNIQUE<EventDeps>();

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

std::unique_ptr<EventDeps>
Interpreter::updateFunArgDeps(unsigned int tid, Function *fun)
{
	if (!depTracker)
		return nullptr;

	ExecutionContext &SF = ECStack().back();
	auto name = fun->getName().str();

	/* Handling non-internals is straightforward: the parameters
	 * of the function called get the data dependencies of the
	 * actual arguments */
	bool isInternal = internalFunNames.count(name);
	if (!isInternal) {
		auto ai = fun->arg_begin();
		for (auto ci = SF.Caller.arg_begin(),
			     ce = SF.Caller.arg_end(); ci != ce; ++ci, ++ai) {
			updateDataDeps(tid, &*ai, &*ci->get());
		}
		return nullptr;
	}

	auto iFunCode = internalFunNames.at(name);
	if (iFunCode == InternalFunctions::FN_Assume) {
		/* We have ctrl dependency on the argument of an assume() */
		for (auto i = SF.Caller.arg_begin(),
			     e = SF.Caller.arg_end(); i != e; ++i) {
			updateCtrlDeps(tid, *i);
		}
	} else if (isMutexCode(iFunCode) || isBarrierCode(iFunCode)) {
		/* We have addr dependency on the argument of mutex/barrier calls */
		return makeEventDeps(getDataDeps(tid, *SF.Caller.arg_begin()),
				     nullptr, getCtrlDeps(tid),
				     getAddrPoDeps(tid), nullptr);
	}
	return nullptr;
}

//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(std::unique_ptr<Module> M, std::unique_ptr<ModuleInfo> MI,
			 GenMCDriver *driver, const Config *userConf)
	: ExecutionEngine(std::move(M)), MI(std::move(MI)), driver(driver) {

  memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));

  // Initialize the "backend"
  initializeExecutionEngine();
  initializeExternalFunctions();
  emitGlobals();

  IL = new IntrinsicLowering(getDataLayout());

  auto mod = Modules.back().get();
  collectStaticAddresses(mod);

  /* Set up a dependency tracker if the model requires it */
  if (userConf->isDepTrackingModel)
	  depTracker = LLVM_MAKE_UNIQUE<DepTracker>();

  /* Set up the system error policy */
  setupErrorPolicy(mod, userConf);

  /* Also run a recovery routine if it is required to do so */
  checkPersistency = userConf->persevere;
  recoveryRoutine = mod->getFunction("__VERIFIER_recovery_routine");
  setupFsInfo(mod, userConf);

  /* Setup the interpreter for the exploration */
  auto mainFun = mod->getFunction(userConf->programEntryFun);
  ERROR_ON(!mainFun, "Could not find program's entry point function!\n");

  createAddMainThread(mainFun);
}

Interpreter::~Interpreter() {
  delete IL;
}

void Interpreter::runAtExitHandlers () {
  while (!AtExitHandlers.empty()) {
    callFunction(AtExitHandlers.back(), std::vector<GenericValue>(), nullptr);
    AtExitHandlers.pop_back();
    run();
  }
}

/// run - Start execution with the specified function and arguments.
///
#ifdef LLVM_EXECUTION_ENGINE_RUN_FUNCTION_VECTOR
GenericValue
Interpreter::runFunction(Function *F,
                         const std::vector<GenericValue> &ArgValues) {
#else
GenericValue
Interpreter::runFunction(Function *F,
                         ArrayRef<GenericValue> ArgValues) {
#endif
  assert (F && "Function *F was null at entry to run()");

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

  // Set up the function call.
  callFunction(F, ActualArgs, nullptr);

  // Start executing the function.
  run();

  return ExitValue;
}
