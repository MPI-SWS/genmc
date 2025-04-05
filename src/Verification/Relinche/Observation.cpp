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
 * Author: Pavel Golovin <pgolovin@mpi-sws.org>
 */

#include "Verification/Relinche/Observation.hpp"
#include "ExecutionGraph/Consistency/ConsistencyChecker.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"

#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <utility>
#include <vector>

static auto isHbRelincheBefore(const ConsistencyChecker *checker, const EventLabel *aLab,
			       const EventLabel *bLab) -> bool
{
	const auto *nextB = aLab->getParent()->po_imm_succ(bLab);
	return nextB != nullptr && checker->getHbRelincheView(nextB).contains(aLab->getPos());
}

static auto isTotallyOrderedOp(const MethodCall &op, const auto &&ops,
			       const ConsistencyChecker *checker) -> bool
{
	return std::ranges::all_of(ops, [&](const auto &other) {
		return op == other || isHbRelincheBefore(checker, other.endLab, op.beginLab) ||
		       isHbRelincheBefore(checker, op.endLab, other.beginLab);
	});
}

/* Collect the set of method calls and non-method RF edges */
static void populateOpsRf(ExecutionGraph &g, std::vector<MethodCall> &ops,
			  VSet<std::pair<int, int>> &rfs)
{
	int thdKindIx = 0; // main thread has index zero
	int thdCopyIx = 0;

	int numOutsideEvents = 0;		       /* number of events outside methods */
	std::unordered_map<Event, int> outsideWrites;  /* Write event -> its index */
	std::vector<std::pair<Event, int>> outsideRFs; /* Write event -> index of Read event */

	for (auto tid : g.thr_ids()) {
		MethodBeginLabel *openBegin = nullptr;
		for (auto &lab : g.po(tid)) {
			if (auto *beginLab = llvm::dyn_cast<MethodBeginLabel>(&lab)) {
				ERROR_ON(openBegin, "Nested method calls are unsupported");
				openBegin = beginLab;
				continue;
			}
			if (auto *endLab = llvm::dyn_cast<MethodEndLabel>(&lab)) {
				ERROR_ON(!openBegin, "Unmatched method return in thread " +
							     std::to_string(tid));
				if (tid > 0) {
					if (ops.back().name == endLab->getName()) {
						++thdCopyIx;
					} else {
						++thdKindIx;
						thdCopyIx = 1;
					}
				}

				/* Method ids are going to be populated later */
				ops.emplace_back(MethodCall::Id(0), endLab->getName(),
						 openBegin->getArgument(), endLab->getResult(),
						 openBegin, endLab,
						 std::make_pair(thdKindIx, thdCopyIx));
				openBegin = nullptr;
				continue;
			}
			if (openBegin)
				continue;

			/* Warn if this is not the MPC */
			auto isNonMPC = (tid > 0 && !llvm::isa<ThreadStartLabel>(&lab) &&
					 !llvm::isa<ThreadFinishLabel>(&lab));
			WARN_ON_ONCE(isNonMPC, "non-mpc-found",
				     "The client is not the \"Most Parallel Client\".");

			if (const auto *rLab = llvm::dyn_cast<ReadLabel>(&lab)) {
				outsideRFs.emplace_back(rLab->getRf()->getPos(), numOutsideEvents);
			} else if (llvm::isa<WriteLabel>(lab)) {
				outsideWrites[lab.getPos()] = numOutsideEvents;
			}
			++numOutsideEvents;
		}
		ERROR_ON(openBegin, "No return label in thread " + std::to_string(tid));
	}

	/* Add non-method RF edges to the Observation */
	for (const auto &edge : outsideRFs)
		if (outsideWrites.contains(edge.first))
			rfs.insert(std::make_pair(outsideWrites.at(edge.first), edge.second));
}

static auto isSameOpWithSmallerUnwitnessedArgument(const std::vector<MethodCall> &ops,
						   const MethodCall &op1, const MethodCall &op2)
{
	return (op1.name == op2.name && op1.retVal == op2.retVal && op1.argVal < op2.argVal &&
		std::ranges::find(ops, op1.argVal, &MethodCall::retVal) == ops.end());
}

static auto findSameOpWithMinUnwitnessedArgument(std::vector<MethodCall> &ops,
						 std::vector<MethodCall>::iterator found)
	-> std::vector<MethodCall>::iterator
{
	for (auto it = ops.begin(); it != ops.end(); ++it) {
		if (isSameOpWithSmallerUnwitnessedArgument(ops, *it, *found))
			found = it;
	}
	return found;
}

/* Normalization exploits symmetries and data independence.
 * Pre: Must be called before populating the `hb` component. */
static void normalize(std::vector<MethodCall> &ops)
{
	/* Exploit data independence: Whenever an operation O with no arguments
	 * returns a value that is passed as an argument to exactly one other
	 * operation, make O return the smallest argument passed to a different
	 * instance of that operation that is not otherwise returned (by swapping
	 * the arguments).
	 * For example, convert `enq(1) enq(2) deq/2` into `enq(2) enq(1) deq/1`. */
	for (auto &op : ops) {
		if (op.argVal != 0 || op.retVal == 0)
			continue;
		if (std::ranges::count(ops, op.retVal, &MethodCall::argVal) != 1)
			continue;
		auto itW = std::ranges::find(ops, op.retVal, &MethodCall::argVal);
		auto itN = findSameOpWithMinUnwitnessedArgument(ops, itW);
		if (itN != itW) {
			op.retVal = itN->argVal;
			itN->argVal = itW->argVal;
			itW->argVal = op.retVal;
		}
	}
	/* Sort to take into account symmetry reduction */
	std::ranges::stable_sort(ops);
}

Observation::Observation(ExecutionGraph &g, const ConsistencyChecker *checker)
{
	/* Collect all method calls and non-method RF edges */
	populateOpsRf(g, ops_, rfs_);

	/* Optimization: remove operations that are HB-ordered with all other
	 * operations (e.g., initialization and finalization operations). */
	while (!ops_.empty() && isTotallyOrderedOp(*ops_.begin(), ops(), checker))
		ops_.erase(ops_.begin());

	/* Normalize operations to exploit symmetries and data independence.
	 * Has to be called before hb population; only applies for the MPC */
	if (rfs_.empty()) /* is MPC? */
		normalize(ops_);

	/* Assign ids to all calls; these are just their indices in ops_ */
	for (auto i = 0U; i < getNumOps(); i++)
		ops_[i].id = MethodCall::Id(i);
	/* Collect exposed HB */
	for (auto &op1 : ops())
		for (auto &op2 : ops())
			if (op1.id != op2.id &&
			    isHbRelincheBefore(checker, op1.beginLab, op2.endLab))
				hb_.insert(std::make_pair(op1.id, op2.id));
}

auto Observation::getContainingCallId(const EventLabel &lab) const -> std::optional<MethodCall::Id>
{
	const auto thread = lab.getPos().thread;
	const auto index = lab.getPos().index;

	auto opRange = ops();
	auto opIt = std::ranges::find_if(opRange, [&](auto &call) {
		return call.beginLab->getPos().thread == thread &&
		       call.beginLab->getPos().index <= index &&
		       call.endLab->getPos().index >= index;
	});
	return opIt != std::ranges::end(opRange) ? std::optional(opIt->id) : std::nullopt;
}

auto operator<<(llvm::raw_ostream &os, const Observation &obs) -> llvm::raw_ostream &
{
	os << "Ops:\n" << format(obs.ops_) << "\n";
	os << "RF:\n" << format(obs.rfs_) << "\n";
	os << "HB:\n" << format(obs.hb_) << "\n";
	return os;
}
