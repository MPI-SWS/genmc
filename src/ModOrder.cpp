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

#include "Error.hpp"
#include "ModOrder.hpp"
#include <algorithm>


/************************************************************
 ** Class Constructors
 ***********************************************************/

ModOrder::ModOrder() {}


/************************************************************
 ** Iterators
 ***********************************************************/

ModOrder::iterator ModOrder::begin() { return mo_.begin(); }
ModOrder::iterator ModOrder::end()   { return mo_.end(); }

ModOrder::const_iterator ModOrder::cbegin() { return mo_.cbegin(); }
ModOrder::const_iterator ModOrder::cend()   { return mo_.cend(); }


/************************************************************
 ** Basic getter and setter methods
 ***********************************************************/

const llvm::GenericValue *ModOrder::getAddrAtPos(ModOrder::iterator it)
{
	return it->first;
}

std::vector<Event> ModOrder::getMoAfter(const llvm::GenericValue *addr, Event e)
{
	std::vector<Event> res;

	/* All stores are mo-after INIT */
	if (e.isInitializer())
		return mo_[addr];

	for (auto rit = mo_[addr].rbegin(); rit != mo_[addr].rend(); ++rit) {
		if (*rit == e) {
			std::reverse(res.begin(), res.end());
			return res;
		}
		res.push_back(*rit);
	}
	BUG();
}

void ModOrder::addAtLocEnd(const llvm::GenericValue *addr, Event e)
{
       mo_[addr].push_back(e);
}

void ModOrder::addAtLocAfter(const llvm::GenericValue *addr, Event pred, Event e)
{
	/* If there is no predecessor, put the store in the beginning */
	if (pred == Event::getInitializer()) {
		mo_[addr].insert(mo_[addr].begin(), e);
		return;
	}

	/* Otherwise, place it in the appropriate place */
	for (auto it = mo_[addr].begin(); it != mo_[addr].end(); ++it) {
		if (*it == pred) {
			mo_[addr].insert(it + 1, e);
			return;
		}
	}
	/* Pred has to be either INIT or in this location's MO */
	BUG();
}

bool ModOrder::locContains(const llvm::GenericValue *addr, Event e)
{
	return e == Event::getInitializer() ||
	      std::any_of(mo_[addr].begin(), mo_[addr].end(), [&e](Event s){ return s == e; });
}

bool ModOrder::areOrdered(const llvm::GenericValue *addr, Event a, Event b)
{
	if (b == Event::getInitializer())
		return true;

	auto foundB = false;
	auto foundA = false;
	for (auto it = mo_[addr].begin(); it != mo_[addr].end(); ++it) {
		foundA |= (*it == a);
		foundB |= (*it == b);
		if (foundA && !foundB)
			return false;
		if (foundA && foundB)
			return true;
	}
	return false;
}

int ModOrder::getStoreOffset(const llvm::GenericValue *addr, Event e)
{
	if (e == Event::getInitializer())
		return -1;

	for (auto it = mo_[addr].begin(); it != mo_[addr].end(); ++it) {
		if (*it == e)
			return std::distance(mo_[addr].begin(), it);
	}
	BUG();
}

void ModOrder::changeStoreOffset(const llvm::GenericValue *addr, Event e, int newOffset)
{
	auto &locMO = mo_[addr];

	locMO.erase(std::find(locMO.begin(), locMO.end(), e));
	locMO.insert(locMO.begin() + newOffset, e);
}


/************************************************************
 ** Overloaded operators
 ***********************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ModOrder &mo)
{
	s << "Modification Order:\n";
	for (auto &kvp : mo.mo_) {
		s << "\t" << kvp.first << ": [ ";
		for (auto &e : kvp.second)
			s << e << " ";
		s << "]\n";
	}
	return s;
}
