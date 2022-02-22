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

#include "config.h"
#include "EliminateUnusedCodePass.hpp"
#include "Error.hpp"

#include <llvm/ADT/SetVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/Local.h>

using namespace llvm;

void EliminateUnusedCodePass::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.setPreservesCFG();
}

bool isEliminable(Instruction *i)
{
	return !mayBeMemoryDependent(*i) && !i->isTerminator();
}

static bool eliminateUnusedCode(Function &F)
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
		if (!alive.count(&i)) {
			worklist.insert(&i);
			i.dropAllReferences();
		}
	}
	for (auto *i : worklist)
		i->eraseFromParent();

	return !worklist.empty();
}

bool EliminateUnusedCodePass::runOnFunction(Function &F)
{
	return eliminateUnusedCode(F);
}

Pass *createEliminateUnusedCodePass()
{
	auto *p = new EliminateUnusedCodePass();
	return p;
}

char EliminateUnusedCodePass::ID = 42;
static llvm::RegisterPass<EliminateUnusedCodePass> P("elim-unused-code",
						     "Eliminates unused code.");
