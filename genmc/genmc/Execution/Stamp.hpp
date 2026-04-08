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

#ifndef GENMC_STAMP_HPP
#define GENMC_STAMP_HPP

#include "genmc/Support/Error.hpp"

#include <cstdint>
#include <format>

/**
 * Represents a label stamp (positive number).
 */
class Stamp {

protected:
	/* Assuming re-assigning, uint32_t should suffice */
	using Value = uint32_t;

public:
	/* Constructors/destructors */
	Stamp() = delete;
	constexpr Stamp(uint32_t v) : value(v) {}
	constexpr Stamp(const Stamp &other) = default;
	constexpr Stamp(Stamp &&other) = default;

	~Stamp() = default;

	auto operator=(const Stamp &other) -> Stamp & = default;
	auto operator=(Stamp &&other) -> Stamp & = default;

	auto operator<=>(const Stamp &other) const = default;

#define IMPL_STAMP_BINOP(_op)                                                                      \
	Stamp &operator _op##=(uint32_t v)                                                         \
	{                                                                                          \
		value _op## = v;                                                                   \
		return *this;                                                                      \
	}                                                                                          \
	Stamp operator _op(uint32_t v) const                                                       \
	{                                                                                          \
		Stamp n(*this);                                                                    \
		n _op## = v;                                                                       \
		return n;                                                                          \
	}

	IMPL_STAMP_BINOP(+);
	IMPL_STAMP_BINOP(-);

	auto operator++() -> Stamp & { return (*this) += 1; }
	auto operator++(int) -> Stamp
	{
		auto tmp = *this;
		++*this;
		return tmp;
	}

	auto operator--() -> Stamp & { return (*this) -= 1; }
	auto operator--(int) -> Stamp
	{
		auto tmp = *this;
		--*this;
		return tmp;
	}

	/* Type-system hole */
	[[nodiscard]] auto get() const -> uint32_t { return value; }
	auto operator()() const -> uint32_t { return get(); }

private:
	Value value;
};

/** Make `Stamp` formattable with `std::format`. */
template <> struct std::formatter<Stamp> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Stamp &s, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}", s.get());
	}
};

#endif /* GENMC_STAMP_HPP */
