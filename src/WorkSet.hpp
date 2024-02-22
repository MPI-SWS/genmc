/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <mixaskok@gmail.com>
 */

#ifndef __WORK_SET_HPP__
#define __WORK_SET_HPP__

#include "EventLabel.hpp"
#include "Revisit.hpp"
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

/*
 * WorkSet class - Represents the set of work items for one particular event
 */
class WorkSet {

public:
	using ItemT = std::unique_ptr<Revisit>;

protected:
	using WorkSetT = std::vector<ItemT>;
	using iterator = WorkSetT::iterator;
	using const_iterator = WorkSetT::const_iterator;

public:
	/* Iterators */
	iterator begin() { return wset_.begin(); }
	iterator end() { return wset_.end(); }
	const_iterator cbegin() const { return wset_.cbegin(); }
	const_iterator cend() const { return wset_.cend(); }

	/* Returns whether this workset is empty */
	bool empty() const { return wset_.empty(); }

	/* Adds an item to the workset */
	void add(ItemT item)
	{
		wset_.push_back(std::move(item));
		return;
	}

	/* Returns the next item to examine for this workset */
	ItemT getNext()
	{
		auto i = std::move(wset_.back());
		wset_.pop_back();
		return i;
	}

	/* Overloaded operators */
	friend llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const WorkSet &wset);

private:
	/* The workset of an event */
	WorkSetT wset_;
};

#endif /* __WORK_SET_HPP__ */
