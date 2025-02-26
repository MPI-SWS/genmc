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

#include "Support/SAddrAllocator.hpp"
#include "ADT/VectorClock.hpp"

void SAddrAllocator::restrict(const VectorClock &view)
{
	for (auto &[tid, index] : dynamicPool_) {
		index = std::max(1, view.getMax(tid)); // don't allocate null
	}
}

auto operator<<(llvm::raw_ostream &rhs, const SAddrAllocator &alloctor) -> llvm::raw_ostream &
{
	rhs << "static: ";
	for (const auto &[tid, idx] : alloctor.staticPool_)
		rhs << "(" << tid << ", " << idx << ") ";
	rhs << "\ndynamic: ";
	for (const auto &[tid, idx] : alloctor.dynamicPool_)
		rhs << "(" << tid << ", " << idx << ") ";
	return rhs << "\n";
}
