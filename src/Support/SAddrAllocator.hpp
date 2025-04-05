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

#include "config.h"

#include "Support/SAddr.hpp"

#include <unordered_map>

class VectorClock;

/*******************************************************************************
 **                         SAddrAllocator Class
 ******************************************************************************/

/**
 * Helper class that allocates addresses within the SAddr domain.  A
 * given allocator will never allocate the same address twice.  This
 * class is *not* thread-safe: each thread should own a different
 * allocator.
 */
class SAddrAllocator {

protected:
	/** Allocates a fresh address at the specified pool */
	template <typename F>
	auto allocate(F &allocFun, SAddr::Width &pool, unsigned int thread, unsigned int size,
		      unsigned int alignment, bool isDurable = false, bool isInternal = false)
		-> SAddr
	{
		auto offset = alignment - 1;
		unsigned newAddr = (pool + offset) & ~offset;
		pool = newAddr + size;
		return allocFun(thread, newAddr, static_cast<bool &&>(isDurable),
				static_cast<bool &&>(isInternal));
	}

public:
	SAddrAllocator() = default;

	/** Allocating methods. Param format: thread, size, alignment, durable?, internal?  */
	template <typename... Ts> auto allocStatic(unsigned thread, Ts &&...params) -> SAddr
	{
		return allocate(SAddr::createStatic<SAddr::Width, SAddr::Width, bool, bool>,
				staticPool_[thread], thread, std::forward<Ts>(params)...);
	}
	template <typename... Ts> auto allocAutomatic(unsigned thread, Ts &&...params) -> SAddr
	{
		return allocate(SAddr::createAutomatic<SAddr::Width, SAddr::Width, bool, bool>,
				dynamicPool_[thread], thread, std::forward<Ts>(params)...);
	}
	template <typename... Ts> auto allocHeap(unsigned thread, Ts &&...params) -> SAddr
	{
		return allocate(SAddr::createHeap<SAddr::Width, SAddr::Width, bool, bool>,
				dynamicPool_[thread], thread, std::forward<Ts>(params)...);
	}

	void restrict(const VectorClock &view);

	friend auto operator<<(llvm::raw_ostream &rhs, const SAddrAllocator &alloctor)
		-> llvm::raw_ostream &;

private:
	/** Helper class to avoid allocating null for heap addresses */
	class WidthProxy {
	public:
		WidthProxy(SAddr::Width value = 1 /* default non-zero value */) : value_(value) {}
		operator auto &() { return value_; }
		operator auto const &() const { return value_; }

	private:
		SAddr::Width value_;
	};

	std::unordered_map<unsigned, SAddr::Width> staticPool_;
	std::unordered_map<unsigned, WidthProxy> dynamicPool_; // Note different type here
};

#endif /* GENMC_SADDR_ALLOCATOR_HPP */
