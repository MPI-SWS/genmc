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

#ifndef GENMC_CONTEXT_BOUND_DECIDER_HPP
#define GENMC_CONTEXT_BOUND_DECIDER_HPP

#include "genmc/config.h"

#include "genmc/ADT/View.hpp"
#include "BoundDecider.hpp"

/** Bound the number of pre-emptive context switches */
class ContextBoundDecider : public BoundDecider {

public:
	ContextBoundDecider() = default;

private:
	[[nodiscard]] auto doesExecutionExceedBound(unsigned int bound) const -> bool override;
	[[nodiscard]] auto doesPrefixExceedBound(View v, int t, unsigned int bound) const -> bool;
	[[nodiscard]] auto getSlack() const -> unsigned override;
#ifdef ENABLE_GENMC_DEBUG
	[[nodiscard]] auto calculate() const -> unsigned override;
	[[nodiscard]] auto calculate(View v, int t) const -> unsigned;
#endif
};

#endif /* GENMC_CONTEXT_BOUND_DECIDER_HPP */
