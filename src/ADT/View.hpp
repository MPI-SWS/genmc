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

#ifndef GENMC_VIEW_HPP
#define GENMC_VIEW_HPP

#include "ADT/IndexedMap.hpp"
#include "ADT/VectorClock.hpp"
#include "ExecutionGraph/Event.hpp"

#include <format>

/**
 * An instantiation of a vector clock where it is assumed that if an index
 * is contained in the clock, all of its po-predecessors are also contained
 * in the clock.
 */
class View : public VectorClock {
private:
	using EventView = genmc::IndexedMap<int>;
	EventView view_;

public:
	/** Constructors */
	View() : VectorClock(VectorClock::VectorClockKind::VC_View), view_(EventView(0)) {}

	/** Iterators */
	using iterator = int *;
	using const_iterator = const int *;

	auto begin() -> iterator { return &view_[0]; };
	auto end() -> iterator { return &view_[0] + size(); }
	[[nodiscard]] auto begin() const -> const_iterator { return empty() ? nullptr : &view_[0]; }
	[[nodiscard]] auto end() const -> const_iterator
	{
		return empty() ? nullptr : &view_[0] + size();
	}

	/** Returns the size of this view (i.e., number of threads seen) */
	[[nodiscard]] auto size() const -> unsigned int override;

	/** Returns true if this view is empty */
	[[nodiscard]] auto empty() const -> bool;

	void clear() override { view_.clear(); }

	/** Returns true if e is contained in the clock */
	[[nodiscard]] auto contains(Event e) const -> bool override;

	/** Updates the view based on another vector clock. We can
	 * only update the current view given another View (and not
	 * some other subclass of VectorClock) */
	auto update(const View &v) -> View & override;
	auto update(const DepView &dv) -> DepView & override;
	auto update(const VectorClock &vc) -> VectorClock & override;

	/** Makes the maximum event seen in e's thread equal to e */
	auto updateIdx(Event e) -> View & override
	{
		if (getMax(e.thread) < e.index)
			setMax(e);
		return *this;
	}

	[[nodiscard]] auto getMax(int thread) const -> int override
	{
		if (thread < (int)view_.size())
			return view_[thread];
		return 0;
	}

	void setMax(Event e) override
	{
		if (e.thread >= (int)view_.size())
			view_.grow(e.thread);
		view_[e.thread] = e.index;
	}

	auto formatData(std::format_context &ctx) const -> std::format_context::iterator override;

	static auto classof(const VectorClock *vc) -> bool { return vc->getKind() == VC_View; }
};

#endif /* GENMC_VIEW_HPP */
