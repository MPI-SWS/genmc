/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#ifndef GENMC_RMWOPS_HPP
#define GENMC_RMWOPS_HPP

#include "Support/ASize.hpp"
#include "Support/Error.hpp"
#include "Support/SVal.hpp"

#include <llvm/IR/Instructions.h>

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
		return oldVal.ugt(val) ? oldVal : val;
	case RMWBinOp::UMin:
		return oldVal.ult(val) ? oldVal : val;
	default:
		WARN_ONCE("invalid-rmw-op", "Unsupported operation in RMW instruction!\n");
		return val;
	}
}

/* Translates an LLVM opreation to our internal one; assumes the
 * operation is one we support (i.e., no fops,udecwrap,etc)*/
inline auto fromLLVMBinOp(llvm::AtomicRMWInst::BinOp op) -> RMWBinOp
{
	static const RMWBinOp lookup[11] = {
		/* xchg */ RMWBinOp::Xchg,
		/* add  */ RMWBinOp::Add,
		/* sub  */ RMWBinOp::Sub,
		/* and  */ RMWBinOp::And,
		/* nand */ RMWBinOp::Nand,
		/* or   */ RMWBinOp::Or,
		/* xor  */ RMWBinOp::Xor,
		/* max  */ RMWBinOp::Max,
		/* min  */ RMWBinOp::Min,
		/* umax */ RMWBinOp::UMax,
		/* umin */ RMWBinOp::UMin,
	};
	BUG_ON(!isValidRMWBinOp(
		static_cast<std::underlying_type_t<llvm::AtomicRMWInst::BinOp>>(op)));
	return lookup[static_cast<size_t>(op)];
}

#endif /* GENMC_RMWOPS_HPP */
