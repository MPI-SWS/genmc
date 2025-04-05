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

#include "Verification/Relinche/Linearization.hpp"
#include "Support/Error.hpp"

/* Ctor helper: keep map from callids to index */
static void calculateOrder(const std::vector<MethodCall::Id> &lin, std::vector<unsigned> &order)
{
	order.resize(lin.size());
	for (auto i = 0U; i < lin.size(); ++i) {
		BUG_ON(lin[i].value() >= lin.size());
		order[lin[i].value()] = i;
	}
}

Linearization::Linearization(const std::vector<MethodCall::Id> &orderedIds) : lin_(orderedIds)
{
	calculateOrder(lin_, order_);
}

Linearization::Linearization(std::vector<MethodCall::Id> &&orderedIds) : lin_(std::move(orderedIds))
{
	calculateOrder(lin_, order_);
}

auto Linearization::applyPermutation(const PermutationMap &pMap) -> Linearization
{
	auto result(*this);
	BUG_ON(pMap.size() != result.size());
	for (auto &id : result.lin_)
		id = pMap[id.value()];
	calculateOrder(result.lin_, result.order_);
	return result;
}

auto operator<<(llvm::raw_ostream &os, const Linearization &lin) -> llvm::raw_ostream &
{
	os << lin.lin_.size() << ": ";
	os << format(lin.lin_);
	return os;
}
