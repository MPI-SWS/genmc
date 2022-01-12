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

#include "RCUCalculator.hpp"

void RCUCalculator::initCalc()
{
	auto &g = getGraph();
	auto &rcuLink = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_link);
	auto &rcu = g.getGlobalRelation(ExecutionGraph::RelationId::rcu);


	rcu = Calculator::GlobalRelation(rcuLink.getElems());
	return;
}

void RCUCalculator::incRcuCounter(Event e, unsigned int &gps, unsigned int &css) const
{
	auto &g = getGraph();
	auto *lab = g.getEventLabel(e);

	if (llvm::isa<RCUSyncLabelLKMM>(lab))
		++gps;
	if (llvm::isa<RCULockLabelLKMM>(lab))
		++css;
	return;
}

void RCUCalculator::decRcuCounter(Event e, unsigned int &gps, unsigned int &css) const
{
	auto &g = getGraph();
	auto *lab = g.getEventLabel(e);

	if (llvm::isa<RCUSyncLabelLKMM>(lab))
		--gps;
	if (llvm::isa<RCULockLabelLKMM>(lab))
		--css;
	return;
}

bool RCUCalculator::checkAddRcuConstraint(Event a, Event b, const unsigned int gps,
					  const unsigned int css)
{
	auto &rcu = getGraph().getGlobalRelation(ExecutionGraph::RelationId::rcu);
	bool changed = false;

	if (gps >= css && !rcu(a, b)) {
		changed = true;
		rcu.addEdge(a, b);
	}
	return changed;
}

bool RCUCalculator::addRcuConstraints()
{
	auto &g = getGraph();
	auto &rcuLink = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_link);
	auto &rcu = g.getGlobalRelation(ExecutionGraph::RelationId::rcu);
	auto &rcuElems = rcu.getElems();

	bool changed = false;
	for (auto e : rcuElems) {
		const EventLabel *lab = g.getEventLabel(e);
		unsigned int gps = 0;
		unsigned int css = 0;

		using NodeId = AdjList<Event, EventHasher>::NodeId;
		using Timestamp = AdjList<Event, EventHasher>::Timestamp;
		using NodeStatus = AdjList<Event, EventHasher>::NodeStatus;

		rcuLink.visitReachable(e,
			[&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f){ /* atEntryV */
				incRcuCounter(rcuElems[i], gps, css);
				return;
			},
			[&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f){ /* atTreeE */
				incRcuCounter(rcuElems[j], gps, css);
				changed |= checkAddRcuConstraint(e, rcuElems[j], gps, css);
				decRcuCounter(rcuElems[j], gps, css);
				return;
			},
			[&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f){ /* atBackE*/
				/* We shouldn't manipulate the counters in back edges,
				 * as such vertices have already been counted */
				changed |= checkAddRcuConstraint(e, rcuElems[j], gps, css);
				return;
			},
			[&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f){ /* atForwE*/
				incRcuCounter(rcuElems[j], gps, css);
				changed |= checkAddRcuConstraint(e, rcuElems[j], gps, css);
				decRcuCounter(rcuElems[j], gps, css);
				return;
			},
			[&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f){ /* atExitV*/
				decRcuCounter(rcuElems[i], gps, css);
			},
			[&](Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f){ return; }); /* atEnd */
	}
	return changed;
}

Calculator::CalculationResult RCUCalculator::doCalc()
{
	auto &g = getGraph();
	auto &rcu = g.getGlobalRelation(ExecutionGraph::RelationId::rcu);

	bool changed = addRcuConstraints();
	rcu.transClosure();
	return Calculator::CalculationResult(changed, rcu.isIrreflexive());
}

void RCUCalculator::removeAfter(const VectorClock &preds)
{
}
