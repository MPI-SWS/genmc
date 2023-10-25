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

#include "config.h"
#include "ExecutionGraph.hpp"
#include "GraphIterators.hpp"
#include "Parser.hpp"
#include <llvm/IR/DebugInfo.h>

/************************************************************
 ** Class constructors/destructors
 ***********************************************************/

ExecutionGraph::ExecutionGraph()
{
	/* Create an entry for main() and push the "initializer" label */
	events.push_back({});
	auto *iLab = addLabelToGraph(InitLabel::create());
	iLab->setCalculated({{}});
	iLab->setViews({{}});
	iLab->setPrefixView(std::make_unique<View>());
	return;
}

ExecutionGraph::~ExecutionGraph() = default;

/************************************************************
 ** Basic getter methods
 ***********************************************************/

const EventLabel *ExecutionGraph::getPreviousNonEmptyLabel(Event e) const
{
	for (int i = e.index - 1; i > 0; i--) {
		const EventLabel *eLab = getEventLabel(Event(e.thread, i));
		if (!llvm::isa<EmptyLabel>(eLab))
			return eLab;
	}
	return getEventLabel(Event(e.thread, 0));
}

Event ExecutionGraph::getPreviousNonTrivial(const Event e) const
{
	for (auto i = e.index - 1; i >= 0; i--) {
		if (isNonTrivial(Event(e.thread, i)))
			return Event(e.thread, i);
	}
	return Event::getInit();
}

Event ExecutionGraph::getLastThreadStoreAtLoc(Event upperLimit, SAddr addr) const
{
	for (auto j = upperLimit.index - 1; j > 0; j--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, j));
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
			if (wLab->getAddr() == addr)
				return wLab->getPos();
		}
	}
	return Event::getInit();
}

Event ExecutionGraph::getLastThreadReleaseAtLoc(Event upperLimit, SAddr addr) const
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
		if (llvm::isa<FenceLabel>(lab) || lab->isAtLeastAcquire())
			result.push_back(lab->getPos());
	}
	return result;
}

Event ExecutionGraph::getMatchingLock(const Event unlock) const
{
	std::vector<Event> locUnlocks;

	auto *uLab = llvm::dyn_cast<UnlockWriteLabel>(getEventLabel(unlock));
	BUG_ON(!uLab);

	for (auto j = unlock.index - 1; j > 0; j--) {
		const EventLabel *lab = getEventLabel(Event(unlock.thread, j));

		/* In case support for reentrant locks is added... */
		if (auto *suLab = llvm::dyn_cast<UnlockWriteLabel>(lab)) {
			if (suLab->getAddr() == uLab->getAddr())
				locUnlocks.push_back(suLab->getPos());
		}
		if (auto *lLab = llvm::dyn_cast<CasWriteLabel>(lab)) {
			if ((llvm::isa<LockCasWriteLabel>(lLab) || llvm::isa<TrylockCasWriteLabel>(lLab)) &&
			    lLab->getAddr() == uLab->getAddr()) {
				if (locUnlocks.empty())
					return lLab->getPos().prev();
				locUnlocks.pop_back();

			}
		}
	}
	return Event::getInit();
}

Event ExecutionGraph::getMatchingUnlock(const Event lock) const
{
	std::vector<Event> locLocks;

	auto *lLab = llvm::dyn_cast<CasReadLabel>(getEventLabel(lock));
	BUG_ON(!lLab || (!llvm::isa<LockCasReadLabel>(lLab) && !llvm::isa<TrylockCasReadLabel>(lLab)));

	for (auto j = lock.index + 2; j < getThreadSize(lock.thread); j++) { /* skip next event */
		const EventLabel *lab = getEventLabel(Event(lock.thread, j));

		/* In case support for reentrant locks is added... */
		if (auto *slLab = llvm::dyn_cast<CasWriteLabel>(lab)) {
			if ((llvm::isa<LockCasWriteLabel>(slLab) || llvm::isa<TrylockCasWriteLabel>(slLab)) &&
			    slLab->getAddr() == lLab->getAddr())
				locLocks.push_back(slLab->getPos().prev());
		}
		if (auto *uLab = llvm::dyn_cast<UnlockWriteLabel>(lab)) {
			if (uLab->getAddr() == lLab->getAddr()) {
				if (locLocks.empty())
					return uLab->getPos();
				locLocks.pop_back();
			}
		}
	}
	return Event::getInit();
}

Event ExecutionGraph::getMatchingRCUUnlockLKMM(Event lock) const
{
	std::vector<Event> locks;

	BUG_ON(!llvm::isa<RCULockLabelLKMM>(getEventLabel(lock)));
	for (auto j = lock.index + 1; j < getThreadSize(lock.thread); j++) {
		const EventLabel *lab = getEventLabel(Event(lock.thread, j));

		if (auto *lLab = llvm::dyn_cast<RCULockLabelLKMM>(lab))
			locks.push_back(lLab->getPos());

		if (auto *uLab = llvm::dyn_cast<RCUUnlockLabelLKMM>(lab)) {
			if (locks.empty())
				return uLab->getPos();
			locks.pop_back();
		}
	}
	return getLastThreadEvent(lock.thread).next();
}

Event ExecutionGraph::getMatchingSpeculativeRead(Event conf, Event *sc /* = nullptr */) const
{
	auto *cLab = llvm::dyn_cast<ReadLabel>(getEventLabel(conf));
	BUG_ON(!cLab);

	for (auto j = conf.index - 1; j > 0; j--) {
		const EventLabel *lab = getEventLabel(Event(conf.thread, j));

		if (sc && lab->isSC())
			*sc = lab->getPos();

		/* We don't care whether all previous confirmations are matched;
		 * the same speculation maybe confirmed multiple times (e.g., baskets) */
		if (auto *rLab = llvm::dyn_cast<SpeculativeReadLabel>(lab)) {
			if (rLab->getAddr() == cLab->getAddr())
				return rLab->getPos();
		}
	}
	return Event::getInit();
}

Event ExecutionGraph::getLastThreadUnmatchedLockLAPOR(const Event upperLimit) const
{
	std::vector<SAddr > unlocks;

	for (auto j = upperLimit.index; j >= 0; j--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, j));

		if (auto *lLab = llvm::dyn_cast<LockLabelLAPOR>(lab)) {
			if (std::find_if(unlocks.rbegin(), unlocks.rend(),
					 [&](SAddr addr){ return lLab->getLockAddr() == addr; })
			    ==  unlocks.rend())
				return lLab->getPos();
		}

		if (auto *uLab = llvm::dyn_cast<UnlockLabelLAPOR>(lab))
			unlocks.push_back(uLab->getLockAddr());
	}
	return Event::getInit();
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
	return Event::getInit();
}

Event ExecutionGraph::getLastThreadLockAtLocLAPOR(const Event upperLimit, SAddr loc) const
{
	for (auto j = upperLimit.index; j >= 0; j--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, j));

		if (auto *lLab = llvm::dyn_cast<LockLabelLAPOR>(lab)) {
			if (lLab->getLockAddr() == loc)
				return lLab->getPos();
		}

	}
	return Event::getInit();
}

Event ExecutionGraph::getLastThreadUnlockAtLocLAPOR(const Event upperLimit, SAddr loc) const
{
	for (auto j = upperLimit.index; j >= 0; j--) {
		const EventLabel *lab = getEventLabel(Event(upperLimit.thread, j));

		if (auto *lLab = llvm::dyn_cast<UnlockLabelLAPOR>(lab)) {
			if (lLab->getLockAddr() == loc)
				return lLab->getPos();
		}

	}
	return Event::getInit();
}

Event ExecutionGraph::getMalloc(const SAddr &addr) const
{
	auto it = std::find_if(label_begin(*this), label_end(*this), [&](auto &lab){
				       if (auto *aLab = llvm::dyn_cast<MallocLabel>(&lab))
					       return aLab->contains(addr);
				       return false;
				});
	return it != label_end(*this) ? it->getPos() : Event::getInit();
}

Event ExecutionGraph::getMallocCounterpart(const FreeLabel *fLab) const
{
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			const EventLabel *oLab = getEventLabel(Event(i, j));
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(oLab)) {
				if (aLab->getAllocAddr() == fLab->getFreedAddr())
					return aLab->getPos();
			}
		}
	}
	return Event::getInit();
}

Event ExecutionGraph::getMinimumStampEvent(const std::vector<Event> &es) const
{
	if (es.empty())
		return Event::getInit();
	return *std::min_element(es.begin(), es.end(), [&](const Event &e1, const Event &e2){
		return getEventLabel(e1)->getStamp() < getEventLabel(e2)->getStamp();
	});
}

Event ExecutionGraph::getPendingRMW(const WriteLabel *sLab) const
{
	/* If this is _not_ an RMW event, return an empty vector */
	if (!isRMWStore(sLab))
		return Event::getInit();

	/* Otherwise, scan for other RMWs that successfully read the same val */
	auto *pLab = llvm::dyn_cast<ReadLabel>(getPreviousLabel(sLab));
	BUG_ON(!pLab->getRf());
	std::vector<Event> pending;

	/* Fastpath: non-init rf */
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(pLab->getRf())) {
		std::for_each(wLab->readers_begin(), wLab->readers_end(), [&](auto &rLab){
				if (isRMWLoad(&rLab) && &rLab != pLab)
					pending.push_back(rLab.getPos());
			});
		return getMinimumStampEvent(pending);
	}

	/* Slowpath: scan init rfs */
	std::for_each(init_rf_begin(pLab->getAddr()), init_rf_end(pLab->getAddr()), [&](auto &rLab){
			     if (rLab.getRf() == pLab->getRf() && &rLab != pLab && isRMWLoad(&rLab))
				     pending.push_back(rLab.getPos());
	});
	return getMinimumStampEvent(pending);
}

std::vector<Event> ExecutionGraph::getRevisitable(const WriteLabel *sLab, const VectorClock &before) const
{
	auto pendingRMW = getPendingRMW(sLab);
	std::vector<Event> loads;

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = before.getMax(i) + 1u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getAddr() == sLab->getAddr() &&
				    rLab->isRevisitable() && rLab->wasAddedMax())
					loads.push_back(rLab->getPos());
			}
		}
	}
	if (!pendingRMW.isInitializer())
		loads.erase(std::remove_if(loads.begin(), loads.end(), [&](Event &e){
			auto *confLab = getEventLabel(pendingRMW);
			return getEventLabel(e)->getStamp() > confLab->getStamp();
		}), loads.end());
	return loads;
}

/* Returns a vector with all reads of a particular location reading from INIT */
std::vector<Event> ExecutionGraph::getInitRfsAtLoc(SAddr addr) const
{
	std::vector<Event> result;

	for (const auto &lab : labels(*this)) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab))
			if (rLab->getRf() && rLab->getRf()->getPos().isInitializer() && rLab->getAddr() == addr)
				result.push_back(rLab->getPos());
	}
	return result;
}


/*******************************************************************************
 **                       Label addition methods
 ******************************************************************************/

EventLabel *ExecutionGraph::addLabelToGraph(std::unique_ptr<EventLabel> lab)
{
	/* Assign stamp if necessary */
	if (!lab->hasStamp())
		lab->setStamp(nextStamp());

	auto pos = lab->getPos();
	if (pos.index < events[pos.thread].size()) {
		auto eLab = getEventLabel(pos);
		BUG_ON(eLab && !llvm::isa<EmptyLabel>(eLab));
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

bool ExecutionGraph::isCoMaximal(SAddr addr, Event e, bool checkCache /* = false */)
{
	if (e.isInitializer())
		return co_begin(addr) == co_end(addr);
	return co_begin(addr) != co_end(addr) && e == co_rbegin(addr)->getPos();
}

void ExecutionGraph::trackCoherenceAtLoc(SAddr addr)
{
	coherence[addr];
	getInitLabel()->initRfs.emplace(addr, CopyableIList<ReadLabel>());
}

/************************************************************
 ** Calculation of writes a read can read from
 ***********************************************************/

void ExecutionGraph::removeLast(unsigned int thread)
{
	auto *lab = getLastThreadLabel(thread);
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
		if (auto *wLab = llvm::dyn_cast_or_null<WriteLabel>(rLab->getRf())) {
			wLab->removeReader([&](ReadLabel &oLab){ return &oLab == rLab; });
		}
	}
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
		for (auto &rLab : wLab->readers()) {
			rLab.setRf(nullptr);
		}
	}
	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
		if (auto *aLab = llvm::dyn_cast_or_null<MallocLabel>(mLab->getAlloc()))
			aLab->removeAccess([&](auto &oLab){ return &oLab == mLab; });
	}
	if (auto *dLab = llvm::dyn_cast<FreeLabel>(lab)) {
		dLab->getAlloc()->setFree(nullptr);
	}
	if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
		if (auto *dLab = llvm::dyn_cast_or_null<FreeLabel>(aLab->getFree()))
			dLab->setAlloc(nullptr);
	}
	/* Nothing to do for create/join: childId remains the same */
	resizeThread(lab->getPos());
}

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

bool ExecutionGraph::isStoreReadByExclusiveRead(Event store, SAddr ptr) const
{
	for (const auto &lab : labels(*this)) {
		if (!isRMWLoad(&lab))
			continue;

		auto *rLab = llvm::dyn_cast<ReadLabel>(&lab);
		if (rLab->getRf()->getPos() == store && rLab->getAddr() == ptr)
			return true;
	}
	return false;
}

bool ExecutionGraph::isStoreReadBySettledRMW(Event store, SAddr ptr, const VectorClock &prefix) const
{
	for (const auto &lab : labels(*this)) {
		if (!isRMWLoad(&lab))
			continue;

		auto *rLab = llvm::dyn_cast<ReadLabel>(&lab);
		if (rLab->getRf()->getPos() != store || rLab->getAddr() != ptr)
			continue;

		auto *wLab = llvm::dyn_cast<WriteLabel>(getNextLabel(rLab));
		if (!rLab->isRevisitable() && !wLab->hasAttr(WriteAttr::RevBlocker))
			return true;
		if (prefix.contains(rLab->getPos()))
			return true;
	}
	return false;
}


/************************************************************
 ** Graph modification methods
 ***********************************************************/

void ExecutionGraph::changeRf(Event read, Event store)
{
	/* First, we set the new reads-from edge */
	auto *rLab = llvm::dyn_cast<ReadLabel>(getEventLabel(read));
	BUG_ON(!rLab);
	auto oldRfLab = rLab->getRf();
	rLab->setRf(store.isBottom() ? nullptr : getEventLabel(store));

	/*
	 * Now, we need to delete the read from the readers list of oldRf.
	 * For that we need to check:
	 *     1) That the old write it was reading from still exists
	 *     2) That oldRf has not been deleted (and a different event is
	 *        now in its place, perhaps after the restoration of some prefix
	 *        during a revisit)
	 *     3) That oldRf is not the initializer */
	if (oldRfLab && containsPos(oldRfLab->getPos())) {
		BUG_ON(!containsLab(oldRfLab));
		if (auto *oldLab = llvm::dyn_cast<WriteLabel>(oldRfLab))
			oldLab->removeReader([&](ReadLabel &oLab){ return &oLab == rLab; });
		else if (oldRfLab->getPos().isInitializer())
			removeInitRfToLoc(rLab);
	}

	/* If this read is now reading from bottom, nothing else to do */
	if (store.isBottom())
		return;

	/* Otherwise, add it to the write's reader list */
	auto *rfLab = getEventLabel(store);
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab)) {
		wLab->addReader(rLab);
	} else {
		BUG_ON(!store.isInitializer());
		addInitRfToLoc(rLab);
	}
}

void ExecutionGraph::addAlloc(MallocLabel *aLab, MemAccessLabel *mLab)
{
	if (aLab) {
		mLab->setAlloc(aLab);
		aLab->addAccess(mLab);
	}
}

/*
 * In the case where the events are not added out-of-order in the graph
 * (i.e., an event has a larger timestamp than all its po-predecessors)
 * we can obtain a view of the graph, given a timestamp. This function
 * returns such a view.
 */
std::unique_ptr<VectorClock>
ExecutionGraph::getViewFromStamp(Stamp stamp) const
{
	auto preds = std::make_unique<View>();

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = (int) getThreadSize(i) - 1; j >= 0; j--) {
			auto *lab = getEventLabel(Event(i, j));
			if (lab->getStamp() <= stamp) {
				preds->setMax(Event(i, j));
				break;
			}
		}
	}
	return preds;
}

void ExecutionGraph::removeAfter(const VectorClock &preds)
{
	VSet<SAddr> keep;

	/* Check which locations should be kept */
	for (auto i = 0u; i < preds.size(); i++) {
		for (auto j = 0u; j <= preds.getMax(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab))
				keep.insert(mLab->getAddr());
		}
	}

	auto rIt = getInitLabel()->initRfs.begin();
	for (auto lIt = coherence.begin(); lIt != coherence.end(); /* empty */) {
		/* Should we keep this memory location lying around? */
		if (!keep.count(lIt->first)) {
			lIt = coherence.erase(lIt);
			rIt = getInitLabel()->initRfs.erase(rIt);
		} else {
			for (auto sIt = lIt->second.begin(); sIt != lIt->second.end(); ) {
				if (!preds.contains(sIt->getPos()))
					sIt = lIt->second.erase(sIt);
				else
					++sIt;
			}
			getInitLabel()->removeReader(lIt->first, [&](auto &rLab){
				return !preds.contains(rLab.getPos());
			});
			++lIt;
			++rIt;
		}
	}
}


void ExecutionGraph::cutToStamp(Stamp stamp)
{
	auto preds = getViewFromStamp(stamp);

	/* Inform all calculators about the events cutted */
	removeAfter(*preds);

	/* Remove any 'pointers' to events that will be removed */
	for (auto i = 0u; i < preds->size(); i++) {
		for (auto j = 0u; j <= preds->getMax(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				wLab->removeReader([&](ReadLabel &rLab){
					return !preds->contains(rLab.getPos());
				});
			}
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getRf() && !preds->contains(rLab->getRf()->getPos()))
					rLab->setRf(nullptr);
			}
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
				if (mLab->getAlloc() && !preds->contains(mLab->getAlloc()))
					mLab->setAlloc(nullptr);
			}
			if (auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(lab)) {
				if (eLab->getParentJoin() && !preds->contains(eLab->getParentJoin()->getPos()))
					eLab->setParentJoin(nullptr);
			}
			if (auto *dLab = llvm::dyn_cast<FreeLabel>(lab)) {
				if (dLab->getAlloc() && !preds->contains(dLab->getAlloc()))
					dLab->setAlloc(nullptr);
			}
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
				if (aLab->getFree() && !preds->contains(aLab->getFree()->getPos()))
					aLab->setFree(nullptr);
				aLab->removeAccess([&](MemAccessLabel &mLab){
					return !preds->contains(mLab.getPos());
				});
			}
			/* No special action for CreateLabels; we can
			 * keep the begin event of the child the since
			 * it will not be deleted */
		}
	}

	/* Restrict the graph according to the view (keep begins around) */
	for (auto i = 0u; i < getNumThreads(); i++) {
		auto &thr = events[i];
		thr.erase(thr.begin() + preds->getMax(i) + 1, thr.end());
	}
	return;
}

void ExecutionGraph::compressStampsAfter(Stamp st)
{
	resetStamp(st + 1);
	for (auto &lab : labels(*this)) {
		if (lab.getStamp() > st)
			lab.setStamp(nextStamp());
	}
}

void ExecutionGraph::copyGraphUpTo(ExecutionGraph &other, const VectorClock &v) const
{
	/* First, populate calculators, etc */
	other.timestamp = timestamp;

	other.recoveryTID = recoveryTID;

	other.bam = bam;

	/* Then, copy the appropriate events */
	// FIXME: The reason why we resize to num of threads instead of v.size() is
	// to keep the same size as the interpreter threads.
	other.events.resize(getNumThreads());
	for (auto i = 0u; i < getNumThreads(); i++) {
		/* Skip the initializer */
		if (i != 0)
			other.addLabelToGraph(getEventLabel(Event(i, 0))->clone());
		for (auto j = 1; j <= v.getMax(i); j++) {
			if (!v.contains(Event(i, j))) {
				other.addLabelToGraph(createHoleLabel(Event(i, j)));
				continue;
			}
			auto *nLab = other.addLabelToGraph(getEventLabel(Event(i, j))->clone());
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(nLab)) {
				const_cast<WriteLabel *>(wLab)->removeReader([&v](ReadLabel &rLab){
					return !v.contains(rLab.getPos());
				});
			}
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(nLab))
				other.trackCoherenceAtLoc(mLab->getAddr());
			if (auto *tcLab = llvm::dyn_cast<ThreadCreateLabel>(nLab))
				;
		}
	}

	for (auto &lab : labels(other)) {
		auto *rLab = llvm::dyn_cast<ReadLabel>(&lab);
		if (rLab && rLab->getRf()) {
			if (!other.containsPos(rLab->getRf()->getPos()))
				rLab->setRf(nullptr);
			else
				rLab->setRf(other.getEventLabel(
						    rLab->getRf()->getPos()));
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(&lab)) {
			wLab->removeReader([](auto &rLab){ return true; });
			for (auto &oLab : getWriteLabel(lab.getPos())->readers())
				if (v.contains(oLab.getPos()))
					wLab->addReader(other.getReadLabel(oLab.getPos()));
		}
		auto *mLab = llvm::dyn_cast<MemAccessLabel>(&lab);
		if (mLab && mLab->getAlloc()) {
			if (!other.containsPos(mLab->getAlloc()->getPos())) {
				mLab->setAlloc(nullptr);
			} else {
				auto *aLab = llvm::dyn_cast<MallocLabel>(
					other.getEventLabel(mLab->getAlloc()->getPos()));
				mLab->setAlloc(aLab);
			}
		}
		auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(&lab);
		if (eLab && eLab->getParentJoin()) {
			if (!v.contains(eLab->getParentJoin()->getPos())) {
				eLab->setParentJoin(nullptr);
			} else {
				auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(other.getEventLabel(eLab->getParentJoin()->getPos()));
				eLab->setParentJoin(jLab);
			}
		}
		auto *aLab = llvm::dyn_cast<MallocLabel>(&lab);
		if (aLab && aLab->getFree()) {
			if (!v.contains(aLab->getFree()->getPos())) {
				aLab->setFree(nullptr);
			} else {
				auto *dLab = llvm::dyn_cast<FreeLabel>(other.getEventLabel(aLab->getFree()->getPos()));
				aLab->setFree(dLab);
			}
		}
		if (aLab) {
			aLab->removeAccess([](auto &mLab){ return true; });
			for (auto &oLab : llvm::dyn_cast<MallocLabel>(getEventLabel(lab.getPos()))->accesses())
				if (v.contains(oLab.getPos()))
					aLab->addAccess(llvm::dyn_cast<MemAccessLabel>(other.getEventLabel(oLab.getPos())));
		}
		auto *dLab = llvm::dyn_cast<FreeLabel>(&lab);
		if (dLab && dLab->getAlloc()) {
			if (!v.contains(dLab->getAlloc()->getPos())) {
				dLab->setAlloc(nullptr);
			} else {
				auto *aLab = llvm::dyn_cast<MallocLabel>(other.getEventLabel(dLab->getAlloc()->getPos()));
				dLab->setAlloc(aLab);
			}
		}
	}

	/* Finally, copy coherence info */
	/* FIXME: Temporary ugly hack */
	// for (auto it = cc->begin(); it != cc->end(); ++it) {
	// 	for (auto sIt = it->second.begin(); sIt != it->second.end(); ++sIt) {
	// 		if (v.contains(*sIt)) {
	// 			occ->addStoreToLoc(it->first, *sIt, -1);
	// 		}
	// 	}
	// }
	for (auto lIt = loc_begin(), lE = loc_end(); lIt != lE; ++lIt) {
		for (auto sIt = lIt->second.begin(); sIt != lIt->second.end(); ++sIt)
			if (v.contains(sIt->getPos())) {
				other.addStoreToCO(other.getWriteLabel(sIt->getPos()), other.co_end(lIt->first));
			}
	}
	for (auto it = loc_begin(); it != loc_end(); ++it) {
		auto &initRfs = getInitLabel()->initRfs;
		for (auto rIt = initRfs.at(it->first).begin(); rIt != initRfs.at(it->first).end(); ++rIt) {
			if (v.contains(rIt->getPos())) {
				other.addInitRfToLoc(other.getReadLabel(rIt->getPos()));
			}
		}
	}
	/* FIXME: Make sure all fields are copied */
	return;
}

std::unique_ptr<ExecutionGraph> ExecutionGraph::getCopyUpTo(const VectorClock &v) const
{
	auto og = std::unique_ptr<ExecutionGraph>(new ExecutionGraph());
	copyGraphUpTo(*og, v);
	return og;
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

	return isRMWStore(mLab) && mLab->getAddr() == rLab->getAddr();
}


/************************************************************
 ** Debugging methods
 ***********************************************************/

void ExecutionGraph::validate(void)
{
	for (auto &lab : labels(*this)) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab)) {
			if (!rLab->getRf())
				continue;

			if (!containsLab(rLab->getRf())) {
				llvm::errs() << "Non-existent RF: " << rLab->getPos() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}

			if (auto *rfLab = llvm::dyn_cast<WriteLabel>(rLab->getRf())) {
				if (std::all_of(rfLab->readers_begin(), rfLab->readers_end(), [rLab](auto &oLab){ return &oLab != rLab; })) {
					llvm::errs() << "Not in RF's readers list: " << rLab->getPos() << "\n";
					llvm::errs() << *this << "\n";
					BUG();
				}
			}
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(&lab)) {
			if (isRMWStore(wLab) &&
			    std::count_if(wLab->readers_begin(), wLab->readers_end(),
					  [&](auto &rLab){ return isRMWLoad(&rLab); }) > 1) {
				llvm::errs() << "Atomicity violation: " << wLab->getPos() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}

			if (std::any_of(wLab->readers_begin(), wLab->readers_end(),
					[&](auto &rLab){ return !containsPosNonEmpty(rLab.getPos()); })) {
				llvm::errs() << "Non-existent/non-read reader: " << wLab->getPos() << "\n";
				llvm::errs() << "Readers: ";
				for (auto &rLab : wLab->readers())
					llvm::errs() << rLab.getPos() << " ";
				llvm::errs() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}

			if (std::any_of(wLab->readers_begin(), wLab->readers_end(),
					[&](auto &rLab){ return rLab.getRf() != wLab; })) {
				llvm::errs() << "RF not properly set: " << wLab->getPos() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}
			for (auto it = wLab->readers_begin(), ie = wLab->readers_end(); it != ie; ++it) {
				if (!containsPosNonEmpty(it->getPos())) {
					llvm::errs() << "Readers list has garbage: " << *it << "\n";
					llvm::errs() << *this << "\n";
					BUG();
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
			s << "\t" << lab->getStamp() << " @ " << *lab << "\n";
		}
	}
	s << "Thread sizes:\n\t";
	for (auto i = 0u; i < g.getNumThreads(); i++)
		s << g.getThreadSize(i) << " ";
	s << "\n";

	for (auto lIt = g.loc_begin(), lE = g.loc_end(); lIt != lE; ++lIt) {
		s << lIt->first << ": ";
		for (auto sIt = g.co_begin(lIt->first); sIt != g.co_end(lIt->first); ++sIt)
			s << sIt->getPos() << " ";
		s << "\n";
	}
	return s;
}
