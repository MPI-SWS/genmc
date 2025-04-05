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

#include "VectorClock.hpp"
#include "ADT/DepView.hpp"
#include "ADT/View.hpp"
#include "ExecutionGraph/EventLabel.hpp"

auto VectorClock::contains(const EventLabel *lab) const -> bool { return contains(lab->getPos()); }

auto VectorClock::clone() const -> std::unique_ptr<VectorClock>
{
	switch (getKind()) {
	case VC_View:
		return std::make_unique<View>(*static_cast<const View *>(this));
	case VC_DepView:
		return std::make_unique<DepView>(*static_cast<const DepView *>(this));
	}
	BUG();
}

auto operator<<(llvm::raw_ostream &s, const VectorClock &vc) -> llvm::raw_ostream &
{
	vc.printData(s);
	return s;
}

auto VectorClock::empty() const -> bool { return size() == 0; }

auto VectorClock::getMax(Event e) const -> int { return getMax(e.thread); }
