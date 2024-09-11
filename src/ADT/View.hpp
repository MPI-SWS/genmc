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

#ifndef GENMC_VIEW_HPP
#define GENMC_VIEW_HPP

#include "ADT/VectorClock.hpp"
#include "ExecutionGraph/Event.hpp"
#include "Support/Error.hpp"
#include <llvm/ADT/IndexedMap.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>

/*******************************************************************************
 **                             View Class
 ******************************************************************************/

/*
 * An instantiation of a vector clock where it is assumed that if an index
 * is contained in the clock, all of its po-predecessors are also contained
 * in the clock.
 */
class View : public VectorClock {
private:
	typedef llvm::IndexedMap<int> EventView;
	EventView view_;

public:
	/* Constructors */
	View() : VectorClock(VectorClock::VectorClockKind::VC_View), view_(EventView(0)) {}

	/* Iterators */
	using iterator = int *;
	using const_iterator = const int *;

	iterator begin() { return &view_[0]; };
	iterator end() { return &view_[0] + size(); }
	const_iterator begin() const { return empty() ? nullptr : &view_[0]; }
	const_iterator end() const { return empty() ? nullptr : &view_[0] + size(); }

	/* Returns the size of this view (i.e., number of threads seen) */
	unsigned int size() const { return view_.size(); }

	/* Returns true if this view is empty */
	bool empty() const { return size() == 0; }

	void clear() override { view_.clear(); }

	/* Returns true if e is contained in the clock */
	bool contains(const Event e) const { return e.index <= getMax(e.thread); }

	/* Updates the view based on another vector clock. We can
	 * only update the current view given another View (and not
	 * some other subclass of VectorClock) */
	View &update(const View &v) override;
	DepView &update(const DepView &dv) override;
	VectorClock &update(const VectorClock &vc) override;

	/* Makes the maximum event seen in e's thread equal to e */
	View &updateIdx(Event e) override
	{
		if (getMax(e.thread) < e.index)
			setMax(e);
		return *this;
	}

	int getMax(int thread) const override
	{
		if (thread < (int)view_.size())
			return view_[thread];
		return 0;
	}

	void setMax(Event e) override
	{
		if (e.thread >= (int)view_.size())
			view_.grow(e.thread);
		view_[e.thread] = e.index;
	}

	void printData(llvm::raw_ostream &s) const;

	static bool classof(const VectorClock *vc) { return vc->getKind() == VC_View; }
};

#endif /* GENMC_VIEW_HPP */
