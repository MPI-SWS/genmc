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

#include "DepExecutionGraph.hpp"

std::unique_ptr<VectorClock> DepExecutionGraph::getViewFromStamp(Stamp stamp) const
{
	auto preds = std::make_unique<DepView>();

	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 1u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (lab->getStamp() <= stamp)
				preds->setMax(Event(i, j));
		}
	}
	return preds;
}

void DepExecutionGraph::cutToStamp(Stamp stamp)
{
	/* First remove events from the modification order */
	auto preds = getViewFromStamp(stamp);

	/* Inform all calculators about the events cutted */
	removeAfter(*preds);
	for (auto labIt = insertionOrder.begin(); labIt != insertionOrder.end();) {
		if (labIt->getIndex() > preds->getMax(labIt->getThread()))
			poLists[labIt->getThread()].remove(*labIt);
		labIt = preds->contains(labIt->getPos()) ? ++labIt : insertionOrder.erase(labIt);
	}

	/* Then, restrict the graph */
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j <= preds->getMax(i); j++) { /* Keeps begins */
			auto *lab = getEventLabel(Event(i, j));
			if (!preds->contains(lab->getPos()))
				continue;

			/* Otherwise, remove 'pointers' to it, in an
			 * analogous manner cutToView(). */
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

	/* Restrict the graph according to the view (keep begins around) */
	for (auto i = 0u; i < getNumThreads(); i++) {
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
			VERIFY(i >= getNumThreads() - 1 || getThreadSize(i + 1) <= 0);
		}
	}

	/* Fix stamps */
	resetStamp(0U);
	for (auto &lab : labels())
		lab.setStamp(nextStamp());

	/* Finally, do not keep any nullptrs in the graph */
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			if (preds->contains(Event(i, j)))
				continue;
			auto it = po_iterator(getEventLabel(Event(i, j)));
			it = poLists[i].erase(it);
			events[i][j] = createHoleLabel(Event(i, j));
			getEventLabel(Event(i, j))->setStamp(nextStamp());
			getEventLabel(Event(i, j))->setParent(this);
			insertionOrder.push_back(*getEventLabel(Event(i, j)));
			poLists[i].insert(it, *getEventLabel(Event(i, j)));
		}
	}

	getState().clear();
}

std::unique_ptr<ExecutionGraph> DepExecutionGraph::getCopyUpTo(const VectorClock &v) const
{
	auto og = std::make_unique<DepExecutionGraph>(ExecutionGraph::Config{
		.consChecker = this->consChecker_, .emitNALabels = this->haveNAs_});
	copyGraphUpTo(*og, v);
	return og;
}
