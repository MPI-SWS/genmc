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

#ifndef GENMC_MEM_ACCESS_HPP
#define GENMC_MEM_ACCESS_HPP

#include "SAddr.hpp"

#include <climits>

/*******************************************************************************
 **                             AAccess Class
 ******************************************************************************/

/**
 * An AAccess comprises an address and a size
 */
struct AAccess {

	AAccess() = default;
	constexpr AAccess(SAddr addr, ASize size) : addr(addr), size(size) {}

	/** Whether the access contains a given address */
	[[nodiscard]] auto contains(SAddr point) const -> bool
	{
		return addr.sameStorageAs(point) && addr <= point && point < addr + size;
	}

	/** Whether the access overlaps with another access */
	[[nodiscard]] auto overlaps(const AAccess &other) const -> bool
	{
		return addr.sameStorageAs(other.addr) && addr + size > other.addr &&
		       addr < other.addr + other.size;
	}

	auto operator==(const AAccess &other) const -> bool = default;

	SAddr addr;
	ASize size;
};

#endif /* GENMC_MEM_ACCESS_HPP */
