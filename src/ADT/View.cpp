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

#include "ADT/View.hpp"
#include "Support/Error.hpp"

auto View::size() const -> unsigned int { return view_.size(); }

auto View::empty() const -> bool { return size() == 0; }

auto View::contains(const Event e) const -> bool { return e.index <= getMax(e.thread); }

auto View::update(const View &v) -> View &
{
	if (v.empty())
		return *this;

	auto size = std::max(this->size(), v.size());
	for (auto i = 0U; i < size; i++)
		if (getMax(i) < v.getMax(i))
			setMax(Event(i, v.getMax(i)));
	return *this;
}

auto View::update(const DepView &v) -> DepView & { BUG(); }

auto View::update(const VectorClock &vc) -> VectorClock &
{
	if (const auto *v = genmc::dyn_cast<View>(&vc))
		return this->update(*v);
	BUG();
}

auto View::formatData(std::format_context &ctx) const -> std::format_context::iterator
{
	auto out = std::format_to(ctx.out(), "[ ");
	for (auto i = 0U; i < size(); i++)
		out = std::format_to(out, "{}: {}", i, getMax(i));
	return std::format_to(out, "]");
}
