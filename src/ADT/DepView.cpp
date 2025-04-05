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

#include "ADT/DepView.hpp"
#include "Support/Error.hpp"

auto DepView::contains(const Event e) const -> bool
{
	return e.index <= getMax(e.thread) &&
	       (e.index == 0 || (e.thread < holes_.size() && !holes_[e.thread].count(e.index)));
}

void DepView::addHole(const Event e)
{
	BUG_ON(e.index > getMax(e.thread));
	holes_[e.thread].insert(e.index);
}

void DepView::addHolesInRange(Event start, int endIdx)
{
	for (auto i = start.index; i < endIdx; i++)
		addHole(Event(start.thread, i));
}

void DepView::removeHole(const Event e) { holes_[e.thread].erase(e.index); }

void DepView::removeAllHoles(int thread) { holes_[thread].clear(); }

void DepView::removeHolesInRange(Event start, int endIdx)
{
	for (auto i = start.index; i < endIdx; i++)
		removeHole(Event(start.thread, i));
}

auto DepView::update(const View &v) -> View & { BUG(); }

auto DepView::update(const DepView &v) -> DepView &
{
	if (v.empty())
		return *this;

	for (auto i = 0U; i < v.size(); i++) {
		auto isec = holes_[i].intersectWith(v.holes_[i]);
		if (getMax(i) < v.getMax(i)) {
			isec.insert(std::lower_bound(v.holes_[i].begin(), v.holes_[i].end(),
						     getMax(i) + 1),
				    v.holes_[i].end());
			view_.setMax(Event(i, v.getMax(i)));
		} else {
			isec.insert(std::lower_bound(holes_[i].begin(), holes_[i].end(),
						     v.getMax(i) + 1),
				    holes_[i].end());
		}
		holes_[i] = std::move(isec);
	}
	return *this;
}

auto DepView::update(const VectorClock &vc) -> VectorClock &
{
	if (const auto *v = llvm::dyn_cast<DepView>(&vc))
		return this->update(*v);
	BUG();
}

void DepView::printData(llvm::raw_ostream &s) const
{
	s << "[\n";
	for (auto i = 0U; i < size(); i++) {
		s << "\t" << i << ": " << getMax(i) << " ( ";
		for (const auto &h : this->holes_[i])
			s << h << " ";
		s << ")\n";
	}

	s << "]";
}
