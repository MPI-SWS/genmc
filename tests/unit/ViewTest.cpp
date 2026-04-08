#include <algorithm>
#include <exception>
#include <map>
#include <vector>

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

#include "genmc/ADT/View.hpp"

/* Models a view using a map */
class Oracle {
public:
	void updateIdx(Event e) { map_[e.thread] = std::max(map_[e.thread], e.index); }

	void setMax(Event e)
	{
		if (e.index == 0) {
			map_.erase(e.thread);
		} else {
			map_[e.thread] = e.index;
		}
	}

	[[nodiscard]] auto getMax(int thread) const -> int
	{
		auto it = map_.find(thread);
		return (it == map_.end()) ? 0 : it->second;
	}

private:
	std::map<int, int> map_;
};

/* A type for view operations (update vs set) */
struct ViewOp {
	enum Kind { UpdateIdx, SetMax } kind;
	Event e;
};

namespace rc {

template <> struct Arbitrary<Event> {
	static Gen<Event> arbitrary()
	{
		/* Try to force collisions */
		return gen::construct<Event>(gen::inRange(0, 5), gen::inRange(0, 100));
	}
};

template <> struct Arbitrary<ViewOp> {
	static Gen<ViewOp> arbitrary()
	{
		return gen::construct<ViewOp>(gen::element(ViewOp::UpdateIdx, ViewOp::SetMax),
					      gen::arbitrary<Event>());
	}
};

} /* namespace rc */

/* Checks on empty view */
TEST(ViewUnitTest, EmptyView)
{
	View v;
	EXPECT_TRUE(v.empty());
	EXPECT_EQ(v.size(), 0);
	EXPECT_EQ(v.getMax(0), 0);
	EXPECT_EQ(v.getMax(42), 0);
}

/* Single updateIdx operation */
TEST(ViewUnitTest, SingleUpdateAndContains)
{
	View v;
	v.updateIdx({1, 10});

	EXPECT_FALSE(v.empty());
	EXPECT_EQ(v.getMax(1), 10);
	EXPECT_TRUE(v.contains({1, 10}));
	EXPECT_TRUE(v.contains(Event(1, 5)));
	EXPECT_FALSE(v.contains(Event(1, 11)));
	EXPECT_FALSE(v.contains(Event(4, 2)));
	EXPECT_TRUE(v.contains(Event(4, 0))); /* 0-idx special case */
}

TEST(ViewUnitTest, CoWIsolation)
{
	View v1;
	v1.updateIdx({0, 10});

	View v2 = v1; /* v2 and v1 share base */
	EXPECT_EQ(v2.getMax(0), 10);

	/* Should trigger hydration */
	v2.updateIdx({0, 20});

	/* v1 must remain unchanged */
	EXPECT_EQ(v2.getMax(0), 20);
	EXPECT_EQ(v1.getMax(0), 10);
}

TEST(ViewUnitTest, SetMaxResetsValue)
{
	View v;
	v.setMax({0, 20});
	EXPECT_EQ(v.getMax(0), 20);

	/* Overwriting with lower value forces hydration (diffs don't handle lowering) */
	v.setMax({0, 5});

	EXPECT_EQ(v.getMax(0), 5);
}

TEST(ViewUnitTest, DiffCollisionForcesHydration)
{
	/* v1: Diff(T1, 10) */
	View v1;
	v1.updateIdx({1, 10});

	/* v2: Diff(T2, 20) */
	View v2;
	v2.updateIdx({2, 20});

	/* T1 != T2 => we can't merge diffs */
	v1.update(v2);

	EXPECT_EQ(v1.getMax(1), 10);
	EXPECT_EQ(v1.getMax(2), 20);
}

TEST(ViewUnitTest, UpdateIdxZeroIsNoop)
{
	View v;
	v.updateIdx({10, 0});

	EXPECT_EQ(v.size(), 0) << "updateIdx(0) should be a noop";
	EXPECT_TRUE(v.empty());

	v.updateIdx({10, 5});
	EXPECT_EQ(v.size(), 11);
	EXPECT_EQ(v.getMax(10), 5);
}

TEST(ViewUnitTest, TrailingZeros)
{
	/* Grow view */
	View v;
	v.setMax({10, 5});
	ASSERT_EQ(v.size(), 11);
	ASSERT_EQ(v.getMax(10), 5);

	/* Reset highest element */
	v.setMax({10, 0});

	/* Size should shrink */
	EXPECT_EQ(v.size(), 0) << "View failed to trim trailing zeros";
	EXPECT_TRUE(v.empty());
}

TEST(ViewUnitTest, TrimmingMiddleZeroDoesNotShrink)
{
	View v;
	v.setMax({10, 5});
	v.setMax({5, 5});

	/* Reset highest element */
	v.setMax({10, 0});

	/* Should shrink to size 6 (not 0) */
	EXPECT_EQ(v.size(), 6);
	EXPECT_EQ(v.getMax(5), 5);
}

TEST(ViewUnitTest, TrailingZerosDoNotInflateSizeCheck)
{
	/* If trailing zeros affect size, contains() checks can fail */
	View vEmptyButLarge;
	vEmptyButLarge.updateIdx({100, 0}); /* should have size 0 */

	View vRealData;
	vRealData.updateIdx({0, 5}); /* size 1 */

	/* If size() optimization sees 101 > 1, the check below might falsely fail */
	EXPECT_TRUE(vRealData.contains(vEmptyButLarge));
}

/*
 * Property: Conformance wrt oracle
 */
RC_GTEST_PROP(ViewPropertyTest, MatchesOracleBehavior, (const std::vector<ViewOp> &ops))
{
	View sut;
	Oracle oracle;

	for (const auto &op : ops) {
		if (op.kind == ViewOp::UpdateIdx) {
			sut.updateIdx(op.e);
			oracle.updateIdx(op.e);
		} else {
			sut.setMax(op.e);
			oracle.setMax(op.e);
		}
	}

	for (auto i = 0; i < 10; ++i)
		RC_ASSERT(sut.getMax(i) == oracle.getMax(i));
}

/*
 * Property: Canonical representation (trailing zeros are trimmed)
 */
RC_GTEST_PROP(ViewPropertyTest, TrailingZerosIndependence, (const std::vector<Event> &events))
{
	View v1;
	for (const auto &e : events)
		v1.updateIdx(e);

	View v2 = v1;

	/* Reset high indices */
	v2.setMax({50, 0});
	v2.setMax({100, 0});

	/* Check logical and physical equality */
	RC_ASSERT(v1 == v2);
	RC_ASSERT(v1.size() == v2.size());
}

/*
 * Property: Update commutativity
 */
RC_GTEST_PROP(ViewPropertyTest, UpdateCommutativity,
	      (const std::vector<Event> &evs1, const std::vector<Event> &evs2))
{
	View v1, v2;
	for (const auto &e : evs1)
		v1.updateIdx(e);
	for (const auto &e : evs2)
		v2.updateIdx(e);

	View join1 = v1;
	join1.update(v2);

	View join2 = v2;
	join2.update(v1);

	RC_ASSERT(join1 == join2);
}

/*
 * Property: Contains transivity (view grows)
 */
RC_GTEST_PROP(ViewPropertyTest, ContainsTransitivity,
	      (const std::vector<Event> &base_evs, const std::vector<Event> &extra_evs))
{
	View v1;
	for (const auto &e : base_evs)
		v1.updateIdx(e);

	/* Create a superset of v1 */
	View v2 = v1;
	for (const auto &e : extra_evs)
		v2.updateIdx(e);

	RC_ASSERT(v2.contains(v1));
}
