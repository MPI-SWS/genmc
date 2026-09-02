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

#include "genmc/ADT/View.hpp"
#include "genmc/ADT/AdaptiveView.hpp"
#include "genmc/ADT/Rc.hpp"
#include "genmc/Support/Error.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <utility>

/*******************************************************************************
 **                          ViewBase Class
 ******************************************************************************/

namespace detail {

auto ViewBase::update(const ViewBase &v) -> ViewBase &
{
	if (v.empty())
		return *this;

	if (std::cmp_less(view_.size(), v.size()))
		view_.resize(v.size(), 0);

	int *__restrict dst = view_.data();
	const int *__restrict src = v.view_.data();
	const size_t size = v.size();
	size_t i = 0;

	/* The compiler should vectorize this loop */
	for (; i < size; ++i)
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		dst[i] = std::max(dst[i], src[i]);
	return *this;
}

auto ViewBase::formatData(std::format_context &ctx) const -> std::format_context::iterator
{
	auto out = std::format_to(ctx.out(), "[ ");
	for (auto i = 0; std::cmp_less(i, size()); i++)
		out = std::format_to(out, "{}: {}", i, getMax(i));
	return std::format_to(out, "]");
}
} // namespace detail

/*******************************************************************************
 **                         View Class
 ******************************************************************************/

auto View::contains(const View &v) const -> bool
{
	if (this == &v)
		return true;

	/* First, check the diffs */
	if (!v.diff_.isInitializer() && v.diff_.index > getMax(v.diff_.thread))
		return false;

	/* Then, check whether we have to check bases as well */
	if (!v.base_ || base_ == v.base_)
		return true;
	if (!base_)
		return v.base_->empty();
	return base_->contains(*v.base_); /* Tough luck */
}

auto View::contains(const AdaptiveView &me) const -> bool { return me.containedIn(*this); }

auto View::updateIdx(Event e) -> View &
{
	/* Fastpath: 0 is always contained; avoid hydration if event is seen */
	if (contains(e))
		return *this;

	/* Fastpath: If we are the unique owner, update directly */
	if (base_ && base_.use_count() == 1) {
		ASSERT(diff_.isInitializer());
		base_->updateIdx(e);
		return *this;
	}

	/* Fastpath: If diff is empty, set to e */
	if (diff_.isInitializer()) {
		diff_ = e;
		return *this;
	}

	/* Fastpath: If diff has the same thread, update with e */
	if (e.thread == diff_.thread) {
		diff_ = std::max(diff_, e);
		return *this;
	}

	hydrate();
	base_->updateIdx(e);
	return *this;
}

auto View::getMax(int thread) const -> int
{
	auto base_max = base_ ? base_->getMax(thread) : 0;
	if (!diff_.isInitializer() && diff_.thread == thread)
		return std::max(base_max, diff_.index);
	return base_max;
}

auto View::setMax(Event e) -> void
{
	/* Fastpath: If we own the view, set directly */
	if (base_ && base_.use_count() == 1) {
		ASSERT(diff_.isInitializer());
		base_->setMax(e);
		return;
	}

	/* Fastpath: If base does not contain e, and diff has the same thread, set diff to e */
	if (!base_ || !base_->contains(e)) {
		if (diff_.isInitializer() || e.thread == diff_.thread) {
			diff_ = e.index == 0 ? Event::getInit() : e;
			return;
		}
	}

	hydrate();
	base_->setMax(e);
}

void View::hydrate()
{
	base_ = base_ ? genmc::make_intrusive<detail::ViewBase>(*base_)
		      : genmc::make_intrusive<detail::ViewBase>();
	if (!diff_.isInitializer()) {
		base_->updateIdx(diff_);
		diff_ = Event::getInit();
	}
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto View::update(const View &v) -> View &
{
	/* Helper function to merge two views */
	auto merge = [this](auto &other) {
		if (other.base_ && base_ != other.base_)
			base_->update(*other.base_);
		if (!other.diff_.isInitializer())
			base_->updateIdx(other.diff_);
	};

	/* Fastpath: If we are hydrated already, merge V */
	if (base_ && base_.use_count() == 1) {
		ASSERT(diff_.isInitializer());
		merge(v);
		return *this;
	}

	/* Fastpath: If THIS is (non-hydrated) empty, copy from V */
	if (!base_ && diff_.isInitializer()) {
		base_ = v.base_;
		diff_ = v.diff_;
		return *this;
	}

	/* Fastpath: If bases are compatible, try to skip hydration */
	if (!base_ || !v.base_ || base_ == v.base_) {
		if (!base_)
			base_ = v.base_;

		/* Case 1: my diff is empty */
		if (diff_.isInitializer()) {
			if (base_ && !base_->contains(v.diff_))
				diff_ = v.diff_;
			return *this;
		}
		/* Case 2: other diff is empty */
		if (v.diff_.isInitializer()) {
			if (base_ && base_->contains(diff_))
				diff_ = Event::getInit();
			return *this;
		}
		/* Case 3: same thread */
		if (diff_.thread == v.diff_.thread) {
			diff_ = std::max(diff_, v.diff_);
			return *this;
		}
		/* Case 4: optimization failed, got to hydrate */
	}

	// TODO: potentially slow test, check whether this is helpful
	if (contains(v))
		return *this;

	hydrate();
	merge(v);
	return *this;
}

auto View::formatData(std::format_context &ctx) const -> std::format_context::iterator
{
	std::format_context::iterator out = ctx.out();
#ifdef ENABLE_GENMC_DEBUG
	out = std::format_to(out, "<base={}", static_cast<void *>(base_.get()));
	if (!diff_.isInitializer())
		out = std::format_to(out, ", diff={}", diff_);
	if (base_.use_count() == 1)
		out = std::format_to(out, ", W");
	else
		out = std::format_to(out, ", R");
	out = std::format_to(out, "> ");
#endif
	out = std::format_to(out, "[ ");
	for (auto i = 0; std::cmp_less(i, size()); i++)
		out = std::format_to(out, "{}:{} ", i, getMax(i));
	return std::format_to(out, "]");
}
