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

#include "EliminateCASPHIsPass.hpp"

#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/Casting.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <algorithm>
#include <vector>

using namespace llvm;

static auto hasPHIIncomingExtract(llvm::PHINode *phi) -> ExtractValueInst *
{
	for (Value *val : phi->incoming_values())
		if (auto *ei = dyn_cast<ExtractValueInst>(val))
			return ei;
	return nullptr;
}

/*
 * Tries to eliminate PHI and returns whether it succeeded. In the process of
 * doing so, also tries to simplify the CFG resulting from the CAS.
 * Populates TODELETE with the instructions that need to be erased.
 * May leave the CFG in an invalid state, requiring cleanup.
 */
static auto tryEliminateCASPHI(llvm::PHINode *phi, DominatorTree &DT,
			       std::vector<Instruction *> &toDelete) -> bool
{
	/* The PHI must have exactly two incomings... */
	if (phi->getNumIncomingValues() != 2)
		return false;

	/* ... one of the must be an extract from a cas... */
	ExtractValueInst *extract = hasPHIIncomingExtract(phi);
	if (!extract)
		return false;

	auto *cas = dyn_cast<AtomicCmpXchgInst>(extract->getAggregateOperand());
	if (!cas)
		return false;

	/* ... while the other needs to be the compare operand of the cas */
	auto otherId = (phi->getIncomingValue(0) == extract);
	auto *other = phi->getIncomingValue(otherId);
	if (cas->getCompareOperand() != other)
		return false;

	/* Finally, the compare operand should be taken when we are coming from the CAS's block
	 * (success) */
	if (phi->getIncomingBlock(otherId) != cas->getParent())
		return false;

	/* We know it's eliminable, so go ahead and replace all uses.
	 * However, also check if we can simplify the CFG a bit */
	for (auto *usr : phi->users()) {
		if (auto *otherPhi = dyn_cast<PHINode>(usr)) {
			if (DT.dominates(otherPhi, phi))
				return false;
		}
	}
	phi->replaceAllUsesWith(extract);
	toDelete.push_back(phi);

	/*
	 * The following should be sound for ARMv8 but let's be conservative
	 */

	// auto *term = llvm::dyn_cast<BranchInst>(cas->getParent()->getTerminator());
	// if (!term)
	// 	return true;

	// /* Try to replace replace the jump from the CAS block with an unconditional one */
	// tryThreadSuccessor(term, phi->getIncomingBlock(extractId));
	// if (term->isConditional() && term->getSuccessor(0) == term->getSuccessor(1)) {
	// 	BranchInst::Create(term->getSuccessor(0), cas->getParent());
	// 	/* Since we are gonna remove the whole PHI-predecessor, do not
	// 	 * remove the PHI as this will be taken care of when we eliminate
	// 	 * the unreachable block. Instead, kill the old branch instruction */
	// 	toDelete.pop_back();
	// 	toDelete.push_back(term);
	// }
	return true;
}

auto EliminateCASPHIsPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	std::vector<Instruction *> toDelete;
	auto modified = false;

	auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
	for (auto &bb : F) {
		for (auto it = bb.begin(); auto *phi = llvm::dyn_cast<llvm::PHINode>(it); ++it)
			modified |= tryEliminateCASPHI(phi, DT, toDelete);
	}

	EliminateUnreachableBlocks(F);
	std::ranges::for_each(toDelete, [](Instruction *i) { i->eraseFromParent(); });
	return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
