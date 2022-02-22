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

#ifndef __MEM_ACCESS_HPP__
#define __MEM_ACCESS_HPP__

#include "config.h"
#include "Error.hpp"

#include <cstdint>

/*******************************************************************************
 **                             AType Enum
 ******************************************************************************/

/*
 * Represents the type of an access: pointer, signed integer, unsigned integer
 */
enum class AType {
	Pointer, Signed, Unsigned
};


/*******************************************************************************
 **                             ASize Class
 ******************************************************************************/

/*
 * Represents the size (in bytes) of an atomic memory access
 */
class ASize {

protected:
	/* We could be a bit more frugal with this, but it should be fine */
	using Size = uint32_t;

public:
	/* Constructors/destructors */
	ASize() = delete;
	ASize(Size s) : size(s) {}

	/* Returns the number of bytes this Size occupies */
	Size get() const { return size; }

	/* Returns the number of bits this Size occupies x*/
	Size getBits() const { return size * CHAR_BIT; }

	inline bool operator==(const ASize &s) const {
		return s.size == size;
	}
	inline bool operator!=(const ASize &s) const {
		return !(*this == s);
	}
	inline bool operator<=(const ASize &s) const {
		return size <= s.size;
	}
	inline bool operator<(const ASize &s) const {
		return size < s.size;
	}
	inline bool operator>=(const ASize &s) const {
		return !(*this < s);
	}
	inline bool operator>(const ASize &s) const {
		return !(*this <= s);
	}
	uint64_t operator()() const { return size; }

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const ASize &s);

private:
	/* The actual size */
	Size size;
};


/*******************************************************************************
 **                             AAccess Class
 ******************************************************************************/

class AAccess {

public:
	AAccess() = delete;
	AAccess(ASize s, AType t) : size(s), type(t) {}

	ASize getSize() const { return size; }
	AType getType() const { return type; }

	bool isPointer() const { return getType() == AType::Pointer; }
	bool isUnsigned() const { return getType() == AType::Unsigned; }
	bool isSigned() const { return getType() == AType::Signed; }

private:
	ASize size;
	AType type;
};

#endif /* __MEM_ACCESS_HPP__ */
