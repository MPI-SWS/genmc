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

#ifndef __SVAL_HPP__
#define __SVAL_HPP__

#include "config.h"
#include "Error.hpp"

#include <cstdint>
#include <climits>

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

public:
	/* Constructors/destructors */
	SVal() : value(0) {}
	SVal(uint64_t v) : value(v) {}

	/* Returns a (limited) representation of this Value */
	uint64_t get() const { return value; }

	/* Returns a (limited) signed representation of this Value */
	int64_t getSigned() const {
		int64_t tmp;
		std::memcpy(&tmp, &value, sizeof(tmp));
		return tmp;
	}

	/* Returns a pointer representation of this Value */
	void *getPointer() const {
		return (void *) (uintptr_t) value;
	}

	/* Returns a (limited) representation of the Value as a boolean */
	bool getBool() const { return (!!*this).get(); }

	/* Sign-extends the number in the bottom B bits of X to SVal::width
	 * Pre: 0 < B <= SVal::width */
	SVal &signExtendBottom(unsigned b) {
		BUG_ON(b == 0 || b > width);
		value = int64_t(get() << (width - b)) >> (width - b);
		return *this;
	}

	inline bool operator==(const SVal &v) const {
		return v.value == value;
	}
	inline bool operator!=(const SVal &v) const {
		return !(*this == v);
	}
	inline bool operator<=(const SVal &v) const {
		return value <= v.value;
	}
	inline bool operator<(const SVal &v) const {
		return value < v.value;
	}
	inline bool operator>=(const SVal &v) const {
		return !(*this < v);
	}
	inline bool operator>(const SVal &v) const {
		return !(*this <= v);
	}

#define IMPL_BINOP(_op)				  \
	SVal operator _op (const SVal &v) const { \
		SVal n(*this);			  \
		n.value _op##= v.value;		  \
		return n;			  \
	}					  \
	SVal &operator _op##= (const SVal &v) {	  \
		value _op##= v.value; 		  \
		return *this;			  \
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

	SVal operator!() const {
		SVal n(*this);
		n.value = !value;
		return n;
	}

	operator bool() const {
		return !!value;
	}
	uint64_t operator()() const { return value; }

	std::string toString(bool sign = false) const {
		return sign ? std::to_string(getSigned()) : std::to_string(get());
	}

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const SVal &v);

private:
	/* The actual value */
	Value value;
};

#endif /* __SVAL_HPP__ */
