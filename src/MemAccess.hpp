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
#include "SAddr.hpp"

#include <climits>
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
 **                             AAccess Class
 ******************************************************************************/

/*
 * An AAccess comprises an address, a size and a type
 */
class AAccess {

public:
	AAccess() = delete;
	AAccess(SAddr a, ASize s, AType t) : addr(a), size(s), type(t) {}

	SAddr getAddr() const { return addr; }
	ASize getSize() const { return size; }
	AType getType() const { return type; }

	bool isPointer() const { return getType() == AType::Pointer; }
	bool isUnsigned() const { return getType() == AType::Unsigned; }
	bool isSigned() const { return getType() == AType::Signed; }

	/* Whether the access contains a given address */
	bool contains(SAddr a) const {
		if (!getAddr().sameStorageAs(a))
			return false;
		return getAddr() <= a && a < getAddr() + getSize();
	}

	/* Whether the access overlaps with another access */
	bool overlaps(const AAccess &other) const {
		if (!getAddr().sameStorageAs(other.getAddr()))
			return false;
		return getAddr() + getSize() > other.getAddr() &&
			getAddr() < other.getAddr() + other.getSize();
	}

private:
	SAddr addr;
	ASize size;
	AType type;
};

#endif /* __MEM_ACCESS_HPP__ */
