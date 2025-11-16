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

#include "NameInfo.hpp"

#include <algorithm>

/* Mark name at offset OFFSET as NAME */
void NameInfo::addOffsetInfo(unsigned int offset, std::string name)
{
	info.emplace_back(offset, name);

	/* Check if info needs sorting */
	if (info.size() <= 1)
		return;
	if (info[info.size() - 2].first < info[info.size() - 1].first)
		std::ranges::sort(info);
}

/* Returns name at offset O */
auto NameInfo::getNameAtOffset(unsigned int offset) const -> std::string
{
	if (info.empty())
		return "";

	for (auto i = 0U; i < info.size(); i++) {
		if (info[i].first > offset) {
			BUG_ON(i == 0);
			return info[i - 1].second;
		}
	}
	return info.back().second;
}
