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
 ** Class Constructors
 ***********************************************************/

RevisitSet::RevisitSet() : rev_({}) {}


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

void RevisitSet::add(std::vector<Event> &es)
{
	rev_.push_back(std::make_pair(es, std::vector<std::pair<Event, Event> >()));
}

void RevisitSet::add(std::vector<Event> &es, std::vector<std::pair<Event, Event> > &mos)
{
	rev_.push_back(std::make_pair(es, mos));
}

bool RevisitSet::contains(std::vector<Event> &es)
{
	for (auto &p : rev_)
		if (p.first == es)
			return true;
	return false;
}

bool RevisitSet::contains(const std::vector<Event> &es,
			  const std::vector<std::pair<Event, Event> > &mos)
{
	for (auto &p : rev_)
		if (p.first == es && p.second == mos)
			return true;
	return false;
}

/************************************************************
 ** Overloaded Operators
 ***********************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const RevisitSet &rev)
{
	s << "Revisit Set:\n";
	for (auto &r : rev.rev_) {
		for (auto &e : r.first)
			s << e << " ";
		s << "\n";
	}
	return s;
}
