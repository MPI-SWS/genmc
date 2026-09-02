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

#include "EscapeCheckerPass.hpp"
#include "genmc/ADT/VSet.hpp"
#include "passes/InternalFunctions.hpp"
#include "passes/LLVMUtils.hpp"
#include "passes/Transforms/CallInfoCollectionPass.hpp"

#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/Casting.h>

#include <algorithm>
#include <ranges>
#include <vector>

using namespace llvm;

/* EscapeInfo impl */

auto EscapeAnalysisResult::escapes(const Value *v) const -> bool
{
	auto it = escapePoints.find(v);
	return it == escapePoints.cend() ? false : !it->second.empty();
}

auto EscapeAnalysisResult::escapesAfter(const Value *val, const Instruction *inst,
					DominatorTree &DT) const -> bool
{
	auto it = escapePoints.find(val);
	return it == escapePoints.cend()
		       ? true
		       : std::all_of(it->second.begin(), it->second.end(),
				     [&](const Instruction *escapePoint) {
					     return DT.dominates(inst, escapePoint);
				     });
}

auto EscapeAnalysisResult::writesDynamicMemory(Value *val /*, AliasAnalysis &AA */) const
	-> Instruction *
{
	auto *ptr = dyn_cast<Instruction>(val);
	if (!ptr)
		return nullptr;

	auto *src = stripCastsGEPs(ptr);
	auto allocIt = std::find(alloc_begin(), alloc_end(), src);
	return allocIt == alloc_end() ? nullptr : *allocIt;

	/*
	 * We could also do some basic alias analysis like below, but
	 * this is not accurate enough for our purposes
	 */

	// /* Escape through parameter arguments? */
	// if (std::any_of(F->arg_begin(), F->arg_end(), [&](const Argument &arg){
	// 			return isa<PointerType>(arg.getType()) && isa<Value>(arg) &&
	// 				!AA.isNoAlias(ptr, dyn_cast<Value>(&arg));
	// 		}))
	// 	return true;

	// /* Escape through globals? */
	// return std::any_of(F->getParent()->global_begin(), F->getParent()->global_end(),
	// 		   [&](const GlobalValue &gv){
	// 			   return isa<Value>(gv) && !AA.isNoAlias(ptr,
	// dyn_cast<Value>(&gv));
	// 		   });
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void EscapeAnalysisResult::calculate(Function &F, const CallAnalysisResult &CAR)
{
	const auto &allocFuns = CAR.alloc;

	/* First, collect all allocations */
	for (auto &i : instructions(F)) {
		if (isAlloc(&i, &allocFuns))
			allocs_.insert(&i);
	}

	/* Then, process them one by one (we need the list fixed before processing) */
	for (auto *i : allocs_) {
		std::vector<Instruction *> worklist = {i};
		VSet<const Instruction *> visited;

		while (!worklist.empty()) {
			auto *current = worklist.back();
			worklist.pop_back();

			if (visited.contains(current))
				continue;
			visited.insert(current);

			for (auto *usr : current->users()) {
				if (auto *inst = dyn_cast<Instruction>(usr))
					worklist.push_back(inst);

				/* Some special function calls first */
				if (auto *ci = dyn_cast<CallInst>(usr)) {
					auto name = getCalledFunOrStripValName(*ci);
					if (isCleanInternalFunction(name))
						continue;
				}

				/* return/call/invoke are escape points */
				if (isa<ReturnInst>(usr) || isa<CallInst>(usr) ||
				    isa<InvokeInst>(usr)) {
					escapePoints[i].push_back(dyn_cast<Instruction>(usr));
					continue;
				}

				/* We have to be careful with stores: we only allow stores
				 * to dynamically allocated memory */
				if (auto *si = dyn_cast<StoreInst>(usr)) {
					if (!writesDynamicMemory(si->getPointerOperand()))
						escapePoints[i].push_back(si);
				}
				if (auto *casi = dyn_cast<AtomicCmpXchgInst>(usr)) {
					if (!writesDynamicMemory(casi->getPointerOperand()))
						escapePoints[i].push_back(casi);
				}
				if (auto *faii = dyn_cast<AtomicRMWInst>(usr)) {
					if (!writesDynamicMemory(faii->getPointerOperand()))
						escapePoints[i].push_back(faii);
				}
				/* We also consider loads as escape points if configured to do so
				 * (e.g., to catch for-loop counters) */
				if (canLoadsEscape()) {
					if (auto *li = dyn_cast<LoadInst>(usr))
						escapePoints[i].push_back(li);
				}
			}
		}
	}

	/* Remove duplicates */
	for (auto &kv : escapePoints) {
		std::ranges::sort(kv.second);
		auto dups = std::ranges::unique(kv.second);
		kv.second.erase(dups.begin(), kv.second.end());
	}
}

void EscapeAnalysisResult::print(raw_ostream &out) const
{
	for (const auto &kv : escapePoints) {
		out << kv.first->getName() << " has " << kv.second.size() << " escape point(s): [";
		for (const auto &escapePoint : kv.second)
			out << " " << *escapePoint << " ";
		out << "]\n";
	}
}

/*  Analysis impl */

auto EscapeAnalysis::run(Module &M, ModuleAnalysisManager &MAM) -> Result
{
	[[maybe_unused]] auto &FAM =
		MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
	auto &CAR = MAM.getResult<CallAnalysis>(M);
	for (auto &F : M | std::views::filter([&](auto &F) { return !F.isDeclaration(); })) {
		result_[&F].calculate(F, CAR);
	}
	return result_;
}

auto EscapeCheckerPass::run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses
{
	EAR_ = MAM.getResult<EscapeAnalysis>(M);
	return PreservedAnalyses::all();
}
