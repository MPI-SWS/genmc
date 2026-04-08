/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"

#include <iostream>
#include <memory>

/************************************************************
 ** Basic getter methods
 ***********************************************************/

[[nodiscard]] auto ExecutionGraph::resolveAccessValue(const EventLabel *lab,
						      const AAccess &access) const -> SVal
{
	auto &state = getState();

	/* Special case for initializer */
	if (lab->getPos().isInitializer()) {
		VERIFY(!(haveNAs_ && access.addr.isDynamic()));
		auto val = access.addr.isDynamic() ? SVal(0) : getInitVal(access);
		/* TODO (WRD): View{} assumes the graph is complete here */
		return haveNAs_ ? val : state.reconstructMemValue(access, View{}, val);
	}

	const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	VERIFY(wLab);
	return haveNAs_ ? wLab->getVal()
			: state.reconstructMemValue(access, getConsChecker()->getHbView(wLab),
						    wLab->getVal());
}

/*
 * In the case where the events are not added out-of-order in the graph
 * (i.e., an event has a larger timestamp than all its po-predecessors)
 * we can obtain a view of the graph, given a timestamp. This function
 * returns such a view.
 */
auto ExecutionGraph::getViewFromStamp(Stamp stamp) const -> std::unique_ptr<VectorClock>
{
	auto preds = std::make_unique<View>();

	for (auto i = 0U; i < getNumThreads(); i++) {
		for (auto j = (int)getThreadSize(i) - 1; j >= 0; j--) {
			const auto *lab = getEventLabel(Event(i, j));
			if (lab->getStamp() <= stamp) {
				preds->setMax(Event(i, j));
				break;
			}
		}
	}
	return preds;
}

auto ExecutionGraph::getCurrentMemValue(SAddr addr, ASize size) -> SVal
{
	auto &state = getState();
	VERIFY(!haveNAs_);

	/* TODO: slow version, see how to optimize later */
	SVal res;
	for (auto i = 0u; i < size; i++) {
		auto info = state.getAtomicWriteInfo(addr + i);

		/* Only has initial value */
		if (!info) {
			/* TODO: missing check for invalid access */
			SVal initVal = addr.isDynamic() ? SVal(0)
							: getInitVal(AAccess(addr + i, 1));
			/* TODO (WRD): View{} assumes the graph is complete here */
			res |= state.reconstructMemValue({addr + i, 1}, View{}, initVal)
			       << SVal(i * 8);
			continue;
		}

		/* At least one atomic write on this location */
		auto atomicAddr = info->first;
		auto *wLab = genmc::cast<WriteLabel>(co_max(atomicAddr));
		auto offset = addr + i - atomicAddr;
		res |= state.reconstructMemValue({addr + i, 1}, getConsChecker()->getHbView(wLab),
						 (wLab->getVal() >> SVal(offset * 8)) & SVal(0xFF))
		       << SVal(i * 8);
	}
	return res;
}

/*******************************************************************************
 **                       Label addition methods
 ******************************************************************************/

auto ExecutionGraph::addLabelToGraph(std::unique_ptr<EventLabel> lab) -> EventLabel *
{
	lab->setParent(this);

	/* Assign stamp if necessary */
	if (!lab->hasStamp())
		lab->setStamp(nextStamp());

	/* Track coherence if necessary */
	if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(&*lab)) {
		trackCoherenceAtLoc(mLab->getAddr());
		accessMap_[mLab->getAddr()].push_back(&*lab);
	}
	/* XXX: Track accesses to each location (no allocs; temp fix) */
	if (auto *dLab = genmc::dyn_cast<FreeLabel>(&*lab)) {
		trackCoherenceAtLoc(dLab->getAddr());
		accessMap_[dLab->getAddr()].push_back(&*lab);
	}

	auto pos = lab->getPos();
	auto *lastLab = getLastThreadLabel(pos.thread);
	if (lastLab && pos.index < lastLab->getIndex()) {
		auto eLab = getEventLabel(pos);
		ASSERT(!eLab || genmc::isa<EmptyLabel>(eLab));
		auto &oldLab = *events[pos.thread][pos.index];
		auto it = poLists[pos.thread].erase(po_iterator(oldLab));
		insertionOrder.remove(oldLab);
		events[pos.thread][pos.index] = std::move(lab);
		auto &newLab = *events[pos.thread][pos.index];
		insertionOrder.push_back(newLab);
		poLists[pos.thread].insert(it, newLab);
		ASSERT(pos.index <= events[pos.thread].size());
		return &newLab;
	}
	events[pos.thread].push_back(std::move(lab));
	auto &newLab = *events[pos.thread].back();
	insertionOrder.push_back(newLab);
	poLists[pos.thread].push_back(newLab);
	return &newLab;
}

void ExecutionGraph::trackCoherenceAtLoc(SAddr addr)
{
	coherence[addr];
	getInitLabel()->initRfs.emplace(addr, CopyableIList<ReadLabel>());
	accessMap_[addr];
}

/************************************************************
 ** Graph modification methods
 ***********************************************************/

auto ExecutionGraph::removeLast(unsigned int thread) -> std::unique_ptr<EventLabel>
{
	auto *lab = getLastThreadLabel(thread);
	if (auto *rLab = genmc::dyn_cast_if_present<ReadLabel>(lab)) {
		if (auto *wLab = genmc::dyn_cast_if_present<WriteLabel>(rLab->getRf())) {
			wLab->removeReader([&](ReadLabel &oLab) { return &oLab == rLab; });
		}
		if (auto *iLab = genmc::dyn_cast_if_present<InitLabel>(rLab->getRf())) {
			iLab->removeReader(rLab->getAddr(),
					   [&](ReadLabel &oLab) { return &oLab == rLab; });
		}
	}
	if (auto *wLab = genmc::dyn_cast<WriteLabel>(lab)) {
		for (auto &rLab : wLab->readers()) {
			rLab.setRfNoCascade(nullptr);
		}
	}
	if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab)) {
		auto &accesses = accessMap_[mLab->getAddr()];
		accesses.erase(std::remove(accesses.begin(), accesses.end(), mLab), accesses.end());
	}
	if (auto *dLab = genmc::dyn_cast<FreeLabel>(lab)) {
		auto &accesses = accessMap_[dLab->getAddr()];
		accesses.erase(std::remove(accesses.begin(), accesses.end(), dLab), accesses.end());
	}
	if (auto *cLab = genmc::dyn_cast<ThreadCreateLabel>(lab)) {
		if (auto *tsLab = genmc::dyn_cast_if_present<ThreadStartLabel>(
			    getFirstThreadLabel(lab->getThread())))
			tsLab->setCreate(nullptr);
	}
	if (auto *tjLab = genmc::dyn_cast<ThreadJoinLabel>(lab)) {
		if (auto *eLab = genmc::dyn_cast_if_present<ThreadFinishLabel>(
			    getLastThreadLabel(tjLab->getChildId())))
			eLab->setParentJoin(nullptr);
	}
	/* Nothing to do for start/finish: childId remains the same */

	/* Remove from lists */
	insertionOrder.remove(*lab);
	poLists[lab->getThread()].remove(*lab);
	lab->setParent(nullptr);

	/* Extract ownership and return */
	auto &threadEvents = events[thread];
	auto popped = std::move(threadEvents.back());
	threadEvents.pop_back();
	return popped;
}

void ExecutionGraph::removeAfter(const VectorClock &preds)
{
	VSet<SAddr> keep;

	/* Check which locations should be kept */
	for (auto i = 0U; i < preds.size(); i++) {
		for (auto j = 0U; j <= preds.getMax(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab))
				keep.insert(mLab->getAddr());
		}
	}

	for (auto lIt = coherence.begin(); lIt != coherence.end(); /* empty */) {
		/* Should we keep this memory location lying around? */
		if (!keep.count(lIt->first)) {
			getInitLabel()->initRfs.erase(lIt->first);
			accessMap_.erase(lIt->first);
			lIt = coherence.erase(lIt);
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
			auto &accesses = accessMap_[lIt->first];
			accesses.erase(std::remove_if(accesses.begin(), accesses.end(),
						      [&](auto *lab) {
							      return !preds.contains(lab->getPos());
						      }),
				       accesses.end());
			++lIt;
		}
	}
}

void ExecutionGraph::cutToStamp(Stamp stamp)
{
	auto preds = getViewFromStamp(stamp);

	/* Inform all calculators about the events cutted */
	removeAfter(*preds);
	for (auto labIt = insertionOrder.begin(), labE = insertionOrder.end(); labIt != labE;) {
		if (preds->contains(labIt->getPos()))
			++labIt;
		else {
			poLists[labIt->getThread()].remove(*labIt);
			labIt = insertionOrder.erase(labIt);
		}
	}

	/* Remove any 'pointers' to events that will be removed */
	for (auto i = 0U; i < getNumThreads(); i++) {
		for (auto j = 0U; j <= preds->getMax(i); j++) {
			auto *lab = getEventLabel(Event(i, j));
			if (auto *wLab = genmc::dyn_cast<WriteLabel>(lab)) {
				wLab->removeReader([&](ReadLabel &rLab) {
					return !preds->contains(rLab.getPos());
				});
			}
			if (auto *rLab = genmc::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getRf() && !preds->contains(rLab->getRf()->getPos()))
					rLab->setRfNoCascade(nullptr);
			}
			if (auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(lab)) {
				if (tsLab->getCreate() &&
				    !preds->contains(tsLab->getCreate()->getPos()))
					tsLab->setCreate(nullptr);
			}
			if (auto *eLab = genmc::dyn_cast<ThreadFinishLabel>(lab)) {
				if (eLab->getParentJoin() &&
				    !preds->contains(eLab->getParentJoin()->getPos()))
					eLab->setParentJoin(nullptr);
			}
			if (auto *begLab = genmc::dyn_cast<MethodBeginLabel>(lab)) {
				begLab->removePredNoCascade([&](auto *endLab) {
					return !preds->contains(endLab->getPos());
				});
			}
			if (auto *endLab = genmc::dyn_cast<MethodEndLabel>(lab)) {
				endLab->removeSuccNoCascade([&](auto *begLab) {
					return !preds->contains(begLab->getPos());
				});
			}
		}
	}

	/* Restrict the graph according to the view (keeps begins around) */
	for (auto i = 0U; i < getNumThreads(); i++) {
		auto &thr = events[i];
		thr.erase(thr.begin() + preds->getMax(i) + 1, thr.end());
	}

	/* Remove begins as well */
	for (auto i : std::views::reverse(thr_ids()) | std::views::take_while([this](auto i) {
			      return getThreadSize(i) == 1 &&
				     !genmc::isa<InitLabel>(getFirstThreadLabel(i));
		      })) {
		auto *bLab = getFirstThreadLabel(i);
		VERIFY(bLab);
		if (!bLab->getCreate()) {
			if (bLab->getSymmPredTid() != -1) {
				auto *symmLab = getFirstThreadLabel(bLab->getSymmPredTid());
				symmLab->setSymmSuccTid(-1);
			}
			insertionOrder.remove(*bLab);
			poLists[i].remove(*bLab);
			events.erase(events.begin() + i);
			poLists.erase(poLists.begin() + i);
		}
	}

	/* Fix stamps */
	resetStamp(0U);
	for (auto &lab : labels())
		lab.setStamp(nextStamp());
}

void ExecutionGraph::copyGraphUpTo(ExecutionGraph &other, const VectorClock &v) const
{
	/* We resize up to g.size() (instead of v.size()) because there might be a create
	 * that is contained in v, but its respective begin is not.
	 * Will clean up orphaned begins later. */
	other.events.resize(getNumThreads());
	other.poLists.resize(getNumThreads());
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
			if (auto *wLab = genmc::dyn_cast<WriteLabel>(nLab)) {
				const_cast<WriteLabel *>(wLab)->removeReader([&v](ReadLabel &rLab) {
					return !v.contains(rLab.getPos());
				});
			}
			if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(nLab))
				other.trackCoherenceAtLoc(mLab->getAddr());
			if (auto *tcLab = genmc::dyn_cast<ThreadCreateLabel>(nLab))
				;
		}
	}

	/* Clean up begins */
	for (auto i :
	     std::views::reverse(other.thr_ids()) | std::views::take_while([&other](auto i) {
		     return other.getThreadSize(i) == 1 &&
			    !genmc::isa<InitLabel>(other.getFirstThreadLabel(i));
	     })) {
		auto *bLab = other.getFirstThreadLabel(i);
		VERIFY(bLab);
		/* Have to use containsPos() below due to trailing begins */
		if (!bLab->getCreate() || !other.containsPos(bLab->getCreate()->getPos())) {
			if (bLab->getSymmPredTid() != -1) {
				auto *symmLab = other.getFirstThreadLabel(bLab->getSymmPredTid());
				symmLab->setSymmSuccTid(-1);
			}
			other.insertionOrder.remove(*bLab);
			other.poLists[i].remove(*bLab);
			other.events.erase(other.events.begin() + i);
			other.poLists.erase(other.poLists.begin() + i);
			VERIFY(i >= other.getNumThreads() - 1 || other.getThreadSize(i + 1) <= 0);
		}
	}

	/* Fix insertion order */
	other.insertionOrder.clear();
	other.resetStamp(0);
	for (const auto &lab : insertionOrder) {
		/* Do not use v here because of trailing begins */
		if (other.containsPos(lab.getPos())) {
			other.insertionOrder.push_back(*other.getEventLabel(lab.getPos()));
			other.getEventLabel(lab.getPos())->setStamp(other.nextStamp());
		}
	}

	/* Helper map to repair views in the copied graph */
	std::unordered_map<const detail::ViewBase *, genmc::intrusive_ptr<detail::ViewBase>>
		viewMap;

	/* Deep-copies a view */
	auto fixView = [&](View &view) {
		if (!view.base_)
			return;

		/* Have we already cloned this viewbase? */
		if (auto it = viewMap.find(view.base_.get()); it != viewMap.end()) {
			view.base_ = it->second;
			return;
		}
		/* We need to deep copy */
		auto newBase = genmc::make_intrusive<detail::ViewBase>(*view.base_);
		viewMap[view.base_.get()] = newBase;
		view.base_ = std::move(newBase);
	};

	/* Fix pointers and views */
	for (auto &lab : other.labels()) {
		for (auto &view : lab.calculatedViews)
			fixView(view);
		if (lab.prefixView)
			if (auto *v = genmc::dyn_cast<View>(lab.prefixView.get()))
				fixView(*v);

		auto *rLab = genmc::dyn_cast<ReadLabel>(&lab);
		if (rLab && rLab->getRf()) {
			if (!v.contains(rLab->getRf()->getPos()))
				rLab->setRfNoCascade(nullptr);
			else
				rLab->setRfNoCascade(other.getEventLabel(rLab->getRf()->getPos()));
		}
		if (auto *wLab = genmc::dyn_cast<WriteLabel>(&lab)) {
			wLab->removeReader([](auto &rLab) { return true; });
			for (auto &oLab : getWriteLabel(lab.getPos())->readers())
				if (v.contains(oLab.getPos()))
					wLab->addReader(other.getReadLabel(oLab.getPos()));
		}
		auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(&lab);
		if (tsLab && tsLab->getCreate()) {
			if (!v.contains(tsLab->getCreate()->getPos())) {
				tsLab->setCreate(nullptr);
			} else {
				auto *tcLab = genmc::dyn_cast<ThreadCreateLabel>(
					other.getEventLabel(tsLab->getCreate()->getPos()));
				tsLab->setCreate(tcLab);
			}
		}
		auto *eLab = genmc::dyn_cast<ThreadFinishLabel>(&lab);
		if (eLab && eLab->getParentJoin()) {
			if (!v.contains(eLab->getParentJoin()->getPos())) {
				eLab->setParentJoin(nullptr);
			} else {
				auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(
					other.getEventLabel(eLab->getParentJoin()->getPos()));
				eLab->setParentJoin(jLab);
			}
		}
		if (auto *begLab = genmc::dyn_cast<MethodBeginLabel>(&lab)) {
			begLab->removePredNoCascade([](auto *endLab) { return true; });
			for (auto *endLab :
			     genmc::dyn_cast<MethodBeginLabel>(getEventLabel(lab.getPos()))
				     ->lin_preds())
				if (v.contains(endLab->getPos()))
					begLab->addPredNoCascade(genmc::dyn_cast<MethodEndLabel>(
						other.getEventLabel(endLab->getPos())));
		}
		if (auto *endLab = genmc::dyn_cast<MethodEndLabel>(&lab)) {
			endLab->removeSuccNoCascade([](auto *endLab) { return true; });
			for (auto *begLab :
			     genmc::dyn_cast<MethodEndLabel>(getEventLabel(lab.getPos()))
				     ->lin_succs())
				if (v.contains(begLab->getPos()))
					endLab->addSuccNoCascade(genmc::dyn_cast<MethodBeginLabel>(
						other.getEventLabel(begLab->getPos())));
		}
	}

	/* Finally, copy coherence info */
	for (auto lIt = loc_begin(), lE = loc_end(); lIt != lE; ++lIt) {
		for (auto sIt = lIt->second.begin(); sIt != lIt->second.end(); ++sIt)
			if (v.contains(sIt->getPos())) {
				other.getWriteLabel(sIt->getPos())
					->addCo(other.co_max(sIt->getAddr()));
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
	for (auto it = loc_begin(); it != loc_end(); ++it) {
		auto &accesses = accessMap_.at(it->first);
		for (auto rIt = accesses.begin(); rIt != accesses.end(); ++rIt) {
			if (v.contains((*rIt)->getPos())) {
				other.accessMap_[it->first].push_back(
					other.getEventLabel((*rIt)->getPos()));
			}
		}
	}
	for (const auto &[loc, val] : initVals_) {
		if (other.containsLoc(loc))
			other.initVals_.insert({loc, val});
	}
}

auto ExecutionGraph::getCopyUpTo(const VectorClock &v) const -> std::unique_ptr<ExecutionGraph>
{
	auto og = std::make_unique<ExecutionGraph>(
		ExecutionGraph::Config{.execState = this->state_,
				       .consChecker = this->consChecker_,
				       .emitNALabels = this->haveNAs_});
	copyGraphUpTo(*og, v);
	return og;
}

/************************************************************
 ** Debugging methods
 ***********************************************************/

void ExecutionGraph::validate()
{
	for (auto &lab : labels()) {
		if (auto *rLab = genmc::dyn_cast<ReadLabel>(&lab)) {
			if (!rLab->getRf())
				continue;

			if (!containsLab(rLab->getRf())) {
				std::cerr << std::format("Non-existent RF: {}\n{}\n",
							 rLab->getPos(), *this);
				UNREACHABLE();
			}

			if (auto *rfLab = genmc::dyn_cast<WriteLabel>(rLab->getRf())) {
				if (std::ranges::all_of(rfLab->readers(), [rLab](auto &oLab) {
					    return &oLab != rLab;
				    })) {
					std::cerr
						<< std::format("Not in RF's readers list: {}\n{}\n",
							       rLab->getPos(), *this);
					UNREACHABLE();
				}
			}
		}
		if (auto *wLab = genmc::dyn_cast<WriteLabel>(&lab)) {
			if (wLab->isRMW() &&
			    std::ranges::count_if(wLab->readers(),
						  [&](auto &rLab) { return rLab.isRMW(); }) > 1) {
				std::cerr << std::format("Atomicity violation: {}\n{}\n",
							 wLab->getPos(), *this);
				UNREACHABLE();
			}

			if (std::ranges::any_of(wLab->readers(), [&](auto &rLab) {
				    return !containsPosNonEmpty(rLab.getPos());
			    })) {
				std::string readers = "";
				for (auto &rLab : wLab->readers())
					readers += std::format("{} ", rLab.getPos());
				std::cerr << std::format("Non-existent/non-read reader: "
							 "{}\nReaders: {}\n{}\n",
							 wLab->getPos(), readers, *this);
				UNREACHABLE();
			}

			if (std::ranges::any_of(wLab->readers(),
						[&](auto &rLab) { return rLab.getRf() != wLab; })) {
				std::cerr << std::format("RF not properly set: {}\n{}\n",
							 wLab->getPos(), *this);
				UNREACHABLE();
			}
			for (auto &rLab : wLab->readers()) {
				if (!containsPosNonEmpty(rLab.getPos())) {
					std::cerr << std::format(
						"Readers list has garbage: {}\n{}\n", rLab, *this);
					UNREACHABLE();
				}
			}
		}
	}
}
