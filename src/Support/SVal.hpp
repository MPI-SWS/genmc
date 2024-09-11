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

#ifndef GENMC_SVAL_HPP
#define GENMC_SVAL_HPP

#include "Error.hpp"
#include "config.h"

#include <climits>
#include <cstdint>

/*
 * Represents a value to be written to memory. All values are represented as
 * integers, and the interpreter has to convert these values to values of
 * the appropriate type.
 */
class SVal {

public:
	/* We represent Values using a type that is big-enough
	 * to accommodate for all the types we are interested in */
	using Value = uint64_t;
	static constexpr unsigned width = sizeof(Value) * CHAR_BIT;

	/* Constructors/destructors */
	SVal() : value(0) {}
	explicit SVal(uint64_t v) : value(v) {}

	/* Returns a (limited) representation of this Value */
	[[nodiscard]] auto get() const -> uint64_t { return value; }

	/* Returns a (limited) signed representation of this Value */
	[[nodiscard]] auto getSigned() const -> int64_t
	{
		int64_t tmp;
		std::memcpy(&tmp, &value, sizeof(tmp));
		return tmp;
	}

	/* Returns a pointer representation of this Value */
	[[nodiscard]] auto getPointer() const -> void * { return (void *)(uintptr_t)value; }

	/* Returns a (limited) representation of the Value as a boolean */
	[[nodiscard]] auto getBool() const -> bool { return (!!*this); }

	/* Sign-extends the number in the bottom B bits of X to SVal::width
	 * Pre: 0 < B <= SVal::width */
	auto signExtendBottom(unsigned b) -> SVal &
	{
		BUG_ON(b == 0 || b > width);
		value = int64_t(get() << (width - b)) >> (width - b);
		return *this;
	}

	/* Equality operators */

	inline auto operator==(const SVal &v) const -> bool { return v.value == value; }
	inline auto operator!=(const SVal &v) const -> bool { return !(*this == v); }

	/* Comparison operators */

	/* Returns true if *this < v if both are considered unsigned */
	[[nodiscard]] auto ult(const SVal &v) const -> bool { return compare(v) < 0; }

	/* Returns true if *this < v if both are considered signed */
	[[nodiscard]] auto slt(const SVal &v) const -> bool { return compareSigned(v) < 0; }

	/* Returns true if *this <= v when both are considered unsigned */
	[[nodiscard]] auto ule(const SVal &v) const -> bool { return compare(v) <= 0; }

	/* Returns true if *this <= RHS when both are considered signed */
	[[nodiscard]] auto sle(const SVal &v) const -> bool { return compareSigned(v) <= 0; }

	/* Returns true if *this > RHS when both are considered unsigned */
	[[nodiscard]] auto ugt(const SVal &v) const -> bool { return !ule(v); }

	/* Returns true if *this > RHS when both are considered signed */
	[[nodiscard]] auto sgt(const SVal &v) const -> bool { return !sle(v); }

	/* Returns true if *this >= RHS when both are considered unsigned */
	[[nodiscard]] auto uge(const SVal &v) const -> bool { return !ult(v); }

	/* Returns true if *this >= RHS when both are considered signed */
	[[nodiscard]] auto sge(const SVal &v) const -> bool { return !slt(v); }

	/* Binary operators */

#define IMPL_BINOP(_op)                                                                            \
	SVal operator _op(const SVal &v) const                                                     \
	{                                                                                          \
		SVal n(*this);                                                                     \
		n.value _op## = v.value;                                                           \
		return n;                                                                          \
	}                                                                                          \
	SVal &operator _op##=(const SVal &v)                                                       \
	{                                                                                          \
		value _op## = v.value;                                                             \
		return *this;                                                                      \
	}

	IMPL_BINOP(+);
	IMPL_BINOP(-);
	IMPL_BINOP(*);
	IMPL_BINOP(/);
	IMPL_BINOP(%);
	IMPL_BINOP(&);
	IMPL_BINOP(|);
	IMPL_BINOP(^);
	IMPL_BINOP(<<);
	IMPL_BINOP(>>);

	auto operator~() const -> SVal { return SVal(~this->value); }

	explicit operator bool() const { return !!this->value; }

	[[nodiscard]] auto toString(bool sign = false) const -> std::string
	{
		return sign ? std::to_string(getSigned()) : std::to_string(get());
	}

	friend auto operator<<(llvm::raw_ostream &rhs, const SVal &v) -> llvm::raw_ostream &;

private:
	[[nodiscard]] auto compare(const SVal &v) const -> int
	{
		return this->value < v.value ? -1 : this->value > v.value;
	}

	[[nodiscard]] auto compareSigned(const SVal &v) const -> int
	{
		auto lhsSext = getSigned();
		auto rhsSext = v.getSigned();
		return lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
	}

	/* The actual value */
	Value value;
};

struct SValUCmp {
	auto operator()(const SVal &lhs, const SVal &rhs) -> bool { return lhs.ult(rhs); }
};

#endif /* GENMC_SVAL_HPP */
