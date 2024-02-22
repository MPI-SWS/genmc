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

#include "ConfirmationAnnotationPass.hpp"
#include "Error.hpp"
#include "InterpreterEnumAPI.hpp"
#include "LLVMUtils.hpp"
#include "config.h"
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;

void ConfirmationAnnotationPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	au.addRequired<DominatorTreeWrapperPass>();
	au.addRequired<LoopInfoWrapperPass>();
	au.setPreservesAll();
}

bool isSpinEndCall(Instruction *i)
{
	auto *ci = llvm::dyn_cast<CallInst>(i);
	if (!ci)
		return false;

	auto name = getCalledFunOrStripValName(*ci);
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::FN_SpinEnd;
}

/*
 * Given the instruction on which a __VERIFIER_spin_end() depends,
 * returns a candidate confirmation instruction: either a CAS or a CMP
 * between two loads.
 */
Instruction *getConfirmationCandidate(Instruction *i)
{
	auto *si = stripCastsConstOps(i);
	if (auto *ei = dyn_cast<ExtractValueInst>(si))
		return extractsFromCAS(ei);
	if (auto *ci = dyn_cast<CmpInst>(si)) {
		auto *op1 = dyn_cast<LoadInst>(stripCastsConstOps(ci->getOperand(0)));
		auto *op2 = dyn_cast<LoadInst>(stripCastsConstOps(ci->getOperand(1)));
		if (!op1 || !op2)
			return nullptr;
		return isa<LoadInst>(op1->stripPointerCasts()) &&
				       isa<LoadInst>(op2->stripPointerCasts())
			       ? ci
			       : nullptr;
	}
	return nullptr;
}

/*
 * Given a basic block containing a __VERIFIER_spin_end(0),
 * returns the common candidate confirmation instruction among
 * the block's predecessors.
 */
Instruction *getCommonConfirmationFromPreds(BasicBlock *bb)
{
	Instruction *conf = nullptr;
	if (std::all_of(pred_begin(bb), pred_end(bb), [&conf](const BasicBlock *pred) {
		    auto *ji = dyn_cast<BranchInst>(pred->getTerminator());
		    if (!ji || !ji->isConditional() || !isa<Instruction>(ji->getCondition()))
			    return false;
		    auto *res = getConfirmationCandidate(dyn_cast<Instruction>(ji->getCondition()));
		    if (!res)
			    return false;
		    if (conf == nullptr)
			    conf = res;
		    return conf == res;
	    }))
		return conf;
	return nullptr;
}

/*
 * Returns a confirmation-like instruction on which a __VERIFIER_spin_end() depends.
 * This instruction is going to be either a CAS or a CMP between two loads.
 * NOTE: The function does not check that the instruction is actually a confirmation
 * (e.g., by checking that the loads read from the same place).
 * This should be done separately later.
 */
Instruction *spinEndsOnConfirmation(CallInst *ci)
{
	auto *endValue = ci->getArgOperand(0);
	if (auto *i = dyn_cast<Instruction>(endValue)) {
		return getConfirmationCandidate(i);
	} else if (auto *c = dyn_cast<Constant>(endValue)) {
		if (c->getType()->isIntegerTy() && c->isZeroValue())
			return getCommonConfirmationFromPreds(ci->getParent());
	}
	return nullptr;
}

std::pair<LoadInst *, Instruction *> getConfirmationPair(Instruction *i, LoopInfo &LI,
							 DominatorTree &DT)
{
	LoadInst *spec = nullptr;
	Instruction *conf = nullptr;
	Value *specPtr = nullptr;
	Value *confPtr = nullptr;
	if (auto *casi = dyn_cast<AtomicCmpXchgInst>(i)) {
		conf = casi;
		spec = dyn_cast<LoadInst>(stripCasts(casi->getCompareOperand()));
		if (!spec)
			return std::make_pair(nullptr, nullptr);
		specPtr = stripCasts(spec->getPointerOperand());
		confPtr = stripCasts(casi->getPointerOperand());
	} else if (auto *ci = dyn_cast<CmpInst>(i)) {
		auto *l1 = dyn_cast<LoadInst>(stripCasts(ci->getOperand(0)));
		auto *l2 = dyn_cast<LoadInst>(stripCasts(ci->getOperand(1)));
		BUG_ON(!l1 || !l2);
		if (!DT.dominates(l1, l2) && !DT.dominates(l2, l1))
			return std::make_pair(nullptr, nullptr);
		if (DT.dominates(l1, l2)) {
			conf = l2;
			spec = l1;
			confPtr = stripCasts(l2->getPointerOperand());
			specPtr = stripCasts(spec->getPointerOperand());
		} else {
			conf = l1;
			spec = l2;
			confPtr = stripCasts(l1->getPointerOperand());
			specPtr = stripCasts(spec->getPointerOperand());
		}
	} else
		BUG();

	auto *l = LI.getLoopFor(conf->getParent());
	if (!l || !l->contains(spec))
		return std::make_pair(nullptr, nullptr);

	if (confPtr == specPtr)
		return std::make_pair(spec, conf);

	auto *specI = dyn_cast<Instruction>(specPtr);
	auto *confI = dyn_cast<Instruction>(confPtr);
	return (specI && confI && confI->isIdenticalTo(specI)) ? std::make_pair(spec, conf)
							       : std::make_pair(nullptr, nullptr);
}

void annotateConfPair(Instruction *i, LoopInfo &LI, DominatorTree &DT)
{
	auto pair = getConfirmationPair(i, LI, DT);
	auto spec = pair.first;
	auto conf = pair.second;
	if (!spec || !conf)
		return;

	annotateInstruction(spec, "genmc.attr", GENMC_KIND_SPECUL);
	annotateInstruction(conf, "genmc.attr", GENMC_KIND_CONFIRM);
}

void annotate(Function &F, LoopInfo &LI, DominatorTree &DT)
{
	for (auto it = inst_begin(F), ie = inst_end(F); it != ie; ++it) {
		if (!isSpinEndCall(&*it))
			continue;

		if (auto *conf = spinEndsOnConfirmation(dyn_cast<CallInst>(&*it)))
			annotateConfPair(conf, LI, DT);
	}
}

bool ConfirmationAnnotationPass::runOnFunction(llvm::Function &F)
{
	annotate(F, getAnalysis<LoopInfoWrapperPass>().getLoopInfo(),
		 getAnalysis<DominatorTreeWrapperPass>().getDomTree());
	return false;
}

FunctionPass *createConfirmationAnnotationPass() { return new ConfirmationAnnotationPass(); }

char ConfirmationAnnotationPass::ID = 42;
static llvm::RegisterPass<ConfirmationAnnotationPass>
	P("annotate-confirmation", "Annotates loads used in confirmation patterns.");
