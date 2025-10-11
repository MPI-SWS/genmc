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

#ifndef GENMC_GENMC_DRIVER_HPP
#define GENMC_GENMC_DRIVER_HPP

#include "ADT/Trie.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "Support/Hash.hpp"
#include "Support/SAddrAllocator.hpp"
#include "Verification/ChoiceMap.hpp"
#include "Verification/Config.hpp"
#include "Verification/InterpreterCallbacks.hpp"
#include "Verification/Relinche/LinearizabilityChecker.hpp"
#include "Verification/Relinche/Specification.hpp"
#include "Verification/Scheduler.hpp"
#include "Verification/VerificationError.hpp"
#include "Verification/VerificationResult.hpp"
#include "Verification/WorkList.hpp"
#include <llvm/ADT/BitVector.h>
#include <llvm/IR/Module.h>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>
#include <random>
#include <utility>
#include <variant>

namespace llvm {
class Interpreter;
}
class ModuleInfo;
class ThreadPool;
class LLIConfig;
class BoundDecider;
class ConsistencyChecker;
class SymmetryChecker;
enum class BoundCalculationStrategy;

class GenMCDriver {

protected:
	using LocalQueueT = WorkList;

public:
	/** The operating mode of the driver */
	struct VerificationMode {};
	struct EstimationMode {
		unsigned int budget;
	};
	using Mode = std::variant<VerificationMode, EstimationMode>;

	/** Represents the execution at a given point */
	struct Execution {
		Execution() = delete;
		Execution(std::unique_ptr<ExecutionGraph> g, LocalQueueT &&w, ChoiceMap &&cm,
			  SAddrAllocator &&alloctor, Event lastAdded);

		Execution(const Execution &) = delete;
		auto operator=(const Execution &) -> Execution & = delete;
		Execution(Execution &&) = default;
		auto operator=(Execution &&) -> Execution & = default;

		/** Returns a reference to the current graph */
		auto getGraph() -> ExecutionGraph & { return *graph; }
		auto getGraph() const -> const ExecutionGraph & { return *graph; }

		auto getWorkqueue() -> LocalQueueT & { return workqueue; }
		auto getWorkqueue() const -> const LocalQueueT & { return workqueue; }

		auto getChoiceMap() -> ChoiceMap & { return choices; }
		auto getChoiceMap() const -> const ChoiceMap & { return choices; }

		auto getAllocator() const -> const SAddrAllocator & { return alloctor; }
		auto getAllocator() -> SAddrAllocator & { return alloctor; }

		auto getLastAdded() const -> const Event & { return lastAdded; }
		auto getLastAdded() -> Event & { return lastAdded; }

		/** Removes all items with stamp >= STAMP from the execution */
		void restrict(Stamp stamp);

		~Execution();

		std::unique_ptr<ExecutionGraph> graph;
		LocalQueueT workqueue;
		ChoiceMap choices;
		SAddrAllocator alloctor;
		Event lastAdded = Event::getInit();
	};

	/** Handler Result Types */
	struct Reset {};
	struct Invalid {};
	template <typename T>
	using HandleResult = std::variant<T, VerificationError, Reset, Invalid>;

	/** Details for an error to be reported */
	struct ErrorDetails {
		ErrorDetails() = default;
		ErrorDetails(Event pos, VerificationError r, std::string err = std::string(),
			     const EventLabel *racyLab = nullptr, bool shouldHalt = true)
			: pos(pos), type(r), msg(std::move(err)), racyLab(racyLab),
			  shouldHalt(shouldHalt)
		{}

		Event pos{};
		VerificationError type{};
		std::string msg{};
		const EventLabel *racyLab{};
		bool shouldHalt = true;
	};

	template <typename... Ts> static auto create(Ts &&...params) -> std::unique_ptr<GenMCDriver>
	{
		return std::unique_ptr<GenMCDriver>(new GenMCDriver(std::forward<Ts>(params)...));
	}

	/**** Generic actions ***/

	/** Returns to the interpreter the next thread to run (nullopt if none) */
	auto scheduleNext(std::span<Action> runnable) -> std::optional<int>;

	/** Attemps to complete the execution by inspecting the cache.
	 * Returns whether it succeeded. */
	auto runFromCache() -> bool;

	/** Things to do when an execution starts/ends */
	void handleExecutionStart();
	void handleExecutionEnd();

	/** Whether there are more executions to be explored */
	bool done();

	/** Returns the result of the verification procedure */
	const VerificationResult &getResult() const { return result; }
	VerificationResult &getResult() { return result; }

	/*** Instruction handling ***/

	/** A thread has just finished execution, nothing for the interpreter */
	void handleThreadFinish(Event pos, SVal val);

	/** A thread has terminated abnormally */
	void handleThreadKill(Event pos);

	/** This method blocks the current thread  */
	void handleAssume(Event pos, AssumeType type);

	/** Returns the value this load reads */
	template <EventLabel::EventLabelKind k, typename... Ts>
	HandleResult<SVal> handleLoad(Event pos, Ts &&...params)
	{
		auto &g = getExec().getGraph();
		if (isExecutionDrivenByGraph(pos)) {
			return getReadRetValue(llvm::dyn_cast<ReadLabel>(g.getEventLabel(pos)));
		}
#define HANDLE_LABEL(NAME)                                                                         \
	if constexpr (k == EventLabel::EventLabelKind::NAME) {                                     \
		return handleLoad(NAME##Label::create(pos, std::forward<Ts>(params)...));          \
	} else
#include "ExecutionGraph/EventLabel.def"
		static_assert(false, "Unhandled load label kind");
	}

	/** A store has been interpreted, nothing for the interpreter */
	template <EventLabel::EventLabelKind k, typename... Ts>
	HandleResult<std::monostate> handleStore(Event pos, Ts &&...params)
	{
		if (isExecutionDrivenByGraph(pos))
			return {};
#define HANDLE_LABEL(NAME)                                                                         \
	if constexpr (k == EventLabel::EventLabelKind::NAME) {                                     \
		return handleStore(NAME##Label::create(pos, std::forward<Ts>(params)...));         \
	} else
#include "ExecutionGraph/EventLabel.def"
		static_assert(false, "Unhandled store label kind");
	}

	/** A fence has been interpreted, nothing for the interpreter */
	void handleFence(Event pos, MemOrdering ord, const EventDeps &deps);

	/** Returns an appropriate result for malloc() */
	template <typename... Ts> SVal handleMalloc(Event pos, Ts &&...params)
	{
		auto &g = getExec().getGraph();
		if (isExecutionDrivenByGraph(pos)) {
			auto *lab = llvm::dyn_cast<MallocLabel>(g.getEventLabel(pos));
			BUG_ON(!lab);
			return SVal(lab->getAllocAddr().get());
		}
		return handleMalloc(MallocLabel::create(pos, std::forward<Ts>(params)...));
	}

	/** A call to free() has been interpreted, nothing for the intepreter */
	std::optional<VerificationError> handleFree(Event pos, SAddr loc, const EventDeps &deps);
	std::optional<VerificationError> handleRetire(Event pos, SAddr loc, const EventDeps &deps);

	/** Returns the TID of the newly created thread */
	int handleThreadCreate(Event pos, ThreadInfo info, const EventDeps &deps);

	/** Returns an appropriate result for pthread_join() */
	HandleResult<SVal> handleThreadJoin(Event pos, unsigned int childTid,
					    const EventDeps &deps);

	/** A helping CAS operation has been interpreter.
	 * Returns whether the helped CAS is present. */
	bool handleHelpingCas(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,
			      SVal cmpVal, SVal newVal, const EventDeps &deps);

	/** A call to __VERIFIER_opt_begin() has been interpreted.
	 * Returns whether the block should expand */
	bool handleOptional(Event pos);

	/** A call to __VERIFIER_spin_start() has been interpreted */
	void handleSpinStart(Event pos);

	/** A call to __VERIFIER_faiZNE_spin_end() has been interpreted */
	void handleFaiZNESpinEnd(Event pos);

	/** A call to __VERIFIER_lockZNE_spin_end() has been interpreted */
	void handleLockZNESpinEnd(Event pos);

	/** Helpers for dummy events */
	void handleLoopBegin(Event pos);
	void handleHpProtect(Event pos, SAddr hpAddr, SAddr protAddr);
	void handleMethodBegin(Event pos, std::string methodName, int32_t argVal);
	void handleMethodEnd(Event pos, std::string methodName, int32_t retVal);
	void handleOutput(Event pos, std::string msg);

	/** This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void reportError(const ErrorDetails &details);

	/** Helper that reports an unreported warning only if it hasn't reported before.
	 * Returns true if the warning should be treated as an error according to the config. */
	bool reportWarningOnce(Event pos, VerificationError r, const EventLabel *racyLab = nullptr);

	virtual ~GenMCDriver();

protected:
	friend class Scheduler;
	friend class ArbitraryScheduler;
	friend class ThreadPool;
	friend class DriverHandlerDispatcher;
	friend void run(GenMCDriver *driver, llvm::Interpreter *EE);
	friend auto estimate(const LLIConfig &lliConfig, std::shared_ptr<const Config> conf,
			     const std::unique_ptr<llvm::Module> &mod,
			     const std::unique_ptr<ModuleInfo> &modInfo) -> VerificationResult;
	friend auto verify(const LLIConfig &lliConfig, std::shared_ptr<const Config> conf,
			   std::unique_ptr<llvm::Module> mod, std::unique_ptr<ModuleInfo> modInfo)
		-> VerificationResult;

	GenMCDriver(std::shared_ptr<const Config> conf, ThreadPool *pool = nullptr,
		    Mode = VerificationMode{});

	/** No copying or copy-assignment of this class is allowed */
	GenMCDriver(GenMCDriver const &) = delete;
	GenMCDriver &operator=(GenMCDriver const &) = delete;

	/** Returns a pointer to the user configuration */
	const Config *getConf() const { return userConf.get(); }

	/** Returns a pointer to the interpreter */
	llvm::Interpreter *getEE() const { return EE; }

	/** Sets pointer to the interpreter */
	void setEE(llvm::Interpreter *interp) { EE = interp; }

	/** Set the callbacks for querying the interpreter. */
	void setInterpCallbacks(InterpreterCallbacks interpCallbacks);

	/** Returns a reference to the current execution */
	Execution &getExec() { return execStack.back(); }
	const Execution &getExec() const { return execStack.back(); }

	/** Returns a reference to the set consistency checker */
	ConsistencyChecker &getConsChecker() { return *consChecker; }
	const ConsistencyChecker &getConsChecker() const { return *consChecker; }

	/** Returns a reference to the symmetry checker */
	SymmetryChecker &getSymmChecker() { return *symmChecker; }
	const SymmetryChecker &getSymmChecker() const { return *symmChecker; }

	LinearizabilityChecker &getRelinche() { return *relinche; }
	const LinearizabilityChecker &getRelinche() const { return *relinche; }

	/** Returns a reference to the scheduler */
	Scheduler &getScheduler() { return *scheduler_; }
	const Scheduler &getScheduler() const { return *scheduler_; }

	/** Stops the verification procedure when an error is found */
	void halt(VerificationError status);

	/** Pushes E to the execution stack. */
	void pushExecution(Execution &&e);

	/** Pops the top stack entry.
	 * Returns false if the stack is empty or this was the last entry. */
	bool popExecution();

	/** Gets/sets the thread pool this driver should account to */
	ThreadPool *getThreadPool() { return pool; }
	ThreadPool *getThreadPool() const { return pool; }
	void setThreadPool(ThreadPool *tp) { pool = tp; }

	/** Initializes the exploration from a given state */
	void initFromState(std::unique_ptr<Execution> s);

	/** Extracts the current driver state.
	 * The driver is left in an inconsistent form */
	std::unique_ptr<Execution> extractState();

	/** Returns the value that a read is reading. This function should be
	 * used when calculating the value that we should return to the
	 * interpreter. */
	HandleResult<SVal> getReadRetValue(const ReadLabel *rLab);
	SVal getRecReadRetValue(const ReadLabel *rLab);

	/** Est: Returns true if we are currently running in estimation mode */
	bool inEstimationMode() const { return std::holds_alternative<EstimationMode>(mode); }

	/** Est: Returns true if the estimation seems "good enough" */
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

private:
	/*** Instruction handling (EventLabel) ***/

	void handleThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab);
	void handleThreadKill(std::unique_ptr<ThreadKillLabel> lab);
	void handleBlock(std::unique_ptr<BlockLabel> bLab);
	HandleResult<SVal> handleLoad(std::unique_ptr<ReadLabel> rLab);
	HandleResult<std::monostate> handleStore(std::unique_ptr<WriteLabel> wLab);
	void handleFence(std::unique_ptr<FenceLabel> fLab);
	SVal handleMalloc(std::unique_ptr<MallocLabel> aLab);
	std::optional<VerificationError> handleFree(std::unique_ptr<FreeLabel> dLab);
	int handleThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab);
	HandleResult<SVal> handleThreadJoin(std::unique_ptr<ThreadJoinLabel> jLab);
	bool handleHelpingCas(std::unique_ptr<HelpingCasLabel> hLab);
	bool handleOptional(std::unique_ptr<OptionalLabel> lab);
	void handleSpinStart(std::unique_ptr<SpinStartLabel> lab);
	void handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab);
	void handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab);
	void handleDummy(std::unique_ptr<EventLabel> lab);

	/*** Exploration-related ***/

	/** Returns whether a revisit results to a valid execution
	 * (e.g., consistent, accessing allocated memory, etc) */
	bool isRevisitValid(const Revisit &revisit);

	/** Returns true if this driver is shutting down */
	bool isHalting() const;

	/** Returns true if this execution is moot */
	bool isMoot() const { return isMootExecution; }

	/** Opt: Mark current execution as moot/normal */
	void moot() { isMootExecution = true; }
	void unmoot() { isMootExecution = false; }

	/** Blocks thread at POS with type T. Tries to moot afterward */
	void blockThreadTryMoot(std::unique_ptr<BlockLabel> bLab);

	/** If LAB accesses a valid location, reports an error  */
	std::optional<VerificationError> checkAccessValidity(const MemAccessLabel *lab);

	/** If LAB accesses an uninitialized location, reports an error */
	std::optional<VerificationError> checkInitializedMem(const ReadLabel *lab);

	/** If LAB accesses improperly initialized memory, reports an error */
	std::optional<VerificationError> checkInitializedMem(const WriteLabel *lab);

	/** If LAB is an IPR read in a location with WW-races, reports an error */
	std::optional<VerificationError> checkIPRValidity(const ReadLabel *rLab);

	/** Checks whether final annotations are used properly in a program:
	 * if there are more than one stores annotated as final at the time WLAB
	 * is added, reports an error */
	std::optional<VerificationError> checkFinalAnnotations(const WriteLabel *wLab);

	/** Liveness: Reports an error on liveness violations */
	void checkLiveness();

	/** Reports an error if there is unfreed memory */
	void checkUnfreedMemory();

	/** Returns true if the exploration is guided by a graph */
	bool isExecutionDrivenByGraph(Event pos);

	/** Returns true if we are in error-replaying mode */
	[[nodiscard]] auto inReplay() const -> bool { return inReplay_; }

	/** Helpers for error replaying (TODO: remove) */
	void startReplay();
	void endReplay();

	/** Opt: Caches LAB to optimize scheduling next time */
	void cacheEventLabel(const EventLabel *lab);

	/** Adds LAB to graph (maintains well-formedness).
	 * If another label exists in the specified position, it is replaced. */
	EventLabel *addLabelToGraph(std::unique_ptr<EventLabel> lab);

	/** Adds each one of LABS to graph (maintains well-formedness) */
	void addLabelsToGraph(const std::vector<std::unique_ptr<EventLabel>> &labs);

	/** Est: Picks (and sets) a random RF among some possible options */
	EventLabel *pickRandomRf(ReadLabel *rLab, std::vector<EventLabel *> &stores);

	/** Est: Picks (and sets) a random CO among some possible options */
	EventLabel *pickRandomCo(WriteLabel *sLab, std::vector<EventLabel *> &cos);

	/** BAM: Reports an error if the executions is not barrier-well-formed.
	 * Returns whether the execution is well-formed */
	bool checkBarrierWellFormedness(BIncFaiWriteLabel *sLab);

	/** BAM: Tries to optimize barrier-related revisits */
	bool tryOptimizeBarrierRevisits(BIncFaiWriteLabel *sLab, std::vector<ReadLabel *> &loads);

	/** IPR: Tries to revisit blocked reads in-place */
	void tryOptimizeIPRs(const WriteLabel *sLab, std::vector<ReadLabel *> &loads);

	/** IPR: Removes a CAS that blocks when reading from SLAB.
	 * Returns whether if the label was removed
	 * (Returns false if RLAB reads from unallocated memory.) */
	bool removeCASReadIfBlocks(const ReadLabel *rLab, const EventLabel *sLab);

	/** Helper: Optimizes revisits of reads that will lead to a failed speculation */
	void optimizeUnconfirmedRevisits(const WriteLabel *sLab, std::vector<ReadLabel *> &loads);

	/** Opt: Tries to optimize revisiting from LAB. It may modify
	 * LOADS, and returns whether we can skip revisiting altogether */
	bool tryOptimizeRevisits(WriteLabel *lab, std::vector<ReadLabel *> &loads);

	/** Constructs a BackwardRevisit representing RLAB <- SLAB */
	std::unique_ptr<BackwardRevisit> constructBackwardRevisit(const ReadLabel *rLab,
								  const WriteLabel *sLab) const;

	/** Given a revisit RLAB <- WLAB, returns the view of the resulting graph.
	 * (This function can be abused and also be utilized for returning the view
	 * of "fictional" revisits, e.g., the view of an event in a maximal path.) */
	std::unique_ptr<VectorClock> getRevisitView(const ReadLabel *rLab,
						    const WriteLabel *sLab) const;

	bool isCoBeforeSavedPrefix(const BackwardRevisit &r, const EventLabel *lab);

	bool coherenceSuccRemainInGraph(const BackwardRevisit &r);

	/** Returns true if all events to be removed by the revisit
	 * RLAB <- SLAB form a maximal extension */
	bool isMaximalExtension(const BackwardRevisit &r);

	/** Returns true if the graph that will be created when sLab revisits rLab
	 * will be the same as the current one */
	bool revisitModifiesGraph(const BackwardRevisit &r) const;

	bool prefixContainsSameLoc(const BackwardRevisit &r, const EventLabel *lab) const;

	/** Calculates all possible coherence placings for SLAB and
	 * pushes them to the worklist. */
	void calcCoOrderings(WriteLabel *sLab, const std::vector<EventLabel *> &cos);

	/** Calculates revisit options and pushes them to the worklist */
	void calcRevisits(WriteLabel *lab);

	/** Modifies the graph accordingly when revisiting a write (MO).
	 * May trigger backward-revisit explorations.
	 * Returns whether the resulting graph should be explored. */
	bool revisitWrite(const WriteForwardRevisit &wi);

	/** Modifies the graph accordingly when revisiting an optional.
	 * Returns true if the resulting graph should be explored */
	bool revisitOptional(const OptionalForwardRevisit &oi);

	/** Modifies (but not restricts) the graph when we are revisiting a read.
	 * Returns true if the resulting graph should be explored. */
	bool revisitRead(const Revisit &s);

	bool forwardRevisit(const ForwardRevisit &fr);
	bool backwardRevisit(const BackwardRevisit &fr);

	/** Adjusts the graph and the worklist according to the backtracking option S.
	 * Returns true if the resulting graph should be explored */
	bool restrictAndRevisit(const WorkList::ItemT &s);

	/** If rLab is the read part of an RMW operation that now became
	 * successful, this function adds the corresponding write part.
	 * Returns a pointer to the newly added event, or nullptr
	 * if the event was not an RMW, or was an unsuccessful one */
	auto completeRevisitedRMW(const ReadLabel *rLab) -> WriteLabel *;

	/** Copies the current EG according to BR's view V.
	 * May modify V but will not execute BR in the copy. */
	std::unique_ptr<ExecutionGraph> copyGraph(const BackwardRevisit *br, VectorClock *v) const;

	/** Given a list of stores that it is consistent to read-from,
	 * filters out options that can be skipped (according to the conf),
	 * and determines the order in which these options should be explored */
	void filterOptimizeRfs(const ReadLabel *lab, std::vector<EventLabel *> &stores);

	bool isExecutionValid(const EventLabel *lab);

	/** Removes rfs from RFS until a consistent option for RLAB is found */
	EventLabel *findConsistentRf(ReadLabel *rLab, std::vector<EventLabel *> &rfs);

	/** Remove cos from COS until a consistent option for WLAB is found */
	EventLabel *findConsistentCo(WriteLabel *wLab, std::vector<EventLabel *> &cos);

	/** SAVer: Checks whether the addition of an event changes our
	 * perspective of a potential spinloop */
	void checkReconsiderFaiSpinloop(const MemAccessLabel *lab);

	/** Opt: Remove possibly invalidated ReadOpt events */
	void checkReconsiderReadOpts(const WriteLabel *sLab);

	/** SAVer: Given the end of a potential FAI-ZNE spinloop,
	 * returns true if it is indeed a spinloop */
	bool areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab);

	/** BAM: Filters out unnecessary rfs for LAB when BAM is enabled */
	void filterConflictingBarriers(const ReadLabel *lab, std::vector<EventLabel *> &stores);

	/** Estimation: Filters outs stores read by RMW loads */
	void filterAtomicityViolations(const ReadLabel *lab, std::vector<EventLabel *> &stores);

	/** IPR: Performs BR in-place */
	void revisitInPlace(const BackwardRevisit &br);

	void repairDanglingReads(ExecutionGraph &g);

	/** Opt: Finds the last memory access that is visible to other threads;
	 * return nullptr if no such access is found */
	const MemAccessLabel *getPreviousVisibleAccessLabel(const EventLabel *start) const;

	/** Opt: Checks whether there is no need to explore the other threads
	 * (e.g., `POS \in B` and will not be removed in all subsequent subexplorations),
	 * and if so moots the current execution */
	void mootExecutionIfFullyBlocked(EventLabel *bLab);

	/** Helper: Wake up any threads blocked on a helping CAS */
	void unblockWaitingHelping(const WriteLabel *lab);

	bool writesBeforeHelpedContainedInView(const HelpedCasReadLabel *lab, const View &view);

	/** Helper: Returns whether there is a valid helped-CAS which the helping-CAS
	 * to be added will be helping. (If an invalid helped-CAS exists, this
	 * method raises an error.) */
	bool checkHelpingCasCondition(const HelpingCasLabel *lab);

	/** Helper: Checks whether the user annotation about helped/helping CASes seems OK */
	void checkHelpingCasAnnotation();

	/** SR: Checks whether CANDIDATE is symmetric to PARENT/INFO */
	bool isSymmetricToSR(int candidate, Event parent, const ThreadInfo &info) const;

	/** SR: Returns the (greatest) ID of a thread that is symmetric to PARENT/INFO */
	int getSymmetricTidSR(const ThreadCreateLabel *tcLab, const ThreadInfo &info) const;

	/** SR: Filter stores that will lead to a symmetric execution */
	void filterSymmetricStoresSR(const ReadLabel *rLab,
				     std::vector<EventLabel *> &stores) const;

	/** SAVer: Filters stores that will lead to an assume-blocked execution */
	void filterValuesFromAnnotSAVER(const ReadLabel *rLab, std::vector<EventLabel *> &stores);

	/*** Estimation-related ***/

	/** Makes an estimation about the state space and updates the current one.
	 * Has to run at the end of an execution */
	void updateStSpaceEstimation();

	/*** Bound-related  ***/

	bool executionExceedsBound(BoundCalculationStrategy strategy) const;

	bool fullExecutionExceedsBound() const;

	bool partialExecutionExceedsBound() const;

#ifdef ENABLE_GENMC_DEBUG
	/** Update bounds histogram with the current, complete execution */
	void trackExecutionBound();
#endif

	/*** Output-related ***/

	/** Returns a view to be used when replaying */
	std::unique_ptr<VectorClock> getReplayView() const;

	/** Prints the source-code instructions leading to Event e.
	 * Assumes that debugging information have already been collected */
	void printTraceBefore(const EventLabel *lab, llvm::raw_ostream &ss = llvm::dbgs());

	/** Helper for printTraceBefore() that prints events according to po U rf */
	void recPrintTraceBefore(const Event &e, View &a, llvm::raw_ostream &ss = llvm::outs());

	/** Returns the name of the variable residing in addr */
	std::string getVarName(const SAddr &addr) const;

	/** Outputs the full graph.
	 * If printMetadata is set, it outputs debugging information
	 * (these should have been collected beforehand) */
	void printGraph(bool printMetadata = false, llvm::raw_ostream &s = llvm::dbgs());

	/** Outputs the current graph into a file (DOT format),
	 * and visually marks events e and c (conflicting).
	 * Assumes debugging information have already been collected  */
	void dotPrintToFile(const std::string &filename, const EventLabel *errLab = nullptr,
			    const EventLabel *racyLab = nullptr, bool printObservation = false);

	void updateLabelViews(EventLabel *lab);
	std::optional<VerificationError> checkForRaces(const EventLabel *lab);

	/** Returns an approximation of consistent rfs for RLAB.
	 * The rfs are ordered according to CO */
	virtual std::vector<EventLabel *> getRfsApproximation(ReadLabel *rLab);

	/** Returns an approximation of the reads that SLAB can revisit.
	 * The reads are ordered in reverse-addition order */
	virtual std::vector<ReadLabel *> getRevisitableApproximation(WriteLabel *sLab);

	/** Returns a vector clock representing the prefix of e,
	 * including e but not e's external dependencies (rf, threadCreate, threadEnd).
	 * Depending on whether dependencies are tracked, the prefix can be
	 * either (po U rf) or (AR U rf) */
	const VectorClock &getPrefixView(const EventLabel *lab) const;

	friend llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const VerificationError &r);

	/** Random generator facilities used */
	using MyRNG = std::mt19937;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;

	/** The operating mode of the driver */
	Mode mode = VerificationMode{};

	/** The thread pool this driver may belong to */
	ThreadPool *pool = nullptr;

	/** User configuration */
	std::shared_ptr<const Config> userConf;

	/** The interpreter used by the driver */
	llvm::Interpreter *EE{};

	/** Execution stack */
	std::vector<Execution> execStack;

	/** Scheduler */
	std::unique_ptr<Scheduler> scheduler_;

	/** Consistency checker (mm-specific) */
	std::unique_ptr<ConsistencyChecker> consChecker;

	/** Symmetry checker) */
	std::unique_ptr<SymmetryChecker> symmChecker;

	/** Decider used to bound the exploration */
	std::unique_ptr<BoundDecider> bounder;

	/** Linearizability checker */
	std::unique_ptr<LinearizabilityChecker> relinche;

	/** Opt: Whether this execution is moot (locking) */
	bool isMootExecution = false;

	/** Verification result to be returned to caller */
	VerificationResult result{};

	/** Whether we are stopping the exploration (e.g., due to an error found) */
	bool shouldHalt = false;

	/** Whether we are in error replaying */
	bool inReplay_ = false;

	/** Dbg: Random-number generators for estimation randomization */
	MyRNG estRng;

	/** Callbacks for querying information from the interpreter. */
	InterpreterCallbacks interpreterCallbacks_;
};

#endif /* GENMC_GENMC_DRIVER_HPP */
