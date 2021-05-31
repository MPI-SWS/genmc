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

#include "XBCalculator.hpp"
#include "PROPCalculator.hpp"
#include "RCUCalculator.hpp"
#include <llvm/ADT/SmallVector.h>

void XBCalculator::initCalc()
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &xb = g.getGlobalRelation(ExecutionGraph::RelationId::xb);
	auto *propCalc = static_cast<PROPCalculator *>(g.getCalculator(ExecutionGraph::RelationId::prop));

	/* Xb should have the same events as prop -- runs after prop inits */
	auto events = prop.getElems();

	xb = Calculator::GlobalRelation(std::move(events));
	g.populatePPoRfEntries(xb);
	xb.transClosure();
	return;
}

bool XBCalculator::addXbConstraints()
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &ar = g.getGlobalRelation(ExecutionGraph::RelationId::ar_lkmm);
	auto &pb = g.getGlobalRelation(ExecutionGraph::RelationId::pb);
	auto &rcufence = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_fence);
	auto &xb = g.getGlobalRelation(ExecutionGraph::RelationId::xb);
	auto &elems = xb.getElems();

	bool changed = false;
	for (auto &e1 : elems) {
		/* Add ar, pb and rb constraints */
		for (auto &e2 : elems) {
			if (!xb(e1, e2) && (ar(e1, e2) || pb(e1, e2) || rcufence(e1, e2))) {
				changed = true;
				xb.addEdge(e1, e2);
			}
		}
	}
	return changed;
}

Calculator::CalculationResult XBCalculator::doCalc()
{
	auto &g = getGraph();
	auto &xb = g.getGlobalRelation(ExecutionGraph::RelationId::xb);

	auto changed = addXbConstraints();
	xb.transClosure();

	return Calculator::CalculationResult(changed, xb.isIrreflexive());
}

void XBCalculator::removeAfter(const VectorClock &preds)
{
}

void XBCalculator::restorePrefix(const ReadLabel *rLab,
				 const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
				 const std::vector<std::pair<Event, Event> > &status)
{
}
