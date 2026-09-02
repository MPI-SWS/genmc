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

#include "RoundBoundDecider.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Support/Cast.hpp"
#include "genmc/Support/Error.hpp"

#include <algorithm>
#include <utility>

static auto areSCPredsInView(const ExecutionGraph &g, const View &v, Event e) -> bool
{
	const auto *lab = g.getEventLabel(e);
	if (genmc::isa<ThreadStartLabel>(lab))
		return v.contains(g.tc_pred(lab)->getPos());

	if (genmc::isa<ThreadJoinLabel>(lab))
		return v.contains(g.tj_pred(lab)->getPos());

	if (!genmc::isa<MemAccessLabel>(lab))
		return true;

	if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab))
		return v.contains(rLab->getRf()->getPos());

	const auto *sLab = genmc::dyn_cast<WriteLabel>(lab);
	VERIFY(sLab);
	const auto *pLab = g.co_imm_pred(sLab);
	if (pLab && !v.contains(pLab->getPos()))
		return false;

	return std::ranges::none_of(g.fr_imm_preds(sLab),
				    [&](const auto &rLab) { return !v.contains(rLab.getPos()); });
}

auto RoundBoundDecider::doesExecutionExceedBound(unsigned int bound) const -> bool
{

	const auto &g = getGraph();
	const auto full = *genmc::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());
	View curr;
	for (auto i = 1; std::cmp_less(i, g.getNumThreads()); i++)
		curr.setMax(Event(i, -1));

	do {
		auto exists = false;
		auto done = true;
		for (auto i = 0; i < g.getNumThreads(); i++) {
			for (auto j = curr.getMax(i); j < full.getMax(i); j++) {
				auto next = Event(i, j + 1);
				if (!areSCPredsInView(g, curr, next)) {
					done = false;
					break;
				}
				exists = true;
				curr.updateIdx(next);
			}
		}
		VERIFY(exists);
		if (done)
			return false;
	} while (bound--);
	return true;
}

#ifdef ENABLE_GENMC_DEBUG
auto RoundBoundDecider::calculate() const -> unsigned
{
	const auto &g = getGraph();
	const auto full = *genmc::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());
	View curr;
	unsigned bound = 0;

	while (true) {
		auto exists = false;
		auto done = true;
		for (auto i = 0; i < g.getNumThreads(); i++) {
			for (auto j = curr.getMax(i); j < full.getMax(i); j++) {
				auto next = Event(i, j + 1);
				if (!areSCPredsInView(g, curr, next)) {
					done = false;
					break;
				}
				exists = true;
				curr.updateIdx(next);
			}
		}
		if (done)
			break;
		VERIFY(exists);
		bound++;
	}
	return bound;
}
#endif
