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

#include "genmc/Support/Hash.hpp"

#include <format>

/**
 * Represents the position of a given label in an execution graph.
 *
 * An event is a pair of integers, representing thread ID and index.
 * Although these are non-negative for valid events, we used signed ints
 * because an index of -1 is often a legitimate choice ("start of time"),
 * whereas unsigned would require using UINT_MAX or similar. (prev() not
 * wrapping and int being SIMD-friendly are two other reasons.)
 *
 * The default constructor exists for container compatibility (e.g.,
 * std::unordered_map::operator[]). It sets both fields to DEF_IDX,
 * which crashes on array-index misuse but is never equal to a real event.
 * For "no event" semantics at API boundaries, use std::optional<Event>.
 */
struct Event {
	Event() = default;

	/** Constructs an event at the given position */
	Event(int tid, int idx) : thread(tid), index(idx) {};

	/** Returns the INIT event */
	static auto getInit() -> Event { return {0, 0}; };

	/** Returns true if *this == INIT */
	[[nodiscard]] auto isInitializer() const -> bool { return *this == getInit(); };

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

	int thread = DEF_IDX;
	int index = DEF_IDX;

private:
	static constexpr int DEF_IDX = -17;
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
