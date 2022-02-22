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

#ifndef __DEP_VIEW_HPP__
#define __DEP_VIEW_HPP__

#include "Error.hpp"
#include "VectorClock.hpp"
#include "View.hpp"
#include "VSet.hpp"
#include <llvm/ADT/IndexedMap.h>
#include <llvm/Support/raw_ostream.h>

/*******************************************************************************
 **                             View Class
 ******************************************************************************/

/*
 * An instantiation of a vector clock where if an event is contained in the clock,
 * it is _not_ guaranteed that all of its po-predecessors are also contained in
 * the clock.
 */
class DepView : public VectorClock {
private:
	using Holes = VSet<int>;

	class HoleView {
		llvm::IndexedMap<Holes> hs_;

	public:
		HoleView() : hs_(Holes()) {}

		unsigned int size() const { return hs_.size(); }

		inline const Holes& operator[](int idx) const {
			if (idx < hs_.size())
				return hs_[idx];
			BUG();
		}

		inline Holes& operator[](int idx) {
			hs_.grow(idx);
			return hs_[idx];
		}
	};

public:
	/* Constructors */
	DepView() : VectorClock(VectorClock::VectorClockKind::VC_DepView),
		    view_(), holes_() {}

	/* Returns the size of the depview (i.e., number of threads seen) */
	unsigned int size() const { return view_.size(); }

	/* Returns true if the clock is empty */
	bool empty() const { return size() == 0; }

	/* Returns true if the clock contains e */
	bool contains(const Event e) const;

	/* Returns true if there's a hole in E's position */
	bool hasHole(const Event e) const {
		return e.thread < holes_.size() && !holes_[e.thread].count(e.index);
	}

	/* Records that the event in the index of e has not been
	 * seen in the respective thread */
	void addHole(const Event e);

	/* Similar to addHole(), but records that a range has not been seen */
	void addHolesInRange(Event start, int endIdx);

	/* Marks event e as seen */
	void removeHole(const Event e);

	/* Marks all events of "thread" as seen */
	void removeAllHoles(int thread);

	/* Marks all events in a range as seen */
	void removeHolesInRange(Event start, int endIdx);

	/* Updates the view based on another clock. The update is valid
	 * only if the other clock provided is also a DepView */
	View& update(const View &v);
	DepView& update(const DepView &v);
	VectorClock &update(const VectorClock &vc);

	/* Overloaded operators */
	inline int operator[](int idx) const {
		return view_[idx];
	}
	inline int &operator[](int idx) {
		holes_[idx]; /* grow both */
		return view_[idx];
	}

	void printData(llvm::raw_ostream &s) const;

	static bool classof(const VectorClock *vc) {
		return vc->getKind() == VC_DepView;
	}

private:
	/* A view containing the highest index seen for each thread */
	View view_;

	/* A view of holes, showing which events are not seen for each thread */
	HoleView holes_;
};

#endif /* __DEP_VIEW_HPP__ */
