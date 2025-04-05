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

#ifndef GENMC_ASIZE_HPP
#define GENMC_ASIZE_HPP

#include "Error.hpp"
#include "config.h"

#include <climits>
#include <cstdint>

/*******************************************************************************
 **                             ASize Class
 ******************************************************************************/

/**
 * Represents the size (in bytes) of an atomic memory access
 */
class ASize {

protected:
	/* We could be a bit more frugal with this, but it should be fine */
	using Size = uint32_t;

public:
	/** Constructors/destructors */
	ASize() = delete;
	ASize(Size s) : size(s) {}

	/** Returns the number of bytes this Size occupies */
	[[nodiscard]] auto get() const -> Size { return size; }

	/** Returns the number of bits this Size occupies */
	[[nodiscard]] auto getBits() const -> Size { return size * CHAR_BIT; }

	inline auto operator<=>(const ASize &other) const = default;
	auto operator()() const -> Size { return size; }

	friend auto operator<<(llvm::raw_ostream &rhs, const ASize &s) -> llvm::raw_ostream &;

private:
	/** The actual size */
	Size size;
};

#endif /* GENMC_ASIZE_HPP */
