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

#ifndef GENMC_ADAPTIVE_VIEW_HPP
#define GENMC_ADAPTIVE_VIEW_HPP

#include "genmc/ADT/SmallVector.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/Event.hpp"

#include <compare>
#include <optional>
#include <variant>

struct AdaptiveView {
public:
	static constexpr std::size_t SmallVectorCapacity = 8;
	using Events = genmc::SmallVector<Event, SmallVectorCapacity>;
	using Update = std::pair<Event, View>;
	using Empty = std::monostate;
	using StorageT = std::variant<Event, Events, View, Update, Empty>;

	class const_iterator {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = Event;
		using difference_type = std::ptrdiff_t;
		using pointer = const Event *;
		using reference = Event; /* by value */

		struct ArrowProxy {
			Event e;
			auto operator->() const -> const Event * { return &e; }
		};

		/* Ctors */
		constexpr const_iterator() = default;
		constexpr const_iterator(const AdaptiveView *v, std::size_t idx)
			: view_(v), idx_(idx)
		{}

		auto operator*() const -> reference { return view_->at(idx_); }
		auto operator->() const -> ArrowProxy { return ArrowProxy{view_->at(idx_)}; }

		auto operator++() -> const_iterator &
		{
			++idx_;
			return *this;
		}
		auto operator++(int) -> const_iterator
		{
			auto tmp = *this;
			++idx_;
			return tmp;
		}

		auto operator--() -> const_iterator &
		{
			--idx_;
			return *this;
		}
		auto operator--(int) -> const_iterator
		{
			auto tmp = *this;
			--idx_;
			return tmp;
		}

		auto operator+=(difference_type n) -> const_iterator &
		{
			/* Implicit wrap-around works for negative n */
			idx_ += static_cast<std::size_t>(n);
			return *this;
		}

		auto operator-=(difference_type n) -> const_iterator &
		{
			idx_ -= static_cast<std::size_t>(n);
			return *this;
		}

		friend auto operator+(const const_iterator &it, difference_type n) -> const_iterator
		{
			return {it.view_, it.idx_ + static_cast<std::size_t>(n)};
		}

		friend auto operator+(difference_type n, const const_iterator &it) -> const_iterator
		{
			return it + n;
		}

		friend auto operator-(const const_iterator &it, difference_type n) -> const_iterator
		{
			return {it.view_, it.idx_ - static_cast<std::size_t>(n)};
		}

		friend auto operator-(const const_iterator &lhs, const const_iterator &rhs)
			-> difference_type
		{
			/* Case before subtracting to avoid unsigned overflow */
			return static_cast<difference_type>(lhs.idx_) -
			       static_cast<difference_type>(rhs.idx_);
		}

		friend auto operator<=>(const const_iterator &, const const_iterator &) = default;

	private:
		const AdaptiveView *view_{};
		std::size_t idx_{};
	};

	[[nodiscard]] auto begin() const -> const_iterator { return {this, 0}; }
	[[nodiscard]] auto end() const -> const_iterator { return {this, size()}; }

	[[nodiscard]] auto empty() const -> bool
	{
		if (std::holds_alternative<Empty>(storage_))
			return true;
		if (const auto *e = std::get_if<Event>(&storage_))
			return e->isInitializer();
		if (const auto *upd = std::get_if<Update>(&storage_))
			return upd->first.isInitializer();
		if (const auto *es = std::get_if<Events>(&storage_))
			return std::ranges::all_of(*es,
						   [](const auto &e) { return e.isInitializer(); });
		if (const auto *v = std::get_if<View>(&storage_))
			return v->empty();
		UNREACHABLE();
	}

	[[nodiscard]] auto size() const -> std::size_t
	{
		if (std::holds_alternative<Empty>(storage_))
			return 0;
		if (std::holds_alternative<Event>(storage_))
			return 1;
		if (std::holds_alternative<Update>(storage_))
			return 1;
		if (const auto *es = std::get_if<Events>(&storage_))
			return es->size();
		if (const auto *v = std::get_if<View>(&storage_))
			return v->size();
		return 0;
	}

	/* Basic ctors/dtor */
	AdaptiveView() : storage_(Empty{}) {}
	AdaptiveView(const Event &e) : storage_{e} {}
	AdaptiveView(const Events &events) : storage_{events} {}
	AdaptiveView(Events &&events) : storage_{std::move(events)} {}
	AdaptiveView(const View &v) : storage_{v} {}
	AdaptiveView(View &&v) : storage_{std::move(v)} {}
	~AdaptiveView() = default;

	/* Update ctors */
	AdaptiveView(Event pos, const View &v) : storage_{Update{pos, v}} {}
	AdaptiveView(Event pos, View &&v) : storage_{Update{pos, std::move(v)}} {}

	/* Copy move */
	AdaptiveView(const AdaptiveView &rhs) = default;
	AdaptiveView(AdaptiveView &&rhs) noexcept = default;
	auto operator=(const AdaptiveView &rhs) -> AdaptiveView & = default;
	auto operator=(AdaptiveView &&rhs) noexcept -> AdaptiveView & = default;

	/** Joins two adaptive views (if LHS is Update, it is treated as Event) */
	auto join(const AdaptiveView &rhs) -> AdaptiveView &;

	/** Returns true if THIS is contained in v */
	[[nodiscard]] auto containedIn(const View &v) const -> bool;

	/** Updates THIS with another AdaptiveView, which should be Empty or Update */
	auto operator+=(const AdaptiveView &rhs) -> AdaptiveView &;

	auto operator==(const AdaptiveView &rhs) const -> bool
	{
		/* Fastpath: same-type variants */
		if (storage_.index() == rhs.storage_.index() && storage_ == rhs.storage_)
			return true;

		/* Do they hold a single event? */
		auto lhsE = getAsEvent();
		auto rhsE = rhs.getAsEvent();
		return lhsE && rhsE && *rhsE == *lhsE;
	}

private:
	[[nodiscard]] auto getAsEvent() const -> std::optional<Event>
	{
		if (const auto *e = std::get_if<Event>(&storage_))
			return *e;
		if (const auto *upd = std::get_if<Update>(&storage_))
			return upd->first;
		if (const auto *es = std::get_if<Events>(&storage_))
			return es->size() == 1 ? std::make_optional(es->front()) : std::nullopt;
		return std::nullopt;
	}

	[[nodiscard]] auto at(std::size_t idx) const -> Event
	{
		if (const auto *e = std::get_if<Event>(&storage_))
			return *e;
		if (const auto *upd = std::get_if<Update>(&storage_))
			return upd->first;
		if (const auto *es = std::get_if<Events>(&storage_))
			return (*es)[idx];
		if (const auto *v = std::get_if<View>(&storage_))
			return {(int)idx, v->getMax((int)idx)};
		UNREACHABLE();
	}

	StorageT storage_;
};

#endif /* GENMC_ADAPTIVE_VIEW_HPP */
