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

#ifndef GENMC_EVENT_HPP
#define GENMC_EVENT_HPP

#include "Support/Hash.hpp"

#include <format>

/**
 * Represents the position of a given label in an execution graph.
 */
struct Event {
	Event() : thread(DEF_IDX), index(DEF_IDX) {};

	/** Constructs an event at the given position */
	Event(int tid, int idx) : thread(tid), index(idx) {};

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

	[[nodiscard]] auto operator<=>(const Event &other) const = default;

	auto operator++() -> Event &
	{
		++index;
		return *this;
	}
	auto operator++(int) -> Event
	{
		auto tmp = *this;
		operator++();
		return tmp;
	}
	auto operator--() -> Event &
	{
		--index;
		return *this;
	}
	auto operator--(int) -> Event
	{
		auto tmp = *this;
		operator--();
		return tmp;
	}

	int thread;
	int index;

private:
	/* Default and bottom events should really be opaque.
	 * Try to throw out of bounds if used as index. */
	static constexpr int DEF_IDX = -17;
	static constexpr int BOT_IDX = -42;
};

using Edge = std::pair<Event, Event>;

namespace std {
template <> struct hash<Event> {
	auto operator()(const Event &e) const -> size_t
	{
		std::size_t hash = 0;
		hash_combine(hash, e.thread);
		hash_combine(hash, e.index);
		return hash;
	}
};
} // namespace std

/** Make `Event` formattable with `std::format`. */
template <> struct std::formatter<Event> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Event &e, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "({}, {})", e.thread, e.index);
	}
};

#endif /* GENMC_EVENT_HPP */
