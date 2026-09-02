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
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Support/Cast.hpp"
#include "genmc/Support/Error.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <utility>

static auto canBlock(const ExecutionGraph &g, const View &s, int tid) -> bool
{
	const auto *nLab = g.po_imm_succ(g.getEventLabel(Event(tid, s.getMax(tid))));
	return nLab && (genmc::isa<ThreadJoinLabel>(nLab) || genmc::isa<LockCasReadLabel>(nLab) ||
			genmc::isa<BWaitReadLabel>(nLab));
}

static auto isEnabled(const ExecutionGraph &g, const View &v, int tid) -> bool
{
	auto last = Event(tid, v.getMax(tid));
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
	if (const auto *lLab = genmc::dyn_cast<LockCasReadLabel>(nLab)) {
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
		const auto *sLab = genmc::dyn_cast<UnlockWriteLabel>(&*it);
		return sLab && std::ranges::none_of(sLab->readers(), [&](const auto &e) {
			       return v.contains(e.getPos().next());
		       });
	}
	if (const auto *bLab = genmc::dyn_cast<BWaitReadLabel>(nLab)) {
		auto addr = bLab->getAddr();
		// Get barrrier initialization value
		const auto *iwLab = genmc::dyn_cast<WriteLabel>(&*std::ranges::begin(g.co(addr)));
		VERIFY(iwLab);
		auto it = std::ranges::find_if(
			g.rco(addr), [&](const auto &w) { return v.contains(w.getPos()); });
		const auto *wLab = genmc::dyn_cast<WriteLabel>(&*it);
		VERIFY(wLab);
		return iwLab->getVal() == wLab->getVal();
	}

	return true;
}

static auto isSCMaximal(const ExecutionGraph &g, const View &v, int tid) -> bool
{
	auto pos = Event(tid, v.getMax(tid));
	const auto *lab = g.getEventLabel(pos);

	if (genmc::isa<ThreadFinishLabel>(lab)) {
		const auto *tjLab = g.tj_succ(lab);
		return !tjLab || !v.contains(tjLab->getPos());
	}

	// ThreadStart has offset 0, but conceptually it is
	// not present in the view if it is the last event
	if (const auto *cLab = genmc::dyn_cast<ThreadCreateLabel>(lab))
		return !v.getMax(cLab->getChildId());

	if (!genmc::isa<MemAccessLabel>(lab))
		return true;

	const EventLabel *wLab = nullptr;
	if (const auto *sLab = genmc::dyn_cast<WriteLabel>(lab)) {
		if (std::ranges::any_of(sLab->readers(), [&](const auto &rLab) {
			    return v.contains(rLab.getPos());
		    }))
			return false;
		wLab = lab;
	} else if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab))
		wLab = rLab->getRf();

	auto addr = genmc::cast<MemAccessLabel>(lab)->getAddr();
	if (g.isLocEmpty(addr))
		return true;

	const auto *succLab = genmc::isa<InitLabel>(wLab)
				      ? &*std::ranges::begin(g.co(addr))
				      : g.co_imm_succ(genmc::cast<WriteLabel>(wLab));
	return !succLab || !v.contains(succLab->getPos());
}

static void shrinkViewByOneInPlace(View &v, int tid) { v.setMax(Event(tid, v.getMax(tid)).prev()); }

static auto shrinkViewByOne(const View &v, int tid) -> View
{
	auto vp = v;
	shrinkViewByOneInPlace(vp, tid);
	return vp;
}

static void unfoldPrefix(const ExecutionGraph &g, View &v, int tid)
{
	while (v.getMax(tid) && isSCMaximal(g, v, tid) && !canBlock(g, v, tid))
		shrinkViewByOneInPlace(v, tid);
}

auto ContextBoundDecider::doesPrefixExceedBound(View v, int tid, unsigned int bound) const -> bool
{
	const auto &g = getGraph();
	unfoldPrefix(g, v, tid);

	if (v.empty())
		return true;

	auto exists = false;
	for (auto j = 0; j < g.getNumThreads(); j++) {
		if (!v.getMax(j) || !isSCMaximal(g, v, j))
			continue;
		exists = true;
		auto sp = shrinkViewByOne(v, j);
		if ((std::cmp_not_equal(j, tid)) && isEnabled(g, v, j)) {
			if (bound && doesPrefixExceedBound(sp, j, bound - 1))
				return true;
		} else if (doesPrefixExceedBound(sp, j, bound))
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
	for (auto i = 0; i < g.getNumThreads(); i++) {
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
	const auto &g = getGraph();
	auto nonEmptyThreads = std::count_if(g.begin(), g.end(),
					     [](const auto &thread) { return thread.size() > 1; });
	auto unstableThreads = std::count_if(g.begin(), g.end(), [](const auto &thread) {
		return std::ranges::any_of(thread,
					   [](const auto &lab) { return !lab->isStable(); });
	});

	return std::min(std::max(0L, unstableThreads - 1), std::max(0L, nonEmptyThreads - 2));
}

#ifdef ENABLE_GENMC_DEBUG
auto ContextBoundDecider::calculate() const -> unsigned
{
	const auto &g = getGraph();
	const auto v = *genmc::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());
	std::optional<unsigned> res;

	for (auto i = 0; i < g.getNumThreads(); i++) {
		if (!v.getMax(i) || !isSCMaximal(g, v, i))
			continue;
		auto sp = shrinkViewByOne(v, i);
		auto bound = calculate(sp, i);
		res = res ? std::min(*res, bound) : bound;
	}

	return *res;
}

auto ContextBoundDecider::calculate(View v, int tid) const -> unsigned
{
	const auto &g = getGraph();
	unfoldPrefix(g, v, tid);

	if (v.empty())
		return 0;

	std::optional<unsigned> res;
	for (auto j = 0; j < g.getNumThreads(); j++) {
		/* F(s) = {tid' | exists s'. s' ->(tid') s} */
		if (!v.getMax(j) || !isSCMaximal(g, v, j))
			continue;
		auto sp = shrinkViewByOne(v, j);
		auto bound = calculate(sp, j) +
			     ((std::cmp_not_equal(tid, j)) && isEnabled(g, v, j) ? 1 : 0);
		res = res ? std::min(*res, bound) : bound;
	}

	return *res;
}
#endif
