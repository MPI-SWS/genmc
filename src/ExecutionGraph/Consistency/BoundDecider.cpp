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

#include "BoundDecider.hpp"
#include "ExecutionGraph/Consistency/ContextBoundDecider.hpp"
#include "ExecutionGraph/Consistency/RoundBoundDecider.hpp"
#include "Verification/Config.hpp"

auto BoundDecider::doesExecutionExceedBound(const ExecutionGraph &g, unsigned int bound,
					    BoundCalculationStrategy strategy) -> bool
{
	setGraph(&g);
	auto slack = (strategy == BoundCalculationStrategy::Slacked) ? getSlack() : 0;
	auto ret = doesExecutionExceedBound(bound + slack);
	setGraph(nullptr);
	return ret;
}

auto BoundDecider::create(BoundType type) -> std::unique_ptr<BoundDecider>
{
	switch (type) {
	case BoundType::context:
		return std::make_unique<ContextBoundDecider>();
	case BoundType::round:
		return std::make_unique<RoundBoundDecider>();
	default:
		BUG();
	}
}

#ifdef ENABLE_GENMC_DEBUG
auto BoundDecider::calculate(const ExecutionGraph &g) -> unsigned
{
	setGraph(&g);
	auto ret = calculate();
	setGraph(nullptr);
	return ret;
}
#endif
