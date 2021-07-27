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

#include "BisimilarityCheckerPass.hpp"
#include "Error.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;
using BsPoint = BisimilarityCheckerPass::BisimilarityPoint;
using Constraint = std::pair<Value *, Value *>;

/*
 * A possible bisimilarity point: p is a bisimilarity point
 * iff constraints.empty() holds
 */
struct ConstrainedBsPoint {
	BsPoint p;
	std::vector<Constraint> constraints;

	ConstrainedBsPoint(BsPoint p) : p(p), constraints() {}
	ConstrainedBsPoint(BsPoint p, const std::vector<Constraint> &cs) : p(p), constraints(cs) {}
};

void BisimilarityCheckerPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	au.setPreservesAll();
}

bool BisimilarityCheckerPass::doInitialization(Module &M)
{
	funcBsPoints.clear();
	return false;
}

/* Given a list of candidates, returns the ones that are satisfiables */
std::vector<BsPoint> getSatisfiableCandidates(const std::vector<ConstrainedBsPoint> &candidates)
{
	std::vector<BsPoint> bsPoints;

	/* If the constraints are not satisfiable, we shouldn't go further up */
	for (auto cit = candidates.begin(), cie = candidates.end();
	     cit != cie && cit->constraints.empty(); ++cit)
		bsPoints.push_back(cit->p);
	return bsPoints;
}

/*
 * Returns whether the bisimilarity point BSP will satisfy the constraint C.
 * (We can later refine this using something similar to FunctionComparator.)
 */
bool solvesConstraint(const BsPoint &bsp, const Constraint &c)
{
	if (auto *c1 = llvm::dyn_cast<Instruction>(c.first))
		if (auto *c2 = llvm::dyn_cast<Instruction>(c.second))
			return std::make_pair(c1, c2) == bsp;
	return false;
}

/*
 * Given a list of constrained bisimilarity points (CANDIDATES) and a bisimilarity point (BSP),
 * filters from CANDIDATES all constraints that are satisfied by BSP.
 */
void filterCandidateConstraints(BsPoint &bsp, const std::vector<Constraint> &cs,
				std::vector<ConstrainedBsPoint> &candidates)
{
	for (auto &cnd : candidates) {
		for (auto cit = cnd.constraints.begin(); cit != cnd.constraints.end(); /* empty */) {
			if (solvesConstraint(bsp, *cit)) {
				/* Remove the solved constraint */
				cit = cnd.constraints.erase(cit);
				/* Add the new ones */
				cit = cnd.constraints.insert(cit, cs.begin(), cs.end());
				cit += cs.size();
				if (cs.empty() && cit != cnd.constraints.end())
					++cit;
			} else {
				++cit;
			}
		}
	}
	candidates.push_back(ConstrainedBsPoint(bsp, cs));
	return;
}

bool calcOperatorConstraints(Instruction *a, Instruction *b, std::vector<Constraint> &constraints)
{
	std::vector<Constraint> cs;

	for (auto i = 0u; i < a->getNumOperands(); i++) {
		auto *opA = a->getOperand(i);
		auto *opB = b->getOperand(i);

		if (isa<Constant>(opA) || isa<BasicBlock>(opA)) {
			if (opA == opB)
				continue;
			return false;
		}
		cs.push_back(std::make_pair(opA, opB));
	}
	constraints.insert(constraints.end(), cs.begin(), cs.end());
	return true;
}

void calcBsPointCandidates(Instruction *a,
			   Instruction *b,
			   std::vector<ConstrainedBsPoint> &candidates)
{
	/* Make sure a and b are valid instructions */
	if (!a || !b)
		return;

	/* Candidate bisimilarity point */
	auto bsp = BsPoint(std::make_pair(a, b));
	std::vector<Constraint> cs; /* to be populated */

	if (a->isIdenticalTo(b)) {
		/* Case 1: a = b */
		filterCandidateConstraints(bsp, cs, candidates);
		calcBsPointCandidates(a->getPrevNode(), b->getPrevNode(), candidates);
	} else if (a->isSameOperationAs(b)) {
		/* Case 2: a ~ b */
		if (!calcOperatorConstraints(a, b, cs))
			return;
		filterCandidateConstraints(bsp, cs, candidates);
		calcBsPointCandidates(a->getPrevNode(), b->getPrevNode(), candidates);
	}
	return;
}

/* Returns the bisimilarity points of a function starting from (A, B)*/
std::vector<BsPoint> getBsPoints(Instruction *a, Instruction *b)
{
	std::vector<ConstrainedBsPoint> candidates;
	calcBsPointCandidates(a, b, candidates);
	return getSatisfiableCandidates(candidates);
}

bool BisimilarityCheckerPass::runOnFunction(Function &F)
{
	for (auto bit = F.begin(), be = F.end(); bit != be; ++bit) {
		/* Only handle 2 preds for the time being (assumption used below) */
		if (std::distance(pred_begin(&*bit), pred_end(&*bit)) != 2)
			continue;

		auto b1 = *pred_begin(&*bit);     /* pred 1 */
		auto b2 = *(++pred_begin(&*bit)); /* pred 2 */

		/* Skip if the predecessors are the same */
		if (b1 == b2)
			continue;

		/* Find bisimilar points and make sure they lead to the same state */
		auto ps = getBsPoints(b1->getTerminator(), b2->getTerminator());
		bool sameState = true;
		for (auto iit = bit->begin(); auto phi = llvm::dyn_cast<llvm::PHINode>(iit); ++iit) {
			auto bp1 = std::find_if(ps.begin(), ps.end(),
					       [&](const BsPoint &p){ return phi->getIncomingValue(0) == p.first; });
			auto bp2 = std::find_if(ps.begin(), ps.end(),
					       [&](const BsPoint &p){ return phi->getIncomingValue(0) == p.second; });
			if ((bp1 == ps.end() && bp2 == ps.end()) ||
			    (bp1 == ps.end() && bp2->first != phi->getIncomingValue(1)) ||
			    (bp2 == ps.end() && bp1->second != phi->getIncomingValue(1)))
				sameState = false;
		}
		if (sameState)
			funcBsPoints[&F].insert(funcBsPoints[&F].end(), ps.begin(), ps.end());
	}

	auto &bsps = funcBsPoints[&F];
	std::sort(bsps.begin(), bsps.end());
	bsps.erase(std::unique(bsps.begin(), bsps.end()), bsps.end());

	return false;
}

FunctionPass *createBisimilarityCheckerPass()
{
	return new BisimilarityCheckerPass();
}

char BisimilarityCheckerPass::ID = 42;
static llvm::RegisterPass<BisimilarityCheckerPass> P("bisimilarity-checker",
						     "Calculates bisimilar points in all functions.");
