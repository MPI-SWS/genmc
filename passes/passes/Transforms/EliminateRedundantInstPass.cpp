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

#include "EliminateRedundantInstPass.hpp"
#include "genmc/ADT/VSet.hpp"

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm;

static auto isRedundantCastPair(const CastInst *ci1, const CastInst *ci2, const DataLayout &DL)
	-> bool
{
	return ci1->getSrcTy() == ci2->getDestTy() &&
	       DL.getTypeAllocSize(ci1->getDestTy()) >= DL.getTypeAllocSize(ci1->getSrcTy());
}

static auto eliminateRedundantInst(Function &F) -> bool
{
	const DataLayout &DL = F.getParent()->getDataLayout();
	auto modified = false;

	VSet<Instruction *> deleted;
	for (auto it = inst_begin(F), ie = inst_end(F); it != ie; ++it) {
		if (deleted.count(&*it))
			continue;
		if (auto *ci = dyn_cast<CastInst>(&*it)) {
			if (auto *csrc = dyn_cast<CastInst>(ci->getOperand(0))) {
				if (isRedundantCastPair(csrc, ci, DL)) { /* A->B->A cast */
					deleted.insert(ci);
					ci->replaceAllUsesWith(csrc->getOperand(0));
					modified = true;
				}
			}
		}
	}
	for (auto *d : deleted)
		d->eraseFromParent();

	return modified;
}

auto EliminateRedundantInstPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	return eliminateRedundantInst(F) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
