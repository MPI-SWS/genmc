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
#include "ExecutionGraph/GraphIterators.hpp"
#include "Support/Parser.hpp"
#include <llvm/IR/DebugInfo.h>

/************************************************************
 ** Basic getter methods
 ***********************************************************/

Event ExecutionGraph::getMinimumStampEvent(const std::vector<Event> &es) const
{
	if (es.empty())
		return Event::getInit();
	return *std::min_element(es.begin(), es.end(), [&](const Event &e1, const Event &e2) {
		return getEventLabel(e1)->getStamp() < getEventLabel(e2)->getStamp();
	});
}

Event ExecutionGraph::getPendingRMW(const WriteLabel *sLab) const
{
	/* If this is _not_ an RMW event, return an empty vector */
	if (!sLab->isRMW())
		return Event::getInit();

	/* Otherwise, scan for other RMWs that successfully read the same val */
	auto *pLab = llvm::dyn_cast<ReadLabel>(po_imm_pred(sLab));
	BUG_ON(!pLab->getRf());
	std::vector<Event> pending;

	/* Fastpath: non-init rf */
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(pLab->getRf())) {
		std::for_each(wLab->readers_begin(), wLab->readers_end(), [&](auto &rLab) {
			if (rLab.isRMW() && &rLab != pLab)
				pending.push_back(rLab.getPos());
		});
		return getMinimumStampEvent(pending);
	}

	/* Slowpath: scan init rfs */
	std::for_each(init_rf_begin(pLab->getAddr()), init_rf_end(pLab->getAddr()),
		      [&](auto &rLab) {
			      if (rLab.getRf() == pLab->getRf() && &rLab != pLab && rLab.isRMW())
				      pending.push_back(rLab.getPos());
		      });
	return getMinimumStampEvent(pending);
}

std::vector<ReadLabel *> ExecutionGraph::getRevisitable(WriteLabel *sLab, const VectorClock &before)
{
	auto pendingRMW = getPendingRMW(sLab);
	std::vector<ReadLabel *> loads;

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = before.getMax(i) + 1u; j < getThreadSize(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getAddr() == sLab->getAddr() && rLab->isRevisitable() &&
				    rLab->wasAddedMax())
					loads.push_back(rLab);
			}
		}
	}
	if (!pendingRMW.isInitializer())
		loads.erase(std::remove_if(loads.begin(), loads.end(),
					   [&](auto &eLab) {
						   const auto *confLab = getEventLabel(pendingRMW);
						   return eLab->getStamp() > confLab->getStamp();
					   }),
			    loads.end());
	return loads;
}

/*******************************************************************************
 **                       Label addition methods
 ******************************************************************************/

EventLabel *ExecutionGraph::addLabelToGraph(std::unique_ptr<EventLabel> lab)
{
	lab->setParent(this);

	/* Assign stamp if necessary */
	if (!lab->hasStamp())
		lab->setStamp(nextStamp());

	/* Track coherence if necessary */
	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(&*lab))
		trackCoherenceAtLoc(mLab->getAddr());

	auto pos = lab->getPos();
	if (pos.index < events[pos.thread].size()) {
		auto eLab = getEventLabel(pos);
		BUG_ON(eLab && !llvm::isa<EmptyLabel>(eLab));
		insertionOrder.remove(*events[pos.thread][pos.index]);
		events[pos.thread][pos.index] = std::move(lab);
	} else {
		events[pos.thread].push_back(std::move(lab));
	}
	insertionOrder.push_back(*events[pos.thread][pos.index]);
	BUG_ON(pos.index > events[pos.thread].size());
	return getEventLabel(pos);
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
			wLab->removeReader([&](ReadLabel &oLab) { return &oLab == rLab; });
		}
	}
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
		for (auto &rLab : wLab->readers()) {
			rLab.setRfNoCascade(nullptr);
		}
	}
	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
		if (auto *aLab = llvm::dyn_cast_or_null<MallocLabel>(mLab->getAlloc()))
			aLab->removeAccess([&](auto &oLab) { return &oLab == mLab; });
	}
	if (auto *dLab = llvm::dyn_cast<FreeLabel>(lab)) {
		dLab->getAlloc()->setFree(nullptr);
	}
	if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
		if (auto *dLab = llvm::dyn_cast_or_null<FreeLabel>(aLab->getFree()))
			dLab->setAlloc(nullptr);
	}
	/* Nothing to do for create/join: childId remains the same */
	insertionOrder.remove(*lab);
	resizeThread(lab->getPos());
}

/************************************************************
 ** Graph modification methods
 ***********************************************************/

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
std::unique_ptr<VectorClock> ExecutionGraph::getViewFromStamp(Stamp stamp) const
{
	auto preds = std::make_unique<View>();

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = (int)getThreadSize(i) - 1; j >= 0; j--) {
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
			for (auto sIt = lIt->second.begin(); sIt != lIt->second.end();) {
				if (!preds.contains(sIt->getPos()))
					sIt = lIt->second.erase(sIt);
				else
					++sIt;
			}
			getInitLabel()->removeReader(lIt->first, [&](auto &rLab) {
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
	for (auto labIt = insertionOrder.begin(), labE = insertionOrder.end(); labIt != labE;) {
		labIt = preds->contains(labIt->getPos()) ? ++labIt : insertionOrder.erase(labIt);
	}

	/* Remove any 'pointers' to events that will be removed */
	for (auto i = 0U; i < preds->size(); i++) {
		for (auto j = 0U; j <= preds->getMax(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				wLab->removeReader([&](ReadLabel &rLab) {
					return !preds->contains(rLab.getPos());
				});
			}
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getRf() && !preds->contains(rLab->getRf()->getPos()))
					rLab->setRfNoCascade(nullptr);
			}
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
				if (mLab->getAlloc() && !preds->contains(mLab->getAlloc()))
					mLab->setAlloc(nullptr);
			}
			if (auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(lab)) {
				if (eLab->getParentJoin() &&
				    !preds->contains(eLab->getParentJoin()->getPos()))
					eLab->setParentJoin(nullptr);
			}
			if (auto *dLab = llvm::dyn_cast<FreeLabel>(lab)) {
				if (dLab->getAlloc() && !preds->contains(dLab->getAlloc()))
					dLab->setAlloc(nullptr);
			}
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
				if (aLab->getFree() && !preds->contains(aLab->getFree()->getPos()))
					aLab->setFree(nullptr);
				aLab->removeAccess([&](MemAccessLabel &mLab) {
					return !preds->contains(mLab.getPos());
				});
			}
			/* No special action for CreateLabels; we can
			 * keep the begin event of the child the since
			 * it will not be deleted */
		}
	}

	/* Restrict the graph according to the view (keep begins around) */
	for (auto i = 0U; i < getNumThreads(); i++) {
		auto &thr = events[i];
		thr.erase(thr.begin() + preds->getMax(i) + 1, thr.end());
	}

	/* Fix stamps */
	resetStamp(0U);
	for (auto &lab : labels())
		lab.setStamp(nextStamp());
}

void ExecutionGraph::copyGraphUpTo(ExecutionGraph &other, const VectorClock &v) const
{
	other.recoveryTID = recoveryTID;

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
				const_cast<WriteLabel *>(wLab)->removeReader([&v](ReadLabel &rLab) {
					return !v.contains(rLab.getPos());
				});
			}
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(nLab))
				other.trackCoherenceAtLoc(mLab->getAddr());
			if (auto *tcLab = llvm::dyn_cast<ThreadCreateLabel>(nLab))
				;
		}
	}

	other.insertionOrder.clear();
	other.resetStamp(0);
	for (auto &lab : insertionOrder) {
		if (v.contains(lab.getPos()) || lab.getIndex() <= v.getMax(lab.getThread())) {
			other.insertionOrder.push_back(*other.getEventLabel(lab.getPos()));
			other.getEventLabel(lab.getPos())->setStamp(other.nextStamp());
		}
	}

	for (auto &lab : other.labels()) {
		auto *rLab = llvm::dyn_cast<ReadLabel>(&lab);
		if (rLab && rLab->getRf()) {
			if (!other.containsPos(rLab->getRf()->getPos()))
				rLab->setRfNoCascade(nullptr);
			else
				rLab->setRfNoCascade(other.getEventLabel(rLab->getRf()->getPos()));
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(&lab)) {
			wLab->removeReader([](auto &rLab) { return true; });
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
				auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(
					other.getEventLabel(eLab->getParentJoin()->getPos()));
				eLab->setParentJoin(jLab);
			}
		}
		auto *aLab = llvm::dyn_cast<MallocLabel>(&lab);
		if (aLab && aLab->getFree()) {
			if (!v.contains(aLab->getFree()->getPos())) {
				aLab->setFree(nullptr);
			} else {
				auto *dLab = llvm::dyn_cast<FreeLabel>(
					other.getEventLabel(aLab->getFree()->getPos()));
				aLab->setFree(dLab);
			}
		}
		if (aLab) {
			aLab->removeAccess([](auto &mLab) { return true; });
			for (auto &oLab :
			     llvm::dyn_cast<MallocLabel>(getEventLabel(lab.getPos()))->accesses())
				if (v.contains(oLab.getPos()))
					aLab->addAccess(llvm::dyn_cast<MemAccessLabel>(
						other.getEventLabel(oLab.getPos())));
		}
		auto *dLab = llvm::dyn_cast<FreeLabel>(&lab);
		if (dLab && dLab->getAlloc()) {
			if (!v.contains(dLab->getAlloc()->getPos())) {
				dLab->setAlloc(nullptr);
			} else {
				auto *aLab = llvm::dyn_cast<MallocLabel>(
					other.getEventLabel(dLab->getAlloc()->getPos()));
				dLab->setAlloc(aLab);
			}
		}
	}

	/* Finally, copy coherence info */
	for (auto lIt = loc_begin(), lE = loc_end(); lIt != lE; ++lIt) {
		for (auto sIt = lIt->second.begin(); sIt != lIt->second.end(); ++sIt)
			if (v.contains(sIt->getPos())) {
				other.addStoreToCOAfter(other.getWriteLabel(sIt->getPos()),
							other.co_max(sIt->getAddr()));
			}
	}
	for (auto it = loc_begin(); it != loc_end(); ++it) {
		auto &initRfs = getInitLabel()->initRfs;
		for (auto rIt = initRfs.at(it->first).begin(); rIt != initRfs.at(it->first).end();
		     ++rIt) {
			if (v.contains(rIt->getPos())) {
				other.addInitRfToLoc(other.getReadLabel(rIt->getPos()));
			}
		}
	}
}

std::unique_ptr<ExecutionGraph> ExecutionGraph::getCopyUpTo(const VectorClock &v) const
{
	auto og = std::unique_ptr<ExecutionGraph>(new ExecutionGraph(this->initValGetter_));
	copyGraphUpTo(*og, v);
	return og;
}

/************************************************************
 ** Debugging methods
 ***********************************************************/

void ExecutionGraph::validate(void)
{
	for (auto &lab : labels()) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab)) {
			if (!rLab->getRf())
				continue;

			if (!containsLab(rLab->getRf())) {
				llvm::errs() << "Non-existent RF: " << rLab->getPos() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}

			if (auto *rfLab = llvm::dyn_cast<WriteLabel>(rLab->getRf())) {
				if (std::all_of(rfLab->readers_begin(), rfLab->readers_end(),
						[rLab](auto &oLab) { return &oLab != rLab; })) {
					llvm::errs()
						<< "Not in RF's readers list: " << rLab->getPos()
						<< "\n";
					llvm::errs() << *this << "\n";
					BUG();
				}
			}
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(&lab)) {
			if (wLab->isRMW() &&
			    std::count_if(wLab->readers_begin(), wLab->readers_end(),
					  [&](auto &rLab) { return rLab.isRMW(); }) > 1) {
				llvm::errs() << "Atomicity violation: " << wLab->getPos() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}

			if (std::any_of(wLab->readers_begin(), wLab->readers_end(),
					[&](auto &rLab) {
						return !containsPosNonEmpty(rLab.getPos());
					})) {
				llvm::errs() << "Non-existent/non-read reader: " << wLab->getPos()
					     << "\n";
				llvm::errs() << "Readers: ";
				for (auto &rLab : wLab->readers())
					llvm::errs() << rLab.getPos() << " ";
				llvm::errs() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}

			if (std::any_of(wLab->readers_begin(), wLab->readers_end(),
					[&](auto &rLab) { return rLab.getRf() != wLab; })) {
				llvm::errs() << "RF not properly set: " << wLab->getPos() << "\n";
				llvm::errs() << *this << "\n";
				BUG();
			}
			for (auto it = wLab->readers_begin(), ie = wLab->readers_end(); it != ie;
			     ++it) {
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

llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const ExecutionGraph &g)
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
