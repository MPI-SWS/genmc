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

#ifndef GENMC_RMWOPS_HPP
#define GENMC_RMWOPS_HPP

#include "Support/ASize.hpp"
#include "Support/Error.hpp"
#include "Support/SVal.hpp"

#include <cstdint>

enum class RMWBinOp : std::uint8_t {
	Xchg = 0,
	Add = 1,
	Sub = 2,
	And = 3,
	Nand = 4,
	Or = 5,
	Xor = 6,
	Max = 7,
	Min = 8,
	UMax = 9,
	UMin = 10,
	LAST = UMin,
};

/* Helper to validate unknown rmw op types */
template <typename Int>
requires std::integral<Int>
inline auto isValidRMWBinOp(Int i) -> bool
{
	return static_cast<Int>(RMWBinOp::Xchg) <= i && i <= static_cast<Int>(RMWBinOp::LAST);
}

inline auto executeRMWBinOp(SVal oldVal, SVal val, ASize size, RMWBinOp op) -> SVal
{
	switch (op) {
	case RMWBinOp::Xchg:
		return val;
	case RMWBinOp::Add:
		return (oldVal + val);
	case RMWBinOp::Sub:
		return (oldVal - val);
	case RMWBinOp::And:
		return oldVal & val;
	case RMWBinOp::Nand:
		return ~(oldVal & val);
	case RMWBinOp::Or:
		return oldVal | val;
	case RMWBinOp::Xor:
		return oldVal ^ val;
	case RMWBinOp::Max:
		return SVal(oldVal).signExtendBottom(size.getBits())
				       .sgt(SVal(val).signExtendBottom(size.getBits()))
			       ? oldVal
			       : val;
	case RMWBinOp::Min:
		return SVal(oldVal).signExtendBottom(size.getBits())
				       .slt(SVal(val).signExtendBottom(size.getBits()))
			       ? oldVal
			       : val;
	case RMWBinOp::UMax:
		return SVal(oldVal).truncate(size.getBits()).ugt(SVal(val).truncate(size.getBits()))
			       ? oldVal
			       : val;
	case RMWBinOp::UMin:
		return SVal(oldVal).truncate(size.getBits()).ult(SVal(val).truncate(size.getBits()))
			       ? oldVal
			       : val;
	default:
		WARN_ONCE("invalid-rmw-op", "Unsupported operation in RMW instruction!\n");
		return val;
	}
}

#endif /* GENMC_RMWOPS_HPP */
