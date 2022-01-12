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

#ifndef __WB_ITERATOR_HPP__
#define __WB_ITERATOR_HPP__

#include "config.h"
#include "AdjList.hpp"
#include "Event.hpp"
#include <iterator>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Support/Casting.h>

using WbT = AdjList<Event, EventHasher>;

/*
 * Implements a couple of different traversal strategies for WB.
 * In principle, we could unify these iterators using a WBIterator class and a strategy
 * pattern. This would allow us to have a unified API for iterating WB (similar
 * to the one existing for MO), but since (1) WB and CO are not treated uniformly now,
 * and (2) there is some code duplication between WB and CO, let's just avoid
 * virtual calls altogether.
 */

/*******************************************************************************
 **                         WBPOIterator Class (Abstract)
 ******************************************************************************/

/*
 * This class implements an iterator for WB as a partial order .
 * The way we iterate WB is determined by some specialization of this class.
 */
class WBPOIterator {

public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = Event;
	using difference_type = signed;
	using pointer = const Event *;
	using reference = const Event &;

protected:
	/* We do not use an actual iterator type because we might have
	 * to traverse the store list both backward and forward */
	using iterator_index = int;

	/*** Constructors/destructor ***/
	WBPOIterator() = delete;

	/* constructor to be used by implementation */
	WBPOIterator(const WbT &wb, value_type e, iterator_index curr)
		: wb(wb), elems(wb.getElems()), store(e), curr(curr) {}

public:

	/*** Operators ***/
	inline reference operator*() const { return elems[curr]; }
	inline pointer operator->() const { return &operator*(); }

	inline bool operator==(const WBPOIterator &other) const {
		return elems == other.elems && store == other.store && curr == other.curr;
	}
	inline bool operator!=(const WBPOIterator& other) const {
		return !operator==(other);
	}

protected:
	const WbT &wb;
	const std::vector<Event> &elems;
	value_type store;
	iterator_index curr;
};

/*
 * Iterator for the wb successors of a given event.
 *
 * Notes:
 *     - It does not iterate over the successors in any particular order
 */
class WBPOSuccIterator : public WBPOIterator {

public:
	/* begin()/end() constructors */
	WBPOSuccIterator(const WbT &wb, Event store)
		: WBPOIterator(wb, store, 0) { advanceToSucc(); }
	WBPOSuccIterator(const WbT &wb, Event store, bool)
		: WBPOIterator(wb, store, wb.getElems().size()) {}

	inline WBPOSuccIterator &operator++() {
		++curr;
		advanceToSucc();
		return *this;
	}
	inline WBPOSuccIterator operator++(int) {
               auto tmp = *this; ++*this; return tmp;
	}

private:
	bool isSuccessor() const {
		return elems[curr] != store && (store.isInitializer() || wb(store, elems[curr]));
	}

	void advanceToSucc() {
		while (curr < elems.size() && !isSuccessor())
			++curr;
	}
};


/*
 * Iterator for the wb predecessors of a given event.
 *
 * Notes:
 *     - It does not iterate over the predecessors in any particular order
 *     - For predecessors, the initializer will _NOT_ be looped over
 */
class WBPOPredIterator : public WBPOIterator {

public:
	/* begin()/end() constructors */
	WBPOPredIterator(const WbT &wb, Event store) : WBPOIterator(wb, store, 0) {
		if (store.isInitializer()) {
			curr = elems.size();
			return;
		}
		advanceToPred();
	}
	WBPOPredIterator(const WbT &wb, Event store, bool)
		: WBPOIterator(wb, store, wb.getElems().size()) {}

	inline WBPOPredIterator &operator++() {
		++curr;
		advanceToPred();
		return *this;
	}
	inline WBPOPredIterator operator++(int) {
               auto tmp = *this; ++*this; return tmp;
	}

private:
	bool isPredecessor() const {
		return elems[curr] != store && wb(elems[curr], store);
	}

	void advanceToPred() {
		while (curr < elems.size() && !isPredecessor())
			++curr;
	}
};


/*******************************************************************************
 **                         WBTOIterator Class (Abstract)
 ******************************************************************************/

class WBTOIterator {

public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = Event;
	using difference_type = signed;
	using pointer = const Event *;
	using reference = const Event &;

protected:
	/* We do not use an actual iterator type because we might have
	 * to traverse the store list both backward and forward */
	using iterator_index = int;

	/*** Constructors/destructor ***/
	WBTOIterator() = delete;

	/* constructor to be used by implementation */
	WBTOIterator(const WbT &wb, value_type e, iterator_index curr)
		: elems(wb.getElems()), store(e), curr(curr) {}
	WBTOIterator(const std::vector<Event> &elems, value_type e, iterator_index curr)
		: elems(elems), store(e), curr(curr) {}

public:

	/*** Operators ***/
	inline reference operator*() const { return elems[curr]; }
	inline pointer operator->() const { return &operator*(); }

	inline bool operator==(const WBTOIterator &other) const {
		return elems == other.elems && store == other.store && curr == other.curr;
	}
	inline bool operator!=(const WBTOIterator& other) const {
		return !operator==(other);
	}

protected:
	const std::vector<Event> &elems;
	value_type store;
	iterator_index curr;
};


/*
 * Iterator for the wb successors of an event with WB being a total order
 */
class WBTOSuccIterator : public WBTOIterator {

public:
	/* begin()/end() constructor pairs */
	WBTOSuccIterator(const WbT &wb, Event store)
		: WBTOIterator(wb.getElems(), store, 0) { advanceToFirstSucc(); }
	WBTOSuccIterator(const WbT &wb, Event store, bool)
		: WBTOIterator(wb.getElems(), store, wb.getElems().size()) {}

	WBTOSuccIterator(const std::vector<Event> &elems, Event store)
		: WBTOIterator(elems, store, 0) { advanceToFirstSucc(); }
	WBTOSuccIterator(const std::vector<Event> &elems, Event store, bool)
		: WBTOIterator(elems, store, elems.size()) {}

	inline WBTOSuccIterator &operator++() { ++curr; return *this; }
	inline WBTOSuccIterator operator++(int) {
               auto tmp = *this; ++*this; return tmp;
	}

private:
	void advanceToFirstSucc() {
		if (store.isInitializer())
			return;
		while (curr < elems.size() && elems[curr] != store)
			++curr;
		if (curr < elems.size())
			++curr;
	}
};

/*
 * Iterator for the wb successors of an event with WB being a total order
 */
class WBTOPredIterator : public WBTOIterator {

public:
	/* begin()/end() constructor pairs */
	WBTOPredIterator(const WbT &wb, Event store)
		: WBTOIterator(wb.getElems(), store, elems.size() - 1) { recedeToFirstPred(); }
	WBTOPredIterator(const WbT &wb, Event store, bool)
		: WBTOIterator(wb.getElems(), store, -1) {}

	WBTOPredIterator(const std::vector<Event> &elems, Event store)
		: WBTOIterator(elems, store, elems.size() - 1) { recedeToFirstPred(); }
	WBTOPredIterator(const std::vector<Event> &elems, Event store, bool)
		: WBTOIterator(elems, store, -1) {}


	inline WBTOPredIterator &operator++() { --curr; return *this; }
	inline WBTOPredIterator operator++(int) {
		auto tmp = *this; ++*this; return tmp;
	}

private:
	void recedeToFirstPred() {
		if (store.isInitializer()) {
			curr = -1;
			return;
		}
		while (curr >= 0 && elems[curr] != store)
			--curr;
		if (curr >= 0)
			--curr;
	}
};


/*******************************************************************************
 **                           Convenience accessors
 ******************************************************************************/

/* Successors --- only non-const versions */
using const_wb_po_succ_iterator = WBPOSuccIterator;
using const_wb_to_succ_iterator = WBTOSuccIterator;

using const_wb_po_succ_range = llvm::iterator_range<const_wb_po_succ_iterator>;
using const_wb_to_succ_range = llvm::iterator_range<const_wb_to_succ_iterator>;

inline const_wb_po_succ_iterator wb_po_succ_begin(const WbT &wb, Event e)
{
	return const_wb_po_succ_iterator(wb, e);
}
inline const_wb_po_succ_iterator wb_po_succ_end(const WbT &wb, Event e)
{
	return const_wb_po_succ_iterator(wb, e, true);
}

inline const_wb_to_succ_iterator wb_to_succ_begin(const WbT &wb, Event e)
{
	return const_wb_to_succ_iterator(wb, e);
}
inline const_wb_to_succ_iterator wb_to_succ_begin(const std::vector<Event> &elems, Event e)
{
	return const_wb_to_succ_iterator(elems, e);
}
inline const_wb_to_succ_iterator wb_to_succ_end(const WbT &wb, Event e)
{
	return const_wb_to_succ_iterator(wb, e, true);
}
inline const_wb_to_succ_iterator wb_to_succ_end(const std::vector<Event> &elems, Event e)
{
	return const_wb_to_succ_iterator(elems, e, true);
}

inline const_wb_po_succ_range wb_po_succs(const WbT &wb, Event e) {
	return const_wb_po_succ_range(wb_po_succ_begin(wb, e), wb_po_succ_end(wb, e));
}

inline const_wb_to_succ_range wb_to_succs(const WbT &wb, Event e) {
	return const_wb_to_succ_range(wb_to_succ_begin(wb, e), wb_to_succ_end(wb, e));
}
inline const_wb_to_succ_range wb_to_succs(const std::vector<Event> &elems, Event e) {
	return const_wb_to_succ_range(wb_to_succ_begin(elems, e), wb_to_succ_end(elems, e));
}

/* Predecessors --- only non-const versions */
using const_wb_po_pred_iterator = WBPOPredIterator;
using const_wb_to_pred_iterator = WBTOPredIterator;

using const_wb_po_pred_range = llvm::iterator_range<const_wb_po_pred_iterator>;
using const_wb_to_pred_range = llvm::iterator_range<const_wb_to_pred_iterator>;

inline const_wb_po_pred_iterator wb_po_pred_begin(const WbT &wb, Event e)
{
	return const_wb_po_pred_iterator(wb, e);
}
inline const_wb_po_pred_iterator wb_po_pred_end(const WbT &wb, Event e)
{
	return const_wb_po_pred_iterator(wb, e, true);
}

inline const_wb_to_pred_iterator wb_to_pred_begin(const WbT &wb, Event e)
{
	return const_wb_to_pred_iterator(wb, e);
}
inline const_wb_to_pred_iterator wb_to_pred_begin(const std::vector<Event> &elems, Event e)
{
	return const_wb_to_pred_iterator(elems, e);
}
inline const_wb_to_pred_iterator wb_to_pred_end(const WbT &wb, Event e)
{
	return const_wb_to_pred_iterator(wb, e, true);
}
inline const_wb_to_pred_iterator wb_to_pred_end(const std::vector<Event> &elems, Event e)
{
	return const_wb_to_pred_iterator(elems, e, true);
}

inline const_wb_po_pred_range wb_po_preds(const WbT &wb, Event e) {
	return const_wb_po_pred_range(wb_po_pred_begin(wb, e), wb_po_pred_end(wb, e));
}
inline const_wb_po_pred_range wb_po_preds(const std::vector<Event> &elems, Event e) {
	return const_wb_po_pred_range(wb_po_pred_begin(elems, e), wb_po_pred_end(elems, e));
}

inline const_wb_to_pred_range wb_to_preds(const WbT &wb, Event e) {
	return const_wb_to_pred_range(wb_to_pred_begin(wb, e), wb_to_pred_end(wb, e));
}

#endif /* __WB_ITERATOR_HPP__ */
