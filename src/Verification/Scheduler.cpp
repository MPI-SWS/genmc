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

#include "Verification/Scheduler.hpp"
#include "ADT/View.hpp"
#include "ExecutionGraph/Event.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/GraphUtils.hpp"
#include "Support/Error.hpp"
#include "Verification/Config.hpp"

#include <llvm/Support/Casting.h>

#include <algorithm>
#include <ranges>
#include <span>
#include <string>
#include <vector>

auto Scheduler::create(const Config *config) -> std::unique_ptr<Scheduler>
{
#define CREATE_SCHEDULER(_policy)                                                                  \
	case SchedulePolicy::_policy:                                                              \
		return std::make_unique<_policy##Scheduler>(config)

	switch (config->schedulePolicy) {
		CREATE_SCHEDULER(LTR);
		CREATE_SCHEDULER(WF);
		CREATE_SCHEDULER(WFR);
		CREATE_SCHEDULER(Arbitrary);
	default:
		BUG();
	}
}

static void calcPorfReplay(const EventLabel *lab, View &view, std::vector<Event> &schedule)
{
	if (!lab || view.contains(lab->getPos()))
		return;

	auto i = view.getMax(lab->getThread());
	view.updateIdx(lab->getPos());

	const auto &g = *lab->getParent();
	for (++i; i <= lab->getIndex(); ++i) {
		const auto *pLab = g.getEventLabel(Event(lab->getThread(), i));
		if (const auto *rLab = llvm::dyn_cast<ReadLabel>(pLab))
			calcPorfReplay(rLab->getRf(), view, schedule);
		else if (const auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(pLab))
			calcPorfReplay(g.getLastThreadLabel(jLab->getChildId()), view, schedule);
		else if (const auto *tsLab = llvm::dyn_cast<ThreadStartLabel>(pLab))
			calcPorfReplay(tsLab->getCreate(), view, schedule);
		if (!llvm::isa<BlockLabel>(pLab))
			schedule.push_back(pLab->getPos());
	}
}

static auto isSchedulable(const ExecutionGraph &g, int thread) -> bool
{
	const auto *lab = g.getLastThreadLabel(thread);
	return !llvm::isa_and_nonnull<TerminatorLabel>(lab);
}

static auto calculateReplaySchedule(const ExecutionGraph &g, const Config *conf)
	-> std::vector<Event>
{
	/* Calculate preliminary replay schedule (reversed order) */
	View view;
	std::vector<Event> result;
	for (auto i = 0U; i < g.getNumThreads(); i++)
		calcPorfReplay(g.getLastThreadLabel(i), view, result);

	/* Erase any non-schedulable threads. */
	if (!conf->replayCompletedThreads) {
		std::erase_if(result, [&g](const auto &pos) {
			auto *lab = g.getLastThreadLabel(pos.thread);
			return !isSchedulable(g, lab->getThread()) &&
			       !llvm::isa_and_nonnull<BlockLabel>(lab);
		});
	}

	/* Erase NAs as they cannot affect the schedule (unless RD is disabled) */
	if (!conf->disableRaceDetection) {
		std::erase_if(result, [&g](const auto &pos) {
			return g.getEventLabel(pos)->isNotAtomic();
		});
	}

	/* The schedule is still reversed, need to fix that. */
	std::ranges::reverse(result);
	// GENMC_DEBUG(llvm::dbgs() << format(std::ranges::reverse_view(result)) << "\n";);
	return result;
}

void Scheduler::resetExplorationOptions(const ExecutionGraph &g)
{
	setRescheduledRead(Event::getInit());
	threadPrios_.clear();
	replaySchedule_ = calculateReplaySchedule(g, getConf());

	/* Check whether the event that led to this execs needs thread prioritization */
	for (auto tid : g.thr_ids()) {
		const auto *rLab = llvm::dyn_cast<ReadLabel>(g.getLastThreadLabel(tid));
		if (!rLab)
			continue;

		auto rpreds = po_preds(g, rLab);
		auto oLabIt = std::ranges::find_if(
			rpreds, [&](auto &oLab) { return llvm::isa<SpeculativeReadLabel>(&oLab); });
		if (llvm::isa<SpeculativeReadLabel>(rLab) || oLabIt != rpreds.end())
			prioritize(rLab->getPos());
	}
}

auto Scheduler::scheduleReplay(const ExecutionGraph &g, std::span<Action> runnable)
	-> std::optional<int>
{
	/* Pop any events already replayed. This is done lazily as:
	 * 1. one interpreter instruction may map to many graph events (currently only for RMWs);
	 *    in such cases, schedule() is not called in between the events.
	 * 2. some runtimes may call schedule() before local instructions not tracked in the
	 *    graph (i.e., runnable remains the same across different calls) */
	while (!replaySchedule_.empty() &&
	       replaySchedule_.back().index <= runnable[replaySchedule_.back().thread].event.index)
		replaySchedule_.pop_back();

	if (replaySchedule_.empty())
		return {};

	/* If the next event to be replayed is an assume()-read, eagerly pop it. Otherwise,
	 * the instruction counter might never increase (due to the read blocking), and
	 * we will be stuck scheduling the same thread forever. */
	auto next = replaySchedule_.back();
	const auto *lastLab = g.getLastThreadLabel(next.thread);
	if (llvm::isa<BlockLabel>(lastLab) && next.index == lastLab->getIndex() - 1 &&
	    llvm::isa<ReadLabel>(g.po_imm_pred(lastLab))) /* overapproximation */
		replaySchedule_.pop_back();
	return {next.thread};
}

auto Scheduler::schedulePrioritized(const ExecutionGraph &g) -> std::optional<int>
{
	if (threadPrios_.empty())
		return {};

	BUG_ON(getConf()->bound.has_value());

	auto result = std::ranges::find_if(
		threadPrios_, [&g](const auto &pos) { return isSchedulable(g, pos.thread); });
	return (result != threadPrios_.end()) ? std::make_optional(result->thread) : std::nullopt;
}

auto Scheduler::rescheduleReads(ExecutionGraph &g) -> std::optional<int>
{
	auto result = std::ranges::find_if(g.thr_ids(), [this, &g](auto tid) {
		auto *bLab = llvm::dyn_cast_or_null<ReadOptBlockLabel>(g.getLastThreadLabel(tid));
		if (!bLab)
			return false;

		BUG_ON(getConf()->bound.has_value());
		setRescheduledRead(bLab->getPos());
		unblockThread(g, bLab->getPos());
		return true;
	});
	return (result != g.thr_ids().end()) ? std::make_optional(*result) : std::nullopt;
}

auto Scheduler::schedule(ExecutionGraph &g, std::span<Action> runnable) -> std::optional<int>
{
	std::optional<int> result;

	/* Check if we are in replay mode*/
	if ((result = scheduleReplay(g, runnable)))
		return result;

	/* Check if we should prioritize some thread */
	if ((result = schedulePrioritized(g)))
		return result;

	/* Schedule the next thread according to the chosen policy. */
	if ((result = schedulePolicy(g, runnable)))
		return result;

	/* All threads are either blocked or terminated, so we check if we can unblock some
	 * blocked reads. */
	return rescheduleReads(g);
}

static auto extractValPrefix(const ExecutionGraph &g, Event pos)
	-> std::pair<std::vector<SVal>, std::vector<Event>>
{
	std::vector<SVal> vals;
	std::vector<Event> events;

	for (auto i = 0U; i < pos.index; i++) {
		const auto *lab = g.getEventLabel(Event(pos.thread, i));
		if (lab->returnsValue()) {
			vals.push_back(lab->getReturnValue());
			events.push_back(lab->getPos());
		}
	}
	return {vals, events};
}

void Scheduler::cacheEventLabel(const ExecutionGraph &g, const EventLabel *lab)
{
	/* Find the respective function ID: if no label has been cached, lab is a begin */
	const auto *firstLab = llvm::isa<ThreadStartLabel>(lab)
				       ? llvm::dyn_cast<ThreadStartLabel>(lab)
				       : g.getFirstThreadLabel(lab->getThread());
	auto cacheKey = std::make_pair(firstLab->getThreadInfo().funId, (unsigned)lab->getThread());

	/* Helper that copies and resets the prefix of LAB starting from index FROM. */
	auto copyPrefix = [&](auto from, auto &lab) {
		std::vector<std::unique_ptr<EventLabel>> labs;
		for (auto i = from; i <= lab->getIndex(); i++) {
			auto cLab = (i == lab->getIndex())
					    ? lab->clone()
					    : g.getEventLabel(Event(lab->getThread(), i))->clone();
			cLab->reset();
			labs.push_back(std::move(cLab));
		}
		return labs;
	};

	/* Extract value prefix and find how much of it has already been cached */
	auto [vals, indices] = extractValPrefix(g, lab->getPos());
	auto commonPrefixLen = seenPrefixes[cacheKey].findLongestCommonPrefix(vals);
	std::vector<SVal> seenVals(vals.begin(), vals.begin() + commonPrefixLen);
	auto *data = retrieveCachedSuccessors(cacheKey, seenVals);
	BUG_ON(!data);

	/*
	 * Fastpath: if there are no new data to cache, return.
	 * (For dep-tracking, we could optimize toIdx and collect until
	 * a new (non-empty) label with a value is found.)
	 */
	if (!data->empty() && data->back()->getIndex() >= lab->getIndex())
		return;

	/*
	 * Fetch the labels to cache. We try to copy as little as possible,
	 * by inspecting what's already cached.
	 */
	auto fromIdx = commonPrefixLen == 0 ? 0 : indices[commonPrefixLen - 1].index + 1;
	if (!data->empty())
		fromIdx = std::max(data->back()->getIndex() + 1, fromIdx);
	auto labs = copyPrefix(fromIdx, lab);

	/* Go ahead and copy */
	for (auto i = 0U; i < labs.size(); i++) {
		/* Ensure label has not already been cached */
		BUG_ON(!data->empty() && data->back()->getIndex() >= labs[i]->getIndex());
		/* If the last cached label returns a value, then we need
		 * to cache to a different bucket */
		if (!data->empty() && data->back()->returnsValue()) {
			seenVals.push_back(vals[seenVals.size()]);
			auto res = seenPrefixes[cacheKey].addSeq(seenVals, {});
			BUG_ON(!res);
			data = retrieveCachedSuccessors(cacheKey, seenVals);
		}
		data->push_back(std::move(labs[i]));
	}
}

static auto findNextLabelToAdd(const ExecutionGraph &g, int thread) -> Event
{
	const auto *firstLab = g.getFirstThreadLabel(thread);
	auto succs = po_succs(g, firstLab);
	auto it =
		std::ranges::find_if(succs, [&](auto &lab) { return llvm::isa<EmptyLabel>(&lab); });
	return it == succs.end() ? g.getLastThreadLabel(thread)->getPos().next() : (*it).getPos();
}

auto Scheduler::retrieveFromCache(const ExecutionGraph &g, int thread)
	-> std::vector<std::unique_ptr<EventLabel>> *
{
	auto key = std::make_pair(g.getFirstThreadLabel(thread)->getThreadInfo().funId,
				  (unsigned)thread);

	auto next = findNextLabelToAdd(g, thread);
	auto [vals, last] = extractValPrefix(g, next);
	auto *res = retrieveCachedSuccessors(key, vals);
	if (res == nullptr || res->empty() || res->back()->getIndex() < next.index)
		return nullptr;
	return res;
}

auto Scheduler::scheduleFromCache(ExecutionGraph &g)
	-> std::optional<std::vector<std::unique_ptr<EventLabel>> *>
{
	auto tids = g.thr_ids();
	auto nextIt = std::ranges::find_if(tids, [&](auto tid) { return isSchedulable(g, tid); });
	if (nextIt != std::ranges::end(tids))
		return std::make_optional(retrieveFromCache(g, *nextIt));

	auto next = rescheduleReads(g);
	return next.has_value() ? std::make_optional(retrieveFromCache(g, *next)) : std::nullopt;
}

static auto getFirstSchedulableSymmetric(const ExecutionGraph &g, int tid) -> int
{
	auto firstSched = tid;
	auto symm = g.getFirstThreadLabel(tid)->getSymmPredTid();
	while (symm != -1) {
		if (isSchedulable(g, symm))
			firstSched = symm;
		symm = g.getFirstThreadLabel(symm)->getSymmPredTid();
	}
	return firstSched;
}

/*******************************************************************************
 **                               LTRScheduler
 ******************************************************************************/

auto LTRScheduler::schedulePolicy(const ExecutionGraph &g, std::span<Action> runnable)
	-> std::optional<int>
{
	auto result = std::ranges::find_if(runnable, [&g](const auto &action) {
		return isSchedulable(g, action.event.thread);
	});
	return (result != runnable.end()) ? std::make_optional(result->event.thread) : std::nullopt;
}

/*******************************************************************************
 **                                WFScheduler
 ******************************************************************************/

auto WFScheduler::schedulePolicy(const ExecutionGraph &g, std::span<Action> runnable)
	-> std::optional<int>
{
	/* Try and find a thread that has a non-load event next.
	 * Keep an LTR fallback option in case this fails. */
	int fallback = -1;
	auto result = std::ranges::find_if(runnable, [&g, &fallback](const auto &action) {
		if (!isSchedulable(g, action.event.thread))
			return false;
		if (fallback == -1)
			fallback = action.event.thread;
		return action.kind != ActionKind::Load;
	});

	if (result != runnable.end())
		return {getFirstSchedulableSymmetric(g, result->event.thread)};

	return (fallback != -1) ? std::make_optional(getFirstSchedulableSymmetric(g, fallback))
				: std::nullopt;
}

/*******************************************************************************
 **                            ArbitraryScheduler
 ******************************************************************************/

ArbitraryScheduler::ArbitraryScheduler(const Config *config) : Scheduler(config)
{
	/* Set up a random-number generator for the scheduler */
	std::random_device rd;
	const auto *conf = getConf();
	auto seedVal = (!conf->randomScheduleSeed.empty())
			       ? static_cast<MyRNG::result_type>(stoull(conf->randomScheduleSeed))
			       : rd();
	rng_.seed(seedVal);
}

auto ArbitraryScheduler::schedulePolicy(const ExecutionGraph &g, std::span<Action> runnable)
	-> std::optional<int>
{
	const auto numThreads = static_cast<int>(runnable.size());

	MyDist dist(0, numThreads);
	const auto random = static_cast<int>(dist(rng_));
	for (auto i = 0; i < numThreads; i++) {
		const auto &action = runnable[(i + random) % numThreads];

		if (!isSchedulable(g, action.event.thread))
			continue;

		/* Found a not-yet-complete thread; schedule it */
		return {getFirstSchedulableSymmetric(g, action.event.thread)};
	}
	return {};
}

/*******************************************************************************
 **                              WFRScheduler
 ******************************************************************************/

auto WFRScheduler::schedulePolicy(const ExecutionGraph &g, std::span<Action> runnable)
	-> std::optional<int>
{
	std::vector<int> nonwrites;
	std::vector<int> writes;
	for (auto &action : runnable) {
		if (!isSchedulable(g, action.event.thread))
			continue;

		if (action.kind != ActionKind::Load)
			writes.push_back(action.event.thread);
		else
			nonwrites.push_back(action.event.thread);
	}

	std::vector<int> &selection = !writes.empty() ? writes : nonwrites;
	if (selection.empty())
		return {};

	MyDist dist(0, selection.size() - 1);
	const auto candidate = selection[dist(getRng())];
	return {getFirstSchedulableSymmetric(g, candidate)};
}
