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
#include <random>
#include <utility>
#include <variant>

namespace llvm {
class Interpreter;
class Module;
} // namespace llvm
struct ModuleInfo;
class ThreadPool;
struct LLIConfig;
class BoundDecider;
class ConsistencyChecker;
class SymmetryChecker;
enum class BoundCalculationStrategy : std::uint8_t;

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
		Execution(std::unique_ptr<ExecutionGraph> g, LocalQueueT &&w, ChoiceMap &&map);

		Execution(const Execution &) = delete;
		auto operator=(const Execution &) -> Execution & = delete;
		Execution(Execution &&) = default;
		auto operator=(Execution &&) -> Execution & = default;

		/** Returns a reference to the current graph (mutable/const getter pair) */
		// NOLINTNEXTLINE(readability-make-member-function-const)
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
		unsigned int count = 0;
	};

	/** CAS-read handlers also return whether the CAS performs its write.
	 * This is necessary for weak CASes, which may spuriously fail. */
	struct CasReadResult {
		SVal val;
		bool success = false;
	};

	using NALoadResult =
		HandleResult<std::conditional_t<Config::emitNALabels, SVal, std::monostate>>;
	using NAStoreResult =
		HandleResult<std::conditional_t<Config::emitNALabels, bool, std::monostate>>;

	/** Debug information for an instruction */
	struct EventDbgInfo {
		std::string file;
		int line = 0;
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
	auto handleExecutionStart() -> bool;

	/** Should be called at the end of each execution */
	auto handleExecutionEnd() -> std::optional<VerificationError>;

	/** Whether there are more executions to be explored */
	auto done() -> bool;

	/** Returns the result of the verification procedure */
	auto getResult() const -> const VerificationResult & { return result; }
	auto getResult() -> VerificationResult & { return result; }

	/*** Instruction handling ***/

	/** A thread has just finished execution, nothing for the interpreter */
	auto handleThreadFinish(const EventDbgInfo *dbg, Event pos, SVal val)
		-> HandleResult<std::monostate>;

	/** A thread has terminated abnormally */
	auto handleThreadKill(const EventDbgInfo *dbg, Event pos) -> HandleResult<std::monostate>;

	/** This method blocks the current thread  */
	auto handleAssume(const EventDbgInfo *dbg, Event pos, AssumeType type)
		-> HandleResult<std::monostate>;

#if EMIT_NA_LABELS
#define GENMC_OLD_VAL_ARG
#define GENMC_NA_STORE_VAL_ARG SVal val,
#else
#define GENMC_NA_STORE_VAL_ARG
#define GENMC_OLD_VAL_ARG std::optional<SVal> oldVal,
#endif

/* Generation of Handler Declarations */
#define HANDLE_LOAD_LABEL(name)                                                                    \
	auto handle##name(const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_ARG MemOrdering ord,   \
			  SAddr addr, ASize size, EventLabel *rfLab,                               \
			  const std::optional<Annotation> &annot, const EventDeps &deps)           \
		-> HandleResult<SVal>;

#define HANDLE_CAS_LOAD_LABEL(name)                                                                \
	auto handle##name(const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_ARG MemOrdering ord,   \
			  MemOrdering failOrd, SAddr addr, ASize size, SVal exp, SVal swap,        \
			  WriteAttr wattr, bool weak, EventLabel *rfLab,                           \
			  const std::optional<Annotation> &annot, const EventDeps &deps)           \
		-> HandleResult<CasReadResult>;

#define HANDLE_LOCK_LOAD_LABEL(name)                                                               \
	auto handle##name(const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_ARG SAddr addr,        \
			  ASize size, const std::optional<Annotation> &annot,                      \
			  const EventDeps &deps) -> HandleResult<SVal>;

#define HANDLE_FAI_LOAD_LABEL(name)                                                                \
	auto handle##name(const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_ARG MemOrdering ord,   \
			  SAddr addr, ASize size, RMWBinOp op, SVal val, WriteAttr wattr,          \
			  EventLabel *rfLab, const std::optional<Annotation> &annot,               \
			  const EventDeps &deps) -> HandleResult<SVal>;

#define HANDLE_STORE_LABEL(name)                                                                   \
	auto handle##name(const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_ARG MemOrdering ord,   \
			  SAddr addr, ASize size, SVal val, WriteAttr wattr,                       \
			  const EventDeps &deps) -> HandleResult<bool>;

#define HANDLE_LOCK_STORE_LABEL(name)                                                              \
	auto handle##name(const EventDbgInfo *dbg, Event pos, SAddr addr, ASize size,              \
			  const EventDeps &deps) -> HandleResult<bool>;

#define HANDLE_CAS_STORE_LABEL(name) HANDLE_STORE_LABEL(name)
#define HANDLE_FAI_STORE_LABEL(name) HANDLE_STORE_LABEL(name)

#include "genmc/Execution/EventLabel.def"

	auto handleNALoad(const EventDbgInfo *dbg, Event pos, SAddr loc, ASize size,
			  const EventDeps &deps) -> NALoadResult;
	auto handleNAStore(const EventDbgInfo *dbg, Event pos, SAddr loc, ASize size,
			   GENMC_NA_STORE_VAL_ARG const EventDeps &deps) -> NAStoreResult;

#undef GENMC_OLD_VAL_ARG
#undef GENMC_NA_STORE_VAL_ARG

	/** Returns an appropriate result for malloc() */
	auto handleMalloc(const EventDbgInfo *dbg, Event pos, ASize size, uint64_t alignment,
			  StorageDuration sdur, StorageType styp, AddressSpace spc,
			  const NameInfo *info, const std::string &name, const EventDeps &deps)
		-> HandleResult<SVal>;

	/** A call to free() has been interpreted, nothing for the intepreter */
	auto handleFree(const EventDbgInfo *dbg, Event pos, SAddr loc, const EventDeps &deps)
		-> HandleResult<std::monostate>;
	auto handleRetire(const EventDbgInfo *dbg, Event pos, SAddr loc, const EventDeps &deps)
		-> HandleResult<std::monostate>;

	/** A fence has been interpreted, nothing for the interpreter */
	auto handleFence(const EventDbgInfo *dbg, Event pos, MemOrdering ord, const EventDeps &deps)
		-> HandleResult<std::monostate>;

	/** Returns the TID of the newly created thread */
	auto handleThreadCreate(const EventDbgInfo *dbg, Event pos, const ThreadInfo &info,
				const EventDeps &deps) -> HandleResult<int>;

	/** Returns an appropriate result for pthread_join() */
	auto handleThreadJoin(const EventDbgInfo *dbg, Event pos, int childTid,
			      const EventDeps &deps) -> HandleResult<SVal>;

	/** A helping CAS operation has been interpreter.
	 * Returns whether the helped CAS is present. */
	auto handleHelpingCas(const EventDbgInfo *dbg, Event pos, MemOrdering ord, SAddr loc,
			      ASize size, SVal cmpVal, SVal newVal, const EventDeps &deps)
		-> HandleResult<std::monostate>;

	/** A call to __VERIFIER_opt_begin() has been interpreted.
	 * Returns whether the block should expand */
	auto handleOptional(const EventDbgInfo *dbg, Event pos) -> HandleResult<bool>;

	/** A call to __VERIFIER_spin_start() has been interpreted */
	auto handleSpinStart(const EventDbgInfo *dbg, Event pos) -> HandleResult<std::monostate>;

	/** A call to __VERIFIER_faiZNE_spin_end() has been interpreted */
	auto handleFaiZNESpinEnd(const EventDbgInfo *dbg, Event pos)
		-> HandleResult<std::monostate>;

	/** A call to __VERIFIER_lockZNE_spin_end() has been interpreted */
	auto handleLockZNESpinEnd(const EventDbgInfo *dbg, Event pos)
		-> HandleResult<std::monostate>;

	/** Helpers for dummy events */
	auto handleLoopBegin(const EventDbgInfo *dbg, Event pos) -> HandleResult<std::monostate>;
	auto handleHpProtect(const EventDbgInfo *dbg, Event pos, SAddr hpAddr, SAddr protAddr)
		-> HandleResult<std::monostate>;
	auto handleMethodBegin(const EventDbgInfo *dbg, Event pos, const std::string &methodName,
			       int32_t argVal) -> HandleResult<std::monostate>;
	auto handleMethodEnd(const EventDbgInfo *dbg, Event pos, const std::string &methodName,
			     int32_t retVal) -> HandleResult<std::monostate>;
	auto handleOutput(const EventDbgInfo *dbg, Event pos, std::string msg)
		-> HandleResult<std::monostate>;
	auto handleError(const EventDbgInfo *dbg, Event pos, std::string msg)
		-> HandleResult<std::monostate>;

	virtual ~GenMCDriver();

	/** Non-copyable and non-movable (constructed via the protected factory below) */
	GenMCDriver(const GenMCDriver &) = delete;
	auto operator=(const GenMCDriver &) -> GenMCDriver & = delete;
	GenMCDriver(GenMCDriver &&) = delete;
	auto operator=(GenMCDriver &&) -> GenMCDriver & = delete;

protected:
	friend class Scheduler;
	friend class ArbitraryScheduler;
	friend class ThreadPool;
	friend class DriverHandlerDispatcher;
	friend void run(GenMCDriver *driver, llvm::Interpreter *interp);
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
		    Mode /*mode*/ = VerificationMode{});

	/** Returns a pointer to the user configuration */
	auto getConf() const -> const Config * { return userConf.get(); }

	/** Returns a reference to the current execution */
	auto getExec() -> Execution & { return execStack.back(); }
	auto getExec() const -> const Execution & { return execStack.back(); }

	/** Returns a reference to the current execution state */
	auto getExecState() -> ExecutionState & { return execState_; }
	auto getExecState() const -> const ExecutionState & { return execState_; }

	/** Returns a reference to the set consistency checker */
	auto getConsChecker() -> ConsistencyChecker & { return *consChecker; }
	auto getConsChecker() const -> const ConsistencyChecker & { return *consChecker; }

	/** Returns a reference to the symmetry checker */
	auto getSymmChecker() -> SymmetryChecker & { return *symmChecker; }
	auto getSymmChecker() const -> const SymmetryChecker & { return *symmChecker; }

	auto getRelinche() -> LinearizabilityChecker & { return *relinche; }
	auto getRelinche() const -> const LinearizabilityChecker & { return *relinche; }

	/** Returns a reference to the scheduler */
	auto getScheduler() -> Scheduler & { return *scheduler_; }
	auto getScheduler() const -> const Scheduler & { return *scheduler_; }

	/** Stops the verification procedure when an error is found */
	void halt(VerificationError status);

	/** Pushes E to the execution stack. */
	void pushExecution(Execution &&e);

	/** Pops the top stack entry.
	 * Returns false if the stack is empty or this was the last entry. */
	auto popExecution() -> bool;

	/** Gets/sets the thread pool this driver should account to */
	auto getThreadPool() -> ThreadPool * { return pool; }
	auto getThreadPool() const -> ThreadPool * { return pool; }
	void setThreadPool(ThreadPool *threadPool) { pool = threadPool; }

	/** Initializes the exploration from a given state */
	void initFromState(std::unique_ptr<Execution> exec);

	/** Extracts the current driver state.
	 * The driver is left in an inconsistent form */
	auto extractState() -> std::unique_ptr<Execution>;

	/** Returns the value that a read is reading. This function should be
	 * used when calculating the value that we should return to the
	 * interpreter. */
	auto getReadRetValue(const ReadLabel *rLab) -> HandleResult<SVal>;

	/** Wraps the value ready by a CAS into a CasReadResult, computing
	 * the success flag from the read label at @p pos. Non-value results
	 * (errors, resets) are propagated unchanged. */
	auto toCasResult(HandleResult<SVal> res, Event pos) -> HandleResult<CasReadResult>;

	/** If @p cLab is a weak CAS that just became a successful RMW, pushes a
	 * worklist item to also explore its spurious-failure outcome. */
	void maybePushWeakCasFailure(const CasReadLabel *cLab);

	/** Est: Returns true if we are currently running in estimation mode */
	auto inEstimationMode() const -> bool
	{
		return std::holds_alternative<EstimationMode>(mode);
	}

	/** Returns true if we are currently running in random mode */
	auto inRandomMode() const -> bool { return std::holds_alternative<RandomMode>(mode); }

	/** Returns true if we are currently running in exhaustive verification mode */
	auto inVerificationMode() const -> bool
	{
		return std::holds_alternative<VerificationMode>(mode);
	}

	/** Est: Returns true if the estimation seems "good enough" */
	auto shouldStopEstimating() -> bool
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
	auto shouldStopRandom() -> bool
	{
		auto remainingBudget = --std::get<RandomMode>(mode).budget;
		return remainingBudget == 0;
	}

private:
	/*** Instruction handling (EventLabel) ***/

	auto handleThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab)
		-> HandleResult<std::monostate>;
	auto handleThreadKill(std::unique_ptr<ThreadKillLabel> lab) -> HandleResult<std::monostate>;
	auto handleBlock(std::unique_ptr<BlockLabel> bLab) -> HandleResult<std::monostate>;
	auto handleLoad(std::unique_ptr<ReadLabel> rLab, std::optional<SVal> oldVal)
		-> HandleResult<SVal>;
	auto handleStore(std::unique_ptr<WriteLabel> wLab, std::optional<SVal> oldVal)
		-> HandleResult<bool>;
	auto handleFence(std::unique_ptr<FenceLabel> fLab) -> HandleResult<std::monostate>;
	auto handleThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab) -> HandleResult<int>;
	auto handleThreadJoin(std::unique_ptr<ThreadJoinLabel> jLab) -> HandleResult<SVal>;
	auto handleHelpingCas(std::unique_ptr<HelpingCasLabel> hLab)
		-> HandleResult<std::monostate>;
	auto handleOptional(std::unique_ptr<OptionalLabel> lab) -> HandleResult<bool>;
	auto handleLoopBegin(std::unique_ptr<LoopBeginLabel> lab) -> HandleResult<std::monostate>;
	auto handleSpinStart(std::unique_ptr<SpinStartLabel> lab) -> HandleResult<std::monostate>;
	auto handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab)
		-> HandleResult<std::monostate>;
	auto handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab)
		-> HandleResult<std::monostate>;
	auto handleDummy(std::unique_ptr<EventLabel> lab) -> HandleResult<std::monostate>;

	auto handleNALoad(Event pos, SAddr loc, ASize size) -> NALoadResult;
	auto handleNAStore(Event pos, SAddr loc, ASize size, std::optional<SVal> val)
		-> NAStoreResult;
	auto handleMalloc(Event pos, ASize size, uint64_t alignment, StorageDuration sdur,
			  StorageType styp, AddressSpace spc, const NameInfo *info,
			  const std::string &name, const EventDeps &deps) -> HandleResult<SVal>;
	auto handleFree(Event pos, SAddr loc, const EventDeps &deps)
		-> HandleResult<std::monostate>;
	auto handleRetire(Event pos, SAddr loc, const EventDeps &deps)
		-> HandleResult<std::monostate>;

	/** This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void reportError(Event pos, const ErrorDetails &details);

	/** Helper that reports an unreported warning only if it hasn't reported before.
	 * Returns true if the warning should be treated as an error according to the config. */
	auto reportWarningOnce(Event pos, VerificationError wcode,
			       const EventLabel *racyLab = nullptr) -> bool;

	/*** Exploration-related ***/

	/** Returns whether a revisit results to a valid execution
	 * (e.g., consistent, accessing allocated memory, etc) */
	auto isRevisitValid(const Revisit &revisit) -> bool;

	/** Returns true if this driver is shutting down */
	auto isHalting() const -> bool;

	/** Returns true if this execution is moot */
	auto isMoot() const -> bool { return isMootExecution; }

	/** Opt: Mark current execution as moot/normal */
	void moot() { isMootExecution = true; }
	void unmoot() { isMootExecution = false; }

	/** Blocks thread at POS with type T. Tries to moot afterward */
	void blockThreadTryMoot(std::unique_ptr<BlockLabel> bLab);

	/** If LAB accesses a valid location, reports an error  */
	auto checkAccessValidity(Event pos, const AAccess &access)
		-> std::optional<VerificationError>;

	/** If LAB accesses an uninitialized location, reports an error */
	auto checkInitializedMem(const ReadLabel *lab) -> std::optional<VerificationError>;

	/** If LAB accesses improperly initialized memory, reports an error */
	auto checkInitializedMem(const WriteLabel *lab) -> std::optional<VerificationError>;

	/** If LAB is an IPR read in a location with WW-races, reports an error */
	auto checkIPRValidity(const ReadLabel *rLab) -> std::optional<VerificationError>;

	/** Checks whether final annotations are used properly in a program:
	 * if there are more than one stores annotated as final at the time WLAB
	 * is added, reports an error */
	auto checkFinalAnnotations(const WriteLabel *wLab) -> std::optional<VerificationError>;

	/** Liveness: Reports an error on liveness violations */
	auto checkLiveness() -> std::optional<VerificationError>;

	/** Reports an error if there is unfreed memory */
	auto checkUnfreedMemory() -> std::optional<VerificationError>;

	/** Returns true if the exploration is guided by a graph */
	auto isExecutionDrivenByGraph(Event curr) -> bool;

	/** Error reporting: initiates an exploration to collect metadata */
	void initiateErrorReplay(const ErrorDetails &details);

	/** Error reporting: stops a metadata-collecting execution (and cleans up) */
	void haltErrorReplay();

	/** Opt: Caches LAB to optimize scheduling next time */
	void cacheEventLabel(const EventLabel *lab);

	/** Adds LAB to graph (maintains well-formedness).
	 * If another label exists in the specified position, it is replaced. */
	auto addLabelToGraph(std::unique_ptr<EventLabel> lab) -> EventLabel *;

	/** Adds each one of LABS to graph (maintains well-formedness) */
	void addLabelsToGraph(const std::vector<std::unique_ptr<EventLabel>> &labs);

	/** Est: Picks (and sets) a random RF among some possible options */
	auto pickRandomRf(ReadLabel *rLab, std::vector<EventLabel *> &stores) -> EventLabel *;

	/** Est: Picks (and sets) a random CO among some possible options */
	auto pickRandomCo(WriteLabel *sLab, std::vector<EventLabel *> &cos) -> EventLabel *;

	/** BAM: Reports an error if the executions is not barrier-well-formed.
	 * Returns whether the execution is well-formed */
	auto checkBarrierWellFormedness(BIncFaiWriteLabel *sLab) -> bool;

	/** BAM: Tries to optimize barrier-related revisits */
	auto tryOptimizeBarrierRevisits(BIncFaiWriteLabel *sLab, std::vector<ReadLabel *> &loads)
		-> bool;

	/** IPR: Tries to revisit blocked reads in-place */
	void tryOptimizeIPRs(const WriteLabel *sLab, std::vector<ReadLabel *> &loads);

	/** IPR: Removes a CAS that blocks when reading from SLAB.
	 * Returns whether if the label was removed
	 * (Returns false if RLAB reads from unallocated memory.) */
	auto removeCASReadIfBlocks(const ReadLabel *rLab, const EventLabel *sLab) -> bool;

	/** Helper: Optimizes revisits of reads that will lead to a failed speculation */
	void optimizeUnconfirmedRevisits(const WriteLabel *sLab, std::vector<ReadLabel *> &loads);

	/** Opt: Tries to optimize revisiting from LAB. It may modify
	 * LOADS, and returns whether we can skip revisiting altogether */
	auto tryOptimizeRevisits(WriteLabel *lab, std::vector<ReadLabel *> &loads) -> bool;

	/** Constructs a BackwardRevisit representing RLAB <- SLAB */
	auto constructBackwardRevisit(const ReadLabel *rLab, const WriteLabel *sLab) const
		-> std::unique_ptr<BackwardRevisit>;

	/** Given a revisit RLAB <- WLAB, returns the view of the resulting graph.
	 * (This function can be abused and also be utilized for returning the view
	 * of "fictional" revisits, e.g., the view of an event in a maximal path.) */
	auto getRevisitView(const ReadLabel *rLab, const WriteLabel *sLab) const
		-> std::unique_ptr<VectorClock>;

	auto isCoBeforeSavedPrefix(const BackwardRevisit &r, const EventLabel *lab) -> bool;

	auto coherenceSuccRemainInGraph(const BackwardRevisit &r) -> bool;

	/** Returns true if all events to be removed by the revisit
	 * RLAB <- SLAB form a maximal extension */
	auto isMaximalExtension(const BackwardRevisit &r) -> bool;

	auto prefixContainsSameLoc(const BackwardRevisit &r, const EventLabel *lab) const -> bool;

	/** Calculates all possible coherence placings for SLAB and
	 * pushes them to the worklist. */
	void calcCoOrderings(WriteLabel *sLab, const std::vector<EventLabel *> &cos);

	/** Calculates revisit options and pushes them to the worklist */
	void calcRevisits(WriteLabel *lab);

	/** Modifies the graph accordingly when revisiting a write (MO).
	 * May trigger backward-revisit explorations.
	 * Returns whether the resulting graph should be explored. */
	auto revisitWrite(const WriteForwardRevisit &ri) -> bool;

	/** Modifies the graph accordingly when revisiting an optional.
	 * Returns true if the resulting graph should be explored */
	auto revisitOptional(const OptionalForwardRevisit &oi) -> bool;

	/** Spuriously fails a weak CAS: the read keeps its rf,
	 * but no write is performed (the CAS is marked).
	 * Returns true if the resulting graph should be explored. */
	auto revisitWeakCasFailure(const WeakCasFailureRevisit &ri) -> bool;

	/** Modifies (but not restricts) the graph when we are revisiting a read.
	 * Returns true if the resulting graph should be explored. */
	auto revisitRead(const Revisit &ri) -> bool;

	auto forwardRevisit(const ForwardRevisit &fr) -> bool;
	auto backwardRevisit(const BackwardRevisit &br) -> bool;

	/** Adjusts the graph and the worklist according to the backtracking option S.
	 * Returns true if the resulting graph should be explored */
	auto restrictAndRevisit(const WorkList::ItemT &item) -> bool;

	/** If rLab is the read part of an RMW operation that now became
	 * successful, this function adds the corresponding write part.
	 * Returns a pointer to the newly added event, or nullptr
	 * if the event was not an RMW, or was an unsuccessful one */
	auto completeRevisitedRMW(const ReadLabel *rLab) -> WriteLabel *;

	/** Copies the current EG according to BR's view V.
	 * May modify V but will not execute BR in the copy. */
	auto copyGraph(const BackwardRevisit *br, VectorClock *v) const
		-> std::unique_ptr<ExecutionGraph>;

	/** Given a list of stores that it is consistent to read-from,
	 * filters out options that can be skipped (according to the conf),
	 * and determines the order in which these options should be explored */
	void filterOptimizeRfs(const ReadLabel *lab, std::vector<EventLabel *> &stores);

	auto isExecutionValid(const EventLabel *lab) -> bool;

	/** Removes rfs from RFS until a consistent option for RLAB is found */
	auto findConsistentRf(ReadLabel *rLab, std::vector<EventLabel *> &rfs) -> EventLabel *;

	/** Remove cos from COS until a consistent option for WLAB is found */
	auto findConsistentCo(WriteLabel *wLab, std::vector<EventLabel *> &cos) -> EventLabel *;

	/** SAVer: Checks whether the addition of an event changes our
	 * perspective of a potential spinloop */
	void checkReconsiderFaiSpinloop(const MemAccessLabel *lab);

	/** Opt: Remove possibly invalidated ReadOpt events */
	void checkReconsiderReadOpts(const WriteLabel *sLab);

	/** SAVer: Given the end of a potential FAI-ZNE spinloop,
	 * returns true if it is indeed a spinloop */
	auto areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab) -> bool;

	/** BAM: Filters out unnecessary rfs for LAB when BAM is enabled */
	void filterConflictingBarriers(const ReadLabel *lab, std::vector<EventLabel *> &stores);

	/** Estimation: Filters outs stores read by RMW loads */
	void filterAtomicityViolations(const ReadLabel *lab, std::vector<EventLabel *> &stores);

	/** IPR: Performs BR in-place */
	void revisitInPlace(const BackwardRevisit &br);

	void repairDanglingReads(ExecutionGraph &g);

	/** Opt: Finds the last memory access that is visible to other threads;
	 * return nullptr if no such access is found */
	auto getPreviousVisibleAccessLabel(const EventLabel *start) const -> const MemAccessLabel *;

	/** Opt: Checks whether there is no need to explore the other threads
	 * (e.g., `POS \in B` and will not be removed in all subsequent subexplorations),
	 * and if so moots the current execution */
	void mootExecutionIfFullyBlocked(EventLabel *bLab);

	/** Helper: Wake up any threads blocked on a helping CAS */
	void unblockWaitingHelping(const WriteLabel *lab);

	auto writesBeforeHelpedContainedInView(const HelpedCasReadLabel *lab, const View &view)
		-> bool;

	/** Helper: Returns whether there is a valid helped-CAS which the helping-CAS
	 * to be added will be helping. (If an invalid helped-CAS exists, this
	 * method raises an error.) */
	auto checkHelpingCasCondition(const HelpingCasLabel *lab) -> bool;

	/** Helper: Checks whether the user annotation about helped/helping CASes seems OK */
	void checkHelpingCasAnnotation();

	/** SR: Checks whether CANDIDATE is symmetric to PARENT/INFO */
	auto isSymmetricToSR(int candidate, Event parent, const ThreadInfo &info) const -> bool;

	/** SR: Returns the (greatest) ID of a thread that is symmetric to PARENT/INFO */
	auto getSymmetricTidSR(const ThreadCreateLabel *tcLab, const ThreadInfo &info) const -> int;

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

	auto executionExceedsBound(BoundCalculationStrategy strategy) const -> bool;

	auto fullExecutionExceedsBound() const -> bool;

	auto partialExecutionExceedsBound() const -> bool;

#ifdef ENABLE_GENMC_DEBUG
	/** Update bounds histogram with the current, complete execution */
	void trackExecutionBound();
#endif

	/*** Output-related ***/

	auto updateErrorInfoAndMaybeExit(Event pos, bool isNA, const EventDbgInfo *dbg)
		-> std::optional<VerificationError>;
	auto getDbgInfo(Event pos) -> const EventDbgInfo *;

	void updateLabelViews(EventLabel *lab);

	auto checkForMixedSize(MemAccessLabel *lab) -> std::optional<VerificationError>;

	auto checkForRaces(const EventLabel *lab) -> std::optional<VerificationError>;

	void configureProbe(MemLabel *rLab, Event pos, SAddr addr, ASize size);

	/** Returns an approximation of consistent rfs for RLAB.
	 * The rfs are ordered according to CO */
	virtual auto getRfsApproximation(ReadLabel *rLab) -> std::vector<EventLabel *>;

	/** Returns an approximation of the reads that SLAB can revisit.
	 * The reads are ordered in reverse-addition order */
	virtual auto getRevisitableApproximation(WriteLabel *sLab) -> std::vector<ReadLabel *>;

	/** Returns a vector clock representing the prefix of e,
	 * including e but not e's external dependencies (rf, threadCreate, threadEnd).
	 * Depending on whether dependencies are tracked, the prefix can be
	 * either (po U rf) or (AR U rf) */
	auto getPrefixView(const EventLabel *lab) const -> const VectorClock &;

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
	VerificationResult result;

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
