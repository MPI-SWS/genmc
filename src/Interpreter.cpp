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
ExecutionEngine *Interpreter::create(Module *M, VariableInfo &&VI, FsInfo &&FI,
				     GenMCDriver *driver, const Config *userConf,
				     std::string* ErrStr) {
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

  return new Interpreter(M, std::move(VI), std::move(FI), driver, userConf);
}

/* Thread::seed is ODR-used -- we need to provide a definition (C++14) */
constexpr int Thread::seed;

llvm::raw_ostream& llvm::operator<<(llvm::raw_ostream &s, const Thread &thr)
{
	return s << "<" << thr.parentId << ", " << thr.id << ">"
		 << " " << thr.threadFun->getName().str();
}

/* Resets the interpreter for a new exploration */
void Interpreter::reset()
{
	/*
	 * Make sure that all execution stacks are empty since there may
	 * have been a failed assume on some thread and a join waiting on
	 * that thread (joins do not empty ECStacks)
	 */
	currentThread = 0;
	for (auto i = 0u; i < threads.size(); i++) {
		threads[i].ECStack = {};
		threads[i].tls = threadLocalVars;
		threads[i].isBlocked = false;
		threads[i].globalInstructions = 0;
		threads[i].rng.seed(Thread::seed);
		clearDeps(i);
	}
}

void Interpreter::setupRecoveryRoutine(int tid)
{
	BUG_ON(tid >= threads.size());
	currentThread = tid;

	/* Only fill the stack if a recovery routine actually exists... */
	ERROR_ON(!recoveryRoutine, "No recovery routine specified!\n");
	callFunction(recoveryRoutine, {});

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

/* Creates an entry for the main() function */
Thread Interpreter::createMainThread(llvm::Function *F)
{
	Thread thr(F, 0);
	thr.tls = threadLocalVars;
	return thr;
}

/* Creates an entry for another thread */
Thread Interpreter::createNewThread(llvm::Function *F, const llvm::GenericValue &arg,
				    int tid, int pid, const llvm::ExecutionContext &SF)
{
	Thread thr(F, arg, tid, pid, SF);
	thr.ECStack.push_back(SF);
	thr.tls = threadLocalVars;
	return thr;
}

Thread Interpreter::createRecoveryThread(int tid)
{
	Thread rec(recoveryRoutine, tid);
	rec.tls = threadLocalVars;
	return rec;
}

/* Returns the (source-code) variable name corresponding to "addr" */
std::string Interpreter::getVarName(const void *addr)
{
	if (isStatic(addr))
		return varNames[static_cast<int>(Storage::ST_Static)][addr];
	if (isStack(addr))
		return varNames[static_cast<int>(Storage::ST_Automatic)][addr];
	if (isHeap(addr))
		return varNames[static_cast<int>(Storage::ST_Heap)][addr];
	return "";
}

bool Interpreter::isInternal(const void *addr)
{
	return alloctor.isInternal(addr);
}

bool Interpreter::isStatic(const void *addr)
{
	return varNames[static_cast<int>(Storage::ST_Static)].count(addr);
}

bool Interpreter::isStack(const void *addr)
{
	return alloctor.hasStorage(addr, Storage::ST_Automatic);
}

bool Interpreter::isHeap(const void *addr)
{
	return alloctor.hasStorage(addr, Storage::ST_Heap);
}

bool Interpreter::isDynamic(const void *addr)
{
	return isStack(addr) || isHeap(addr);
}

bool Interpreter::isShared(const void *addr)
{
	return isStatic(addr) || isDynamic(addr);
}

/* Returns a fresh address to be used from the interpreter */
void *Interpreter::getFreshAddr(unsigned int size, Storage s, AddressSpace spc)
{
	return alloctor.allocate(size, s, spc);
}

void Interpreter::trackAlloca(const void *addr, unsigned int size,
			      Storage s, AddressSpace spc)
{
	/* We cannot call updateVarNameInfo just yet, since we might be simply
	 * restoring a prefix, and cannot get the respective Value. The naming
	 * information will be updated from the interpreter */
	alloctor.track(addr, size, s, spc);
	return;
}

void Interpreter::untrackAlloca(const void *addr, unsigned int size,
				Storage s, AddressSpace spc)
{
	alloctor.untrack(addr, size, s, spc);
	eraseVarNameInfo((char *) addr, size, s, spc);
	return;
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
	int fd = my_find_first_unset(FI.fds);
#else
	int fd = FI.fds.find_first_unset();
#endif

	/* If no available descriptor found, grow fds and try again */
	if (fd == -1) {
		FI.fds.resize(2 * FI.fds.size() + 1);
		FI.fdToFile.grow(FI.fds.size());
		return getFreshFd();
	}

	/* Otherwise, mark the file descriptor as used */
	markFdAsUsed(fd);
	return fd;
}

void Interpreter::markFdAsUsed(int fd)
{
	FI.fds.set(fd);
}

void Interpreter::reclaimUnusedFd(int fd)
{
	FI.fds.reset(fd);
}

void collectUnnamedGlobalAddress(Value *v, char *ptr, unsigned int typeSize,
				 std::unordered_map<const void *, std::string> &vars)
{
	BUG_ON(!isa<GlobalVariable>(v));
	auto gv = static_cast<GlobalVariable *>(v);

	/* Exit if it is a private variable; it is not accessible in the program */
	if (gv->hasPrivateLinkage())
		return;

	/* Otherwise, collect the addresses anyway and use a default name */
	WARN_ONCE("name-info", ("Inadequate naming info for variable " +
				v->getName() + ".\nPlease submit a bug report to "
				PACKAGE_BUGREPORT "\n"));
	for (auto i = 0u; i < typeSize; i++) {
		vars[ptr + i] = v->getName().str();
	}
	return;
}

void updateVarInfoHelper(char *ptr, unsigned int typeSize,
			 std::unordered_map<const void *, std::string> &vars,
			 VariableInfo::NameInfo &vi, const std::string &baseName)
{
	/* If there are no info for the variable, just use the base name.
	 * (Except for internal types, this should normally be handled by the caller) */
	if (vi.empty()) {
		for (auto j = 0u; j < typeSize; j++)
			vars[ptr + j] = baseName;
		return;
	}

	/* If there is no name for the beginning of the block, use a default one */
	if (vi[0].first != 0) {
		WARN_ONCE("name-info", ("Inadequate naming info for variable " +
					baseName + ".\nPlease submit a bug report to "
					PACKAGE_BUGREPORT "\n"));
		for (auto j = 0u; j < vi[0].first; j++)
			vars[ptr + j] = baseName;
	}
	for (auto i = 0u; i < vi.size() - 1; i++) {
		for (auto j = 0u; j < vi[i + 1].first - vi[i].first; j++)
			vars[ptr + vi[i].first + j] = baseName + vi[i].second;
	}
	auto &last = vi.back();
	for (auto j = 0u; j < typeSize - last.first; j++)
		vars[ptr + last.first + j] = baseName + last.second;
	return;
}

void Interpreter::updateUserTypedVarName(char *ptr, unsigned int typeSize, Storage s,
					 Value *v, const std::string &prefix,
					 const std::string &internal)
{
	auto &vars = varNames[static_cast<int>(s)];
	auto &vi = (s == Storage::ST_Automatic) ? VI.localInfo[v] : VI.globalInfo[v];

	if (vi.empty()) {
		/* If it is not a local value, then we should collect the address
		 * anyway, since globalVars will be used to check whether some
		 * access accesses a global variable */
		if (s == Storage::ST_Static)
			collectUnnamedGlobalAddress(v, ptr, typeSize, vars);
		return;
	}
	updateVarInfoHelper(ptr, typeSize, vars, vi, v->getName().str());
	return;
}

void Interpreter::updateUserUntypedVarName(char *ptr, unsigned int typeSize, Storage s,
					   Value *v, const std::string &prefix,
					   const std::string &internal)
{
	/* FIXME: Does nothing now */
	return;
}

void Interpreter::updateInternalVarName(char *ptr, unsigned int typeSize, Storage s,
					Value *v, const std::string &prefix,
					const std::string &internal)
{
	auto &vars = varNames[static_cast<int>(s)];
	auto &vi = VI.internalInfo[internal]; /* should be the name for internals */

	updateVarInfoHelper(ptr, typeSize, vars, vi, prefix);
	return;
}

void Interpreter::updateVarNameInfo(char *ptr, unsigned int typeSize, Storage s,
				    AddressSpace spc, Value *v,
				    const std::string &prefix, const std::string &extra)
{
	if (spc == AddressSpace::AS_Internal) {
		updateInternalVarName(ptr, typeSize, s, v, prefix, extra);
		return;
	}

	BUG_ON(spc != AddressSpace::AS_User);
	switch (s) {
	case Storage::ST_Static:
	case Storage::ST_Automatic:
		updateUserTypedVarName(ptr, typeSize, s, v, prefix, extra);
		return;
	case Storage::ST_Heap:
		updateUserUntypedVarName(ptr, typeSize, s, v, prefix, extra);
		return;
	default:
		BUG();
	}
	return;
}

void Interpreter::eraseVarNameInfo(char *addr, unsigned int size, Storage s, AddressSpace spc)
{
	for (auto i = 0u; i < size; i++)
		varNames[static_cast<int>(s)].erase(addr + i);
	return;
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
	char *allocBegin = nullptr;
	for (auto &v : M->getGlobalList()) {
		char *ptr = static_cast<char *>(GVTOP(getConstantValue(&v)));
		unsigned int typeSize =
		        TD.getTypeAllocSize(v.getType()->getElementType());

		/* The allocation pool will point just after the static address
		 * WARNING: This will not track the allocated space. Do that
		 * either below, or elsewhere for internal variables */
		if (!allocBegin || ptr > allocBegin)
			allocBegin = ptr + typeSize;

		/* Record whether this is a thread local variable or not */
		if (v.isThreadLocal()) {
			for (auto i = 0u; i < typeSize; i++)
				threadLocalVars[ptr + i] = getConstantValue(v.getInitializer());
			continue;
		}

		/* Update the name for this global. We cheat a bit since we will use this
		 * to indicate whether this is an allocated static address (see isStatic()) */
		if (GET_GV_ADDRESS_SPACE(v) != 42) /* exclude internal variables */
			updateVarNameInfo(ptr, typeSize, Storage::ST_Static, AddressSpace::AS_User, &v);
	}
	/* The allocator will start giving out addresses greater than the maximum static address */
	if (allocBegin)
		alloctor.initPoolAddress(allocBegin);
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
	/* Setup config options first */
	FI.fds = llvm::BitVector(20);
	FI.fdToFile.grow(FI.fds.size());
	FI.blockSize = userConf->blockSize;
	FI.maxFileSize = userConf->maxFileSize;
	FI.journalData = userConf->journalData;
	FI.delalloc = FI.journalData == JournalDataFS::ordered && !userConf->disableDelalloc;

	auto *inodeVar = M->getGlobalVariable("__genmc_dir_inode");
	auto *fileVar = M->getGlobalVariable("__genmc_dummy_file");

	/* unistd.h not included -- not dealing with fs stuff */
	if (!inodeVar || !fileVar)
		return;

	FI.inodeTyp = dyn_cast<StructType>(inodeVar->getType()->getElementType());
	FI.fileTyp = dyn_cast<StructType>(fileVar->getType()->getElementType());
	BUG_ON(!FI.inodeTyp || !FI.fileTyp);

	/* Initialize the directory's inode -- assume that the first field is int
	 * We track this here to have custom naming info */
	unsigned int inodeSize = getTypeSize(FI.inodeTyp);
	FI.dirInode = static_cast<char *>(GVTOP(getConstantValue(inodeVar)));
	trackAlloca(FI.dirInode, inodeSize, Storage::ST_Heap, AddressSpace::AS_Internal);

	Type *intTyp = FI.inodeTyp->getElementType(0);
	unsigned int intSize = getTypeSize(intTyp);
	updateVarNameInfo((char *) FI.dirInode, intSize, Storage::ST_Heap,
			  AddressSpace::AS_Internal, nullptr, "__dir_inode.lock", "dir_inode_lock");
	updateVarNameInfo((char *) FI.dirInode + 2 * intSize, intSize, Storage::ST_Heap,
			  AddressSpace::AS_Internal, nullptr, "__dir_inode.i_transaction", "dir_inode_itrans");

	unsigned int count = 0;
	unsigned int intPtrSize = getTypeSize(intTyp->getPointerTo());
	auto *SL = TD.getStructLayout(FI.inodeTyp);
	for (auto &fname : FI.nameToInodeAddr) {
		auto *addr = (char *) FI.dirInode + SL->getElementOffset(4) + count * intPtrSize;
		fname.second = addr;
		updateVarNameInfo((char *) addr, intPtrSize, Storage::ST_Heap, AddressSpace::AS_Internal,
				  nullptr, "__dir_inode.addr[" + fname.first + "]", "dir_inode_locs");
		++count;
	}
	return;
}

const DepInfo *Interpreter::getAddrPoDeps(unsigned int tid)
{
	if (!depTracker)
		return nullptr;
	return depTracker->getAddrPoDeps(tid);
}

const DepInfo *Interpreter::getDataDeps(unsigned int tid, Value *i)
{
	if (!depTracker)
		return nullptr;
	return depTracker->getDataDeps(tid, i);
}

const DepInfo *Interpreter::getCtrlDeps(unsigned int tid)
{
	if (!depTracker)
		return nullptr;
	return depTracker->getCtrlDeps(tid);
}

const DepInfo *Interpreter::getCurrentAddrDeps() const
{
	if (!depTracker)
		return nullptr;
	return depTracker->getCurrentAddrDeps();
}

const DepInfo *Interpreter::getCurrentDataDeps() const
{
	if (!depTracker)
		return nullptr;
	return depTracker->getCurrentDataDeps();
}

const DepInfo *Interpreter::getCurrentCtrlDeps() const
{
	if (!depTracker)
		return nullptr;
	return depTracker->getCurrentCtrlDeps();
}

const DepInfo *Interpreter::getCurrentAddrPoDeps() const
{
	if (!depTracker)
		return nullptr;
	return depTracker->getCurrentAddrPoDeps();
}

const DepInfo *Interpreter::getCurrentCasDeps() const
{
	if (!depTracker)
		return nullptr;
	return depTracker->getCurrentCasDeps();
}

void Interpreter::setCurrentDeps(const DepInfo *addr, const DepInfo *data,
				 const DepInfo *ctrl, const DepInfo *addrPo,
				 const DepInfo *cas)
{
	if (!depTracker)
		return;

	depTracker->setCurrentAddrDeps(addr);
	depTracker->setCurrentDataDeps(data);
	depTracker->setCurrentCtrlDeps(ctrl);
	depTracker->setCurrentAddrPoDeps(addrPo);
	depTracker->setCurrentCasDeps(cas);
}

void Interpreter::updateDataDeps(unsigned int tid, Value *dst, Value *src)
{
	if (depTracker)
		depTracker->updateDataDeps(tid, dst, src);
}

void Interpreter::updateDataDeps(unsigned int tid, Value *dst, const DepInfo *e)
{
	if (depTracker)
		depTracker->updateDataDeps(tid, dst, *e);
}

void Interpreter::updateDataDeps(unsigned int tid, Value *dst, Event e)
{
	if (depTracker)
		depTracker->updateDataDeps(tid, dst, e);
}

void Interpreter::updateAddrPoDeps(unsigned int tid, Value *src)
{
	if (!depTracker)
		return;

	depTracker->updateAddrPoDeps(tid, src);
	depTracker->setCurrentAddrPoDeps(getAddrPoDeps(tid));
}

void Interpreter::updateCtrlDeps(unsigned int tid, Value *src)
{
	if (!depTracker)
		return;

	depTracker->updateCtrlDeps(tid, src);
	depTracker->setCurrentCtrlDeps(getCtrlDeps(tid));
}

void Interpreter::updateFunArgDeps(unsigned int tid, Function *fun)
{
	if (!depTracker)
		return;

	ExecutionContext &SF = ECStack().back();
	auto name = fun->getName().str();

	/* First handle special cases and then normal function calls */
	bool isInternal = internalFunNames.count(name);
	if (isInternal) {
		auto iFunCode = internalFunNames.at(name);
		if (iFunCode == InternalFunctions::FN_Assume) {
			/* We have ctrl dependency on the argument of an assume() */
			for (auto i = SF.Caller.arg_begin(),
				     e = SF.Caller.arg_end(); i != e; ++i) {
				updateCtrlDeps(tid, *i);
			}
		} else if (iFunCode == InternalFunctions::FN_MutexLock ||
			   iFunCode == InternalFunctions::FN_MutexUnlock ||
			   iFunCode == InternalFunctions::FN_MutexTrylock) {
			/* We have addr dependency on the argument of mutex calls */
			setCurrentDeps(getDataDeps(tid, *SF.Caller.arg_begin()),
				       nullptr, getCtrlDeps(tid),
				       getAddrPoDeps(tid), nullptr);
		}
	} else {
		/* The parameters of the function called get the data
		 * dependencies of the actual arguments */
		auto ai = fun->arg_begin();
		for (auto ci = SF.Caller.arg_begin(),
			     ce = SF.Caller.arg_end(); ci != ce; ++ci, ++ai) {
			updateDataDeps(tid, &*ai, &*ci->get());
		}
	}
	return;
}

void Interpreter::clearDeps(unsigned int tid)
{
	if (depTracker)
		depTracker->clearDeps(tid);
}


//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(Module *M, VariableInfo &&VI, FsInfo &&FI,
			 GenMCDriver *driver, const Config *userConf)
#ifdef LLVM_EXECUTIONENGINE_MODULE_UNIQUE_PTR
  : ExecutionEngine(std::unique_ptr<Module>(M)),
#else
  : ExecutionEngine(M),
#endif
    TD(M), VI(std::move(VI)), FI(std::move(FI)), driver(driver) {

  memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
#ifdef LLVM_EXECUTIONENGINE_DATALAYOUT_PTR
  setDataLayout(&TD);
#endif
  // Initialize the "backend"
  initializeExecutionEngine();
  initializeExternalFunctions();
  emitGlobals();

  varNames.grow(static_cast<int>(Storage::ST_StorageLast));
  collectStaticAddresses(M);

  /* Set up a dependency tracker if the model requires it */
  if (userConf->isDepTrackingModel)
	  depTracker = LLVM_MAKE_UNIQUE<IMMDepTracker>();

  /* Set up the system error policy */
  setupErrorPolicy(M, userConf);

  /* Also run a recovery routine if it is required to do so */
  checkPersistency = userConf->persevere;
  recoveryRoutine = M->getFunction("__VERIFIER_recovery_routine");
  setupFsInfo(M, userConf);

  IL = new IntrinsicLowering(TD);
}

Interpreter::~Interpreter() {
  delete IL;
}

void Interpreter::runAtExitHandlers () {
  while (!AtExitHandlers.empty()) {
    callFunction(AtExitHandlers.back(), std::vector<GenericValue>());
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
  callFunction(F, ActualArgs);

  // Start executing the function.
  run();

  return ExitValue;
}
