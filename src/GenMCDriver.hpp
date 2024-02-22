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
#include "ExecutionGraph.hpp"
#include "SAddrAllocator.hpp"
#include "Trie.hpp"
#include "VerificationError.hpp"
#include "WorkSet.hpp"
#include <llvm/ADT/BitVector.h>
#include <llvm/IR/Module.h>

#include <cstdint>
#include <ctime>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <unordered_set>
#include <variant>

namespace llvm {
class Interpreter;
}
class ModuleInfo;
class ThreadPool;
class BoundDecider;
enum class BoundCalculationStrategy;

class GenMCDriver {

protected:
	using LocalQueueT = std::map<Stamp, WorkSet>;
	using ValuePrefixT = std::unordered_map<
		unsigned int,
		Trie<std::vector<SVal>, std::vector<std::unique_ptr<EventLabel>>, SValUCmp>>;
	using ChoiceMap = std::map<Stamp, VSet<Event>>;

public:
	/* The operating mode of the driver */
	struct VerificationMode {};
	struct EstimationMode {
		unsigned int budget;
	};
	using Mode = std::variant<VerificationMode, EstimationMode>;

	/* Verification result */
	struct Result {
		VerificationError status = VerificationError::VE_OK; /* Whether the verification
									completed successfully */
		unsigned explored{};	      /* Number of complete executions explored */
		unsigned exploredBlocked{};   /* Number of blocked executions explored */
		unsigned boundExceeding{};    /* Number of bound-exceeding executions explored */
		long double estimationMean{}; /* The mean of estimations */
		long double estimationVariance{}; /* The (biased) variance of the estimations */
#ifdef ENABLE_GENMC_DEBUG
		unsigned exploredMoot{}; /* Number of moot executions _encountered_ */
		unsigned duplicates{};	 /* Number of duplicate executions explored */
		llvm::IndexedMap<int> exploredBounds{}; /* Number of complete executions not
							   exceeding each bound */
#endif
		std::string message{};		    /* A message to be printed */
		VSet<VerificationError> warnings{}; /* The warnings encountered */

		Result() = default;

		auto operator+=(const Result &other) -> Result &
		{
			/* Propagate latest error */
			if (other.status != VerificationError::VE_OK)
				status = other.status;
			message += other.message;
			explored += other.explored;
			exploredBlocked += other.exploredBlocked;
			boundExceeding += other.boundExceeding;
			estimationMean += other.estimationMean;
			estimationVariance += other.estimationVariance;
#ifdef ENABLE_GENMC_DEBUG
			exploredMoot += other.exploredMoot;
			/* Bound-blocked executions are calculated at the end */
			exploredBounds.grow(other.exploredBounds.size() - 1);
			for (auto i = 0U; i < other.exploredBounds.size(); i++)
				exploredBounds[i] += other.exploredBounds[i];
			duplicates += other.duplicates;
#endif
			warnings.insert(other.warnings);
			return *this;
		}
	};

	/* Driver (global) state */
	struct State {
		std::unique_ptr<ExecutionGraph> graph;
		ChoiceMap choices;
		SAddrAllocator alloctor;
		llvm::BitVector fds;
		ValuePrefixT cache;
		Event lastAdded;

		State() = delete;
		State(std::unique_ptr<ExecutionGraph> g, ChoiceMap &&m, SAddrAllocator &&alloctor,
		      llvm::BitVector &&fds, ValuePrefixT &&cache, Event la);

		State(const State &) = delete;
		auto operator=(const State &) -> State & = delete;
		State(State &&) = default;
		auto operator=(State &&) -> State & = default;

		~State();
	};

private:
	struct Execution;

	static bool isInvalidAccessError(VerificationError s)
	{
		return VerificationError::VE_InvalidAccessBegin <= s &&
		       s <= VerificationError::VE_InvalidAccessEnd;
	};

public:
	/**** Generic actions ***/

	/* Sets up the next thread to run in the interpreter */
	bool scheduleNext();

	/* Opt: Tries to optimize the scheduling of next instruction by checking the cache */
	bool tryOptimizeScheduling(Event pos);

	/* Things to do when an execution starts/ends */
	void handleExecutionStart();
	void handleExecutionEnd();

	/* Pers: Functions that run at the start/end of the recovery routine */
	void handleRecoveryStart();
	void handleRecoveryEnd();

	/* Starts the verification procedure for a driver */
	void run();

	/* Stops the verification procedure when an error is found */
	void halt(VerificationError status);

	/* Returns the result of the verification procedure */
	const Result &getResult() const { return result; }
	Result &getResult() { return result; }

	/* Creates driver instance(s) and starts verification for the given module. */
	static Result verify(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
			     std::unique_ptr<ModuleInfo> modInfo);

	static Result estimate(std::shared_ptr<const Config> conf,
			       const std::unique_ptr<llvm::Module> &mod,
			       const std::unique_ptr<ModuleInfo> &modInfo);

	/* Gets/sets the thread pool this driver should account to */
	ThreadPool *getThreadPool() { return pool; }
	ThreadPool *getThreadPool() const { return pool; }
	void setThreadPool(ThreadPool *tp) { pool = tp; }

	/* Initializes the exploration from a given state */
	void initFromState(std::unique_ptr<State> s);

	/* Extracts the current driver state.
	 * The driver is left in an inconsistent form */
	std::unique_ptr<State> extractState();

	/*** Instruction-related actions ***/

	/* Returns the value this load reads */
	std::optional<SVal> handleLoad(std::unique_ptr<ReadLabel> rLab);

	/* A function modeling a write to disk has been interpreted.
	 * Returns the value read */
	SVal handleDskRead(std::unique_ptr<DskReadLabel> rLab);

	/* A store has been interpreted, nothing for the interpreter */
	void handleStore(std::unique_ptr<WriteLabel> wLab);

	/* A function modeling a write to disk has been interpreted */
	void handleDskWrite(std::unique_ptr<DskWriteLabel> wLab);

	/* A helping CAS operation has been interpreter.
	 * Returns whether the helped CAS is present. */
	bool handleHelpingCas(std::unique_ptr<HelpingCasLabel> hLab);

	/* A function modeling the beginning of the opening of a file.
	 * The interpreter will get back the file descriptor */
	SVal handleDskOpen(std::unique_ptr<DskOpenLabel> oLab);

	/* An fsync() operation has been interpreted */
	void handleDskFsync(std::unique_ptr<DskFsyncLabel> fLab);

	/* A sync() operation has been interpreted */
	void handleDskSync(std::unique_ptr<DskSyncLabel> fLab);

	/* A call to __VERIFIER_pbarrier() has been interpreted */
	void handleDskPbarrier(std::unique_ptr<DskPbarrierLabel> fLab);

	/* A fence has been interpreted, nothing for the interpreter */
	void handleFence(std::unique_ptr<FenceLabel> fLab);

	/* A cache line flush has been interpreted, nothing for the interpreter */
	void handleCLFlush(std::unique_ptr<CLFlushLabel> fLab);

	/* A call to __VERIFIER_opt_begin() has been interpreted.
	 * Returns whether the block should expand */
	bool handleOptional(std::unique_ptr<OptionalLabel> lab);

	/* A call to __VERIFIER_loop_begin() has been interpreted */
	void handleLoopBegin(std::unique_ptr<LoopBeginLabel> lab);

	/* A call to __VERIFIER_spin_start() has been interpreted */
	void handleSpinStart(std::unique_ptr<SpinStartLabel> lab);

	/* A call to __VERIFIER_faiZNE_spin_end() has been interpreted */
	void handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab);

	/* A call to __VERIFIER_lockZNE_spin_end() has been interpreted */
	void handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab);

	/* A thread has terminated abnormally */
	void handleThreadKill(std::unique_ptr<ThreadKillLabel> lab);

	/* Returns the TID of the newly created thread */
	int handleThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab);

	/* Returns an appropriate result for pthread_join() */
	std::optional<SVal> handleThreadJoin(std::unique_ptr<ThreadJoinLabel> jLab);

	/* A thread has just finished execution, nothing for the interpreter */
	void handleThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab);

	/* __VERIFIER_hp_protect() has been called */
	void handleHpProtect(std::unique_ptr<HpProtectLabel> hpLab);

	/* Returns an appropriate result for malloc() */
	SVal handleMalloc(std::unique_ptr<MallocLabel> aLab);

	/* A call to free() has been interpreted, nothing for the intepreter */
	void handleFree(std::unique_ptr<FreeLabel> dLab);

	/* This method blocks the current thread  */
	void handleBlock(std::unique_ptr<BlockLabel> bLab);

	/* LKMM: Handle RCU functions */
	void handleRCULockLKMM(std::unique_ptr<RCULockLabelLKMM> lab);
	void handleRCUUnlockLKMM(std::unique_ptr<RCUUnlockLabelLKMM> lab);
	void handleRCUSyncLKMM(std::unique_ptr<RCUSyncLabelLKMM> lab);

	/* This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void reportError(Event pos, VerificationError r, const std::string &err = std::string(),
			 const EventLabel *racyLab = nullptr, bool shouldHalt = true);

	/* Helper that reports an unreported warning only if it hasn't reported before.
	 * Returns true if the warning should be treated as an error according to the config. */
	bool reportWarningOnce(Event pos, VerificationError r, const EventLabel *racyLab = nullptr);

	virtual ~GenMCDriver();

protected:
	GenMCDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		    std::unique_ptr<ModuleInfo> MI, Mode = VerificationMode{});

	/* No copying or copy-assignment of this class is allowed */
	GenMCDriver(GenMCDriver const &) = delete;
	GenMCDriver &operator=(GenMCDriver const &) = delete;

	/* Returns a pointer to the user configuration */
	const Config *getConf() const { return userConf.get(); }

	/* Returns a pointer to the interpreter */
	llvm::Interpreter *getEE() const { return EE.get(); }

	/* Returns a reference to the current execution */
	Execution &getExecution() { return execStack.back(); }
	const Execution &getExecution() const { return execStack.back(); }

	/* Returns a reference to the current graph */
	ExecutionGraph &getGraph() { return getExecution().getGraph(); }
	const ExecutionGraph &getGraph() const { return getExecution().getGraph(); }

	LocalQueueT &getWorkqueue() { return getExecution().getWorkqueue(); }
	const LocalQueueT &getWorkqueue() const { return getExecution().getWorkqueue(); }

	ChoiceMap &getChoiceMap() { return getExecution().getChoiceMap(); }
	const ChoiceMap &getChoiceMap() const { return getExecution().getChoiceMap(); }

	/* Pushes E to the execution stack. */
	void pushExecution(Execution &&e);

	/* Pops the top stack entry.
	 * Returns false if the stack is empty or this was the last entry. */
	bool popExecution();

	/* Returns the address allocator */
	const SAddrAllocator &getAddrAllocator() const { return alloctor; }
	SAddrAllocator &getAddrAllocator() { return alloctor; }

	/* Returns a fresh address for a new allocation */
	SAddr getFreshAddr(const MallocLabel *aLab);

	/* Pers: Returns a fresh file descriptor for a new open() call (marks it as in use) */
	int getFreshFd();

	/* Pers: Marks that the file descriptor fd is in use */
	void markFdAsUsed(int fd);

	/* Given a write event from the graph, returns the value it writes */
	SVal getWriteValue(const EventLabel *wLab, const AAccess &a);
	SVal getWriteValue(const WriteLabel *wLab)
	{
		return getWriteValue(wLab, wLab->getAccess());
	}

	/* Returns the value written by a disk write */
	SVal getDskWriteValue(const EventLabel *wLab, const AAccess &a);
	SVal getDskWriteValue(const DskWriteLabel *wLab)
	{
		return getDskWriteValue(wLab, wLab->getAccess());
	}

	/* Returns the value read by a read */
	SVal getReadValue(const ReadLabel *rLab)
	{
		return getWriteValue(rLab->getRf(), rLab->getAccess());
	}

	/* Returns the value read by a disk read */
	SVal getDskReadValue(const DskReadLabel *rLab)
	{
		return getDskWriteValue(rLab->getRf(), rLab->getAccess());
	}

	/* Returns the value returned by the terminated thread */
	SVal getJoinValue(const ThreadJoinLabel *jLab) const;

	/* Returns the value passed to the spawned thread */
	SVal getStartValue(const ThreadStartLabel *bLab) const;

	/* Returns all values read leading up to POS */
	std::pair<std::vector<SVal>, Event> extractValPrefix(Event pos);

	/* Returns the value that a read is reading. This function should be
	 * used when calculating the value that we should return to the
	 * interpreter. */
	std::optional<SVal> getReadRetValue(const ReadLabel *rLab);
	SVal getRecReadRetValue(const ReadLabel *rLab);

	int getSymmPredTid(int tid) const;
	int getSymmSuccTid(int tid) const;
	bool isEcoBefore(const EventLabel *lab, int tid) const;
	bool isEcoSymmetric(const EventLabel *lab, int tid) const;
	bool isPredSymmetryOK(const EventLabel *lab, int tid);
	bool isPredSymmetryOK(const EventLabel *lab);
	bool isSuccSymmetryOK(const EventLabel *lab, int tid);
	bool isSuccSymmetryOK(const EventLabel *lab);
	bool isSymmetryOK(const EventLabel *lab);
	void updatePrefixWithSymmetriesSR(EventLabel *lab);

	/* Returns the value with which a barrier at PTR has been initialized */
	SVal getBarrierInitValue(const AAccess &a);

	/* Pers: Returns true if we are currently running the recovery routine */
	bool inRecoveryMode() const;

	/* Est: Returns true if we are currently running in estimation mode */
	bool inEstimationMode() const { return std::holds_alternative<EstimationMode>(mode); }

	/* Est: Returns true if the estimation seems "good enough" */
	bool shouldStopEstimating()
	{
		auto remainingBudget = --std::get<EstimationMode>(mode).budget;
		if (remainingBudget == 0)
			return true;

		auto totalExplored = result.explored + result.exploredBlocked;
		auto sd = std::sqrt(result.estimationVariance);
		return (totalExplored >= getConf()->estimationMin) &&
		       (sd <= result.estimationMean / getConf()->sdThreshold ||
			totalExplored > result.estimationMean);
	}

	/* Liveness: Checks whether a spin-blocked thread reads co-maximal values */
	bool threadReadsMaximal(int tid);

	/* Liveness: Calls visitError() if there is a liveness violation */
	void checkLiveness();

	/* Returns true if E is maximal in ADDR at P*/
	bool isCoMaximal(SAddr addr, Event e, bool checkCache = false);

	/* Returns true if MLAB is protected by a hazptr */
	bool isHazptrProtected(const MemAccessLabel *mLab) const;

private:
	/* Represents the execution at a given point */
	struct Execution {
		Execution() = delete;
		Execution(std::unique_ptr<ExecutionGraph> g, LocalQueueT &&w, ChoiceMap &&cm);

		Execution(const Execution &) = delete;
		auto operator=(const Execution &) -> Execution & = delete;
		Execution(Execution &&) = default;
		auto operator=(Execution &&) -> Execution & = default;

		/* Returns a reference to the current graph */
		ExecutionGraph &getGraph() { return *graph; }
		const ExecutionGraph &getGraph() const { return *graph; }

		LocalQueueT &getWorkqueue() { return workqueue; }
		const LocalQueueT &getWorkqueue() const { return workqueue; }

		ChoiceMap &getChoiceMap() { return choices; }
		const ChoiceMap &getChoiceMap() const { return choices; }

		void restrict(Stamp stamp);

		~Execution();

	private:
		/* Removes all items with stamp >= STAMP from the list */
		void restrictWorklist(Stamp stamp);
		void restrictGraph(Stamp stamp);
		void restrictChoices(Stamp stamp);

		std::unique_ptr<ExecutionGraph> graph;
		ChoiceMap choices;
		LocalQueueT workqueue;
	};

	/*** Worklist-related ***/

	/* Adds an appropriate entry to the worklist */
	void addToWorklist(Stamp stamp, WorkSet::ItemT item);

	/* Fetches the next backtrack option.
	 * A default-constructed item means that the list is empty */
	std::pair<Stamp, WorkSet::ItemT> getNextItem();

	/*** Exploration-related ***/

	/* The workhorse for run().
	 * Exhaustively explores all  consistent executions of a program */
	void explore();

	/* Returns whether a revisit results to a valid execution
	 * (e.g., consistent, accessing allocated memory, etc) */
	bool isRevisitValid(const Revisit &revisit);

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

	/* Returns true if THREAD is schedulable (i.e., there are more
	 * instructions to run and it is not blocked) */
	bool isSchedulable(int thread) const;

	int getFirstSchedulableSymmetric(int tid);

	/* Ensures the scheduler respects atomicity */
	bool scheduleAtomicity();

	/* Tries to schedule according to the current prioritization scheme */
	bool schedulePrioritized();

	/* Returns true if the next instruction of TID is a load
	 * Note: assumes there is a next instruction in TID*/
	bool isNextThreadInstLoad(int tid);

	/* Helpers for schedule according to a policy */
	bool scheduleNextLTR();
	bool scheduleNextWF();
	bool scheduleNextWFR();
	bool scheduleNextRandom();

	/* Tries to schedule the next instruction according to the
	 * chosen policy */
	bool scheduleNormal();

	/* Blocks thread with BLAB (needs to be maximal) */
	void blockThread(std::unique_ptr<BlockLabel> bLab);

	/* Blocks thread at POS with type T. Tries to moot afterward */
	void blockThreadTryMoot(std::unique_ptr<BlockLabel> bLab);

	/* Unblocks thread at POS */
	void unblockThread(Event pos);

	/* Returns whether the current execution is blocked */
	bool isExecutionBlocked() const;

	/* Opt: Tries to reschedule any reads that were added blocked */
	bool rescheduleReads();

	/* Resets the prioritization scheme */
	void resetThreadPrioritization();

	/* Returns whether LAB accesses a valid location.  */
	bool isAccessValid(const MemAccessLabel *lab) const;

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

	/* Checks whether the IPR optimization is valid (i.e., no WW-races),
	 * and reports an error if it's not. Returns the validity result */
	void checkIPRValidity(const ReadLabel *rLab);

	/* Returns true if the exploration is guided by a graph */
	bool isExecutionDrivenByGraph(const EventLabel *lab);

	/* Returns true if we are currently replaying a graph */
	bool inReplay() const;

	/* Opt: Caches LAB to optimize scheduling next time */
	void cacheEventLabel(const EventLabel *lab);

	/* Opt: Checks whether SEQ has been seen before for THREAD and
	 * if so returns its successors. Returns nullptr otherwise. */
	std::vector<std::unique_ptr<EventLabel>> *
	retrieveCachedSuccessors(unsigned int thread, const std::vector<SVal> &seq)
	{
		return seenPrefixes[thread].lookup(seq);
	}

	/* Adds LAB to graph (maintains well-formedness).
	 * If another label exists in the specified position, it is replaced. */
	EventLabel *addLabelToGraph(std::unique_ptr<EventLabel> lab);

	/* Est: Picks (and sets) a random RF among some possible options */
	std::optional<SVal> pickRandomRf(ReadLabel *rLab, std::vector<Event> &stores);

	/* Est: Picks (and sets) a random CO among some possible options */
	void pickRandomCo(WriteLabel *sLab,
			  const llvm::iterator_range<ExecutionGraph::co_iterator> &coRange);

	/* BAM: Tries to optimize barrier-related revisits */
	bool tryOptimizeBarrierRevisits(const BIncFaiWriteLabel *sLab, std::vector<Event> &loads);

	/* IPR: Tries to revisit blocked reads in-place */
	void tryOptimizeIPRs(const WriteLabel *sLab, std::vector<Event> &loads);

	/* IPR: Removes a CAS that blocks when reading from SLAB.
	 * Returns whether if the label was removed
	 * (Returns false if RLAB reads from unallocated memory.) */
	bool removeCASReadIfBlocks(const ReadLabel *rLab, const EventLabel *sLab);

	/* Helper: Optimizes revisits of reads that will lead to a failed speculation */
	void optimizeUnconfirmedRevisits(const WriteLabel *sLab, std::vector<Event> &loads);

	/* Helper: Optimize the revisit in case SLAB is a RevBlocker */
	bool tryOptimizeRevBlockerAddition(const WriteLabel *sLab, std::vector<Event> &loads);

	/* Opt: Tries to optimize revisiting from LAB. It may modify
	 * LOADS, and returns whether we can skip revisiting altogether */
	bool tryOptimizeRevisits(const WriteLabel *lab, std::vector<Event> &loads);

	/* Constructs a BackwardRevisit representing RLAB <- SLAB */
	std::unique_ptr<BackwardRevisit> constructBackwardRevisit(const ReadLabel *rLab,
								  const WriteLabel *sLab) const;

	/* Given a revisit RLAB <- WLAB, returns the view of the resulting graph.
	 * (This function can be abused and also be utilized for returning the view
	 * of "fictional" revisits, e.g., the view of an event in a maximal path.) */
	std::unique_ptr<VectorClock> getRevisitView(const ReadLabel *rLab, const WriteLabel *sLab,
						    const WriteLabel *midLab = nullptr) const;

	/* Returnes true if the revisit R will delete LAB from the graph */
	bool revisitDeletesEvent(const BackwardRevisit &r, const EventLabel *lab) const
	{
		auto &v = r.getViewNoRel();
		return !v->contains(lab->getPos()) && !prefixContainsSameLoc(r, lab);
	}

	/* Returns true if ELAB has been revisited by some event that
	 * will be deleted by the revisit R */
	bool hasBeenRevisitedByDeleted(const BackwardRevisit &r, const EventLabel *eLab);

	/* Returns whether the prefix of SLAB contains LAB's matching lock */
	bool prefixContainsMatchingLock(const BackwardRevisit &r, const EventLabel *lab);

	bool isCoBeforeSavedPrefix(const BackwardRevisit &r, const EventLabel *lab);

	bool coherenceSuccRemainInGraph(const BackwardRevisit &r);

	/* Returns true if all events to be removed by the revisit
	 * RLAB <- SLAB form a maximal extension */
	bool isMaximalExtension(const BackwardRevisit &r);

	/* Returns true if the graph that will be created when sLab revisits rLab
	 * will be the same as the current one */
	bool revisitModifiesGraph(const BackwardRevisit &r) const;

	bool prefixContainsSameLoc(const BackwardRevisit &r, const EventLabel *lab) const;

	bool isConflictingNonRevBlocker(const EventLabel *pLab, const WriteLabel *sLab,
					const Event &s);

	/* Helper: Checks whether the execution should continue upon SLAB revisited LOADS.
	 * Returns true if yes, and false (+moot) otherwise  */
	bool checkRevBlockHELPER(const WriteLabel *sLab, const std::vector<Event> &loads);

	/* Calculates all possible coherence placings for SLAB and
	 * pushes them to the worklist. */
	void calcCoOrderings(WriteLabel *sLab);

	/* Calculates revisit options and pushes them to the worklist.
	 * Returns true if the current exploration should continue */
	bool calcRevisits(const WriteLabel *lab);

	/* Modifies the graph accordingly when revisiting a write (MO).
	 * May trigger backward-revisit explorations.
	 * Returns whether the resulting graph should be explored. */
	bool revisitWrite(const WriteForwardRevisit &wi);

	/* Modifies the graph accordingly when revisiting an optional.
	 * Returns true if the resulting graph should be explored */
	bool revisitOptional(const OptionalForwardRevisit &oi);

	/* Modifies (but not restricts) the graph when we are revisiting a read.
	 * Returns true if the resulting graph should be explored. */
	bool revisitRead(const Revisit &s);

	bool forwardRevisit(const ForwardRevisit &fr);
	bool backwardRevisit(const BackwardRevisit &fr);

	/* Adjusts the graph and the worklist according to the backtracking option S.
	 * Returns true if the resulting graph should be explored */
	bool restrictAndRevisit(Stamp st, const WorkSet::ItemT &s);

	/* If rLab is the read part of an RMW operation that now became
	 * successful, this function adds the corresponding write part.
	 * Returns a pointer to the newly added event, or nullptr
	 * if the event was not an RMW, or was an unsuccessful one */
	WriteLabel *completeRevisitedRMW(const ReadLabel *rLab);

	/* Copies the current EG according to BR's view V.
	 * May modify V but will not execute BR in the copy. */
	std::unique_ptr<ExecutionGraph> copyGraph(const BackwardRevisit *br, VectorClock *v) const;

	ChoiceMap createChoiceMapForCopy(const ExecutionGraph &og) const;

	/* Given a list of stores that it is consistent to read-from,
	 * filters out options that can be skipped (according to the conf),
	 * and determines the order in which these options should be explored */
	bool filterOptimizeRfs(const ReadLabel *lab, std::vector<Event> &stores);

	bool isExecutionValid(const EventLabel *lab)
	{
		return isSymmetryOK(lab) && isConsistent(lab) && !partialExecutionExceedsBound();
	}

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
	void annotateStoreHELPER(WriteLabel *wLab);

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

	/* Opt: Remove possibly invalidated ReadOpt events */
	void checkReconsiderReadOpts(const WriteLabel *sLab);

	/* SAVer: Given the end of a potential FAI-ZNE spinloop,
	 * returns true if it is indeed a spinloop */
	bool areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab);

	/* BAM: Filters out unnecessary rfs for LAB when BAM is enabled */
	void filterConflictingBarriers(const ReadLabel *lab, std::vector<Event> &stores);

	/* Estimation: Filters outs stores read by RMW loads */
	void filterAtomicityViolations(const ReadLabel *lab, std::vector<Event> &stores);

	/* IPR: Returns true if RLAB would block when reading val */
	bool willBeAssumeBlocked(const ReadLabel *rLab, SVal val);

	/* IPR: Performs BR in-place */
	void revisitInPlace(const BackwardRevisit &br);

	/* Opt: Finds the last memory access that is visible to other threads;
	 * return nullptr if no such access is found */
	const MemAccessLabel *getPreviousVisibleAccessLabel(Event start) const;

	/* Opt: Checks whether there is no need to explore the other threads
	 * (e.g., POS \in B and will not be removed in all subsequent subexplorations),
	 * and if so moots the current execution */
	void mootExecutionIfFullyBlocked(Event pos);

	/* LKMM: Helper for visiting LKMM fences */
	void handleFenceLKMM(std::unique_ptr<FenceLabel> fLab);

	/* Helper: Wake up any threads blocked on a helping CAS */
	void unblockWaitingHelping();

	bool writesBeforeHelpedContainedInView(const HelpedCasReadLabel *lab, const View &view);

	/* Helper: Returns whether there is a valid helped-CAS which the helping-CAS
	 * to be added will be helping. (If an invalid helped-CAS exists, this
	 * method raises an error.) */
	bool checkHelpingCasCondition(const HelpingCasLabel *lab);

	/* Helper: Checks whether the user annotation about helped/helping CASes seems OK */
	void checkHelpingCasAnnotation();

	/* SR: Checks whether CANDIDATE is symmetric to PARENT/INFO */
	bool isSymmetricToSR(int candidate, Event parent, const ThreadInfo &info) const;

	/* SR: Returns the (greatest) ID of a thread that is symmetric to PARENT/INFO */
	int getSymmetricTidSR(const ThreadCreateLabel *tcLab, const ThreadInfo &info) const;

	int calcLargestSymmPrefixBeforeSR(int tid, Event pos) const;

	/* SR: Returns true if TID has the same prefix up to POS.INDEX as POS.THREAD */
	bool sharePrefixSR(int tid, Event pos) const;

	/* SR: Filter stores that will lead to a symmetric execution */
	void filterSymmetricStoresSR(const ReadLabel *rLab, std::vector<Event> &stores) const;

	/* SAVer: Filters stores that will lead to an assume-blocked execution */
	void filterValuesFromAnnotSAVER(const ReadLabel *rLab, std::vector<Event> &stores);

	/*** Estimation-related ***/

	/* Registers that RLAB can read from all stores in STORES */
	void updateStSpaceChoices(const ReadLabel *rLab, const std::vector<Event> &stores);

	/* Registers that each L in LOADS can read from SLAB */
	void updateStSpaceChoices(const std::vector<Event> &loads, const WriteLabel *sLab);

	/* Registers that SLAB can be after each S in STORES */
	void updateStSpaceChoices(const WriteLabel *sLab, const std::vector<Event> &stores);

	/* Makes an estimation about the state space and updates the current one.
	 * Has to run at the end of an execution */
	void updateStSpaceEstimation();

	/*** Bound-related  ***/

	bool executionExceedsBound(BoundCalculationStrategy strategy) const;

	bool fullExecutionExceedsBound() const;

	bool partialExecutionExceedsBound() const;

#ifdef ENABLE_GENMC_DEBUG
	/* Update bounds histogram with the current, complete execution */
	void trackExecutionBound();
#endif

	/*** Output-related ***/

	/* Returns a view to be used when replaying */
	std::unique_ptr<VectorClock> getReplayView() const;

	/* Prints the source-code instructions leading to Event e.
	 * Assumes that debugging information have already been collected */
	void printTraceBefore(const EventLabel *lab, llvm::raw_ostream &ss = llvm::dbgs());

	/* Helper for printTraceBefore() that prints events according to po U rf */
	void recPrintTraceBefore(const Event &e, View &a, llvm::raw_ostream &ss = llvm::outs());

	/* Returns the name of the variable residing in addr */
	std::string getVarName(const SAddr &addr) const;

	/* Outputs the full graph.
	 * If printMetadata is set, it outputs debugging information
	 * (these should have been collected beforehand) */
	void printGraph(bool printMetadata = false, llvm::raw_ostream &s = llvm::dbgs());

	/* Outputs the current graph into a file (DOT format),
	 * and visually marks events e and c (conflicting).
	 * Assumes debugging information have already been collected  */
	void dotPrintToFile(const std::string &filename, const EventLabel *errLab,
			    const EventLabel *racyLab);

	/*** To be overrided by instances of the Driver ***/

	/* Updates lab with model-specific information.
	 * Needs to be called every time a new label is added to the graph */
	virtual void updateMMViews(EventLabel *lab) = 0;

	void updateLabelViews(EventLabel *lab);
	VerificationError checkForRaces(const EventLabel *lab);

	/* Returns an approximation of consistent rfs for RLAB.
	 * The rfs are ordered according to CO */
	virtual std::vector<Event> getRfsApproximation(const ReadLabel *rLab);

	/* Returns an approximation of the reads that SLAB can revisit.
	 * The reads are ordered in reverse-addition order */
	virtual std::vector<Event> getRevisitableApproximation(const WriteLabel *sLab);

	/* Returns true if the current graph is consistent when E is added */
	virtual bool isConsistent(const EventLabel *lab) const = 0;
	virtual bool isRecoveryValid(const EventLabel *lab) const = 0;
	virtual VerificationError checkErrors(const EventLabel *lab,
					      const EventLabel *&race) const = 0;
	virtual std::vector<VerificationError>
	checkWarnings(const EventLabel *lab, const VSet<VerificationError> &reported,
		      std::vector<const EventLabel *> &races) const = 0;
	virtual std::vector<Event> getCoherentRevisits(const WriteLabel *sLab,
						       const VectorClock &pporf) = 0;
	virtual std::vector<Event> getCoherentStores(SAddr addr, Event read) = 0;
	virtual llvm::iterator_range<ExecutionGraph::co_iterator>
	getCoherentPlacings(SAddr addr, Event read, bool isRMW) = 0;

	virtual bool isDepTracking() const = 0;

	/* Returns a vector clock representing the prefix of e.
	 * Depending on whether dependencies are tracked, the prefix can be
	 * either (po U rf) or (AR U rf) */
	const VectorClock &getPrefixView(const EventLabel *lab) const
	{
		if (!lab->hasPrefixView())
			lab->setPrefixView(calculatePrefixView(lab));
		return lab->getPrefixView();
	}

	virtual std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const = 0;

	virtual const View &getHbView(const EventLabel *lab) const = 0;

#ifdef ENABLE_GENMC_DEBUG
	void checkForDuplicateRevisit(const ReadLabel *rLab, const WriteLabel *sLab);
#endif

	friend llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const VerificationError &r);

	static constexpr unsigned int defaultFdNum = 20;

	/* Random generator facilities used */
	using MyRNG = std::mt19937;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;

	/* The operating mode of the driver */
	Mode mode = VerificationMode{};

	/* The thread pool this driver may belong to */
	ThreadPool *pool = nullptr;

	/* User configuration */
	std::shared_ptr<const Config> userConf;

	/* The interpreter used by the driver */
	std::unique_ptr<llvm::Interpreter> EE;

	/* Execution stack */
	std::vector<Execution> execStack;

	/* An allocator for fresh addresses */
	SAddrAllocator alloctor;

	/* Pers: A bitvector of available file descriptors */
	llvm::BitVector fds{defaultFdNum};

	/* Opt: Cached labels for optimized scheduling */
	ValuePrefixT seenPrefixes;

	/* Decider used to bound the exploration */
	std::unique_ptr<BoundDecider> bounder;

	/* Opt: Which thread(s) the scheduler should prioritize
	 * (empty if none) */
	std::vector<Event> threadPrios;

	/* Opt: Whether this execution is moot (locking) */
	bool isMootExecution = false;

	/* Opt: Whether a particular read needs to be repaired during rescheduling */
	Event readToReschedule = Event::getInit();

	/* Opt: Keeps track of the last event added for scheduling opt */
	Event lastAdded = Event::getInit();

	/* Verification result to be returned to caller */
	Result result{};

	/* Whether we are stopping the exploration (e.g., due to an error found) */
	bool shouldHalt = false;

	/* Dbg: Random-number generators for scheduling/estimation randomization */
	MyRNG rng;
	MyRNG estRng;
};

#endif /* __GENMC_DRIVER_HPP__ */
