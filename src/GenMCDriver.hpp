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
#include "ExecutionGraph.hpp"
#include "Interpreter.h"
#include "WorkSet.hpp"
#include "Library.hpp"
#include <llvm/IR/Module.h>

#include <ctime>
#include <memory>
#include <random>
#include <unordered_set>

class GenMCDriver {

public:
	/* Different error types that may occur.
	 * Public to enable the interpreter utilize it */
	enum DriverErrorKind {
		DE_Safety,
		DE_Recovery,
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
		DE_InvalidRecoveryCall,
		DE_InvalidTruncate,
		DE_SystemError,
	};

private:
	static bool isInvalidAccessError(DriverErrorKind e) {
		return DE_InvalidAccessBegin <= e &&
			e <= DE_InvalidAccessEnd;
	};

	/* Enumeration for different types of revisits */
	enum StackItemType {
		SRead,        /* Forward revisit */
		SReadLibFunc, /* Forward revisit (functional lib) */
		SRevisit,     /* Backward revisit */
		MOWrite,      /* Alternative MO position */
		MOWriteLib,   /* Alternative MO position (lib) */
		None /* For default constructor */
	};

	/* Types of worklist items */
	struct StackItem {
		StackItem() : type(None) {};
		StackItem(StackItemType t, Event e, Event shouldRf,
			  std::vector<std::unique_ptr<EventLabel> > &&writePrefix,
			  std::vector<std::pair<Event, Event> > &&moPlacings,
			  int newMoPos)
			: type(t), toRevisit(e), shouldRf(shouldRf),
			  writePrefix(std::move(writePrefix)),
			  moPlacings(std::move(moPlacings)),
			  moPos(newMoPos) {};

		/* Type of the revisit taking place */
		StackItemType type;

		/* Position in the graph of the event to be revisited */
		Event toRevisit;

		/* Where the event revisited should read from.
		 * It is INIT if not applicable for this revisit type */
		Event shouldRf;

		/* The prefix to be restored in the graph */
		std::vector<std::unique_ptr<EventLabel> > writePrefix;

		/* Appropriate positionings for the writes in the prefix
		 * to be restored */
		std::vector<std::pair<Event, Event> > moPlacings;

		/* New position in MO for the event to be revisited
		 * (only if the driver tracks MO) */
		int moPos;
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
	visitLoad(llvm::Interpreter::InstAttr attr, llvm::AtomicOrdering ord,
		  const llvm::GenericValue *addr, llvm::Type *typ,
		  llvm::GenericValue cmpVal = llvm::GenericValue(),
		  llvm::GenericValue rmwVal = llvm::GenericValue(),
		  llvm::AtomicRMWInst::BinOp op =
		  llvm::AtomicRMWInst::BinOp::BAD_BINOP);

	/* Returns the value this load reads, as well as whether
	 * the interpreter should block due to a blocking library read */
	std::pair<llvm::GenericValue, bool>
	visitLibLoad(llvm::Interpreter::InstAttr attr, llvm::AtomicOrdering ord,
		     const llvm::GenericValue *addr, llvm::Type *typ,
		     std::string functionName);

	/* A function modeling a write to disk has been interpreted.
	 * Returns the value read */
	llvm::GenericValue
	visitDskRead(const llvm::GenericValue *readAddr, llvm::Type *typ);

	/* A store has been interpreted, nothing for the interpreter */
	void
	visitStore(llvm::Interpreter::InstAttr attr, llvm::AtomicOrdering ord,
		   const llvm::GenericValue *addr, llvm::Type *typ,
		   const llvm::GenericValue &val);

	/* A lib store has been interpreted, nothing for the interpreter */
	void
	visitLibStore(llvm::Interpreter::InstAttr attr,
		      llvm::AtomicOrdering ord,
		      const llvm::GenericValue *addr, llvm::Type *typ,
		      llvm::GenericValue &val, std::string functionName,
		      bool isInit = false);

	/* A function modeling a write to disk has been interpreted */
	void
	visitDskWrite(const llvm::GenericValue *addr, llvm::Type *typ,
		      const llvm::GenericValue &val, void *mapping,
		      llvm::Interpreter::InstAttr attr =
		      llvm::Interpreter::InstAttr::IA_None,
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

	/* A fence has been interpreted, nothing for the interpreter */
	void
	visitFence(llvm::AtomicOrdering ord);

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
	void
	visitFree(const llvm::AllocaHolder::Allocas &ptrs); /* Helper for bulk-deallocs */

	/* This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void
	visitError(DriverErrorKind t, const std::string &err = std::string(),
		   Event confEvent = Event::getInitializer());

	virtual ~GenMCDriver() {};

protected:

	GenMCDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
		    std::vector<Library> &granted, std::vector<Library> &toVerify,
		    clock_t start);

	/* No copying or copy-assignment of this class is allowed */
	GenMCDriver(GenMCDriver const&) = delete;
	GenMCDriver &operator=(GenMCDriver const &) = delete;

	/* Returns a pointer to the user configuration */
	const Config *getConf() const { return userConf.get(); }

	/* Returns a pointer to the interpreter */
	llvm::Interpreter *getEE() const { return EE; }

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

	/* Returns true if we should check consistency at p */
	bool shouldCheckCons(ProgramPoint p);

	/* Returns true if full consistency needs to be checked at p.
	 * Assumes that consistency needs to be checked anyway. */
	bool shouldCheckFullCons(ProgramPoint p);

	/* Returns true if we should check persistency at p */
	bool shouldCheckPers(ProgramPoint p);

	/* Returns true if a is hb-before b */
	bool isHbBefore(Event a, Event b, ProgramPoint p = ProgramPoint::step);

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

	/* Performs POSIX checks whenever an unlock event is added.
	 * Appropriately calls visitError() and terminates */
	void checkUnlockValidity();

	/* Checks whether there is some race when allocating/deallocating
	 * memory and reports an error as necessary.
	 * Helpers for checkForMemoryRaces() */
	void findMemoryRaceForMemAccess(const MemAccessLabel *mLab);
	void findMemoryRaceForAllocAccess(const FreeLabel *fLab);

	/* Checks for memory races (e.g., double free, access freed memory, etc)
	 * whenever a read/write/free is added.
	 * Appropriately calls visitError() and terminates */
	void checkForMemoryRaces(const void *addr);

	/* Calls visitError() if rLab is reading from an uninitialized
	 * (dynamically allocated) memory location */
	void checkForUninitializedMem(const ReadLabel *rLab);

	/* Returns true if the exploration is guided by a graph */
	bool isExecutionDrivenByGraph();

	/* Pers: Returns true if we are currently running the recovery routine */
	bool inRecoveryMode() const;

	/* If the execution is guided, returns the corresponding label for
	 * this instruction. Reports an error if the execution is not guided */
	const EventLabel *getCurrentLabel() const;

	/* Returns true if the current graph is consistent */
	bool isConsistent(ProgramPoint p);

	/* Pers: Returns true if current recovery routine is valid */
	bool isRecoveryValid(ProgramPoint p);

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
	std::vector<Event> properlyOrderStores(llvm::Interpreter::InstAttr attr,
					       llvm::Type *typ,
					       const llvm::GenericValue *ptr,
					       llvm::GenericValue &expVal,
					       std::vector<Event> &stores);

	/* Helper for visitLoad() that creates a ReadLabel and adds it to the graph */
	const ReadLabel *
	createAddReadLabel(llvm::Interpreter::InstAttr attr,
			   llvm::AtomicOrdering ord,
			   const llvm::GenericValue *addr,
			   llvm::Type *typ,
			   const llvm::GenericValue &cmpVal,
			   const llvm::GenericValue &rmwVal,
			   llvm::AtomicRMWInst::BinOp op,
			   Event store);

	/* Removes rfs from "rfs" until a consistent option for rLab is found,
	 * if that is dictated by the CLI options */
	bool ensureConsistentRf(const ReadLabel *rLab, std::vector<Event> &rfs);

	/* Helper for visitStore() that creates a WriteLabel and adds it to the graph */
	const WriteLabel *
	createAddStoreLabel(llvm::Interpreter::InstAttr attr,
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
	void repairLock(Event lock);

	/* Opt: Repairs some locks that may be "dangling", as part of the
	 * in-place revisiting of locks */
	bool repairDanglingLocks();

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


	/*** Output-related ***/

	/* Prints statistics when the verification is over */
	void printResults();

	/* Prints the source-code instructions leading to Event e */
	void printTraceBefore(Event e);

	/* Helper for printTraceBefore() that prints events according to po U rf */
	void recPrintTraceBefore(const Event &e, View &a,
				 llvm::raw_ostream &ss = llvm::dbgs());

	/* Outputs the full graph.
	 * If getMetadata is set, it outputs more debugging information */
	void printGraph(bool getMetadata = false);

	/* Outputs the graph in a condensed form */
	void prettyPrintGraph();

	/* Outputs the current graph into a file (DOT format),
	 * and visually marks events e and c (conflicting)  */
	void dotPrintToFile(const std::string &filename, Event e, Event c);


	/*** To be overrided by instances of the Driver ***/

	/* Creates a label for a plain read to be added to the graph */
	virtual std::unique_ptr<ReadLabel>
	createReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			const llvm::GenericValue *ptr, const llvm::Type *typ,
			Event rf) = 0;

	/* Creates a label for a FAI read to be added to the graph */
	virtual std::unique_ptr<FaiReadLabel>
	createFaiReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			   const llvm::GenericValue *ptr, const llvm::Type *typ,
			   Event rf, llvm::AtomicRMWInst::BinOp op,
			   const llvm::GenericValue &opValue) = 0;

	/* Creates a label for a CAS read to be added to the graph */
	virtual std::unique_ptr<CasReadLabel>
	createCasReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			   const llvm::GenericValue *ptr, const llvm::Type *typ,
			   Event rf, const llvm::GenericValue &expected,
			   const llvm::GenericValue &swap,
			   bool isLock = false) = 0;

	/* Creates a label for a library read to be added to the graph */
	virtual std::unique_ptr<LibReadLabel>
	createLibReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			   const llvm::GenericValue *ptr, const llvm::Type *typ,
			   Event rf, std::string functionName) = 0 ;

	/* Creates a label for a disk read to be added to the graph */
	virtual std::unique_ptr<DskReadLabel>
	createDskReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			   const llvm::GenericValue *addr, const llvm::Type *typ,
			   Event rf) = 0;

	/* Creates a label for a plain write to be added to the graph */
	virtual std::unique_ptr<WriteLabel>
	createStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			 const llvm::GenericValue *ptr, const llvm::Type *typ,
			 const llvm::GenericValue &val, bool isUnlock = false) = 0;

	/* Creates a label for a FAI write to be added to the graph */
	virtual std::unique_ptr<FaiWriteLabel>
	createFaiStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    const llvm::GenericValue &val) = 0;

	/* Creates a label for a CAS write to be added to the graph */
	virtual std::unique_ptr<CasWriteLabel>
	createCasStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    const llvm::GenericValue &val, bool isLock = false) = 0;

	/* Creates a label for a library write to be added to the graph */
	virtual std::unique_ptr<LibWriteLabel>
	createLibStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    llvm::GenericValue &val, std::string functionName,
			    bool isInit) = 0;

	/* Creates a label for a disk write to be added to the graph */
	virtual std::unique_ptr<DskWriteLabel>
	createDskWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    const llvm::GenericValue &val, void *mapping) = 0;

	virtual std::unique_ptr<DskMdWriteLabel>
	createDskMdWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
			      const llvm::GenericValue *ptr, const llvm::Type *typ,
			      const llvm::GenericValue &val, void *mapping,
			      std::pair<void *, void *> ordDataRange) = 0;

	virtual std::unique_ptr<DskDirWriteLabel>
	createDskDirWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
			       const llvm::GenericValue *ptr, const llvm::Type *typ,
			       const llvm::GenericValue &val, void *mapping) = 0;

	virtual std::unique_ptr<DskJnlWriteLabel>
	createDskJnlWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
			       const llvm::GenericValue *ptr, const llvm::Type *typ,
			       const llvm::GenericValue &val, void *mapping, void *transInode) = 0;

	/* Creates a label for a fence to be added to the graph */
	virtual std::unique_ptr<FenceLabel>
	createFenceLabel(int tid, int index, llvm::AtomicOrdering ord) = 0;


	/* Creates a label for a malloc event to be added to the graph */
	virtual std::unique_ptr<MallocLabel>
	createMallocLabel(int tid, int index, const void *addr,
			  unsigned int size, Storage s, AddressSpace spc) = 0;

	/* Creates a label for a free event to be added to the graph */
	virtual std::unique_ptr<FreeLabel>
	createFreeLabel(int tid, int index, const void *addr) = 0;

	/* Creates a label for a disk open event to be added to the graph */
	virtual std::unique_ptr<DskOpenLabel>
	createDskOpenLabel(int tid, int index, const char *fileName,
			   const llvm::GenericValue &fd) = 0;

	/* Creates a label for an fsync() event to be added to the graph */
	virtual std::unique_ptr<DskFsyncLabel>
	createDskFsyncLabel(int tid, int index, const void *inode,
			    unsigned int size) = 0;

	/* Creates a label for a sync() event to be added to the graph */
	virtual std::unique_ptr<DskSyncLabel>
	createDskSyncLabel(int tid, int index) = 0;

	/* Creates a label for a persistency barrier
	 * (__VERIFIER_pbarrier()) to be added to the graph */
	virtual std::unique_ptr<DskPbarrierLabel>
	createDskPbarrierLabel(int tid, int index) = 0;

	/* Creates a label for the creation of a thread to be added to the graph */
	virtual std::unique_ptr<ThreadCreateLabel>
	createTCreateLabel(int tid, int index, int cid) = 0;

	/* Creates a label for the join of a thread to be added to the graph */
	virtual std::unique_ptr<ThreadJoinLabel>
	createTJoinLabel(int tid, int index, int cid) = 0;

	/* Creates a label for the start of a thread to be added to the graph */
	virtual std::unique_ptr<ThreadStartLabel>
	createStartLabel(int tid, int index, Event tc, int symm = -1) = 0;

	/* Creates a label for the end of a thread to be added to the graph */
	virtual std::unique_ptr<ThreadFinishLabel>
	createFinishLabel(int tid, int index) = 0;

	/* LAPOR: Creates a (dummy) label for a lock() operation */
	virtual std::unique_ptr<LockLabelLAPOR>
	createLockLabelLAPOR(int tid, int index, const llvm::GenericValue *addr) = 0;

	/* LAPOR: Creates a (dummy) label for an unlock() operation */
	virtual std::unique_ptr<UnlockLabelLAPOR>
	createUnlockLabelLAPOR(int tid, int index, const llvm::GenericValue *addr) = 0;

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

	/* The LLVM module for this program */
	std::unique_ptr<llvm::Module> mod;

	/* Specifications for libraries assumed correct */
	std::vector<Library> grantedLibs;

	/* Specifications for libraries that need to be verified (TODO) */
	std::vector<Library> toVerifyLibs;

	/* The interpreter used by the driver */
	llvm::Interpreter *EE;

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
