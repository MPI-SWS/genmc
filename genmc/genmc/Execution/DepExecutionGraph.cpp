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

#include "DepExecutionGraph.hpp"
#include "genmc/ADT/DepView.hpp"
#include "genmc/ADT/VectorClock.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/Stamp.hpp"
#include "genmc/Support/Cast.hpp"

#include <memory>

auto DepExecutionGraph::getViewFromStamp(Stamp stamp) const -> std::unique_ptr<VectorClock>
{
	auto preds = std::make_unique<DepView>();

	for (auto i = 0; i < getNumThreads(); i++) {
		for (auto j = 1; j < getThreadSize(i); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!lab->isSkeleton() && lab->getStamp() <= stamp)
				preds->setMax(Event(i, j));
		}
	}
	return preds;
}

void DepExecutionGraph::cutToStamp(Stamp stamp)
{
	/* The read at STAMP, if any, is re-pointed by the forward revisit, so its
	 * dependents go too */
	const ReadLabel *changed = nullptr;
	for (const auto &lab : labels()) {
		if (lab.getStamp() == stamp) {
			changed = genmc::dyn_cast<ReadLabel>(&lab);
			break;
		}
	}
	deleteEvents(*getViewFromStamp(stamp), changed);
}

auto DepExecutionGraph::getCopyUpTo(const VectorClock &v) const -> std::unique_ptr<ExecutionGraph>
{
	auto og = std::make_unique<DepExecutionGraph>(
		ExecutionGraph::Config{.execState = this->state_,
				       .consChecker = this->consChecker_,
				       .emitNALabels = this->haveNAs_});
	copyGraphUpTo(*og, v);
	return og;
}
