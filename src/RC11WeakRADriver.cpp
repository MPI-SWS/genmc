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

#include "RC11WeakRADriver.hpp"

/************************************************************
 ** WEAK RA DRIVER -- UNIMPLEMENTED
 ***********************************************************/

std::vector<Event> RC11WeakRADriver::getStoresToLoc(llvm::GenericValue *addr)
{
	auto &g = getGraph();
	auto overwritten = g.findOverwrittenBoundary(addr, EE->getCurThr().id);
	std::vector<Event> stores;

	if (overwritten.empty()) {
		auto &locMO = g.modOrder[addr];
		stores.push_back(Event::getInitializer());
		stores.insert(stores.end(), locMO.begin(), locMO.end());
		return stores;
	}

	auto before = g.getHbRfBefore(overwritten);
	for (auto i = 0u; i < g.events.size(); i++) {
		for (auto j = before[i] + 1u; j < g.events[i].size(); j++) {
			EventLabel &lab = g.events[i][j];
			if (lab.isWrite() && lab.getAddr() == addr)
				stores.push_back(lab.getPos());
		}
	}
	return stores;
}

std::pair<int, int> RC11WeakRADriver::getPossibleMOPlaces(llvm::GenericValue *addr, bool isRMW)
{
	BUG();
}

std::vector<Event> RC11WeakRADriver::getRevisitLoads(EventLabel &lab)
{
	BUG();
}

std::pair<std::vector<EventLabel>, std::vector<std::pair<Event, Event> > >
	  RC11WeakRADriver::getPrefixToSaveNotBefore(EventLabel &lab, View &before)
{
	BUG();
}

bool RC11WeakRADriver::checkPscAcyclicity()
{
	WARN("Unimplemented!\n");
	abort();
}

bool RC11WeakRADriver::isExecutionValid()
{
	WARN("Unimplemented!\n");
	abort();
}
