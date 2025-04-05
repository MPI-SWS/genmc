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

#include "ADT/View.hpp"
#include "Support/Error.hpp"

auto View::size() const -> unsigned int { return view_.size(); }

auto View::empty() const -> bool { return size() == 0; }

auto View::contains(const Event e) const -> bool { return e.index <= getMax(e.thread); }

auto View::update(const View &v) -> View &
{
	if (v.empty())
		return *this;

	auto size = std::max(this->size(), v.size());
	for (auto i = 0U; i < size; i++)
		if (getMax(i) < v.getMax(i))
			setMax(Event(i, v.getMax(i)));
	return *this;
}

auto View::update(const DepView &v) -> DepView & { BUG(); }

auto View::update(const VectorClock &vc) -> VectorClock &
{
	if (const auto *v = llvm::dyn_cast<View>(&vc))
		return this->update(*v);
	BUG();
}

void View::printData(llvm::raw_ostream &s) const
{
	s << "[ ";
	for (auto i = 0U; i < size(); i++)
		s << i << ":" << getMax(i) << " ";
	s << "]";
}
