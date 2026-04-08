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

#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Verification/ChoiceMap.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/Relinche/LinearizabilityChecker.hpp"
#include "genmc/Verification/Scheduler.hpp"
#include "genmc/Verification/VerificationError.hpp"
#include "genmc/Verification/VerificationResult.hpp"
#include "genmc/Verification/WorkList.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <random>
#include <utility>
#include <variant>

namespace llvm {
class Interpreter;
class Module;
} // namespace llvm
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
	/** A randomized verification mode: sample executions for a fixed budget */
	struct RandomMode {
		unsigned int budget;
	};
	using Mode = std::variant<VerificationMode, EstimationMode, RandomMode>;

	/** Represents the execution at a given point */
	struct Execution {
		Execution() = delete;
		Execution(std::unique_ptr<ExecutionGraph> g, LocalQueueT &&w, ChoiceMap &&cm);

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

		/** Removes all items with stamp >= STAMP from the execution */
		void restrict(Stamp stamp);

		~Execution();

		std::unique_ptr<ExecutionGraph> graph;
		LocalQueueT workqueue;
		ChoiceMap choices;
	};

	/** Scheduler result type */
	struct Finished {};
	struct Blocked {};
	struct Error {};
	using ScheduleResult = std::variant<Finished, Blocked, Error, int>;

	/** Handler Result Types */
	struct Reset {};
	struct Invalid {};
	template <typename T> using ResultType = std::variant<T, VerificationError, Reset, Invalid>;
	template <typename T> struct HandleResult {
		ResultType<T> result;
		unsigned int count;
	};

	using NALoadResult =
		HandleResult<std::conditional_t<Config::emitNALabels, SVal, std::monostate>>;
	using NAStoreResult =
		HandleResult<std::conditional_t<Config::emitNALabels, bool, std::monostate>>;

	/** Debug information for an instruction */
	struct EventDbgInfo {
		std::string file;
		int line;
		std::string functionName;
		std::string source;
		std::string accessedVarName;
	};

	/** Packs together debug information for a graph */
	using GraphDbgInfo = std::unordered_map<Event, EventDbgInfo>;

	template <typename... Ts> static auto create(Ts &&...params) -> std::unique_ptr<GenMCDriver>
	{
		return std::unique_ptr<GenMCDriver>(new GenMCDriver(std::forward<Ts>(params)...));
	}

	/**** Generic actions ***/

	/** Returns to the interpreter the next thread to run (nullopt if none) */
	auto scheduleNext(std::span<Action> runnable) -> ScheduleResult;

	/** Attemps to complete the execution by inspecting the cache.
	 * Returns whether it succeeded. */
	auto runFromCache() -> bool;

	/** Should be called to register a static allocation, before the execution loop.
	 *  Returns an SAddr.
	 *
	 *  For EMIT_NA_LABELS=ON: @p initData must point to memory that will
	 *  hold the initial bytes. The pointer must remain valid for the driver's lifetime. */
#if EMIT_NA_LABELS
	auto allocateGlobal(ASize size, uint64_t alignment, const void *initData,
			    bool persistent = false, bool internal = false) -> SAddr;
#else
	auto allocateGlobal(ASize size, uint64_t alignment, bool persistent = false,
			    bool internal = false) -> SAddr;
#endif

	/** Should be called at the beginning of each execution. Returns whether the frontend
	 * could collect debug information (non-mandatory) */
	bool handleExecutionStart();

	/** Should be called at the end of each execution */
	auto handleExecutionEnd() -> std::optional<VerificationError>;

	/** Whether there are more executions to be explored */
	bool done();

	/** Returns the result of the verification procedure */
	const VerificationResult &getResult() const { return result; }
	VerificationResult &getResult() { return result; }

	/*** Instruction handling ***/

	/** A thread has just finished execution, nothing for the interpreter */
	HandleResult<std::monostate> handleThreadFinish(const EventDbgInfo *dbg, Event pos,
							SVal val);

	/** A thread has terminated abnormally */
	HandleResult<std::monostate> handleThreadKill(const EventDbgInfo *dbg, Event pos);

	/** This method blocks the current thread  */
	HandleResult<std::monostate> handleAssume(const EventDbgInfo *dbg, Event pos,
						  AssumeType type);

#if EMIT_NA_LABELS
#define GENMC_OLD_VAL_ARG
#define GENMC_NA_STORE_VAL_ARG SVal val,
#else
#define GENMC_NA_STORE_VAL_ARG
#define GENMC_OLD_VAL_ARG std::optional<SVal> oldVal,
#endif

/* Generation of Handler Declarations */
#define HANDLE_LOAD_LABEL(name)                                                                    \
	HandleResult<SVal> handle##name(const EventDbgInfo *dbg, Event pos,                        \
					GENMC_OLD_VAL_ARG MemOrdering ord, SAddr addr, ASize size, \
					EventLabel *rfLab, std::optional<Annotation> annot,        \
					const EventDeps &deps);

#define HANDLE_CAS_LOAD_LABEL(name)                                                                \
	HandleResult<SVal> handle##name(const EventDbgInfo *dbg, Event pos,                        \
					GENMC_OLD_VAL_ARG MemOrdering ord, SAddr addr, ASize size, \
					SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,   \
					std::optional<Annotation> annot, const EventDeps &deps);

#define HANDLE_LOCK_LOAD_LABEL(name)                                                               \
	HandleResult<SVal> handle##name(const EventDbgInfo *dbg, Event pos,                        \
					GENMC_OLD_VAL_ARG SAddr addr, ASize size,                  \
					std::optional<Annotation> annot, const EventDeps &deps);

#define HANDLE_FAI_LOAD_LABEL(name)                                                                \
	HandleResult<SVal> handle##name(const EventDbgInfo *dbg, Event pos,                        \
					GENMC_OLD_VAL_ARG MemOrdering ord, SAddr addr, ASize size, \
					RMWBinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab, \
					std::optional<Annotation> annot, const EventDeps &deps);

#define HANDLE_STORE_LABEL(name)                                                                   \
	HandleResult<bool> handle##name(const EventDbgInfo *dbg, Event pos,                        \
					GENMC_OLD_VAL_ARG MemOrdering ord, SAddr addr, ASize size, \
					SVal val, WriteAttr wattr, const EventDeps &deps);

#define HANDLE_LOCK_STORE_LABEL(name)                                                              \
	HandleResult<bool> handle##name(const EventDbgInfo *dbg, Event pos, SAddr addr,            \
					ASize size, const EventDeps &deps);

#define HANDLE_CAS_STORE_LABEL(name) HANDLE_STORE_LABEL(name)
#define HANDLE_FAI_STORE_LABEL(name) HANDLE_STORE_LABEL(name)

#include "genmc/Execution/EventLabel.def"

	NALoadResult handleNALoad(const EventDbgInfo *dbg, Event pos, SAddr loc, ASize size,
				  const EventDeps &deps);
	NAStoreResult handleNAStore(const EventDbgInfo *dbg, Event pos, SAddr loc, ASize size,
				    GENMC_NA_STORE_VAL_ARG const EventDeps &deps);

#undef GENMC_OLD_VAL_ARG
#undef GENMC_NA_STORE_VAL_ARG

	/** Returns an appropriate result for malloc() */
	HandleResult<SVal> handleMalloc(const EventDbgInfo *dbg, Event pos, ASize size,
					uint64_t alignment, StorageDuration sdur, StorageType styp,
					AddressSpace spc, const NameInfo *info,
					const std::string &name, const EventDeps &deps);

	/** A call to free() has been interpreted, nothing for the intepreter */
	HandleResult<std::monostate> handleFree(const EventDbgInfo *dbg, Event pos, SAddr loc,
						const EventDeps &deps);
	HandleResult<std::monostate> handleRetire(const EventDbgInfo *dbg, Event pos, SAddr loc,
						  const EventDeps &deps);

	/** A fence has been interpreted, nothing for the interpreter */
	HandleResult<std::monostate> handleFence(const EventDbgInfo *dbg, Event pos,
						 MemOrdering ord, const EventDeps &deps);

	/** Returns the TID of the newly created thread */
	HandleResult<int> handleThreadCreate(const EventDbgInfo *dbg, Event pos, ThreadInfo info,
					     const EventDeps &deps);

	/** Returns an appropriate result for pthread_join() */
	HandleResult<SVal> handleThreadJoin(const EventDbgInfo *dbg, Event pos,
					    unsigned int childTid, const EventDeps &deps);

	/** A helping CAS operation has been interpreter.
	 * Returns whether the helped CAS is present. */
	HandleResult<std::monostate> handleHelpingCas(const EventDbgInfo *dbg, Event pos,
						      MemOrdering ord, SAddr loc, ASize size,
						      SVal cmpVal, SVal newVal,
						      const EventDeps &deps);

	/** A call to __VERIFIER_opt_begin() has been interpreted.
	 * Returns whether the block should expand */
	HandleResult<bool> handleOptional(const EventDbgInfo *dbg, Event pos);

	/** A call to __VERIFIER_spin_start() has been interpreted */
	HandleResult<std::monostate> handleSpinStart(const EventDbgInfo *dbg, Event pos);

	/** A call to __VERIFIER_faiZNE_spin_end() has been interpreted */
	HandleResult<std::monostate> handleFaiZNESpinEnd(const EventDbgInfo *dbg, Event pos);

	/** A call to __VERIFIER_lockZNE_spin_end() has been interpreted */
	HandleResult<std::monostate> handleLockZNESpinEnd(const EventDbgInfo *dbg, Event pos);

	/** Helpers for dummy events */
	HandleResult<std::monostate> handleLoopBegin(const EventDbgInfo *dbg, Event pos);
	HandleResult<std::monostate> handleHpProtect(const EventDbgInfo *dbg, Event pos,
						     SAddr hpAddr, SAddr protAddr);
	HandleResult<std::monostate> handleMethodBegin(const EventDbgInfo *dbg, Event pos,
						       std::string methodName, int32_t argVal);
	HandleResult<std::monostate> handleMethodEnd(const EventDbgInfo *dbg, Event pos,
						     std::string methodName, int32_t retVal);
	HandleResult<std::monostate> handleOutput(const EventDbgInfo *dbg, Event pos,
						  std::string msg);
	HandleResult<std::monostate> handleError(const EventDbgInfo *dbg, Event pos,
						 std::string msg);

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
	friend auto sample(const LLIConfig &lliConfig, std::shared_ptr<const Config> conf,
			   std::unique_ptr<llvm::Module> mod, std::unique_ptr<ModuleInfo> modInfo)
		-> VerificationResult;
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

	/** Returns a reference to the current execution */
	Execution &getExec() { return execStack.back(); }
	const Execution &getExec() const { return execStack.back(); }

	/** Returns a reference to the current execution state */
	ExecutionState &getExecState() { return execState_; }
	const ExecutionState &getExecState() const { return execState_; }

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

	/** Est: Returns true if we are currently running in estimation mode */
	bool inEstimationMode() const { return std::holds_alternative<EstimationMode>(mode); }

	/** Returns true if we are currently running in random mode */
	bool inRandomMode() const { return std::holds_alternative<RandomMode>(mode); }

	/** Returns true if we are currently running in exhaustive verification mode */
	bool inVerificationMode() const { return std::holds_alternative<VerificationMode>(mode); }

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

	/** Returns true if we have exhausted the budget */
	bool shouldStopRandom()
	{
		auto remainingBudget = --std::get<RandomMode>(mode).budget;
		return remainingBudget == 0;
	}

private:
	/*** Instruction handling (EventLabel) ***/

	HandleResult<std::monostate> handleThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab);
	HandleResult<std::monostate> handleThreadKill(std::unique_ptr<ThreadKillLabel> lab);
	HandleResult<std::monostate> handleBlock(std::unique_ptr<BlockLabel> bLab);
	HandleResult<SVal> handleLoad(std::unique_ptr<ReadLabel> rLab, std::optional<SVal> oldVal);
	HandleResult<bool> handleStore(std::unique_ptr<WriteLabel> wLab,
				       std::optional<SVal> oldVal);
	HandleResult<std::monostate> handleFence(std::unique_ptr<FenceLabel> fLab);
	HandleResult<int> handleThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab);
	HandleResult<SVal> handleThreadJoin(std::unique_ptr<ThreadJoinLabel> jLab);
	HandleResult<std::monostate> handleHelpingCas(std::unique_ptr<HelpingCasLabel> hLab);
	HandleResult<bool> handleOptional(std::unique_ptr<OptionalLabel> lab);
	HandleResult<std::monostate> handleLoopBegin(std::unique_ptr<LoopBeginLabel> lab);
	HandleResult<std::monostate> handleSpinStart(std::unique_ptr<SpinStartLabel> lab);
	HandleResult<std::monostate> handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab);
	HandleResult<std::monostate> handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab);
	HandleResult<std::monostate> handleDummy(std::unique_ptr<EventLabel> lab);

	NALoadResult handleNALoad(Event pos, SAddr loc, ASize size);
	NAStoreResult handleNAStore(Event pos, SAddr loc, ASize size, std::optional<SVal> val);
	HandleResult<SVal> handleMalloc(Event pos, ASize size, uint64_t alignment,
					StorageDuration sdur, StorageType styp, AddressSpace spc,
					const NameInfo *info, const std::string &name,
					const EventDeps &deps);
	HandleResult<std::monostate> handleFree(Event pos, SAddr loc, const EventDeps &deps);
	HandleResult<std::monostate> handleRetire(Event pos, SAddr loc, const EventDeps &deps);

	/** This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void reportError(Event pos, ErrorDetails &&details);

	/** Helper that reports an unreported warning only if it hasn't reported before.
	 * Returns true if the warning should be treated as an error according to the config. */
	bool reportWarningOnce(Event pos, VerificationError r, const EventLabel *racyLab = nullptr);

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
	std::optional<VerificationError> checkAccessValidity(Event pos, const AAccess &access);

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
	auto checkLiveness() -> std::optional<VerificationError>;

	/** Reports an error if there is unfreed memory */
	auto checkUnfreedMemory() -> std::optional<VerificationError>;

	/** Returns true if the exploration is guided by a graph */
	bool isExecutionDrivenByGraph(Event pos);

	/** Error reporting: initiates an exploration to collect metadata */
	void initiateErrorReplay(const ErrorDetails &details);

	/** Error reporting: stops a metadata-collecting execution (and cleans up) */
	void haltErrorReplay();

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

	std::optional<VerificationError> updateErrorInfoAndMaybeExit(Event pos, bool isNA,
								     const EventDbgInfo *dbg);
	const EventDbgInfo *getDbgInfo(Event pos);

	void updateLabelViews(EventLabel *lab);

	std::optional<VerificationError> checkForMixedSize(MemAccessLabel *lab);

	std::optional<VerificationError> checkForRaces(const EventLabel *lab);

	void configureProbe(MemLabel *rLab, Event pos, SAddr addr, ASize size);

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

	/** Random generator facilities used */
	using MyRNG = std::mt19937;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;

	/** The operating mode of the driver */
	Mode mode = VerificationMode{};

	/** The thread pool this driver may belong to */
	ThreadPool *pool = nullptr;

	/** User configuration */
	std::shared_ptr<const Config> userConf;

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

	/** Random-number generators for estimation randomization */
	MyRNG estRng;

	/** Stores instruction debug info */
	GraphDbgInfo dbgInfo_;

	/** The details of an error to be reported (buffered) */
	std::optional<ErrorDetails> bufferedError_;

	/** "Puppet" labels to be used for race detection */
	std::unique_ptr<ReadLabel> readProbeLab_;
	std::unique_ptr<WriteLabel> writeProbeLab_;
	std::unique_ptr<MallocLabel> mallocProbeLab_;
	std::unique_ptr<FreeLabel> freeProbeLab_;
	std::unique_ptr<HpRetireLabel> retireProbeLab_;

	/** Execution state owned by this driver.  All graphs on the execStack hold
	 *  a non-owning pointer to this object (see ExecutionGraph::setStatePtr). */
	ExecutionState execState_;
};

#endif /* GENMC_GENMC_DRIVER_HPP */
