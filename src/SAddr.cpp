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

#include "SAddr.hpp"

llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const SAddr &addr)
{
	auto internal = addr.isInternal() ? "I" : "U";

	if (addr.isStatic())
		s << "G" << internal << "#" << (addr.get() & SAddr::addressMask);
	else if (addr.isAutomatic())
		s << "S" << internal << "#" << (addr.get() & SAddr::addressMask);
	else if (addr.isHeap())
		s << "H" << internal << "#" << (addr.get() & SAddr::addressMask);
	else
		BUG();
	return s;
}
