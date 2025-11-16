/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#ifndef GENMC_ASIZE_HPP
#define GENMC_ASIZE_HPP

#include "Error.hpp"

#include <climits>
#include <cstdint>
#include <format>

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

	auto operator<=>(const ASize &other) const = default;
	auto operator()() const -> Size { return size; }

private:
	/** The actual size */
	Size size;
};

/** Make `ASize` formattable with `std::format`. */
template <> struct std::formatter<ASize> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const ASize &size, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}", size.get());
	}
};

#endif /* GENMC_ASIZE_HPP */
