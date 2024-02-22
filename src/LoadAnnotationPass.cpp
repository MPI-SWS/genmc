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

#include "Error.hpp"
#include "InstAnnotator.hpp"
#include "LLVMUtils.hpp"
#include "LoadAnnotationPass.hpp"
#include "SExpr.hpp"
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;

void LoadAnnotationPass::getAnalysisUsage(llvm::AnalysisUsage &au) const { au.setPreservesAll(); }

/* Helper for getSourceLoads() -- see below */
void calcSourceLoads(Instruction *i, VSet<PHINode *> phis, std::vector<Instruction *> &source)
{
	if (!i)
		return;

	/* Don't go past stores or allocas (CASes are OK) */
	if (isa<StoreInst>(i) || isa<AtomicRMWInst>(i) || isa<AllocaInst>(i))
		return;

	/* If we reached a (source) load, collect it */
	if (isa<LoadInst>(i) || isa<AtomicCmpXchgInst>(i)) {
		source.push_back(i);
		return;
	}

	/* If this is an already encountered Φ, don't go into circles */
	if (auto *phi = dyn_cast<PHINode>(i)) {
		if (phis.count(phi))
			return;
		phis.insert(phi);
	}

	/* Otherwise, recurse */
	for (auto &u : i->operands()) {
		if (auto *pi = dyn_cast<Instruction>(u.get())) {
			calcSourceLoads(pi, phis, source);
		} else if (auto *c = dyn_cast<Constant>(u.get())) {
			if (auto *phi = dyn_cast<PHINode>(i)) {
				auto *term = phi->getIncomingBlock(u)->getTerminator();
				if (auto *bi = dyn_cast<BranchInst>(term))
					if (bi->isConditional())
						calcSourceLoads(
							dyn_cast<Instruction>(bi->getCondition()),
							phis, source);
			}
		}
	}
	return;
}

std::vector<Instruction *> LoadAnnotationPass::getSourceLoads(CallInst *assm) const
{
	VSet<PHINode *> phis;
	std::vector<Instruction *> source;

	if (auto *arg = dyn_cast<Instruction>(assm->getOperand(0)))
		calcSourceLoads(arg, phis, source);
	std::sort(source.begin(), source.end());
	source.erase(std::unique(source.begin(), source.end()), source.end());
	return source;
}

std::vector<Instruction *>
LoadAnnotationPass::filterAnnotatableFromSource(CallInst *assm,
						const std::vector<Instruction *> &source) const
{
	std::vector<Instruction *> result;

	/* Collect candidates for which the path to the assume is clear */
	for (auto *li : source) {
		auto assumeFound = false;
		auto loadFound = false;
		auto sideEffects = false;
		foreachInBackPathTo(assm->getParent(), li->getParent(), [&](Instruction &i) {
			/* wait until we find the assume */
			if (!assumeFound) {
				assumeFound |= (dyn_cast<CallInst>(&i) == assm);
				return;
			}
			/* we should stop when the load is found */
			if (assumeFound && !loadFound) {
				sideEffects |= (hasSideEffects(&i) && &i != li); /* also CASes */
				sideEffects |= (isa<LoadInst>(&i) && &i != li);
			}
			if (!loadFound) {
				loadFound |= (&i == li);
				if (loadFound) {
					if (!sideEffects)
						result.push_back(li);
					/* reset for next path */
					assumeFound = false;
					loadFound = false;
					sideEffects = false;
				}
			}
		});
	}
	result.erase(std::unique(result.begin(), result.end()), result.end());
	return result;
}

std::vector<Instruction *> LoadAnnotationPass::getAnnotatableLoads(CallInst *assm) const
{
	if (!isAssumeFunction(getCalledFunOrStripValName(*assm)))
		return std::vector<Instruction *>(); /* yet another check... */

	auto sourceLoads = getSourceLoads(assm);
	return filterAnnotatableFromSource(assm, sourceLoads);
}

bool LoadAnnotationPass::runOnFunction(llvm::Function &F)
{
	InstAnnotator annotator;

	for (auto &i : instructions(F)) {
		if (auto *a = llvm::dyn_cast<llvm::CallInst>(&i)) {
			if (isAssumeFunction(getCalledFunOrStripValName(*a))) {
				auto loads = getAnnotatableLoads(a);
				for (auto *l : loads) {
					LAI.annotMap[l] = annotator.annotate(l);
				}
			}
		}
	}
	return false;
}

FunctionPass *createLoadAnnotationPass(AnnotationInfo<Instruction *, Value *> &LAI)
{
	return new LoadAnnotationPass(LAI);
}

char LoadAnnotationPass::ID = 42;
// static llvm::RegisterPass<LoadAnnotationPass> P("annotate-loads",
// 						"Annotates loads used by assume() statements.");
