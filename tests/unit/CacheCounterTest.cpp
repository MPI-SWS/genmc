#include <algorithm>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/GraphUtils.hpp"

/* Counts all non-init labels; unlike isSC(), also matches block and probe labels */
class CountingChecker : public ConsistencyChecker {
public:
	CountingChecker() : ConsistencyChecker(nullptr) {}

	[[nodiscard]] auto getCounter() const -> int { return counter_; }

	static auto matches(const EventLabel *lab) -> bool { return !genmc::isa<InitLabel>(lab); }

	void recomputeCacheCounters(const ExecutionGraph &g) const override
	{
		counter_ = static_cast<int>(
			std::count_if(g.label_begin(), g.label_end(),
				      [](const auto &lab) { return matches(&lab); }));
	}
	void resetCacheCounters() const override { counter_ = -1; }
	void maybeDecreaseCacheCounters(const EventLabel *lab) const override
	{
		if (matches(lab))
			--counter_;
	}
	void maybeIncreaseCacheCounters(const EventLabel *lab) const override
	{
		if (matches(lab))
			++counter_;
	}

	/* Unused below */
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
		return std::make_unique<View>();
	}
	auto getHbView(const EventLabel * /*lab*/) const -> const View & override { return hb_; }
	[[nodiscard]] auto isDepTracking() const -> bool override { return false; }

private:
	mutable int counter_ = 0;
	View hb_;
};

/* A graph with a counting checker attached */
struct CountedGraph {
	CountingChecker checker;
	ExecutionGraph graph{ExecutionGraph::Config{.consChecker = &checker}};

	/* Appends a fence to thread 0 */
	auto addFence() -> EventLabel *
	{
		auto pos = Event(0, graph.getThreadSize(0));
		return graph.add(FenceLabel::create(pos, MemOrdering::SequentiallyConsistent));
	}

	[[nodiscard]] auto matching() const -> int
	{
		return static_cast<int>(
			std::count_if(graph.label_begin(), graph.label_end(), [](const auto &lab) {
				return CountingChecker::matches(&lab);
			}));
	}
};

/* (empty) */
TEST(CacheCounterTest, StartsInSync)
{
	CountedGraph g;

	EXPECT_EQ(g.checker.getCounter(), g.matching());
}

/* (empty) -> F -> F;F -> F;F;F -> F;F;F;F */
TEST(CacheCounterTest, AddCountsLabels)
{
	CountedGraph g;

	for (auto i = 0; i < 4; i++) {
		g.addFence();
		EXPECT_EQ(g.checker.getCounter(), g.matching());
	}
	EXPECT_EQ(g.checker.getCounter(), 4);
}

/* F;F;F;F -> F;F;F -> F;F -> F -> (empty) */
TEST(CacheCounterTest, RemoveLastUncountsLabels)
{
	CountedGraph g;

	for (auto i = 0; i < 4; i++)
		g.addFence();
	while (g.graph.getThreadSize(0) > 1) {
		g.graph.removeLast(0);
		EXPECT_EQ(g.checker.getCounter(), g.matching());
	}
	EXPECT_EQ(g.checker.getCounter(), 0);
}

/* (empty) -> F -> (empty): a scoped label hands ownership back when the guard dies,
 * so the counter has to as well */
TEST(CacheCounterTest, ScopedLabelIsBalanced)
{
	CountedGraph g;
	auto lab = FenceLabel::create(Event(0, 1), MemOrdering::SequentiallyConsistent);

	{
		auto guard = g.graph.addScoped(lab);
		EXPECT_EQ(g.checker.getCounter(), 1);
	}
	EXPECT_TRUE(lab);
	EXPECT_EQ(g.checker.getCounter(), g.matching());
	EXPECT_EQ(g.checker.getCounter(), 0);
}

/* (empty) -> F: whereas a committed one stays in the graph, and stays counted */
TEST(CacheCounterTest, CommittedScopedLabelStaysCounted)
{
	CountedGraph g;
	auto lab = FenceLabel::create(Event(0, 1), MemOrdering::SequentiallyConsistent);

	{
		auto guard = g.graph.addScoped(lab);
		guard.commit();
	}
	EXPECT_EQ(g.checker.getCounter(), g.matching());
	EXPECT_EQ(g.checker.getCounter(), 1);
}

/* F -> F;B -> F */
TEST(CacheCounterTest, BlockAndUnblockAreBalanced)
{
	CountedGraph g;
	g.addFence();
	auto pos = Event(0, g.graph.getThreadSize(0));

	blockThread(g.graph, BlockLabel::createAssumeBlock(pos, AssumeType::User));
	EXPECT_EQ(g.checker.getCounter(), g.matching());

	unblockThread(g.graph, pos);
	EXPECT_EQ(g.checker.getCounter(), g.matching());
	EXPECT_EQ(g.checker.getCounter(), 1);
}

/* F;F;F -> F;F;F + a copy of it: copies share the checker, so building one must leave
 * the counter describing the original */
TEST(CacheCounterTest, CopyingTheGraphLeavesTheCounterAlone)
{
	CountedGraph g;

	for (auto i = 0; i < 3; i++)
		g.addFence();
	auto copy = g.graph.clone();
	EXPECT_EQ(g.checker.getCounter(), g.matching());
	EXPECT_EQ(g.checker.getCounter(), 3);
}

/* F;F -> F;F;B -> F;F -> F;F;F -> F;F -> F: whatever ran, recomputing from scratch
 * must not change the cached value */
TEST(CacheCounterTest, RecomputeAgreesWithIncrementalUpdates)
{
	CountedGraph g;
	auto expectNoDrift = [&g] {
		auto cached = g.checker.getCounter();
		g.checker.recomputeCacheCounters(g.graph);
		EXPECT_EQ(cached, g.checker.getCounter());
	};

	g.addFence();
	g.addFence();
	expectNoDrift();

	auto pos = Event(0, g.graph.getThreadSize(0));
	blockThread(g.graph, BlockLabel::createAssumeBlock(pos, AssumeType::User));
	expectNoDrift();
	unblockThread(g.graph, pos);
	expectNoDrift();

	auto probe = FenceLabel::create(pos, MemOrdering::SequentiallyConsistent);
	{
		auto guard = g.graph.addScoped(probe);
		expectNoDrift();
	}
	expectNoDrift();

	g.graph.removeLast(0);
	expectNoDrift();
}

/* F;F;F;F -> F;F: cutting does not report the labels it drops, so the recompute after
 * it has to restore sync */
TEST(CacheCounterTest, RecomputeRestoresTheCounterAfterACut)
{
	CountedGraph g;

	for (auto i = 0; i < 4; i++)
		g.addFence();
	g.graph.cutToStamp(g.graph.getEventLabel(Event(0, 2))->getStamp());
	g.checker.recomputeCacheCounters(g.graph);
	EXPECT_EQ(g.checker.getCounter(), g.matching());
	EXPECT_EQ(g.checker.getCounter(), 2);
}
