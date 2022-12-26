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

#ifndef __GENMC_DRIVER_HPP__
#define __GENMC_DRIVER_HPP__

#include "Config.hpp"
#include "DepInfo.hpp"
#include "EventLabel.hpp"
#include "RevisitSet.hpp"
#include "WorkSet.hpp"
#include <llvm/IR/Module.h>

#include <ctime>
#include <map>
#include <memory>
#include <random>
#include <unordered_set>

namespace llvm {
	struct ExecutionContext;
	class Interpreter;
	struct DynamicComponents;
	using EELocalState = DynamicComponents;
	class EESharedState;
}
class ModuleInfo;
class ExecutionGraph;
class ThreadPool;

class GenMCDriver {

protected:
	using LocalQueueT = std::map<unsigned int, WorkSet>;
	using RevisitSetT = std::map<unsigned int, RevisitSet>;

public:
	/* Verification status.
	 * Public to enable the interpreter utilize it */
	enum class Status {
		VS_OK,
		VS_Safety,
		VS_Recovery,
		VS_Liveness,
		VS_RaceNotAtomic,
		VS_RaceFreeMalloc,
		VS_FreeNonMalloc,
		VS_DoubleFree,
		VS_Allocation,
		VS_InvalidAccessBegin,
		VS_UninitializedMem,
		VS_AccessNonMalloc,
		VS_AccessFreed,
		VS_InvalidAccessEnd,
		VS_InvalidJoin,
		VS_InvalidUnlock,
		VS_InvalidBInit,
		VS_InvalidRecoveryCall,
		VS_InvalidTruncate,
		VS_Annotation,
		VS_MixedSize,
		VS_SystemError,
	};

	/* Verification result */
	struct Result {
		Status status;            /* Verification status */
		unsigned explored;        /* Number of complete executions explored */
		unsigned exploredBlocked; /* Number of blocked executions explored */
		unsigned exploredMoot;
#ifdef ENABLE_GENMC_DEBUG
		unsigned duplicates;      /* Number of duplicate executions explored */
#endif
		std::string message;      /* A message to be printed */

		Result() : status(Status::VS_OK), explored(0), exploredBlocked(0), exploredMoot(0),
#ifdef ENABLE_GENMC_DEBUG
			   duplicates(0),
#endif
			   message() {}

		Result &operator+=(const Result &other) {
			/* Propagate latest error */
			if (other.status != Status::VS_OK) {
				status = other.status;
				message = other.message;
			}
			explored += other.explored;
			exploredBlocked += other.exploredBlocked;
			exploredMoot += other.exploredMoot;
#ifdef ENABLE_GENMC_DEBUG
			duplicates += other.duplicates;
#endif
			return *this;
		}
	};

	/* Represents the exploration state at any given point */
	struct LocalState {
		std::unique_ptr<ExecutionGraph> graph;
		RevisitSetT revset;
		LocalQueueT workqueue;
		std::unique_ptr<llvm::EELocalState> interpState;
		bool isMootExecution;
		Event readToReschedule;
		std::vector<Event> threadPrios;

		/* FIXME: Ensure that move semantics work properly for std::unordered_map<> */
		LocalState() = delete;
		LocalState(std::unique_ptr<ExecutionGraph> g, RevisitSetT &&r,
			   LocalQueueT &&w, std::unique_ptr<llvm::EELocalState> state,
			   bool isMootExecution, Event readToReschedule,
			   const std::vector<Event> &threadPrios);

		~LocalState();
	};
	struct SharedState {
		std::unique_ptr<ExecutionGraph> graph;
		std::unique_ptr<llvm::EESharedState> interpState;

		/* FIXME: Ensure that move semantics work properly for std::unordered_map<> */
		SharedState() = delete;
		SharedState(std::unique_ptr<ExecutionGraph> g,
			    std::unique_ptr<llvm::EESharedState> state);

		~SharedState();
	};


private:
	static bool isInvalidAccessError(Status s) {
		return Status::VS_InvalidAccessBegin <= s &&
			s <= Status::VS_InvalidAccessEnd;
	};

public:
	/*** State-related ***/

	/* FIXME: Document */
	std::unique_ptr<LocalState> releaseLocalState();
	void restoreLocalState(std::unique_ptr<GenMCDriver::LocalState> state);

	std::unique_ptr<SharedState> getSharedState();
	void setSharedState(std::unique_ptr<GenMCDriver::SharedState> state);

	/**** Generic actions ***/

	/* Sets up the next thread to run in the interpreter */
	bool scheduleNext();

	/* Things need to be done before a particular execution starts */
	void handleExecutionBeginning();

	/* Things to do at each step of an execution */
	void handleExecutionInProgress();

	/* Things to do when an execution ends */
	void handleFinishedExecution();

	/* Pers: Functions that run at the start/end of the recovery routine */
	void handleRecoveryStart();
	void handleRecoveryEnd();

	/* Starts the verification procedure for a driver */
	void run();

	/* Stops the verification procedure when an error is found */
	void halt(Status status);

	/* Returns the result of the verification procedure */
	Result getResult() const { return result; }

	/* Creates driver instance(s) and starts verification for the given module. */
	static Result verify(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod);

	/* Gets/sets the thread pool this driver should account to */
	ThreadPool *getThfreadPool() { return pool; }
	ThreadPool *getThreadPool() const { return pool; }
	void setThreadPool(ThreadPool *tp) { pool = tp; }

	/*** Instruction-related actions ***/

	/* Returns the value this load reads */
	SVal visitLoad(std::unique_ptr<ReadLabel> rLab, const EventDeps *deps);

	/* A function modeling a write to disk has been interpreted.
	 * Returns the value read */
	SVal visitDskRead(std::unique_ptr<DskReadLabel> rLab);

	/* A store has been interpreted, nothing for the interpreter */
	void visitStore(std::unique_ptr<WriteLabel> wLab, const EventDeps *deps);

	/* A function modeling a write to disk has been interpreted */
	void visitDskWrite(std::unique_ptr<DskWriteLabel> wLab);

	/* A lock() operation has been interpreted, nothing for the interpreter */
	void visitLock(Event pos, SAddr addr, ASize size, const EventDeps *deps);

	/* An unlock() operation has been interpreted, nothing for the interpreter */
	void visitUnlock(Event pos, SAddr addr, ASize size, const EventDeps *deps);

	/* A helping CAS operation has been interpreter, the result is unobservable */
	void visitHelpingCas(std::unique_ptr<HelpingCasLabel> hLab, const EventDeps *deps);

	/* A function modeling the beginning of the opening of a file.
	 * The interpreter will get back the file descriptor */
	SVal visitDskOpen(std::unique_ptr<DskOpenLabel> oLab);

	/* An fsync() operation has been interpreted */
	void visitDskFsync(std::unique_ptr<DskFsyncLabel> fLab);

	/* A sync() operation has been interpreted */
	void visitDskSync(std::unique_ptr<DskSyncLabel> fLab);

	/* A call to __VERIFIER_pbarrier() has been interpreted */
	void visitDskPbarrier(std::unique_ptr<DskPbarrierLabel> fLab);

	/* A fence has been interpreted, nothing for the interpreter */
	void visitFence(std::unique_ptr<FenceLabel> fLab, const EventDeps *deps);

	/* A call to __VERIFIER_opt_begin() has been interpreted.
	 * Returns whether the block should expand */
	bool
	visitOptional(std::unique_ptr<OptionalLabel> lab);

	/* A call to __VERIFIER_loop_begin() has been interpreted */
	void
	visitLoopBegin(std::unique_ptr<LoopBeginLabel> lab);

	/* A call to __VERIFIER_spin_start() has been interpreted */
	void visitSpinStart(std::unique_ptr<SpinStartLabel> lab);

	/* A call to __VERIFIER_faiZNE_spin_end() has been interpreted */
	void
	visitFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab);

	/* A call to __VERIFIER_lockZNE_spin_end() has been interpreted */
	void
	visitLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab);

	/* A thread has terminated abnormally */
	void
	visitThreadKill(std::unique_ptr<ThreadKillLabel> lab);

	/* Returns an appropriate result for pthread_self() */
	SVal visitThreadSelf(const EventDeps *deps);

	/* Returns the TID of the newly created thread */
	int visitThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab, const EventDeps *deps,
			      llvm::Function *F, SVal arg, const llvm::ExecutionContext &SF);

	/* Returns an appropriate result for pthread_join() */
	SVal visitThreadJoin(std::unique_ptr<ThreadJoinLabel> jLab, const EventDeps *deps);

	/* A thread has just finished execution, nothing for the interpreter */
	void visitThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab);

	/* __VERIFIER_hp_protect() has been called */
	void visitHpProtect(std::unique_ptr<HpProtectLabel> hpLab, const EventDeps *deps);

	/* Returns an appropriate result for malloc() */
	SVal visitMalloc(std::unique_ptr<MallocLabel> aLab, const EventDeps *deps,
			 unsigned int alignment, Storage s, AddressSpace spc);

	/* A call to free() has been interpreted, nothing for the intepreter */
	void visitFree(std::unique_ptr<FreeLabel> dLab, const EventDeps *deps);

	/* This method blocks the current thread  */
	void visitBlock(std::unique_ptr<BlockLabel> bLab);

	/* LKMM: Visit RCU functions */
	void
	visitRCULockLKMM(std::unique_ptr<RCULockLabelLKMM> lab);
	void
	visitRCUUnlockLKMM(std::unique_ptr<RCUUnlockLabelLKMM> lab);
	void
	visitRCUSyncLKMM(std::unique_ptr<RCUSyncLabelLKMM> lab);

	/* This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void visitError(Event pos, Status r, const std::string &err = std::string(),
			Event confEvent = Event::getInitializer());

	virtual ~GenMCDriver();

protected:

	GenMCDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		    std::unique_ptr<ModuleInfo> MI);

	/* No copying or copy-assignment of this class is allowed */
	GenMCDriver(GenMCDriver const&) = delete;
	GenMCDriver &operator=(GenMCDriver const &) = delete;

	/* Returns a pointer to the user configuration */
	const Config *getConf() const { return userConf.get(); }

	/* Returns a pointer to the interpreter */
	llvm::Interpreter *getEE() const { return EE.get(); }

	/* Returns a reference to the current graph */
	ExecutionGraph &getGraph() { return *execGraph; };
	ExecutionGraph &getGraph() const { return *execGraph; };

	/* Given a write event from the graph, returns the value it writes */
	SVal getWriteValue(Event w, SAddr p, AAccess a);
	SVal getWriteValue(const WriteLabel *wLab) {
		return getWriteValue(wLab->getPos(), wLab->getAddr(), wLab->getAccess());
	}

	/* Returns the value written by a disk write */
	SVal getDskWriteValue(Event w, SAddr p, AAccess a);
	SVal getDskWriteValue(const DskWriteLabel *wLab) {
		return getDskWriteValue(wLab->getPos(), wLab->getAddr(), wLab->getAccess());
	}

	/* Returns the value read by a read */
	SVal getReadValue(const ReadLabel *rLab) {
		return getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getAccess());
	}

	/* Returns the value read by a disk read */
	SVal getDskReadValue(const DskReadLabel *rLab) {
		return getDskWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getAccess());
	}

	/* Returns the value that a read is reading. This function should be
	 * used when calculating the value that we should return to the
	 * interpreter; if the read is reading from an invalid place
	 * (e.g., bottom) also blocks the currently running thread. */
	SVal getReadRetValueAndMaybeBlock(const ReadLabel *rLab);
	SVal getRecReadRetValue(const ReadLabel *rLab);

	/* Returns the value with which a barrier at PTR has been initialized */
	SVal getBarrierInitValue(SAddr ptr, AAccess a);

	/* Returns the type of consistency types we need to perform at
	 * P according to the configuration */
	CheckConsType getCheckConsType(ProgramPoint p) const;

	/* Returns true if we should check persistency at p */
	bool shouldCheckPers(ProgramPoint p);

	/* Returns true if the current graph is consistent */
	bool isConsistent(ProgramPoint p);

	/* Pers: Returns true if current recovery routine is valid */
	bool isRecoveryValid(ProgramPoint p);

	/* Liveness: Checks whether a spin-blocked thread reads co-maximal values */
	bool threadReadsMaximal(int tid);

	/* Liveness: Calls visitError() if there is a liveness violation */
	void checkLiveness();

	/* Returns true if A is hb-before B at P */
	bool isHbBefore(Event a, Event b, ProgramPoint p = ProgramPoint::step);

	/* Returns true if E is maximal in ADDR at P*/
	bool isCoMaximal(SAddr addr, Event e, bool checkCache = false,
			 ProgramPoint p = ProgramPoint::step);

private:
	/*** Worklist-related ***/

	/* Adds an appropriate entry to the worklist */
	void addToWorklist(WorkSet::ItemT item);

	/* Fetches the next backtrack option.
	 * A default-constructed item means that the list is empty */
	WorkSet::ItemT getNextItem();

	/* Restricts the worklist only to entries that were added before lab */
	void restrictWorklist(const EventLabel *lab);


	/*** Revisit-related ***/

	/* Returns true if the current revisit set for rLab contains
	 * the pair (writePrefix, moPlacings) */
	bool revisitSetContains(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
				const std::vector<std::pair<Event, Event> > &moPlacings);

	/* Adds to the revisit set of rLab the pair (writePrefix, moPlacings) */
	void addToRevisitSet(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
			     const std::vector<std::pair<Event, Event> > &moPlacings);

	void restrictRevisitSet(const EventLabel *lab);


	/*** Exploration-related ***/

	/* The workhorse for run().
	 * Exhaustively explores all  consistent executions of a program */
	void explore();

	/* Returns true if this driver is shutting down */
	bool isHalting() const;

	/* Returns true if this execution is moot */
	bool isMoot() const { return isMootExecution; }

	/* Opt: Mark current execution as moot/normal */
	void moot() { isMootExecution = true; }
	void unmoot() { isMootExecution = false; }

	/* Opt: Whether the exploration should try to repair R */
	bool isRescheduledRead(Event r) const { return readToReschedule == r; }

	/* Opt: Sets R as a read to be repaired */
	void setRescheduledRead(Event r) { readToReschedule = r; }

	/* Resets some options before the beginning of a new execution */
	void resetExplorationOptions();

	/* Sets up a prioritization scheme among threads */
	void prioritizeThreads();

	/* Deprioritizes the current thread */
	void deprioritizeThread(const UnlockLabelLAPOR *uLab);

	/* Returns true if THREAD is schedulable (i.e., there are more
	 * instructions to run and it is not blocked) */
	bool isSchedulable(int thread) const;

	/* Tries to schedule according to the current prioritization scheme */
	bool schedulePrioritized();

	/* Returns true if the next instruction of TID is a load
	 * Note: assumes there is a next instruction in TID*/
	bool isNextThreadInstLoad(int tid);

	/* Helpers for schedule according to a policy */
	bool scheduleNextLTR();
	bool scheduleNextWF();
	bool scheduleNextRandom();

	/* Tries to schedule the next instruction according to the
	 * chosen policy */
	bool scheduleNormal();

	/* Returns whether the current execution is blocked */
	bool isExecutionBlocked() const;

	/* Opt: Tries to reschedule any reads that were added blocked */
	bool rescheduleReads();

	/* Resets the prioritization scheme */
	void resetThreadPrioritization();

	/* Returns whether LAB accesses a valid location.  */
	bool isAccessValid(const MemAccessLabel *lab);

	/* Checks for data races when a read/write is added, and calls
	 * visitError() if a race is found. */
	void checkForDataRaces(const MemAccessLabel *mlab);

	/* Performs POSIX checks whenever a lock event is added.
	 * Given its list of possible rfs, makes sure it cannot read
	 * from a destroyed lock.
	 * Appropriately calls visitErro() and terminates */
	void checkLockValidity(const ReadLabel *rLab, const std::vector<Event> &rfs);

	/* Performs POSIX checks whenever an unlock event is added.
	 * Appropriately calls visitError() and terminates */
	void checkUnlockValidity(const WriteLabel *wLab);

	/* Perfoms POSIX checks whenever a barrier_init event is added.
	 Appropriately calls visitError() and terminates */
	void checkBInitValidity(const WriteLabel *wLab);

	/* Perfoms POSIX checks whenever a barrier_wait event is added.
	 Appropriately calls visitError() and terminates */
	void checkBIncValidity(const ReadLabel *rLab, const std::vector<Event> &rfs);

	/* Checks whether final annotations are used properly in a program:
	 * if there are more than one stores annotated as final at the time WLAB
	 * is added, visitError() is called */
	void checkFinalAnnotations(const WriteLabel *wLab);

	/* Returns true if MLAB (allocated @ ALAB) is protected by a hazptr */
	bool isHazptrProtected(const MallocLabel *aLab, const MemAccessLabel *mLab) const;

	/* Checks for memory races (e.g., double free, access freed memory, etc)
	 * whenever a read/write/free is added, and calls visitError() if a race is found.
	 * Returns whether a race was found */
	bool checkForMemoryRaces(const MemAccessLabel *mLab);
	bool checkForMemoryRaces(const FreeLabel *lab);

	/* Returns true if the exploration is guided by a graph */
	bool isExecutionDrivenByGraph();

	/* Returns true if we are currently replaying a graph */
	bool inReplay() const;

	/* Pers: Returns true if we are currently running the recovery routine */
	bool inRecoveryMode() const;

	/* If the execution is guided, returns the corresponding label for
	 * this instruction. Reports an error if the execution is not guided */
	const EventLabel *getCurrentLabel() const;

	/* BAM: Tries to optimize barrier-related revisits */
	bool tryOptimizeBarrierRevisits(const BIncFaiWriteLabel *sLab, std::vector<Event> &loads);

	/* Helper: Optimizes revisits of reads that will lead to a failed speculation */
	void optimizeUnconfirmedRevisits(const WriteLabel *sLab, std::vector<Event> &loads);

	/* Helper: Optimize the revisit in case SLAB is a RevBlocker */
	bool tryOptimizeRevBlockerAddition(const WriteLabel *sLab, std::vector<Event> &loads);

	/* Opt: Tries to optimize revisiting from LAB. It may modify
	 * LOADS, and returns whether we can skip revisiting altogether */
	bool tryOptimizeRevisits(const WriteLabel *lab, std::vector<Event> &loads);

	/* Constructs a BackwardRevisit representing RLAB <- SLAB */
	std::unique_ptr<BackwardRevisit>
	constructBackwardRevisit(const ReadLabel *rLab, const WriteLabel *sLab);

	/* Helper: Checks whether the execution should continue upon SLAB revisited LOADS.
	 * Returns true if yes, and false (+moot) otherwise  */
	bool checkRevBlockHELPER(const WriteLabel *sLab, const std::vector<Event> &loads);

	/* Calculates revisit options and pushes them to the worklist.
	 * Returns true if the current exploration should continue */
	bool calcRevisits(const WriteLabel *lab);

	/* Modifies (but not restricts) the graph when we are revisiting a read.
	 * Returns true if the resulting graph should be explored. */
	bool revisitRead(const ReadRevisit &s);

	/* Adjusts the graph and the worklist according to the backtracking option S.
	 * Returns true if the resulting graph should be explored */
	bool restrictAndRevisit(WorkSet::ItemT s);

	/* If rLab is the read part of an RMW operation that now became
	 * successful, this function adds the corresponding write part.
	 * Returns a pointer to the newly added event, or nullptr
	 * if the event was not an RMW, or was an unsuccessful one */
	const WriteLabel *completeRevisitedRMW(const ReadLabel *rLab);

	/* Informs the interpreter that the events *not* contained in V
	 * are being deleted from the execution graph */
	void notifyEERemoved(const VectorClock &v);

	/* Removes all labels with stamp >= st from the graph */
	void restrictGraph(const EventLabel *lab);

	/* Copies the current EG according to BR's view V.
	 * May modify V but will not execute BR in the copy. */
	std::unique_ptr<ExecutionGraph>
	copyGraph(const BackwardRevisit *br, VectorClock *v) const;

	/* Given a list of stores that it is consistent to read-from,
	 * filters out options that can be skipped (according to the conf),
	 * and determines the order in which these options should be explored */
	void filterOptimizeRfs(const ReadLabel *lab, std::vector<Event> &stores);

	/* Removes rfs from "rfs" until a consistent option for rLab is found,
	 * if that is dictated by the CLI options */
	bool ensureConsistentRf(const ReadLabel *rLab, std::vector<Event> &rfs);

	/* Checks whether the addition of WLAB creates an atomicity violation.
	 * If so, returns false and moots the execution if possible. */
	bool checkAtomicity(const WriteLabel *wLab);

	/* Makes sure that the current graph is consistent, if that is dictated
	 * by the CLI options. Since that is not always the case for stores
	 * (e.g., w/ LAPOR), it returns whether it is the case or not */
	bool ensureConsistentStore(const WriteLabel *wLab);

	/* Helper: Annotates a store as RevBlocker, if possible */
	void annotateStoreHELPER(WriteLabel *wLab) const;

	/* Pers: removes _all_ options from "rfs" that make the recovery invalid.
	 * Sets the rf of rLab to the first valid option in rfs */
	void filterInvalidRecRfs(const ReadLabel *rLab, std::vector<Event> &rfs);

	/* SAVer: Checks whether a write has any actual memory effects */
	bool isWriteEffectful(const WriteLabel *wLab);

	/* SAVer: Checks whether the effects of a write are observable */
	bool isWriteObservable(const WriteLabel *lab);

	/* SAVer: Checks whether the addition of an event changes our
	 * perspective of a potential spinloop */
	void checkReconsiderFaiSpinloop(const MemAccessLabel *lab);

	/* SAVer: Given the end of a potential FAI-ZNE spinloop,
	 * returns true if it is indeed a spinloop */
	bool areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab);

	/* BAM: Filters out unnecessary rfs for LAB when BAM is enabled */
	void filterConflictingBarriers(const ReadLabel *lab, std::vector<Event> &stores);

	/* Opt: Futher reduces the set of available read-from options for a
	 * read that is part of a lock() op  */
	void filterAcquiredLocks(const ReadLabel *rLab, std::vector<Event> &stores);

	/* Helper: Filters out RFs that will make the CAS fail */
	void filterConfirmingRfs(const ReadLabel *lab, std::vector<Event> &stores);

	/* Helper: Returns true if there is a speculative read that hasn't been confirmed */
	bool existsPendingSpeculation(const ReadLabel *lab, const std::vector<Event> &stores);

	/* Helper: Ensures a speculative read will not be added if
	 * there are other speculative (unconfirmed) reads */
	void filterUnconfirmedReads(const ReadLabel *lab, std::vector<Event> &stores);

	/* Opt: Tries to in-place revisit a read that is part of a lock.
	 * Returns true if the optimization succeeded */
	bool tryRevisitLockInPlace(const BackwardRevisit &r);

	/* Opt: Repairs the reads-from edge of a dangling lock */
	void repairLock(LockCasReadLabel *lab);

	/* Opt: Repairs some locks that may be "dangling", as part of the
	 * in-place revisiting of locks */
	void repairDanglingLocks();

	/* Opt: Repairs barriers that may be "dangling" after cutting the graph. */
	void repairDanglingBarriers();

	/* Opt: Finds the last memory access that is visible to other threads;
	 * return nullptr if no such access is found */
	const MemAccessLabel *getPreviousVisibleAccessLabel(Event start) const;

	/* Opt: Checks whether there is no need to explore the other threads
	 * (e.g., POS \in B and will not be removed in all subsequent subexplorations),
	 * and if so moots the current execution */
	void mootExecutionIfFullyBlocked(Event pos);

	/* LKMM: Helper for visiting LKMM fences */
	void visitFenceLKMM(std::unique_ptr<FenceLabel> fLab, const EventDeps *deps);

	/* LAPOR: Returns whether the current execution is lock-well-formed */
	bool isLockWellFormedLAPOR() const;

	/* LAPOR: Helper for visiting a lock()/unlock() event */
	void visitLockLAPOR(std::unique_ptr<LockLabelLAPOR> lab, const EventDeps *deps);
	void visitUnlockLAPOR(std::unique_ptr<UnlockLabelLAPOR> uLab, const EventDeps *deps);

	/* Helper: Wake up any threads blocked on a helping CAS */
	void unblockWaitingHelping();

	/* Helper: Returns whether there is a valid helped-CAS which the helping-CAS
	 * to be added will be helping. (If an invalid helped-CAS exists, this
	 * method raises an error.) */
	bool checkHelpingCasCondition(const HelpingCasLabel *lab);

	/* Helper: Checks whether the user annotation about helped/helping CASes seems OK */
	void checkHelpingCasAnnotation();

	/* SR: Checks whether CANDIDATE is symmetric to THREAD */
	bool isSymmetricToSR(int candidate, int thread, Event parent,
			     llvm::Function *threadFun, SVal threadArg) const;

	/* SR: Returns the (greatest) ID of a thread that is symmetric to THREAD */
	int getSymmetricTidSR(int thread, Event parent, llvm::Function *threadFun,
			      SVal threadArg) const;

	/* SR: Returns true if TID has the same prefix up to POS.INDEX as POS.THREAD */
	bool sharePrefixSR(int tid, Event pos) const;

	/* SR: Filter stores that will lead to a symmetric execution */
	void filterSymmetricStoresSR(const ReadLabel *rLab, std::vector<Event> &stores) const;

	/* SAVer: Filters stores that will lead to an assume-blocked execution */
	bool filterValuesFromAnnotSAVER(const ReadLabel *rLab, std::vector<Event> &stores);


	/*** Output-related ***/

	/* Returns a view to be used when replaying */
	View getReplayView() const;

	/* Prints the source-code instructions leading to Event e.
	 * Assumes that debugging information have already been collected */
	void printTraceBefore(Event e, llvm::raw_ostream &ss = llvm::dbgs());

	/* Helper for printTraceBefore() that prints events according to po U rf */
	void recPrintTraceBefore(const Event &e, View &a,
				 llvm::raw_ostream &ss = llvm::outs());

	/* Returns the name of the variable residing in addr */
	std::string getVarName(const SAddr &addr) const;

	/* Outputs the full graph.
	 * If printMetadata is set, it outputs debugging information
	 * (these should have been collected beforehand) */
	void printGraph(bool printMetadata = false, llvm::raw_ostream &s = llvm::dbgs());

	/* Outputs the current graph into a file (DOT format),
	 * and visually marks events e and c (conflicting).
	 * Assumes debugging information have already been collected  */
	void dotPrintToFile(const std::string &filename, Event e, Event c);


	/*** To be overrided by instances of the Driver ***/

	/* Updates lab with model-specific information.
	 * Needs to be called every time a new label is added to the graph */
	virtual void updateLabelViews(EventLabel *lab, const EventDeps *deps) = 0;

	/* Checks for races after a load/store is added to the graph.
	 * Should return the racy event, or INIT if no such event exists */
	virtual Event findDataRaceForMemAccess(const MemAccessLabel *mLab) = 0;

	/* Returns an approximation of consistent rfs for RLAB.
	 * The rfs are ordered according to CO */
	virtual std::vector<Event> getRfsApproximation(const ReadLabel *rLab);

	/* Returns an approximation of the reads that SLAB can revisit.
	 * The reads are ordered in reverse-addition order */
	virtual std::vector<Event> getRevisitableApproximation(const WriteLabel *sLab);

	/* Changes the reads-from edge for the specified label.
	 * This effectively changes the label, hence this method is virtual */
	virtual void changeRf(Event read, Event store) = 0;

	/* Synchronizes thread begins with thread create events. */
	virtual void updateStart(Event create, Event begin) = 0;

	/* Used to make a join label synchronize with a finished thread.
	 * Returns true if the child thread has finished and updates the
	 * views of the join, or false otherwise */
	virtual bool updateJoin(Event join, Event childLast) = 0;

	/* Performs the necessary initializations for the
	 * consistency calculation */
	virtual void initConsCalculation() = 0;

#ifdef ENABLE_GENMC_DEBUG
	void checkForDuplicateRevisit(const ReadLabel *rLab, const WriteLabel *sLab);
#endif

	/* Random generator facilities used */
	using MyRNG  = std::mt19937;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;

	/* The thread pool this driver may belong to */
	ThreadPool *pool = nullptr;

	/* The source code of the program under test */
	std::string sourceCode;

	/* User configuration */
	std::shared_ptr<const Config> userConf;

	/* The interpreter used by the driver */
	std::unique_ptr<llvm::Interpreter> EE;

	/* The graph managing object */
	std::unique_ptr<ExecutionGraph> execGraph;

	/* The worklist for backtracking. map[stamp->work set] */
	LocalQueueT workqueue;

	/* The revisit sets used during the exploration map[stamp->revisit set] */
	RevisitSetT revisitSet;

	/* Opt: Which thread(s) the scheduler should prioritize
	 * (empty if none) */
	std::vector<Event> threadPrios;

	/* Opt: Whether this execution is moot (locking) */
	bool isMootExecution;

	/* Opt: Whether a particular read needs to be repaired during rescheduling */
	Event readToReschedule;

	/* Verification result to be returned to caller */
	Result result;

	/* Whether we are stopping the exploration (e.g., due to an error found) */
	bool shouldHalt;

	/* Dbg: Random-number generator for scheduling randomization */
	MyRNG rng;

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s,
					     const Status &r);
};

#endif /* __GENMC_DRIVER_HPP__ */
