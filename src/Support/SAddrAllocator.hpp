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

#ifndef GENMC_SADDR_ALLOCATOR_HPP
#define GENMC_SADDR_ALLOCATOR_HPP

#include "Error.hpp"
#include "SAddr.hpp"
#include "config.h"

/*******************************************************************************
 **                         SAddrAllocator Class
 ******************************************************************************/

/*
 * Helper class that allocates addresses within the SAddr domain.  A
 * given allocator will never allocate the same address twice.  This
 * class is *not* thread-safe: each thread should own a different
 * allocator.
 */
class SAddrAllocator {

protected:
	/* Allocates a fresh address at the specified pool */
	template <typename F>
	SAddr allocate(F &&allocFun, SAddr::Width &pool, unsigned int size, unsigned int alignment,
		       bool isDurable = false, bool isInternal = false)
	{
		auto offset = alignment - 1;
		auto oldAddr = pool;
		pool += (offset + size);
		return allocFun(((SAddr::Width)oldAddr + offset) & ~(alignment - 1),
				static_cast<bool &&>(isDurable), static_cast<bool &&>(isInternal));
	}

public:
	SAddrAllocator() = default;

	/* Allocating methods */
	template <typename... Ts> SAddr allocStatic(Ts &&...params)
	{
		return allocate(SAddr::createStatic<SAddr::Width, bool, bool>, staticPool,
				std::forward<Ts>(params)...);
	}
	template <typename... Ts> SAddr allocAutomatic(Ts &&...params)
	{
		return allocate(SAddr::createAutomatic<SAddr::Width, bool, bool>, automaticPool,
				std::forward<Ts>(params)...);
	}
	template <typename... Ts> SAddr allocHeap(Ts &&...params)
	{
		return allocate(SAddr::createHeap<SAddr::Width, bool, bool>, heapPool,
				std::forward<Ts>(params)...);
	}

private:
	SAddr::Width staticPool = 0;
	SAddr::Width automaticPool = 0;
	SAddr::Width heapPool = 1; /* avoid allocating null */
};

#endif /* GENMC_SADDR_ALLOCATOR_HPP */
