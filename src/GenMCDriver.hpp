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
#include <random>
#include <unordered_set>

enum StackItemType { SRead, SRevisit, MOWrite, MOWriteLib, SReadLibFunc, None };

struct StackItem {
	StackItemType type;
	Event toRevisit;
	Event oldRf;
	Event shouldRf;
	std::vector<EventLabel> writePrefix;
	std::vector<std::pair<Event, Event> > moPlacings;
	int moPos;

	StackItem() : type(None) {};
	StackItem(StackItemType t, Event e, Event oldRf, Event shouldRf,
		  std::vector<EventLabel> &&writePrefix,
		  std::vector<std::pair<Event, Event> > &&moPlacings,
		  int newMoPos)
		: type(t), toRevisit(e), oldRf(oldRf), shouldRf(shouldRf),
		  writePrefix(writePrefix), moPlacings(moPlacings), moPos(newMoPos) {};
};

class GenMCDriver {

protected:
	using MyRNG  = std::mt19937;
	using MyDist = std::uniform_int_distribution<MyRNG::result_type>;

	std::string sourceCode;
	std::unique_ptr<Config> userConf;
	std::unique_ptr<llvm::Module> mod;
	std::vector<Library> grantedLibs;
	std::vector<Library> toVerifyLibs;
	llvm::Interpreter *EE;
	ExecutionGraph execGraph;
	std::map<unsigned int, std::vector<StackItem> > workqueue;
	bool isMootExecution;
	int prioritizeThread;
	int explored;
	int exploredBlocked;
	int duplicates;
	std::unordered_set<std::string> uniqueExecs;
	MyRNG rng;
	clock_t start;

	GenMCDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
		    std::vector<Library> &granted, std::vector<Library> &toVerify,
		    clock_t start);

	/* No copying or copy-assignment of this class is allowed */
	GenMCDriver(GenMCDriver const&) = delete;
	GenMCDriver &operator=(GenMCDriver const &) = delete;

public:

	virtual ~GenMCDriver() {};

	void addToWorklist(StackItemType t, Event e, Event shouldRf,
			   std::vector<EventLabel> &&prefix,
			   std::vector<std::pair<Event, Event> > &&moPlacings,
			   int newMoPos);
	StackItem getNextItem();
	void restrictWorklist(unsigned int stamp);
	void restrictGraph(unsigned int stamp);

	std::vector<Library> &getGrantedLibs()  { return grantedLibs; };
	std::vector<Library> &getToVerifyLibs() { return toVerifyLibs; };
	ExecutionGraph &getGraph() { return execGraph; };

	void run();
	void printResults();

	void resetExplorationOptions();
	void handleExecutionBeginning();
	void handleExecutionInProgress();
	void handleFinishedExecution();

	/* Scheduling methods */
	bool scheduleNext();

	void visitGraph();
	Event checkForRaces();
	bool isExecutionDrivenByGraph();
	EventLabel& getCurrentLabel();

	llvm::GenericValue visitThreadSelf(llvm::Type *typ);
	int visitThreadCreate(llvm::Function *F, const llvm::ExecutionContext &SF);
	llvm::GenericValue visitThreadJoin(llvm::Function *F, const llvm::GenericValue &arg);
	void visitThreadFinish();
	void visitFence(llvm::AtomicOrdering ord);
	llvm::GenericValue visitLoad(EventAttr attr, llvm::AtomicOrdering ord,
				     llvm::GenericValue *addr, llvm::Type *typ,
				     llvm::GenericValue &&cmpVal = llvm::GenericValue(),
				     llvm::GenericValue &&rmwVal = llvm::GenericValue(),
				     llvm::AtomicRMWInst::BinOp op = llvm::AtomicRMWInst::BinOp::BAD_BINOP);
	void visitStore(EventAttr attr, llvm::AtomicOrdering ord, llvm::GenericValue *addr,
			llvm::Type *typ, llvm::GenericValue &val);
	llvm::GenericValue visitMalloc(const llvm::GenericValue &size);
	void visitFree(llvm::GenericValue *ptr);
	void visitError(std::string &err);

	bool calcRevisits(EventLabel &lab);
	llvm::GenericValue visitLibLoad(EventAttr attr, llvm::AtomicOrdering ord,
					llvm::GenericValue *addr, llvm::Type *typ,
					std::string functionName);
	void visitLibStore(EventAttr attr, llvm::AtomicOrdering ord,
			   llvm::GenericValue *addr, llvm::Type *typ, llvm::GenericValue &val,
			   std::string functionName, bool isInit = false);
	bool calcLibRevisits(EventLabel &lab);
	bool revisitReads(StackItem &s);

	bool tryToRevisitLock(EventLabel &rLab, View &preds, EventLabel &sLab, View &before,
			      std::vector<Event> &writePrefixPos,
			      std::vector<std::pair<Event, Event> > &moPlacings);
	std::vector<Event> filterAcquiredLocks(llvm::GenericValue *ptr,
					       std::vector<Event> &stores, View &before);
	std::vector<Event> properlyOrderStores(EventAttr attr, llvm::Type *typ, llvm::GenericValue *ptr,
					       llvm::GenericValue &expVal, std::vector<Event> &stores);

	/* Outputting facilities */
	void printTraceBefore(Event e);
	void prettyPrintGraph();
	void dotPrintToFile(std::string &filename, View &before, Event e);
	void calcTraceBefore(const Event &e, View &a, std::stringstream &buf);

	virtual std::vector<Event> getStoresToLoc(llvm::GenericValue *addr) = 0;
	virtual std::pair<int, int> getPossibleMOPlaces(llvm::GenericValue *addr, bool isRMW = false) = 0;
	virtual std::vector<Event> getRevisitLoads(EventLabel &lab) = 0;
	virtual std::pair<std::vector<EventLabel>, std::vector<std::pair<Event, Event> > >
			  getPrefixToSaveNotBefore(EventLabel &lab, View &before) = 0;

	virtual bool checkPscAcyclicity() = 0;
	virtual bool isExecutionValid() = 0;
};

#endif /* __GENMC_DRIVER_HPP__ */
