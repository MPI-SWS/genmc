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

#ifndef GENMC_INTERVAL_MAP_HPP
#define GENMC_INTERVAL_MAP_HPP

#include "genmc/Support/Error.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <map>
#include <optional>

namespace genmc {

template <typename T>
concept IKey =
	std::strict_weak_order<std::less<T>, T, T> && std::copyable<T> && requires(T a, T b) {
		{ b - a };
	};

template <typename T> constexpr auto key_max()
{
	if constexpr (requires { T::max(); })
		return T::max();
	else
		return std::numeric_limits<T>::max();
}

template <typename T>
concept IVal = std::equality_comparable<T> && std::copyable<T> && requires(T lhs, const T &rhs) {
	{ lhs += rhs };
};

/**
 * Defines a helper interval class for interval map methods.
 * The intervals are assumed to be right-open by default.
 */
template <IKey Key> struct Interval {
	constexpr Interval(Key s, Key e) : start(std::move(s)), end(std::move(e)) {}

	/** Returns true if the interval is valid (star < end) and non-empty */
	[[nodiscard]] constexpr auto is_valid() const -> bool { return start < end; }

	auto operator<=>(const Interval &) const = default;

	/** Intersection support */
	auto operator&(const Interval &other) const -> Interval
	{
		Key res_start = std::max(start, other.start);
		Key res_end = std::min(end, other.end);
		return {res_start, std::max(res_start, res_end)};
	}

	Key start;
	Key end;
};

/* ADL helper functions */
template <typename Key> auto first(const Interval<Key> &iv) -> const Key & { return iv.start; }
template <typename Key> auto length(const Interval<Key> &iv) { return iv.end - iv.start; }
template <typename Key> auto lower(const Interval<Key> &iv) -> const Key & { return iv.start; }
template <typename Key> auto upper(const Interval<Key> &iv) -> const Key & { return iv.end; }

/**
 * An interval map implementation with aggregating behavior on overlapping intervals.
 *
 * Notes:
 *   - The intervals mapped are assumed to be right-open, e.g., [6,7).
 *   - Values should provide an identity element (default ctor) and an operator+= (aggregation)
 */
template <IKey Key, IVal Val> class IntervalMap {
public:
	/* Helper struct for the class iterator */
	struct Entry {
		Interval<Key> first;
		const Val &second;
	};

	class Iterator {
	public:
		using MapType = std::map<Key, Val>;
		using MapIterator = typename std::map<Key, Val>::const_iterator;
		using iterator_category = std::forward_iterator_tag;
		using value_type = Entry;
		using difference_type = std::ptrdiff_t;
		using pointer = Entry *;
		using reference = Entry;

		struct ArrowProxy {
			Entry e;
			auto operator->() -> Entry * { return &e; }
		};

		Iterator() : currIt(), endIt(), id_(nullptr) {}
		Iterator(MapIterator it, MapIterator endIt, const Val &identity)
			: currIt(it), endIt(endIt), id_(&identity)
		{
			skipIdentity();
		}

		auto operator*() const -> Entry
		{
			const auto &s = currIt->first;
			const auto &v = currIt->second;

			auto next = std::next(currIt);
			Key e = (next == endIt) ? key_max<Key>() : next->first;
			return {Interval<Key>(s, e), v};
		}
		auto operator->() const -> ArrowProxy { return {**this}; }

		auto operator++() -> Iterator &
		{
			++currIt;
			skipIdentity();
			return *this;
		}
		auto operator++(int) -> Iterator
		{
			auto tmp = *this;
			++(*this);
			return tmp;
		}
		auto operator==(const Iterator &other) const -> bool = default;

	private:
		void skipIdentity()
		{
			while (currIt != endIt && currIt->second == *id_)
				++currIt;
		}

		MapIterator currIt;
		MapIterator endIt;
		const Val *id_;
	};

	/* Ctors/dtor */
	explicit IntervalMap(Val identity = Val()) : id_(identity) {}
	IntervalMap(const IntervalMap &) = default;
	IntervalMap(IntervalMap &&) noexcept = default;
	~IntervalMap() = default;

	auto operator=(const IntervalMap &) -> IntervalMap & = default;
	auto operator=(IntervalMap &&) noexcept -> IntervalMap & = default;

	auto begin() const -> Iterator { return {map_.begin(), map_.end(), id_}; }
	auto end() const -> Iterator { return {map_.end(), map_.end(), id_}; }

	/** Returns whether the map is empty */
	[[nodiscard]] auto empty() const -> bool { return begin() == end(); }

	/** Clears the map */
	void clear() { map_.clear(); }

	/** Returns a reference to the underlying data-structure (map) */
	auto data() const -> const std::map<Key, Val> & { return map_; }

	/** Returns an iterator to the first segment that overlaps or follows [start, end).
	 * If no such segment exists, returns end() */
	auto lower_bound(Key start, Key end) const -> Iterator
	{
		if (start >= end)
			return this->end();

		auto it = map_.upper_bound(start);
		if (it != map_.begin())
			it = std::prev(it);

		/* Find the first segment [a,b) such that b > start */
		while (it != map_.end()) {
			auto next = std::next(it);
			Key b = (next == map_.end()) ? key_max<Key>() : next->first;
			if (b > start)
				break;
			++it;
		}

		return Iterator(it, map_.end(), id_);
	}

	auto lower_bound(const Interval<Key> &iv) const -> Iterator
	{
		return lower_bound(iv.start, iv.end);
	}

	/** Returns an iterator to the first segment strictly after [start, end).
	 * If no such segment exists, returns end() */
	auto upper_bound(Key start, Key end) const -> Iterator
	{
		if (start >= end)
			return this->end();

		auto it = map_.lower_bound(end);
		return Iterator(it, map_.end(), id_);
	}

	auto upper_bound(const Interval<Key> &iv) const -> Iterator
	{
		return upper_bound(iv.start, iv.end);
	}

	/** Returns an iterator to the interval containing POINT.
	 * If no valid interval covers the point, returns end(). */
	auto find(const Key &point) const -> Iterator
	{
		auto it = find_internal(point);
		if (it == map_.end())
			return end();
		return Iterator(it, map_.end(), id_);
	}

	auto find(const Interval<Key> &iv) const -> Iterator
	{
		auto it = find(iv.start);
		/* find(interval) only returns if it fully contains the interval */
		if (it != end()) {
			auto entry = *it;
			if (entry.first.end >= iv.end)
				return it;
		}
		return end();
	}

	/** Returns true if POINT is covered by a non-identity interval. */
	[[nodiscard]] auto contains(const Key &point) const -> bool
	{
		return find_internal(point) != map_.end();
	}

	auto contains(const Interval<Key> &iv) const -> bool { return find(iv) != end(); }

	/** Adds a (right-open) interval to the map */
	void add(Key start, Key end, const Val &val)
	{
		/* Fastpath: invalid interval or identity value */
		if (start >= end || val == id_)
			return;

		/* Iterate and update: O(K), where K is the number of conflicts */
		auto it = create_split(start);
		while (it != map_.end() && it->first < end) {
			auto nextIt = std::next(it);

			/* If the current segment extends beyond END, split.
			 * nextIt is the correct hint for insertion because it is
			 * strictly greater than end (or map.end()) */
			if (nextIt == map_.end() || nextIt->first > end)
				nextIt = map_.emplace_hint(nextIt, end, it->second);

			it->second += val;

			/* We have to check whether the update made this interval
			 * equal to the previous one (occurs for non-injective types) */
			canonicalize(it);
			it = nextIt;
		}

		/* We still have to clean up at the end */
		VERIFY(it != map_.end() && it->first == end);
		canonicalize(it);
	}

	void add(const Interval<Key> &iv, const Val &val) { add(iv.start, iv.end, val); }

	void add(const std::pair<Interval<Key>, Val> &p) { add(p.first, p.second); }

	/** Erases a (right-open) interval from the map (sets to identity) */
	void erase(Key start, Key end)
	{
		if (start >= end)
			return;

		auto it = create_split(start);
		while (it != map_.end() && it->first < end) {
			auto nextIt = std::next(it);

			if (nextIt == map_.end() || nextIt->first > end)
				nextIt = map_.emplace_hint(nextIt, end, it->second);

			it->second = id_;
			canonicalize(it);
			it = nextIt;
		}

		VERIFY(it != map_.end() && it->first == end);
		canonicalize(it);
	}

	void erase(const Interval<Key> &iv) { erase(iv.start, iv.end); }

	/** Returns the value associated with the inteval in which POINT belongs to.
	 *  If no such interval exists, returns the identity element */
	auto operator[](const Key &point) const -> const Val &
	{
		auto it = find_internal(point);
		return it == map_.end() ? id_ : it->second;
	}

	auto operator<=>(const IntervalMap &) const = default;

#ifdef ENABLE_GENMC_DEBUG
	[[nodiscard]] auto validate() const -> std::optional<std::string>
	{
		if (empty())
			return {};

		auto it = begin();
		if (it->second == id_)
			return std::format("Map starts with identity value");

		auto itPrev = it;
		it++;
		while (it != end()) {
			auto prev = *itPrev;
			auto curr = *it;

			if (prev.first.end > curr.first.start)
				return std::format("Overlap; [{}, {}) vs [{}, {})",
						   prev.first.start, prev.first.end,
						   curr.first.start, curr.first.end);
			if (prev.first.end == curr.first.start && prev.second == curr.second)
				return std::format(
					"Same-value adjacent intervals: [{}, {}) vs [{}, {})",
					prev.first.start, prev.first.end, curr.first.start,
					curr.first.end);
			itPrev = it;
			it++;
		}
		return {};
	}
#endif

private:
	/** Returns an (internal) iterator pointing to KEY (if exists).
	 * If it doesn't exist (it's part of an interval), creates a new split at
	 * KEY with the value of the existing interval */
	auto create_split(const Key &point) -> std::map<Key, Val>::iterator
	{
		auto lb = map_.lower_bound(point);
		if (lb != map_.end() && lb->first == point)
			return lb;

		auto prevVal = id_;
		if (lb != map_.begin())
			prevVal = std::prev(lb)->second;
		return map_.emplace_hint(lb, point, prevVal);
	}

	/** Canonicalizes the (internal) map by removing IT if it either
	 *  points to identity, or if prev(IT)->val == IT->val (merging
	 *  adjacent intervals) */
	void canonicalize(std::map<Key, Val>::iterator it)
	{
		if (it == map_.end())
			return;
		if (it == map_.begin()) {
			if (it->second == id_)
				map_.erase(it);
		} else {
			auto prev = std::prev(it);
			if (prev->second == it->second)
				map_.erase(it);
		}
	}

	/** Core lookup logic used by find(), contains(), and operator[].
	 * Returns the internal map iterator pointing to the segment covering POINT,
	 * or map_.end() if the point is empty/identity.
	 */
	auto find_internal(const Key &point) const -> std::map<Key, Val>::const_iterator
	{
		auto it = map_.upper_bound(point);
		if (it != map_.begin()) {
			auto prev = std::prev(it);
			if (prev->second != id_)
				return prev;
		}
		return map_.end();
	}

	std::map<Key, Val> map_;
	Val id_;
};

}; /* namespace genmc */

/** Make `Interval` formattable with `std::format`. */
template <typename Key> struct std::formatter<genmc::Interval<Key>> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const genmc::Interval<Key> &iv, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "[{}, {})", iv.start, iv.end);
	}
};

#endif /* GENMC_INTERVAL_MAP_HPP */
