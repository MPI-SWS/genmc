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

#include "EliminateCASPHIsPass.hpp"
#include "Error.hpp"
#include "LLVMUtils.hpp"
#include "VSet.hpp"
#include "config.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using namespace llvm;

void EliminateCASPHIsPass::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.addRequired<DominatorTreeWrapperPass>();
}

ExtractValueInst *hasPHIIncomingExtract(llvm::PHINode *phi)
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
bool tryEliminateCASPHI(llvm::PHINode *phi, DominatorTree &DT, std::vector<Instruction *> &toDelete)
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
	auto extractId = (otherId == 0);
	auto *other = phi->getIncomingValue(otherId);
	if (cas->getCompareOperand() != other)
		return false;

	/* Finally, the compare operand should be taken when we are coming from the CAS's block
	 * (success) */
	if (phi->getIncomingBlock(otherId) != cas->getParent())
		return false;

	/* We know it's eliminable, so go ahead and replace all uses.
	 * However, also check if we can simplify the CFG a bit */
	for (auto *u : phi->users()) {
		if (auto *p = dyn_cast<PHINode>(u)) {
			if (DT.dominates(p, phi))
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

bool EliminateCASPHIsPass::runOnFunction(Function &F)
{
	std::vector<Instruction *> toDelete;
	auto modified = false;

	auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
	for (auto &BB : F) {
		for (auto it = BB.begin(); auto phi = llvm::dyn_cast<llvm::PHINode>(it); ++it)
			modified |= tryEliminateCASPHI(phi, DT, toDelete);
	}

	EliminateUnreachableBlocks(F);
	std::for_each(toDelete.begin(), toDelete.end(),
		      [](Instruction *i) { i->eraseFromParent(); });
	return modified;
}

Pass *createEliminateCASPHIsPass()
{
	auto *p = new EliminateCASPHIsPass();
	return p;
}

char EliminateCASPHIsPass::ID = 42;
static llvm::RegisterPass<EliminateCASPHIsPass> P("elim-cas-phis",
						  "Eliminates certain CAS-related PHIs.");
