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

#include "genmc/ADT/Rc.hpp"
#include "genmc/ADT/SmallVector.hpp"
#include "genmc/ADT/VectorClock.hpp"
#include "genmc/Execution/Event.hpp"

#include <format>
#include <ranges>

class AdaptiveView;

namespace detail {
/**
 * ViewBase class implements basic view functionalities.
 */
class ViewBase : public genmc::Rc<ViewBase> {
public:
	/** Constructors */
	ViewBase() = default;
	ViewBase(const ViewBase &other) = default;
	ViewBase(ViewBase &&other) noexcept = default;

	/** Assignment operators */
	auto operator=(const ViewBase &other) -> ViewBase & = default;
	auto operator=(ViewBase &&other) noexcept -> ViewBase & = default;

	/** Destructor */
	~ViewBase() = default;

	/** Returns the size of this view (i.e., number of threads seen) */
	[[nodiscard]] auto size() const -> unsigned int { return view_.size(); }

	/** Returns true if this view is empty */
	[[nodiscard]] auto empty() const -> bool { return view_.empty(); }

	void clear() { view_.clear(); }

	/** Returns true if e is contained in the clock */
	[[nodiscard]] auto contains(Event e) const -> bool { return e.index <= getMax(e.thread); }

	/** Returns true if v is contained in the clock */
	[[nodiscard]] auto contains(const ViewBase &v) const -> bool
	{
		/* Fastpath: If V is larger, it implies it has seen more non-zero elements */
		if (v.size() > size())
			return false;

		const int *A = view_.data();
		const int *B = v.view_.data();
		size_t n = v.size();
		size_t i = 0;

		for (; i < n; i++) {
			if (B[i] > A[i])
				return false;
		}
		return true;
	}

	/** Updates the view based on another view */
	auto update(const ViewBase &v) -> ViewBase &;

	/** Makes the maximum event seen in e's thread equal to e */
	auto updateIdx(Event e) -> ViewBase &
	{
		if (getMax(e.thread) < e.index)
			setMax(e);
		return *this;
	}

	[[nodiscard]] auto getMax(int thread) const -> int
	{
		if (thread < (int)view_.size())
			return view_[thread];
		return 0;
	}

	/** Makes the maximum event seen in e's thread equal to e */
	void setMax(Event e)
	{
		VERIFY(e.thread >= 0);
		if (e.index == 0 && (size_t)e.thread >= view_.size())
			return;

		if ((size_t)e.thread >= view_.size())
			view_.resize(e.thread + 1, 0);
		view_[e.thread] = e.index;

		if (e.index == 0 && e.thread == (int)view_.size() - 1)
			trim();
	}

	auto formatData(std::format_context &ctx) const -> std::format_context::iterator;

	auto operator==(const ViewBase &rhs) const -> bool
	{
		if (size() != rhs.size())
			return false;
		return std::memcmp(view_.data(), rhs.view_.data(), size() * sizeof(int)) == 0;
	}

private:
	void trim()
	{
		while (!view_.empty() && view_.back() == 0)
			view_.pop_back();
	}

	static constexpr int SIMD_WIDTH = 8;
	using StorageT = genmc::SmallVector<int, SIMD_WIDTH>;
	StorageT view_;
};
} // namespace detail

/**
 * An instantiation of a vector clock where it is assumed that if an index
 * is contained in the clock, all of its po-predecessors are also contained
 * in the clock.
 */
class View : public VectorClock {
public:
	static auto classof(const VectorClock *vc) -> bool { return vc->getKind() == VC_View; }

	/** Constructors */
	View() : VectorClock(VectorClock::VectorClockKind::VC_View), base_(nullptr), diff_(0, 0) {}
	View(const View &rhs)
		: VectorClock(VectorClock::VectorClockKind::VC_View), base_(rhs.base_),
		  diff_(rhs.diff_)
	{}
	View(View &&rhs) noexcept
		: VectorClock(VectorClock::VectorClockKind::VC_View), base_(std::move(rhs.base_)),
		  diff_(rhs.diff_)
	{}

	/** Assignment operators */
	auto operator=(const View &rhs) -> View &
	{
		if (this == &rhs)
			return *this;
		base_ = rhs.base_;
		diff_ = rhs.diff_;
		return *this;
	}
	auto operator=(View &&rhs) noexcept -> View &
	{
		if (this == &rhs)
			return *this;
		base_ = std::move(rhs.base_);
		diff_ = rhs.diff_;
		return *this;
	}

	/** Destructor */
	~View() override = default;

	/** Returns the size of this view (i.e., number of threads seen) */
	[[nodiscard]] auto size() const -> unsigned int override
	{
		auto base_size = base_ ? base_->size() : 0;
		auto diff_size = diff_.isInitializer() ? 0 : diff_.thread + 1U;
		return std::max(base_size, diff_size);
	}

	/** Returns true if this view is empty */
	[[nodiscard]] auto empty() const -> bool
	{
		return diff_.index == 0 && (!base_ || base_->empty());
	}

	auto clear() -> void override
	{
		base_ = nullptr;
		diff_ = Event::getInit();
	}

	/** Returns true if e is contained in the clock */
	[[nodiscard]] auto contains(Event e) const -> bool override
	{
		return e.index <= getMax(e.thread);
	}

	/** Returns true if v is contained in the clock */
	[[nodiscard]] auto contains(const View &v) const -> bool;

	/** Returns true if es are all contained in the clock */
	template <std::ranges::input_range R> [[nodiscard]] auto contains(const R &es) const -> bool
	{
		return std::ranges::all_of(es, [this](auto &&e) { return contains(e); });
	}

	[[nodiscard]] auto contains(const AdaptiveView &me) const -> bool;

	/** Updates the maximum event seen in E.THREAD with e.INDEX */
	auto updateIdx(Event e) -> View & override;

	/** Calls updateIdx for each event in ES */
	template <std::ranges::input_range R> auto updateIdxs(const R &es) -> View &
	{
		for (auto &e : es)
			updateIdx(e);
		return *this;
	}

	/** Returns the maximum index seen for THREAD */
	[[nodiscard]] auto getMax(int thread) const -> int override;

	/** Sets the maximum index seen in E.THREAD to E.INDEX */
	void setMax(Event e) override;

	/** Updates the view based on another vector clock. We can
	 * only update the current view given another View (and not
	 * some other subclass of VectorClock) */
	auto update(const View &v) -> View & override;
	auto update(const DepView & /*dv*/) -> DepView & override { UNREACHABLE(); }
	auto update(const VectorClock &vc) -> VectorClock & override
	{
		if (const auto *v = genmc::dyn_cast<View>(&vc))
			return this->update(*v);
		UNREACHABLE();
	}

	auto operator==(const View &rhs) const -> bool
	{
		if (this == &rhs)
			return true;

		if (base_ == rhs.base_ && diff_ == rhs.diff_)
			return true;

		auto sz = std::max(size(), rhs.size());
		for (auto i = 0; i < (int)sz; i++) {
			if (getMax(i) != rhs.getMax(i))
				return false;
		}
		return true;
	}

	auto formatData(std::format_context &ctx) const -> std::format_context::iterator override;

private:
	friend class ExecutionGraph;

	/** Hydrates the view into a writable, full-blown view */
	void hydrate();

	genmc::intrusive_ptr<detail::ViewBase> base_;
	Event diff_;
};

template <> struct std::formatter<View> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const View &view, std::format_context &ctx) const
	{
		return view.formatData(ctx);
	}
};

#endif /* GENMC_VIEW_HPP */
