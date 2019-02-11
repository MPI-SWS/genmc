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

class View {
protected:
	typedef llvm::IndexedMap<int> EventView;
	EventView view_;

public:
	/* Constructors */
	View();

	/* Basic operation on Views */
	unsigned int size() const;
	bool empty() const;
	void updateMax(View &v);
	View getMax(View &v);

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
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const View &v);
};

#endif /* __VIEW_HPP__ */
