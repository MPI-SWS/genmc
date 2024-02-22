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
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#ifndef __MODULE_ID_HPP__
#define __MODULE_ID_HPP__

#include <llvm/IR/Value.h>

/*
 * IDInfo struct -- Contains (unique) identification information for
 * some of the module's crucial components (e.g., GVs, functions, etc)
 */
struct ModuleID {
	using ID = unsigned int;

	std::unordered_map<ID, const llvm::Value *> IDV;
	std::unordered_map<const llvm::Value *, ID> VID;

	void clear()
	{
		IDV.clear();
		VID.clear();
	}
};

#endif /* __MODULE_ID_HPP__ */
