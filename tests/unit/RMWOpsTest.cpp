#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

#include "genmc/Support/RMWOps.hpp"

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace {

/* Computes the operation OP on the narrow type the access was made with */
template <std::unsigned_integral U> auto narrowRMW(U lhs, U rhs, RMWBinOp op) -> U
{
	using S = std::make_signed_t<U>;
	switch (op) {
	case RMWBinOp::Xchg:
		return rhs;
	case RMWBinOp::Add:
		return static_cast<U>(lhs + rhs);
	case RMWBinOp::Sub:
		return static_cast<U>(lhs - rhs);
	case RMWBinOp::And:
		return lhs & rhs;
	case RMWBinOp::Nand:
		return static_cast<U>(~(lhs & rhs));
	case RMWBinOp::Or:
		return lhs | rhs;
	case RMWBinOp::Xor:
		return lhs ^ rhs;
	case RMWBinOp::Max:
		return static_cast<S>(lhs) > static_cast<S>(rhs) ? lhs : rhs;
	case RMWBinOp::Min:
		return static_cast<S>(lhs) < static_cast<S>(rhs) ? lhs : rhs;
	case RMWBinOp::UMax:
		return lhs > rhs ? lhs : rhs;
	case RMWBinOp::UMin:
		return lhs < rhs ? lhs : rhs;
	}
	return rhs;
}

/* Check a random op on zero-extended operands against the same op on the narrow type */
template <std::unsigned_integral U> void checkMatchesNarrowRMW()
{
	const auto lhs = *rc::gen::arbitrary<U>();
	const auto rhs = *rc::gen::arbitrary<U>();
	const auto op = static_cast<RMWBinOp>(
		*rc::gen::inRange<unsigned>(0, static_cast<unsigned>(RMWBinOp::LAST) + 1));

	const auto result = executeRMWBinOp(SVal(lhs), SVal(rhs), ASize(sizeof(U)), op);
	RC_ASSERT(result.get() == static_cast<uint64_t>(narrowRMW(lhs, rhs, op)));
}

} // namespace

RC_GTEST_PROP(RMWOpsPropertyTest, MatchesNarrowRMW8, ()) { checkMatchesNarrowRMW<uint8_t>(); }

RC_GTEST_PROP(RMWOpsPropertyTest, MatchesNarrowRMW16, ()) { checkMatchesNarrowRMW<uint16_t>(); }

RC_GTEST_PROP(RMWOpsPropertyTest, MatchesNarrowRMW32, ()) { checkMatchesNarrowRMW<uint32_t>(); }

RC_GTEST_PROP(RMWOpsPropertyTest, MatchesNarrowRMW64, ()) { checkMatchesNarrowRMW<uint64_t>(); }
