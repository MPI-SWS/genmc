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

#include "BisimilarityCheckerPass.hpp"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/Casting.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

using namespace llvm;
using BsPoint = BisimilarityAnalysis::BisimilarityPoint;
using Constraint = std::pair<Value *, Value *>;

/*
 * A possible bisimilarity point: point is a bisimilarity point
 * iff constraints.empty() holds
 */
struct ConstrainedBsPoint {
	BsPoint point;
	std::vector<Constraint> constraints;

	ConstrainedBsPoint(BsPoint point) : point(std::move(point)) {}
	ConstrainedBsPoint(BsPoint point, std::vector<Constraint> constrs)
		: point(point), constraints(std::move(constrs))
	{}
};

/* Given a list of candidates, returns the ones that are satisfiables */
static auto getSatisfiableCandidates(const std::vector<ConstrainedBsPoint> &candidates)
	-> std::vector<BsPoint>
{
	std::vector<BsPoint> bsPoints;

	/* If the constraints are not satisfiable, we shouldn't go further up */
	for (auto cit = candidates.begin(), cie = candidates.end();
	     cit != cie && cit->constraints.empty(); ++cit)
		bsPoints.push_back(cit->point);
	return bsPoints;
}

/*
 * Returns whether the bisimilarity point BSP will satisfy the constraint C.
 * (We can later refine this using something similar to FunctionComparator.)
 */
static auto solvesConstraint(const BsPoint &bsp, const Constraint &constraint) -> bool
{
	if (auto *inst1 = llvm::dyn_cast<Instruction>(constraint.first))
		if (auto *inst2 = llvm::dyn_cast<Instruction>(constraint.second))
			return std::make_pair(inst1, inst2) == bsp;
	return false;
}

/*
 * Given a list of constrained bisimilarity points (CANDIDATES) and a bisimilarity point (BSP),
 * filters from CANDIDATES all constraints that are satisfied by BSP.
 */
static void filterCandidateConstraints(BsPoint &bsp, const std::vector<Constraint> &constrs,
				       std::vector<ConstrainedBsPoint> &candidates)
{
	for (auto &cnd : candidates) {
		for (auto cit = cnd.constraints.begin(); cit != cnd.constraints.end();
		     /* empty */) {
			if (solvesConstraint(bsp, *cit)) {
				/* Remove the solved constraint */
				cit = cnd.constraints.erase(cit);
				/* Add the new ones */
				cit = cnd.constraints.insert(cit, constrs.begin(), constrs.end());
				cit += static_cast<std::ptrdiff_t>(constrs.size());
				if (constrs.empty() && cit != cnd.constraints.end())
					++cit;
			} else {
				++cit;
			}
		}
	}
	candidates.emplace_back(bsp, constrs);
}

static auto calcOperatorConstraints(Instruction *instA, Instruction *instB,
				    std::vector<Constraint> &constraints) -> bool
{
	std::vector<Constraint> constrs;

	for (auto i = 0U; i < instA->getNumOperands(); i++) {
		auto *opA = instA->getOperand(i);
		auto *opB = instB->getOperand(i);

		if (isa<Constant>(opA) || isa<BasicBlock>(opA)) {
			if (opA == opB)
				continue;
			return false;
		}
		constrs.emplace_back(opA, opB);
	}
	constraints.insert(constraints.end(), constrs.begin(), constrs.end());
	return true;
}

static void calcBsPointCandidates(Instruction *instA, Instruction *instB,
				  std::vector<ConstrainedBsPoint> &candidates)
{
	/* Make sure instA and instB are valid instructions */
	if (!instA || !instB)
		return;

	/* Candidate bisimilarity point */
	auto bsp = BsPoint(std::make_pair(instA, instB));
	std::vector<Constraint> constrs; /* to be populated */

	if (instA->isIdenticalTo(instB)) {
		/* Case 1: instA = instB */
		filterCandidateConstraints(bsp, constrs, candidates);
		calcBsPointCandidates(instA->getPrevNode(), instB->getPrevNode(), candidates);
	} else if (instA->isSameOperationAs(instB)) {
		/* Case 2: instA ~ instB */
		if (!calcOperatorConstraints(instA, instB, constrs))
			return;
		filterCandidateConstraints(bsp, constrs, candidates);
		calcBsPointCandidates(instA->getPrevNode(), instB->getPrevNode(), candidates);
	}
}

/* Returns the bisimilarity points of a function starting from (A, B)*/
static auto getBsPoints(Instruction *instA, Instruction *instB) -> std::vector<BsPoint>
{
	std::vector<ConstrainedBsPoint> candidates;
	calcBsPointCandidates(instA, instB, candidates);
	return getSatisfiableCandidates(candidates);
}

auto BisimilarityAnalysis::run(Function &F, FunctionAnalysisManager & /*FAM*/) -> Result
{
	funcBsPoints_.clear();
	for (auto &bb : F) {
		/* Only handle 2 preds for the time being (assumption used below) */
		if (std::distance(pred_begin(&bb), pred_end(&bb)) != 2)
			continue;

		auto *bb1 = *pred_begin(&bb);	  /* pred 1 */
		auto *bb2 = *(++pred_begin(&bb)); /* pred 2 */

		/* Skip if the predecessors are the same */
		if (bb1 == bb2)
			continue;

		/* Find bisimilar points and make sure they lead to the same state */
		auto points = getBsPoints(bb1->getTerminator(), bb2->getTerminator());
		bool sameState = true;
		for (auto iit = bb.begin(); auto *phi = llvm::dyn_cast<llvm::PHINode>(iit); ++iit) {
			auto bp1 = std::ranges::find_if(points, [&](const BsPoint &point) {
				return phi->getIncomingValue(0) == point.first;
			});
			auto bp2 = std::ranges::find_if(points, [&](const BsPoint &point) {
				return phi->getIncomingValue(0) == point.second;
			});
			if ((bp1 == points.end() && bp2 == points.end()) ||
			    (bp1 == points.end() && bp2->first != phi->getIncomingValue(1)) ||
			    (bp2 == points.end() && bp1->second != phi->getIncomingValue(1)))
				sameState = false;
		}
		if (sameState)
			funcBsPoints_.insert(funcBsPoints_.end(), points.begin(), points.end());
	}

	auto &bsps = funcBsPoints_;
	std::ranges::sort(bsps);
	bsps.erase(std::ranges::unique(bsps).begin(), bsps.end());

	return funcBsPoints_;
}

auto BisimilarityCheckerPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	bsps_ = FAM.getResult<BisimilarityAnalysis>(F);
	return PreservedAnalyses::all();
}
