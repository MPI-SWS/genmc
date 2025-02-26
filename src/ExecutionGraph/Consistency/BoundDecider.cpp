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
 * Author: Iason Marmanis <imarmanis@mpi-sws.org>
 */

#include "BoundDecider.hpp"
#include "Config/Config.hpp"
#include "ExecutionGraph/Consistency/ContextBoundDecider.hpp"
#include "ExecutionGraph/Consistency/RoundBoundDecider.hpp"

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
