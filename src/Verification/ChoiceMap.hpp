/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <mixaskok@gmail.com>
 */

#ifndef GENMC_CHOICE_MAP_HPP
#define GENMC_CHOICE_MAP_HPP

#include "ADT/VSet.hpp"
#include "ExecutionGraph/Event.hpp"
#include "ExecutionGraph/EventLabel.hpp"

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
