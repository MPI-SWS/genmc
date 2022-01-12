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

#include "PBCalculator.hpp"
#include "PROPCalculator.hpp"

void PBCalculator::initCalc()
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &pb = g.getGlobalRelation(ExecutionGraph::RelationId::pb);

	/* pb should have the same events as prop and ar */
	auto events = prop.getElems();
	pb = Calculator::GlobalRelation(std::move(events));
	return;
}

bool PBCalculator::addPbConstraints()
{
	auto &g = getGraph();
	auto &ar = g.getGlobalRelation(ExecutionGraph::RelationId::ar_lkmm);
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &pb = g.getGlobalRelation(ExecutionGraph::RelationId::pb);
	auto *propCalc = static_cast<PROPCalculator *>(g.getCalculator(ExecutionGraph::RelationId::prop));
	auto &elems = ar.getElems();
	auto &strongFences = propCalc->getStrongFences();

	bool changed = false;
	for (auto &e1 : elems) {
		/* Add prop;strong-fence;ar* constraints (part of PB) */
		for (auto &e2 : elems) {
			if (!prop(e1, e2))
				continue;
			for (auto &e3 : elems) {
				if (!pb(e1, e3) && std::any_of(strongFences.begin(), strongFences.end(),
							       [&](Event f){ return f.isBetween(e2, e3); })) {
					changed = true;
					pb.addEdge(e1, e3);
				}
			}
		}
	}
	return changed;
}

Calculator::CalculationResult PBCalculator::doCalc()
{
	auto &g = getGraph();
	auto &pb = g.getGlobalRelation(ExecutionGraph::RelationId::pb);

	auto changed = addPbConstraints();
	pb.transClosure();

	return Calculator::CalculationResult(changed, pb.isIrreflexive());
}

void PBCalculator::removeAfter(const VectorClock &preds)
{
}
