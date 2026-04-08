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

#include "genmc/Verification/ChoiceMap.hpp"

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
