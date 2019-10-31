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

#ifndef __VIEW_HPP__
#define __VIEW_HPP__

#include <llvm/ADT/IndexedMap.h>
#include <llvm/Support/raw_ostream.h>
#include "Event.hpp"
#include "VectorClock.hpp"

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
	View();

	/* Iterators */
	typedef int *iterator;
	typedef const int *const_iterator;
	iterator begin();
	iterator end();
	const_iterator cbegin();
	const_iterator cend();

	/* Returns the size of this view (i.e., number of threads seen) */
	unsigned int size() const;

	/* Returns true if this view is empty */
	bool empty() const;

	/* Returns true if e is contained in the clock */
	bool contains(const Event e) const;

	/* Updates the view based on another vector clock. We can
	 * only update the current view given another View (and not
	 * some other subclass of VectorClock) */
	View& update(const View &v);
	DepView& update(const DepView &dv);
	VectorClock &update(const VectorClock &vc);

	/* Makes the maximum event seen in e's thread equal to e */
	View& updateIdx(const Event e);

	/* Overloaded operators */
	inline int operator[](int idx) const {
		if (idx < (int) view_.size())
			return view_[idx];
		else
			return 0;
	}
	inline int &operator[](int idx) {
		view_.grow(idx);
		return view_[idx];
	}
	inline bool operator<=(const View &v) const {
		for (auto i = 0u; i < this->size(); i++)
			if ((*this)[i] > v[i])
				return false;
		return true;
	}

	void printData(llvm::raw_ostream &s) const;

	static bool classof(const VectorClock *vc) {
		return vc->getKind() == VC_View;
	}
};

#endif /* __VIEW_HPP__ */
