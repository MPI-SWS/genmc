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

#include "LoopJumpThreadingPass.hpp"
#include "genmc/Support/SExpr.hpp"
#include "genmc/Support/SExprVisitor.hpp"
#include "passes/LLVMUtils.hpp"
#include "passes/Transforms/InstAnnotator.hpp"

#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Use.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <llvm/Transforms/Scalar/LoopPassManager.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

using namespace llvm;

static auto inLoopBody(Loop *l, BasicBlock *bb) -> bool
{
	return l->contains(bb) && bb != l->getHeader();
}

static auto isNonTrivialUser(User *usr, PHINode *criticalPHI) -> bool
{
	auto *phi = dyn_cast<PHINode>(usr);
	return !phi || std::any_of(phi->user_begin(), phi->user_end(),
				   [criticalPHI](User *phiUser) { return phiUser != criticalPHI; });
}

static auto isCriticalPHIUsedTrivially(Loop *l, PHINode *criticalPHI) -> bool
{
	return std::none_of(criticalPHI->user_begin(), criticalPHI->user_end(), [&](User *usr) {
		auto *inst = dyn_cast<Instruction>(usr);
		if (!inst)
			return false;
		auto *parent = inst->getParent();
		return (inLoopBody(l, parent) && isNonTrivialUser(usr, criticalPHI)) ||
		       (parent == l->getHeader() &&
			std::any_of(criticalPHI->incoming_values().begin(),
				    criticalPHI->incoming_values().end(),
				    [usr](Value *in) { return usr == in; }));
	});
}

static auto areNonCriticalPHIsUsedInBody(Loop *l, PHINode *criticalPHI) -> bool
{
	for (auto &phi : l->getHeader()->phis()) {
		if (&phi == criticalPHI)
			continue;
		for (auto *usr : phi.users()) {
			auto *userInst = dyn_cast<Instruction>(usr);
			if (!userInst)
				continue;
			if (inLoopBody(l, userInst->getParent()))
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
static auto loopUsesHeaderPHIsTrivially(Loop *l, PHINode *criticalPHI) -> bool
{
	return isCriticalPHIUsedTrivially(l, criticalPHI) &&
	       !areNonCriticalPHIsUsedInBody(l, criticalPHI);
}

static auto getPHIConstEntryValueUsedInCond(Loop *l) -> PHINode *
{
	for (auto iit = l->getHeader()->begin(); auto *phi = dyn_cast<PHINode>(iit); ++iit) {
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

static auto generateExprJumpsToBody(Loop *l) -> std::unique_ptr<SExpr<Value *>>
{
	auto condExp = InstAnnotator().annotateBBCond(l->getHeader(), l->getLoopPredecessor());
	auto *bi = dyn_cast<BranchInst>(l->getHeader()->getTerminator());
	if (!bi)
		return ConcreteExpr<Value *>::createFalse();

	if (inLoopBody(l, bi->getSuccessor(0)))
		return condExp;
	return NotExpr<Value *>::create(std::move(condExp));
}

static auto entryAlwaysJumpsToBody(Loop *l) -> bool
{
	/* Make sure that the header (conditionally) jumps at the body */
	auto *header = l->getHeader();
	if (std::none_of(succ_begin(header), succ_end(header),
			 [&](BasicBlock *bb) { return inLoopBody(l, bb); }))
		return false;

	/* Get the expression that jumps from the header to the body.. */
	auto expr = generateExprJumpsToBody(l);

	/* ...and check whether it always evaluates to true */
	size_t numSeen = 0;
	auto res = SExprEvaluator<Value *>().evaluate(expr.get(), SExprEvaluator<Value *>::VMap(),
						      &numSeen);
	return (numSeen == 0) && res.getBool();
}

static auto invertLoop(Loop *l, PHINode *criticalPHI) -> bool
{
	auto *preheader = l->getLoopPredecessor();
	auto *phbi = dyn_cast<BranchInst>(preheader->getTerminator());
	auto *header = l->getHeader();
	auto *hbi = dyn_cast<BranchInst>(header->getTerminator());
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
	replaceUsesWithIf(criticalPHI, criticalPHI->getIncomingValueForBlock(preheader),
			  [&](Use &use) {
				  auto *userInst = dyn_cast<Instruction>(use.getUser());
				  return userInst && inLoopBody(l, userInst->getParent());
			  });

	// /*
	//  * Create a new PHI node to the bb of the body the entry jumps to:
	//  * this is gonnab be the new header
	//  */
	// auto *newPHI = PHINode::Create(criticalPHI->getType(), 2,
	// 			       criticalPHI->getName() + ".inv");
	// newPHI->addIncoming(criticalPHI->getIncomingValueForBlock(preheader), preheader);
	// newPHI->addIncoming(criticalPHI, header);

	// /* Replace uses of the old PHI in the loop's body and insert the new PHI */
	// criticalPHI->replaceAllUsesWith(newPHI);
	// newPHI->insertBefore(b->begin());

	/* Fix PHI in the old header */
	header->removePredecessor(preheader);

	/* Set preheader's successor to the loop body */
	auto phJmpIdx = (phbi->getSuccessor(0) == header) ? 0 : 1;
	auto *bodyBB = (inLoopBody(l, hbi->getSuccessor(0))) ? hbi->getSuccessor(0)
							     : hbi->getSuccessor(1);
	phbi->setSuccessor(phJmpIdx, bodyBB);

	/* Actually change the header */
	l->moveToHeader(bodyBB);
	return true;
}

auto LoopJumpThreadingPass::run(Loop &L, LoopAnalysisManager & /*AM*/,
				LoopStandardAnalysisResults & /*AR*/, LPMUpdater & /*U*/)
	-> PreservedAnalyses
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
