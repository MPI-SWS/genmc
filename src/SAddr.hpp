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

#ifndef __SADDR_HPP__
#define __SADDR_HPP__

#include "config.h"
#include "Error.hpp"
#include "ASize.hpp"

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

	static constexpr Width staticMask = (Width) 1 << 63;
	static constexpr Width automaticMask = (Width) 1 << 62;
	static constexpr Width internalMask = (Width) 1 << 61;
	static constexpr Width durableMask = (Width) 1 << 60;
	static constexpr Width storageMask = staticMask | automaticMask | internalMask | durableMask;
	static constexpr Width addressMask = durableMask - 1;

	static constexpr Width limit = addressMask;

	static SAddr create(Width storageMask, Width value, bool durable, bool internal) {
		BUG_ON(value >= SAddr::limit);
		Width fresh = 0;
		fresh |= storageMask;
		fresh ^= (-(unsigned long)(!!durable) ^ fresh) & durableMask;
		fresh ^= (-(unsigned long)(!!internal) ^ fresh) & internalMask;
		fresh |= value;
		return SAddr(fresh);
	}

public:
	SAddr() : addr(0) {}
	SAddr(Width a) : addr(a) {}
	SAddr(void *a) : addr((Width) a) {}

	/* Helper methods to create a new address */
	template <typename... Ts>
	static SAddr createStatic(Ts&&... params) {
		return create(staticMask, std::forward<Ts>(params)...);
	}
	template <typename... Ts>
	static SAddr createHeap(Ts&&... params) {
		return create(0, std::forward<Ts>(params)...);
	}
	template <typename... Ts>
	static SAddr createAutomatic(Ts&&... params) {
		return create(automaticMask, std::forward<Ts>(params)...);
	}

	/* Return information regarding the address */
	bool isStatic() const { return addr & staticMask; }
	bool isDynamic() const { return !isStatic(); }
	bool isAutomatic() const { return addr & automaticMask; }
	bool isHeap() const { return !isAutomatic(); }
	bool isInternal() const { return addr & internalMask; }
	bool isUser() const { return !isInternal(); }
	bool isDurable() const { return addr & durableMask; }
	bool isVolatile() const { return !isDurable(); }
	bool isNull() const { return addr == 0; }

	/* Whether two addresses are on the same storage */
	bool sameStorageAs(const SAddr &a) const {
		return (addr & storageMask) == (a.addr & storageMask);
	}

	/* Return an address aligned to the previous word boundary */
	SAddr align() const {
		return ((Width) (addr >> wordSize) << wordSize);
	}

	Width get() const { return addr; }

	inline bool operator==(const SAddr &a) const {
		return a.addr == addr;
	}
	inline bool operator!=(const SAddr &a) const {
		return !(*this == a);
	}
	inline bool operator<=(const SAddr &a) const {
		return addr <= a.addr;
	}
	inline bool operator<(const SAddr &a) const {
		return addr < a.addr;
	}
	inline bool operator>=(const SAddr &a) const {
		return !(*this < a);
	}
	inline bool operator>(const SAddr &a) const {
		return !(*this <= a);
	}
	SAddr operator+(const ASize &size) const {
		SAddr s(*this);
		s.addr += size.get();
		return s;
	};
	SAddr operator-(const ASize &size) const {
		SAddr s(*this);
		s.addr -= size.get();
		return s;
	};
	Width operator-(const SAddr &a) const {
		return this->get() - a.get();
	};
	SAddr operator>>(unsigned int num) const {
		SAddr s(*this);
		s.addr >>= num;
		return s;
	}
	SAddr operator<<(unsigned int num) const {
		SAddr s(*this);
		s.addr <<= num;
		return s;
	}

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const SAddr &addr);

private:
	/* The actual address */
	Width addr;
};

namespace std {
	template<>
	struct hash<SAddr> {
		std::size_t operator()(const SAddr &a) const {
			using std::hash;
			return hash<SAddr::Width>()(a.get());
		};
	};
}

#endif /* __SADDR_HPP__ */
