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

#include "Library.hpp"
#include "MOCoherenceCalculator.hpp"
#include "WBCoherenceCalculator.hpp"
#include "Parser.hpp"
#include "ExecutionGraph.hpp"
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
		if (llvm::isa<ThreadCreateLabel>(lab) || llvm::isa<ThreadFinishLabel>(lab)) {
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
		if (llvm::isa<ThreadCreateLabel>(lab) || llvm::isa<ThreadFinishLabel>(lab)) {
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
		if (llvm::isa<ThreadJoinLabel>(lab))
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

std::vector<Event> ExecutionGraph::getStoresHbAfterStores(const llvm::GenericValue *loc,
							  const std::vector<Event> &chain) const
{
	auto &stores = getStoresToLoc(loc);
	std::vector<Event> result;

	for (auto &s : stores) {
		if (std::find(chain.begin(), chain.end(), s) != chain.end())
			continue;
		auto &before = getHbBefore(s);
		if (std::any_of(chain.begin(), chain.end(), [&before](Event e)
				{ return e.index < before[e.thread]; }))
			result.push_back(s);
	}
	return result;
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

void ExecutionGraph::calcHbRfBefore(Event e, const llvm::GenericValue *addr,
				    View &a) const
{
	if (a.contains(e))
		return;
	int ai = a[e.thread];
	a[e.thread] = e.index;
	for (int i = ai + 1; i <= e.index; i++) {
		const EventLabel *lab = getEventLabel(Event(e.thread, i));
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (rLab->getAddr() == addr ||
			    rLab->getHbView().contains(rLab->getRf()))
				calcHbRfBefore(rLab->getRf(), addr, a);
		} else if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab)) {
			calcHbRfBefore(bLab->getParentCreate(), addr, a);
		} else if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab)) {
			calcHbRfBefore(jLab->getChildLast(), addr, a);
		}
	}
	return;
}

View ExecutionGraph::getHbRfBefore(const std::vector<Event> &es) const
{
	View a;

	for (auto &e : es) {
		const EventLabel *lab = getEventLabel(e);
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			calcHbRfBefore(e, rLab->getAddr(), a);
		} else {
			a.update(lab->getHbView());
		}
	}
	return a;
}

void ExecutionGraph::calcRelRfPoBefore(Event last, View &v) const
{
	for (auto i = last.index; i > 0; i--) {
		const EventLabel *lab = getEventLabel(Event(last.thread, i));
		if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire())
			return;
		if (!llvm::isa<ReadLabel>(lab))
			continue;
		auto *rLab = static_cast<const ReadLabel *>(lab);
		if (rLab->getOrdering() == llvm::AtomicOrdering::Monotonic ||
		    rLab->getOrdering() == llvm::AtomicOrdering::Release) {
			const EventLabel *rfLab = getEventLabel(rLab->getRf());
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
				v.update(wLab->getMsgView());
		}
	}
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
					     const VectorClock &porfBefore) const
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
			if (porfBefore.contains(rLab->getPos()))
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

void ExecutionGraph::changeStoreOffset(const llvm::GenericValue *addr,
				       Event s, int newOffset)
{
	BUG_ON(!llvm::isa<MOCoherenceCalculator>(getCoherenceCalculator()));
	auto *cohTracker = static_cast<MOCoherenceCalculator *>(
		getCoherenceCalculator());

	cohTracker->changeStoreOffset(addr, s, newOffset);
}

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

void ExecutionGraph::cutToStamp(unsigned int stamp)
{
	auto preds = getViewFromStamp(stamp);

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

	/* Remove cutted events from the coherence order as well */
	getCoherenceCalculator()->removeStoresAfter(preds);
	return;
}

std::vector<std::pair<Event, Event> >
ExecutionGraph::saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
				    const ReadLabel *rLab) const
{
	return getCoherenceCalculator()->saveCoherenceStatus(prefix, rLab);
}

void ExecutionGraph::restoreStorePrefix(const ReadLabel *rLab,
					std::vector<std::unique_ptr<EventLabel> > &storePrefix,
					std::vector<std::pair<Event, Event> > &moPlacings)
{
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

	/* Insert the writes of storePrefix into the appropriate places */
	getCoherenceCalculator()->restoreCoherenceStatus(moPlacings);
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
 ** Consistency checks
 ***********************************************************/

bool ExecutionGraph::isConsistent(void) const
{
	return true;
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

std::vector<const llvm::GenericValue *> ExecutionGraph::getDoubleLocs() const
{
	std::vector<const llvm::GenericValue *> singles, doubles;

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 1u; j < getThreadSize(i); j++) { /* Do not consider thread inits */
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!llvm::isa<MemAccessLabel>(lab))
				continue;

			auto *mLab = static_cast<const MemAccessLabel *>(lab);
			if (std::find(doubles.begin(), doubles.end(),
				      mLab->getAddr()) != doubles.end())
				continue;
			if (std::find(singles.begin(), singles.end(),
				      mLab->getAddr()) != singles.end()) {
				singles.erase(std::remove(singles.begin(),
							  singles.end(),
							  mLab->getAddr()),
					      singles.end());
				doubles.push_back(mLab->getAddr());
			} else {
				singles.push_back(mLab->getAddr());
			}
		}
	}
	return doubles;
}

std::vector<Event> ExecutionGraph::calcSCFencesSuccs(const std::vector<Event> &fcs,
						     const Event e) const
{
	std::vector<Event> succs;

	if (isRMWLoad(e))
		return succs;
	for (auto &f : fcs) {
		if (getHbBefore(f).contains(e))
			succs.push_back(f);
	}
	return succs;
}

std::vector<Event> ExecutionGraph::calcSCFencesPreds(const std::vector<Event> &fcs,
						     const Event e) const
{
	std::vector<Event> preds;
	auto &before = getHbBefore(e);

	if (isRMWLoad(e))
		return preds;
	for (auto &f : fcs) {
		if (before.contains(f))
			preds.push_back(f);
	}
	return preds;
}

std::vector<Event> ExecutionGraph::calcSCSuccs(const std::vector<Event> &fcs,
					       const Event e) const
{
	const EventLabel *lab = getEventLabel(e);

	if (isRMWLoad(lab))
		return {};
	if (lab->isSC())
		return {e};
	else
		return calcSCFencesSuccs(fcs, e);
}

std::vector<Event> ExecutionGraph::calcSCPreds(const std::vector<Event> &fcs,
					       const Event e) const
{
	const EventLabel *lab = getEventLabel(e);

	if (isRMWLoad(lab))
		return {};
	if (lab->isSC())
		return {e};
	else
		return calcSCFencesPreds(fcs, e);
}

std::vector<Event> ExecutionGraph::calcRfSCSuccs(const std::vector<Event> &fcs,
						 const Event ev) const
{
	const EventLabel *lab = getEventLabel(ev);
	std::vector<Event> rfs;

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto succs = calcSCSuccs(fcs, e);
		rfs.insert(rfs.end(), succs.begin(), succs.end());
	}
	return rfs;
}

std::vector<Event> ExecutionGraph::calcRfSCFencesSuccs(const std::vector<Event> &fcs,
						       const Event ev) const
{
	const EventLabel *lab = getEventLabel(ev);
	std::vector<Event> fenceRfs;

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto fenceSuccs = calcSCFencesSuccs(fcs, e);
		fenceRfs.insert(fenceRfs.end(), fenceSuccs.begin(), fenceSuccs.end());
	}
	return fenceRfs;
}

void ExecutionGraph::addRbEdges(const std::vector<Event> &fcs,
				const std::vector<Event> &moAfter,
				const std::vector<Event> &moRfAfter,
				Matrix2D<Event> &matrix,
				const Event &ev) const
{
	const EventLabel *lab = getEventLabel(ev);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto preds = calcSCPreds(fcs, e);
		auto fencePreds = calcSCFencesPreds(fcs, e);

		matrix.addEdgesFromTo(preds, moAfter);        /* Base/fence: Adds rb-edges */
		matrix.addEdgesFromTo(fencePreds, moRfAfter); /* Fence: Adds (rb;rf)-edges */
	}
	return;
}

void ExecutionGraph::addMoRfEdges(const std::vector<Event> &fcs,
				  const std::vector<Event> &moAfter,
				  const std::vector<Event> &moRfAfter,
				  Matrix2D<Event> &matrix,
				  const Event &ev) const
{
	auto preds = calcSCPreds(fcs, ev);
	auto fencePreds = calcSCFencesPreds(fcs, ev);
	auto rfs = calcRfSCSuccs(fcs, ev);

	matrix.addEdgesFromTo(preds, moAfter);        /* Base/fence:  Adds mo-edges */
	matrix.addEdgesFromTo(preds, rfs);            /* Base/fence:  Adds rf-edges (hb_loc) */
	matrix.addEdgesFromTo(fencePreds, moRfAfter); /* Fence:       Adds (mo;rf)-edges */
	return;
}

/*
 * addSCEcos - Helper function that calculates a part of PSC_base and PSC_fence
 *
 * For PSC_base and PSC_fence, it adds mo, rb, and hb_loc edges. The
 * procedure for mo and rb is straightforward: at each point, we only
 * need to keep a list of all the mo-after writes that are either SC,
 * or can reach an SC fence. For hb_loc, however, we only consider
 * rf-edges because the other cases are implicitly covered (sb, mo, etc).
 *
 * For PSC_fence only, it adds (mo;rf)- and (rb;rf)-edges. Simple cases like
 * mo, rf, and rb are covered by PSC_base, and all other combinations with
 * more than one step either do not compose, or lead to an already added
 * single-step relation (e.g, (rf;rb) => mo, (rb;mo) => rb)
 */
void ExecutionGraph::addSCEcos(const std::vector<Event> &fcs,
			       const std::vector<Event> &mo,
			       Matrix2D<Event> &matrix) const
{
	std::vector<Event> moAfter;   /* mo-after SC writes or SC fences reached by an mo-after write */
	std::vector<Event> moRfAfter; /* SC fences that can be reached by (mo;rf)-after reads */

	for (auto rit = mo.rbegin(); rit != mo.rend(); rit++) {

		/* First, add edges to SC events that are (mo U rb);rf?-after this write */
		addRbEdges(fcs, moAfter, moRfAfter, matrix, *rit);
		addMoRfEdges(fcs, moAfter, moRfAfter, matrix, *rit);

		/* Then, update the lists of mo and mo;rf SC successors */
		auto succs = calcSCSuccs(fcs, *rit);
		auto fenceRfs = calcRfSCFencesSuccs(fcs, *rit);
		moAfter.insert(moAfter.end(), succs.begin(), succs.end());
		moRfAfter.insert(moRfAfter.end(), fenceRfs.begin(), fenceRfs.end());
	}
}

/*
 * Similar to addSCEcos but uses a partial order among stores (WB) to
 * add coherence (mo/wb) and rb edges.
 */
void ExecutionGraph::addSCEcos(const std::vector<Event> &fcs,
			       Matrix2D<Event> &wbMatrix,
			       Matrix2D<Event> &pscMatrix) const
{
	auto &stores = wbMatrix.getElems();
	for (auto i = 0u; i < stores.size(); i++) {

		/*
		 * Calculate which of the stores are wb-after the current
		 * write, and then collect wb-after and (wb;rf)-after SC successors
		 */
		std::vector<Event> wbAfter, wbRfAfter;
		for (auto j = 0u; j < stores.size(); j++) {
			if (wbMatrix(i, j)) {
				auto succs = calcSCSuccs(fcs, stores[j]);
				auto fenceRfs = calcRfSCFencesSuccs(fcs, stores[j]);
				wbAfter.insert(wbAfter.end(), succs.begin(), succs.end());
				wbRfAfter.insert(wbRfAfter.end(), fenceRfs.begin(), fenceRfs.end());
			}
		}

		/* Then, add the proper edges to PSC using wb-after and (wb;rf)-after successors */
		addRbEdges(fcs, wbAfter, wbRfAfter, pscMatrix, stores[i]);
		addMoRfEdges(fcs, wbAfter, wbRfAfter, pscMatrix, stores[i]);
	}
}

/*
 * Adds sb as well as [Esc];sb_(<>loc);hb;sb_(<>loc);[Esc] edges. The first
 * part of this function is common for PSC_base and PSC_fence, while the second
 * part of this function is not triggered for fences (these edges are covered in
 * addSCEcos()).
 */
void ExecutionGraph::addSbHbEdges(Matrix2D<Event> &matrix) const
{
	auto &scs = matrix.getElems();
	for (auto i = 0u; i < scs.size(); i++) {
		for (auto j = 0u; j < scs.size(); j++) {
			if (i == j)
				continue;
			const EventLabel *eiLab = getEventLabel(scs[i]);
			const EventLabel *ejLab = getEventLabel(scs[j]);

			/* PSC_base/PSC_fence: Adds sb-edges*/
			if (eiLab->getThread() == ejLab->getThread()) {
				if (eiLab->getIndex() < ejLab->getIndex())
					matrix(i, j) = true;
				continue;
			}

			/* PSC_base: Adds [Esc];sb_(<>loc);hb;sb_(<>loc);[Esc] edges.
			 * We do need to consider the [Fsc];hb? cases, since these
			 * will be covered by addSCEcos(). (More speficically, from
			 * the rf/hb_loc case in addMoRfEdges().)  */
			const EventLabel *ejPrevLab = getPreviousNonEmptyLabel(ejLab);
			if (!llvm::isa<MemAccessLabel>(ejPrevLab) ||
			    !llvm::isa<MemAccessLabel>(ejLab) ||
			    !llvm::isa<MemAccessLabel>(eiLab))
				continue;

			if (eiLab->getPos() == getLastThreadEvent(eiLab->getThread()))
				continue;

			auto *ejPrevMLab = static_cast<const MemAccessLabel *>(ejPrevLab);
			auto *ejMLab = static_cast<const MemAccessLabel *>(ejLab);
			auto *eiMLab = static_cast<const MemAccessLabel *>(eiLab);

			if (ejPrevMLab->getAddr() != ejMLab->getAddr()) {
				Event next = eiMLab->getPos().next();
				const EventLabel *eiNextLab = getEventLabel(next);
				if (auto *eiNextMLab =
				    llvm::dyn_cast<MemAccessLabel>(eiNextLab)) {
					if (eiMLab->getAddr() != eiNextMLab->getAddr() &&
					    ejPrevMLab->getHbView().contains(eiNextMLab->getPos()))
						matrix(i, j) = true;
				}
			}
		}
	}
	return;
}

void ExecutionGraph::addInitEdges(const std::vector<Event> &fcs,
				  Matrix2D<Event> &matrix) const
{
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			/* Consider only reads that read from the initializer write */
			if (!llvm::isa<ReadLabel>(lab) || isRMWLoad(lab))
				continue;
			auto *rLab = static_cast<const ReadLabel *>(lab);
			if (!rLab->getRf().isInitializer())
				continue;

			auto preds = calcSCPreds(fcs, rLab->getPos());
			auto fencePreds = calcSCFencesPreds(fcs, rLab->getPos());
			for (auto &w : getStoresToLoc(rLab->getAddr())) {
				/* Can be casted to WriteLabel by construction */
				auto *wLab = static_cast<const WriteLabel *>(
					getEventLabel(w));
				auto wSuccs = calcSCSuccs(fcs, w);
				matrix.addEdgesFromTo(preds, wSuccs); /* Adds rb-edges */
				for (auto &r : wLab->getReadersList()) {
					auto fenceSuccs = calcSCFencesSuccs(fcs, r);
					matrix.addEdgesFromTo(fencePreds, fenceSuccs); /*Adds (rb;rf)-edges */
				}
			}
		}
	}
	return;
}

template <typename F>
bool ExecutionGraph::addSCEcosMO(const std::vector<Event> &fcs,
				 const std::vector<const llvm::GenericValue *> &scLocs,
				 Matrix2D<Event> &matrix, F cond) const
{
	BUG_ON(!llvm::isa<MOCoherenceCalculator>(getCoherenceCalculator()));
	for (auto loc : scLocs) {
		auto &stores = getStoresToLoc(loc); /* Will already be ordered... */
		addSCEcos(fcs, stores, matrix);
	}
	matrix.transClosure();
	return cond(matrix);
}

template <typename F>
bool ExecutionGraph::addSCEcosWBWeak(const std::vector<Event> &fcs,
				     const std::vector<const llvm::GenericValue *> &scLocs,
				     Matrix2D<Event> &matrix, F cond) const
{
	const auto *cc = getCoherenceCalculator();

	BUG_ON(!llvm::isa<WBCoherenceCalculator>(cc));
	auto *cohTracker = static_cast<const WBCoherenceCalculator *>(cc);
	for (auto loc : scLocs) {
		auto wb = cohTracker->calcWb(loc);
		auto sortedStores = wb.topoSort();
		addSCEcos(fcs, sortedStores, matrix);
	}
	matrix.transClosure();
	return cond(matrix);
}

template <typename F>
bool ExecutionGraph::addSCEcosWB(const std::vector<Event> &fcs,
				 const std::vector<const llvm::GenericValue *> &scLocs,
				 Matrix2D<Event> &matrix, F cond) const
{
	const auto *cc = getCoherenceCalculator();

	BUG_ON(!llvm::isa<WBCoherenceCalculator>(cc));
	auto *cohTracker = static_cast<const WBCoherenceCalculator *>(cc);
	for (auto loc : scLocs) {
		auto wb = cohTracker->calcWb(loc);
		addSCEcos(fcs, wb, matrix);
	}
	matrix.transClosure();
	return cond(matrix);
}

template <typename F>
bool ExecutionGraph::addSCEcosWBFull(const std::vector<Event> &fcs,
				     const std::vector<const llvm::GenericValue *> &scLocs,
				     Matrix2D<Event> &matrix, F cond) const
{
	const auto *cc = getCoherenceCalculator();

	BUG_ON(!llvm::isa<WBCoherenceCalculator>(cc));
	auto *cohTracker = static_cast<const WBCoherenceCalculator *>(cc);

	std::vector<std::vector<std::vector<Event> > > topoSorts(scLocs.size());
	for (auto i = 0u; i < scLocs.size(); i++) {
		auto wb = cohTracker->calcWb(scLocs[i]);
		topoSorts[i] = wb.allTopoSort();
	}

	unsigned int K = topoSorts.size();
	std::vector<unsigned int> count(K, 0);

	/*
	 * It suffices to find one combination for the WB extensions of all
	 * locations, for which PSC is acyclic. This loop is like an odometer:
	 * given an array that contains K vectors, we keep a counter for each
	 * vector, and proceed by incrementing the rightmost counter. Like in
	 * addition, if a carry is created, this is propagated to the left.
	 */
	while (count[0] < topoSorts[0].size()) {
		/* Process current combination */
		auto tentativePSC(matrix);
		for (auto i = 0u; i < K; i++)
			addSCEcos(fcs, topoSorts[i][count[i]], tentativePSC);

		tentativePSC.transClosure();
		if (cond(tentativePSC))
			return true;

		/* Find next combination */
		++count[K - 1];
		for (auto i = K - 1; (i > 0) && (count[i] == topoSorts[i].size()); --i) {
			count[i] = 0;
			++count[i - 1];
		}
	}

	/* No valid MO combination found */
	return false;
}

template <typename F>
bool ExecutionGraph::addEcoEdgesAndCheckCond(CheckPSCType t,
					     const std::vector<Event> &fcs,
					     Matrix2D<Event> &matrix, F cond) const
{
	const auto *cohTracker = getCoherenceCalculator();

	std::vector<const llvm::GenericValue *> scLocs = getDoubleLocs();
	if (auto *moTracker = llvm::dyn_cast<MOCoherenceCalculator>(cohTracker)) {
		switch (t) {
		case CheckPSCType::nocheck:
			return true;
		case CheckPSCType::weak:
		case CheckPSCType::wb:
			WARN_ONCE("check-mo-psc", "The full PSC condition is going "
				  "to be checked for the MO-tracking exploration...\n");
		case CheckPSCType::full:
			return addSCEcosMO(fcs, scLocs, matrix, cond);
		default:
			WARN("Unimplemented model!\n");
			BUG();
		}
	} else if (auto *wbTacker = llvm::dyn_cast<WBCoherenceCalculator>(cohTracker)) {
		switch (t) {
		case CheckPSCType::nocheck:
			return true;
		case CheckPSCType::weak:
			return addSCEcosWBWeak(fcs, scLocs, matrix, cond);
		case CheckPSCType::wb:
			return addSCEcosWB(fcs, scLocs, matrix, cond);
		case CheckPSCType::full:
			return addSCEcosWBFull(fcs, scLocs, matrix, cond);
		default:
			WARN("Unimplemented model!\n");
			BUG();
		}
	}
	BUG();
	return false;
}


template <typename F>
bool ExecutionGraph::checkPscCondition(CheckPSCType t, F cond) const
{
	/* Collect all SC events (except for RMW loads) */
	auto accesses = getSCs();
	auto &scs = accesses.first;
	auto &fcs = accesses.second;

	/* If there are no SC events, it is a valid execution */
	if (scs.empty())
		return true;

	/* Depending on the part of PSC calculated, instantiate the matrix */
	Matrix2D<Event> matrix(scs);

	/* Add edges from the initializer write (special case) */
	addInitEdges(fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(matrix);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence
	 */
	return addEcoEdgesAndCheckCond(t, fcs, matrix, cond);
}

template bool ExecutionGraph::checkPscCondition<bool (*)(const Matrix2D<Event>&)>
(CheckPSCType, bool (*)(const Matrix2D<Event>&)) const;
template bool ExecutionGraph::checkPscCondition<std::function<bool(const Matrix2D<Event>&)>>
(CheckPSCType, std::function<bool(const Matrix2D<Event>&)>) const;

bool __isPscAcyclic(const Matrix2D<Event> &psc)
{
	return !psc.isReflexive();
}

bool ExecutionGraph::isPscAcyclic(CheckPSCType t) const
{
	return checkPscCondition(t, __isPscAcyclic);
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
			  Matrix2D<Event> &matrix)
{
	for (auto &p : pairs) {
		for (auto k = 0u; k < p.second.size(); k++) {
			matrix(p.first, p.second[k]) = true;
		}
	}
}

void ExecutionGraph::addStepEdgeToMatrix(std::vector<Event> &es,
					 Matrix2D<Event> &relMatrix,
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

llvm::StringMap<Matrix2D<Event> >
ExecutionGraph::calculateAllRelations(const Library &lib, std::vector<Event> &es)
{
	llvm::StringMap<Matrix2D<Event> > relMap;

	for (auto &r : lib.getRelations()) {
		Matrix2D<Event> relMatrix(es);
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
			{ return !relations[c.getName()].isReflexive(); }))
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
