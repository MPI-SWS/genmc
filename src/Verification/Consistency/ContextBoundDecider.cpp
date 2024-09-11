/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Iason Marmanis <imarmanis@mpi-sws.org>
 */

#include "ContextBoundDecider.hpp"
#include "ADT/View.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"

#include <ranges>

auto canBlock(const ExecutionGraph &g, const View &s, int t) -> bool
{
	const auto *nLab = g.getNextLabel(g.getEventLabel(Event(t, s.getMax(t))));
	return nLab && (llvm::isa<ThreadJoinLabel>(nLab) || llvm::isa<LockCasReadLabel>(nLab) ||
			llvm::isa<BWaitReadLabel>(nLab));
}

auto isEnabled(const ExecutionGraph &g, const View &v, int t) -> bool
{
	auto last = Event(t, v.getMax(t));
	const auto *llab = g.getEventLabel(last);
	if (llvm::isa<TerminatorLabel>(llab))
		return false;

	/* If thread has no more events in the current execution,
	 * we consider it disabled for the context-bound. */
	const auto *nLab = g.getNextLabel(llab);
	if (!nLab)
		return false;

	if (llvm::isa<ThreadJoinLabel>(nLab))
		return v.contains(tj_pred(g, nLab)->getPos());

	/* Special cases for locks and barriers */
	if (auto *lLab = llvm::dyn_cast<LockCasReadLabel>(nLab)) {
		auto addr = lLab->getAddr();
		// Get latest store in the view
		auto it = std::find_if(g.co_rbegin(addr), g.co_rend(addr),
				       [&](const auto &w) { return v.contains(w.getPos()); });

		// No such store exists
		if (it == g.co_rend(addr)) {
			return std::none_of(g.getInitLabel()->rf_begin(addr),
					    g.getInitLabel()->rf_end(addr),
					    [&](const auto &e) { return v.contains(e.getPos()); });
		}
		// It is an unlock that is not already read by another completed lock
		auto *sLab = llvm::dyn_cast<UnlockWriteLabel>(&*it);
		return sLab &&
		       std::none_of(sLab->readers_begin(), sLab->readers_end(),
				    [&](const auto &e) { return v.contains(e.getPos().next()); });
	}
	if (auto *bLab = llvm::dyn_cast<BWaitReadLabel>(nLab)) {
		auto addr = bLab->getAddr();
		// Get barrrier initialization value
		auto *iwLab = llvm::dyn_cast<WriteLabel>(&*g.co_begin(addr));
		BUG_ON(!iwLab);
		auto it = std::find_if(g.co_rbegin(addr), g.co_rend(addr),
				       [&](const auto &w) { return v.contains(w.getPos()); });
		auto *wLab = llvm::dyn_cast<WriteLabel>(&*it);
		BUG_ON(!wLab);
		return iwLab->getVal() == wLab->getVal();
	}

	return true;
}

auto isSCMaximal(const ExecutionGraph &g, const View &v, int t) -> bool
{
	auto pos = Event(t, v.getMax(t));
	auto *lab = g.getEventLabel(pos);

	if (llvm::isa<ThreadFinishLabel>(lab)) {
		auto *tjLab = tj_succ(g, lab);
		return !tjLab || !v.contains(tjLab->getPos());
	}

	// ThreadStart has offset 0, but conceptually it is
	// not present in the view if it is the last event
	if (auto *cLab = llvm::dyn_cast<ThreadCreateLabel>(lab))
		return !v.getMax(cLab->getChildId());

	if (!llvm::isa<MemAccessLabel>(lab))
		return true;

	const EventLabel *wLab;
	if (auto sLab = llvm::dyn_cast<WriteLabel>(lab)) {
		if (std::any_of(sLab->readers_begin(), sLab->readers_end(),
				[&](const auto &rLab) { return v.contains(rLab.getPos()); }))
			return false;
		wLab = lab;
	} else if (auto rLab = llvm::dyn_cast<ReadLabel>(lab))
		wLab = rLab->getRf();

	auto addr = llvm::cast<MemAccessLabel>(lab)->getAddr();
	if (g.isLocEmpty(addr))
		return true;

	auto *succLab = llvm::isa<InitLabel>(wLab) ? &*g.co_begin(addr)
						   : g.co_imm_succ(llvm::cast<WriteLabel>(wLab));
	return !succLab || !v.contains(succLab->getPos());
}

void shrinkViewByOneInPlace(View &v, int t) { v.setMax(Event(t, v.getMax(t)).prev()); }

auto shrinkViewByOne(const View &v, int t) -> View
{
	auto vp = v;
	shrinkViewByOneInPlace(vp, t);
	return vp;
}

void unfoldPrefix(const ExecutionGraph &g, View &v, int t)
{
	while (v.getMax(t) && isSCMaximal(g, v, t) && !canBlock(g, v, t))
		shrinkViewByOneInPlace(v, t);
}

auto ContextBoundDecider::doesPrefixExceedBound(View v, int t, unsigned int bound) const -> bool
{
	auto &g = getGraph();
	unfoldPrefix(g, v, t);

	if (std::all_of(v.begin(), v.end(), [](auto &max) { return !max; }))
		return true;

	auto exists = false;
	for (auto tp = 0u; tp < g.getNumThreads(); tp++) {
		if (!v.getMax(tp) || !isSCMaximal(g, v, tp))
			continue;
		exists = true;
		auto sp = shrinkViewByOne(v, tp);
		if ((tp != t) && isEnabled(g, v, tp)) {
			if (bound && doesPrefixExceedBound(sp, tp, bound - 1))
				return true;
		} else if (doesPrefixExceedBound(sp, tp, bound))
			return true;
	}
	BUG_ON(!exists);

	return false;
}

auto ContextBoundDecider::doesExecutionExceedBound(unsigned int bound) const -> bool
{
	const auto &g = getGraph();
	const auto v = *llvm::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());

	auto exists = false;
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (!v.getMax(i) || !isSCMaximal(g, v, i))
			continue;
		exists = true;
		auto sp = shrinkViewByOne(v, i);
		if (doesPrefixExceedBound(sp, i, bound))
			return false;
	}
	BUG_ON(!exists);

	return true;
}

auto ContextBoundDecider::getSlack() const -> unsigned
{
	auto &g = getGraph();
	auto nonEmptyThreads =
		std::count_if(g.begin(), g.end(), [](const auto &t) { return t.size() > 1; });
	auto unstableThreads = std::count_if(g.begin(), g.end(), [](const auto &t) {
		return std::ranges::any_of(t, [](const auto &lab) { return !lab->isStable(); });
	});

	return std::min(std::max(0l, unstableThreads - 1), std::max(0l, nonEmptyThreads - 2));
}

#ifdef ENABLE_GENMC_DEBUG
auto ContextBoundDecider::calculate() const -> unsigned
{
	auto &g = getGraph();
	const auto v = *llvm::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());
	std::optional<unsigned> res;

	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (!v.getMax(i) || !isSCMaximal(g, v, i))
			continue;
		auto sp = shrinkViewByOne(v, i);
		auto b = calculate(sp, i);
		res = res ? std::min(*res, b) : b;
	}

	return *res;
}

auto ContextBoundDecider::calculate(View v, int t) const -> unsigned
{
	auto &g = getGraph();
	unfoldPrefix(g, v, t);

	if (std::all_of(v.begin(), v.end(), [](auto &max) { return !max; }))
		return 0;

	std::optional<unsigned> res;
	for (auto tp = 0u; tp < g.getNumThreads(); tp++) {
		/* F(s) = {t' | exists s'. s' ->(t') s} */
		if (!v.getMax(tp) || !isSCMaximal(g, v, tp))
			continue;
		auto sp = shrinkViewByOne(v, tp);
		auto bound = calculate(sp, tp) + ((t != tp) && isEnabled(g, v, tp) ? 1 : 0);
		res = res ? std::min(*res, bound) : bound;
	}

	return *res;
}
#endif
