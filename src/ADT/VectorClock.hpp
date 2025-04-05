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

#ifndef GENMC_VECTOR_CLOCK_HPP
#define GENMC_VECTOR_CLOCK_HPP

#include "ExecutionGraph/Event.hpp"
#include <llvm/Support/Casting.h>

/*******************************************************************************
 **                        VectorClock Class (Abstract)
 ******************************************************************************/

class View;
class DepView;
class EventLabel;

/**
 * An abstract class for modeling vector clocks. Contains the bare
 * minimum that all different types of vector clocks (e.g., plain ones
 * or dependency tracking ones) should have. Vector clocks are supposed
 * to work with events.
 */
class VectorClock {

public:
	VectorClock(const VectorClock &) = default;
	VectorClock(VectorClock &&) = delete;
	auto operator=(const VectorClock &) -> VectorClock & = default;
	auto operator=(VectorClock &&) -> VectorClock & = delete;

	/** Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * It is public to allow clients perform a switch() on it */
	enum VectorClockKind {
		VC_View,
		VC_DepView,
	};

protected:
	VectorClock(VectorClockKind k) : kind(k) {}

public:
	virtual ~VectorClock() {};

	/** Returns the kind of this vector clock */
	[[nodiscard]] auto getKind() const -> VectorClockKind { return kind; }

	/** Returns the size of this vector clock */
	[[nodiscard]] virtual auto size() const -> unsigned int = 0;

	/** Returns true if this vector clock is empty */
	[[nodiscard]] auto empty() const -> bool;

	virtual void clear() = 0;

	/** Returns true if this clock contains e */
	[[nodiscard]] virtual auto contains(Event e) const -> bool = 0;
	auto contains(const EventLabel *lab) const -> bool;

	/** Updates the clock based on another clock **of the same kind** */
	virtual auto update(const View &v) -> View & = 0;
	virtual auto update(const DepView &v) -> DepView & = 0;
	virtual auto update(const VectorClock &v) -> VectorClock & = 0;

	/** Ensures event E is included in the clock */
	virtual auto updateIdx(Event e) -> VectorClock & = 0;

	[[nodiscard]] virtual auto getMax(int thread) const -> int = 0;
	[[nodiscard]] auto getMax(Event e) const -> int;

	virtual void setMax(Event e) = 0;

	/** Clones a VectorClock */
	[[nodiscard]] auto clone() const -> std::unique_ptr<VectorClock>;

	/** Printing facilities */
	virtual void printData(llvm::raw_ostream &s) const = 0;
	friend auto operator<<(llvm::raw_ostream &s, const VectorClock &v) -> llvm::raw_ostream &;

private:
	/** The kind of this VectorClock */
	VectorClockKind kind;
};

/** Helper cloner class */
struct VectorClockCloner {
	auto operator()(const VectorClock &x) const -> VectorClock * { return x.clone().release(); }
	// VectorClock *operator()(VectorClock &&x) const { return new VectorClock(std::move(x)); }
};

#endif /* GENMC_VECTOR_CLOCK_HPP */
