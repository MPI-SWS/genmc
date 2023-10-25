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

#ifndef GENMC_STAMP_HPP
#define GENMC_STAMP_HPP

#include "config.h"
#include "Error.hpp"

#include <cstdint>

/*
 * Represents a label stamp (positive number).
 */
class Stamp {

protected:
	/* Assuming re-assigning, uint32_t should suffice */
	using Value = uint32_t;

public:
	/* Constructors/destructors */
	Stamp() = delete;
	constexpr Stamp(uint32_t v) : value(v) {}
	constexpr Stamp(const Stamp &other) = default;
	constexpr Stamp(Stamp &&other) = default;

	inline bool operator==(const Stamp &v) const {
		return v.value == value;
	}
	inline bool operator!=(const Stamp &v) const {
		return !(*this == v);
	}
	inline bool operator<=(const Stamp &v) const {
		return value <= v.value;
	}
	inline bool operator<(const Stamp &v) const {
		return value < v.value;
	}
	inline bool operator>=(const Stamp &v) const {
		return !(*this < v);
	}
	inline bool operator>(const Stamp &v) const {
		return !(*this <= v);
	}

#define IMPL_STAMP_BINOP(_op)			  \
	Stamp &operator _op##= (uint32_t v) {	  \
		value _op##= v; 		  \
		return *this;			  \
	}					  \
	Stamp operator _op (uint32_t v) const {	  \
		Stamp n(*this);			  \
		n += v;				  \
		return n;			  \
	}

	IMPL_STAMP_BINOP(+);
	IMPL_STAMP_BINOP(-);

	Stamp &operator++() {
		return (*this) += 1;
	}
	Stamp operator++(int) {
		auto tmp = *this; ++*this; return tmp;
	}

	Stamp &operator--() {
		return (*this) -= 1;
	}
	Stamp operator--(int) {
		auto tmp = *this; --*this; return tmp;
	}

	Stamp &operator=(const Stamp &other) = default;
	Stamp &operator=(Stamp &&other) = default;

	/* Type-system hole */
	uint32_t get() const { return value; }
	uint32_t operator()() const { return get(); }

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const Stamp &s);

private:
	Value value;
};

#endif /* GENMC_STAMP_HPP */
