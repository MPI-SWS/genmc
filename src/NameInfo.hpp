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

#ifndef __NAME_INFO_HPP__
#define __NAME_INFO_HPP__

#include "config.h"
#include "Error.hpp"

#include <string>

/*
 * Represents naming information for a specific type/allocation
 */
class NameInfo {

public:
	NameInfo() = default;

	/* Mark name at offset O as N */
	void addOffsetInfo(unsigned int o, std::string n);

	/* Returns name at offset O */
	std::string getNameAtOffset(unsigned int o) const;

	/* Returns the number of different offset information registered */
	size_t size() const { return info.size(); }

	/* Whether we have any information stored */
	bool empty() const { return info.empty(); }

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const NameInfo &info);

private:
	/*
	 * We keep a map (Values -> (offset, name_at_offset)), and after
	 * the interpreter and the variables are allocated and initialized,
	 * we use the map to dynamically find out the name corresponding to
	 * a particular address.
	 */
	using OffsetInfo = std::vector<std::pair<unsigned, std::string > >;

	/* Naming information at different offsets */
	OffsetInfo info;
};

#endif /* __NAME_INFO_HPP__ */
