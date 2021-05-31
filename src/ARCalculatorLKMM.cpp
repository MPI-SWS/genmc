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

#include "ARCalculatorLKMM.hpp"
#include "PROPCalculator.hpp"

void ARCalculatorLKMM::initCalc()
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &ar = g.getGlobalRelation(ExecutionGraph::RelationId::ar_lkmm);

	/* ar should have the same events as prop -- runs after prop inits */
	auto events = prop.getElems();

	ar = Calculator::GlobalRelation(std::move(events));
	g.populatePPoRfEntries(ar);
	ar.transClosure();
	return;
}

bool ARCalculatorLKMM::addArConstraints()
{
	auto &g = getGraph();
	auto &ar = g.getGlobalRelation(ExecutionGraph::RelationId::ar_lkmm);
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &elems = ar.getElems();

	bool changed = false;
	for (auto &e1 : elems) {
		/* Add prop_i constraints (not part of pporf views) */
		for (auto &e2 : elems) {
			if (e1.thread == e2.thread && prop(e1, e2) && !ar(e1, e2)) {
				changed = true;
				ar.addEdge(e1, e2);
			}
		}
	}
	return changed;
}

Calculator::CalculationResult ARCalculatorLKMM::doCalc()
{
	auto &g = getGraph();
	auto &ar = g.getGlobalRelation(ExecutionGraph::RelationId::ar_lkmm);

	auto changed = addArConstraints();
	ar.transClosure();

	return Calculator::CalculationResult(changed, ar.isIrreflexive());
}

void ARCalculatorLKMM::removeAfter(const VectorClock &preds)
{
}

void ARCalculatorLKMM::restorePrefix(const ReadLabel *rLab,
				 const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
				 const std::vector<std::pair<Event, Event> > &status)
{
}
