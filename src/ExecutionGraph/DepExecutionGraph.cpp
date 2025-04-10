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

#include "DepExecutionGraph.hpp"
#include "config.h"

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
			if (auto *tsLab = llvm::dyn_cast<ThreadStartLabel>(lab)) {
				if (tsLab->getCreate() &&
				    !preds->contains(tsLab->getCreate()->getPos()))
					tsLab->setCreate(nullptr);
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
			if (auto *begLab = llvm::dyn_cast<MethodBeginLabel>(lab)) {
				begLab->removePredNoCascade([&](auto *endLab) {
					return !preds->contains(endLab->getPos());
				});
			}
			if (auto *endLab = llvm::dyn_cast<MethodEndLabel>(lab)) {
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
			setEventLabel(Event(i, j), createHoleLabel(Event(i, j)));
			getEventLabel(Event(i, j))->setStamp(nextStamp());
			getEventLabel(Event(i, j))->setParent(this);
			insertionOrder.push_back(*getEventLabel(Event(i, j)));
			poLists[i].insert(it, *getEventLabel(Event(i, j)));
		}
	}
}

std::unique_ptr<ExecutionGraph> DepExecutionGraph::getCopyUpTo(const VectorClock &v) const
{
	auto og = std::unique_ptr<DepExecutionGraph>(new DepExecutionGraph(this->initValGetter_));
	copyGraphUpTo(*og, v);
	return og;
}
