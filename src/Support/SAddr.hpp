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

#ifndef GENMC_SADDR_HPP
#define GENMC_SADDR_HPP

#include "ASize.hpp"
#include "Error.hpp"

#include <cstdint>
#include <format>

/*******************************************************************************
 **                             SAddr Class
 ******************************************************************************/

/**
 * Represents a memory address. An address is a bitfield with the
 * following structure:
 *
 *     B63: 1 -> static, 0 -> dynamic
 *     B62: 1 -> automatic, 0 -> heap
 *     B61: 1 -> internal, 0 -> user
 *     B60: 1 -> durable, 0 -> volatile
 *     B32-59: thread id
 *     B0-B31: thread allocation index
 */
class SAddr {

public:
	using Width = uint64_t;

	static constexpr Width staticMask = (Width)1 << 63;
	static constexpr Width automaticMask = (Width)1 << 62;
	static constexpr Width internalMask = (Width)1 << 61;
	static constexpr Width durableMask = (Width)1 << 60;
	static constexpr Width storageMask = staticMask | automaticMask | internalMask |
					     durableMask;

	static constexpr Width threadStartBit = 32;
	static constexpr Width indexMask = ((Width)1 << threadStartBit) - 1;
	static constexpr Width threadMask = (durableMask - ((Width)1 << threadStartBit));

	static constexpr Width threadLimit = threadMask >> threadStartBit;
	static constexpr Width allocLimit = indexMask;

protected:
	static auto create(Width storageMask, Width thread, Width index, bool durable,
			   bool internal) -> SAddr
	{
		BUG_ON(thread > SAddr::threadLimit);
		BUG_ON(index > SAddr::allocLimit);
		Width fresh = 0;
		fresh |= storageMask;
		fresh ^= (-(Width)(!!durable) ^ fresh) & durableMask;
		fresh ^= (-(Width)(!!internal) ^ fresh) & internalMask;
		fresh |= (thread << threadStartBit);
		fresh |= (index);
		return {fresh};
	}

public:
	SAddr() : addr(0) {}
	SAddr(Width addr) : addr(addr) {}
	SAddr(void *addr) : addr((Width)addr) {}

	/** Helper methods to create a new address */
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

	/** Return information regarding the address */
	[[nodiscard]] auto isStatic() const -> bool { return addr & staticMask; }
	[[nodiscard]] auto isDynamic() const -> bool { return !isStatic(); }
	[[nodiscard]] auto isAutomatic() const -> bool { return addr & automaticMask; }
	[[nodiscard]] auto isHeap() const -> bool { return !isAutomatic(); }
	[[nodiscard]] auto isInternal() const -> bool { return addr & internalMask; }
	[[nodiscard]] auto isUser() const -> bool { return !isInternal(); }
	[[nodiscard]] auto isDurable() const -> bool { return addr & durableMask; }
	[[nodiscard]] auto isVolatile() const -> bool { return !isDurable(); }
	[[nodiscard]] auto isNull() const -> bool { return addr == 0; }

	/** Whether two addresses are on the same storage */
	[[nodiscard]] auto sameStorageAs(const SAddr &other) const -> bool
	{
		return (addr & storageMask) == (other.addr & storageMask);
	}

	[[nodiscard]] auto get() const -> Width { return addr; }

	auto operator<=>(const SAddr &other) const = default;

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

private:
	/** The actual address */
	Width addr;
};

/** Make `SAddr` formattable with `std::format`. */
template <> struct std::formatter<SAddr> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const SAddr &addr, std::format_context &ctx) const
	{
		auto internal = addr.isInternal() ? "I" : "";

		std::string_view prefix;
		if (addr.isStatic())
			prefix = "G";
		else if (addr.isAutomatic())
			prefix = "S";
		else if (addr.isHeap())
			prefix = "H";
		else
			BUG();

		return std::format_to(ctx.out(), "{}{}#({}, {})", prefix, internal,
				      ((addr.get() & SAddr::threadMask) >> SAddr::threadStartBit),
				      (addr.get() & SAddr::indexMask));
	}
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
