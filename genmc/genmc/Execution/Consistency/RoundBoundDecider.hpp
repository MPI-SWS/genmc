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

#ifndef GENMC_ROUND_BOUND_DECIDER_HPP
#define GENMC_ROUND_BOUND_DECIDER_HPP

#include "genmc/config.h"

#include "BoundDecider.hpp"
#include "genmc/Execution/Event.hpp"

/** Bound the number of round-robin scheduling rounds */
class RoundBoundDecider : public BoundDecider {

public:
	RoundBoundDecider() = default;

private:
	[[nodiscard]] auto doesExecutionExceedBound(unsigned int bound) const -> bool override;
#ifdef ENABLE_GENMC_DEBUG
	[[nodiscard]] auto calculate() const -> unsigned override;
#endif
};

#endif /* GENMC_ROUND_BOUND_DECIDER_HPP */
