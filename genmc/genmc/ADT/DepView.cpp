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

#include "genmc/ADT/DepView.hpp"
#include "genmc/Support/Error.hpp"

auto DepView::contains(const Event e) const -> bool
{
	return e.index <= getMax(e.thread) &&
	       (e.index == 0 || (e.thread < holes_.size() && !holes_[e.thread].count(e.index)));
}

void DepView::addHole(const Event e)
{
	VERIFY(e.index <= getMax(e.thread));
	holes_[e.thread].insert(e.index);
}

void DepView::addHolesInRange(Event start, int endIdx)
{
	for (auto i = start.index; i < endIdx; i++)
		addHole(Event(start.thread, i));
}

void DepView::removeHole(const Event e) { holes_[e.thread].erase(e.index); }

void DepView::removeAllHoles(int thread) { holes_[thread].clear(); }

void DepView::removeHolesInRange(Event start, int endIdx)
{
	for (auto i = start.index; i < endIdx; i++)
		removeHole(Event(start.thread, i));
}

auto DepView::update(const View &v) -> View & { UNREACHABLE(); }

auto DepView::update(const DepView &v) -> DepView &
{
	if (v.empty())
		return *this;

	for (auto i = 0U; i < v.size(); i++) {
		auto isec = holes_[i].intersectWith(v.holes_[i]);
		if (getMax(i) < v.getMax(i)) {
			isec.insert(std::lower_bound(v.holes_[i].begin(), v.holes_[i].end(),
						     getMax(i) + 1),
				    v.holes_[i].end());
			view_.setMax(Event(i, v.getMax(i)));
		} else {
			isec.insert(std::lower_bound(holes_[i].begin(), holes_[i].end(),
						     v.getMax(i) + 1),
				    holes_[i].end());
		}
		holes_[i] = std::move(isec);
	}
	return *this;
}

auto DepView::update(const VectorClock &vc) -> VectorClock &
{
	if (const auto *v = genmc::dyn_cast<DepView>(&vc))
		return this->update(*v);
	UNREACHABLE();
}

auto DepView::formatData(std::format_context &ctx) const -> std::format_context::iterator
{
	auto out = std::format_to(ctx.out(), "[\n");
	for (auto i = 0U; i < size(); i++) {
		out = std::format_to(out, "\t{}: {} ( ", i, getMax(i));
		for (const auto &h : this->holes_[i]) {
			out = std::format_to(out, "{} ", h);
		}
		out = std::format_to(out, ")\n");
	}
	return std::format_to(out, "]");
}
