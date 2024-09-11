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

#ifndef GENMC_SADDR_HPP
#define GENMC_SADDR_HPP

#include "ASize.hpp"
#include "Error.hpp"
#include "config.h"

#include <cstdint>

/*******************************************************************************
 **                             SAddr Class
 ******************************************************************************/

/*
 * Represents a memory address. An address is a bitfield with the
 * following structure:
 *
 *     B64: 1 -> static, 0 -> dynamic
 *     B63: 1 -> automatic, 0 -> heap
 *     B62: 1 -> internal, 0 -> user
 *     B61: 1 -> durable, 0 -> volatile
 *     B0-B60: address
 */
class SAddr {

public:
	using Width = uintptr_t;

protected:
	static constexpr Width wordSize = 3;

	static constexpr Width staticMask = (Width)1 << 63;
	static constexpr Width automaticMask = (Width)1 << 62;
	static constexpr Width internalMask = (Width)1 << 61;
	static constexpr Width durableMask = (Width)1 << 60;
	static constexpr Width storageMask = staticMask | automaticMask | internalMask |
					     durableMask;
	static constexpr Width addressMask = durableMask - 1;

	static constexpr Width limit = addressMask;

	static auto create(Width storageMask, Width value, bool durable, bool internal) -> SAddr
	{
		BUG_ON(value >= SAddr::limit);
		Width fresh = 0;
		fresh |= storageMask;
		fresh ^= (-(unsigned long)(!!durable) ^ fresh) & durableMask;
		fresh ^= (-(unsigned long)(!!internal) ^ fresh) & internalMask;
		fresh |= value;
		return {fresh};
	}

public:
	SAddr() : addr(0) {}
	SAddr(Width addr) : addr(addr) {}
	SAddr(void *addr) : addr((Width)addr) {}

	/* Helper methods to create a new address */
	template <typename... Ts> static auto createStatic(Ts &&...params) -> SAddr
	{
		return create(staticMask, std::forward<Ts>(params)...);
	}
	template <typename... Ts> static auto createHeap(Ts &&...params) -> SAddr
	{
		return create(0, std::forward<Ts>(params)...);
	}
	template <typename... Ts> static auto createAutomatic(Ts &&...params) -> SAddr
	{
		return create(automaticMask, std::forward<Ts>(params)...);
	}

	/* Return information regarding the address */
	[[nodiscard]] auto isStatic() const -> bool { return addr & staticMask; }
	[[nodiscard]] auto isDynamic() const -> bool { return !isStatic(); }
	[[nodiscard]] auto isAutomatic() const -> bool { return addr & automaticMask; }
	[[nodiscard]] auto isHeap() const -> bool { return !isAutomatic(); }
	[[nodiscard]] auto isInternal() const -> bool { return addr & internalMask; }
	[[nodiscard]] auto isUser() const -> bool { return !isInternal(); }
	[[nodiscard]] auto isDurable() const -> bool { return addr & durableMask; }
	[[nodiscard]] auto isVolatile() const -> bool { return !isDurable(); }
	[[nodiscard]] auto isNull() const -> bool { return addr == 0; }

	/* Whether two addresses are on the same storage */
	[[nodiscard]] auto sameStorageAs(const SAddr &other) const -> bool
	{
		return (addr & storageMask) == (other.addr & storageMask);
	}

	/* Return an address aligned to the previous word boundary */
	[[nodiscard]] auto align() const -> SAddr
	{
		return ((Width)(addr >> wordSize) << wordSize);
	}

	[[nodiscard]] auto get() const -> Width { return addr; }

	inline auto operator<=>(const SAddr &other) const = default;

	auto operator+(const ASize &size) const -> SAddr
	{
		SAddr s(*this);
		s.addr += size.get();
		return s;
	};
	auto operator-(const ASize &size) const -> SAddr
	{
		SAddr s(*this);
		s.addr -= size.get();
		return s;
	};
	auto operator-(const SAddr &other) const -> Width { return this->get() - other.get(); };
	auto operator>>(unsigned int num) const -> SAddr
	{
		SAddr s(*this);
		s.addr >>= num;
		return s;
	}
	auto operator<<(unsigned int num) const -> SAddr
	{
		SAddr s(*this);
		s.addr <<= num;
		return s;
	}

	friend auto operator<<(llvm::raw_ostream &rhs, const SAddr &addr) -> llvm::raw_ostream &;

private:
	/* The actual address */
	Width addr;
};

namespace std {
template <> struct hash<SAddr> {
	auto operator()(const SAddr &addr) const -> std::size_t
	{
		using std::hash;
		return hash<SAddr::Width>()(addr.get());
	};
};
} // namespace std

#endif /* GENMC_SADDR_HPP */
