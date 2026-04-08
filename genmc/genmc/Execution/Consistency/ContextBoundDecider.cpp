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

#include "ContextBoundDecider.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"

#include <ranges>

auto canBlock(const ExecutionGraph &g, const View &s, int t) -> bool
{
	const auto *nLab = g.po_imm_succ(g.getEventLabel(Event(t, s.getMax(t))));
	return nLab && (genmc::isa<ThreadJoinLabel>(nLab) || genmc::isa<LockCasReadLabel>(nLab) ||
			genmc::isa<BWaitReadLabel>(nLab));
}

auto isEnabled(const ExecutionGraph &g, const View &v, int t) -> bool
{
	auto last = Event(t, v.getMax(t));
	const auto *llab = g.getEventLabel(last);
	if (genmc::isa<TerminatorLabel>(llab))
		return false;

	/* If thread has no more events in the current execution,
	 * we consider it disabled for the context-bound. */
	const auto *nLab = g.po_imm_succ(llab);
	if (!nLab)
		return false;

	if (genmc::isa<ThreadJoinLabel>(nLab))
		return v.contains(g.tj_pred(nLab)->getPos());

	/* Special cases for locks and barriers */
	if (auto *lLab = genmc::dyn_cast<LockCasReadLabel>(nLab)) {
		auto addr = lLab->getAddr();
		// Get latest store in the view
		auto it = std::ranges::find_if(
			g.rco(addr), [&](const auto &w) { return v.contains(w.getPos()); });

		// No such store exists
		if (it == std::ranges::end(g.rco(addr))) {
			return std::none_of(g.getInitLabel()->rf_begin(addr),
					    g.getInitLabel()->rf_end(addr),
					    [&](const auto &e) { return v.contains(e.getPos()); });
		}
		// It is an unlock that is not already read by another completed lock
		auto *sLab = genmc::dyn_cast<UnlockWriteLabel>(&*it);
		return sLab && std::ranges::none_of(sLab->readers(), [&](const auto &e) {
			       return v.contains(e.getPos().next());
		       });
	}
	if (auto *bLab = genmc::dyn_cast<BWaitReadLabel>(nLab)) {
		auto addr = bLab->getAddr();
		// Get barrrier initialization value
		auto *iwLab = genmc::dyn_cast<WriteLabel>(&*std::ranges::begin(g.co(addr)));
		VERIFY(iwLab);
		auto it = std::ranges::find_if(
			g.rco(addr), [&](const auto &w) { return v.contains(w.getPos()); });
		auto *wLab = genmc::dyn_cast<WriteLabel>(&*it);
		VERIFY(wLab);
		return iwLab->getVal() == wLab->getVal();
	}

	return true;
}

auto isSCMaximal(const ExecutionGraph &g, const View &v, int t) -> bool
{
	auto pos = Event(t, v.getMax(t));
	auto *lab = g.getEventLabel(pos);

	if (genmc::isa<ThreadFinishLabel>(lab)) {
		auto *tjLab = g.tj_succ(lab);
		return !tjLab || !v.contains(tjLab->getPos());
	}

	// ThreadStart has offset 0, but conceptually it is
	// not present in the view if it is the last event
	if (auto *cLab = genmc::dyn_cast<ThreadCreateLabel>(lab))
		return !v.getMax(cLab->getChildId());

	if (!genmc::isa<MemAccessLabel>(lab))
		return true;

	const EventLabel *wLab;
	if (auto sLab = genmc::dyn_cast<WriteLabel>(lab)) {
		if (std::ranges::any_of(sLab->readers(), [&](const auto &rLab) {
			    return v.contains(rLab.getPos());
		    }))
			return false;
		wLab = lab;
	} else if (auto rLab = genmc::dyn_cast<ReadLabel>(lab))
		wLab = rLab->getRf();

	auto addr = genmc::cast<MemAccessLabel>(lab)->getAddr();
	if (g.isLocEmpty(addr))
		return true;

	auto *succLab = genmc::isa<InitLabel>(wLab) ? &*std::ranges::begin(g.co(addr))
						    : g.co_imm_succ(genmc::cast<WriteLabel>(wLab));
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

	if (v.empty())
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
	VERIFY(exists);

	return false;
}

auto ContextBoundDecider::doesExecutionExceedBound(unsigned int bound) const -> bool
{
	const auto &g = getGraph();
	const auto v = *genmc::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());

	auto exists = false;
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (!v.getMax(i) || !isSCMaximal(g, v, i))
			continue;
		exists = true;
		auto sp = shrinkViewByOne(v, i);
		if (doesPrefixExceedBound(sp, i, bound))
			return false;
	}
	VERIFY(exists);

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
	const auto v = *genmc::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());
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

	if (v.empty())
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
