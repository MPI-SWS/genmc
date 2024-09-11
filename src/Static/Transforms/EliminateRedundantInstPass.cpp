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

#include "EliminateRedundantInstPass.hpp"
#include "Static/LLVMUtils.hpp"
#include "Support/Error.hpp"
#include "config.h"

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
