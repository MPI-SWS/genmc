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

#include "Config.hpp"
#include "Error.hpp"
#include "LLVMModule.hpp"
#include "GenMCDriver.hpp"
#include "Interpreter.h"
#include "Parser.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Format.h>

#include <algorithm>
#include <csignal>
#include <sstream>

/************************************************************
 ** GENERIC MODEL CHECKING DRIVER
 ***********************************************************/

void abortHandler(int signum)
{
	exit(42);
}

GenMCDriver::GenMCDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
			 std::vector<Library> &granted, std::vector<Library> &toVerify,
			 clock_t start)
	: userConf(std::move(conf)), mod(std::move(mod)),
	  grantedLibs(granted), toVerifyLibs(toVerify),
	  execGraph(), isMootExecution(false), prioritizeThread(-1),
	  explored(0), exploredBlocked(0), duplicates(0), start(start)
{
	/* Register a signal handler for abort() */
	std::signal(SIGABRT, abortHandler);

	/* Set up a random-number generator (for the scheduler) */
	std::random_device rd;
	auto seedVal = (userConf->randomizeScheduleSeed != "") ?
		(MyRNG::result_type) stoull(userConf->randomizeScheduleSeed) : rd();
	if (userConf->printRandomizeScheduleSeed)
		llvm::dbgs() << "Seed: " << seedVal << "\n";
	rng.seed(seedVal);

	/*
	 * Make sure we can resolve symbols in the program as well. We use 0
	 * as an argument in order to load the program, not a library. This
	 * is useful as it allows the executions of external functions in the
	 * user code.
	 */
	std::string ErrorStr;
	if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(0, &ErrorStr))
		WARN("Could not resolve symbols in the program: " + ErrorStr);
}

void GenMCDriver::printResults()
{
	std::stringstream dups;
	dups << " (" << duplicates << " duplicates)";
	llvm::dbgs() << "Number of complete executions explored: " << explored
		     << ((userConf->countDuplicateExecs) ? dups.str() : "") << "\n";
	if (exploredBlocked) {
		llvm::dbgs() << "Number of blocked executions seen: " << exploredBlocked
			     << "\n";
	}
	llvm::dbgs() << "Total wall-clock time: "
		     << llvm::format("%.2f", ((float) clock() - start)/CLOCKS_PER_SEC)
		     << "s\n";
}

void GenMCDriver::resetExplorationOptions()
{
	isMootExecution = false;
	prioritizeThread = -1;
}

void GenMCDriver::handleExecutionBeginning()
{
	auto &g = getGraph();

	/* Set-up (optimize) the interpreter for the new exploration */
	for (auto i = 1u; i < g.events.size(); i++) {

		/* Skip not-yet-created threads */
		BUG_ON(g.events[i].empty());
		auto &labFst = g.events[i][0];
		if (labFst.rf.index >= (int) g.events[labFst.rf.thread].size())
			continue;

		/* This could fire for main() (?) */
		BUG_ON(!g.getEventLabel(labFst.rf).isCreate());

		/* Skip finished threads */
		auto &labLast = g.getLastThreadLabel(i);
		if (labLast.isFinish())
			continue;

		/* Initialize ECStacks in interpreter */
		auto &thr = EE->getThrById(i);
		BUG_ON(!thr.ECStack.empty() || thr.isBlocked);
		thr.ECStack.push_back(thr.initSF);

		/* Mark threads that are blocked appropriately */
		if (labLast.isRead() && labLast.isLock())
			thr.block();
	}
}

void GenMCDriver::handleExecutionInProgress()
{
	/* Check if there are checks to be done while running */
	if (userConf->validateExecGraphs)
		getGraph().validate();
	return;
}

void GenMCDriver::handleFinishedExecution()
{
	/* First, reset all exploration options */
	resetExplorationOptions();

	/* Ignore the execution if some assume has failed */
	if (std::any_of(EE->threads.begin(), EE->threads.end(),
			[](llvm::Thread &thr){ return thr.isBlocked; })) {
		++exploredBlocked;
		return;
	}

	auto &g = getGraph();
	if (!checkPscAcyclicity())
		return;
	if (userConf->checkWbAcyclicity && !g.isWbAcyclic())
		return;
	if (userConf->printExecGraphs)
		llvm::dbgs() << g << g.modOrder << "\n";
	if (userConf->prettyPrintExecGraphs)
		prettyPrintGraph();
	if (userConf->countDuplicateExecs) {
		std::string exec;
		llvm::raw_string_ostream buf(exec);
		buf << g;
		if (uniqueExecs.find(buf.str()) != uniqueExecs.end())
			++duplicates;
		else
			uniqueExecs.insert(buf.str());
	}
	++explored;
}

void GenMCDriver::run()
{
	std::string buf;

	LLVMModule::transformLLVMModule(*mod, userConf->spinAssume, userConf->unroll);
	if (userConf->transformFile != "")
		LLVMModule::printLLVMModule(*mod, userConf->transformFile);

	/* Create an interpreter for the program's instructions. */
	EE = (llvm::Interpreter *) llvm::Interpreter::create(&*mod, this, &buf);

	/* Create main thread and start event */
	auto mainFun = mod->getFunction(userConf->programEntryFun);
	if (!mainFun) {
		WARN("ERROR: Could not find program's entry point function!\n");
		abort();
	}
	auto main = llvm::Thread(mainFun, 0);
	EE->threads.push_back(main);

	/* Explore all graphs and print the results */
	visitGraph();
	printResults();
	return;
}

void GenMCDriver::addToWorklist(StackItemType t, Event e, Event shouldRf,
			       std::vector<EventLabel> &&prefix,
			       std::vector<std::pair<Event, Event> > &&moPlacings,
			       int newMoPos = 0)

{
	auto &lab = getGraph().getEventLabel(e);
	StackItem s(t, e, lab.rf, shouldRf,
		    std::move(prefix), std::move(moPlacings), newMoPos);

	workqueue[lab.getStamp()].push_back(std::move(s));
}

StackItem GenMCDriver::getNextItem()
{
	for (auto rit = workqueue.rbegin(); rit != workqueue.rend(); ++rit) {
		if (rit->second.empty())
			continue;

		auto si = rit->second.back();
		rit->second.pop_back();
		return si;
	}
	return StackItem();
}

void GenMCDriver::restrictWorklist(unsigned int stamp)
{
	std::vector<int> idxsToRemove;
	for (auto rit = workqueue.rbegin(); rit != workqueue.rend(); ++rit)
		if (rit->first > stamp && rit->second.empty())
			idxsToRemove.push_back(rit->first); // TODO: break out of loop?

	for (auto &i : idxsToRemove)
		workqueue.erase(i);
}

void GenMCDriver::restrictGraph(unsigned int stamp)
{
	auto &g = getGraph();
	auto v = g.getViewFromStamp(stamp);

	/* First, free memory allocated by events that will no longer be in the graph */
	for (auto i = 0u; i < g.events.size(); i++) {
		for (auto j = v[i] + 1u; j < g.events[i].size(); j++) {
			auto &lab = g.events[i][j];
			if (lab.isMalloc())
				EE->freeRegion(lab.getAddr(), lab.val.IntVal.getLimitedValue());
		}
	}

	/* Then, restrict the graph */
	g.cutToView(v);
	return;
}

/************************************************************
 ** Scheduling methods
 ***********************************************************/

bool GenMCDriver::scheduleNext()
{
	if (isMootExecution)
		return false;

	auto &g = getGraph();

	/* First, check if we should prioritize some thread */
	if (0 <= prioritizeThread && prioritizeThread < (int) g.events.size()) {
		auto &thr = EE->getThrById(prioritizeThread);
		if (!thr.ECStack.empty() && !thr.isBlocked &&
		    !g.getLastThreadLabel(prioritizeThread).isFinish()) {
			EE->currentThread = prioritizeThread;
			return true;
		}
	}
	prioritizeThread = -2;


	/* Check if randomize scheduling is enabled and schedule some thread */
	MyDist dist(0, g.events.size());
	auto random = (userConf->randomizeSchedule) ? dist(rng) : 0;
	for (auto j = 0u; j < g.events.size(); j++) {
		auto i = (j + random) % g.events.size();
		auto &thr = EE->getThrById(i);

		if (thr.ECStack.empty() || thr.isBlocked)
			continue;

		if (g.getLastThreadLabel(i).isFinish()) {
			thr.ECStack.clear();
			continue;
		}

		/* Found a not-yet-complete thread; schedule it */
		EE->currentThread = i;
		return true;
	}

	/* No schedulable thread found */
	return false;
}

void GenMCDriver::visitGraph()
{
	while (true) {
		EE->reset();

		/* Get main program function and run the program */
		EE->runStaticConstructorsDestructors(false);
		EE->runFunctionAsMain(mod->getFunction(userConf->programEntryFun), {"prog"}, 0);
		EE->runStaticConstructorsDestructors(true);

		auto validExecution = true;
		do {
			/*
			 * revisitReads() might deem some execution unfeasible,
			 * so we have to reset all exploration options before
			 * calling it again
			 */
			resetExplorationOptions();

			auto p = getNextItem();
			if (p.type == None) {
				EE->reset();  /* To free memory */
				return;
			}
			validExecution = revisitReads(p);
		} while (!validExecution);
	}
}

bool GenMCDriver::isExecutionDrivenByGraph()
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	return ++thr.globalInstructions < g.events[thr.id].size();
}

EventLabel& GenMCDriver::getCurrentLabel()
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	BUG_ON(thr.globalInstructions >= g.events[thr.id].size());
	return g.events[thr.id][thr.globalInstructions];
}

/*
 * This function is called to check for races when a new event is added.
 * When a race is detected, we have to actually ensure that the execution is valid,
 * in the sense that it is consistent. Thus, this method relies on overrided
 * virtual methods to check for consistency, depending on the operating mode
 * of the driver.
 */
Event GenMCDriver::checkForRaces()
{
	auto &g = getGraph();
	auto &lab = g.getLastThreadLabel(EE->getCurThr().id);

	/* We only check for races when reads and writes are added */
	BUG_ON(!(lab.isRead() || lab.isWrite()));

	auto racy = Event::getInitializer();
	if (lab.isRead())
		racy = g.findRaceForNewLoad(lab.getPos());
	else
		racy = g.findRaceForNewStore(lab.getPos());

	/* If a race is found and the execution is consistent, return it */
	if (!racy.isInitializer() && isExecutionValid())
		return racy;

	/* Else, if this is a heap allocation, also look for memory errors */
	if (!EE->isHeapAlloca(lab.getAddr()))
		return Event::getInitializer();

	auto before = g.getHbBefore(lab.getPos().prev());
	for (auto i = 0u; i < g.events.size(); i++)
		for (auto j = 0u; j < g.events[i].size(); j++) {
			auto &oLab = g.events[i][j];
			if (oLab.isFree() && oLab.getAddr() == lab.getAddr())
				return oLab.getPos();
			if (oLab.isMalloc() && oLab.getAddr() <= lab.getAddr() &&
			    lab.getAddr() < oLab.val.PointerVal &&
			    oLab.getIndex() > before[oLab.getThread()])
				return oLab.getPos();
	}
	return Event::getInitializer();
}

std::vector<Event> GenMCDriver::filterAcquiredLocks(llvm::GenericValue *ptr,
						    std::vector<Event> &stores,
						    View &before)
{
	auto &g = getGraph();
	std::vector<Event> valid, conflicting;

	for (auto &s : stores) {
		if (g.getEventLabel(s).isLock() || g.isStoreReadBySettledRMW(s, ptr, before))
			continue;

		if (g.isStoreReadByExclusiveRead(s, ptr))
			conflicting.push_back(s);
		else
			valid.push_back(s);
	}

	if (valid.empty()) {
		auto lit = std::find_if(stores.begin(), stores.end(),
					[&](Event &s){ return g.getEventLabel(s).isLock(); });
		BUG_ON(lit == stores.end());
		prioritizeThread = lit->thread;
		valid.push_back(*lit);
	}
	valid.insert(valid.end(), conflicting.begin(), conflicting.end());
	return valid;
}

std::vector<Event> GenMCDriver::properlyOrderStores(EventAttr attr, llvm::Type *typ, llvm::GenericValue *ptr,
						    llvm::GenericValue &expVal, std::vector<Event> &stores)
{
	if (attr == ATTR_PLAIN || attr == ATTR_UNLOCK)
		return stores;

	auto &g = getGraph();
	auto before = g.getPorfBefore(g.getLastThreadEvent(EE->getCurThr().id));

	if (attr == ATTR_LOCK)
		return filterAcquiredLocks(ptr, stores, before);

	std::vector<Event> valid, conflicting;
	for (auto &s : stores) {
		auto oldVal = EE->loadValueFromWrite(s, typ, ptr);
		if ((attr == ATTR_FAI || EE->compareValues(typ, oldVal, expVal)) &&
		    g.isStoreReadBySettledRMW(s, ptr, before))
			continue;

		if (g.isStoreReadByExclusiveRead(s, ptr))
			conflicting.push_back(s);
		else
			valid.push_back(s);
	}
	valid.insert(valid.end(), conflicting.begin(), conflicting.end());
	return valid;
}

llvm::GenericValue GenMCDriver::visitThreadSelf(llvm::Type *typ)
{
	llvm::GenericValue result;

	result.IntVal = llvm::APInt(typ->getIntegerBitWidth(), EE->getCurThr().id);
	return result;
}

int GenMCDriver::visitThreadCreate(llvm::Function *calledFun, const llvm::ExecutionContext &SF)
{
	auto &g = getGraph();
	auto &curThr = EE->getCurThr();
	int tid = 0;

	if (!isExecutionDrivenByGraph()) {
		Event cur(curThr.id, g.events[curThr.id].size());

		/* First, check if the thread to be created already exists */
		while (tid < (int) g.events.size()) {
			if (!g.events[tid].empty() && g.events[tid][0].rf == cur)
				break;
			++tid;
		}

		/* Add an event for the thread creation */
		g.addTCreateToGraph(curThr.id, tid);

		/* Prepare the execution context for the new thread */
		llvm::Thread thr(calledFun, tid, curThr.id, SF);
		thr.ECStack.push_back(SF);
		thr.tls = EE->threadLocalVars;

		if (tid == (int) g.events.size()) {
			/* If the thread does not exist in the graph, make an entry for it */
			EE->threads.push_back(thr);
			g.events.push_back({});
			g.addStartToGraph(tid, g.getLastThreadEvent(EE->getCurThr().id)); // need to refetch ref
		} else {
			/* Otherwise, just push the execution context to the interpreter */
			EE->threads[tid] = thr;
		}
	} else {
		tid = getCurrentLabel().cid;
	}
	return tid;
}

llvm::GenericValue GenMCDriver::visitThreadJoin(llvm::Function *F, const llvm::GenericValue &arg)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	int tid = arg.IntVal.getLimitedValue(std::numeric_limits<int>::max());
	if (tid < 0 || int (EE->threads.size()) <= tid || tid == thr.id) {
		std::stringstream ss;
		ss << "ERROR: Invalid TID in pthread_join(): " << tid;
		if (tid == thr.id)
			ss << " (TID cannot be the same as the calling thread)\n";
		WARN(ss.str());
		abort();
	}

	if (!isExecutionDrivenByGraph()) {
		/* Add a relevant event to the graph */
		g.addTJoinToGraph(thr.id, tid);
	}

	auto &cLab = g.getLastThreadLabel(tid);
	if (!cLab.isFinish()) {
		/* This thread will remain blocked until the respective child terminates */
		thr.block();
	} else {
		auto &lab = getCurrentLabel();
		lab.rf = cLab.pos;
		cLab.rfm1.push_back(lab.pos);
		lab.hbView.updateMax(cLab.msgView);
		lab.porfView.updateMax(cLab.porfView);
	}

	/* Return a value indicating that pthread_join() succeeded */
	llvm::GenericValue result;
	result.IntVal = llvm::APInt(F->getReturnType()->getIntegerBitWidth(), 0);
	return result;
}

void GenMCDriver::visitThreadFinish()
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (!isExecutionDrivenByGraph() && /* Make sure that there is not a failed assume... */
	    !thr.isBlocked) {
		g.addFinishToGraph(thr.id);

		if (thr.id == 0)
			return;

		auto &lab = g.getLastThreadLabel(thr.id);
		for (auto i = 0u; i < g.events.size(); i++) {
			EventLabel &pLab = g.getLastThreadLabel(i);
			if (pLab.isJoin() && pLab.cid == thr.id) {
				/* If parent thread is waiting for me, relieve it */
				EE->getThrById(i).unblock();
				/* Update relevant join event */
				pLab.rf = lab.pos;
				lab.rfm1.push_back(pLab.pos);
				pLab.hbView.updateMax(lab.msgView);
				pLab.porfView.updateMax(lab.porfView);
			}
		}
	} /* FIXME: Maybe move view update into thread finish creation? */
	  /* FIXME: Thread return values? */
}

void GenMCDriver::visitFence(llvm::AtomicOrdering ord)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph())
		return;

	g.addFenceToGraph(thr.id, ord);
	return;
}

llvm::GenericValue GenMCDriver::visitLoad(EventAttr attr, llvm::AtomicOrdering ord,
					 llvm::GenericValue *addr, llvm::Type *typ,
					 llvm::GenericValue &&cmpVal,
					 llvm::GenericValue &&rmwVal,
					 llvm::AtomicRMWInst::BinOp op)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph()) {
		auto &lab = getCurrentLabel();
		auto val = EE->loadValueFromWrite(lab.rf, typ, addr);
		return val;
	}

	/* Get all stores to this location from which we can read from */
	auto stores = getStoresToLoc(addr);
	auto validStores = properlyOrderStores(attr, typ, addr, cmpVal, stores);

	/* ... and add a label with the appropriate store. */
	auto &lab = g.addReadToGraph(thr.id, attr, ord, addr, typ, validStores[0],
				     std::move(cmpVal), std::move(rmwVal), op);

	/* Check for races */
	if (!checkForRaces().isInitializer()) {
		llvm::dbgs() << "Race detected!\n";
		printResults();
		abort();
	}

	/* Push all the other alternatives choices to the Stack */
	for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
		addToWorklist(SRead, lab.getPos(), *it, {}, {});
	return EE->loadValueFromWrite(validStores[0], typ, addr);
}

void GenMCDriver::visitStore(EventAttr attr, llvm::AtomicOrdering ord,
			    llvm::GenericValue *addr, llvm::Type *typ,
			    llvm::GenericValue &val)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph())
		return;

	auto placesRange = getPossibleMOPlaces(addr, attr != ATTR_PLAIN && attr != ATTR_UNLOCK);
	auto &begO = placesRange.first;
	auto &endO = placesRange.second;

	/* It is always consistent to add the store at the end of MO */
	auto &sLab = g.addStoreToGraph(thr.id, attr, ord, addr, typ, val, endO);

	/* Check for races */
	if (!checkForRaces().isInitializer()) {
		llvm::dbgs() << "Race detected!\n";
		printResults();
		abort();
	}

	auto &locMO = g.modOrder[addr];
	for (auto it = locMO.begin() + begO; it != locMO.begin() + endO; ++it) {

		/* We cannot place the write just before the write of an RMW */
		if (g.getEventLabel(*it).isRMW())
			continue;

		/* Push the stack item */
		addToWorklist(MOWrite, sLab.getPos(), sLab.rf, {}, {},
			      std::distance(locMO.begin(), it));
	}

	calcRevisits(sLab);
	return;
}

llvm::GenericValue GenMCDriver::visitMalloc(const llvm::GenericValue &argSize)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();
	llvm::GenericValue allocBegin, allocEnd;

	if (isExecutionDrivenByGraph()) {
		auto &lab = getCurrentLabel();
		allocBegin.PointerVal = lab.getAddr();
		return allocBegin;
	}

	WARN_ON_ONCE(argSize.IntVal.getBitWidth() > 64, "malloc-alignment",
		     "WARNING: malloc()'s alignment larger than 64-bit! Limiting...\n");

	/* Get a fresh address and also store the size of this allocation */
	allocBegin.PointerVal = EE->heapAllocas.empty() ? (char *) EE->globalVars.back() + 1
		                                        : (char *) EE->heapAllocas.back() + 1;
	uint64_t size = argSize.IntVal.getLimitedValue();
	allocEnd.PointerVal = (char *) allocBegin.PointerVal + size;

	/* Track the address allocated*/
	char *ptr = static_cast<char *>(allocBegin.PointerVal);
	for (auto i = 0u; i < size; i++)
		EE->heapAllocas.push_back(ptr + i);

	/* Memory allocations are modeled as read events reading from INIT */
	g.addMallocToGraph(thr.id, static_cast<llvm::GenericValue *>(allocBegin.PointerVal),
			   allocEnd);

	return allocBegin;
}

void GenMCDriver::visitFree(llvm::GenericValue *ptr)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();
	llvm::GenericValue val;

	if (isExecutionDrivenByGraph())
		return;

	/* Attempt to free a NULL pointer */
	if (ptr == NULL)
		return;

	Event m = Event::getInitializer();
	auto before = g.getHbBefore(g.getLastThreadEvent(thr.id));
	for (auto i = 0u; i < g.events.size(); i++) {
		for (auto j = 1; j <= before[i]; j++) {
			auto &lab = g.events[i][j];
			if (lab.isMalloc() && lab.getAddr() == ptr) {
				m = lab.pos;
				break;
			}
		}
	}

	if (m == Event::getInitializer()) {
		WARN("ERROR: Attempted to free non-malloc'ed memory!\n");
		abort();
	}

	/* FIXME: Check the whole graph */
	/* Check to see whether this memory block has already been freed */
	if (std::find(EE->freedMem.begin(), EE->freedMem.end(), ptr)
	    != EE->freedMem.end()) {
		WARN("ERROR: Double-free error!\n");
		abort();
	}
	EE->freedMem.push_back(ptr);

	val.PointerVal = g.getEventLabel(m).val.PointerVal;

	/* Add a label with the appropriate store */
	g.addFreeToGraph(thr.id, ptr, val);
	return;
}

void GenMCDriver::visitError(std::string &err)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	/* Is the execution that led to the error consistent? */
	if (!isExecutionValid()) {
		EE->ECStack().clear();
		thr.block();
		return;
	}

	auto assertThrId = thr.id;
	auto errorEvent = g.getLastThreadEvent(assertThrId);
	auto before = g.getPorfBefore(errorEvent);

	/* Print error trace */
	if (userConf->printErrorTrace) {
		EE->replayExecutionBefore(before);
		printTraceBefore(g.getLastThreadEvent(assertThrId));
	}

	/* Dump the graph into a file (DOT format) */
	if (userConf->dotFile != "") {
		EE->replayExecutionBefore(before);
		dotPrintToFile(userConf->dotFile, before, errorEvent);
	}

	/* Print results and abort */
	llvm::dbgs() << "Assertion violation: " << err << "\n";
	printResults();
	abort();
}

bool graphCoveredByViews(ExecutionGraph &g, View &v)
{
	for (auto i = 0u; i < g.events.size(); i++) {
		if (v[i] + 1 != (int) g.events[i].size())
			return false;
	}
	return true;
}

bool GenMCDriver::tryToRevisitLock(EventLabel &rLab, View &preds, EventLabel &sLab, View &before,
				   std::vector<Event> &writePrefixPos,
				   std::vector<std::pair<Event, Event> > &moPlacings)
{
	auto &g = execGraph;
	auto v(preds);

	v.updateMax(before);
	if (graphCoveredByViews(g, v) && !rLab.revs.contains(writePrefixPos, moPlacings)) {
		EE->getThrById(rLab.getThread()).unblock();

		g.changeRf(rLab, sLab.getPos());
		auto newVal = rLab.nextVal;
		auto offsetMO = g.modOrder.getStoreOffset(rLab.getAddr(), rLab.rf) + 1;
		auto &sLab = g.addStoreToGraph(rLab.getThread(), rLab.getAttr(), rLab.ord,
					       rLab.getAddr(), rLab.valTyp, newVal, offsetMO);

		/* Adding to graph invalidates references, so we need to refetch */
		auto &rLab = g.getPreviousLabel(sLab.getPos());
		prioritizeThread = rLab.getThread(); // need to refetch
		if (EE->getThrById(rLab.getThread()).globalInstructions != 0)
			++EE->getThrById(rLab.getThread()).globalInstructions;

		rLab.revs.add(writePrefixPos, moPlacings);
		return true;
	}
	return false;
}

bool GenMCDriver::calcRevisits(EventLabel &lab)
{
	auto &g = getGraph();
	auto loads = getRevisitLoads(lab);
	auto pendingRMWs = g.getPendingRMWs(lab);

	if (pendingRMWs.size() > 0)
		loads.erase(std::remove_if(loads.begin(), loads.end(), [&](Event &e)
					{ auto &confLab = g.getEventLabel(pendingRMWs.back());
					  return g.getEventLabel(e).getStamp() > confLab.getStamp(); }),
			    loads.end());

	for (auto &l : loads) {
		auto &rLab = g.getEventLabel(l);
		auto preds = g.getViewFromStamp(rLab.getStamp());

		auto before = g.getPorfBefore(lab.getPos());
		auto prefixP = getPrefixToSaveNotBefore(lab, preds);
		auto &writePrefix = prefixP.first;
		auto &moPlacings = prefixP.second;

		auto writePrefixPos = g.getRfsNotBefore(writePrefix, preds);
		writePrefixPos.insert(writePrefixPos.begin(), lab.getPos());

		if (rLab.isLock() && EE->getThrById(rLab.getThread()).isBlocked &&
		    (int) g.events[rLab.getThread()].size() == rLab.getIndex() + 1) {
			if (tryToRevisitLock(rLab, preds, lab, before, writePrefixPos, moPlacings))
				continue;
			isMootExecution = true;
		}

		if (rLab.revs.contains(writePrefixPos, moPlacings))
			continue;

		rLab.revs.add(writePrefixPos, moPlacings);
		addToWorklist(SRevisit, rLab.getPos(), lab.getPos(),
			      std::move(writePrefix), std::move(moPlacings));
	}
	return !isMootExecution && (!lab.isRMW() || pendingRMWs.empty());
}

bool GenMCDriver::revisitReads(StackItem &p)
{
	auto &g = getGraph();
	auto &lab = g.getEventLabel(p.toRevisit);

	/* Restrict to the predecessors of the event we are revisiting */
	restrictGraph(lab.getStamp());
	restrictWorklist(lab.getStamp());

	/* Restore events that might have been deleted from the graph */
	switch (p.type) {
	case SRead:
	case SReadLibFunc:
		/* Nothing to restore in this case */
		break;
	case SRevisit:
		/*
		 * Restore the part of the SBRF-prefix of the store that revisits a load,
		 * that is not already present in the graph, the MO edges between that
		 * part and the previous MO, and make that part non-revisitable
		 */
		g.restoreStorePrefix(lab, p.writePrefix, p.moPlacings);
		break;
	case MOWrite:
		/* Try a different MO ordering, and also consider reads to revisit */
		g.modOrder.changeStoreOffset(lab.getAddr(), lab.getPos(), p.moPos);
		return calcRevisits(lab); /* Nothing else to do */
	case MOWriteLib:
		g.modOrder.changeStoreOffset(lab.getAddr(), lab.getPos(), p.moPos);
		return calcLibRevisits(lab);
	default:
		BUG();
	}

	/*
	 * For the case where an reads-from is changed, change the respective reads-from label
	 * and check whether a part of an RMW should be added
	 */
	g.changeRf(lab, p.shouldRf);

	/* If the revisited label became an RMW, add the store part and revisit */
	llvm::GenericValue rfVal = EE->loadValueFromWrite(lab.rf, lab.valTyp, lab.getAddr());
	if (lab.isRead() && lab.isRMW() && (lab.isFAI() || EE->compareValues(lab.valTyp, lab.val, rfVal))) {
		auto newVal = lab.nextVal;
		if (lab.isFAI())
			EE->executeAtomicRMWOperation(newVal, rfVal, lab.nextVal, lab.op);
		auto offsetMO = g.modOrder.getStoreOffset(lab.getAddr(), lab.rf) + 1;
		auto &sLab = g.addStoreToGraph(lab.getThread(), lab.getAttr(), lab.ord,
					       lab.getAddr(), lab.valTyp, newVal, offsetMO);
		return calcRevisits(sLab);
	}

	/* Blocked lock -> prioritize locking thread */
	if (lab.isRead() && lab.isLock()) {
		prioritizeThread = lab.rf.thread;
		EE->getThrById(p.toRevisit.thread).block();
	}

	if (p.type == SReadLibFunc)
		return calcLibRevisits(lab);

	return true;
}

llvm::GenericValue GenMCDriver::visitLibLoad(EventAttr attr, llvm::AtomicOrdering ord,
					    llvm::GenericValue *addr, llvm::Type *typ,
					    std::string functionName)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);

	if (isExecutionDrivenByGraph()) {
		auto &lab = getCurrentLabel();
		auto val = EE->loadValueFromWrite(lab.rf, typ, addr);
		return val;
	}

	/* Add the event to the graph so we'll be able to calculate consistent RFs */
	auto &lab = g.addLibReadToGraph(thr.id, ATTR_PLAIN, ord, addr, typ, Event::getInitializer(), functionName);

	/* Make sure there exists an initializer event for this memory location */
	auto stores(g.modOrder[addr]);
	auto it = std::find_if(stores.begin(), stores.end(),
			       [&g](Event e){ return g.getEventLabel(e).isLibInit(); });

	if (it == stores.end()) {
		WARN("Uninitialized memory location used by library found!\n");
		abort();
	}

	auto preds = g.getViewFromStamp(lab.getStamp());
	auto validStores = g.getLibConsRfsInView(*lib, lab, stores, preds);

	/*
	 * If this is a non-functional library, choose one of the available reads-from
	 * options, push the rest to the stack, and return an appropriate value to
	 * the interpreter
	 */
	if (!lib->hasFunctionalRfs()) {
		BUG_ON(validStores.empty());
		g.changeRf(lab, validStores[0]);
		for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
			addToWorklist(SRead, lab.getPos(), *it, {}, {});
		return EE->loadValueFromWrite(lab.rf, typ, addr);
	}

	/* Otherwise, first record all the inconsistent options */
	std::copy_if(stores.begin(), stores.end(), std::back_inserter(lab.invalidRfs),
		     [&validStores](Event &e){ return std::find(validStores.begin(),
								validStores.end(), e) == validStores.end(); });

	/* Then, partition the stores based on whether they are read */
	auto invIt = std::partition(validStores.begin(), validStores.end(),
				    [&g](Event &e){ return g.getEventLabel(e).rfm1.empty(); });

	/* Push all options that break RF functionality to the stack */
	for (auto it = invIt; it != validStores.end(); ++it) {
		auto &sLab = g.getEventLabel(*it);
		BUG_ON(sLab.rfm1.size() > 1);
		if (g.getEventLabel(sLab.rfm1.back()).isRevisitable())
			addToWorklist(SReadLibFunc, lab.getPos(), *it, {}, {});
	}

	/* If there is no valid RF, we have to read BOTTOM */
	if (invIt == validStores.begin()) {
		WARN_ONCE("lib-not-always-block", "FIXME: SHOULD NOT ALWAYS BLOCK -- ALSO IN EE\n");
		auto tempRf = Event::getInitializer();
		return EE->loadValueFromWrite(tempRf, typ, addr);
	}

	/*
	 * If BOTTOM is not the only option, push it to inconsistent RFs as well,
	 * choose a valid store to read-from, and push the other alternatives to
	 * the stack
	 */
	WARN_ONCE("lib-check-before-push", "FIXME: CHECK IF IT'S A NON BLOCKING LIB BEFORE PUSHING?\n");
	lab.invalidRfs.push_back(Event::getInitializer());
	g.changeRf(lab, validStores[0]);

	for (auto it = validStores.begin() + 1; it != invIt; ++it)
		addToWorklist(SRead, lab.getPos(), *it, {}, {});

	return EE->loadValueFromWrite(lab.rf, typ, addr);
}

void GenMCDriver::visitLibStore(EventAttr attr, llvm::AtomicOrdering ord, llvm::GenericValue *addr,
			       llvm::Type *typ, llvm::GenericValue &val, std::string functionName,
			       bool isInit)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph())
		return;

	/* TODO: Make virtual the race-tracking function ?? */

	/*
	 * We need to try all possible MO placements, but after the initialization write,
	 * which is explicitly included in MO, in the case of libraries.
	 */
	auto &locMO = g.modOrder[addr];
	auto begO = 1;
	auto endO = (int) locMO.size();

	/* If there was not any store previously, check if this location was initialized  */
	if (endO == 0 && !isInit) {
		WARN("Uninitialized memory location used by library found!\n");
		abort();
	}

	/* It is always consistent to add a new event at the end of MO */
	auto &sLab = g.addLibStoreToGraph(thr.id, attr, ord, addr, typ, val, endO, functionName, isInit);

	calcLibRevisits(sLab);

	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);
	if (lib && !lib->tracksCoherence())
		return;

	/*
	 * Check for alternative MO placings. Temporarily remove sLab from
	 * MO, find all possible alternatives, and push them to the workqueue
	 */
	locMO.pop_back();
	for (auto i = begO; i < endO; ++i) {
		locMO.insert(locMO.begin() + i, sLab.getPos());

		/* Check consistency for the graph with this MO */
		auto preds = g.getViewFromStamp(sLab.getStamp());
		if (g.isLibConsistentInView(*lib, preds))
			addToWorklist(MOWriteLib, sLab.getPos(), sLab.rf, {}, {}, i);
		locMO.erase(locMO.begin() + i);
	}
	locMO.push_back(sLab.getPos());
	return;
}

bool GenMCDriver::calcLibRevisits(EventLabel &lab)
{
	/* Get the library of the event causing the revisiting */
	auto &g = getGraph();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), lab.functionName);
	auto valid = true;
	std::vector<Event> loads, stores;

	/*
	 * If this is a read of a functional library causing the revisit,
	 * then this is a functionality violation: we need to find the conflicting
	 * event, and find an alternative reads-from edge for it
	 */
	if (lib->hasFunctionalRfs() && lab.isRead()) {
		auto conf = g.getPendingLibRead(lab);
		loads = {conf};
		stores = g.getEventLabel(conf).invalidRfs;
		valid = false;
	} else {
		/* It is a normal store -- we need to find revisitable loads */
		loads = g.getRevisitable(lab);
		stores = {lab.getPos()};
	}

	/* Next, find which of the 'stores' can be read by 'loads' */
	auto before = g.getPorfBefore(lab.getPos());
	for (auto &l : loads) {
		/* Calculate the view of the resulting graph */
		auto &rLab = g.getEventLabel(l);
		auto preds = g.getViewFromStamp(rLab.getStamp());
		auto v = preds;
		v.updateMax(before);

		/* Check if this the resulting graph is consistent */
		auto rfs = g.getLibConsRfsInView(*lib, rLab, stores, v);

		for (auto &rf : rfs) {
			/* Push consistent options to stack */
			auto writePrefix = g.getPrefixLabelsNotBefore(before, preds);
			auto moPlacings = (lib->tracksCoherence()) ? g.getMOPredsInBefore(writePrefix, preds)
				                                   : std::vector<std::pair<Event, Event> >();
			auto writePrefixPos = g.getRfsNotBefore(writePrefix, preds);
			writePrefixPos.insert(writePrefixPos.begin(), lab.getPos());

			if (rLab.revs.contains(writePrefixPos, moPlacings))
				continue;

			rLab.revs.add(writePrefixPos, moPlacings);
			addToWorklist(SRevisit, rLab.getPos(), rf,
				      std::move(writePrefix), std::move(moPlacings));

		}
	}
	return valid;
}


/************************************************************
 ** Printing facilities
 ***********************************************************/

void GenMCDriver::prettyPrintGraph()
{
	auto &g = getGraph();
	for (auto i = 0u; i < g.events.size(); i++) {
		auto &thr = EE->getThrById(i);
		llvm::dbgs() << "<" << thr.parentId << "," << thr.id
			     << "> " << thr.threadFun->getName() << ": ";
		for (auto j = 0u; j < g.events[i].size(); j++) {
			auto &lab = g.events[i][j];
			if (lab.isRead()) {
				if (lab.isRevisitable())
					llvm::dbgs().changeColor(llvm::raw_ostream::Colors::GREEN);
				auto val = EE->loadValueFromWrite(lab.rf, lab.valTyp, lab.getAddr());
				llvm::dbgs() << lab.type << EE->getGlobalName(lab.getAddr()) << ","
					     << val.IntVal << " ";
				llvm::dbgs().resetColor();
			} else if (lab.isWrite()) {
				llvm::dbgs() << lab.type << EE->getGlobalName(lab.getAddr()) << ","
					     << lab.val.IntVal << " ";
			}
		}
		llvm::dbgs() << "\n";
	}
	llvm::dbgs() << "\n";
}

void GenMCDriver::dotPrintToFile(std::string &filename, View &before, Event e)
{
	ExecutionGraph &g = getGraph();
	std::ofstream fout(filename);
	std::string dump;
	llvm::raw_string_ostream ss(dump);

	/* Create a directed graph graph */
	ss << "strict digraph {\n";
	/* Specify node shape */
	ss << "\tnode [shape=box]\n";
	/* Left-justify labels for clusters */
	ss << "\tlabeljust=l\n";
	/* Create a node for the initializer event */
	ss << "\t\"" << Event::getInitializer() << "\"[label=INIT,root=true]\n";

	/* Print all nodes with each thread represented by a cluster */
	for (auto i = 0u; i < before.size(); i++) {
		auto &thr = EE->getThrById(i);
		ss << "subgraph cluster_" << thr.id << "{\n";
		ss << "\tlabel=\"" << thr.threadFun->getName().str() << "()\"\n";
		for (auto j = 1; j <= before[i]; j++) {
			std::stringstream buf;
			auto lab = g.events[i][j];

			Parser::parseInstFromMData(buf, thr.prefixLOC[j], "");
			ss << "\t" << lab.getPos() << " [label=\"" << buf.str() << "\""
			   << (lab.getPos() == e ? ",style=filled,fillcolor=yellow" : "") << "]\n";
		}
		ss << "}\n";
	}

	/* Print relations between events (po U rf) */
	for (auto i = 0u; i < before.size(); i++) {
		auto &thr = EE->getThrById(i);
		for (auto j = 0; j <= before[i]; j++) {
			std::stringstream buf;
			auto lab = g.events[i][j];

			Parser::parseInstFromMData(buf, thr.prefixLOC[j], "");
			/* Print a po-edge, but skip dummy start events for
			 * all threads except for the first one */
			if (j < before[i] && !(lab.isStart() && i > 0))
				ss << lab.getPos() << " -> " << lab.getPos().next() << "\n";
			if (lab.isRead())
				ss << "\t" << lab.rf << " -> " << lab.getPos() << "[color=green]\n";
			if (thr.id > 0 && lab.isStart())
				ss << "\t" << lab.rf << " -> " << lab.getPos().next() << "[color=blue]\n";
			if (lab.isJoin())
				ss << "\t" << lab.rf << " -> " << lab.getPos() << "[color=blue]\n";
		}
	}

	ss << "}\n";
	fout << ss.str();
	fout.close();
}

void GenMCDriver::calcTraceBefore(const Event &e, View &a, std::stringstream &buf)
{
	auto &g = getGraph();
	auto ai = a[e.thread];

	if (e.index <= ai)
		return;

	a[e.thread] = e.index;
	auto &thr = EE->getThrById(e.thread);
	for (int i = ai; i <= e.index; i++) {
		auto &lab = g.events[e.thread][i];
		if (lab.hasReadSem() && !lab.rf.isInitializer())
			calcTraceBefore(lab.rf, a, buf);
		Parser::parseInstFromMData(buf, thr.prefixLOC[i], thr.threadFun->getName().str());
	}
	return;
}

void GenMCDriver::printTraceBefore(Event e)
{
	std::stringstream buf;
	View a;

	calcTraceBefore(e, a, buf);
	llvm::dbgs() << buf.str();
}
