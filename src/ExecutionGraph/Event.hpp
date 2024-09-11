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

#ifndef GENMC_EVENT_HPP
#define GENMC_EVENT_HPP

#include <llvm/ADT/Hashing.h>
#include <llvm/Support/raw_ostream.h>

#include <compare>

/**
 * Represents the position of a given label in an execution graph.
 */
struct Event {
	Event() : thread(DEF_IDX), index(DEF_IDX){};

	/** Constructs an event at the given position */
	Event(int tid, int idx) : thread(tid), index(idx){};

	/** Returns the INIT event */
	static auto getInit() -> Event { return {0, 0}; };

	/** Returns a BOTTOM event representing an invalid position */
	static auto getBottom() -> Event { return {BOT_IDX, BOT_IDX}; };

	/** Returns true is *this == INIT */
	[[nodiscard]] auto isInitializer() const -> bool { return *this == getInit(); };

	/** Returns true if *this == BOT */
	[[nodiscard]] auto isBottom() const -> bool { return *this == getBottom(); };

	/** Returns the po-predecessor. No bounds checking is performed. */
	[[nodiscard]] auto prev() const -> Event { return {thread, index - 1}; };

	/** Returns the po-successor. No bounds checking is performed  */
	[[nodiscard]] auto next() const -> Event { return {thread, index + 1}; };

	[[nodiscard]] inline auto operator==(const Event &) const -> bool = default;
	[[nodiscard]] inline auto operator<=>(const Event &other) const -> std::partial_ordering
	{
		return this->thread == other.thread ? this->index <=> other.index
						    : std::partial_ordering::unordered;
	}

	inline auto operator++() -> Event &
	{
		++index;
		return *this;
	}
	inline auto operator++(int) -> Event
	{
		auto tmp = *this;
		operator++();
		return tmp;
	}
	inline auto operator--() -> Event &
	{
		--index;
		return *this;
	}
	inline auto operator--(int) -> Event
	{
		auto tmp = *this;
		operator--();
		return tmp;
	}

	static auto hash_value(const Event &e) -> llvm::hash_code
	{
		return llvm::hash_combine(e.thread, e.index);
	};

	int thread;
	int index;

private:
	/* Default and bottom events should really be opaque.
	 * Try to throw out of bounds if used as index. */
	static constexpr int DEF_IDX = -17;
	static constexpr int BOT_IDX = -42;
};

auto operator<<(llvm::raw_ostream &s, Event e) -> llvm::raw_ostream &;

struct EventHasher {

	template <class T> inline void hash_combine(std::size_t &seed, const T &v) const
	{
		seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	auto operator()(const Event &e) const -> std::size_t
	{
		std::size_t hash = 0;
		hash_combine(hash, e.thread);
		hash_combine(hash, e.index);
		return hash;
	}
};

#endif /* GENMC_EVENT_HPP */
