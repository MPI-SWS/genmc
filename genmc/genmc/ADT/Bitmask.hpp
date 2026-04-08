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

DEFINE_ASSIGNMENT_OPERATOR(|)
DEFINE_ASSIGNMENT_OPERATOR(&)
DEFINE_ASSIGNMENT_OPERATOR(^)

#endif /* GENMC_BITMASK_HPP */
