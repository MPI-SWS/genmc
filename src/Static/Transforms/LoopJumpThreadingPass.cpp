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

#include "LoopJumpThreadingPass.hpp"
#include "Static/LLVMUtils.hpp"
#include "Static/Transforms/InstAnnotator.hpp"
#include "Support/SExprVisitor.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>

using namespace llvm;

auto inLoopBody(Loop *l, BasicBlock *bb) -> bool { return l->contains(bb) && bb != l->getHeader(); }

auto isNonTrivialUser(User *u, PHINode *criticalPHI) -> bool
{
	auto *p = dyn_cast<PHINode>(u);
	return !p || std::any_of(p->user_begin(), p->user_end(),
				 [criticalPHI](User *us) { return us != criticalPHI; });
}

auto isCriticalPHIUsedTrivially(Loop *l, PHINode *criticalPHI) -> bool
{
	return std::none_of(criticalPHI->user_begin(), criticalPHI->user_end(), [&](User *u) {
		auto *p = dyn_cast<Instruction>(u)->getParent();
		return (inLoopBody(l, p) && isNonTrivialUser(u, criticalPHI)) ||
		       (p == l->getHeader() && std::any_of(criticalPHI->incoming_values().begin(),
							   criticalPHI->incoming_values().end(),
							   [u](Value *in) { return u == in; }));
	});
}

auto areNonCriticalPHIsUsedInBody(Loop *l, PHINode *criticalPHI) -> bool
{
	for (auto &phi : l->getHeader()->phis()) {
		if (&phi == criticalPHI)
			continue;
		for (auto *u : phi.users()) {
			auto *ui = dyn_cast<Instruction>(u);
			if (!ui)
				continue;
			if (inLoopBody(l, ui->getParent()))
				return true;
		}
	}
	return false;
}

/*
 * Returns true if the header PHIs are used in the body in a "harmless" manner.
 * Only the criticalPHI is allowed to have uses within the body, and these uses
 * need to be PHI nodes that are in turn used in the criticalPHI: such uses
 * are OK because if we prove that the header always jumps to the body,
 * then these uses can be replaced with the initial value.
 */
auto loopUsesHeaderPHIsTrivially(Loop *l, PHINode *criticalPHI) -> bool
{
	return isCriticalPHIUsedTrivially(l, criticalPHI) &&
	       !areNonCriticalPHIsUsedInBody(l, criticalPHI);
}

auto getPHIConstEntryValueUsedInCond(Loop *l) -> PHINode *
{
	for (auto iit = l->getHeader()->begin(); auto phi = dyn_cast<PHINode>(iit); ++iit) {
		for (auto &v : phi->incoming_values()) {
			if (isa<ConstantInt>(v) &&
			    phi->getIncomingBlock(v) == l->getLoopPredecessor() &&
			    isDependentOn(l->getHeader()->getTerminator(), phi)) {
				return phi;
			}
		}
	}
	return nullptr;
}

auto generateExprJumpsToBody(Loop *l) -> std::unique_ptr<SExpr<Value *>>
{
	auto condExp = InstAnnotator().annotateBBCond(l->getHeader(), l->getLoopPredecessor());
	auto *bi = dyn_cast<BranchInst>(l->getHeader()->getTerminator());
	if (!bi)
		return ConcreteExpr<Value *>::createFalse();

	if (inLoopBody(l, bi->getSuccessor(0)))
		return condExp;
	return NotExpr<Value *>::create(std::move(condExp));
}

auto entryAlwaysJumpsToBody(Loop *l) -> bool
{
	/* Make sure that the header (conditionally) jumps at the body */
	auto *h = l->getHeader();
	if (std::none_of(succ_begin(h), succ_end(h),
			 [&](BasicBlock *bb) { return inLoopBody(l, bb); }))
		return false;

	/* Get the expression that jumps from the header to the body.. */
	auto e = generateExprJumpsToBody(l);

	/* ...and check whether it always evaluates to true */
	size_t numSeen;
	auto res = SExprEvaluator<Value *>().evaluate(e.get(), SExprEvaluator<Value *>::VMap(),
						      &numSeen);
	return (numSeen == 0) && res.getBool();
}

auto invertLoop(Loop *l, PHINode *criticalPHI) -> bool
{
	auto *ph = l->getLoopPredecessor();
	auto *phbi = dyn_cast<BranchInst>(ph->getTerminator());
	auto *h = l->getHeader();
	auto *hbi = dyn_cast<BranchInst>(h->getTerminator());
	if (!phbi || !hbi) // sanity check
		return false;

	/* If header's Φ nodes are used in the loop body non-trivially, skip...*/
	if (!loopUsesHeaderPHIsTrivially(l, criticalPHI))
		return false;

	/*
	 * In principle, we can invert the loop even if the body was using the header's
	 * Φs (as in the comment below), but this does not buy us anything, as we would not get
	 * rid of any Φ nodes (new ones would have to be inserted).
	 *
	 * Now, however, we have proven that the critical PHI's value remains unchanged
	 * as long as the loop is executed, so we can replace all uses with the initial value
	 */
	replaceUsesWithIf(criticalPHI, criticalPHI->getIncomingValueForBlock(ph), [&](Use &u) {
		auto *us = dyn_cast<Instruction>(u.getUser());
		return us && inLoopBody(l, us->getParent());
	});

	// /*
	//  * Create a new PHI node to the BB of the body the entry jumps to:
	//  * this is gonnab be the new header
	//  */
	// auto *newPHI = PHINode::Create(criticalPHI->getType(), 2,
	// 			       criticalPHI->getName() + ".inv");
	// newPHI->addIncoming(criticalPHI->getIncomingValueForBlock(ph), ph);
	// newPHI->addIncoming(criticalPHI, h);

	// /* Replace uses of the old PHI in the loop's body and insert the new PHI */
	// criticalPHI->replaceAllUsesWith(newPHI);
	// newPHI->insertBefore(b->begin());

	/* Fix PHI in the old header */
	h->removePredecessor(ph);

	/* Set preheader's successor to the loop body */
	auto phJmpIdx = (phbi->getSuccessor(0) == h) ? 0 : 1;
	auto *b = (inLoopBody(l, hbi->getSuccessor(0))) ? hbi->getSuccessor(0)
							: hbi->getSuccessor(1);
	phbi->setSuccessor(phJmpIdx, b);

	/* Actually change the header */
	l->moveToHeader(b);
	return true;
}

auto LoopJumpThreadingPass::run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR,
				LPMUpdater &U) -> PreservedAnalyses
{
	/* The whole point is to get rid of Φ-nodes in the header... */
	if (!isa<PHINode>(L.getHeader()->begin()))
		return PreservedAnalyses::all();

	/* If the header has multiple predecessors, skip */
	if (!L.getLoopPredecessor())
		return PreservedAnalyses::all();

	/*
	 * The header needs to have at least one constant incoming
	 * from the entry, so that we can evaluate the header condition
	 */
	auto *criticalPHI = getPHIConstEntryValueUsedInCond(&L);
	if (!criticalPHI)
		return PreservedAnalyses::all();

	if (entryAlwaysJumpsToBody(&L))
		return invertLoop(&L, criticalPHI) ? PreservedAnalyses::none()
						   : PreservedAnalyses::all();
	return PreservedAnalyses::all();
}
