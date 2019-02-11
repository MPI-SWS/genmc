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

std::vector<Event> RC11WBDriver::getStoresToLoc(llvm::GenericValue *addr)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();
	auto wb = g.calcWb(addr);
	auto &stores = wb.first;
	auto &wbMatrix = wb.second;
	auto hbBefore = g.getHbBefore(g.getLastThreadEvent(thr.id));
	auto porfBefore = g.getPorfBefore(g.getLastThreadEvent(thr.id));

	// Find the stores from which we can read-from
	std::vector<Event> result;
	for (auto i = 0u; i < stores.size(); i++) {
		bool allowed = true;
		for (auto j = 0u; j < stores.size(); j++) {
			if (wbMatrix[i * stores.size() + j] &&
			    g.isWriteRfBefore(hbBefore, stores[j]))
				allowed = false;
		}
		if (allowed)
			result.push_back(stores[i]);
	}

	// Also check the initializer event
	bool allowed = true;
	for (auto j = 0u; j < stores.size(); j++)
		if (g.isWriteRfBefore(hbBefore, stores[j]))
			allowed = false;
	if (allowed)
		result.insert(result.begin(), Event::getInitializer());
	return result;
}

std::pair<int, int> RC11WBDriver::getPossibleMOPlaces(llvm::GenericValue *addr, bool isRMW)
{
	auto locMOSize = (int) getGraph().modOrder[addr].size();
	return std::make_pair(locMOSize, locMOSize);
}

std::vector<Event> RC11WBDriver::getRevisitLoads(EventLabel &sLab)
{
	auto &g = getGraph();
	auto ls = g.getRevisitable(sLab);
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
		auto v = g.getViewFromStamp(g.getEventLabel(l).getStamp());
		v.updateMax(sLab.porfView);

		auto wb = g.calcWbRestricted(sLab.addr, v);
		auto &stores = wb.first;
		auto &wbMatrix = wb.second;
		auto i = std::find(stores.begin(), stores.end(), sLab.pos) - stores.begin();

		auto hbBefore = g.getHbBefore(l.prev());
		bool allowed = true;
		for (auto j = 0u; j < stores.size(); j++) {
			if (wbMatrix[i * stores.size() + j] &&
			    g.isWriteRfBefore(hbBefore, stores[j]))
				allowed = false;
		}
		if (allowed)
			result.push_back(l);
	}
	return result;
}

std::pair<std::vector<EventLabel>, std::vector<std::pair<Event, Event> > >
	  RC11WBDriver::getPrefixToSaveNotBefore(EventLabel &lab, View &before)
{
	auto writePrefix = getGraph().getPrefixLabelsNotBefore(lab.porfView, before);
	return std::make_pair(std::move(writePrefix), std::vector<std::pair<Event, Event>>());
}

bool RC11WBDriver::checkPscAcyclicity()
{
	switch (userConf->checkPscAcyclicity) {
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
