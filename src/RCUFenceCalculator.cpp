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

#include "RCUFenceCalculator.hpp"
#include "PROPCalculator.hpp"

void RCUFenceCalculator::initCalc()
{
	auto &g = getGraph();
	auto &rcuFence = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_fence);

	auto events = g.collectAllEvents([&](const EventLabel* lab){
						 /* Also collect non-atomic events for rcu-fence */
						 return PROPCalculator::isNonTrivial(lab);
					 });

	rcuFence = Calculator::GlobalRelation(std::move(events));
	return;
}

bool RCUFenceCalculator::checkAddRcuFenceConstraint(Event a, Event b)
{
	auto &g = getGraph();
	auto &rcufence = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_fence);

	bool changed = false;
	auto toLimitA = llvm::isa<RCUSyncLabelLKMM>(g.getEventLabel(a)) ? a.index :
		g.getMatchingRCUUnlockLKMM(a).index;
	for (auto i = 1; i < toLimitA; i++) {
		auto *labA = g.getEventLabel(Event(a.thread, i));
		if (!PROPCalculator::isNonTrivial(labA))
			continue;
		for (auto j = b.index; j < g.getThreadSize(b.thread); j++) {
			auto *labB = g.getEventLabel(Event(b.thread, j));
			if (!PROPCalculator::isNonTrivial(labB))
				continue;
			if (!rcufence(labA->getPos(), labB->getPos())) {
				changed = true;
				rcufence.addEdge(labA->getPos(), labB->getPos());
			}
		}
	}
	return changed;
}

bool RCUFenceCalculator::addRcuFenceConstraints()
{
	auto &g = getGraph();
	auto &rcu = g.getGlobalRelation(ExecutionGraph::RelationId::rcu);
	auto &rcuElems = rcu.getElems();

	if (rcuElems.empty())
		return false;

	bool changed = false;
	for (auto r : rcuElems) {
		for (auto it = rcu.adj_begin(r), ie = rcu.adj_end(r); it != ie; ++it)
			changed |= checkAddRcuFenceConstraint(r, rcuElems[*it]);
	}
	return changed;
}

Calculator::CalculationResult RCUFenceCalculator::doCalc()
{
	auto &g = getGraph();
	auto &rcuFence = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_fence);

	bool changed = addRcuFenceConstraints();
	return Calculator::CalculationResult(changed, rcuFence.isIrreflexive());
}

void RCUFenceCalculator::removeAfter(const VectorClock &preds)
{
}

void RCUFenceCalculator::restorePrefix(const ReadLabel *rLab,
				 const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
				 const std::vector<std::pair<Event, Event> > &status)
{
}
