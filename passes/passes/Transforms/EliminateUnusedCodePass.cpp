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

#include "EliminateUnusedCodePass.hpp"
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Use.h>
#include <llvm/Support/Casting.h>

#include <llvm/ADT/SetVector.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Transforms/Utils/Local.h>

#define MAY_BE_MEM_DEPENDENT(i) mayHaveNonDefUseDependency(i)

using namespace llvm;

static auto isEliminable(Instruction *i) -> bool
{
	return !MAY_BE_MEM_DEPENDENT(*i) && !i->isTerminator();
}

static auto eliminateUnusedCode(Function &F) -> bool
{
	SmallPtrSet<Instruction *, 16> alive;
	SmallSetVector<Instruction *, 16> worklist;

	/* Iterate and collect instructions that are live */
	for (auto &i : instructions(F)) {
		if (!isEliminable(&i)) {
			alive.insert(&i);
			worklist.insert(&i);
		}
	}

	/* Propagate liveness to operands */
	while (!worklist.empty()) {
		auto *i = worklist.pop_back_val();
		for (Use &op : i->operands()) {
			if (auto *inst = dyn_cast<Instruction>(op))
				if (alive.insert(inst).second)
					worklist.insert(inst);
		}
	}

	/* Everything not in ALIVE is dead now */
	for (auto &i : instructions(F)) {
		if (!alive.contains(&i)) {
			worklist.insert(&i);
			i.dropAllReferences();
		}
	}
	for (auto *i : worklist)
		i->eraseFromParent();

	return !worklist.empty();
}

auto EliminateUnusedCodePass::run(Function &F, FunctionAnalysisManager & /*FAM*/)
	-> PreservedAnalyses
{
	return eliminateUnusedCode(F) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
