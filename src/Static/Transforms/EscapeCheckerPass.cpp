/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include "EscapeCheckerPass.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "Static/LLVMUtils.hpp"
#include "Static/Transforms/CallInfoCollectionPass.hpp"
#include "Support/Error.hpp"

#include <llvm/IR/Dominators.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;

/* EscapeInfo impl */

bool EscapeAnalysisResult::escapes(const Value *v) const
{
	auto it = escapePoints.find(v);
	return it == escapePoints.cend() ? false : !it->second.empty();
}

bool EscapeAnalysisResult::escapesAfter(const Value *a, const Instruction *b,
					DominatorTree &DT) const
{
	auto it = escapePoints.find(a);
	return it == escapePoints.cend()
		       ? true
		       : std::all_of(it->second.begin(), it->second.end(),
				     [&](const Instruction *p) { return DT.dominates(b, p); });
}

auto EscapeAnalysisResult::writesDynamicMemory(Value *val /*, AliasAnalysis &AA */) -> Instruction *
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

			if (visited.count(current))
				continue;
			visited.insert(current);

			for (auto *u : current->users()) {
				if (auto *inst = dyn_cast<Instruction>(u))
					worklist.push_back(inst);

				/* Some special function calls first */
				if (auto *ci = dyn_cast<CallInst>(u)) {
					auto name = getCalledFunOrStripValName(*ci);
					if (isCleanInternalFunction(name))
						continue;
				}

				/* return/call/invoke are escape points */
				if (isa<ReturnInst>(u) || isa<CallInst>(u) || isa<InvokeInst>(u)) {
					escapePoints[i].push_back(dyn_cast<Instruction>(u));
					continue;
				}

				/* We have to be careful with stores: we only allow stores
				 * to dynamically allocated memory */
				if (auto *si = dyn_cast<StoreInst>(u)) {
					if (!writesDynamicMemory(si->getPointerOperand()))
						escapePoints[i].push_back(si);
				}
				if (auto *casi = dyn_cast<AtomicCmpXchgInst>(u)) {
					if (!writesDynamicMemory(casi->getPointerOperand()))
						escapePoints[i].push_back(casi);
				}
				if (auto *faii = dyn_cast<AtomicRMWInst>(u)) {
					if (!writesDynamicMemory(faii->getPointerOperand()))
						escapePoints[i].push_back(faii);
				}
				/* We also consider loads as escape points if configured to do so
				 * (e.g., to catch for-loop counters) */
				if (canLoadsEscape()) {
					if (auto *li = dyn_cast<LoadInst>(u))
						escapePoints[i].push_back(li);
				}
			}
		}
	}

	/* Remove duplicates */
	for (auto &kv : escapePoints) {
		std::sort(kv.second.begin(), kv.second.end());
		auto last = std::unique(kv.second.begin(), kv.second.end());
		kv.second.erase(last, kv.second.end());
	}
}

void EscapeAnalysisResult::print(raw_ostream &s) const
{
	for (auto P : escapePoints) {
		s << P.first->getName() << " has " << P.second.size() << " escape point(s): [";
		for (auto &p : P.second)
			s << " " << *p << " ";
		s << "]\n";
	}
}

/*  Analysis impl */

auto EscapeAnalysis::run(Module &M, ModuleAnalysisManager &MAM) -> Result
{
	auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
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
