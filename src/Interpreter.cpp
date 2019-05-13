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
ExecutionEngine *Interpreter::create(Module *M, GenMCDriver *driver,
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

  return new Interpreter(M, driver);
}

/* Thread::seed is ODR-used -- we need to provide a definition (C++14) */
constexpr int Thread::seed;

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

	/*
	 * Free all allocated memory, and no longer track the stack addresses
	 * for this execution
	 */
	for (auto mem : stackMem)
		free(mem);
	stackMem.clear();
	stackAllocas.clear();
}

std::vector<ExecutionContext> &Interpreter::ECStack()
{
	return getCurThr().ECStack;
}

#ifdef LLVM_EXECUTIONENGINE_DATALAYOUT_PTR
# define GET_TYPE_ALLOC_SIZE(M, x)		\
	(M)->getDataLayout()->getTypeAllocSize((x))
#else
# define GET_TYPE_ALLOC_SIZE(M, x)		\
	(M)->getDataLayout().getTypeAllocSize((x))
#endif

void Interpreter::collectGPs(Module *M, void *ptr, llvm::Type *typ)
{
	if (!(typ->isPointerTy() || typ->isAggregateType() || typ->isVectorTy()))
		return;

	unsigned int offset = 0;
	if (ArrayType *AT = dyn_cast<ArrayType>(typ)) {
		unsigned int elemSize = GET_TYPE_ALLOC_SIZE(M, AT->getElementType());
		for (auto i = 0u; i < AT->getNumElements(); i++) {
			collectGPs(M, (char *) ptr + offset, AT->getElementType());
			offset += elemSize;
		}
	} else if (StructType *ST = dyn_cast<StructType>(typ)) {
		for (auto it = ST->element_begin(); it != ST->element_end(); ++it) {
			unsigned int elemSize = GET_TYPE_ALLOC_SIZE(M, *it);
			collectGPs(M, (char *) ptr + offset, *it);
			offset += elemSize;
		}
	}
	return;
}

void Interpreter::storeGlobals(Module *M)
{
	/* Collect all global and thread-local variables */
	for (auto &v : M->getGlobalList()) {
		char *ptr = static_cast<char *>(GVTOP(getConstantValue(&v)));
		unsigned int typeSize = GET_TYPE_ALLOC_SIZE(M, v.getType()->getElementType());

		/* Record whether this is a thread local variable or not */
		for (auto i = 0u; i < typeSize; i++) {
			if (v.isThreadLocal())
				threadLocalVars[ptr + i] = getConstantValue(v.getInitializer());
			else
				globalVars.push_back(ptr + i);
		}

		/* Check whether it is a global pointer */
		collectGPs(M, ptr, v.getType()->getElementType());

		/* Store the variable's name for printing and debugging */
		if (ArrayType *AT = dyn_cast<ArrayType>(v.getType()->getElementType())) {
			unsigned int elemTypeSize = typeSize / AT->getArrayNumElements();
			for (auto i = 0u, s = 0u; s < typeSize; i++, s += elemTypeSize) {
				std::string name = v.getName().str() + "[" + std::to_string(i) + "]";
				globalVarNames.push_back(std::make_pair(ptr + s, name));
			}
		} else {
			globalVarNames.push_back(std::make_pair(ptr, v.getName()));
		}
	}
	/* We sort all vectors to facilitate searching */
	std::sort(globalVars.begin(), globalVars.end());
	std::unique(globalVars.begin(), globalVars.end());

	std::sort(globalVarNames.begin(), globalVarNames.end());
	std::unique(globalVarNames.begin(), globalVarNames.end());
}

bool Interpreter::isGlobal(const void *addr)
{
	auto gv = std::equal_range(globalVars.begin(), globalVars.end(), addr);
	auto ha = std::equal_range(heapAllocas.begin(), heapAllocas.end(), addr);
	return (gv.first != gv.second) || (ha.first != ha.second);
}

bool Interpreter::isStackAlloca(const void *addr)
{
	auto sa = std::find(stackAllocas.begin(), stackAllocas.end(), addr);
	return sa != stackAllocas.end();
}

bool Interpreter::isHeapAlloca(const void *addr)
{
	auto sa = std::find(heapAllocas.begin(), heapAllocas.end(), addr);
	return sa != stackAllocas.end();
}

std::string Interpreter::getGlobalName(const void *addr)
{
	typedef std::pair<const void *, std::string> namePair;
	namePair loc = std::make_pair(addr, "");
	auto res = std::equal_range(globalVarNames.begin(), globalVarNames.end(), loc,
				    [](namePair kv1, namePair kv2)
				    { return kv1.first < kv2.first; });
	if (res.first != res.second)
		return res.first->second;
	return "";
}

void Interpreter::freeRegion(const void *addr, int size)
{
	heapAllocas.erase(std::remove_if(heapAllocas.begin(), heapAllocas.end(),
					 [&](void *loc)
					 { return loc >= addr &&
						  (char *) loc < (const char *) addr + size; }),
			  heapAllocas.end());
	return;
}

//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(Module *M, GenMCDriver *driver)
#ifdef LLVM_EXECUTIONENGINE_MODULE_UNIQUE_PTR
  : ExecutionEngine(std::unique_ptr<Module>(M)),
#else
  : ExecutionEngine(M),
#endif
    TD(M), driver(driver) {

  memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
#ifdef LLVM_EXECUTIONENGINE_DATALAYOUT_PTR
  setDataLayout(&TD);
#endif
  // Initialize the "backend"
  initializeExecutionEngine();
  initializeExternalFunctions();
  emitGlobals();

  /* Store the addresses of all global variables */
  storeGlobals(M);

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
