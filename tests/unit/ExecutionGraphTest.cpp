#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "StubChecker.hpp"
#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/GraphUtils.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/MemoryModel.hpp"

struct TestGraph {
	StubChecker checker;
	ExecutionGraph graph{ExecutionGraph::Config{.consChecker = &checker}};

	/* Appends a fence to thread 0 */
	auto addFence() -> EventLabel *
	{
		auto pos = Event(0, graph.getThreadSize(0));
		return graph.add(FenceLabel::create(pos, MemOrdering::SequentiallyConsistent));
	}
};

/* On an uncut graph, getEventLabelIfPresent() agrees with getEventLabel() */
TEST(ExecutionGraphTest, IfPresentMatchesGetEventLabelWhenUncut)
{
	TestGraph g;

	for (auto i = 0; i < 4; i++)
		g.addFence();

	for (auto i = 0; i < g.graph.getThreadSize(0); i++) {
		auto e = Event(0, i);
		EXPECT_EQ(g.graph.getEventLabelIfPresent(e), g.graph.getEventLabel(e));
		EXPECT_EQ(g.graph.getEventLabelIfPresent(e)->getPos(), e);
	}
}

/* Positions outside the graph yield nullptr instead of UB */
TEST(ExecutionGraphTest, IfPresentReturnsNullForAbsentPositions)
{
	TestGraph g;

	g.addFence();

	auto size = g.graph.getThreadSize(0);
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(0, size)), nullptr);
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(0, size + 42)), nullptr);
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(0, -1)), nullptr);
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(-1, 0)), nullptr);
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(g.graph.getNumThreads(), 0)), nullptr);
}

/* Locations with no recorded views yield empty views */
TEST(ExecutionGraphTest, InitLocViewsEmptyByDefault)
{
	TestGraph g;
	const auto *iLab = g.graph.getInitLabel();

	EXPECT_TRUE(iLab->locView(SAddr(0x100), 0).empty());
	EXPECT_TRUE(iLab->locView(SAddr(0x100), 3).empty());
}

/* Each loc-view update replaces the previously stored view */
TEST(ExecutionGraphTest, InitLocViewsTrackLatestWrite)
{
	TestGraph g;
	auto *iLab = g.graph.getInitLabel();
	auto addr = SAddr(0x100);

	View v1;
	v1.setMax(Event(1, 4));
	iLab->setLocView(addr, 1, v1);
	EXPECT_EQ(iLab->locView(addr, 1).getMax(1), 4);

	View v2;
	v2.setMax(Event(2, 7));
	iLab->setLocView(addr, 1, v2);

	const auto &latest = iLab->locView(addr, 1);
	EXPECT_EQ(latest.getMax(1), 0);
	EXPECT_EQ(latest.getMax(2), 7);

	/* Other indices and other locations remain empty */
	EXPECT_TRUE(iLab->locView(addr, 0).empty());
	EXPECT_TRUE(iLab->locView(SAddr(0x200), 1).empty());
}

/* Calculating the SC closure doesn't pull in unrelated threads/events.
 *
 * TC  F
 * Wx1 Rx1
 */
TEST(ExecutionGraphTest, SCPrefixClosureFollowsPoRfTc)
{
	TestGraph g;
	auto addr = SAddr(0x100);

	auto tinfo = ThreadInfo{1, 0, 0, SVal(0), "t1"};
	auto *tcLab = g.graph.add(ThreadCreateLabel::create(Event(0, 1), tinfo));
	auto *wLab = genmc::cast<WriteLabel>(g.graph.add(WriteLabel::create(
		Event(0, 2), MemOrdering::SequentiallyConsistent, addr, ASize(4), SVal(1))));
	wLab->addCo(g.graph.co_max(addr));

	g.graph.addNewThread();
	g.graph.add(ThreadStartLabel::create(Event(1, 0), tcLab->getPos(),
					     genmc::cast<ThreadCreateLabel>(tcLab), tinfo));
	auto *fLab =
		g.graph.add(FenceLabel::create(Event(1, 1), MemOrdering::SequentiallyConsistent));
	auto *rLab = g.graph.add(ReadLabel::create(Event(1, 2), MemOrdering::SequentiallyConsistent,
						   addr, ASize(4), wLab, std::nullopt));

	/* Closing the fence pulls the creator's prefix, but not the write */
	auto vFence = calcSCPrefixClosure(g.graph, {fLab});
	EXPECT_TRUE(vFence.containsStrict(Event(1, 1)));
	EXPECT_FALSE(vFence.containsStrict(Event(1, 2)));
	EXPECT_TRUE(vFence.containsStrict(Event(0, 1)));
	EXPECT_FALSE(vFence.containsStrict(Event(0, 2)));

	/* Closing the read additionally pulls its rf-source's prefix */
	auto vRead = calcSCPrefixClosure(g.graph, {rLab});
	EXPECT_TRUE(vRead.containsStrict(Event(1, 2)));
	EXPECT_TRUE(vRead.containsStrict(Event(0, 2)));
}

/* Gives LAB n views, with view 0 having MARK as thread 0's maximum */
static void seedViews(EventLabel *lab, size_t n, int mark = 0)
{
	auto views = std::vector<View>(n);
	views[0].setMax(Event(0, mark));
	lab->setViews(std::move(views));
}

/* Check that graph cutting (1) prunes the events, (2) updates the
 * initializer's cache, (3) redirects orphaned readers, and (4) keeps
 * lookups sound.
 *
 * Wx1 Rx1   (Rx1 reads Wx1; Wx1 gets pruned)
 * Wx2
 */
TEST(ExecutionGraphTest, CutToViewPrunesAndRedirects)
{
	TestGraph g;
	auto addr = SAddr(0x100);
	auto *iLab = g.graph.getInitLabel();
	seedViews(iLab, 2);

	auto *w1 = genmc::cast<WriteLabel>(g.graph.add(WriteLabel::create(
		Event(0, 1), MemOrdering::SequentiallyConsistent, addr, ASize(4), SVal(1))));
	w1->addCo(g.graph.co_max(addr));
	seedViews(w1, 2, 7);
	auto *w2 = genmc::cast<WriteLabel>(g.graph.add(WriteLabel::create(
		Event(0, 2), MemOrdering::SequentiallyConsistent, addr, ASize(4), SVal(2))));
	w2->addCo(g.graph.co_max(addr));
	seedViews(w2, 2, 9);

	g.graph.addNewThread();
	auto *tsLab = g.graph.add(ThreadStartLabel::create(Event(1, 0), Event(0, 0), nullptr,
							   ThreadInfo{1, 0, 0, SVal(0), "t1"}));
	seedViews(tsLab, 2);
	auto *rLab = genmc::cast<ReadLabel>(
		g.graph.add(ReadLabel::create(Event(1, 1), MemOrdering::SequentiallyConsistent,
					      addr, ASize(4), w1, std::nullopt)));
	seedViews(rLab, 2);

	/* Remove exactly w1 (a predecessor-closed prefix) */
	View v;
	v.updateIdx(Event(0, 2));
	auto removed = g.graph.cutToView(v);
	EXPECT_EQ(removed, 1U);

	/* Lookups: w1 is gone, survivors keep their positions */
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(0, 1)), nullptr);
	ASSERT_NE(g.graph.getEventLabelIfPresent(Event(0, 2)), nullptr);
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(0, 2))->getPos(), Event(0, 2));
	EXPECT_EQ(g.graph.getEventLabelIfPresent(Event(1, 1)), rLab);

	/* Position-based lookups remain sound for the survivors */
	EXPECT_TRUE(g.graph.wasPruned());
	EXPECT_EQ(g.graph.getEventLabel(Event(0, 2))->getPos(), Event(0, 2));
	EXPECT_TRUE(g.graph.containsPos(Event(0, 2)));
	EXPECT_FALSE(g.graph.containsPos(Event(0, 1)));

	/* w1 (the co-max pruned write) updated the initializer's cache for x... */
	EXPECT_EQ(iLab->locView(addr, 0).getMax(0), 7);

	/* ...and its views were restored onto thread 0's start (the initializer) */
	EXPECT_EQ(iLab->view(0).getMax(0), 7);

	/* ...and the orphaned reader now reads "init" */
	EXPECT_TRUE(genmc::isa<InitLabel>(rLab->getRf()));
	EXPECT_FALSE(iLab->rfs(addr).empty());

	/* Thread 0 is still runnable (no terminator was cut) */
	EXPECT_FALSE(g.graph.getFirstThreadLabel(0)->isTerminated());
}

/* After a cut, an acquire read that reads "init" at a pruned location must
 * inherit what the pruned release write had released (through the
 * initializer's per-location cache, read by the generated view calculation
 * at rf edges).
 *
 * Wx42   Rz0acq   (the writes get pruned; the reads read "init")
 * Wy1rel Ry1acq
 */
TEST(ExecutionGraphTest, InitReadersInheritPrunedViews)
{
	Config conf;
	conf.model = ModelType::RC11;
	auto checker = ConsistencyChecker::create(&conf);
	ExecutionGraph g{ExecutionGraph::Config{.consChecker = &*checker, .emitNALabels = true}};

	auto xAddr = SAddr(0x100);
	auto yAddr = SAddr(0x200);
	auto zAddr = SAddr(0x300);

	/* t0: x = 42 (na); y = 1 (rel) */
	auto *wx = genmc::cast<WriteLabel>(g.add(WriteLabel::create(
		Event(0, 1), MemOrdering::NotAtomic, xAddr, ASize(4), SVal(42))));
	wx->addCo(g.co_max(xAddr));
	checker->updateMMViews(wx);
	auto *wy = genmc::cast<WriteLabel>(g.add(
		WriteLabel::create(Event(0, 2), MemOrdering::Release, yAddr, ASize(4), SVal(1))));
	wy->addCo(g.co_max(yAddr));
	checker->updateMMViews(wy);

	/* Prune both writes; the cache at y now stands for the release write */
	View v;
	v.updateIdx(Event(0, 3));
	g.cutToView(v);

	/* t1: first a control acquire of an untouched location, then an
	 * acquire read of y; both read "init" */
	g.addNewThread();
	auto *tsLab = g.add(ThreadStartLabel::create(Event(1, 0), Event(0, 0), nullptr,
						     ThreadInfo{1, 0, 0, SVal(0), "t1"}));
	checker->updateMMViews(tsLab);

	/* Control: acquiring "init" at an untouched location acquires nothing */
	auto *rz = genmc::cast<ReadLabel>(g.add(ReadLabel::create(
		Event(1, 1), MemOrdering::Acquire, zAddr, ASize(4), nullptr, std::nullopt)));
	rz->setRf(g.getInitLabel());
	checker->updateMMViews(rz);
	EXPECT_FALSE(checker->getHbView(rz).contains(Event(0, 1)));

	auto *ry = genmc::cast<ReadLabel>(g.add(ReadLabel::create(
		Event(1, 2), MemOrdering::Acquire, yAddr, ASize(4), nullptr, std::nullopt)));
	ry->setRf(g.getInitLabel());
	checker->updateMMViews(ry);

	/* The read acquired the pruned write's release view */
	EXPECT_TRUE(checker->getHbView(ry).contains(Event(0, 1)));
	EXPECT_TRUE(checker->getHbView(ry).contains(Event(0, 2)));
}

/* Cutting a thread's terminator marks the (surviving) start label.
 *
 *  F
 *  TE
 */
TEST(ExecutionGraphTest, CutToViewTerminatesFullyCutThreads)
{
	TestGraph g;
	seedViews(g.graph.getInitLabel(), 2);

	g.graph.addNewThread();
	auto *tsLab = g.graph.add(ThreadStartLabel::create(Event(1, 0), Event(0, 0), nullptr,
							   ThreadInfo{1, 0, 0, SVal(0), "t1"}));
	seedViews(tsLab, 2);
	auto *fLab =
		g.graph.add(FenceLabel::create(Event(1, 1), MemOrdering::SequentiallyConsistent));
	seedViews(fLab, 2);
	auto *eLab = g.graph.add(ThreadFinishLabel::create(Event(1, 2), SVal(0)));
	seedViews(eLab, 2, 5);

	View v;
	v.updateIdx(Event(1, 3));
	auto removed = g.graph.cutToView(v);
	EXPECT_EQ(removed, 2U);

	EXPECT_EQ(g.graph.getThreadSize(1), 1);
	EXPECT_TRUE(g.graph.getFirstThreadLabel(1)->isTerminated());
	/* The start label stands for the removed prefix */
	EXPECT_EQ(g.graph.getFirstThreadLabel(1)->view(0).getMax(0), 5);
	/* A second cut with nothing to remove is a no-op */
	EXPECT_EQ(g.graph.cutToView(v), 0U);
}

/* Without NA labels, reading "init" at a pruned location reconstructs the
 * value against the pruned write's cached hb view: NA writes the pruned
 * write had overwritten must not resurface.
 *
 * Wx7na Wx42   (Wx42 is hb-after the NA write and gets pruned;
 *                the NA write exists only in the state, not the graph)
 */
TEST(ExecutionGraphTest, PrunedInitReadsUseFoldedHbView)
{
	StubChecker checker;
	ExecutionState st;
	ExecutionGraph g{ExecutionGraph::Config{
		.execState = &st, .consChecker = &checker, .emitNALabels = false}};
	auto addr = SAddr(0x100);
	auto access = AAccess(addr, ASize(4));
	seedViews(g.getInitLabel(), 1);

	/* The NA write of 7 at (0, 1), recorded in the state only */
	View naView;
	naView.setMax(Event(0, 1));
	st.onNAStore(Event(0, 1), access, naView, SVal(7));

	/* The atomic write of 42 that overwrote it (hb view covers the NA) */
	g.addNewThread();
	auto *tsLab = g.add(ThreadStartLabel::create(Event(1, 0), Event(0, 0), nullptr,
						     ThreadInfo{1, 0, 0, SVal(0), "t1"}));
	seedViews(tsLab, 1);
	auto *w = genmc::cast<WriteLabel>(g.add(WriteLabel::create(
		Event(1, 1), MemOrdering::SequentiallyConsistent, addr, ASize(4), SVal(42))));
	w->addCo(g.co_max(addr));
	View hb;
	hb.setMax(Event(0, 1));
	hb.setMax(Event(1, 1));
	w->setViews({hb}); /* view 0 is the stub's hb view */
	st.onATStore(Event(1, 1), access, hb, false, false);

	/* Prune the write; reading "init" must yield 42, not the stale 7 */
	View v;
	v.updateIdx(Event(1, 2));
	g.cutToView(v);
	EXPECT_EQ(g.getInitLabel()->getAccessValue(access).get(), 42U);
}

/* Byte-wise reads survive a location whose atomic writes were all pruned
 * (empty coherence): the value comes from the initializer's cache.
 *
 *  Wx42    (gets pruned; co(x) becomes empty)
 */
TEST(ExecutionGraphTest, CurrentMemValueSurvivesFullyPrunedCo)
{
	StubChecker checker;
	ExecutionState st;
	ExecutionGraph g{ExecutionGraph::Config{
		.execState = &st, .consChecker = &checker, .emitNALabels = false}};
	auto addr = SAddr(0x100);
	auto access = AAccess(addr, ASize(4));
	seedViews(g.getInitLabel(), 1);

	auto *w = genmc::cast<WriteLabel>(g.add(WriteLabel::create(
		Event(0, 1), MemOrdering::SequentiallyConsistent, addr, ASize(4), SVal(42))));
	w->addCo(g.co_max(addr));
	View hb;
	hb.setMax(Event(0, 1));
	w->setViews({hb}); /* view 0 is the stub's hb view */
	st.onATStore(Event(0, 1), access, hb, false, false);

	View v;
	v.updateIdx(Event(0, 2));
	g.cutToView(v);
	EXPECT_EQ(g.getCurrentMemValue(addr, ASize(4)).get(), 42U);
}
