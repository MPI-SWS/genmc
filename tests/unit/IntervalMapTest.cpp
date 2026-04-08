#include <exception>
#include <vector>

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

#include "genmc/ADT/IntervalMap.hpp"

/* An oracle interval-map implementation that maps every
 * point in the domain to a value. */
class Oracle {
public:
	Oracle(size_t size) : memory_(size, 0) {}

	void add(size_t start, size_t end, int val)
	{
		for (auto i = start; i < end; ++i) {
			if (i < memory_.size())
				memory_[i] += val;
		}
	}

	auto operator[](size_t index) const -> int
	{
		return index >= memory_.size() ? 0 : memory_[index];
	}

private:
	std::vector<int> memory_;
};

/* Basic sanity checks */
TEST(IntervalMapUnitTest, BasicOverlap)
{
	genmc::IntervalMap<int, int> m(0);
	m.add(1, 5, 10);
	m.add(3, 7, 5);

	/* Expected: [1,3)->10, [3,5)->15, [5,7)->5 */
	EXPECT_EQ(m[2], 10);
	EXPECT_EQ(m[4], 15);
	EXPECT_EQ(m[6], 5);
}

TEST(IntervalMapUnitTest, BasicCoalesce)
{
	genmc::IntervalMap<int, int> m(0);

	m.add(40, 42, 100);
	m.add(42, 43, 100);

	/* Expected: [40,43)->100 */
	EXPECT_EQ(m[40], 100);
	EXPECT_EQ(m[41], 100);
	EXPECT_EQ(m[42], 100);
	EXPECT_EQ(m[43], 0);

	genmc::IntervalMap<int, int> n(0);
	n.add(1, 2, 1);
	n.add(1, 2, -1);

	/* Expected: empty */
	EXPECT_EQ(n.empty(), true);

	genmc::IntervalMap<int, bool> p(false);
	p.add(0, 10, true);
	p.add(20, 30, true);
	p.add(0, 30, true);

	/* Expected: coalesced into a single segment [0, 30) */
	EXPECT_EQ(std::distance(p.begin(), p.end()), 1);
	EXPECT_EQ(p[0], true);
	EXPECT_EQ(p[29], true);
	EXPECT_EQ(p[30], false);
}

TEST(IntervalMapUnitTest, MidSegmentBound)
{
	genmc::IntervalMap<int, int> m(0);
	m.add(0, 10, 100);

	/* lower_bound(5, 6) should return the segment starting at 0 */
	auto it = m.lower_bound(5, 6);
	ASSERT_NE(it, m.end());
	EXPECT_EQ(it->first.start, 0);
	EXPECT_EQ(it->second, 100);

	/* lower_bound(10, 11) should return end() if no segments follow */
	EXPECT_EQ(m.lower_bound(10, 11), m.end());

	/* lower_bound(-5, -1) should return the segment starting at 0 */
	it = m.lower_bound(-5, -1);
	ASSERT_NE(it, m.end());
	EXPECT_EQ(it->first.start, 0);
}

/* Check conformance wrt oracle */
RC_GTEST_PROP(IntervalMapPropertyTest, MatchesOracleBehavior, ())
{
	/* Small domain to have frequent collisions */
	const size_t domain_size = 50;

	genmc::IntervalMap<size_t, int> sut(0);
	Oracle oracle(domain_size);

	auto num_ops = *rc::gen::inRange(10, 50);
	for (auto i = 0; i < num_ops; ++i) {
		/* We only construct valid ranges */
		auto start = *rc::gen::inRange<size_t>(0, domain_size - 1);
		auto end = *rc::gen::inRange<size_t>(start + 1, domain_size);
		auto val = *rc::gen::inRange<int>(-10, 10);

		/* Add interval to both implementation */
		sut.add(start, end, val);
		oracle.add(start, end, val);
	}

	/* Check conformance */
	for (auto i = 0; i < domain_size; ++i)
		RC_ASSERT(sut[i] == oracle[i]);

	/* Check internal invariants */
#ifdef ENABLE_GENMC_DEBUG
	auto err = sut.validate();
	if (err.has_value())
		RC_FAIL("Interval map invariant broken: " + *err);
#endif
}
