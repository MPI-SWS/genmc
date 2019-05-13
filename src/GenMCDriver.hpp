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
#include "Interpreter.h"
#include "ExecutionGraph.hpp"
#include "Library.hpp"
#include <llvm/IR/Module.h>

#include <ctime>
#include <memory>
#include <random>
#include <unordered_set>

class GenMCDriver {

private:
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
	/* Different error types that may occur.
	 * Public to enable the interpreter utilize it */
	enum DriverErrorKind {
		DE_Safety,
		DE_UninitializedMem,
		DE_RaceNotAtomic,
		DE_RaceFreeMalloc,
		DE_FreeNonMalloc,
		DE_AccessNonMalloc,
		DE_AccessFreed,
		DE_DoubleFree,
		DE_InvalidJoin,
	};

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

	/* A store has been interpreted, nothing for the interpreter */
	void
	visitStore(llvm::Interpreter::InstAttr attr, llvm::AtomicOrdering ord,
		   const llvm::GenericValue *addr,
		   llvm::Type *typ, llvm::GenericValue &val);

	/* A lib store has been interpreted, nothing for the interpreter */
	void
	visitLibStore(llvm::Interpreter::InstAttr attr,
		      llvm::AtomicOrdering ord,
		      const llvm::GenericValue *addr, llvm::Type *typ,
		      llvm::GenericValue &val, std::string functionName,
		      bool isInit = false);

	/* A fence has been interpreted, nothing for the interpreter */
	void
	visitFence(llvm::AtomicOrdering ord);

	/* Returns an appropriate result for pthread_self() */
	llvm::GenericValue
	visitThreadSelf(llvm::Type *typ);

	/* Returns the TID of the newly created thread */
	int
	visitThreadCreate(llvm::Function *F, const llvm::ExecutionContext &SF);

	/* Returns an appropriate result for pthread_join() */
	llvm::GenericValue
	visitThreadJoin(llvm::Function *F, const llvm::GenericValue &arg);

	/* A thread has just finished execution, nothing for the interpreter */
	void
	visitThreadFinish();

	/* Returns an appropriate result for malloc() */
	llvm::GenericValue
	visitMalloc(const llvm::GenericValue &size);

	/* A call to free() has been interpreted, nothing for the intepreter */
	void
	visitFree(llvm::GenericValue *ptr);

	/* This method either blocks the offending thread (e.g., if the
	 * execution is invalid), or aborts the exploration */
	void
	visitError(std::string err, Event confEvent,
		   DriverErrorKind t = DE_Safety);

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

	/* Returns a reference to the current graph (can be modified) */
	ExecutionGraph &getGraph() { return execGraph; };

	/* Given a write event from the graph, returns the value it writes */
	llvm::GenericValue getWriteValue(Event w,
					 const llvm::GenericValue *a,
					 const llvm::Type *t);

private:
	/*** Worklist-related ***/

	/* Adds an appropriate entry to the worklist */
	void addToWorklist(StackItemType t, Event e, Event shouldRf,
			   std::vector<std::unique_ptr<EventLabel> > &&prefix,
			   std::vector<std::pair<Event, Event> > &&moPlacings,
			   int newMoPos);

	/* Fetches the next backtrack option.
	 * A default-constructed item means that the list is empty */
	StackItem getNextItem();

	/* Restricts the worklist only to entries with stamps <= st */
	void restrictWorklist(unsigned int st);

	/*** Exploration-related ***/

	/* The workhorse for run().
	 * Exhaustively explores all  consistent executions of a program */
	void visitGraph();

	/* Resets some options before the beginning of a new execution */
	void resetExplorationOptions();

	/* Checks for races when a load or a store is added.
	 * Appropriately calls visitError() and terminates */
	void checkForRaces();

	/* Returns true if the exploration is guided by a graph */
	bool isExecutionDrivenByGraph();

	/* If the execution is guided, returns the corresponding label for
	 * this instruction */
	const EventLabel *getCurrentLabel();

	/* Calculates revisit options and pushes them to the worklist.
	 * Returns true if the current exploration should continue */
	bool calcRevisits(const WriteLabel *lab);
	bool calcLibRevisits(const EventLabel *lab);

	/* Adjusts the graph and the worklist for the next backtracking option.
	 * Returns true if the resulting graph should be explored */
	bool revisitReads(StackItem &s);

	/* Removes all labels with stamp >= st from the graph */
	void restrictGraph(unsigned int st);

	/* Given a list of stores that it is consistent to read-from,
	 * removes options that violate atomicity, and determines the
	 * order in which these options should be explored */
	std::vector<Event> properlyOrderStores(llvm::Interpreter::InstAttr attr,
					       llvm::Type *typ,
					       const llvm::GenericValue *ptr,
					       llvm::GenericValue &expVal,
					       std::vector<Event> &stores);

	/* Opt: Futher reduces the set of available read-from options for a
	 * read that is part of a lock() op. Returns the filtered set of RFs  */
	std::vector<Event> filterAcquiredLocks(const llvm::GenericValue *ptr,
					       const std::vector<Event> &stores,
					       const View &before);

	/* Opt: Tries to in-place revisit a read that is part of a lock.
	 * Returns true if the optimization succeeded */
	bool tryToRevisitLock(const CasReadLabel *rLab, const View &preds,
			      const WriteLabel *sLab, const View &before,
			      const std::vector<Event> &writePrefixPos,
			      const std::vector<std::pair<Event, Event> > &moPlacings);


	/*** Output-related ***/

	/* Prints statistics when the verification is over */
	void printResults();

	/* Prints the source-code instructions leading to Event e */
	void printTraceBefore(Event e);

	/* Nicely outputs the graph at the end of each execution */
	void prettyPrintGraph();

	/* Outputs the current graph into a file (DOT format), and marks
	 * Events e and c */
	void dotPrintToFile(const std::string &filename, Event e, Event c);

	/* Collects metadata for all events leading up to e in buf */
	void calcTraceBefore(const Event &e, View &a, std::stringstream &buf);

	/*** To be overrided by instances of the Driver ***/

	/* Should return the set of stores that it is consistent for current
	 * load to read-from  (excluding atomicity violations) */
	virtual std::vector<Event>
	getStoresToLoc(const llvm::GenericValue *addr) = 0;

	/* Should return the range of available MO places for current store */
	virtual std::pair<int, int>
	getPossibleMOPlaces(const llvm::GenericValue *addr, bool isRMW = false) = 0;

	/* Should return the set of reads that lab can revisit */
	virtual std::vector<Event>
	getRevisitLoads(const WriteLabel *lab) = 0;

	/* Should return the prefix of lab that is not in before,
	 * as well as the placings in MO for all stores in that prefix */
	virtual std::pair<std::vector<std::unique_ptr<EventLabel> >,
			  std::vector<std::pair<Event, Event> > >
	getPrefixToSaveNotBefore(const WriteLabel *lab, View &before) = 0;

	/* Should return true if the current graph is PSC-consistent */
	virtual bool checkPscAcyclicity() = 0;

	/* Should return true if the current graph is consistent */
	virtual bool isExecutionValid() = 0;

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

	/* The execution graph */
	ExecutionGraph execGraph;

	/* The worklist for backtracking. map[stamp->stack item list] */
	std::map<unsigned int, std::vector<StackItem> > workqueue;

	/* Opt: Whether this execution is moot (locking) */
	bool isMootExecution;

	/* Opt: Which thread the scheduler should prioritize
	 * (negative if none) */
	int prioritizeThread;

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
