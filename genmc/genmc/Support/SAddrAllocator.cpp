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

#include "genmc/Support/SAddrAllocator.hpp"
#include "genmc/ADT/VectorClock.hpp"

void SAddrAllocator::restrict(const VectorClock &view)
{
	for (auto &[tid, index] : dynamicPool_) {
		index = std::max(1, view.getMax(tid)); // don't allocate null
	}
}
