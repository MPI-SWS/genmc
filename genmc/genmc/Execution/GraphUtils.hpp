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

#ifndef GENMC_GRAPH_UTILS_HPP
#define GENMC_GRAPH_UTILS_HPP

#include "EventLabel.hpp"
#include "genmc/Support/MemAccess.hpp"
#include "genmc/Support/SVal.hpp"

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

/** Returns true if the addition of SLAB violates atomicity in the graph */
auto violatesAtomicity(const WriteLabel *sLab) -> bool;

/** Given a write SLAB, if SLAB is an RMW, returns the write of a conflicting RMW.
 * If SLAB is not an RMW or no conflicting RMW exists, returns nullptr.
 * If multiple conflicting RMWs exist, returns the one with the smallest stamp.
 * (The latter case might only occur as part of some tiebraking condition.) */
auto findPendingRMW(WriteLabel *sLab) -> WriteLabel *;
auto findPendingRMW(const WriteLabel *sLab) -> const WriteLabel *;

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

/** Returns the initializing value for a barrier event.
 * Assumes there is exactly one such event */
auto findBarrierInitValue(const BIncFaiWriteLabel *wLab) -> SVal;

/** Retuns whether WLAB is the final write in a barrier round */
auto isLastInBarrierRound(const BIncFaiWriteLabel *wLab) -> bool;

/** Retuns whether RLAB reads a barrier-unblocking value */
auto readsBarrierUnblockingValue(const BWaitReadLabel *rLab) -> bool;

/** Blocks thread with BLAB. BLAB needs to either replace the last label or be maximal */
void blockThread(ExecutionGraph &g, std::unique_ptr<BlockLabel> bLab);

/** Unblocks thread at Event `pos`. The given thread must be blocked. */
void unblockThread(ExecutionGraph &g, Event pos);

/** If rLab is the read part of a successful RMW operation, this function creates the corresponding
 * write label, but does NOT add it to the graph. Returns an empty `unique_ptr` if the event was not
 * an RMW, or was an unsuccessful one. */
auto createRMWWriteLabel(const ExecutionGraph &g, const ReadLabel *rLab)
	-> std::unique_ptr<WriteLabel>;

#endif /* GENMC_GRAPH_UTILS_HPP */
