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

#ifndef GENMC_SADDR_ALLOCATOR_HPP
#define GENMC_SADDR_ALLOCATOR_HPP

#include "Support/SAddr.hpp"

#include <format>
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

	friend struct std::formatter<SAddrAllocator>;

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

/** Make `SAddrAllocator` formattable with `std::format`. */
template <> struct std::formatter<SAddrAllocator> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const SAddrAllocator &allocator, std::format_context &ctx) const
	{
		auto out = std::format_to(ctx.out(), "static: ");
		for (const auto &[tid, idx] : allocator.staticPool_) {
			out = std::format_to(out, "({}, {}) ", tid, idx);
		}
		out = std::format_to(out, "\ndynamic: ");
		for (const auto &[tid, idx] : allocator.dynamicPool_) {
			out = std::format_to(out, "({}, {}) ", tid, static_cast<SAddr::Width>(idx));
		}
		return std::format_to(out, "\n");
	}
};

#endif /* GENMC_SADDR_ALLOCATOR_HPP */
