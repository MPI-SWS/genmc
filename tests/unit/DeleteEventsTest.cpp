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

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "genmc/ADT/DepView.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/DepExecutionGraph.hpp"
#include "genmc/Execution/DepInfo.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Support/SAddr.hpp"

namespace {

/* A checker that accepts everything; only the base oracle is exercised */
class StubChecker : public ConsistencyChecker {
public:
	StubChecker() : ConsistencyChecker(nullptr) {}
	[[nodiscard]] auto isConsistent(const EventLabel * /*lab*/) const -> bool override
	{
		return true;
	}
	[[nodiscard]] auto isConsistent(const ExecutionGraph & /*g*/) const -> bool override
	{
		return true;
	}
	auto checkErrors(const EventLabel * /*lab*/, const EventLabel *& /*race*/) const
		-> std::optional<VerificationError> override
	{
		return {};
	}
	auto checkWarnings(const EventLabel * /*lab*/, const VSet<VerificationError> & /*reported*/,
			   std::vector<const EventLabel *> & /*races*/) const
		-> std::vector<VerificationError> override
	{
		return {};
	}
	void filterCoherentRevisits(WriteLabel * /*sLab*/,
				    std::vector<ReadLabel *> & /*ls*/) override
	{}
	auto getCoherentStores(ReadLabel * /*rLab*/) -> std::vector<EventLabel *> override
	{
		return {};
	}
	auto getCoherentPlacings(WriteLabel * /*wLab*/) -> std::vector<EventLabel *> override
	{
		return {};
	}
	void updateMMViews(EventLabel * /*lab*/) override {}
	auto calculatePrefixView(const EventLabel * /*lab*/) const
		-> std::unique_ptr<VectorClock> override
	{
		return std::make_unique<DepView>();
	}
	auto getHbViewIndex() const -> unsigned override { return 0; }
	[[nodiscard]] auto isDepTracking() const -> bool override { return true; }
	void resetCacheCounters() const override {}
	void recomputeCacheCounters(const ExecutionGraph & /*g*/) const override {}
	void maybeDecreaseCacheCounters(const EventLabel * /*lab*/) const override {}
	void maybeIncreaseCacheCounters(const EventLabel * /*lab*/) const override {}
};

/* A dependency-tracking graph whose events all live in thread 0 (after the
 * initializer), so no thread bookkeeping gets in the way */
struct Hollow {
	StubChecker checker;
	DepExecutionGraph g{ExecutionGraph::Config{.consChecker = &checker}};

	static auto loc(unsigned idx) -> SAddr { return SAddr::createStatic(0, idx, false, false); }
	auto next() const -> Event { return Event(0, g.getThreadSize(0)); }

	auto read(SAddr addr, EventLabel *rf, const EventDeps &deps = {}) -> ReadLabel *
	{
		auto *rLab = g.add(ReadLabel::create(next(), MemOrdering::Relaxed, addr, ASize(4),
						     nullptr, std::optional<Annotation>{}, deps));
		rLab->setRf(rf);
		return rLab;
	}
	auto write(SAddr addr, int val, const EventDeps &deps = {}) -> WriteLabel *
	{
		auto *wLab = g.add(WriteLabel::create(next(), MemOrdering::Relaxed, addr, ASize(4),
						      SVal(val), deps));
		wLab->addCo(g.co_max(addr));
		return wLab;
	}
	auto fence() -> FenceLabel *
	{
		return g.add(FenceLabel::create(next(), MemOrdering::SequentiallyConsistent));
	}

	/* A view keeping exactly the given thread-0 positions (plus the initializer) */
	static auto keep(std::initializer_list<int> idxs) -> DepView
	{
		DepView v;
		v.setMax(Event(0, 0));
		for (auto i : idxs)
			v.setMax(Event(0, i));
		return v;
	}
};

auto depsOn(Event e) -> EventDeps
{
	EventDeps d;
	d.data = DepInfo(e);
	return d;
}
auto ctrlOn(Event e) -> EventDeps
{
	EventDeps d;
	d.ctrl = DepInfo(e);
	return d;
}
auto addrOn(Event e) -> EventDeps
{
	EventDeps d;
	d.addr = DepInfo(e);
	return d;
}

} // namespace

/* R x; W y (data on R x); R z (ctrl on R x); F  -- deleting R x skeletonizes W y
 * (only its value changes) but removes R z and everything po-after it */
TEST(DeleteEventsTest, RemovesFromFirstIdentityChange)
{
	Hollow h;
	auto *init = h.g.getInitLabel();
	auto *rx = h.read(Hollow::loc(1), init);
	auto *wy = h.write(Hollow::loc(2), 1, depsOn(rx->getPos()));
	h.read(Hollow::loc(3), init, ctrlOn(rx->getPos()));
	h.fence();
	ASSERT_EQ(h.g.getThreadSize(0), 5);

	h.g.deleteEvents(Hollow::keep({}), nullptr);

	/* The last label stays concrete, so the trailing skeleton W y is popped too */
	EXPECT_EQ(h.g.getThreadSize(0), 1);
	(void)wy;
}

/* R x; W y (data on R x); W z; R w -- with R w kept, the skeletons in between stay */
TEST(DeleteEventsTest, SkeletonizesBetweenKeptEvents)
{
	Hollow h;
	auto *init = h.g.getInitLabel();
	auto *rx = h.read(Hollow::loc(1), init);
	h.write(Hollow::loc(2), 1, depsOn(rx->getPos()));
	h.write(Hollow::loc(3), 1);
	auto *rw = h.read(Hollow::loc(4), init);

	h.g.deleteEvents(Hollow::keep({4}), nullptr);

	ASSERT_EQ(h.g.getThreadSize(0), 5);
	for (auto i = 1; i <= 3; i++)
		EXPECT_TRUE(h.g.getEventLabel(Event(0, i))->isSkeleton());
	EXPECT_FALSE(rw->isSkeleton());
	EXPECT_EQ(h.g.getReadLabel(Event(0, 1))->getRf(), nullptr);
	EXPECT_FALSE(h.g.getWriteLabel(Event(0, 2))->isInCo());
	EXPECT_EQ(h.g.co_max(Hollow::loc(2)), h.g.getInitLabel());
	EXPECT_EQ(rw->getRf(), init);
}

/* W y; R y (rfi) -- deleting W y leaves R y dangling onto the skeleton */
TEST(DeleteEventsTest, KeepsDanglingRfi)
{
	Hollow h;
	auto *wy = h.write(Hollow::loc(2), 1);
	auto *ry = h.read(Hollow::loc(2), wy);
	h.fence(); /* keeps R y from being the last label */

	h.g.deleteEvents(Hollow::keep({2, 3}), nullptr);

	EXPECT_TRUE(wy->isSkeleton());
	EXPECT_FALSE(ry->isSkeleton());
	EXPECT_EQ(ry->getRf(), wy);
	EXPECT_EQ(std::ranges::distance(wy->readers()), 1);
	EXPECT_FALSE(wy->isInCo());
}

/* R x (kept, about to be revisited); W y (data on R x); R y (rfi); R z (addr on R y)
 * -- the changed value reaches R z through data;rfi, so R z is removed */
TEST(DeleteEventsTest, ChangedReadTaintsThroughDataRfi)
{
	Hollow h;
	auto *init = h.g.getInitLabel();
	auto *rx = h.read(Hollow::loc(1), init);
	auto *wy = h.write(Hollow::loc(2), 1, depsOn(rx->getPos()));
	auto *ry = h.read(Hollow::loc(2), wy);
	h.read(Hollow::loc(3), init, addrOn(ry->getPos()));
	h.fence();

	h.g.deleteEvents(Hollow::keep({1, 3}), rx);

	EXPECT_EQ(h.g.getThreadSize(0), 4);
	EXPECT_FALSE(rx->isSkeleton());
	EXPECT_TRUE(wy->isSkeleton());
	EXPECT_FALSE(ry->isSkeleton());
	EXPECT_EQ(ry->getRf(), wy);
}

/* W x; W x -- a deleted store leaves co and its stamp keeps indexing it */
TEST(DeleteEventsTest, SkeletonWriteLeavesCoKeepsStamp)
{
	Hollow h;
	auto *w1 = h.write(Hollow::loc(1), 1);
	auto *w2 = h.write(Hollow::loc(1), 2);
	h.fence();

	h.g.deleteEvents(Hollow::keep({2, 3}), nullptr);

	EXPECT_TRUE(w1->isSkeleton());
	EXPECT_FALSE(w1->isInCo());
	EXPECT_TRUE(w1->hasStamp());
	EXPECT_EQ(h.g.co_max(Hollow::loc(1)), w2);
	EXPECT_EQ(h.g.co_imm_pred(w2), nullptr);
}

/* T0: TC(1); TJ(1) and T1: TS; TF -- deleting the join pops it (a trailing skeleton),
 * which must drop the child finish's pointer to it (a later copy dereferences it) */
TEST(DeleteEventsTest, PoppedSkeletonJoinUnlinksChildFinish)
{
	Hollow h;
	ThreadInfo info;
	info.id = 1;
	info.parentId = 0;
	auto *tcLab = h.g.add(ThreadCreateLabel::create(h.next(), info));
	h.g.addNewThread();
	auto *tsLab =
		h.g.add(ThreadStartLabel::create(Event(1, 0), tcLab->getPos(), tcLab, info, -1));
	auto *tfLab = h.g.add(ThreadFinishLabel::create(Event(1, 1), SVal(0)));
	auto *tjLab = h.g.add(ThreadJoinLabel::create(h.next(), 1));
	tfLab->setParentJoin(tjLab);

	auto keep = Hollow::keep({1});
	keep.setMax(Event(1, 1));
	h.g.deleteEvents(keep, nullptr);

	EXPECT_EQ(h.g.getThreadSize(0), 2);
	EXPECT_EQ(tfLab->getParentJoin(), nullptr);
	EXPECT_EQ(tsLab->getCreate(), tcLab);
}

/* T0: TC(1); F and T1: TS; F -- deleting the create skeletonizes it and its begin
 * (the thread's events go, the thread stays, pointing at the skeleton create) */
TEST(DeleteEventsTest, SkeletonizesCreateWithBegin)
{
	Hollow h;
	ThreadInfo info;
	info.id = 1;
	info.parentId = 0;
	auto *tcLab = h.g.add(ThreadCreateLabel::create(h.next(), info));
	h.g.addNewThread();
	auto *tsLab =
		h.g.add(ThreadStartLabel::create(Event(1, 0), tcLab->getPos(), tcLab, info, -1));
	h.g.add(FenceLabel::create(Event(1, 1), MemOrdering::SequentiallyConsistent));
	h.fence(); /* keeps TC from being the last label */

	h.g.deleteEvents(Hollow::keep({2}), nullptr);

	EXPECT_EQ(h.g.getNumThreads(), 2);
	EXPECT_TRUE(tcLab->isSkeleton());
	EXPECT_TRUE(tsLab->isSkeleton());
	EXPECT_EQ(h.g.getThreadSize(1), 1);
	EXPECT_EQ(tsLab->getCreate(), tcLab);
}

/* T0: R x; TC(1) (ctrl on R x) and T1: TS; F -- deleting R x removes the create, so
 * the thread goes back to its begin and, being trailing, is dropped */
TEST(DeleteEventsTest, RemovedCreateDropsThread)
{
	Hollow h;
	auto *rx = h.read(Hollow::loc(1), h.g.getInitLabel());
	ThreadInfo info;
	info.id = 1;
	info.parentId = 0;
	auto *tcLab = h.g.add(ThreadCreateLabel::create(h.next(), info, ctrlOn(rx->getPos())));
	h.g.addNewThread();
	h.g.add(ThreadStartLabel::create(Event(1, 0), tcLab->getPos(), tcLab, info, -1));
	h.g.add(FenceLabel::create(Event(1, 1), MemOrdering::SequentiallyConsistent));

	h.g.deleteEvents(Hollow::keep({}), nullptr);

	EXPECT_EQ(h.g.getNumThreads(), 1);
	EXPECT_EQ(h.g.getThreadSize(0), 1);
}

/* W y (skeleton); R y (rfi); F -- a copy keeps the skeleton in po only, with its reader */
TEST(DeleteEventsTest, CloneKeepsSkeletons)
{
	Hollow h;
	auto *wy = h.write(Hollow::loc(2), 1);
	h.read(Hollow::loc(2), wy);
	h.fence();
	h.g.deleteEvents(Hollow::keep({2, 3}), nullptr);

	auto copy = h.g.clone();

	auto *cwLab = copy->getWriteLabel(Event(0, 1));
	auto *crLab = copy->getReadLabel(Event(0, 2));
	EXPECT_TRUE(cwLab->isSkeleton());
	EXPECT_FALSE(cwLab->isInCo());
	EXPECT_TRUE(cwLab->hasStamp());
	EXPECT_EQ(crLab->getRf(), cwLab);
	EXPECT_EQ(std::ranges::distance(cwLab->readers()), 1);
	EXPECT_EQ(&*cwLab->readers().begin(), crLab);
	EXPECT_TRUE(
		std::ranges::none_of(copy->labels(), [](auto &lab) { return lab.isSkeleton(); }));
}
