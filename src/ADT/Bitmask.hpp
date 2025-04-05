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

#ifndef GENMC_BITMASK_HPP
#define GENMC_BITMASK_HPP

#include <type_traits>

template <typename Enum> struct EnableBitmaskOperators {
	static const bool enable = false;
};

#define ENABLE_BITMASK_OPERATORS(T)                                                                \
	template <> struct EnableBitmaskOperators<T> {                                             \
		static const bool enable = true;                                                   \
	};

#define DEFINE_BINARY_OPERATOR(_op)                                                                \
	template <typename Enum>                                                                   \
	auto operator _op(Enum lhs, Enum rhs)                                                      \
		->Enum                                                                             \
	requires EnableBitmaskOperators<Enum>::enable                                              \
	{                                                                                          \
		using underlying = std::underlying_type_t<Enum>;                                   \
		return static_cast<Enum>(static_cast<underlying>(lhs)                              \
						 _op static_cast<underlying>(rhs));                \
	}

DEFINE_BINARY_OPERATOR(|);
DEFINE_BINARY_OPERATOR(&);
DEFINE_BINARY_OPERATOR(^);

template <typename Enum>
auto operator~(Enum rhs) -> Enum
requires EnableBitmaskOperators<Enum>::enable
{
	using underlying = std::underlying_type_t<Enum>;
	return static_cast<Enum>(~static_cast<underlying>(rhs));
}

template <typename Enum>
auto operator!(Enum rhs) -> bool
requires EnableBitmaskOperators<Enum>::enable
{
	return rhs == static_cast<Enum>(0);
}

#define DEFINE_ASSIGNMENT_OPERATOR(_op)                                                            \
	template <typename Enum>                                                                   \
	auto operator _op##=(Enum &lhs, Enum rhs)                                                  \
		->Enum                                                                             \
	requires EnableBitmaskOperators<Enum>::enable                                              \
	{                                                                                          \
		using underlying = std::underlying_type_t<Enum>;                                   \
		lhs = static_cast<Enum>(static_cast<underlying>(lhs)                               \
						_op static_cast<underlying>(rhs));                 \
		return lhs;                                                                        \
	}

DEFINE_ASSIGNMENT_OPERATOR(|);
DEFINE_ASSIGNMENT_OPERATOR(&);
DEFINE_ASSIGNMENT_OPERATOR(^);

#endif /* GENMC_BITMASK_HPP */
