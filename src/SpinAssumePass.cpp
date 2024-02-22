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

#include "CallInfoCollectionPass.hpp"
#include "DeclareInternalsPass.hpp"
#include "Error.hpp"
#include "EscapeCheckerPass.hpp"
#include "InstAnnotator.hpp"
#include "InterpreterEnumAPI.hpp"
#include "LLVMUtils.hpp"
#include "SExprVisitor.hpp"
#include "SpinAssumePass.hpp"
#include "VSet.hpp"
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/LoopUtils.h>

#include <optional>
#include <unordered_set>
#include <utility>

using namespace llvm;

#define POSTDOM_PASS PostDominatorTreeWrapperPass
#define GET_POSTDOM_PASS() getAnalysis<POSTDOM_PASS>().getPostDomTree();

#if LLVM_VERSION_MAJOR >= 9
#define INSERT_PREHEADER_FOR_LOOP(L, DT, LI) llvm::InsertPreheaderForLoop(L, DT, LI, nullptr, false)
#else
#define INSERT_PREHEADER_FOR_LOOP(L, DT, LI) llvm::InsertPreheaderForLoop(L, DT, LI, false)
#endif

void SpinAssumePass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	au.addRequired<DominatorTreeWrapperPass>();
	au.addRequired<POSTDOM_PASS>();
	au.addRequired<DeclareInternalsPass>();
	au.addRequired<CallInfoCollectionPass>();
	au.addRequired<EscapeCheckerPass>();
	au.setPreservesAll();
}

void getLoopCASs(const Loop *l, SmallVector<const AtomicCmpXchgInst *, 4> &cass)
{
	for (auto bb = l->block_begin(); bb != l->block_end(); ++bb) {
		for (auto i = (*bb)->begin(); i != (*bb)->end(); ++i) {
			if (auto *casi = dyn_cast<AtomicCmpXchgInst>(i))
				cass.push_back(casi);
		}
	}
	return;
}

bool isBlockPHIClean(const BasicBlock *bb) { return !isa<PHINode>(&*bb->begin()); }

bool accessSameVariable(const Value *p1, const Value *p2)
{
	/* Check if they are trivially the same */
	if (p1 == p2)
		return true;

	/* Check if they come from identical instructions */
	if (auto *i1 = dyn_cast<Instruction>(p1))
		if (auto *i2 = dyn_cast<Instruction>(p2))
			if (i1->isIdenticalTo(i2))
				return true;

	// FIXME: Is this really necessary?
	/* Special case: check if they are produced by bitcasts */
	if (auto *bi1 = dyn_cast<BitCastInst>(p1)) {
		if (bi1->getOperand(0) == p2)
			return true;
		if (auto *bi2 = dyn_cast<BitCastInst>(p2)) {
			if (p1 == bi2->getOperand(0))
				return true;
			if (bi1->getOperand(0) == bi2->getOperand(0))
				return true;
		}
	}
	return false;
}

bool isPHIRelatedToCASCmp(const PHINode *curr,
			  const SmallVector<const AtomicCmpXchgInst *, 4> &cass,
			  SmallVector<const PHINode *, 4> &phiChain, VSet<const PHINode *> &related)
{
	/* Check if we have already decided (or assumed) this phi is good */
	if (related.count(curr) ||
	    std::find(phiChain.begin(), phiChain.end(), curr) != phiChain.end())
		return true;

	for (Value *val : curr->incoming_values()) {
		val = stripCasts(val);
		if (auto *uv = dyn_cast<UndefValue>(val)) {
			continue;
		} else if (auto *li = dyn_cast<LoadInst>(val)) {
			if (std::all_of(
				    cass.begin(), cass.end(), [&](const AtomicCmpXchgInst *casi) {
					    return !accessSameVariable(li->getPointerOperand(),
								       casi->getPointerOperand()) ||
						   !areSameLoadOrdering(li->getOrdering(),
									casi->getSuccessOrdering());
				    }))
				return false;
		} else if (auto *extract = dyn_cast<ExtractValueInst>(val)) {
			auto *ecasi = extractsFromCAS(extract);
			if (!ecasi || *extract->idx_begin() != 0)
				return false;
			if (std::all_of(
				    cass.begin(), cass.end(), [&](const AtomicCmpXchgInst *casi) {
					    return !accessSameVariable(ecasi->getPointerOperand(),
								       casi->getPointerOperand()) ||
						   !areSameLoadOrdering(ecasi->getSuccessOrdering(),
									casi->getSuccessOrdering());
				    }))
				return false;
		} else {
			auto *phi = dyn_cast<PHINode>(val);
			if (!phi)
				return false;

			phiChain.push_back(curr);
			if (!isPHIRelatedToCASCmp(phi, cass, phiChain, related))
				return false;
			phiChain.pop_back();
		}
	}
	return true;
}

bool isPHIRelatedToCASRes(const PHINode *curr,
			  const SmallVector<const AtomicCmpXchgInst *, 4> &cass,
			  SmallVector<const PHINode *, 4> &phiChain, VSet<const PHINode *> &related)
{
	/* Check if we have already decided (or assumed) this phi is good */
	if (related.count(curr) ||
	    std::find(phiChain.begin(), phiChain.end(), curr) != phiChain.end())
		return true;

	for (Value *val : curr->incoming_values()) {
		val = stripCasts(val);
		if (auto *c = dyn_cast<Constant>(val)) {
			if (auto *ci = dyn_cast<ConstantInt>(c)) {
				if (!ci->isZero())
					return false;
			} else if (!isa<UndefValue>(c)) {
				return false;
			}
		} else if (auto *extract = dyn_cast<ExtractValueInst>(val)) {
			auto *ecasi = extractsFromCAS(extract);
			if (!ecasi || *extract->idx_begin() != 1)
				return false;
			if (std::all_of(
				    cass.begin(), cass.end(), [&](const AtomicCmpXchgInst *casi) {
					    return !accessSameVariable(ecasi->getPointerOperand(),
								       casi->getPointerOperand()) ||
						   !areSameLoadOrdering(ecasi->getSuccessOrdering(),
									casi->getSuccessOrdering());
				    }))
				return false;
		} else {
			auto *phi = dyn_cast<PHINode>(val);
			if (!phi)
				return false;

			phiChain.push_back(curr);
			if (!isPHIRelatedToCASRes(phi, cass, phiChain, related))
				return false;
			phiChain.pop_back();
		}
	}

	/* If the current PHI does not depend on any other PHI being good, then we mark it as such
	 */
	if (phiChain.empty())
		related.insert(curr);
	return true;
}

bool isPHIRelatedToCASCmp(const PHINode *phi, const SmallVector<const AtomicCmpXchgInst *, 4> &cass)
{
	VSet<const PHINode *> related;
	SmallVector<const PHINode *, 4> phiChain;

	return isPHIRelatedToCASCmp(phi, cass, phiChain, related);
}

bool isPHIRelatedToCASRes(const PHINode *phi, const SmallVector<const AtomicCmpXchgInst *, 4> &cass)
{
	VSet<const PHINode *> related;
	SmallVector<const PHINode *, 4> phiChain;

	return isPHIRelatedToCASRes(phi, cass, phiChain, related);
}

bool isPHIRelatedToLoad(const PHINode *curr, Value *&loadPtr,
			std::optional<AtomicOrdering> &loadOrd,
			SmallVector<const PHINode *, 4> &phiChain, VSet<const PHINode *> &related)
{
	/* Check if we have already decided (or assumed) this phi is good */
	if (related.count(curr) ||
	    std::find(phiChain.begin(), phiChain.end(), curr) != phiChain.end())
		return true;

	for (Value *val : curr->incoming_values()) {
		val = stripCasts(val);
		if (auto *li = dyn_cast_or_null<LoadInst>(val)) {
			if (loadPtr && !accessSameVariable(li->getPointerOperand(), loadPtr))
				return false;
			if (loadOrd.has_value() &&
			    !areSameLoadOrdering(li->getOrdering(), *loadOrd))
				return false;
			loadPtr = li->getPointerOperand();
			loadOrd = li->getOrdering();
		} else {
			auto *phi = dyn_cast<PHINode>(val);
			if (!phi)
				return false;

			phiChain.push_back(curr);
			if (!isPHIRelatedToLoad(phi, loadPtr, loadOrd, phiChain, related))
				return false;
			phiChain.pop_back();
		}
	}
	return true;
}

bool isPHIRelatedToLoad(const PHINode *phi)
{
	Value *loadPtr = nullptr;
	std::optional<AtomicOrdering> loadOrd;
	VSet<const PHINode *> related;
	SmallVector<const PHINode *, 4> phiChain;

	return isPHIRelatedToLoad(phi, loadPtr, loadOrd, phiChain, related);
}

/*
 * This function checks whether a PHI node is tied to some load or CAS ('good' PHI).
 * A 'good' PHI node has incoming values that are either 1) PHI nodes that have been
 * deemed 'good', 2) constants and results of loads/CASes, or 3) loads at the same
 * location as some CAS and the compare operands of some CAS.
 * To avoid circles between PHIs, whenever we try to see whether a PHI is good,
 * we keep the current path in <phiChain>; if a node is deemed good and the chain
 * is empty (i.e., it does not depend on another node being deemed good), it is
 * moved to <related>, which stores PHIs that are related to some CAS operation.
 */
bool areBlockPHIsRelatedToLoopCASs(const BasicBlock *bb, Loop *l)
{
	SmallVector<const AtomicCmpXchgInst *, 4> cass;

	getLoopCASs(l, cass);
	if (cass.empty()) {
		return std::all_of(bb->phis().begin(), bb->phis().end(),
				   [&](auto &phi) { return isPHIRelatedToLoad(&phi); });
	}

	return std::all_of(bb->phis().begin(), bb->phis().end(), [&](auto &phi) {
		return isPHIRelatedToCASCmp(&phi, cass) || isPHIRelatedToCASRes(&phi, cass) ||
		       isPHIRelatedToLoad(&phi);
	});
}

/*
 * Returns whether INST extracts from a CAS. If a second argument is provided, it is ensured
 * that the extract extracts from this particular CAS.
 */
bool isCASExtract(Instruction *inst, AtomicCmpXchgInst *cas = nullptr)
{
	auto *extract = llvm::dyn_cast_or_null<ExtractValueInst>(inst);
	if (!extract)
		return false;

	if (!extract->getType()->isIntegerTy() || extract->getNumIndices() > 1 ||
	    *extract->idx_begin() != 1)
		return false;

	auto *ecasi = dyn_cast<AtomicCmpXchgInst>(extract->getAggregateOperand());
	if (!ecasi)
		return false;
	return (cas) ? (cas == ecasi) : true;
}

/*
 * Tries to extract extractinsts corresponding to the results of a list of CASes.
 * Returns whether that was possible or not.
 */
bool tryGetCASResultExtracts(const std::vector<AtomicCmpXchgInst *> &cass,
			     std::vector<ExtractValueInst *> &extracts)
{
	for (auto &cas : cass) {
		/* Try both subsequent instructions as we might have
		 * eliminated one of them */
		if (isCASExtract(cas->getNextNode()->getNextNode(), cas)) {
			extracts.push_back(llvm::dyn_cast<ExtractValueInst>(
				cas->getNextNode()->getNextNode()));
		} else if (isCASExtract(cas->getNextNode(), cas)) {
			extracts.push_back(llvm::dyn_cast<ExtractValueInst>(cas->getNextNode()));
		} else {
			return false;
		}
	}
	return true;
}

/*
 * Returns whether the CASes of a backedge LATCH -> header(L) will lead to the loop header if
 * they fail. (If we exit the backedge paths with any of the CASes being successful, this function
 * will return false.)
 */
bool failedCASesLeadToHeader(const std::vector<AtomicCmpXchgInst *> &cass, BasicBlock *latch,
			     Loop *l, const CallInfoCollectionPass::CallSet &cleanSet)
{
	if (cass.empty())
		return true;

	std::vector<ExtractValueInst *> extracts;

	if (!tryGetCASResultExtracts(cass, extracts))
		return false;

	std::vector<std::unique_ptr<SExpr<Value *>>> casConditions;
	for (auto *cas : cass)
		casConditions.push_back(
			InstAnnotator().annotateCASWithBackedgeCond(cas, latch, l, &cleanSet));

	auto backedgeCondition = ConjunctionExpr<Value *>::create(std::move(casConditions));
	for (auto i = 1u; i < (1 << extracts.size()); i++) {

		std::unordered_map<Value *, SVal> valueMap;
		for (auto j = 0u; j < extracts.size(); j++)
			valueMap[extracts[j]] = (i & (1 << j)) ? SVal(1) : SVal(0);

		size_t unknowns;
		auto res = SExprEvaluator<Value *>().evaluate(backedgeCondition.get(), valueMap,
							      &unknowns);
		if (unknowns > 0 || res.getBool())
			return false;
	}
	return true;
}

bool isStoreLocal(StoreInst *si, EscapeInfo &EI, DominatorTree &DT)
{
	/* A store is local if it is either marked or writes to dynamic memory */
	auto attr = getWriteAttr(*si);
	auto *alloc = EI.writesDynamicMemory(si->getPointerOperand());
	return (alloc && EI.escapesAfter(alloc, si, DT)) || !!(attr & WriteAttr::Local);
}

bool SpinAssumePass::isPathToHeaderEffectFree(BasicBlock *latch, Loop *l, bool &checkDynamically)
{
	auto &cleanSet = getAnalysis<CallInfoCollectionPass>().getCleanCalls();
	auto &EI = getAnalysis<EscapeCheckerPass>().getEscapeInfo();
	auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
	auto effects = false;
	std::vector<AtomicCmpXchgInst *> cass;

	foreachInBackPathTo(latch, l->getHeader(), [&](Instruction &i) {
		/* Try to prove that failed CASes imply another iteration */
		if (auto *casi = dyn_cast<AtomicCmpXchgInst>(&i)) {
			cass.push_back(casi);
			return;
		}
		/* Local stores are allowed */
		if (auto *si = dyn_cast<StoreInst>(&i)) {
			if (isStoreLocal(si, EI, DT))
				return;
		}
		effects |= hasSideEffects(&i, &cleanSet);
	});
	if (effects) {
		checkDynamically = true;
		return false;
	}

	std::sort(cass.begin(), cass.end());
	cass.erase(std::unique(cass.begin(), cass.end()), cass.end());

	auto casOK = true;
	if (!cass.empty()) {
		casOK = failedCASesLeadToHeader(cass, latch, l, cleanSet);
		checkDynamically |= !casOK;
	}
	return casOK;
}

template <typename F> bool checkConstantsCondition(const Value *v1, const Value *v2, F &&cond)
{
	auto c1 = dyn_cast<ConstantInt>(v1);
	auto c2 = dyn_cast<ConstantInt>(v2);

	if (!c1 || !c2)
		return false;

	return cond(c1->getValue(), c2->getValue());
}

bool areCancelingBinops(const AtomicRMWInst *a, const AtomicRMWInst *b)
{
	using namespace llvm;
	using BinOp = AtomicRMWInst::BinOp;

	auto op1 = a->getOperation();
	auto op2 = b->getOperation();
	auto v1 = a->getValOperand();
	auto v2 = b->getValOperand();

	/* The two operations need to manipulate the same memory location */
	if (!accessSameVariable(a->getPointerOperand(), b->getPointerOperand()))
		return false;

	/* The operators need to be opposite and the values the same, or vice versa */
	if (((op1 == BinOp::Add && op2 == BinOp::Sub) ||
	     (op1 == BinOp::Sub && op2 == BinOp::Add)) &&
	    checkConstantsCondition(v1, v2,
				    [&](const APInt &i1, const APInt &i2) { return i1 == i2; }))
		return true;

	bool overflow;
	if (op1 == op2 && (op1 == BinOp::Add || op1 == BinOp::Sub) &&
	    checkConstantsCondition(v1, v2, [&](const APInt &i1, const APInt &i2) {
		    return i1.sadd_ov(i2, overflow) == 0;
	    }))
		return true;
	return false;
}

bool dominatesAndPostdominates(Instruction *a, Instruction *b, DominatorTree &DT,
			       PostDominatorTree &PDT)
{
	return DT.dominates(a, b) && PDT.dominates(a->getParent(), b->getParent());
}

bool SpinAssumePass::isPathToHeaderFAIZNE(BasicBlock *latch, Loop *l, Instruction *&lastEffect)
{
	auto &cleanSet = getAnalysis<CallInfoCollectionPass>().getCleanCalls();
	auto effects = false;
	VSet<AtomicCmpXchgInst *> cass;
	VSet<AtomicRMWInst *> fais;

	foreachInBackPathTo(latch, l->getHeader(), [&](Instruction &i) {
		if (auto *faii = dyn_cast<AtomicRMWInst>(&i)) {
			fais.insert(faii);
			return;
		}
		/* CASes are OK as long as they are before the decrement */
		if (auto *casi = dyn_cast<AtomicCmpXchgInst>(&i)) {
			cass.insert(casi);
			return;
		}
		effects |= hasSideEffects(&i, &cleanSet);
	});

	if (effects || fais.size() != 2)
		return false;

	/* Check domination conditions; we need more checks here due to the (unordered) VSet */
	auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
	auto &PDT = GET_POSTDOM_PASS();

	AtomicRMWInst *inci = nullptr;
	AtomicRMWInst *deci = nullptr;
	if (DT.dominates(fais[0], fais[1]) && PDT.dominates(l->getHeader(), fais[1]->getParent())) {
		inci = fais[0];
		deci = fais[1];
	} else if (DT.dominates(fais[1], fais[0]) &&
		   PDT.dominates(l->getHeader(), fais[0]->getParent())) {
		inci = fais[1];
		deci = fais[0];
	}
	if (!inci || !deci)
		return false;
	if (std::any_of(cass.begin(), cass.end(),
			[&](AtomicCmpXchgInst *casi) { return !DT.dominates(casi, deci); }))
		return false;

	/* Check cancelation */
	if (!areCancelingBinops(inci, deci))
		return false;

	lastEffect = deci;
	return true;
}

bool SpinAssumePass::isPathToHeaderLockZNE(BasicBlock *latch, Loop *l, Instruction *&lastEffect)
{
	auto &cleanSet = getAnalysis<CallInfoCollectionPass>().getCleanCalls();
	auto effects = false;
	VSet<CallInst *> locks;
	VSet<CallInst *> unlocks;
	VSet<PHINode *> phis;

	foreachInBackPathTo(latch, l->getHeader(), [&](Instruction &i) {
		if (auto *ci = dyn_cast<CallInst>(&i)) {
			auto name = getCalledFunOrStripValName(*ci);
			if (isInternalFunction(name)) {
				auto icode = internalFunNames.at(name);
				if (icode == InternalFunctions::FN_MutexLock) {
					locks.insert(ci);
					return;
				}
				if (icode == InternalFunctions::FN_MutexUnlock) {
					unlocks.insert(ci);
					return;
				}
			}
		}
		if (auto *phi = dyn_cast<PHINode>(&i)) {
			if (phi->getParent() == l->getHeader()) /* PHIs in the body are OK */
				phis.insert(phi);
			return;
		}
		effects |= hasSideEffects(&i, &cleanSet);
	});

	if (effects || locks.size() != 1 || unlocks.size() != 1 || !phis.empty())
		return false;

	auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
	auto &PDT = GET_POSTDOM_PASS();
	auto lDomU = dominatesAndPostdominates(locks[0], unlocks[0], DT, PDT);
	if (!lDomU || !accessSameVariable(*locks[0]->arg_begin(), *unlocks[0]->arg_begin()))
		return false;

	lastEffect = unlocks[0];
	return true;
}

Value *getOrCreateExitingCondition(BasicBlock *header, Instruction *term)
{
	if (auto *ibr = dyn_cast<IndirectBrInst>(term))
		return ConstantInt::getFalse(term->getContext());

	auto *bi = dyn_cast<BranchInst>(term);
	BUG_ON(!bi);

	if (bi->isUnconditional())
		return ConstantInt::getFalse(term->getContext());
	if (bi->getSuccessor(0) != header)
		return bi->getCondition();
	return BinaryOperator::CreateNot(bi->getCondition(), "", term);
}

void addLoopBeginCallBeforeTerm(BasicBlock *preheader)
{
	auto *term = preheader->getTerminator();
	auto *beginFun = preheader->getParent()->getParent()->getFunction("__VERIFIER_loop_begin");
	BUG_ON(!beginFun);

	auto *ci = CallInst::Create(beginFun, {}, "", term);
	ci->setMetadata("dbg", term->getMetadata("dbg"));
	return;
}

void addSpinEndCallBeforeTerm(BasicBlock *latch, BasicBlock *header)
{
	auto *term = latch->getTerminator();
	auto *endFun = latch->getParent()->getParent()->getFunction("__VERIFIER_spin_end");
	BUG_ON(!endFun);

	auto *cond = getOrCreateExitingCondition(header, term);
	auto *ci = CallInst::Create(endFun, {cond}, "", term);
	ci->setMetadata("dbg", term->getMetadata("dbg"));
	return;
}

void addPotentialSpinEndCallBeforeLastFai(BasicBlock *latch, BasicBlock *header,
					  Instruction *lastEffect)
{
	auto *endFun = latch->getParent()->getParent()->getFunction("__VERIFIER_faiZNE_spin_end");
	BUG_ON(!endFun);

	auto *ci = CallInst::Create(endFun, {}, "", lastEffect);
	ci->setMetadata("dbg", lastEffect->getMetadata("dbg"));
	return;
}

void addPotentialSpinEndCallBeforeUnlock(BasicBlock *latch, BasicBlock *header,
					 Instruction *lastEffect)
{
	auto *endFun = latch->getParent()->getParent()->getFunction("__VERIFIER_lockZNE_spin_end");
	BUG_ON(!endFun);

	auto *ci = CallInst::Create(endFun, {}, "", lastEffect);
	ci->setMetadata("dbg", lastEffect->getMetadata("dbg"));
	return;
}

void addSpinStartCall(Loop *l)
{
	auto *startFun =
		l->getHeader()->getParent()->getParent()->getFunction("__VERIFIER_spin_start");

	auto *i = l->getHeader()->getFirstNonPHI();
	auto *ci = CallInst::Create(startFun, {}, "", i);
}

bool SpinAssumePass::runOnLoop(Loop *l, LPPassManager &lpm)
{
	auto *header = l->getHeader();

	if (!isBlockPHIClean(header) && !areBlockPHIsRelatedToLoopCASs(header, l))
		return false;

	SmallVector<BasicBlock *, 4> latches;
	l->getLoopLatches(latches);

	auto spinloop = true;
	auto modified = false;
	auto checkDynamically = false;
	llvm::Instruction *lastZNEEffect = nullptr;
	for (auto &latch : latches) {
		if (isPathToHeaderFAIZNE(latch, l, lastZNEEffect)) {
			checkDynamically = true;
			modified = true;
			addPotentialSpinEndCallBeforeLastFai(latch, header, lastZNEEffect);
		} else if (isPathToHeaderLockZNE(latch, l, lastZNEEffect)) {
			/* Check for lockZNE before effect-free paths,
			 * as locks are function calls too...  */
			checkDynamically = true;
			modified = true;
			addPotentialSpinEndCallBeforeUnlock(latch, header, lastZNEEffect);
		} else if (isPathToHeaderEffectFree(latch, l, checkDynamically)) {
			/* If we have to dynamically validate the loop,
			 * the above check will return false, but the path
			 * may be checked dynamically */
			addSpinEndCallBeforeTerm(latch, header);
			modified = true; /* At the very least, a spin_start is inserted */
		} else {
			spinloop = false;
		}
	}
	if (checkDynamically)
		spinloop = false;

	/* Mark spinloop start if we have to */
	if (checkDynamically || (modified && markStarts)) {
		addSpinStartCall(l);
		/* DSA also requires us to know when we actually enter the loop;
		 * mark the beginning anyway to compose with liveness checks and nested loops */
		auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
		auto &LI = lpm.getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
		auto *ph = INSERT_PREHEADER_FOR_LOOP(l, &DT, &LI);
		addLoopBeginCallBeforeTerm(ph);
	}

	/* If the transformation applied did not apply in all backedges, this is indeed a loop */
	if (!spinloop)
		return modified;
	/* An actual spinloop: we used to remove it from the loop list but let's keep it */
	return modified;
}

Pass *createSpinAssumePass(bool markStarts /* = false */)
{
	auto *p = new SpinAssumePass();
	p->markSpinloopStarts(markStarts);
	return p;
}

char SpinAssumePass::ID = 42;
static llvm::RegisterPass<SpinAssumePass> P("spin-assume",
					    "Performs GenMC's spin-assume transformation.");
