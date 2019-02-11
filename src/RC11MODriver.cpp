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

#include "RC11MODriver.hpp"

/************************************************************
 ** MO DRIVER
 ***********************************************************/

std::vector<Event> RC11MODriver::getStoresToLoc(llvm::GenericValue *addr)
{
	std::vector<Event> stores;

	auto &g = getGraph();
	auto &locMO = g.modOrder[addr];
	auto before = g.getHbBefore(g.getLastThreadEvent(EE->getCurThr().id));

	auto splitRange = g.splitLocMOBefore(addr, before);
	auto &begO = splitRange.first;
	auto &endO = splitRange.second;

	/*
	 * If there are not stores (hb;rf?)-before the current event
	 * then we can read read from all concurrent stores and the
	 * initializer store. Otherwise, we can read from all concurrent
	 * stores and the mo-latest of the (hb;rf?)-before stores.
	 */
	if (begO == 0)
		stores.push_back(Event::getInitializer());
	else
		stores.push_back(*(locMO.begin() + begO - 1));
	stores.insert(stores.end(), locMO.begin() + begO, locMO.begin() + endO);
	return stores;
}

std::pair<int, int> RC11MODriver::getPossibleMOPlaces(llvm::GenericValue *addr, bool isRMW)
{
	auto &g = getGraph();
	auto &pLab = g.getLastThreadLabel(EE->getCurThr().id);

	if (isRMW) {
		auto offset = g.modOrder.getStoreOffset(addr, pLab.rf) + 1;
		return std::make_pair(offset, offset);
	}

	auto before = g.getHbBefore(pLab.getPos());
	return g.splitLocMOBefore(addr, before);
}

std::vector<Event> RC11MODriver::getRevisitLoads(EventLabel &sLab)
{
	auto &g = getGraph();
	auto ls = g.getRevisitable(sLab);
	auto &locMO = g.modOrder[sLab.getAddr()];

	/* If this store is mo-maximal then we are done */
	if (locMO.back() == sLab.getPos())
		return ls;

	/* Otherwise, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = g.getMoOptRfAfter(sLab.getPos());
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
				{ View before = g.getHbPoBefore(e);
				  return std::any_of(optRfs.begin(), optRfs.end(),
					 [&](Event ev)
					 { return ev.index <= before[ev.thread]; });
				}), ls.end());
	return ls;
}

std::pair<std::vector<EventLabel>, std::vector<std::pair<Event, Event> > >
	  RC11MODriver::getPrefixToSaveNotBefore(EventLabel &lab, View &before)
{
	auto writePrefix = getGraph().getPrefixLabelsNotBefore(lab.porfView, before);
	auto moPlacings = getGraph().getMOPredsInBefore(writePrefix, before);
	return std::make_pair(std::move(writePrefix), std::move(moPlacings));
}

bool RC11MODriver::checkPscAcyclicity()
{
	switch (userConf->checkPscAcyclicity) {
	case CheckPSCType::nocheck:
		return true;
	case CheckPSCType::weak:
	case CheckPSCType::wb:
		WARN_ONCE("check-mo-psc", "WARNING: The full PSC condition is going "
			  "to be checked for the MO-tracking exploration...\n");
	case CheckPSCType::full:
		return getGraph().isPscAcyclicMO();
	default:
		WARN("Unimplemented model!\n");
		BUG();
	}
}

bool RC11MODriver::isExecutionValid()
{
	return getGraph().isPscAcyclicMO();
}
