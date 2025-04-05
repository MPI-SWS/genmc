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
 * Author: Pavel Golovin <pgolovin@mpi-sws.org>
 */

#ifndef GENMC_LINEARIZATION_HPP
#define GENMC_LINEARIZATION_HPP

#include "Verification/Relinche/Observation.hpp"

#include <llvm/ADT/IndexedMap.h>

#include <vector>

/** Represents a total ordering of MethodCall */
class Linearization {
private:
	using Index = unsigned int; /**< Index in a linear order */

public:
	using PermutationMap = llvm::IndexedMap<MethodCall::Id>;

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

	friend auto operator<<(llvm::raw_ostream &os, const Linearization &lin)
		-> llvm::raw_ostream &;

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

#endif /* GENMC_LINEARIZATION_HPP */
