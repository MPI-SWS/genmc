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

#ifndef GENMC_BOUND_DECIDER_HPP
#define GENMC_BOUND_DECIDER_HPP

#include "genmc/config.h"

#include <memory>

class ExecutionGraph;
enum class BoundType : std::uint8_t;

enum class BoundCalculationStrategy { Slacked, NonSlacked };

/** Abstract class for bounding the model checker's search space */
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
