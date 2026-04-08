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

#include "FunctionInlinerPass.hpp"
#include "passes/InternalFunctions.hpp"
#include <llvm/ADT/SCCIterator.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace llvm;

/**
 * Do not inline any functions that are (mutually) recursive.
 *
 * Example transforms that are permitted (below only f1 will be inlined):
 * f->f1->f2->f3->f4->f2->f3->f4... => f->f2->f3->f4->f2->f3->f4
 * f->f1->f2->f2->f2->... => main->f2->f2->f2...
 */
static auto isRecursive(CallGraph &CG, Function &F) -> bool
{
	for (auto sccIt = scc_begin(&CG); !sccIt.isAtEnd(); ++sccIt) {
		if (std::ranges::find(*sccIt, CG[&F]) != sccIt->end() && sccIt.hasCycle())
			return true;
	}
	return false;
}

static auto isInlinable(CallGraph &CG, Function &F) -> bool
{
	return !F.isDeclaration() && !isInternalFunction(F.getName().str()) && !isRecursive(CG, F);
}

static auto inlineCall(CallBase *cb) -> bool
{
	llvm::InlineFunctionInfo ifi;

#if LLVM_VERSION_MAJOR >= 11
	return InlineFunction(*cb, ifi).isSuccess();
#else
	return InlineFunction(*cb, ifi);
#endif
}

static auto inlineFunction(Module &M, Function *toInline) -> bool
{
	std::vector<CallBase *> calls;
	for (auto &F : M) {
		if (&F == toInline) /* No need to inline calls to itself */
			continue;

		for (auto &iit : instructions(F)) {
			if (!isa<InvokeInst>(&iit) && !isa<CallInst>(&iit))
				continue;

			auto *cb = dyn_cast<CallBase>(&iit);
			if (cb->getCalledFunction() == toInline)
				calls.push_back(cb);
		}
	}

	auto changed = false;
	for (auto *ci : calls) {
		changed |= inlineCall(ci);
	}
	return changed;
}

auto FunctionInlinerPass::run(Module &M, ModuleAnalysisManager &AM) -> PreservedAnalyses
{
	CallGraph CG(M);

	auto changed = false;
	for (auto &F : M) {
		/* Don't try on functions with empty bodies, external declarations, GenMCs own
		 * functions and (mutually) recursive functions */
		if (!F.empty() && isInlinable(CG, F))
			changed |= inlineFunction(M, &F);
	}
	return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
