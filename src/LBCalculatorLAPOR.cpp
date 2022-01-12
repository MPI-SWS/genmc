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

#include "LBCalculatorLAPOR.hpp"
#include "ExecutionGraph.hpp"

Event LBCalculatorLAPOR::getLastNonTrivialInCS(const Event lock) const
{
	const auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(lock);
	BUG_ON(!llvm::isa<LockLabelLAPOR>(lab));
	auto *lLab = static_cast<const LockLabelLAPOR *>(lab);

	auto i = 0u;
	for (i = lLab->getIndex() + 1u; i < g.getThreadSize(lLab->getThread()); i++) {
		const EventLabel *eLab = g.getEventLabel(Event(lLab->getThread(), i));

		/* If this is a matching unlock, break */
		if (auto *uLab = llvm::dyn_cast<UnlockLabelLAPOR>(eLab)) {
			if (uLab->getLockAddr() == lLab->getLockAddr())
				break;
		}
	}
	/* Else, we have reached the end of the thread; we will start backtracking */
	for (--i; i > lLab->getIndex(); i--) {
		auto e = Event(lab->getThread(), i);
		if (g.isNonTrivial(e))
			return e;
	}
	/* The lock label should be returned, if no other access is found */
	return lock;
}

Event LBCalculatorLAPOR::getFirstNonTrivialInCS(const Event lock) const
{
	const auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(lock);
	BUG_ON(!llvm::isa<LockLabelLAPOR>(lab));
	auto *lLab = static_cast<const LockLabelLAPOR *>(lab);

	for (auto i = lLab->getIndex() + 1u; i < g.getThreadSize(lLab->getThread()); i++) {
		const EventLabel *eLab = g.getEventLabel(Event(lLab->getThread(), i));

		if (g.isNonTrivial(eLab->getPos()))
			return eLab->getPos();

		/* If this is a matching unlock, break */
		if (auto *uLab = llvm::dyn_cast<UnlockLabelLAPOR>(eLab)) {
			if (uLab->getLockAddr() == lLab->getLockAddr())
				break;
		}
	}
	/* Since we did not find an appropriate event, we return the lock */
	return lock;
}

std::vector<Event> LBCalculatorLAPOR::getLbOrdering() const
{
	auto &g = getGraph();
	auto &lbRelation = g.getPerLocRelation(ExecutionGraph::RelationId::lb);
	std::vector<Event> threadPrios;

	for (auto &lbLoc : lbRelation) {
		auto topoLoc = lbLoc.second.topoSort();
		threadPrios.insert(threadPrios.end(), topoLoc.begin(), topoLoc.end());
	}

	/* Remove threads that are unordered with all other threads */
	threadPrios.erase(
		std::remove_if(threadPrios.begin(), threadPrios.end(), [&](Event e){
				auto *lab = g.getEventLabel(e);
				if (auto *lLab = llvm::dyn_cast<LockLabelLAPOR>(lab))
					return lbRelation[lLab->getLockAddr()].hasNoEdges(e);
				BUG();
			}),
		threadPrios.end());
	return threadPrios;
}

bool LBCalculatorLAPOR::addLbConstraints()
{
	auto &g = getGraph();
	auto &hbRelation = g.getGlobalRelation(ExecutionGraph::RelationId::hb);
	auto &lbRelation = g.getPerLocRelation(ExecutionGraph::RelationId::lb);
	bool changed = false;

	for (auto &lbLoc : lbRelation) {
		for (auto &l : lbLoc.second) {
			auto ul = getLastNonTrivialInCS(l);

			auto lIndex = lbLoc.second.getIndex(l);
			for (auto &o : lbLoc.second) {
				if (!lbLoc.second(lIndex, o))
					continue;
				auto ol = getFirstNonTrivialInCS(o);

				if (!hbRelation(ul, ol)) {
					changed = true;
					hbRelation.addEdge(ul, ol);
				}
			}
		}
	}
	return changed;
}

void LBCalculatorLAPOR::calcLbFromLoad(const ReadLabel *rLab,
				       const LockLabelLAPOR *lLab)
{
	auto &g = getGraph();
	auto &lbRelation = g.getPerLocRelation(ExecutionGraph::RelationId::lb);
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);
	const auto &co = coRelation[rLab->getAddr()];
	Event lock = lLab->getPos();

	Event rfLock = g.getLastThreadLockAtLocLAPOR(rLab->getRf(), lLab->getLockAddr());
	if (rfLock != Event::getInitializer() && rfLock != lock &&
	    rLab->getRf().index >= rfLock.index) {
		lbRelation[lLab->getLockAddr()].addEdge(rfLock, lock);
	}

	for (auto &s : co.getElems()) {
		if (!rLab->getRf().isInitializer() && !co(rLab->getRf(), s))
			continue;
		auto sLock = g.getLastThreadLockAtLocLAPOR(s, lLab->getLockAddr());
		if (sLock.isInitializer() || sLock == lock)
			continue;

		lbRelation[lLab->getLockAddr()].addEdge(lock, sLock);
	}
}

void LBCalculatorLAPOR::calcLbFromStore(const WriteLabel *wLab,
					const LockLabelLAPOR *lLab)
{
	auto &g = getGraph();
	auto &lbRelation = g.getPerLocRelation(ExecutionGraph::RelationId::lb);
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);
	const auto &co = coRelation[wLab->getAddr()];
	Event lock = lLab->getPos();

	/* Add an lb-edge if there exists a co-later store
	 * in a different critical section of lLab */
	auto &elems = co.getElems();
	auto labIndex = co.getIndex(wLab->getPos());
	for (auto it = co.adj_begin(labIndex), ei = co.adj_end(labIndex); it != ei; ++it) {
		auto sLock = g.getLastThreadLockAtLocLAPOR(elems[*it], lLab->getLockAddr());
		if (sLock.isInitializer() || sLock == lock)
			continue;

		lbRelation[lLab->getLockAddr()].addEdge(lock, sLock);
	}
}

void LBCalculatorLAPOR::calcLbRelation()
{
	auto &g = getGraph();
	auto &lbRelation = g.getPerLocRelation(ExecutionGraph::RelationId::lb);

	for (auto &lbLoc : lbRelation) {
		for (auto &l : lbLoc.second) {
			const EventLabel *lab = g.getEventLabel(l);
			BUG_ON(!llvm::isa<LockLabelLAPOR>(lab));
			auto *lLab = static_cast<const LockLabelLAPOR *>(lab);

			auto ul = getLastNonTrivialInCS(l);
			for (auto i = l.index; i <= ul.index; i++) { /* l.index >= 0 */
				const EventLabel *lab = g.getEventLabel(Event(l.thread, i));
				if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
					calcLbFromLoad(rLab, lLab);
				if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab))
					calcLbFromStore(wLab, lLab);
			}
		}
		lbLoc.second.transClosure();
	}
}

void LBCalculatorLAPOR::initCalc()
{
	auto &g = getGraph();
	auto &lbRelation = g.getPerLocRelation(ExecutionGraph::RelationId::lb);

	for (auto it = locks.begin(); it != locks.end(); ++it)
		lbRelation[it->first] = GlobalRelation(it->second);
	return;
}

Calculator::CalculationResult LBCalculatorLAPOR::doCalc()
{
	auto &g = getGraph();
	auto &hbRelation = g.getGlobalRelation(ExecutionGraph::RelationId::hb);
	auto &lbRelation = g.getPerLocRelation(ExecutionGraph::RelationId::lb);
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);

	if (!hbRelation.isIrreflexive())
		return Calculator::CalculationResult(false, false);
	calcLbRelation();
	for (auto &lbLoc : lbRelation) {
		if (!lbLoc.second.isIrreflexive())
			return Calculator::CalculationResult(false, false);
	}
	bool changed = addLbConstraints();
	hbRelation.transClosure();

	/* Check that no reads read from overwritten initialization write */
	const auto &elems = hbRelation.getElems();
	for (auto r = 0u; r < elems.size(); r++) {
		const EventLabel *lab = g.getEventLabel(elems[r]);

		if (!llvm::isa<ReadLabel>(lab))
			continue;

		auto *rLab = static_cast<const ReadLabel *>(lab);
		if (rLab->getRf() != Event::getInitializer())
			continue;

		auto &stores = coRelation[rLab->getAddr()].getElems();
		for (auto s : stores) {
			if (hbRelation(s, r))
				return Calculator::CalculationResult(changed, false);
		}
	}

	/* Check that co is acyclic, and that it does not contradict hb */
	for (auto &coLoc : coRelation) {
		if (!coLoc.second.isIrreflexive())
			return Calculator::CalculationResult(changed, false);

		for (auto &s1 : coLoc.second.getElems())
			for (auto &s2 : coLoc.second.getElems()) {
				if (coLoc.second(s1, s2) && hbRelation(s2, s1))
					return Calculator::CalculationResult(changed, false);
				if (coLoc.second(s1, s2) && g.isWriteRfBeforeRel(hbRelation, s2, s1))
					return Calculator::CalculationResult(changed, false);
				if (coLoc.second(s1, s2) && g.isHbOptRfBeforeRel(hbRelation, s2, s1))
					return Calculator::CalculationResult(changed, false);

				auto *sLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s2));
				BUG_ON(!sLab);
				if (coLoc.second(s1, s2) &&
				    std::any_of(sLab->readers_begin(), sLab->readers_end(),
						[&](Event r)
						{ return g.isHbOptRfBeforeRel(hbRelation, r, s1); }))
					return Calculator::CalculationResult(changed, false);
			}

	}
	return Calculator::CalculationResult(changed, true);
}

void LBCalculatorLAPOR::removeAfter(const VectorClock &preds)
{
	for (auto it = locks.begin(); it != locks.end(); ++it)
		it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
						[&](Event &e)
						{ return !preds.contains(e); }),
				 it->second.end());
	return;
}

void LBCalculatorLAPOR::addLockToList(SAddr addr, const Event lock)
{
	locks[addr].push_back(lock);
}
