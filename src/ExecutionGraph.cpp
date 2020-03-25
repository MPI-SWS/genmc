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

#include "ExecutionGraph.hpp"
#include "Library.hpp"
#include "MOCoherenceCalculator.hpp"
#include "Parser.hpp"
#include "WBCoherenceCalculator.hpp"
#include <llvm/IR/DebugInfo.h>

/************************************************************
 ** Class Constructors
 ***********************************************************/

ExecutionGraph::ExecutionGraph() : timestamp(1)
{
	/* Create an entry for main() and push the "initializer" label */
	events.push_back({});
	events[0].push_back( std::unique_ptr<ThreadStartLabel>(
				     new ThreadStartLabel(
					     0, llvm::AtomicOrdering::Acquire,
					     Event(0, 0),
					     Event::getInitializer() )
				     ) );

	globalRelations.push_back(Calculator::GlobalRelation());
	globalRelationsCache.push_back(Calculator::GlobalRelation());
	relationIndex[RelationId::hb] = 0;
	calculatorIndex[RelationId::hb] = -42; /* no calculator for hb */
	return;
}


/************************************************************
 ** Basic getter methods
 ***********************************************************/

unsigned int ExecutionGraph::nextStamp()
{
	return timestamp++;
}

const EventLabel *ExecutionGraph::getEventLabel(Event e) const
{
	return events[e.thread][e.index].get();
}

const EventLabel *ExecutionGraph::getPreviousLabel(Event e) const
{
	return events[e.thread][e.index - 1].get();
}

const EventLabel *ExecutionGraph::getPreviousLabel(const EventLabel *lab) const
{
	return events[lab->getThread()][lab->getIndex() - 1].get();
}

const EventLabel *ExecutionGraph::getPreviousNonEmptyLabel(Event e) const
{
	for (int i = e.index - 1; i > 0; i--) {
		const EventLabel *eLab = getEventLabel(Event(e.thread, i));
		if (!llvm::isa<EmptyLabel>(eLab))
			return eLab;
	}
	return getEventLabel(Event(e.thread, 0));
}

const EventLabel *ExecutionGraph::getPreviousNonEmptyLabel(const EventLabel *lab) const
{
	return getPreviousNonEmptyLabel(lab->getPos());
}

Event ExecutionGraph::getPreviousNonTrivial(const Event e) const
{
	for (auto i = e.index - 1; i >= 0; i--) {
		if (isNonTrivial(Event(e.thread, i)))
			return Event(e.thread, i);
	}
	return Event::getInitializer();
}

const EventLabel *ExecutionGraph::getLastThreadLabel(int thread) const
{
	return events[thread][events[thread].size() - 1].get();
}

Event ExecutionGraph::getLastThreadEvent(int thread) const
{
	return Event(thread, events[thread].size() - 1);
}

Event ExecutionGraph::getLastThreadReleaseAtLoc(Event upperLimit,
						const llvm::GenericValue *addr) const
{
	for (int i = upperLimit.index - 1; i > 0; i--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, i));
		if (llvm::isa<ThreadCreateLabel>(lab) || llvm::isa<ThreadFinishLabel>(lab) ||
		    llvm::isa<UnlockLabelLAPOR>(lab)) {
			return Event(upperLimit.thread, i);
		}
		if (auto *fLab = llvm::dyn_cast<FenceLabel>(lab)) {
			if (fLab->isAtLeastRelease())
				return Event(upperLimit.thread, i);
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
			if (wLab->isAtLeastRelease() && wLab->getAddr() == addr)
				return Event(upperLimit.thread, i);
		}
	}
	return Event(upperLimit.thread, 0);
}

Event ExecutionGraph::getLastThreadRelease(Event upperLimit) const
{
	for (int i = upperLimit.index - 1; i > 0; i--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, i));
		if (llvm::isa<ThreadCreateLabel>(lab) || llvm::isa<ThreadFinishLabel>(lab) ||
		    llvm::isa<UnlockLabelLAPOR>(lab)) {
			return Event(upperLimit.thread, i);
		}
		if (auto *fLab = llvm::dyn_cast<FenceLabel>(lab)) {
			if (fLab->isAtLeastRelease())
				return Event(upperLimit.thread, i);
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
			if (wLab->isAtLeastRelease())
				return Event(upperLimit.thread, i);
		}
	}
	return Event(upperLimit.thread, 0);
}

/* Assumes that all events prior to upperLimit have already been added */
std::vector<Event> ExecutionGraph::getThreadAcquiresAndFences(Event upperLimit) const
{
	std::vector<Event> result;

	result.push_back(Event(upperLimit.thread, 0));
	for (int i = 1u; i < upperLimit.index; i++) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, i));
		if (llvm::isa<ThreadJoinLabel>(lab) || llvm::isa<LockLabelLAPOR>(lab))
			result.push_back(lab->getPos());
		if (auto *fLab = llvm::dyn_cast<FenceLabel>(lab))
			result.push_back(lab->getPos());
		if (auto *wLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (wLab->isAtLeastAcquire())
				result.push_back(lab->getPos());
		}
	}
	return result;
}

Event ExecutionGraph::getMatchingLock(const Event unlock) const
{
	std::vector<Event> locUnlocks;

	const EventLabel *unlockL = getEventLabel(unlock);
	BUG_ON(!llvm::isa<WriteLabel>(unlockL));
	auto *uLab = static_cast<const WriteLabel *>(unlockL);
	BUG_ON(!uLab->isUnlock());

	for (auto j = unlock.index - 1; j > 0; j--) {
		const EventLabel *lab = getEventLabel(Event(unlock.thread, j));

		/* In case support for reentrant locks is added... */
		if (auto *suLab = llvm::dyn_cast<WriteLabel>(lab)) {
			if (suLab->isUnlock() && suLab->getAddr() == uLab->getAddr())
				locUnlocks.push_back(suLab->getPos());
		}
		if (auto *lLab = llvm::dyn_cast<CasReadLabel>(lab)) {
			if (lLab->isLock() && lLab->getAddr() == uLab->getAddr()) {
				if (locUnlocks.empty())
					return lLab->getPos();
				else
					locUnlocks.pop_back();
			}
		}
	}
	return Event::getInitializer();
}

Event ExecutionGraph::getMatchingUnlock(const Event lock) const
{
	std::vector<Event> locLocks;

	const EventLabel *lockL = getEventLabel(lock);
	BUG_ON(!llvm::isa<CasReadLabel>(lockL));
	auto *lLab = static_cast<const CasReadLabel *>(lockL);
	BUG_ON(!lLab->isLock());

	for (auto j = lock.index + 1; j < getThreadSize(lock.thread); j++) {
		const EventLabel *lab = getEventLabel(Event(lock.thread, j));

		/* In case support for reentrant locks is added... */
		if (auto *slLab = llvm::dyn_cast<CasReadLabel>(lab)) {
			if (slLab->isLock() && slLab->getAddr() == lLab->getAddr())
				locLocks.push_back(slLab->getPos());
		}
		if (auto *uLab = llvm::dyn_cast<WriteLabel>(lab)) {
			if (uLab->isUnlock() && uLab->getAddr() == lLab->getAddr()) {
				if (locLocks.empty())
					return uLab->getPos();
				else
					locLocks.pop_back();
			}
		}
	}
	return Event::getInitializer();
}

Event ExecutionGraph::getLastThreadUnmatchedLockLAPOR(const Event upperLimit) const
{
	std::vector<const llvm::GenericValue *> unlocks;

	for (auto j = upperLimit.index; j >= 0; j--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, j));

		if (auto *lLab = llvm::dyn_cast<LockLabelLAPOR>(lab)) {
			if (std::find_if(unlocks.rbegin(), unlocks.rend(),
					 [&](const llvm::GenericValue *addr)
					 { return lLab->getLockAddr() == addr; })
			    ==  unlocks.rend())
				return lLab->getPos();
		}

		if (auto *uLab = llvm::dyn_cast<UnlockLabelLAPOR>(lab))
			unlocks.push_back(uLab->getLockAddr());
	}
	return Event::getInitializer();
}

Event ExecutionGraph::getMatchingUnlockLAPOR(const Event lock) const
{
	std::vector<Event> locLocks;

	const EventLabel *lockL = getEventLabel(lock);
	BUG_ON(!llvm::isa<LockLabelLAPOR>(lockL));
	auto *lLab = static_cast<const LockLabelLAPOR *>(lockL);

	for (auto j = lock.index + 1; j < getThreadSize(lock.thread); j++) {
		const EventLabel *lab = getEventLabel(Event(lock.thread, j));

		if (auto *slLab = llvm::dyn_cast<LockLabelLAPOR>(lab)) {
			if (slLab->getLockAddr() == lLab->getLockAddr())
				locLocks.push_back(slLab->getPos());
		}
		if (auto *uLab = llvm::dyn_cast<UnlockLabelLAPOR>(lab)) {
			if (uLab->getLockAddr() == lLab->getLockAddr()) {
				if (locLocks.empty())
					return uLab->getPos();
				else
					locLocks.pop_back();
			}
		}
	}
	return Event::getInitializer();
}

Event ExecutionGraph::getLastThreadLockAtLocLAPOR(const Event upperLimit,
						  const llvm::GenericValue *loc) const
{
	for (auto j = upperLimit.index; j >= 0; j--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, j));

		if (auto *lLab = llvm::dyn_cast<LockLabelLAPOR>(lab)) {
			if (lLab->getLockAddr() == loc)
				return lLab->getPos();
		}

	}
	return Event::getInitializer();
}

Event ExecutionGraph::getLastThreadUnlockAtLocLAPOR(const Event upperLimit,
						    const llvm::GenericValue *loc) const
{
	for (auto j = upperLimit.index; j >= 0; j--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, j));

		if (auto *lLab = llvm::dyn_cast<UnlockLabelLAPOR>(lab)) {
			if (lLab->getLockAddr() == loc)
				return lLab->getPos();
		}

	}
	return Event::getInitializer();
}

std::vector<Event> ExecutionGraph::getPendingRMWs(const WriteLabel *sLab) const
{
	std::vector<Event> pending;

	/* If this is _not_ an RMW event, return an empty vector */
	if (!llvm::isa<FaiWriteLabel>(sLab) && !llvm::isa<CasWriteLabel>(sLab))
		return pending;

	/* Otherwise, scan for other RMWs that successfully read the same val */
	const auto *pLab = static_cast<const ReadLabel *>(getPreviousLabel(sLab));
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 1u; j < getThreadSize(i); j++) { /* Skip thread start */
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!isRMWLoad(lab))
				continue;
			const auto *rLab = static_cast<const ReadLabel *>(lab);
			if (rLab->getRf() == pLab->getRf() &&
			    rLab->getAddr() == pLab->getAddr() &&
			    rLab->getPos() != pLab->getPos())
				pending.push_back(lab->getPos());
		}
	}
	BUG_ON(pending.size() > 1);
	return pending;
}

Event ExecutionGraph::getPendingLibRead(const LibReadLabel *lab) const
{
	/* Should only be called with a read of a functional library that doesn't read BOT */
	BUG_ON(lab->getRf().isInitializer());

	/* Get the conflicting label */
	const auto *sLab = static_cast<const WriteLabel *>(getEventLabel(lab->getRf()));
	auto it = std::find_if(sLab->getReadersList().begin(),
			       sLab->getReadersList().end(),
			       [&](const Event &e) { return e != lab->getPos(); });
	BUG_ON(it == sLab->getReadersList().end());
	return *it;
}

std::vector<Event> ExecutionGraph::getRevisitable(const WriteLabel *sLab) const
{
	std::vector<Event> loads;

	auto &before = getPorfBefore(sLab->getPos());
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = before[i] + 1u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getAddr() == sLab->getAddr() &&
				    rLab->isRevisitable())
					loads.push_back(rLab->getPos());
			}
		}
	}
	return loads;
}

/* Returns a vector with all reads of a particular location reading from INIT */
std::vector<Event> ExecutionGraph::getInitRfsAtLoc(const llvm::GenericValue *addr) const
{
	std::vector<Event> result;

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 1u; j < getThreadSize(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
				if (rLab->getRf().isInitializer() &&
				    rLab->getAddr() == addr)
					result.push_back(rLab->getPos());
		}
	}
	return result;
}


/*******************************************************************************
 **                       Label addition methods
 ******************************************************************************/

const ReadLabel *ExecutionGraph::addReadLabelToGraph(std::unique_ptr<ReadLabel> lab,
						     Event rf)
{
	EventLabel *rfLab = getEventLabel(rf);
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab)) {
		wLab->addReader(lab->getPos());
	}

	return static_cast<const ReadLabel *>(addOtherLabelToGraph(std::move(lab)));
}

const WriteLabel *ExecutionGraph::addWriteLabelToGraph(std::unique_ptr<WriteLabel> lab,
						     unsigned int offsetMO)
{
	getCoherenceCalculator()->addStoreToLoc(lab->getAddr(), lab->getPos(), offsetMO);
	return static_cast<const WriteLabel *>(addOtherLabelToGraph(std::move(lab)));
}

const WriteLabel *ExecutionGraph::addWriteLabelToGraph(std::unique_ptr<WriteLabel> lab,
						     Event pred)
{
	getCoherenceCalculator()->addStoreToLocAfter(lab->getAddr(), lab->getPos(), pred);
	return static_cast<const WriteLabel *>(addOtherLabelToGraph(std::move(lab)));
}

const LockLabelLAPOR *ExecutionGraph::addLockLabelToGraphLAPOR(std::unique_ptr<LockLabelLAPOR> lab)
{
	getLbCalculatorLAPOR()->addLockToList(lab->getLockAddr(), lab->getPos());
	return static_cast<const LockLabelLAPOR *>(addOtherLabelToGraph(std::move(lab)));
}

const EventLabel *ExecutionGraph::addOtherLabelToGraph(std::unique_ptr<EventLabel> lab)
{
	auto pos = lab->getPos();

	if (pos.index < events[pos.thread].size()) {
		events[pos.thread][pos.index] = std::move(lab);
	} else {
		events[pos.thread].push_back(std::move(lab));
	}
	BUG_ON(pos.index > events[pos.thread].size());
	return getEventLabel(pos);
}


/************************************************************
 ** Calculation of [(po U rf)*] predecessors and successors
 ***********************************************************/

void ExecutionGraph::addCalculator(std::unique_ptr<Calculator> cc, RelationId r,
				 bool perLoc, bool partial /* = false */)
{
	/* Add a calculator for this relation */
	auto calcSize = getCalcs().size();
	consistencyCalculators.push_back(std::move(cc));
	if (partial)
		partialConsCalculators.push_back(calcSize);

	/* Add a matrix for this relation */
	auto relSize = 0u;
	if (perLoc) {
		relSize = perLocRelations.size();
		perLocRelations.push_back(Calculator::PerLocRelation());
		perLocRelationsCache.push_back(Calculator::PerLocRelation());
	} else {
		relSize = globalRelations.size();
		globalRelations.push_back(Calculator::GlobalRelation());
		globalRelationsCache.push_back(Calculator::GlobalRelation());
	}

	/* Update indices trackers */
	calculatorIndex[r] = calcSize;
	relationIndex[r] = relSize;
}

Calculator::GlobalRelation& ExecutionGraph::getGlobalRelation(RelationId id)
{
	BUG_ON(relationIndex.count(id) == 0);
	return globalRelations[relationIndex[id]];
}

Calculator::PerLocRelation& ExecutionGraph::getPerLocRelation(RelationId id)
{
	BUG_ON(relationIndex.count(id) == 0);
	return perLocRelations[relationIndex[id]];
}

Calculator::GlobalRelation& ExecutionGraph::getCachedGlobalRelation(RelationId id)
{
	BUG_ON(relationIndex.count(id) == 0);
	return globalRelationsCache[relationIndex[id]];
}

Calculator::PerLocRelation& ExecutionGraph::getCachedPerLocRelation(RelationId id)
{
	BUG_ON(relationIndex.count(id) == 0);
	return perLocRelationsCache[relationIndex[id]];
}

void ExecutionGraph::cacheRelations(bool copy /* = true */)
{
	if (copy) {
		for (auto i = 0u; i < globalRelations.size(); i++)
			globalRelationsCache[i] = globalRelations[i];
		for (auto i = 0u; i < perLocRelations.size(); i++)
			perLocRelationsCache[i] = perLocRelations[i];
	} else {
		for (auto i = 0u; i < globalRelations.size(); i++)
			globalRelationsCache[i] = std::move(globalRelations[i]);
		for (auto i = 0u; i < perLocRelations.size(); i++)
			perLocRelationsCache[i] = std::move(perLocRelations[i]);
	}
	return;
}

void ExecutionGraph::restoreCached(bool move /* = false */)
{
	if (!move) {
		for (auto i = 0u; i < globalRelations.size(); i++)
			globalRelations[i] = globalRelationsCache[i];
		for (auto i = 0u; i < perLocRelations.size(); i++)
			perLocRelations[i] = perLocRelationsCache[i];
	} else {
		for (auto i = 0u; i < globalRelations.size(); i++)
			globalRelations[i] = std::move(globalRelationsCache[i]);
		for (auto i = 0u; i < perLocRelations.size(); i++)
			perLocRelations[i] = std::move(perLocRelationsCache[i]);
	}
	return;
}

Calculator *ExecutionGraph::getCalculator(RelationId id)
{
	return consistencyCalculators[calculatorIndex[id]].get();
}

CoherenceCalculator *ExecutionGraph::getCoherenceCalculator()
{
	return static_cast<CoherenceCalculator *>(
		consistencyCalculators[relationIndex[RelationId::co]].get());
}

CoherenceCalculator *ExecutionGraph::getCoherenceCalculator() const
{
	return static_cast<CoherenceCalculator *>(
		consistencyCalculators.at(relationIndex.at(RelationId::co)).get());
}

LBCalculatorLAPOR *ExecutionGraph::getLbCalculatorLAPOR()
{
	return static_cast<LBCalculatorLAPOR *>(
		consistencyCalculators[relationIndex[RelationId::lb]].get());
}

LBCalculatorLAPOR *ExecutionGraph::getLbCalculatorLAPOR() const
{
	return static_cast<LBCalculatorLAPOR *>(
		consistencyCalculators.at(relationIndex.at(RelationId::lb)).get());
}

std::vector<Event> ExecutionGraph::getLbOrderingLAPOR() const
{
	return getLbCalculatorLAPOR()->getLbOrdering();
}

const std::vector<Calculator *> ExecutionGraph::getCalcs() const
{
	std::vector<Calculator *> result;

	for (auto i = 0u; i < consistencyCalculators.size(); i++)
		result.push_back(consistencyCalculators[i].get());
	return result;
}

const std::vector<Calculator *> ExecutionGraph::getPartialCalcs() const
{
	std::vector<Calculator *> result;

	for (auto i = 0u; i < partialConsCalculators.size(); i++)
		result.push_back(consistencyCalculators[partialConsCalculators[i]].get());
	return result;
}

void ExecutionGraph::doInits(bool full /* = false */)
{
	auto &hb = globalRelations[relationIndex[RelationId::hb]];
	populateHbEntries(hb);
	hb.transClosure();

	auto &calcs = consistencyCalculators;
	auto &partial = partialConsCalculators;
	for (auto i = 0u; i < calcs.size(); i++) {
		if (!full && std::find(partial.begin(), partial.end(), i) == partial.end())
			continue;

		calcs[i]->initCalc();
	}
	return;
}

Calculator::CalculationResult ExecutionGraph::doCalcs(bool full /* = false */)
{
	Calculator::CalculationResult result;

	auto &calcs = consistencyCalculators;
	auto &partial = partialConsCalculators;
	for (auto i = 0u; i < calcs.size(); i++) {
		if (!full && std::find(partial.begin(), partial.end(), i) == partial.end())
			continue;

		result |= calcs[i]->doCalc();

		/* If an inconsistency was spotted, no reason to call
		 * the other calculators */
		if (!result.cons)
			return result;
	}
	return result;
}

void ExecutionGraph::trackCoherenceAtLoc(const llvm::GenericValue *addr)
{
	return getCoherenceCalculator()->trackCoherenceAtLoc(addr);
}

const std::vector<Event>&
ExecutionGraph::getStoresToLoc(const llvm::GenericValue *addr) const
{
	return getCoherenceCalculator()->getStoresToLoc(addr);
}

const std::vector<Event>&
ExecutionGraph::getStoresToLoc(const llvm::GenericValue *addr)
{
	return getCoherenceCalculator()->getStoresToLoc(addr);
}

std::pair<int, int>
ExecutionGraph::getCoherentPlacings(const llvm::GenericValue *addr,
				    Event pos, bool isRMW) {
	return getCoherenceCalculator()->getPossiblePlacings(addr, pos, isRMW);
};

std::vector<Event>
ExecutionGraph::getCoherentStores(const llvm::GenericValue *addr, Event pos)
{
	return getCoherenceCalculator()->getCoherentStores(addr, pos);
}

std::vector<Event>
ExecutionGraph::getCoherentRevisits(const WriteLabel *wLab)
{
	return getCoherenceCalculator()->getCoherentRevisits(wLab);
}

std::unique_ptr<VectorClock>
ExecutionGraph::getRevisitView(const ReadLabel *rLab,
			       const WriteLabel *wLab) const
{
	auto preds = llvm::make_unique<View>(getViewFromStamp(rLab->getStamp()));
	preds->update(wLab->getPorfView());
	return std::move(preds);
}

const DepView &ExecutionGraph::getPPoRfBefore(Event e) const
{
	return getEventLabel(e)->getPPoRfView();
}

const View &ExecutionGraph::getPorfBefore(Event e) const
{
	return getEventLabel(e)->getPorfView();
}

const View &ExecutionGraph::getHbBefore(Event e) const
{
	return getEventLabel(e)->getHbView();
}

View ExecutionGraph::getHbBefore(const std::vector<Event> &es) const
{
	View v;

	for (auto &e : es)
		v.update(getEventLabel(e)->getHbView());
	return v;
}

const View &ExecutionGraph::getHbPoBefore(Event e) const
{
	return getPreviousNonEmptyLabel(e)->getHbView();
}

#define IMPLEMENT_POPULATE_ENTRIES(MATRIX, GET_VIEW)			\
do {								        \
									\
        const std::vector<Event> &es = MATRIX.getElems();		\
        auto len = es.size();						\
									\
        for (auto i = 0u; i < len; i++) {				\
		for (auto j = 0u; j < len; j++) {			\
			if (es[i] != es[j] &&				\
			    GET_VIEW(es[j]).contains(es[i]))		\
				MATRIX.addEdge(i, j);			\
		}							\
        }								\
} while (0)

void ExecutionGraph::populatePorfEntries(AdjList<Event, EventHasher> &relation) const
{
	IMPLEMENT_POPULATE_ENTRIES(relation, getPorfBefore);
}

void ExecutionGraph::populatePPoRfEntries(AdjList<Event, EventHasher> &relation) const
{
	IMPLEMENT_POPULATE_ENTRIES(relation, getPPoRfBefore);
}

void ExecutionGraph::populateHbEntries(AdjList<Event, EventHasher> &relation) const
{
	std::vector<Event> elems;
	std::vector<std::pair<Event, Event> > edges;

	for (auto i = 0u; i < getNumThreads(); i++) {
		auto thrIdx = elems.size();
		for (auto j = 0u; j < getThreadSize(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (!isNonTrivial(lab))
				continue;

			auto labIdx = elems.size();
			elems.push_back(Event(i, j));

			if (labIdx == thrIdx) {
				auto *bLab = getEventLabel(Event(i, 0));
				BUG_ON(!llvm::isa<ThreadStartLabel>(bLab));

				auto parentLast = getPreviousNonTrivial(
					llvm::dyn_cast<ThreadStartLabel>(bLab)->getParentCreate());
				if (!parentLast.isInitializer())
					edges.push_back(std::make_pair(parentLast, elems[labIdx]));
			}
			if (labIdx > thrIdx)
				edges.push_back(std::make_pair(elems[labIdx - 1], elems[labIdx]));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (!rLab->getRf().isInitializer()) {
					auto pred = (labIdx > thrIdx) ?
						elems[labIdx - 1] : Event::getInitializer();
					auto &v = rLab->getHbView();
					auto &predV = getHbBefore(pred);
					for (auto k = 0u; k < v.size(); k++) {
						if (k != rLab->getThread() &&
						    v[k] > 0 &&
						    !predV.contains(Event(k, v[k]))) {
							auto cndt = getPreviousNonTrivial(Event(k, v[k]).next());
							if (cndt.isInitializer())
								continue;
							edges.push_back(std::make_pair(cndt, rLab->getPos()));
						}
					}
				}
			}
		}
	}
	relation = std::move(AdjList<Event, EventHasher>(std::move(elems)));
	for (auto &e : edges)
		relation.addEdge(e.first, e.second);
	return;
}


/************************************************************
 ** Calculation of particular sets of events/event labels
 ***********************************************************/

const VectorClock& ExecutionGraph::getPrefixView(Event e) const
{
	return getPorfBefore(e);
}

std::unique_ptr<VectorClock> ExecutionGraph::getPredsView(Event e) const
{
	auto stamp = getEventLabel(e)->getStamp();
	return llvm::make_unique<View>(getViewFromStamp(stamp));
}

std::vector<std::unique_ptr<EventLabel> >
ExecutionGraph::getPrefixLabelsNotBefore(const EventLabel *sLab,
					 const ReadLabel *rLab) const
{
	std::vector<std::unique_ptr<EventLabel> > result;

	auto &prefix = sLab->getPorfView();
	auto before = getViewFromStamp(rLab->getStamp());
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = before[i] + 1; j <= prefix[i]; j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			result.push_back(std::unique_ptr<EventLabel>(lab->clone()));

			auto &curLab = result.back();
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(curLab.get())) {
				wLab->removeReader([&](Event r) {
						return !prefix.contains(r) &&
						       !before.contains(r);
					});
			} else if (auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(curLab.get())) {
				if (!prefix.contains(eLab->getParentJoin()) &&
				    !before.contains(eLab->getParentJoin()))
					eLab->setParentJoin(Event::getInitializer());
			} else if (auto *cLab = llvm::dyn_cast<ThreadCreateLabel>(curLab.get())) {
				/* We can keep the begin event of the child
				 * the since it will not be deleted */
				;
			}
		}
	}
	return result;
}

std::vector<Event>
ExecutionGraph::extractRfs(const std::vector<std::unique_ptr<EventLabel> > &labs) const
{
	std::vector<Event> rfs;

	for (auto const &lab : labs) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab.get()))
			rfs.push_back(rLab->getRf());
	}
	return rfs;
}


/************************************************************
 ** Calculation of writes a read can read from
 ***********************************************************/

bool ExecutionGraph::isNonTrivial(const Event e) const
{
	return isNonTrivial(getEventLabel(e));
}

bool ExecutionGraph::isNonTrivial(const EventLabel *lab) const
{
	if (auto *lLab = llvm::dyn_cast<LockLabelLAPOR>(lab))
		return isCSEmptyLAPOR(lLab);
	return llvm::isa<MemAccessLabel>(lab) ||
	       llvm::isa<FenceLabel>(lab);
}

bool ExecutionGraph::isCSEmptyLAPOR(const LockLabelLAPOR *lLab) const
{
	if (lLab->getIndex() == getThreadSize(lLab->getThread()) - 1)
		return true;

	auto *nLab = getEventLabel(lLab->getPos().next());
	if (auto *uLab = llvm::dyn_cast<UnlockLabelLAPOR>(nLab))
		return lLab->getLockAddr() == uLab->getLockAddr();
	return false;
}

bool ExecutionGraph::isHbOptRfBefore(const Event e, const Event write) const
{
	const EventLabel *lab = getEventLabel(write);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *sLab = static_cast<const WriteLabel *>(lab);
	if (sLab->getHbView().contains(e))
		return true;

	for (auto &r : sLab->getReadersList()) {
		if (getHbBefore(r).contains(e))
			return true;
	}
	return false;
}

bool ExecutionGraph::isHbOptRfBeforeInView(const Event e, const Event write,
					   const VectorClock &v) const
{
	const EventLabel *lab = getEventLabel(write);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *sLab = static_cast<const WriteLabel *>(lab);
	if (sLab->getHbView().contains(e))
		return true;

	for (auto &r : sLab->getReadersList()) {
		if (v.contains(r) && r != e && getHbBefore(r).contains(e))
			return true;
	}
	return false;
}

bool ExecutionGraph::isWriteRfBefore(Event a, Event b) const
{
	auto &before = getEventLabel(b)->getHbView();
	if (before.contains(a))
		return true;

	const EventLabel *lab = getEventLabel(a);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (auto &r : wLab->getReadersList())
		if (before.contains(r))
			return true;
	return false;
}

bool ExecutionGraph::isStoreReadByExclusiveRead(Event store,
						const llvm::GenericValue *ptr) const
{
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!isRMWLoad(lab))
				continue;

			auto *rLab = static_cast<const ReadLabel *>(lab);
			if (rLab->getRf() == store && rLab->getAddr() == ptr)
				return true;
		}
	}
	return false;
}

bool ExecutionGraph::isStoreReadBySettledRMW(Event store, const llvm::GenericValue *ptr,
					     const VectorClock &prefix) const
{
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!isRMWLoad(lab))
				continue;

			auto *rLab = static_cast<const ReadLabel *>(lab);
			if (rLab->getRf() != store || rLab->getAddr() != ptr)
				continue;

			if (!rLab->isRevisitable())
				return true;
			if (prefix.contains(rLab->getPos()))
				return true;
		}
	}
	return false;
}

bool ExecutionGraph::revisitModifiesGraph(const ReadLabel *rLab,
					  const EventLabel *sLab) const
{
	auto v = getViewFromStamp(rLab->getStamp());
	auto &pfx = getPorfBefore(sLab->getPos());

	v.update(pfx);
	for (auto i = 0u; i < getNumThreads(); i++) {
		if (v[i] + 1 != (int) getThreadSize(i))
			return true;
	}
	return false;
}


/************************************************************
 ** Graph modification methods
 ***********************************************************/

void ExecutionGraph::changeRf(Event read, Event store)
{
	EventLabel *lab = events[read.thread][read.index].get();
	BUG_ON(!llvm::isa<ReadLabel>(lab));

	/* First, we set the new reads-from edge */
	ReadLabel *rLab = static_cast<ReadLabel *>(lab);
	Event oldRf = rLab->getRf();
	rLab->setRf(store);

	/*
	 * Now, we need to delete the read from the readers list of oldRf.
	 * For that we need to check:
	 *     1) That the old write it was reading from still exists
	 *     2) That oldRf has not been deleted (and a different event is
	 *        now in its place, perhaps after the restoration of some prefix
	 *        during a revisit)
	 *     3) That oldRf is not the initializer */
	if (oldRf.index > 0 && oldRf.index < (int) getThreadSize(oldRf.thread)) {
		EventLabel *labRef = events[oldRf.thread][oldRf.index].get();
		if (auto *oldLab = llvm::dyn_cast<WriteLabel>(labRef))
			oldLab->removeReader([&](Event r){ return r == rLab->getPos(); });
	}

	/* If this read is now reading from bottom, nothing else to do */
	if (store.isBottom())
		return;

	/* Otherwise, add it to the write's reader list */
	EventLabel *rfLab = events[store.thread][store.index].get();
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
		wLab->addReader(rLab->getPos());
}

bool ExecutionGraph::updateJoin(Event join, Event childLast)
{
	EventLabel *lab = events[join.thread][join.index].get();
	BUG_ON(!llvm::isa<ThreadJoinLabel>(lab));

	auto *jLab = static_cast<ThreadJoinLabel *>(lab);
	EventLabel *cLastLab = events[childLast.thread][childLast.index].get();

	/* If the child thread has not terminated, do not update anything */
	if (!llvm::isa<ThreadFinishLabel>(cLastLab))
		return false;

	auto *fLab = static_cast<ThreadFinishLabel *>(cLastLab);

	jLab->setChildLast(childLast);
	fLab->setParentJoin(jLab->getPos());
	return true;
}

void ExecutionGraph::resetJoin(Event join)
{
	/* If there is no parent join label, return */
	EventLabel *lab = events[join.thread][join.index].get();
	if (!llvm::isa<ThreadJoinLabel>(lab))
		return;

	/* Otherwise, reset parent join */
	auto *jLab = static_cast<ThreadJoinLabel *>(lab);
	jLab->setChildLast(Event::getInitializer());
	return;
}

/*
 * In the case where the events are not added out-of-order in the graph
 * (i.e., an event has a larger timestamp than all its po-predecessors)
 * we can obtain a view of the graph, given a timestamp. This function
 * returns such a view.
 */
View ExecutionGraph::getViewFromStamp(unsigned int stamp) const
{
	View preds;

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = (int) getThreadSize(i) - 1; j >= 0; j--) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (lab->getStamp() <= stamp) {
				preds[i] = j;
				break;
			}
		}
	}
	return preds;
}

DepView ExecutionGraph::getDepViewFromStamp(unsigned int stamp) const
{
	DepView preds;

	for (auto i = 0u; i < getNumThreads(); i++) {
		int prevPos = 0; /* Position of last concrent event in view */
		for (auto j = 1u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (lab->getStamp() <= stamp) {
				preds[i] = j;
				preds.addHolesInRange(Event(i, prevPos + 1), j);
				prevPos = j;
			}
		}
	}
	return preds;
}

void ExecutionGraph::changeStoreOffset(const llvm::GenericValue *addr,
				     Event s, int newOffset)
{
	BUG_ON(!llvm::isa<MOCoherenceCalculator>(getCoherenceCalculator()));
	auto *cohTracker = static_cast<MOCoherenceCalculator *>(
		getCoherenceCalculator());

	cohTracker->changeStoreOffset(addr, s, newOffset);
}

std::vector<std::pair<Event, Event> >
ExecutionGraph::saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
				    const ReadLabel *rLab) const
{
	return getCoherenceCalculator()->saveCoherenceStatus(prefix, rLab);
}

void ExecutionGraph::cutToStamp(unsigned int stamp)
{
	auto preds = getViewFromStamp(stamp);

	/* Inform all calculators about the events cutted */
	auto &calcs = consistencyCalculators;
	for (auto i = 0u; i < calcs.size(); i++)
		calcs[i]->removeAfter(preds);

	/* Restrict the graph according to the view (keep begins around) */
	for (auto i = 0u; i < getNumThreads(); i++) {
		auto &thr = events[i];
		thr.erase(thr.begin() + preds[i] + 1, thr.end());
	}

	/* Remove any 'pointers' to events that have been removed */
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			EventLabel *lab = events[i][j].get();
			/*
			 * If it is a join and the respective Finish has been
			 * removed, renew the Views of this label and continue
			 */
			if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab)) {
				Event cl = jLab->getChildLast();
				if (cl.index >= (int) getThreadSize(cl.thread))
					resetJoin(jLab->getPos());
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				wLab->removeReader([&](Event r){
						return !preds.contains(r);
					});
			}
			/* No special action for CreateLabels; we can
			 * keep the begin event of the child the since
			 * it will not be deleted */
		}
	}
	return;
}

void ExecutionGraph::restoreStorePrefix(const ReadLabel *rLab,
					std::vector<std::unique_ptr<EventLabel> > &storePrefix,
					std::vector<std::pair<Event, Event> > &moPlacings)
{
	auto &calcs = consistencyCalculators;
	for (auto i = 0u; i < calcs.size() ; i++)
		calcs[i]->restorePrefix(rLab, storePrefix, moPlacings);

	std::vector<Event> inserted;

	for (auto &lab : storePrefix) {
		inserted.push_back(lab->getPos());
		if (events[lab->getThread()].size() <= lab->getIndex()) {
			events[lab->getThread()].resize(lab->getIndex());
			events[lab->getThread()].push_back(std::move(lab));
		} else { /* size() > index */
			events[lab->getThread()][lab->getIndex()] = std::move(lab);
		}
	}

	for (const auto &e : inserted) {
		EventLabel *curLab = events[e.thread][e.index].get();
		if (auto *curRLab = llvm::dyn_cast<ReadLabel>(curLab)) {
			curRLab->setRevisitStatus(false);
			Event curRf = curRLab->getRf();
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(
				    events[curRf.thread][curRf.index].get())) {
				wLab->addReader(curRLab->getPos());
			}
		} else if (auto *curWLab = llvm::dyn_cast<WriteLabel>(curLab)) {
			curWLab->removeReader([&](Event r){ return r == rLab->getPos(); });
		}
	}

	/* Do not keep any nullptrs in the graph */
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			if (events[i][j])
				continue;
			events[i][j] = std::unique_ptr<EmptyLabel>(
				new EmptyLabel(nextStamp(), Event(i, j)));
		}
	}
}

bool ExecutionGraph::revisitSetContains(const ReadLabel *r, const std::vector<Event> &writePrefix,
					const std::vector<std::pair<Event, Event> > &moPlacings) const
{
	EventLabel *lab = events[r->getThread()][r->getIndex()].get();
	BUG_ON(!llvm::isa<ReadLabel>(lab));

	ReadLabel *rLab = static_cast<ReadLabel *>(lab);
	return rLab->revs.contains(writePrefix, moPlacings);
}

void ExecutionGraph::addToRevisitSet(const ReadLabel *r, const std::vector<Event> &writePrefix,
				     const std::vector<std::pair<Event, Event> > &moPlacings)
{
	EventLabel *lab = events[r->getThread()][r->getIndex()].get();
	BUG_ON(!llvm::isa<ReadLabel>(lab));

	ReadLabel *rLab = static_cast<ReadLabel *>(lab);
	return rLab->revs.add(writePrefix, moPlacings);
}


/************************************************************
 ** PSC calculation
 ***********************************************************/

bool ExecutionGraph::isRMWLoad(const EventLabel *lab) const
{
	if (!llvm::isa<CasReadLabel>(lab) && !llvm::isa<FaiReadLabel>(lab))
		return false;
	const ReadLabel *rLab = static_cast<const ReadLabel *>(lab);

	if (lab->getIndex() == (int) getThreadSize(lab->getThread()) - 1)
		return false;
	const EventLabel *nLab =
		getEventLabel(Event(rLab->getThread(), rLab->getIndex() + 1));

	if (!llvm::isa<MemAccessLabel>(nLab))
		return false;
	auto *mLab = static_cast<const MemAccessLabel *>(nLab);

	if ((llvm::isa<CasWriteLabel>(mLab) || llvm::isa<FaiWriteLabel>(mLab)) &&
	    mLab->getAddr() == rLab->getAddr())
		return true;
	return false;
}

bool ExecutionGraph::isRMWLoad(Event e) const
{
	return isRMWLoad(getEventLabel(e));
}

std::pair<std::vector<Event>, std::vector<Event> >
ExecutionGraph::getSCs() const
{
	std::vector<Event> scs, fcs;

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (lab->isSC() && !isRMWLoad(lab))
				scs.push_back(lab->getPos());
			if (lab->isSC() && llvm::isa<FenceLabel>(lab))
				fcs.push_back(lab->getPos());
		}
	}
	return std::make_pair(scs,fcs);
}


/************************************************************
 ** Library consistency checking methods
 ***********************************************************/

std::vector<Event> ExecutionGraph::getLibEventsInView(const Library &lib, const View &v)
{
	std::vector<Event> result;

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 1; j <= v[i]; j++) { /* Do not consider thread inits */
			const EventLabel *lab = getEventLabel(Event(i, j));

			if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab)) {
				if (lib.hasMember(rLab->getFunctionName()))
					result.push_back(rLab->getPos());
			}
			if (auto *wLab = llvm::dyn_cast<LibWriteLabel>(lab)) {
				if (lib.hasMember(wLab->getFunctionName()))
					result.push_back(wLab->getPos());
			}
		}
	}
	return result;
}

void ExecutionGraph::getPoEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			for (auto &e : tos)
				if (froms[i].second[j].thread == e.thread &&
				    froms[i].second[j].index < e.index)
					buf.push_back(e);
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getRfm1EdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				      std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (std::find(tos.begin(), tos.end(), rLab->getRf()) !=
				    tos.end())
					buf.push_back(rLab->getRf());
			}
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getRfEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				const std::vector<Event> readers = wLab->getReadersList();
				std::copy_if(readers.begin(), readers.end(),
					     std::back_inserter(buf),
					     [&](const Event &e) {
						     return std::find(tos.begin(), tos.end(),
								      e) != tos.end();
					     });
			}
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getHbEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			for (auto &e : tos) {
				auto &before = getHbBefore(e);
				if (froms[i].second[j].index <
				    before[froms[i].second[j].thread])
					buf.push_back(e);
			}
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getWbEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	auto *cc = getCoherenceCalculator();
	BUG_ON(!llvm::isa<WBCoherenceCalculator>(cc));
	auto *cohTracker = static_cast<WBCoherenceCalculator *>(cc);
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (!llvm::isa<WriteLabel>(lab))
				continue;

			auto *wLab = static_cast<const WriteLabel *>(lab);
			View v(getHbBefore(tos[0]));
			for (auto &t : tos)
				v.update(getEventLabel(t)->getHbView());

			auto wb = cohTracker->calcWbRestricted(wLab->getAddr(), v);
			auto &ss = wb.getElems();
			// TODO: Make a map with already calculated WBs??

			if (std::find(ss.begin(), ss.end(), wLab->getPos()) == ss.end())
				continue;

			/* Collect all wb-after stores that are in "tos" range */
			auto k = wb.getIndex(wLab->getPos());
			for (auto l = 0u; l < ss.size(); l++)
				if (wb(k, l))
					buf.push_back(ss[l]);
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getMoEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	BUG_ON(!llvm::isa<MOCoherenceCalculator>(getCoherenceCalculator()));
	auto *cohTracker = static_cast<MOCoherenceCalculator *>(getCoherenceCalculator());

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (!llvm::isa<WriteLabel>(lab))
				continue;

			/* Collect all mo-after events that are in the "tos" range */
			auto *wLab = static_cast<const WriteLabel *>(lab);
			auto moAfter = cohTracker->getMOAfter(wLab->getAddr(), wLab->getPos());
			for (const auto &s : moAfter)
				if (std::find(tos.begin(), tos.end(), s) != tos.end())
					buf.push_back(s);
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::calcSingleStepPairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
					 const std::string &step, std::vector<Event> &tos)
{
	if (step == "po")
		return getPoEdgePairs(froms, tos);
	else if (step == "rf")
		return getRfEdgePairs(froms, tos);
	else if (step == "hb")
		return getHbEdgePairs(froms, tos);
	else if (step == "rf-1")
		return getRfm1EdgePairs(froms, tos);
	else if (step == "wb")
		return getWbEdgePairs(froms, tos);
	else if (step == "mo")
		return getMoEdgePairs(froms, tos);
	else
		BUG();
}

void addEdgePairsToMatrix(std::vector<std::pair<Event, std::vector<Event> > > &pairs,
			  AdjList<Event, EventHasher> &matrix)
{
	for (auto &p : pairs) {
		for (auto k = 0u; k < p.second.size(); k++) {
			matrix.addEdge(p.first, p.second[k]);
		}
	}
}

void ExecutionGraph::addStepEdgeToMatrix(std::vector<Event> &es,
					 AdjList<Event, EventHasher> &relMatrix,
					 const std::vector<std::string> &substeps)
{
	std::vector<std::pair<Event, std::vector<Event> > > edges;

	/* Initialize edges */
	for (auto &e : es)
		edges.push_back(std::make_pair(e, std::vector<Event>({e})));

	for (auto i = 0u; i < substeps.size(); i++)
		calcSingleStepPairs(edges, substeps[i], es);
	addEdgePairsToMatrix(edges, relMatrix);
}

llvm::StringMap<AdjList<Event, EventHasher> >
ExecutionGraph::calculateAllRelations(const Library &lib, std::vector<Event> &es)
{
	llvm::StringMap<AdjList<Event, EventHasher> > relMap;

	for (auto &r : lib.getRelations()) {
		AdjList<Event, EventHasher> relMatrix(es);
		auto &steps = r.getSteps();
		for (auto &s : steps)
			addStepEdgeToMatrix(es, relMatrix, s);

		if (r.isTransitive())
			relMatrix.transClosure();
		relMap[r.getName()] = relMatrix;
	}
	return relMap;
}

bool ExecutionGraph::isLibConsistentInView(const Library &lib, const View &v)
{
	auto es = getLibEventsInView(lib, v);
	auto relations = calculateAllRelations(lib, es);
	auto &constraints = lib.getConstraints();
	if (std::all_of(constraints.begin(), constraints.end(),
			[&](const Constraint &c)
			{ return relations[c.getName()].isIrreflexive(); }))
		return true;
	return false;
}

void ExecutionGraph::addInvalidRfs(Event read, const std::vector<Event> &rfs)
{
	EventLabel *lab = events[read.thread][read.index].get();
	BUG_ON(!llvm::isa<LibReadLabel>(lab));

	auto *lLab = static_cast<LibReadLabel *>(lab);
	for (auto &s : rfs)
		lLab->addInvalidRf(s);
}

void ExecutionGraph::addBottomToInvalidRfs(Event read)
{
	EventLabel *lab = events[read.thread][read.index].get();

	if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab)) {
		rLab->addInvalidRf(Event::getInitializer());
		return;
	}
	BUG();
}


/************************************************************
 ** Debugging methods
 ***********************************************************/

static bool containsEvent(const std::vector<Event> &es, const Event e)
{
	return std::find(es.begin(), es.end(), e) != es.end();
}

void ExecutionGraph::validate(void)
{
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getRf().isInitializer())
					continue;

				auto *rfLab = static_cast<const WriteLabel *>(
					getEventLabel(rLab->getRf()));
				if (containsEvent(rfLab->getReadersList(), rLab->getPos()))
					continue;
				WARN("Read event is not the appropriate rf-1 list!\n");
				llvm::dbgs() << rLab->getPos() << "\n";
				llvm::dbgs() << *this << "\n";
				abort();
			}
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				const std::vector<Event> &rs = wLab->getReadersList();
				if (llvm::isa<FaiWriteLabel>(wLab) ||
				    llvm::isa<CasWriteLabel>(wLab)) {
					if (1 >= std::count_if(rs.begin(), rs.end(),
							       [&](const Event &r) {
								       return isRMWLoad(r);
							       }))
						continue;
					WARN("RMW store is read from more than 1 load!\n");
					llvm::dbgs() << "RMW store: " << wLab->getPos()
						     << "\nReads:";
					for (auto &r : rs)
						llvm::dbgs() << r << " ";
					llvm::dbgs() << "\n";
					llvm::dbgs() << *this << "\n";
					abort();
				}

				if (std::any_of(rs.begin(), rs.end(), [&](Event r) {
				        if (auto *rLab = llvm::dyn_cast<ReadLabel>(
						    getEventLabel(r))) {
						return rLab->getRf() != wLab->getPos();
					} else {
						WARN("Non-read event found in rf-1 list!\n");
						abort();
					}
					})) {
						WARN("Write event is not marked in the read event!\n");
						llvm::dbgs() << wLab->getPos() << "\n";
						llvm::dbgs() << *this << "\n";
						abort();
				}
				for (auto &r : rs) {
					if (r.thread > (int) getNumThreads() ||
					    r.index >= (int) getThreadSize(r.thread)) {
						WARN("Event in write's rf-list does not exist!\n");
						llvm::dbgs() << r << "\n";
						llvm::dbgs() << *this << "\n";
						abort();
					}
				}

			}
		}
	}
	return;
}


/*******************************************************************************
 **                           Overloaded operators
 ******************************************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ExecutionGraph &g)
{
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		s << "Thread " << i << ":\n";
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			s << "\t" << *lab << "\n";
		}
	}
	s << "Thread sizes:\n\t";
	for (auto i = 0u; i < g.getNumThreads(); i++)
		s << g.events[i].size() << " ";
	s << "\n";
	return s;
}
