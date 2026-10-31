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

#include "ConsistencyChecker.hpp"
#include "genmc/ADT/VectorClock.hpp"
#include "genmc/Execution/Consistency/ARM8Checker.hpp" // NOLINT(misc-include-cleaner)
#include "genmc/Execution/Consistency/IMMChecker.hpp"  // NOLINT(misc-include-cleaner)
#include "genmc/Execution/Consistency/RAChecker.hpp"   // NOLINT(misc-include-cleaner)
#include "genmc/Execution/Consistency/RC11Checker.hpp" // NOLINT(misc-include-cleaner)
#include "genmc/Execution/Consistency/SCChecker.hpp"   // NOLINT(misc-include-cleaner)
#include "genmc/Execution/Consistency/TSOChecker.hpp"  // NOLINT(misc-include-cleaner)
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/Stamp.hpp"
#include "genmc/Support/Cast.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/MemoryModel.hpp"

#include <algorithm>
#include <memory>

auto ConsistencyChecker::getHbView(const EventLabel *lab) const -> const View &
{
	return lab->view(getHbViewIndex());
}

auto ConsistencyChecker::create(const Config *conf) -> std::unique_ptr<ConsistencyChecker>
{
#define CREATE_CHECKER(_model)                                                                     \
	case ModelType::_model:                                                                    \
		return std::make_unique<_model##Checker>(conf);

	switch (conf->model) {
		CREATE_CHECKER(SC);
		CREATE_CHECKER(TSO);
		CREATE_CHECKER(RA);
		CREATE_CHECKER(RC11);
		CREATE_CHECKER(IMM);
		CREATE_CHECKER(ARM8);
	default:
		UNREACHABLE();
	}
}

auto ConsistencyChecker::survives(const EventLabel &lab, Stamp ref, const VectorClock &keep,
				  const EventLabel *except) -> bool
{
	return !lab.isSkeleton() && &lab != except &&
	       (lab.getStamp() < ref || keep.contains(lab.getPos()));
}

auto ConsistencyChecker::hasSurvivingCoSucc(const EventLabel *lab, const MemAccessLabel *aLab,
					    const VectorClock &keep, const EventLabel *except)
	-> bool
{
	ASSERT(genmc::isa<InitLabel>(lab) || genmc::isa<WriteLabel>(lab));

	const auto &g = *lab->getParent();
	auto surviving = [&](const WriteLabel &sLab) {
		return survives(sLab, aLab->getStamp(), keep, except);
	};

	/* Walk all successors: a deleted one may itself be placed maximally (wb2 under IMM) */
	return genmc::isa<InitLabel>(lab) ? std::ranges::any_of(g.co(aLab->getAddr()), surviving)
					  : std::ranges::any_of(g.co_succs(lab), surviving);
}

auto ConsistencyChecker::getMaximalRf(const ReadLabel *rLab) const -> const EventLabel *
{
	return rLab->getParent()->co_max(rLab->getAddr());
}

auto ConsistencyChecker::getMaximalCo(const WriteLabel *wLab) const -> const EventLabel *
{
	const auto &g = *wLab->getParent();
	auto rco = g.rco(wLab->getAddr());
	auto it = std::ranges::find_if(rco, [wLab](auto &oLab) { return &oLab != wLab; });
	return it != rco.end() ? static_cast<const EventLabel *>(&*it) : g.getInitLabel();
}

auto ConsistencyChecker::isMaximalRf(const ReadLabel *rLab, const VectorClock &keep,
				     const EventLabel *except) const -> bool
{
	return rLab->wasAddedMax() && !hasSurvivingCoSucc(rLab->getRf(), rLab, keep, except);
}

auto ConsistencyChecker::isMaximalCo(const WriteLabel *wLab, const VectorClock &keep,
				     const EventLabel *except) const -> bool
{
	return wLab->wasAddedMax() && !hasSurvivingCoSucc(wLab, wLab, keep, except);
}
