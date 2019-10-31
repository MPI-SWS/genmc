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
#include "View.hpp"
#include <algorithm>

/************************************************************
 ** Constructors
 ***********************************************************/

View::View() : view_(EventView(0)) { }


/************************************************************
 ** Iterators
 ***********************************************************/

View::iterator View::begin() { return &((*this)[0]); }
View::iterator View::end()   { return &((*this)[0]) + size(); }

View::const_iterator View::cbegin() { return &((*this)[0]); }
View::const_iterator View::cend()   { return &((*this)[0]) + size(); }


/************************************************************
 ** Basic operations on Views
 ***********************************************************/

unsigned int View::size() const
{
	return view_.size();
}

bool View::empty() const
{
	return this->size() == 0;
}

bool View::contains(Event e) const
{
	return e.index <= (*this)[e.thread];
}

View& View::updateIdx(const Event e)
{
	if ((*this)[e.thread] < e.index)
		(*this)[e.thread] = e.index;
	return *this;
}

View& View::update(const View &v)
{
	if (v.empty())
		return *this;

	auto size = std::max(this->size(), v.size());
	for (auto i = 0u; i < size; i++)
		if ((*this)[i] < v[i])
			(*this)[i] = v[i];
	return *this;
}

DepView& View::update(const DepView &v)
{
	BUG();
}

VectorClock &View::update(const VectorClock &vc)
{
	if (auto *v = llvm::dyn_cast<View>(&vc))
		this->update(*v);
	BUG();
}

void View::printData(llvm::raw_ostream &s) const
{
	s << "[ ";
	for (auto i = 0u; i < size(); i++)
		s << i << ":" << (*this)[i] << " ";
	s << "]";
}
