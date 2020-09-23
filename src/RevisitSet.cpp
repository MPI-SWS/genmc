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
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#include "RevisitSet.hpp"
#include <algorithm>

/************************************************************
 ** Iterators
 ***********************************************************/

RevisitSet::iterator RevisitSet::begin() { return rev_.begin(); }
RevisitSet::iterator RevisitSet::end()   { return rev_.end(); }

RevisitSet::const_iterator RevisitSet::cbegin() { return rev_.cbegin(); }
RevisitSet::const_iterator RevisitSet::cend()   { return rev_.cend(); }


/************************************************************
 ** Basic getter/setter methods and existential checks
 ***********************************************************/

void RevisitSet::add(const std::vector<Event> &es, const std::vector<std::pair<Event, Event> > &mos)
{
	auto hash = size_t(llvm::hash_combine_range(es.begin(), es.end()));
	rev_[hash].emplace_back(RevisitItem(es, mos));
}

bool RevisitSet::contains(const std::vector<Event> &es,
			  const std::vector<std::pair<Event, Event> > &mos) const
{
	auto hash = size_t(llvm::hash_combine_range(es.begin(), es.end()));
	if (rev_.count(hash) == 0)
		return false;
	for (auto &p : rev_.at(hash)) {
		if (p.prefix == es && p.mos == mos)
			return true;
	}
	return false;
}

/************************************************************
 ** Overloaded Operators
 ***********************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const RevisitSet &rev)
{
	s << "Revisit Set:\n";
	for (auto &kv : rev.rev_) {
		for (auto &ri : kv.second)
			for (auto &e : ri.prefix)
				s << e << " ";
		s << "\n";
	}
	return s;
}
