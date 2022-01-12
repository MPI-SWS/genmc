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
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#include "NameInfo.hpp"

/* Mark name at offset O as N */
void NameInfo::addOffsetInfo(unsigned int o, std::string n)
{
	info.push_back(std::make_pair(o, n));

	/* Check if info needs sorting */
	if (info.size() <= 1)
		return;
	if (info[info.size() - 2].first < info[info.size() - 1].first)
		std::sort(info.begin(), info.end());
}

/* Returns name at offset O */
std::string NameInfo::getNameAtOffset(unsigned int o) const
{
	if (info.empty())
		return "";

	for (auto i = 0u; i < info.size(); i++) {
		if (info[i].first > o) {
			BUG_ON(i == 0);
			return info[i - 1].second;
		}
	}
	return info.back().second;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const NameInfo &info)
{
	for (const auto &kv : info.info)
		rhs << "" << kv.first << ": " << kv.second << "\n";
	return rhs;
}
