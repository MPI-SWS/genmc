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

#include "Verification/ChoiceMap.hpp"

#include <algorithm>

void ChoiceMap::update(const ReadLabel *rLab, const std::vector<EventLabel *> &stores)
{
	auto &choices = cmap_[rLab->getPos()];
	std::vector<Event> storeEvents;
	std::ranges::transform(stores, std::back_inserter(storeEvents),
			       [](auto &wLab) { return wLab->getPos(); });
	choices = std::move(storeEvents);
}

void ChoiceMap::update(const WriteLabel *wLab, const std::vector<EventLabel *> &stores)
{
	auto &choices = cmap_[wLab->getPos()];
	std::vector<Event> storeEvents;
	std::ranges::transform(stores, std::back_inserter(storeEvents),
			       [](auto &wLab) { return wLab->getPos(); });
	choices = std::move(storeEvents);
}

void ChoiceMap::update(const std::vector<ReadLabel *> &loads, const WriteLabel *sLab)
{
	for (const auto *rLab : loads) {
		cmap_[rLab->getPos()].insert(sLab->getPos());
	}
}

void ChoiceMap::cut(const VectorClock &v)
{
	for (auto it = cmap_.begin(); it != cmap_.end();) {
		if (!v.contains(it->first)) {
			it = cmap_.erase(it);
			continue;
		}

		VSet<Event> toRemove;
		for (const auto &e : it->second)
			if (!v.contains(e))
				toRemove.insert(e);
		it->second.erase(toRemove);
		++it;
	}
}
