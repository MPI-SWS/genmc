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
n * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#ifndef __DEP_INFO_HPP__
#define __DEP_INFO_HPP__

#include "Event.hpp"
#include "VSet.hpp"
#include <llvm/Support/raw_ostream.h>

/*******************************************************************************
 **                             DepInfo Class
 ******************************************************************************/

/*
 * A class to model the dependencies (of some kind) of an event. Each DepInfo
 * objects holds a collection of events on which some events depend on. In
 * principle, such an object should be used for each event of each thread.
 */
class DepInfo {

protected:
	using Set = VSet<Event>;

public:
	/* Constructors */
	DepInfo() : set_() {}
	DepInfo(Event e) : set_({ e }) {}

	/* Updates this object based on the dependencies of dep (union) */
	void update(const DepInfo& dep);

	/* Clears all the stored dependencies */
	void clear();

	/* Returns true if e is contained in the dependencies */
	bool contains(Event e) const { return set_.count(e); }

	/* Returns true if there are no dependencies */
	bool empty() const;

	/* Iterators */
	using const_iterator = typename Set::const_iterator;
	using const_reverse_iterator = typename Set::const_reverse_iterator;

	const_iterator begin() const { return set_.begin(); };
	const_iterator end() const { return set_.end(); };

	/* Printing */
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const DepInfo &dep);

private:
	/* The actual container for the dependencies */
	Set set_;
};

#endif /* __DEP_INFO_HPP__ */
