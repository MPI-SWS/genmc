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
#include "Event.hpp"
#include "EventLabel.hpp"
#include "RevisitSet.hpp"
#include "WorkSet.hpp"
#include "Library.hpp"
#include <llvm/IR/Module.h>

#include <ctime>
#include <memory>
#include <random>
#include <unordered_set>

namespace llvm {
	struct ExecutionContext;
	class Interpreter;
}
class ExecutionGraph;

class GenMCDriver {

public:
	/* Different error types that may occur.
	 * Public to enable the interpreter utilize it */
	enum DriverErrorKind {
		DE_Safety,
		DE_Recovery,
		DE_Liveness,
		DE_RaceNotAtomic,
		DE_RaceFreeMalloc,
		DE_FreeNonMalloc,
		DE_DoubleFree,
		DE_Allocation,
		DE_InvalidAccessBegin,
		DE_UninitializedMem,
		DE_AccessNonMalloc,
		DE_AccessFreed,
		DE_InvalidAccessEnd,
		DE_InvalidJoin,
		DE_InvalidUnlock,
		DE_InvalidBInit,
		DE_InvalidRecoveryCall,
		DE_InvalidTruncate,
		DE_SystemError,
	};

private:
	static bool isInvalidAccessError(DriverErrorKind e) {
		return DE_InvalidAccessBegin <= e &&
			e <= DE_InvalidAccessEnd;
	};

public:
	/* Returns a list of the libraries the specification of which are given */
	const std::vector<Library> &getGrantedLibs()  const { return grantedLibs; };

	/* Returns a list of the libraries which need to be verified (TODO) */
	const std::vector<Library> &getToVerifyLibs() const { return toVerifyLibs; };

	/**** Generic actions ***/

	/* Starts the verification procedure */
	void run();

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

	/*** Instruction-related actions ***/

	/* Returns the value this load reads */
	llvm::GenericValue
	visitLoad(InstAttr attr,
		  llvm::AtomicOrdering ord,
		  const llvm::GenericValue *addr,
		  llvm::Type *typ,
		  llvm::GenericValue cmpVal = llvm::GenericValue(),
		  llvm::GenericValue rmwVal = llvm::GenericValue(),
		  llvm::AtomicRMWInst::BinOp op =
		  llvm::AtomicRMWInst::BinOp::BAD_BINOP);

	/* Returns the value this load reads, as well as whether
	 * the interpreter should block due to a blocking library read */
	std::pair<llvm::GenericValue, bool>
	visitLibLoad(InstAttr attr,
		     llvm::AtomicOrdering ord,
		     const llvm::GenericValue *addr,
		     llvm::Type *typ,
		     std::string functionName);

	/* A function modeling a write to disk has been interpreted.
	 * Returns the value read */
	llvm::GenericValue
	visitDskRead(const llvm::GenericValue *readAddr, llvm::Type *typ);

	/* A store has been interpreted, nothing for the interpreter */
	void
	visitStore(InstAttr attr,
		   llvm::AtomicOrdering ord,
		   const llvm::GenericValue *addr,
		   llvm::Type *typ,
		   const llvm::GenericValue &val);

	/* A lib store has been interpreted, nothing for the interpreter */
	void
	visitLibStore(InstAttr attr,
		      llvm::AtomicOrdering ord,
		      const llvm::GenericValue *addr,
		      llvm::Type *typ,
		      llvm::GenericValue &val,
		      std::string functionName,
		      bool isInit = false);

	/* A function modeling a write to disk has been interpreted */
	void
	visitDskWrite(const llvm::GenericValue *addr,
		      llvm::Type *typ,
		      const llvm::GenericValue &val,
		      void *mapping,
		      InstAttr attr = InstAttr::IA_None,
		      std::pair<void *, void *> ordDataRange =
		        std::pair<void *, void *>{(void *) nullptr, (void *) nullptr},
		      void *transInode = nullptr);

	/* A lock() operation has been interpreted, nothing for the interpreter */
	void visitLock(const llvm::GenericValue *addr, llvm::Type *typ);

	/* An unlock() operation has been interpreted, nothing for the interpreter */
	void visitUnlock(const llvm::GenericValue *addr, llvm::Type *typ);

	/* A function modeling the beginning of the opening of a file.
	 * The interpreter will get back the file descriptor */
	llvm::GenericValue
	visitDskOpen(const char *fileName, llvm::Type *intTyp);

	/* An fsync() operation has been interpreted */
	void
	visitDskFsync(void *inodeData, unsigned int size);

	/* A sync() operation has been interpreted */
	void
	visitDskSync();

	/* A call to __VERIFIER_pbarrier() has been interpreted */
	void
	visitDskPbarrier();

	/* A fence has been interpreted, nothing for the interpreter.
	 * For LKMM fences, the argument lkmmType points to a description
	 * of the fence's actual type */
	void
	visitFence(llvm::AtomicOrdering ord, const char *lkmmType = nullptr);

	/* A call to __VERIFIER_loop_begin() has been interpreted */
	void
	visitLoopBegin();

	/* A call to __VERIFIER_spin_start() has been interpreted */
	void
	visitSpinStart();

	/* A call to __VERIFIER_faiZNE_spin_end() has been interpreted */
	void
	visitFaiZNESpinEnd();

	/* A call to __VERIFIER_lockZNE_spin_end() has been interpreted */
	void
	visitLockZNESpinEnd();

	/* Returns an appropriate result for pthread_self() */
	llvm::GenericValue
	visitThreadSelf(llvm::Type *typ);

	/* Returns the TID of the newly created thread */
	int
	visitThreadCreate(llvm::Function *F, const llvm::GenericValue &arg, const llvm::ExecutionContext &SF);

	/* Returns an appropriate result for pthread_join() */
	llvm::GenericValue
	visitThreadJoin(llvm::Function *F, const llvm::GenericValue &arg);

	/* A thread has just finished execution, nothing for the interpreter */
	void
	visitThreadFinish();

	/* Returns an appropriate result for malloc() */
	llvm::GenericValue
	visitMalloc(uint64_t allocSize, unsigned int alignment, Storage s, AddressSpace spc);

	/* A call to free() has been interpreted, nothing for the intepreter */
	void
	visitFree(void *ptr);
	/* Helper for bulk-deallocs */
	template<typename ITER>
	void visitFree(ITER begin, ITER end) {
		for (auto it = begin; it != end; ++it)
			visitFree(*it);
	}

	/* LKMM: Visit RCU functions */
	void
	visitRCULockLKMM();
	void
	visitRCUUnlockLKMM();
	void
	visitRCUSyncLKMM();

	/* This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void
	visitError(DriverErrorKind t, const std::string &err = std::string(),
		   Event confEvent = Event::getInitializer());

	virtual ~GenMCDriver();

protected:

	GenMCDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod, clock_t start);

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
	llvm::GenericValue getWriteValue(Event w,
					 const llvm::GenericValue *a,
					 const llvm::Type *t);
	llvm::GenericValue getDskWriteValue(Event w,
					    const llvm::GenericValue *a,
					    const llvm::Type *t);

	/* Returns the value that a read is reading. This function should be
	 * used when calculating the value that we should return to the
	 * interpreter; if the read is reading from an invalid place
	 * (e.g., bottom) also blocks the currently running thread. */
	llvm::GenericValue getReadRetValueAndMaybeBlock(Event r,
							const llvm::GenericValue *addr,
							const llvm::Type *t);
	llvm::GenericValue getRecReadRetValue(const llvm::GenericValue *addr,
					      const llvm::Type *typ);

	/* Returns the value with which a barrier at PTR has been initialized */
	llvm::GenericValue getBarrierInitValue(const llvm::GenericValue *ptr,
					       const llvm::Type *typ);

	/* Returns true if we should check consistency at p */
	bool shouldCheckCons(ProgramPoint p);

	/* Returns true if full consistency needs to be checked at p.
	 * Assumes that consistency needs to be checked anyway. */
	bool shouldCheckFullCons(ProgramPoint p);

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

	/* Returns true if a is hb-before b */
	bool isHbBefore(Event a, Event b, ProgramPoint p = ProgramPoint::step);

	/* Returns true if e is maximal in addr */
	bool isCoMaximal(const llvm::GenericValue *addr, Event e,
			 bool checkCache = false, ProgramPoint p = ProgramPoint::step);

private:
	/*** Worklist-related ***/

	/* Adds an appropriate entry to the worklist */
	void addToWorklist(std::unique_ptr<WorkItem> item);

	/* Fetches the next backtrack option.
	 * A default-constructed item means that the list is empty */
	std::unique_ptr<WorkItem> getNextItem();

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

	/* Resets some options before the beginning of a new execution */
	void resetExplorationOptions();

	/* Sets up a prioritization scheme among threads */
	void prioritizeThreads();

	/* Deprioritizes the current thread */
	void deprioritizeThread();

	/* Returns true if THREAD is schedulable (i.e., there are more
	 * instructions to run and it is not blocked) */
	bool isSchedulable(int thread) const;

	/* Tries to schedule according to the current prioritization scheme */
	bool schedulePrioritized();

	/* Returns true if the next instruction of TID is a load
	 * Note: assumes there is a next instruction in TID*/
	bool isNextThreadInstLoad(int tid);

	/* Helpers that try to schedule the next thread according to
	 * the chosen policy */
	bool scheduleNextLTR();
	bool scheduleNextWF();
	bool scheduleNextRandom();

	/* Resets the prioritization scheme */
	void resetThreadPrioritization();

	/* Returns whether ADDR a valid address or not.  */
	bool isAccessValid(const llvm::GenericValue *addr);

	/* Checks for data races when a read/write is added.
	 * Appropriately calls visitError() and terminates */
	void checkForDataRaces();

	/* Performs POSIX checks whenever a lock event is added.
	 * Given its list of possible rfs, makes sure it cannot read
	 * from a destroyed lock.
	 * Appropriately calls visitErro() and terminates */
	void checkLockValidity(const std::vector<Event> &rfs);

	/* Performs POSIX checks whenever an unlock event is added.
	 * Appropriately calls visitError() and terminates */
	void checkUnlockValidity();

	/* Perfoms POSIX checks whenever a barrier_init event is added.
	 Appropriately calls visitError() and terminates */
	void checkBInitValidity();

	/* Perfoms POSIX checks whenever a barrier_wait event is added.
	 Appropriately calls visitError() and terminates */
	void checkBIncValidity(const std::vector<Event> &rfs);

	/* Checks whether there is some race when allocating/deallocating
	 * memory and reports an error as necessary.
	 * Helpers for checkForMemoryRaces() */
	void findMemoryRaceForMemAccess(const MemAccessLabel *mLab);
	void findMemoryRaceForAllocAccess(const FreeLabel *fLab);

	/* Checks for memory races (e.g., double free, access freed memory, etc)
	 * whenever a read/write/free is added.
	 * Appropriately calls visitError() and terminates */
	void checkForMemoryRaces(const void *addr);

	/* Calls visitError() if a newly added read can read from an uninitialized
	 * (dynamically allocated) memory location */
	void checkForUninitializedMem(const std::vector<Event> &rfs);

	/* Returns true if the exploration is guided by a graph */
	bool isExecutionDrivenByGraph();

	/* Pers: Returns true if we are currently running the recovery routine */
	bool inRecoveryMode() const;

	/* If the execution is guided, returns the corresponding label for
	 * this instruction. Reports an error if the execution is not guided */
	const EventLabel *getCurrentLabel() const;

	/* Calculates revisit options and pushes them to the worklist.
	 * Returns true if the current exploration should continue */
	bool calcRevisits(const WriteLabel *lab);
	bool calcLibRevisits(const EventLabel *lab);

	/* Adjusts the graph and the worklist for the next backtracking option.
	 * Returns true if the resulting graph should be explored */
	bool revisitReads(std::unique_ptr<WorkItem> s);

	/* If rLab is the read part of an RMW operation that now became
	 * successful, this function adds the corresponding write part.
	 * Returns a pointer to the newly added event, or nullptr
	 * if the event was not an RMW, or was an unsuccessful one */
	const WriteLabel *completeRevisitedRMW(const ReadLabel *rLab);

	/* Informs the interpreter about events being deleted before restriction */
	void notifyEERemoved(unsigned int cutStamp);

	/* Removes all labels with stamp >= st from the graph */
	void restrictGraph(const EventLabel *lab);

	/* Informs the interpreter about events being restored before restoration */
	void notifyEERestored(const std::vector<std::unique_ptr<EventLabel> > &prefix);

	/* Restores the previously saved prefix and coherence status */
	void restorePrefix(const EventLabel *lab,
			   std::vector<std::unique_ptr<EventLabel> > &&prefix,
			   std::vector<std::pair<Event, Event> > &&moPlacings);

	/* Given a list of stores that it is consistent to read-from,
	 * removes options that violate atomicity, and determines the
	 * order in which these options should be explored */
	std::vector<Event> properlyOrderStores(InstAttr attr,
					       llvm::Type *typ,
					       const llvm::GenericValue *ptr,
					       llvm::GenericValue &expVal,
					       std::vector<Event> &stores);

	/* Helper for visitLoad() that creates a ReadLabel and adds it to the graph */
	const ReadLabel *
	createAddReadLabel(InstAttr attr,
			   llvm::AtomicOrdering ord,
			   const llvm::GenericValue *addr,
			   llvm::Type *typ,
			   std::unique_ptr<SExpr> annot,
			   const llvm::GenericValue &cmpVal,
			   const llvm::GenericValue &rmwVal,
			   llvm::AtomicRMWInst::BinOp op,
			   Event store);

	/* Removes rfs from "rfs" until a consistent option for rLab is found,
	 * if that is dictated by the CLI options */
	bool ensureConsistentRf(const ReadLabel *rLab, std::vector<Event> &rfs);

	/* Helper for visitStore() that creates a WriteLabel and adds it to the graph */
	const WriteLabel *
	createAddStoreLabel(InstAttr attr,
			    llvm::AtomicOrdering ord,
			    const llvm::GenericValue *addr,
			    llvm::Type *typ,
			    const llvm::GenericValue &val, int moPos);

	/* Makes sure that the current graph is consistent, if that is dictated
	 * by the CLI options. Since that is not always the case for stores
	 * (e.g., w/ LAPOR), it returns whether it is the case or not */
	bool ensureConsistentStore(const WriteLabel *wLab);

	/* Pers: removes _all_ options from "rfs" that make the recovery invalid.
	 * Sets the rf of rLab to the first valid option in rfs */
	void filterInvalidRecRfs(const ReadLabel *rLab, std::vector<Event> &rfs);

	std::vector<Event>
	getLibConsRfsInView(const Library &lib, Event read,
			    const std::vector<Event> &stores,
			    const View &v);

	/* SAVer: Checks whether the effects of a write are observable */
	bool isWriteObservable(const WriteLabel *lab);

	/* SAVer: Checks whether the addition of an event changes our
	 * perspective of a potential spinloop */
	void checkReconsiderFaiSpinloop(const MemAccessLabel *lab);

	/* SAVer: Given the end of a potential FAI-ZNE spinloop,
	 * returns true if it is indeed a spinloop */
	bool areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab);

	/* Opt: Futher reduces the set of available read-from options for a
	 * read that is part of a lock() op. Returns the filtered set of RFs  */
	std::vector<Event> filterAcquiredLocks(const llvm::GenericValue *ptr,
					       const std::vector<Event> &stores,
					       const VectorClock &before);

	/* Opt: Tries to in-place revisit a read that is part of a lock.
	 * Returns true if the optimization succeeded */
	bool tryToRevisitLock(const CasReadLabel *rLab, const WriteLabel *sLab,
			      const std::vector<Event> &writePrefixPos,
			      const std::vector<std::pair<Event, Event> > &moPlacings);

	/* Opt: Repairs the reads-from edge of a dangling lock */
	void repairLock(LockCasReadLabel *lab);

	/* Opt: Repairs some locks that may be "dangling", as part of the
	 * in-place revisiting of locks */
	void repairDanglingLocks();

	/* Opt: Repairs barriers that may be "dangling" after cutting the graph. */
	void repairDanglingBarriers();

	/* LKMM: Helper for visiting LKMM fences */
	void visitFenceLKMM(llvm::AtomicOrdering ord, const char *lkmmType);

	/* LAPOR: Returns whether the current execution is lock-well-formed */
	bool isLockWellFormedLAPOR() const;

	/* LAPOR: Helper for visiting a lock()/unlock() event */
	void visitLockLAPOR(const llvm::GenericValue *addr);
	void visitUnlockLAPOR(const llvm::GenericValue *addr);

	/* SR: Checks whether CANDIDATE is symmetric to THREAD */
	bool isSymmetricToSR(int candidate, int thread, Event parent,
			     llvm::Function *threadFun, const llvm::GenericValue &threadArg) const;

	/* SR: Returns the (greatest) ID of a thread that is symmetric to THREAD */
	int getSymmetricTidSR(int thread, Event parent, llvm::Function *threadFun,
			      const llvm::GenericValue &threadArg) const;

	/* SR: Returns true if TID has the same prefix up to POS.INDEX as POS.THREAD */
	bool sharePrefixSR(int tid, Event pos) const;

	/* SR: Filter stores that will lead to a symmetric execution */
	void filterSymmetricStoresSR(const llvm::GenericValue *addr, llvm::Type *typ,
				     std::vector<Event> &stores) const;

	/* SAVer: Filters stores that will lead to an assume-blocked execution */
	bool filterValuesFromAnnotSAVER(const llvm::GenericValue *addr, llvm::Type *typ,
					const SExpr *annot, std::vector<Event> &stores);


	/*** Output-related ***/

	/* Prints statistics when the verification is over */
	void printResults();

	/* Prints the source-code instructions leading to Event e */
	void printTraceBefore(Event e);

	/* Helper for printTraceBefore() that prints events according to po U rf */
	void recPrintTraceBefore(const Event &e, View &a,
				 llvm::raw_ostream &ss = llvm::outs());

	/* Outputs the full graph.
	 * If getMetadata is set, it outputs more debugging information */
	void printGraph(bool getMetadata = false);

	/* Outputs the graph in a condensed form */
	void prettyPrintGraph();

	/* Outputs the current graph into a file (DOT format),
	 * and visually marks events e and c (conflicting)  */
	void dotPrintToFile(const std::string &filename, Event e, Event c);


	/*** To be overrided by instances of the Driver ***/

	/* Updates lab with model-specific information.
	 * Needs to be called every time a new label is added to the graph */
	virtual void updateLabelViews(EventLabel *lab) = 0;

	/* Checks for races after a load/store is added to the graph.
	 * Should return the racy event, or INIT if no such event exists */
	virtual Event findDataRaceForMemAccess(const MemAccessLabel *mLab) = 0;

	/* Should return the set of stores that it is consistent for current
	 * load to read-from  (excluding atomicity violations) */
	virtual std::vector<Event>
	getStoresToLoc(const llvm::GenericValue *addr) = 0;

	/* Should return the set of reads that lab can revisit */
	virtual std::vector<Event>
	getRevisitLoads(const WriteLabel *lab) = 0;

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

	/* Does some final consistency checks after the fixpoint is over,
	 * and returns the final decision re. consistency */
	virtual bool doFinalConsChecks(bool checkFull = false);

	/* Random generator facilities used */
	using MyRNG  = std::mt19937;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;

	/* The source code of the program under test */
	std::string sourceCode;

	/* User configuration */
	std::unique_ptr<Config> userConf;

	/* Specifications for libraries assumed correct */
	std::vector<Library> grantedLibs;

	/* Specifications for libraries that need to be verified (TODO) */
	std::vector<Library> toVerifyLibs;

	/* The interpreter used by the driver */
	std::unique_ptr<llvm::Interpreter> EE;

	/* The graph managing object */
	std::unique_ptr<ExecutionGraph> execGraph;

	/* The worklist for backtracking. map[stamp->work set] */
	std::map<unsigned int, WorkSet> workqueue;

	/* The revisit sets used during the exploration map[stamp->revisit set] */
	std::map<unsigned int, RevisitSet> revisitSet;

	/* Opt: Whether this execution is moot (locking) */
	bool isMootExecution;

	/* Opt: Which thread(s) the scheduler should prioritize
	 * (empty if none) */
	std::vector<Event> threadPrios;

	/* Number of complete executions explored */
	int explored;

	/* Number of blocked executions explored */
	int exploredBlocked;

	/* Number of duplicate executions explored */
	int duplicates;

	/* Set of (po U rf) unique executions explored */
	std::unordered_set<std::string> uniqueExecs;

	/* Total wall-clock time for the verification */
	clock_t start;

	/* Dbg: Random-number generator for scheduling randomization */
	MyRNG rng;

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s,
					     const DriverErrorKind &o);
};

#endif /* __GENMC_DRIVER_HPP__ */
