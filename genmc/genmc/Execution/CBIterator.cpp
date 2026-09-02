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

#include "genmc/Execution/CBIterator.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Support/Cast.hpp"

static auto cb_pred(const EventLabel *lab) -> const EventLabel *
{
	if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab))
		return rLab->getRf();
	if (const auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(lab))
		return lab->getParent()->getLastThreadLabel(jLab->getChildId());
	if (const auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(lab))
		return tsLab->getCreate();
	return nullptr;
}

void CBIterator::advance()
{
	while (!stack_.empty()) {
		/* Check if we're past the last index for this call */
		auto &f = stack_.back();
		if (f.i > f.end.index) {
			stack_.pop_back();
			continue;
		}

		/* Process the predecessors of the event first */
		const auto *lab = g_->getEventLabel({f.end.thread, f.i});
		if (!f.descended) {
			if (const auto *pLab = cb_pred(lab); pLab && !a_.contains(pLab->getPos())) {
				f.descended = true;
				auto pIdx = a_.getMax(pLab->getThread());
				a_.setMax(pLab->getPos());
				stack_.emplace_back(pLab->getPos(), pIdx, false);
				continue;
			}
		}

		/* yield label and advance cursor */
		current_ = lab;
		++f.i;
		f.descended = false;
		return;
	}
}
