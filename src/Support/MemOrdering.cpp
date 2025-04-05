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

#include "Support/MemOrdering.hpp"

auto operator<<(llvm::raw_ostream &rhs, MemOrdering ord) -> llvm::raw_ostream &
{
	switch (ord) {
	case MemOrdering::NotAtomic:
		return rhs << "na";
	case MemOrdering::Relaxed:
		return rhs << "rlx";
	case MemOrdering::Acquire:
		return rhs << "acq";
	case MemOrdering::Release:
		return rhs << "rel";
	case MemOrdering::AcquireRelease:
		return rhs << "ar";
	case MemOrdering::SequentiallyConsistent:
		return rhs << "sc";
	default:
		PRINT_BUGREPORT_INFO_ONCE("print-ordering-type", "Cannot print ordering");
		return rhs;
	}
	return rhs;
}
