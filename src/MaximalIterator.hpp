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

#ifndef __MAXIMAL_ITERATOR_HPP__
#define __MAXIMAL_ITERATOR_HPP__

#include "View.hpp"
#include "config.h"
#include <iterator>
#include <llvm/ADT/iterator_range.h>
#include <type_traits>
#include <utility>

/*******************************************************************************
 **                         MaximalIterator Class
 ******************************************************************************/

/*
 * Helper iterator for Views that yields events instead of indices.
 * (Effectively a proxy, but the underlying view cannot be modified.)
 */
class MaximalIterator {

public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = Event;
	using difference_type = signed;
	using pointer = const Event *;
	using reference = const Event &;

	/*** begin()/end() ctors ***/
	MaximalIterator() = delete;

	MaximalIterator(const View &v) : view(v)
	{
		curr = view.begin();
		if (view.begin() != view.end())
			e = Event(0, *curr);
	}

	MaximalIterator(const View &v, bool) : view(v) { curr = v.end(); }

	/*** Operators ***/
	inline reference operator*() const { return e; }
	inline pointer operator->() const { return &operator*(); }

	inline bool operator==(const MaximalIterator &other) const { return curr == other.curr; }

	inline bool operator!=(const MaximalIterator &other) const { return !operator==(other); }

	MaximalIterator &operator++()
	{
		if (++curr != view.end())
			e = Event(curr - view.begin(), *curr);
		return *this;
	}
	inline MaximalIterator operator++(int)
	{
		auto tmp = *this;
		++*this;
		return tmp;
	}

	MaximalIterator &operator--()
	{
		if (--curr != view.end())
			e = Event(curr - view.begin(), *curr);
		return *this;
	}
	inline MaximalIterator operator--(int)
	{
		auto tmp = *this;
		--*this;
		return tmp;
	}

protected:
	const View &view;
	View::const_iterator curr;
	Event e = Event::getInit();
};

using maximal_iterator = MaximalIterator;
using maximal_range = llvm::iterator_range<maximal_iterator>;

inline maximal_iterator maximal_begin(const View &v) { return maximal_iterator(v); }
inline maximal_iterator maximal_end(const View &v) { return maximal_iterator(v, true); }
inline maximal_range maximals(const View &v)
{
	return maximal_range(maximal_begin(v), maximal_end(v));
}

#define FOREACH_MAXIMAL(result, graph, view)                                                       \
	for (auto i = 0u; i < view.size(); i++)                                                    \
		if (auto *result = graph.getEventLabel(Event(i, view.getMax(i))); true)

#endif /*__MAXIMAL_ITERATOR_HPP__ */
