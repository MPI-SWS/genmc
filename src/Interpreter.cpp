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
ExecutionEngine *Interpreter::create(Module *M, VariableInfo &&VI,
				     GenMCDriver *driver,
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

  return new Interpreter(M, std::move(VI), driver);
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
	}
}

/* Creates an entry for the main() function */
Thread Interpreter::createMainThread(llvm::Function *F)
{
	Thread thr(F, 0);
	thr.tls = threadLocalVars;
	return thr;
}

/* Creates an entry for another thread */
Thread Interpreter::createNewThread(llvm::Function *F, int tid, int pid,
				    const llvm::ExecutionContext &SF)
{
	Thread thr(F, tid, pid, SF);
	thr.ECStack.push_back(SF);
	thr.tls = threadLocalVars;
	return thr;
}

/* Returns true if "addr" points to global memory (static or heap) */
bool Interpreter::isGlobal(const void *addr)
{
	auto ha = std::equal_range(heapAllocas.begin(), heapAllocas.end(), addr);
	return (globalVars.count(addr) || (ha.first != ha.second));
}

/* Returns true if "addr" points to the stack */
bool Interpreter::isStackAlloca(const void *addr)
{
	return stackAllocas.count(addr);
}

/* Returns ture if "addr" points to heap-allocated memory */
bool Interpreter::isHeapAlloca(const void *addr)
{
	return heapAllocas.count(addr);
}

/* Returns the (source-code) variable name corresponding to "addr" */
std::string Interpreter::getVarName(const void *addr)
{
	if (isStackAlloca(addr))
		return stackVars[addr];
	if (isGlobal(addr))
		return globalVars[addr];
	return "";
}

/* Returns a fresh address to be used from the interpreter */
void *Interpreter::getFreshAddr(unsigned int size, bool isLocal /* false */)
{
	char *newAddr = allocRangeBegin; /* Fetch the next from the pool */
	allocRangeBegin += size;

	/* Track the allocated space */
	if (isLocal) {
		for (auto i = 0u; i < size; i++)
			stackAllocas.insert(newAddr + i);
		/* The name information will be updated after control
		 * returns to the interpreter */
	} else {
		for (auto i = 0u; i < size; i++)
			heapAllocas.insert(newAddr + i);
	}
	return newAddr;
}

/* Stops tracking of a memory allocation after deletion from the graph */
void Interpreter::deallocateAddr(const void *addr, unsigned int size, bool isLocal /* false */)
{
	if (isLocal) {
		for (auto i = 0u; i < size; i++)
			stackAllocas.erase((char *) addr + i);
		for (auto i = 0u; i < size; i++)
			stackVars.erase((char *) addr + i);
	} else {
		for (auto i = 0u; i < size; i++)
			heapAllocas.erase((char *) addr + i);
	}
	return;
}

/* Updates the name corresponding to an address based on the collected VariableInfo */
void Interpreter::updateVarNameInfo(Value *v, char *ptr, unsigned int typeSize,
				    bool isLocal /* false */)
{
	auto &vars = (isLocal) ? stackVars : globalVars;
	auto &vi = (isLocal) ? VI.localInfo[v] : VI.globalInfo[v];

	if (vi.empty())
		return;

	for (auto i = 0u; i < vi.size() - 1; i++) {
		for (auto j = 0u; j < vi[i + 1].first - vi[i].first; j++)
			vars[ptr + vi[i].first + j] = vi[i].second;
	}
	auto &last = vi.back();
	for (auto j = 0u; j < typeSize - last.first; j++)
		vars[ptr + last.first + j] = last.second;
}

/* Updates the names for all global variables, and calculates the
 * starting address of the allocation pool */
void Interpreter::collectGlobalAddresses(Module *M)
{
	/* Collect all global and thread-local variables */
	for (auto &v : M->getGlobalList()) {
		char *ptr = static_cast<char *>(GVTOP(getConstantValue(&v)));
		unsigned int typeSize =
		        TD.getTypeAllocSize(v.getType()->getElementType());

		/* The allocation pool will point just after the static address */
		if (!allocRangeBegin || ptr > allocRangeBegin)
			allocRangeBegin = ptr + typeSize;

		/* Record whether this is a thread local variable or not */
		if (v.isThreadLocal()) {
			for (auto i = 0u; i < typeSize; i++)
				threadLocalVars[ptr + i] = getConstantValue(v.getInitializer());
			continue;
		}

		/* Update the name for this global */
		updateVarNameInfo(&v, ptr, typeSize);
	}
	/* If there are no global variables, pick a random address for the pool */
	if (!allocRangeBegin)
		allocRangeBegin = (char *) 0x25031821;
}

//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(Module *M, VariableInfo &&VI, GenMCDriver *driver)
#ifdef LLVM_EXECUTIONENGINE_MODULE_UNIQUE_PTR
  : ExecutionEngine(std::unique_ptr<Module>(M)),
#else
  : ExecutionEngine(M),
#endif
    TD(M), VI(std::move(VI)), driver(driver) {

  memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
#ifdef LLVM_EXECUTIONENGINE_DATALAYOUT_PTR
  setDataLayout(&TD);
#endif
  // Initialize the "backend"
  initializeExecutionEngine();
  initializeExternalFunctions();
  emitGlobals();

  collectGlobalAddresses(M);

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
