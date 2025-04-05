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

/*
 * This file defines the memory orderings supported by GenMC.
 * Inspired by the AtomicOrdering class in LLVM
 */

#ifndef GENMC_MEMORDERING_HPP
#define GENMC_MEMORDERING_HPP

#include "Support/Error.hpp"

#include <llvm/Support/AtomicOrdering.h>
#include <llvm/Support/raw_ostream.h>

#include <concepts>
#include <cstddef>
#include <cstdint>

/** C11 memory ordering */
enum class MemOrdering : std::uint8_t {
	NotAtomic = 0,
	Relaxed = 1,
	// In case we support consume
	Acquire = 3,
	Release = 4,
	AcquireRelease = 5,
	SequentiallyConsistent = 6,
	LAST = SequentiallyConsistent
};

auto operator<(MemOrdering, MemOrdering) -> bool = delete;
auto operator>(MemOrdering, MemOrdering) -> bool = delete;
auto operator<=(MemOrdering, MemOrdering) -> bool = delete;
auto operator>=(MemOrdering, MemOrdering) -> bool = delete;

/** Helper to validate unknown integral types */
template <typename Int>
requires std::integral<Int>
inline auto isValidMemOrdering(Int i) -> bool
{
	return static_cast<Int>(MemOrdering::NotAtomic) <= i &&
	       i <= static_cast<Int>(MemOrdering::LAST) && i != 2;
}

/** Returns whether ord is stronger than other */
inline auto isStrongerThan(MemOrdering ord, MemOrdering other) -> bool
{
	static const bool lookup[7][7] = {
		//               NA      RX     CO     AC     RE     AR     SC
		/* nonatomic */ {false, false, false, false, false, false, false},
		/* relaxed   */ {true, false, false, false, false, false, false},
		/* consume   */ {true, true, false, false, false, false, false},
		/* acquire   */ {true, true, true, false, false, false, false},
		/* release   */ {true, true, false, false, false, false, false},
		/* acq_rel   */ {true, true, true, true, true, false, false},
		/* seq_cst   */ {true, true, true, true, true, true, false},
	};
	return lookup[static_cast<size_t>(ord)][static_cast<size_t>(other)];
}

inline auto isAtLeastOrStrongerThan(MemOrdering ord, MemOrdering other) -> bool
{
	static const bool lookup[7][7] = {
		//               NA     RX     CO     AC     RE     AR     SC
		/* nonatomic */ {true, false, false, false, false, false, false},
		/* relaxed   */ {true, true, false, false, false, false, false},
		/* consume   */ {true, true, true, false, false, false, false},
		/* acquire   */ {true, true, true, true, false, false, false},
		/* release   */ {true, true, false, false, true, false, false},
		/* acq_rel   */ {true, true, true, true, true, true, false},
		/* seq_cst   */ {true, true, true, true, true, true, true},
	};
	return lookup[static_cast<size_t>(ord)][static_cast<size_t>(other)];
}

/** Translates an LLVM ordering to our internal one; assumes the
 * ordering is one we support (i.e., currently not Unordered)*/
inline auto fromLLVMOrdering(llvm::AtomicOrdering ord) -> MemOrdering
{
	static const MemOrdering lookup[8] = {
		/* NotAtomic */ MemOrdering::NotAtomic,
		/* Unordered */ MemOrdering::Relaxed,
		/* monotonic */ MemOrdering::Relaxed,
		/* consume   */ MemOrdering::Acquire,
		/* acquire   */ MemOrdering::Acquire,
		/* release   */ MemOrdering::Release,
		/* acq_rel   */ MemOrdering::AcquireRelease,
		/* seq_cst   */ MemOrdering::SequentiallyConsistent,
	};
	return lookup[static_cast<size_t>(ord)];
}

auto operator<<(llvm::raw_ostream &rhs, MemOrdering ord) -> llvm::raw_ostream &;

#endif /* GENMC_MEMORDERING_HPP */
