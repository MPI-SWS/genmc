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

#include "RoundBoundDecider.hpp"
#include "ExecutionGraph.hpp"
#include "GraphIterators.hpp"
#include "View.hpp"

auto areSCPredsInView(const ExecutionGraph &g, const View &v, Event e) -> bool
{
	auto *lab = g.getEventLabel(e);
	if (llvm::isa<ThreadStartLabel>(lab))
		return v.contains(tc_pred(g, lab)->getPos());

	if (llvm::isa<ThreadJoinLabel>(lab))
		return v.contains(tj_pred(g, lab)->getPos());

	if (!llvm::isa<MemAccessLabel>(lab))
		return true;

	if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
		return v.contains(rLab->getRf()->getPos());

	auto *sLab = llvm::dyn_cast<WriteLabel>(lab);
	BUG_ON(!sLab);
	auto *pLab = g.co_imm_pred(sLab);
	if (pLab && !v.contains(pLab->getPos()))
		return false;

	return std::none_of(g.fr_imm_pred_begin(sLab), g.fr_imm_pred_end(sLab),
			    [&](const auto &rLab) { return !v.contains(rLab.getPos()); });
}

auto RoundBoundDecider::doesExecutionExceedBound(unsigned int bound) const -> bool
{

	auto &g = getGraph();
	const auto full = *llvm::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());
	View curr;

	do {
		auto exists = false;
		auto done = true;
		for (auto i = 0U; i < g.getNumThreads(); i++) {
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
		BUG_ON(!exists);
		if (done)
			return false;
	} while (bound--);
	return true;
}

#ifdef ENABLE_GENMC_DEBUG
auto RoundBoundDecider::calculate() const -> unsigned
{
	auto &g = getGraph();
	const auto full = *llvm::dyn_cast<View>(g.getViewFromStamp(g.getMaxStamp()).get());
	View curr;
	unsigned bound = 0;

	while (true) {
		auto exists = false;
		auto done = true;
		for (auto i = 0U; i < g.getNumThreads(); i++) {
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
		BUG_ON(!exists);
		bound++;
	}
	return bound;
}
#endif
