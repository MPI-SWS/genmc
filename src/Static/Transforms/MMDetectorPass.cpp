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

#include "MMDetectorPass.hpp"
#include "Static/ModuleInfo.hpp"
#include "config.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace llvm;

auto isSCNAOrdering(AtomicOrdering o) -> bool
{
	return o == AtomicOrdering::SequentiallyConsistent || o == AtomicOrdering::NotAtomic;
}

auto isRANAOrdering(AtomicOrdering o) -> bool
{
	using AO = AtomicOrdering;
	return o == AO::Acquire || o == AO::Release || o == AO::AcquireRelease ||
	       o == AtomicOrdering::NotAtomic;
}

auto MMAnalysis::run(Module &M, ModuleAnalysisManager &MAM) -> MMAnalysis::Result
{
	auto isSC = true;
	auto isRA = true;
	for (auto &F : M) {
		for (auto &I : instructions(F)) {
			if (auto *li = dyn_cast<LoadInst>(&I)) {
				if (!isSCNAOrdering(li->getOrdering())) {
					isSC = false;
				}
				if (!isRANAOrdering(li->getOrdering())) {
					isRA = false;
				}
			} else if (auto *si = dyn_cast<StoreInst>(&I)) {
				if (!isSCNAOrdering(si->getOrdering())) {
					isSC = false;
				}
				if (!isRANAOrdering(si->getOrdering())) {
					isRA = false;
				}
			} else if (auto *casi = dyn_cast<AtomicCmpXchgInst>(&I)) {
				if (!isSCNAOrdering(casi->getSuccessOrdering()) ||
				    !isSCNAOrdering(casi->getFailureOrdering())) {
					isSC = false;
				}
				if (!isRANAOrdering(casi->getSuccessOrdering()) ||
				    !isRANAOrdering(casi->getFailureOrdering())) {
					isRA = false;
				}
			} else if (auto *faii = dyn_cast<AtomicRMWInst>(&I)) {
				if (!isSCNAOrdering(faii->getOrdering())) {
					isSC = false;
				}
				if (!isRANAOrdering(faii->getOrdering())) {
					isRA = false;
				}
			} else if (auto *fi = dyn_cast<FenceInst>(&I)) {
				if (!isSCNAOrdering(fi->getOrdering())) {
					isSC = false;
				}
				if (!isRANAOrdering(fi->getOrdering())) {
					isRA = false;
				}
			}
		}
	}
	if (isSC)
		return {ModelType::SC};
	return isRA ? std::make_optional(ModelType::RA) : std::nullopt;
}

auto MMDetectorPass::run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses
{
	PMI.determinedMM = MAM.getResult<MMAnalysis>(M);
	return PreservedAnalyses::all();
}
