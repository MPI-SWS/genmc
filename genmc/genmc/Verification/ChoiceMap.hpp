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

#ifndef GENMC_CHOICE_MAP_HPP
#define GENMC_CHOICE_MAP_HPP

#include "genmc/ADT/VSet.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Execution/EventLabel.hpp"

#include <unordered_map>
#include <vector>

/**
 * ChoiceMap class - Maintains alternative exploration options encountered
 * across an execution.
 */
class ChoiceMap {

public:
	ChoiceMap() = default;

	[[nodiscard]] auto begin() const { return cmap_.begin(); }
	auto begin() { return cmap_.begin(); }

	[[nodiscard]] auto end() const { return cmap_.end(); }
	auto end() { return cmap_.end(); }

	/** Registers that RLAB can read from all stores in STORES */
	void update(const ReadLabel *rLab, const std::vector<EventLabel *> &stores);

	/** Registers that each L in LOADS can read from SLAB */
	void update(const std::vector<ReadLabel *> &loads, const WriteLabel *sLab);

	/** Registers that SLAB can be after each S in STORES */
	void update(const WriteLabel *wLab, const std::vector<EventLabel *> &stores);

	void cut(const VectorClock &v);

private:
	std::unordered_map<Event, VSet<Event>> cmap_;
};

#endif /* GENMC_CHOICE_MAP_HPP */
