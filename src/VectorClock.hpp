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

#ifndef __VECTOR_CLOCK_HPP__
#define __VECTOR_CLOCK_HPP__

#include "Event.hpp"
#include <llvm/Support/Casting.h>

/*******************************************************************************
 **                        VectorClock Class (Abstract)
 ******************************************************************************/

/* Instantiations of this abstract class */
class View;
class DepView;

/*
 * An abstract class for modeling vector clocks. Contains the bare
 * minimum that all different types of vector clocks (e.g., plain ones
 * or dependency tracking ones) should have. Vector clocks are supposed
 * to work with events.
 */
class VectorClock {

public:
	/* Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * It is public to allow clients perform a switch() on it */
	enum VectorClockKind {
		VC_View,
		VC_DepView,
	};

protected:
	VectorClock(VectorClockKind k) : kind(k) {}

public:
	/* Returns the kind of this vector clock */
	VectorClockKind getKind() const { return kind; }

	/* Returns true if this clock contains e */
	virtual bool contains(const Event e) const = 0;

	/* Updates the clock based on another clock **of the same kind** */
	virtual View& update(const View &v) = 0;
	virtual DepView& update(const DepView &v) = 0;
	virtual VectorClock& update(const VectorClock &v) = 0;

	/* Returns the size of this vector clock. As size, we basically
	 * define the ID of the maximum thread the clock has seen */
	virtual unsigned int size() const = 0;

	/* Returns true if this vector clock is empty */
	bool empty() const { return size() == 0; }

	/* Clones a VectorClock (deep copying) */
	VectorClock *clone() const;

	virtual ~VectorClock() {};

	/* Returns the ID of the maximum event in thread with ID=idx */
	virtual inline int operator[](int idx) const = 0;
	virtual inline int &operator[](int idx) = 0;

	/* Printing facilities */
	virtual void printData(llvm::raw_ostream &s) const = 0;
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const VectorClock &v);

private:
	/* The kind of this VectorClock */
	VectorClockKind kind;
};

#endif /* __VECTOR_CLOCK_HPP__ */
