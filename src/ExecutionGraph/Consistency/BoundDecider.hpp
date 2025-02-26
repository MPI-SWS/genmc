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

#ifndef GENMC_BOUND_DECIDER_HPP
#define GENMC_BOUND_DECIDER_HPP

#include <memory>

class ExecutionGraph;
enum class BoundType;

enum class BoundCalculationStrategy { Slacked, NonSlacked };

class BoundDecider {

public:
	BoundDecider() = default;
	virtual ~BoundDecider() = default;

	BoundDecider(const BoundDecider &) = delete;
	BoundDecider(BoundDecider &&) = delete;
	auto operator=(const BoundDecider &) -> BoundDecider & = delete;
	auto operator=(BoundDecider &&) -> BoundDecider & = delete;

	static auto create(BoundType type) -> std::unique_ptr<BoundDecider>;

	auto doesExecutionExceedBound(const ExecutionGraph &g, unsigned int bound,
				      BoundCalculationStrategy strategy) -> bool;

#ifdef ENABLE_GENMC_DEBUG
	auto calculate(const ExecutionGraph &g) -> unsigned;
#endif

protected:
	[[nodiscard]] auto getGraph() const -> const ExecutionGraph & { return *graph; }

private:
	void setGraph(const ExecutionGraph *g) { graph = g; }

	[[nodiscard]] virtual auto doesExecutionExceedBound(unsigned int bound) const -> bool = 0;
	[[nodiscard]] virtual auto getSlack() const -> unsigned { return 0; }
#ifdef ENABLE_GENMC_DEBUG
	[[nodiscard]] virtual auto calculate() const -> unsigned = 0;
#endif

	const ExecutionGraph *graph = nullptr;
};

#endif /* GENMC_BOUND_DECIDER_HPP */
