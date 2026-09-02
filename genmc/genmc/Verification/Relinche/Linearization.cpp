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

#include "genmc/Verification/Relinche/Linearization.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Verification/Relinche/Observation.hpp"

#include <utility>
#include <vector>

/* Ctor helper: keep map from callids to index */
static void calculateOrder(const std::vector<MethodCall::Id> &lin, std::vector<unsigned> &order)
{
	order.resize(lin.size());
	for (auto i = 0U; i < lin.size(); ++i) {
		VERIFY(lin[i].value() < lin.size());
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
	VERIFY(pMap.size() == result.size());
	for (auto &id : result.lin_)
		id = pMap[id.value()];
	calculateOrder(result.lin_, result.order_);
	return result;
}
