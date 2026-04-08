/*
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

#include "RustPrepPass.hpp"
#include "genmc/ADT/VSet.hpp"
#include "passes/InternalFunctions.hpp"
#include "genmc/Support/Error.hpp"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/Regex.h>

using namespace llvm;

/**
 * The LLVM-IR output for Rust will not call the "main"-function the user wrote,
 * but instead the main-function in LLVM-IR sets up Rusts runtime which then calls
 * the user-written "main" function:
 *  main -> std::rt::lang_start -> std::rt::lang_start_internal -> user-written "main"
 * As std::rt::lang_start_internal is empty in the LLVM-IR output (implemented by rustc directly)
 * and we don't require said rust-runtime environment, we replace the call to std::rt::lang_start
 * with a call to the user-written "main" directly.
 */
static auto inlineLangStartCall(Module &M) -> bool
{
	Function *F = M.getFunction("main");
	if (!F) /* Allow for: #![no_main] */
		return false;

	Regex matchRegex("^.+std.+rt.+lang_start.+"); /* Look for calls to std::rt::lang_start */
	for (auto &I : instructions(F)) {
		auto *CI = dyn_cast<CallInst>(&I);
		if (!CI)
			continue;

		Function *calledF = CI->getCalledFunction();
		if (!calledF) /* Skip indirect calls */
			continue;

		if (!matchRegex.match(calledF->getName())) /* Only calls to the specified regex */
			continue;

		IRBuilder<> builder(CI);

		/* Get the "main" function-pointer passed in the argument */
		Value *fp = CI->getArgOperand(0);
		Function *mainF = dyn_cast<Function>(fp);
		VERIFY(mainF);

		/* Create a new call to Rusts "main" directly */
		CallInst *newCall = builder.CreateCall(mainF, {});

		/* std::rt::lang_start returns an i64 whereas Rusts "main" returns void.
		   We replace uses of lang_starts return-value with a 0 as a default. */
		Type *retType = CI->getType();
		Value *defaultVal = ConstantInt::get(retType, 0);
		CI->replaceAllUsesWith(defaultVal);

		/* Remove the original call to std::rt::lang_start */
		CI->eraseFromParent();
		return true;
	}

	return false;
}

/**
 * __VERIFIER_* internal functions are not defined as external functions in Rust to avoid linking
 * errors when using cargo, which links automatically.
 * We remove their function bodies here instead.
 */
static auto removeInternalFunctionsBody(Module &M) -> bool
{
	bool modified = false;
	for (Function &F : M) {
		if (F.isDeclaration())
			continue;

		if (isInternalFunction(F.getName().str())) {
			F.deleteBody();
			modified = true;
		}
	}

	return modified;
}

/**
 * Newer Rust versions will deliberately add an external global variable
 * "__rust_no_alloc_shim_is_unstable" if the compiler detects that a different STD is being used (as
 * is in our case) to force the user to link manually and recognize they may not have a memory
 * allocator. https://github.com/rust-lang/rust/pull/86844 We do, infact, still use Rusts STD behind
 * a wrapper with the standard memory allocator, which would be instantiated by __rust_alloc, which
 * we reroute to genmc__rust_alloc in this same pass.
 */
static auto removeAllocShim(Module &M) -> bool
{
	GlobalVariable *GV = M.getGlobalVariable("__rust_no_alloc_shim_is_unstable");
	if (!GV) { // Not required in older Rust-versions
		return false;
	}

	/* Init __rust_no_alloc_shim_is_unstable */
	Constant *zero = ConstantInt::get(Type::getInt8Ty(M.getContext()), 0);
	GV->setInitializer(zero);
	GV->setLinkage(GlobalValue::LinkageTypes::PrivateLinkage);
	GV->setAlignment(Align(16));

	/* Collect the loads to replace, their values are unused */
	SmallVector<Instruction *, 8> loadsToReplace;
	for (User *U : GV->users()) {
		if (auto *LI = dyn_cast<LoadInst>(U)) {
			loadsToReplace.push_back(LI);
		}
	}

	/* Remove the loads */
	for (Instruction *LI : loadsToReplace) {
		LI->replaceAllUsesWith(zero);
		LI->eraseFromParent();
	}

	/* Remove __rust_no_alloc_shim_is_unstable if not used otherwise */
	if (GV->use_empty()) {
		GV->eraseFromParent();
	}
	return true;
}

/**
 * Replace all calls to __rust_alloc and co. with calls to genmc__rust_alloc & co.
 * instead.
 */
static auto replaceRustAllocCalls(Function &F) -> bool
{
	bool modified = false;

	/* Replacement functions for memory allocation */
	Module *M = F.getParent();
	Function *genmc__rust_alloc = M->getFunction("genmc__rust_alloc");
	Function *genmc__rust_dealloc = M->getFunction("genmc__rust_dealloc");
	Function *genmc__rust_realloc = M->getFunction("genmc__rust_realloc");
	Function *genmc__rust_alloc_zeroed = M->getFunction("genmc__rust_alloc_zeroed");

	/* Fail if genmc-std wasn't imported correctly */
	VERIFY(genmc__rust_alloc && genmc__rust_dealloc && genmc__rust_realloc &&
	       genmc__rust_alloc_zeroed);

	/* Replace each call to __rust_alloc => genmc__rust_alloc etc. */
	for (auto &I : instructions(F)) {
		auto *CI = dyn_cast<CallInst>(&I);
		if (!CI)
			continue;

		Function *calledF = CI->getCalledFunction();
		if (!calledF) // Skip indirect function calls
			continue;

		std::string fName = calledF->getGlobalIdentifier();
		if ("__rust_alloc" == fName) {
			CI->setCalledFunction(genmc__rust_alloc);
			modified = true;
		} else if ("__rust_dealloc" == fName) {
			CI->setCalledFunction(genmc__rust_dealloc);
			modified = true;
		} else if ("__rust_realloc" == fName) {
			CI->setCalledFunction(genmc__rust_realloc);
			modified = true;
		} else if ("__rust_alloc_zeroed" == fName) {
			CI->setCalledFunction(genmc__rust_alloc_zeroed);
			modified = true;
		}
	}

	return modified;
}

/**
 * Use BFS to collect all dependents on I in a list, closest to furthest in terms
 * of usage, i.e. instructions on the back of the list have no more instructions
 * using them and themselves use instructions in the further in the front of the list.
 */
const auto collectDependents(Instruction *I, VSet<Instruction *> &toRemove)
{
	std::vector<Instruction *> worklist;
	worklist.push_back(I);

	while (!worklist.empty()) {
		Instruction *current = worklist.back();
		worklist.pop_back();

		for (User *U : current->users()) {
			Instruction *userI = dyn_cast<Instruction>(U);
			if (!userI)
				continue;

			if (toRemove.insert(userI).second) {
				worklist.push_back(userI);
			}
		}
	}
}

/**
 * We remove all *.dbg.spill UIDs generated for the debugger as they're not needed, as well as
 * all instructions that depend on them as all of those instructions are purely for
 * debugging purposes. We don't disable debugging in rustc completely as to still have information
 * for later presentation to the user.
 */
static auto cleanDbgSpill(Function &F) -> bool
{
	SmallVector<Instruction *, 8> toRemove;
	bool modified = false;

	/* Collect *.dbg.spill instructions */
	for (Instruction &I : instructions(F)) {
		if (!I.hasName())
			continue;

		StringRef name = I.getName();
		if (name.contains(".dbg.spill"))
			toRemove.push_back(&I);
	}

	for (Instruction *i : toRemove) {
		/* Collect a list of all dependents of I using BFS,
		   i.e. last in list is furthest away and has no more
		   instructions using it.
		*/
		VSet<Instruction *> toRemoveDependents;
		toRemoveDependents.insert(i);
		collectDependents(i, toRemoveDependents);

		/* Remove in reverse order for safety */
		for (auto it = toRemoveDependents.rbegin(); it != toRemoveDependents.rend(); ++it) {
			Instruction *I = *it;
			I->eraseFromParent();
		}
	}

	return modified;
}

PreservedAnalyses RustPrepPass::run(Module &M, ModuleAnalysisManager &AM)
{
	bool modified = false;
	modified |= inlineLangStartCall(M);
	modified |= removeInternalFunctionsBody(M);

	for (Function &F : M) {
		if (F.isDeclaration())
			continue;
		modified |= replaceRustAllocCalls(F);
		modified |= cleanDbgSpill(F);
	}

	modified |= removeAllocShim(M);

	return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
