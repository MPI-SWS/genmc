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

int RC11MODriver::splitLocMOBefore(const llvm::GenericValue *addr, const View &before)
{
	auto &g = getGraph();
	auto &locMO = g.modOrder[addr];

	for (auto rit = locMO.rbegin(); rit != locMO.rend(); ++rit) {
		if (before.empty() || !g.isWriteRfBefore(before, *rit))
			continue;
		return std::distance(rit, locMO.rend());
	}
	return 0;
}

std::vector<Event> RC11MODriver::getStoresToLoc(const llvm::GenericValue *addr)
{
	std::vector<Event> stores;

	auto &g = getGraph();
	auto &locMO = g.modOrder[addr];
	auto &before = g.getHbBefore(g.getLastThreadEvent(getEE()->getCurThr().id));

	auto begO = splitLocMOBefore(addr, before);

	/*
	 * If there are no stores (hb;rf?)-before the current event
	 * then we can read read from all concurrent stores and the
	 * initializer store. Otherwise, we can read from all concurrent
	 * stores and the mo-latest of the (hb;rf?)-before stores.
	 */
	if (begO == 0)
		stores.push_back(Event::getInitializer());
	else
		stores.push_back(*(locMO.begin() + begO - 1));
	stores.insert(stores.end(), locMO.begin() + begO, locMO.end());
	return stores;
}

std::pair<int, int> RC11MODriver::getPossibleMOPlaces(const llvm::GenericValue *addr, bool isRMW)
{
	auto &g = getGraph();
	const EventLabel *pLab = g.getLastThreadLabel(getEE()->getCurThr().id);

	if (isRMW) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(pLab)) {
			auto offset = g.modOrder.getStoreOffset(addr, rLab->getRf()) + 1;
			return std::make_pair(offset, offset);
		}
		BUG();
	}

	auto &before = g.getHbBefore(pLab->getPos());
	return std::make_pair(splitLocMOBefore(addr, before), g.modOrder[addr].size());
}

std::vector<Event> RC11MODriver::getRevisitLoads(const WriteLabel *sLab)
{
	auto &g = getGraph();
	auto ls = g.getRevisitable(sLab);
	auto &locMO = g.modOrder[sLab->getAddr()];

	/* If this store is mo-maximal then we are done */
	if (locMO.back() == sLab->getPos())
		return ls;

	/* Otherwise, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = g.getMoOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
				{ const View &before = g.getHbPoBefore(e);
				  return std::any_of(optRfs.begin(), optRfs.end(),
					 [&](Event ev)
					 { return before.contains(ev); });
				}), ls.end());
	return ls;
}

std::pair<std::vector<std::unique_ptr<EventLabel> >,
	  std::vector<std::pair<Event, Event> > >
RC11MODriver::getPrefixToSaveNotBefore(const WriteLabel *lab, View &before)
{
	auto &g = getGraph();
	auto writePrefix = g.getPrefixLabelsNotBefore(g.getPorfBefore(lab->getPos()), before);
	auto moPlacings = g.getMOPredsInBefore(writePrefix, before);
	return std::make_pair(std::move(writePrefix), std::move(moPlacings));
}

bool RC11MODriver::checkPscAcyclicity()
{
	switch (getConf()->checkPscAcyclicity) {
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
