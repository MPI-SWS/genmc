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

#include "View.hpp"
#include <algorithm>

View::View() : view_(EventView(0)) { }

unsigned int View::size() const
{
	return view_.size();
}

bool View::empty() const
{
	return this->size() == 0;
}

void View::updateMax(View &v)
{
	if (v.empty())
		return;

	auto size = std::max(this->size(), v.size());
	for (auto i = 0u; i < size; i++)
		if ((*this)[i] < v[i])
			(*this)[i] = v[i];
	return;
}

View View::getMax(View &v)
{
	if (this->empty())
		return v;
	if (v.empty())
		return *this;

	View result(*this);

	auto size = std::max(this->size(), v.size());
	for (auto i = 0u; i < size; i++)
		if (result[i] < v[i])
			result[i] = v[i];
	return result;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const View &v)
{
	s << "[ ";
	for (auto i = 0u; i < v.size(); i++)
		s << i << ":" << v[i] << " ";
	s << "]";
	return s;
}
