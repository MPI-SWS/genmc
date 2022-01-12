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

#include "RCULinkCalculator.hpp"
#include "PROPCalculator.hpp"

void RCULinkCalculator::initCalc()
{
	auto &g = getGraph();
	auto &rcu = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_link);

	/* Only collect synchronize_rcu()s and rcu_read_lock()s */
	auto rcuEvents = g.collectAllEvents([&](const EventLabel* lab){
						    return llvm::isa<RCUSyncLabelLKMM>(lab) ||
							   llvm::isa<RCULockLabelLKMM>(lab);
					    });

	rcu = Calculator::GlobalRelation(std::move(rcuEvents));
	return;
}

/* Returns true if E links to R.
 * R should be a synchronize_rcu() (F-type) or a rcu_read_lock() (L-type) event.
 *  - An event links to an F-type event if it is po-before it.
 *  - An event links to an L-type event if it is po-before its matching unlock.
 **/
bool RCULinkCalculator::linksTo(Event e, Event r) const
{
	auto &g = getGraph();
	auto *lab = g.getEventLabel(r);

	if (llvm::isa<RCUSyncLabelLKMM>(lab)) {
		return e.thread == r.thread && e.index < r.index;
	} else if (llvm::isa<RCULockLabelLKMM>(lab)) {
		auto ul = g.getMatchingRCUUnlockLKMM(r);
		return e.thread == ul.thread && e.index < ul.index;
	}
	BUG();
}

/* Fetches rcu links that are (pb*;prop;po)-after e1 */
std::vector<Event> RCULinkCalculator::getPbOptPropPoLinks(Event e1) const
{
	auto &g = getGraph();
	auto &ar = g.getGlobalRelation(ExecutionGraph::RelationId::ar_lkmm);
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &pb = g.getGlobalRelation(ExecutionGraph::RelationId::pb);
	auto &rcuLink = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_link);
	auto &elems = prop.getElems();
	auto &candidates = rcuLink.getElems(); /* link candidates */
	std::vector<Event> links;

	for (auto e2 : elems) {
		/* First, add prop;po edges */
		if (prop(e1, e2)) {
			std::copy_if(candidates.begin(), candidates.end(), std::back_inserter(links),
				     [&](Event r){ return linksTo(e2, r); });
		}
		/* Then, add pb*;prop;po edges */
		if (!pb(e1, e2))
			continue;
		for (auto e3 : elems) {
			if (prop(e2, e3)) {
				std::copy_if(candidates.begin(), candidates.end(), std::back_inserter(links),
					     [&](Event r){ return linksTo(e3, r); });
			} else if (ar(e2, e3)) {
				for (auto e4 : elems) {
					if (prop(e3, e4))
						std::copy_if(candidates.begin(), candidates.end(), std::back_inserter(links),
							     [&](Event r){ return linksTo(e4, r); });
				}
			}
		}
	}
	return links;
}

bool RCULinkCalculator::addRcuLinks(Event e)
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &ar = g.getGlobalRelation(ExecutionGraph::RelationId::ar_lkmm);
	auto &rcuLink = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_link);
	auto &pb = g.getGlobalRelation(ExecutionGraph::RelationId::pb);
	auto &elems = prop.getElems();
	bool changed = false;

	/* Calculate the upper limit in po until which we will look for links */
	auto *lab = g.getEventLabel(e);
	auto upperLimit = (llvm::isa<RCULockLabelLKMM>(lab)) ?
		g.getMatchingRCUUnlockLKMM(e) : g.getLastThreadEvent(e.thread).next();

	/* Start looking for links */
	for (auto i = e.index + 1; i < upperLimit.index; i++) {
		auto *lab = g.getEventLabel(Event(e.thread, i));
		if (!PROPCalculator::isNonTrivial(lab) || lab->isNotAtomic())
			continue;

		auto links = getPbOptPropPoLinks(Event(e.thread, i));
		for (auto ev : elems) {
			if (ev != lab->getPos() && ar(lab->getPos(), ev)) {
				auto arLinks = getPbOptPropPoLinks(ev);
				links.insert(links.end(), arLinks.begin(), arLinks.end());
			}
		}
		for (auto l : links) {
			if (!rcuLink(e, l)) {
				changed = true;
				rcuLink.addEdge(e, l);
			}
		}
	}
	return changed;
}

bool RCULinkCalculator::addRcuLinkConstraints()
{
	auto &g = getGraph();
	auto &rcuElems = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_link).getElems();
	bool changed = false;

	for (auto e : rcuElems)
		changed |= addRcuLinks(e);
	return changed;
}

Calculator::CalculationResult RCULinkCalculator::doCalc()
{
	auto &g = getGraph();
	auto &rcuLink = g.getGlobalRelation(ExecutionGraph::RelationId::rcu_link);

	bool changed = addRcuLinkConstraints();
	return Calculator::CalculationResult(changed, true);
}

void RCULinkCalculator::removeAfter(const VectorClock &preds)
{
}
