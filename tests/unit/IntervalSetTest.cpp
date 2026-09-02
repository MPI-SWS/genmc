#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

#include "genmc/ADT/IntervalSet.hpp"

#include <vector>

using namespace genmc;

/* Basic sanity checks */
TEST(IntervalSetUnitTest, BasicOperations)
{
	IntervalSet<int> s;

	s.insert(10, 20);
	EXPECT_TRUE(s.contains(10));
	EXPECT_TRUE(s.contains(15));
	EXPECT_TRUE(s.contains(19));
	EXPECT_FALSE(s.contains(9));
	EXPECT_FALSE(s.contains(20));

	s.insert(20, 30);
	/* Should be coalesced: [10, 30) */
	EXPECT_TRUE(s.contains(Interval{10, 30}));
	EXPECT_TRUE(s.contains(20));

	s.erase(15, 25);
	/* Expected segments: [10, 15) and [25, 30) */
	EXPECT_TRUE(s.contains(10));
	EXPECT_FALSE(s.contains(15));
	EXPECT_FALSE(s.contains(20));
	EXPECT_FALSE(s.contains(24));
	EXPECT_TRUE(s.contains(25));
	EXPECT_TRUE(s.contains(29));

	auto it = s.begin();
	ASSERT_NE(it, s.end());
	EXPECT_EQ((*it).start, 10);
	EXPECT_EQ((*it).end, 15);
	++it;
	ASSERT_NE(it, s.end());
	EXPECT_EQ((*it).start, 25);
	EXPECT_EQ((*it).end, 30);
	++it;
	EXPECT_EQ(it, s.end());
}

TEST(IntervalSetUnitTest, IntersectsAndContains)
{
	IntervalSet<int> s;
	s.insert(10, 20);
	s.insert(30, 40);

	EXPECT_TRUE(s.intersects(5, 15));
	EXPECT_TRUE(s.intersects(15, 25));
	EXPECT_TRUE(s.intersects(10, 20));
	EXPECT_FALSE(s.intersects(20, 30));
	EXPECT_TRUE(s.intersects(35, 45));

	EXPECT_TRUE(s.contains(Interval{12, 18}));
	EXPECT_TRUE(s.contains(Interval{10, 20}));
	EXPECT_FALSE(s.contains(Interval{10, 21}));
	EXPECT_FALSE(s.contains(Interval{5, 15}));
	EXPECT_FALSE(s.contains(Interval{15, 35}));
}

/* Check conformance wrt oracle */
class SetOracle {
public:
	SetOracle(size_t size) : memory_(size, false) {}

	void insert(size_t start, size_t end)
	{
		for (auto i = start; i < end; ++i) {
			if (i < memory_.size())
				memory_[i] = true;
		}
	}

	void erase(size_t start, size_t end)
	{
		for (auto i = start; i < end; ++i) {
			if (i < memory_.size())
				memory_[i] = false;
		}
	}

	auto contains(size_t index) const -> bool
	{
		return index >= memory_.size() ? false : memory_[index];
	}

private:
	std::vector<bool> memory_;
};

RC_GTEST_PROP(IntervalSetPropertyTest, MatchesOracleBehavior, ())
{
	const size_t domain_size = 100;

	IntervalSet<size_t> sut;
	SetOracle oracle(domain_size);

	auto num_ops = *rc::gen::inRange(20, 100);
	for (auto i = 0; i < num_ops; ++i) {
		auto start = *rc::gen::inRange<size_t>(0, domain_size - 1);
		auto end = *rc::gen::inRange<size_t>(start + 1, domain_size);

		if (*rc::gen::arbitrary<bool>()) {
			sut.insert(start, end);
			oracle.insert(start, end);
		} else {
			sut.erase(start, end);
			oracle.erase(start, end);
		}
	}

	for (auto i = 0; i < domain_size; ++i)
		RC_ASSERT(sut.contains(i) == oracle.contains(i));

#ifdef ENABLE_GENMC_DEBUG
	auto err = sut.validate();
	if (err.has_value())
		RC_FAIL("Interval set invariant broken: " + *err);
#endif
}
