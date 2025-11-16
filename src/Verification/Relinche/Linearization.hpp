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

#ifndef GENMC_LINEARIZATION_HPP
#define GENMC_LINEARIZATION_HPP

#include "ADT/IndexedMap.hpp"
#include "Verification/Relinche/Observation.hpp"

#include <format>
#include <vector>

/** Represents a total ordering of MethodCall */
class Linearization {
private:
	using Index = unsigned int; /**< Index in a linear order */

public:
	using PermutationMap = genmc::IndexedMap<MethodCall::Id>;

	Linearization() = default;
	explicit Linearization(const std::vector<MethodCall::Id> &orderedIds);
	explicit Linearization(std::vector<MethodCall::Id> &&orderedIds);

	[[nodiscard]] auto begin() const { return lin_.begin(); }
	[[nodiscard]] auto end() const { return lin_.end(); }

	[[nodiscard]] auto empty() const { return lin_.empty(); }

	[[nodiscard]] auto size() const { return lin_.size(); }

	[[nodiscard]] auto isBefore(MethodCall::Id id1, MethodCall::Id id2) const -> bool
	{
		return getIndex(id1) < getIndex(id2);
	}

	auto applyPermutation(const PermutationMap &pMap) -> Linearization;

	[[nodiscard]] auto operator<=>(const Linearization &other) const = default;

	friend struct std::formatter<Linearization>;

private:
	/** Index of an event in the order */
	[[nodiscard]] auto getIndex(MethodCall::Id id) const -> Index { return order_[id.value()]; }

	/** Ordered CallIds representing the linearization */
	std::vector<MethodCall::Id> lin_;

	/** A map so that we have O(1) complexity when comparing order of CallIds in
	 * a linearization. This is actually an indexed map (CallIx -> Index), but we
	 * don't use an IndexedMap because we need comparison operators for Linearizations */
	std::vector<Index> order_;
};

/** Make `Linearization` formattable with `std::format`. */
template <> struct std::formatter<Linearization> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Linearization &lin, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}: {}", lin.lin_.size(), lin.lin_);
	}
};

#endif /* GENMC_LINEARIZATION_HPP */
