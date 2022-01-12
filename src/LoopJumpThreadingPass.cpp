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
#include "InstAnnotator.hpp"
#include "LLVMUtils.hpp"
#include "SExprVisitor.hpp"
#include <llvm/IR/Constants.h>

using namespace llvm;

void LoopJumpThreadingPass::getAnalysisUsage(AnalysisUsage &au) const
{
	LoopPass::getAnalysisUsage(au);
}

bool inLoopBody(Loop *l, BasicBlock *bb)
{
	return l->contains(bb) && bb != l->getHeader();
}

bool loopBodyUsesHeaderPHIs(Loop *l)
{
	auto *h = l->getHeader();
	for (auto iit = h->begin(); auto *phi = dyn_cast<PHINode>(&*iit); ++iit) {
		for (auto *u : phi->users()) {
			if (auto *ui = dyn_cast<Instruction>(u))
				if (inLoopBody(l, ui->getParent()))
					return true;
		}
	}
	return false;
}

PHINode *getPHIConstEntryValueUsedInCond(Loop *l)
{
	for (auto iit = l->getHeader()->begin(); auto phi = dyn_cast<PHINode>(iit); ++iit) {
		for (auto &v : phi->incoming_values()) {
			if (isa<ConstantInt>(v) && phi->getIncomingBlock(v) == l->getLoopPredecessor() &&
			    isDependentOn(l->getHeader()->getTerminator(), phi)) {
				return phi;
			}
		}
	}
	return nullptr;
}

std::unique_ptr<SExpr<Value *>> generateExprJumpsToBody(Loop *l)
{
	auto condExp  = InstAnnotator().annotateBBCond(l->getHeader(), l->getLoopPredecessor());
	auto *bi = dyn_cast<BranchInst>(l->getHeader()->getTerminator());
	if (!bi)
		return ConcreteExpr<Value *>::createFalse();

	if (inLoopBody(l, bi->getSuccessor(0)))
		return condExp;
	return NotExpr<Value *>::create(std::move(condExp));
}

bool entryAlwaysJumpsToBody(Loop *l)
{
	/* Make sure that the header (conditionally) jumps at the body */
	auto *h = l->getHeader();
	if (std::none_of(succ_begin(h), succ_end(h), [&](BasicBlock *bb){ return inLoopBody(l, bb); }))
		return false;

	/* Get the expression that jumps from the header to the body.. */
	auto e = generateExprJumpsToBody(l);

	/* ...and check whether it always evaluates to true */
	size_t numSeen;
	auto res = SExprEvaluator<Value *>().evaluate(e.get(), SExprEvaluator<Value *>::VMap(), &numSeen);
	return (numSeen == 0) && res.getBool();
}

bool invertLoop(Loop *l, PHINode *criticalPHI)
{
	auto *ph = l->getLoopPredecessor();
	auto *phbi = dyn_cast<BranchInst>(ph->getTerminator());
	auto *h = l->getHeader();
	auto *hbi = dyn_cast<BranchInst>(h->getTerminator());
	if (!phbi || !hbi)
		return false;

	/* Set preheader's successor to the loop body */
	auto phJmpIdx = (phbi->getSuccessor(0) == h) ? 0 : 1;
	auto *b = (inLoopBody(l, hbi->getSuccessor(0))) ? hbi->getSuccessor(0) : hbi->getSuccessor(1);
	phbi->setSuccessor(phJmpIdx, b);

	// We could invert the loop even if the body was using the header's Φs (as below)
	// but this does not buy us anything, as we would not get rid of any Φ nodes
	// (new ones would have to be inserted)
	//
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
	// newPHI->insertBefore(&*b->begin());

	/* Actually change the header */
	l->moveToHeader(b);

	/* Fix PHI in the old header */
	h->removePredecessor(ph);
	return true;
}

bool LoopJumpThreadingPass::runOnLoop(Loop *l, LPPassManager &lpm)
{
	bool modified = false;

	/* The whole point is to get rid of Φ-nodes in the header... */
	if (!isa<PHINode>(*l->getHeader()->begin()))
		return modified;

	/* If header's Φ nodes are used in the loop body, skip...*/
	if (loopBodyUsesHeaderPHIs(l))
		return modified;

	/* If the header has multiple predecessors, skip */
	if (!l->getLoopPredecessor())
		return modified;

	/*
	 * The header needs to have at least one constant incoming
	 * from the entry, so that we can evaluate the header condition
	 */
	auto *criticalPHI = getPHIConstEntryValueUsedInCond(l);
	if (!criticalPHI)
		return modified;

	if (entryAlwaysJumpsToBody(l))
		modified = invertLoop(l, criticalPHI);
	return modified;
}

Pass *createLoopJumpThreadingPass()
{
	return new LoopJumpThreadingPass();
}

char LoopJumpThreadingPass::ID = 42;
static RegisterPass<LoopJumpThreadingPass> P("loop-jump-threading",
					     "Performs jump threading for simple loop headers.");
