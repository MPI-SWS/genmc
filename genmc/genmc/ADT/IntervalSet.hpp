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

#ifndef GENMC_INTERVAL_SET_HPP
#define GENMC_INTERVAL_SET_HPP

#include "genmc/ADT/IntervalMap.hpp"

#include <format>

namespace genmc {

/**
 * A wrapper around bool that provides monoid semantics (OR-aggregation)
 * for use with IntervalMap.
 */
struct MonoidBool {
	bool val = false;

	MonoidBool() = default;
	MonoidBool(bool b) : val(b) {}

	auto operator+=(const MonoidBool &rhs) -> MonoidBool &
	{
		val = val || rhs.val;
		return *this;
	}

	auto operator==(const MonoidBool &rhs) const -> bool = default;
	explicit operator bool() const { return val; }
};

/**
 * An interval set implementation based on IntervalMap.
 * Represents a union of right-open intervals [start, end).
 */
template <IKey Key> class IntervalSet {
private:
	IntervalMap<Key, MonoidBool> map_;

public:
	IntervalSet() : map_(false) {}

	/** A thin iterator that yields the intervals in the set. */
	class Iterator {
	public:
		using MapIterator = typename IntervalMap<Key, MonoidBool>::Iterator;
		using iterator_category = std::forward_iterator_tag;
		using value_type = Interval<Key>;
		using difference_type = std::ptrdiff_t;
		using pointer = Interval<Key> *;
		using reference = Interval<Key>;

		Iterator(MapIterator it, MapIterator end) : currIt(it), endIt(end) {}

		auto operator*() const -> Interval<Key> { return (*currIt).first; }

		auto operator++() -> Iterator &
		{
			++currIt;
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
		MapIterator currIt;
		MapIterator endIt;
	};

	auto begin() const -> Iterator { return {map_.begin(), map_.end()}; }
	auto end() const -> Iterator { return {map_.end(), map_.end()}; }

	/** Returns whether POINT is in the set */
	[[nodiscard]] auto contains(const Key &point) const -> bool { return map_.contains(point); }

	/** Returns whether the entire interval is in the set */
	[[nodiscard]] auto contains(const Interval<Key> &iv) const -> bool
	{
		return map_.contains(iv);
	}

	/** Returns whether the set overlaps with [start, end) */
	[[nodiscard]] auto intersects(Key start, Key end) const -> bool
	{
		return map_.lower_bound(start, end) != map_.end();
	}

	[[nodiscard]] auto overlaps(const Interval<Key> &iv) const -> bool
	{
		return map_.lower_bound(iv) != map_.end();
	}

	/** Returns whether the set is empty */
	[[nodiscard]] auto empty() const -> bool { return map_.empty(); }

	/** Clears the set */
	void clear() { map_.clear(); }

	/** Inserts an interval into the set */
	void add(const Interval<Key> &iv) { map_.add(iv, true); }
	void add(Key start, Key end) { map_.add(start, end, true); }

	/** Removes an interval from the set */
	void subtract(const Interval<Key> &iv) { map_.erase(iv); }
	void subtract(Key start, Key end) { map_.erase(start, end); }

	/** Standard API aliases */
	void insert(Key start, Key end) { add(start, end); }
	void insert(const Interval<Key> &iv) { add(iv); }
	void erase(Key start, Key end) { subtract(start, end); }
	void erase(const Interval<Key> &iv) { subtract(iv); }

	auto operator<=>(const IntervalSet &) const = default;

#ifdef ENABLE_GENMC_DEBUG
	[[nodiscard]] auto validate() const -> std::optional<std::string>
	{
		return map_.validate();
	}
#endif
};

/** Compatibility helper for boost::icl::is_empty */
template <typename T> inline auto is_empty(const IntervalSet<T> &set) -> bool
{
	return set.empty();
}

} /* namespace genmc */

/** Make `IntervalSet` formattable with `std::format`. */
template <typename Key> struct std::formatter<genmc::IntervalSet<Key>> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const genmc::IntervalSet<Key> &set, std::format_context &ctx) const
	{
		auto out = std::format_to(ctx.out(), "{{ ");
		for (const auto &interval : set) {
			out = std::format_to(out, "{} ", interval);
		}
		return std::format_to(out, " }}");
	}
};

#endif /* GENMC_INTERVAL_SET_HPP */
