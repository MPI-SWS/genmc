#include <initializer_list>

#include <gtest/gtest.h>

#include "genmc/ADT/View.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Execution/ExecutionState.hpp"
#include "genmc/Support/ASize.hpp"
#include "genmc/Support/MemAccess.hpp"
#include "genmc/Support/SAddr.hpp"

static auto viewOf(std::initializer_list<Event> es) -> View
{
	View v;
	for (const auto &e : es)
		v.setMax(e);
	return v;
}

static void write(ExecutionState &st, Event pos, const AAccess &access)
{
	st.onATStore(pos, access, viewOf({pos}), false, false);
}

/* Every byte of the access needs a write in hb */
TEST(ExecutionStateTest, EachByteNeedsHbBeforeWriter)
{
	ExecutionState st;
	auto low = AAccess(SAddr(0x100), ASize(2));
	auto high = AAccess(SAddr(0x102), ASize(2));
	auto whole = AAccess(SAddr(0x100), ASize(4));

	write(st, Event(1, 1), low);
	write(st, Event(2, 1), high);
	write(st, Event(2, 2), whole);

	EXPECT_TRUE(st.isInitializedFor(low, viewOf({Event(1, 1)})));
	EXPECT_FALSE(st.isInitializedFor(whole, viewOf({Event(1, 1)})));
	EXPECT_TRUE(st.isInitializedFor(whole, viewOf({Event(2, 2)})));
	EXPECT_TRUE(st.isInitializedFor(whole, viewOf({Event(1, 1), Event(2, 1)})));
}
