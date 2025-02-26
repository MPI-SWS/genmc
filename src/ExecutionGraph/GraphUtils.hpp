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

#ifndef GENMC_GRAPH_UTILS_HPP
#define GENMC_GRAPH_UTILS_HPP

#include "Support/MemAccess.hpp"
#include "Support/SVal.hpp"

class SAddr;
class MemAccessLabel;
class UnlockWriteLabel;
class WriteLabel;
class CasWriteLabel;
class ReadLabel;
class SpeculativeReadLabel;
class MallocLabel;
class EventLabel;
class ExecutionGraph;

/** Returns true if MLAB is protected by a hazptr */
auto isHazptrProtected(const MemAccessLabel *mLab) -> bool;

/** Returns the lock that matches ULAB.
 * If no such event exists, returns nullptr */
auto findMatchingLock(const UnlockWriteLabel *uLab) -> const CasWriteLabel *;

/** Returns the unlock that matches lLAB.
 *If no such event exists, returns nullptr */
auto findMatchingUnlock(const CasWriteLabel *lLab) -> const UnlockWriteLabel *;

/** Helper: Returns the last speculative read in CLAB's location that
 * is not matched. If no such event exists, returns nullptr. */
auto findMatchingSpeculativeRead(const ReadLabel *cLab, const EventLabel *&scLab)
	-> const SpeculativeReadLabel *;

/** Returns the allocating event for ADDR.
 * Assumes that only one such event may exist */
auto findAllocatingLabel(ExecutionGraph &g, const SAddr &addr) -> MallocLabel *;
auto findAllocatingLabel(const ExecutionGraph &g, const SAddr &addr) -> const MallocLabel *;

/** Returns the initializing value for a barrier event.
 * Assumes there is exactly one such event */
auto findBarrierInitValue(const ExecutionGraph &g, const AAccess &access) -> SVal;

#endif /* GENMC_GRAPH_UTILS_HPP */
