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

#include "RC11WBDriver.hpp"

/************************************************************
 ** WB DRIVER
 ***********************************************************/

/*
 * Checks which of the stores are (rf?;hb)-before some event e, given the
 * hb-before view of e
 */
View RC11WBDriver::getRfOptHbBeforeStores(const std::vector<Event> &stores,
					  const View &hbBefore)
{
	auto &g = getGraph();
	View result;

	for (const auto &w : stores) {
		/* Check if w itself is in the hb view */
		if (hbBefore.contains(w)) {
			result.updateIdx(w);
			continue;
		}

		const EventLabel *lab = g.getEventLabel(w);
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);

		/* Check whether [w];rf;[r] is in the hb view, for some r */
		for (const auto &r : wLab->getReadersList()) {
			if (r.thread != w.thread && hbBefore.contains(r)) {
				result.updateIdx(w);
				result.updateIdx(r);
				break;
			}
		}
	}
	return result;
}

void RC11WBDriver::expandMaximalAndMarkOverwritten(const std::vector<Event> &stores,
						   View &storeView)
{
	auto &g = getGraph();

	/* Expand view for maximal stores */
	for (const auto &w : stores) {
		/* If the store is not maximal, skip */
		if (w.index != storeView[w.thread])
			continue;

		const EventLabel *lab = g.getEventLabel(w);
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);

		for (const auto &r : wLab->getReadersList()) {
			if (r.thread != w.thread)
				storeView.updateIdx(r);
		}
	}

	/* Check if maximal writes have been overwritten */
	for (const auto &w : stores) {
		/* If the store is not maximal, skip*/
		if (w.index != storeView[w.thread])
			continue;

		const EventLabel *lab = g.getEventLabel(w);
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);

		for (const auto &r : wLab->getReadersList()) {
			if (r.thread != w.thread && r.index < storeView[r.thread]) {
				const EventLabel *lab = g.getEventLabel(Event(r.thread, storeView[r.thread]));
				if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
					if (rLab->getRf() != w) {
						storeView[w.thread]++;
						break;
					}
				}
			}
		}
	}
	return;
}

std::vector<Event> RC11WBDriver::getStoresToLoc(const llvm::GenericValue *addr)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();
	auto &allStores = g.modOrder[addr];
	auto &hbBefore = g.getHbBefore(g.getLastThreadEvent(thr.id));
	std::vector<Event> result;

	auto view = getRfOptHbBeforeStores(allStores, hbBefore);

	/* Can we read from the initializer event? */
	if (std::none_of(view.begin(), view.end(), [](int i){ return i > 0; }))
		result.push_back(Event::getInitializer());


	expandMaximalAndMarkOverwritten(allStores, view);

	int count = 0;
	for (const auto &w : allStores) {
		if (w.index >= view[w.thread]) {
			if (count++ > 0) {
				result.pop_back();
				break;
			}
			result.push_back(w);
		}
	}
	if (count <= 1)
		return result;

	auto wb = g.calcWb(addr);
	auto &stores = wb.getElems();

	/* Find the stores from which we can read-from */
	for (auto i = 0u; i < stores.size(); i++) {
		auto allowed = true;
		for (auto j = 0u; j < stores.size(); j++) {
			if (wb(i, j) && g.isWriteRfBefore(hbBefore, stores[j])) {
				allowed = false;
				break;
			}
		}
		if (allowed)
			result.push_back(stores[i]);
	}

	return result;
}

std::pair<int, int> RC11WBDriver::getPossibleMOPlaces(const llvm::GenericValue *addr, bool isRMW)
{
	auto locMOSize = (int) getGraph().modOrder[addr].size();
	return std::make_pair(locMOSize, locMOSize);
}

std::vector<Event> RC11WBDriver::getRevisitLoads(const WriteLabel *sLab)
{
	auto &g = getGraph();
	auto ls = g.getRevisitable(sLab);

	/* Optimization:
	 * Since sLab is a porf-maximal store, unless it is an RMW, it is
	 * wb-maximal (and so, all revisitable loads can read from it).
	 */
	if (!llvm::isa<FaiWriteLabel>(sLab) && !llvm::isa<CasWriteLabel>(sLab))
		return ls;

	/* Optimization:
	 * If sLab is maximal in WB, then all revisitable loads can read
	 * from it.
	 */
	if (ls.size() > 1) {
		auto wb = g.calcWb(sLab->getAddr());
		auto i = wb.getIndex(sLab->getPos());
		bool allowed = true;
		for (auto j = 0u; j < wb.size(); j++)
			if (wb(i,j)) {
				allowed = false;
				break;
			}
		if (allowed)
			return ls;
	}

	std::vector<Event> result;

	/*
	 * We calculate WB again, in order to filter-out inconsistent
	 * revisit options. For example, if sLab is an RMW, we cannot
	 * revisit a read r for which:
	 * \exists c_a in C_a .
	 *         (c_a, r) \in (hb;[\lW_x];\lRF^?;hb;po)
	 *
	 * since this will create a cycle in WB
	 */

	for (auto &l : ls) {
		auto v = g.getViewFromStamp(g.getEventLabel(l)->getStamp());
		v.update(g.getPorfBefore(sLab->getPos()));

		auto wb = g.calcWbRestricted(sLab->getAddr(), v);
		auto &stores = wb.getElems();
		auto i = wb.getIndex(sLab->getPos());

		auto &hbBefore = g.getHbBefore(l.prev());
		bool allowed = true;
		for (auto j = 0u; j < stores.size(); j++) {
			if (wb(i, j) && g.isWriteRfBefore(hbBefore, stores[j])) {
				allowed = false;
				break;
			}
		}
		if (allowed)
			result.push_back(l);
	}
	return result;
}

std::pair<std::vector<std::unique_ptr<EventLabel> >,
	  std::vector<std::pair<Event, Event> > >
RC11WBDriver::getPrefixToSaveNotBefore(const WriteLabel *lab, View &before)
{
	auto &g = getGraph();
	auto writePrefix = g.getPrefixLabelsNotBefore(g.getPorfBefore(lab->getPos()), before);
	return std::make_pair(std::move(writePrefix), std::vector<std::pair<Event, Event>>());
}

bool RC11WBDriver::checkPscAcyclicity()
{
	switch (getConf()->checkPscAcyclicity) {
	case CheckPSCType::nocheck:
		return true;
	case CheckPSCType::weak:
		return getGraph().isPscWeakAcyclicWB();
	case CheckPSCType::wb:
		return getGraph().isPscWbAcyclicWB();
	case CheckPSCType::full:
		return getGraph().isPscAcyclicWB();
	default:
		WARN("Unimplemented model!\n");
		BUG();
	}
}

bool RC11WBDriver::isExecutionValid()
{
	return getGraph().isPscAcyclicWB();
}
