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

std::vector<Event> DepExecutionGraph::getRevisitable(const WriteLabel *sLab) const
{
	std::vector<Event> loads;

	for (auto i = 0u; i < getNumThreads(); i++) {
		if (sLab->getThread() == i)
			continue;
		for (auto j = 1u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getAddr() == sLab->getAddr() &&
				    !sLab->getPPoRfView().contains(rLab->getPos()) &&
				    rLab->isRevisitable())
					loads.push_back(rLab->getPos());
			}
		}
	}
	return loads;
}

std::unique_ptr<VectorClock>
DepExecutionGraph::getRevisitView(const ReadLabel *rLab,
				  const WriteLabel *wLab) const
{
	auto preds = llvm::make_unique<DepView>(getDepViewFromStamp(rLab->getStamp()));
	preds->update(wLab->getPPoRfView());
	return std::move(preds);
}

const VectorClock& DepExecutionGraph::getPrefixView(Event e) const
{
	return getPPoRfBefore(e);
}

std::unique_ptr<VectorClock> DepExecutionGraph::getPredsView(Event e) const
{
	auto stamp = getEventLabel(e)->getStamp();
	return llvm::make_unique<DepView>(getDepViewFromStamp(stamp));
}

bool DepExecutionGraph::revisitModifiesGraph(const ReadLabel *rLab,
					     const EventLabel *sLab) const
{
	auto v = getDepViewFromStamp(rLab->getStamp());
	auto &pfx = getPPoRfBefore(sLab->getPos());

	v.update(pfx);
	for (auto i = 0u; i < getNumThreads(); i++) {
		if (v[i] + 1 != (int) getThreadSize(i))
			return true;
		for (auto j = 0u; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!v.contains(lab->getPos()) &&
			    !llvm::isa<EmptyLabel>(lab))
				return true;
		}
	}
	return false;
}

std::vector<std::unique_ptr<EventLabel> >
DepExecutionGraph::getPrefixLabelsNotBefore(const EventLabel *sLab,
					    const ReadLabel *rLab) const
{
	std::vector<std::unique_ptr<EventLabel> > result;

	auto &pporf = sLab->getPPoRfView();
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 1; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (lab->getStamp() <= rLab->getStamp() ||
			    !pporf.contains(lab->getPos()))
				continue;

			result.push_back(std::unique_ptr<EventLabel>(lab->clone()));

			auto &curLab = result.back();
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(curLab.get())) {
				wLab->removeReader([&](Event r) {
						return getEventLabel(r)->getStamp() > rLab->getStamp() &&
						       !pporf.contains(r);
					});
			} else if (auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(curLab.get())) {
				if (getEventLabel(eLab->getParentJoin())->getStamp() > rLab->getStamp() &&
				    !pporf.contains(eLab->getPos()))
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

void DepExecutionGraph::cutToStamp(unsigned int stamp)
{
	/* First remove events from the modification order */
	auto preds = getDepViewFromStamp(stamp);
	getCoherenceCalculator()->removeStoresAfter(preds);

	/* Then, restrict the graph */
	for (auto i = 0u; i < getNumThreads(); i++) {
		/* We reset the maximum index for this thread, as we
		 * do not know the order in which events were added */
		auto newMax = 1u;
		for (auto j = 1u; j < getThreadSize(i); j++) { /* Keeps begins */
			const EventLabel *lab = getEventLabel(Event(i, j));
			/* If this label should not be deleted, check if it
			 * is gonna be the new maximum, and then skip */
			if (lab->getStamp() <= stamp) {
				if (lab->getIndex() >= newMax)
					newMax = lab->getIndex() + 1;
				continue;
			}

			/* Otherwise, remove 'pointers' to it, in an
			 * analogous manner cutToView(). */
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				Event rf = rLab->getRf();
				/* Make sure RF label exists */
				EventLabel *rfLab = getEventLabel(rf);
				if (rfLab) {
					if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab)) {
						wLab->removeReader([&](Event r){
								return r == rLab->getPos();
							});
					}
				}
			} else if (auto *fLab = llvm::dyn_cast<ThreadFinishLabel>(lab)) {
				Event pj = fLab->getParentJoin();
				if (getEventLabel(pj)) /* Make sure the parent exists */
					resetJoin(pj);
			}
			setEventLabel(Event(i, j), nullptr);
		}
		resizeThread(i, newMax);
	}

	/* Finally, do not keep any nullptrs in the graph */
	for (auto i = 0u; i < getNumThreads(); i++) {
		for (auto j = 0u; j < getThreadSize(i); j++) {
			if (getEventLabel(Event(i, j)))
				continue;
			setEventLabel(Event(i, j), std::unique_ptr<EmptyLabel>(
				      new EmptyLabel(nextStamp(), Event(i, j))));
		}
	}
}
