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

#ifndef GENMC_SVAL_HPP
#define GENMC_SVAL_HPP

#include "Error.hpp"

#include <climits>
#include <cstdint>
#include <cstring>
#include <format>

/**
 * Represents a value to be written to memory. All values are represented as
 * integers, and the interpreter has to convert these values to values of
 * the appropriate type.
 */
class SVal {

public:
	/* We represent Values using a type that is big-enough
	 * to accommodate for all the types we are interested in */
	using Value = uint64_t;
	static constexpr unsigned width = sizeof(Value) * CHAR_BIT;

	/** Constructors/destructors */
	constexpr SVal() : value(0), provenance(0) {}
	constexpr explicit SVal(uint64_t v) : value(v), provenance(0) {}
	constexpr SVal(Value value, Value prov) : value(value), provenance(prov) {}

	/** Returns a (limited) representation of this value */
	[[nodiscard]] auto get() const -> uint64_t { return value; }

	/** Returns a (limited) signed representation of this value */
	[[nodiscard]] auto getSigned() const -> int64_t
	{
		int64_t tmp;
		std::memcpy(&tmp, &value, sizeof(tmp));
		return tmp;
	}

	/** Returns a pointer representation of this value */
	[[nodiscard]] auto getPointer() const -> void * { return (void *)(uintptr_t)value; }

	/** Returns a (limited) representation of the Value as a boolean */
	[[nodiscard]] auto getBool() const -> bool { return (!!*this); }

	/** Get any provanence information for this value */
	[[nodiscard]] auto getProvenance() const -> uint64_t { return provenance; }

	/** Sign-extends the number in the bottom B bits of X to SVal::width
	 * Pre: 0 < B <= SVal::width */
	auto signExtendBottom(unsigned b) -> SVal &
	{
		ASSERT(b != 0 && b <= width);
		value = int64_t(get() << (width - b)) >> (width - b);
		return *this;
	}

	/** Truncates the value to the specified width W.
	 * Pre: 0 < W <= SVal::width */
	auto truncate(unsigned w) -> SVal &
	{
		ASSERT(w != 0 && w <= width);
		value = w == 64 ? get() : (get() & ((uint64_t(1) << w) - 1));
		return *this;
	}

	/** Equality operators */

	auto operator==(const SVal &v) const -> bool { return v.value == value; }
	auto operator!=(const SVal &v) const -> bool { return !(*this == v); }

	/** Comparison operators */

	/** Returns true if *this < v if both are considered unsigned */
	[[nodiscard]] auto ult(const SVal &v) const -> bool { return compare(v) < 0; }

	/** Returns true if *this < v if both are considered signed */
	[[nodiscard]] auto slt(const SVal &v) const -> bool { return compareSigned(v) < 0; }

	/** Returns true if *this <= v when both are considered unsigned */
	[[nodiscard]] auto ule(const SVal &v) const -> bool { return compare(v) <= 0; }

	/** Returns true if *this <= RHS when both are considered signed */
	[[nodiscard]] auto sle(const SVal &v) const -> bool { return compareSigned(v) <= 0; }

	/** Returns true if *this > RHS when both are considered unsigned */
	[[nodiscard]] auto ugt(const SVal &v) const -> bool { return !ule(v); }

	/** Returns true if *this > RHS when both are considered signed */
	[[nodiscard]] auto sgt(const SVal &v) const -> bool { return !sle(v); }

	/** Returns true if *this >= RHS when both are considered unsigned */
	[[nodiscard]] auto uge(const SVal &v) const -> bool { return !ult(v); }

	/** Returns true if *this >= RHS when both are considered signed */
	[[nodiscard]] auto sge(const SVal &v) const -> bool { return !slt(v); }

	/** Binary operators */

#define IMPL_BINOP(_op)                                                                            \
	SVal operator _op(const SVal &v) const                                                     \
	{                                                                                          \
		SVal n(*this);                                                                     \
		n.value _op## = v.value;                                                           \
		return n;                                                                          \
	}                                                                                          \
	SVal &operator _op##=(const SVal & v)                                                      \
	{                                                                                          \
		value _op## = v.value;                                                             \
		return *this;                                                                      \
	}

	IMPL_BINOP(+);
	IMPL_BINOP(-);
	IMPL_BINOP(*);
	IMPL_BINOP(/);
	IMPL_BINOP(%);
	IMPL_BINOP(&);
	IMPL_BINOP(|);
	IMPL_BINOP(^);
	IMPL_BINOP(<<);
	IMPL_BINOP(>>);

	auto operator~() const -> SVal { return SVal(~this->value); }

	explicit operator bool() const { return !!this->value; }

	[[nodiscard]] auto toString(bool sign = false) const -> std::string
	{
		return sign ? std::to_string(getSigned()) : std::to_string(get());
	}

private:
	[[nodiscard]] auto compare(const SVal &v) const -> int
	{
		return this->value < v.value ? -1 : this->value > v.value;
	}

	[[nodiscard]] auto compareSigned(const SVal &v) const -> int
	{
		auto lhsSext = getSigned();
		auto rhsSext = v.getSigned();
		return lhsSext < rhsSext ? -1 : lhsSext > rhsSext;
	}

	/** The actual value */
	Value value;

	/** Pointer provenance; binary operations preserve LHS provenance */
	Value provenance;
};

/** Comparator for SVal */
struct SValUCmp {
	auto operator()(const SVal &lhs, const SVal &rhs) -> bool { return lhs.ult(rhs); }
};

/** Make `SVal` formattable with `std::format`. */
template <> struct std::formatter<SVal> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const SVal &v, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}", v.get());
	}
};

#endif /* GENMC_SVAL_HPP */
