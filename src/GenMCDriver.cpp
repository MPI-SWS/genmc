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
#include <llvm/Support/raw_os_ostream.h>

#include <algorithm>
#include <csignal>

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
	std::string dups = " (" + std::to_string(duplicates) + " duplicates)";
	llvm::dbgs() << "Number of complete executions explored: " << explored
		     << ((userConf->countDuplicateExecs) ? dups : "") << "\n";
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

		const EventLabel *lab = g.getEventLabel(Event(i, 0));
		BUG_ON(!llvm::isa<ThreadStartLabel>(lab));

		auto *labFst = static_cast<const ThreadStartLabel *>(lab);
		Event parent = labFst->getParentCreate();

		/* Skip if parent create does not exist yet */
		if (parent.index >= (int) g.events[parent.thread].size())
			continue;

		/* This could fire for the main() thread */
		BUG_ON(!llvm::isa<ThreadCreateLabel>(g.getEventLabel(parent)));

		/* Skip finished threads */
		const EventLabel *labLast = g.getLastThreadLabel(i);
		if (llvm::isa<ThreadFinishLabel>(labLast))
			continue;

		/* Otherwise, initialize ECStacks in interpreter */
		auto &thr = EE->getThrById(i);
		BUG_ON(!thr.ECStack.empty() || thr.isBlocked);
		thr.ECStack.push_back(thr.initSF);

		/* Mark threads that are blocked appropriately */
		if (auto *lLab = llvm::dyn_cast<CasReadLabel>(labLast)) {
			if (lLab->isLock())
				thr.block();
		}
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
		printGraph();
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
				std::vector<std::unique_ptr<EventLabel> > &&prefix,
				std::vector<std::pair<Event, Event> > &&moPlacings,
				int newMoPos = 0)

{
	const EventLabel *lab = getGraph().getEventLabel(e);
	StackItem s(t, e, shouldRf, std::move(prefix),
		    std::move(moPlacings), newMoPos);

	workqueue[lab->getStamp()].push_back(std::move(s));
}

GenMCDriver::StackItem GenMCDriver::getNextItem()
{
	for (auto rit = workqueue.rbegin(); rit != workqueue.rend(); ++rit) {
		if (rit->second.empty())
			continue;

		auto si = std::move(rit->second.back());
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
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *mLab = llvm::dyn_cast<MallocLabel>(lab))
				EE->freeRegion(mLab->getAllocAddr(),
					       mLab->getAllocSize());
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
		    !llvm::isa<ThreadFinishLabel>(g.getLastThreadLabel(prioritizeThread))) {
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

		if (llvm::isa<ThreadFinishLabel>(g.getLastThreadLabel(i))) {
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

const EventLabel *GenMCDriver::getCurrentLabel()
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	BUG_ON(thr.globalInstructions >= g.events[thr.id].size());
	return g.getEventLabel(Event(thr.id, thr.globalInstructions));
}

llvm::GenericValue GenMCDriver::getWriteValue(Event write,
					      const llvm::GenericValue *ptr,
					      const llvm::Type *typ)
{
	if (write.isInitializer())
		return getEE()->getLocInitVal((llvm::GenericValue *)ptr,
					      (llvm::Type *) typ);

	const EventLabel *lab = getGraph().getEventLabel(write);
	BUG_ON(!llvm::isa<WriteLabel>(lab));
	return static_cast<const WriteLabel *>(lab)->getVal();
}



/*
 * This function is called to check for races when a new event is added.
 * When a race is detected, we have to actually ensure that the execution is valid,
 * in the sense that it is consistent. Thus, this method relies on overrided
 * virtual methods to check for consistency, depending on the operating mode
 * of the driver.
 */
void GenMCDriver::checkForRaces()
{
	if (userConf->disableRaceDetection)
		return;

	auto &g = getGraph();
	const EventLabel *lab = g.getLastThreadLabel(EE->getCurThr().id);

	/* We only check for races when reads and writes are added */
	BUG_ON(!llvm::isa<MemAccessLabel>(lab));

	auto racy = Event::getInitializer();
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
		racy = g.findRaceForNewLoad(rLab);
	else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab))
		racy = g.findRaceForNewStore(wLab);

	/* If a race is found and the execution is consistent, return it */
	if (!racy.isInitializer() && isExecutionValid()) {
		visitError("", racy, DE_RaceNotAtomic);
		BUG(); /* visitError() should abort since it is consistent */
	}

	/* Else, if this is a heap allocation, also look for memory errors */
	auto *mLab = static_cast<const MemAccessLabel *>(lab);
	if (!EE->isHeapAlloca(mLab->getAddr()))
		return;

	const View &before = g.getHbBefore(lab->getPos().prev());
	for (auto i = 0u; i < g.events.size(); i++)
		for (auto j = 0u; j < g.events[i].size(); j++) {
			const EventLabel *oLab = g.getEventLabel(Event(i, j));
			if (auto *fLab = llvm::dyn_cast<FreeLabel>(oLab)) {
				if (fLab->getFreedAddr() == mLab->getAddr()) {
					visitError("The accessed address has already been freed!",
						   oLab->getPos(), DE_AccessFreed);
				}
			}
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(oLab)) {
				if (aLab->getAllocAddr() <= mLab->getAddr() &&
				    ((char *) aLab->getAllocAddr() +
				     aLab->getAllocSize() > (char *) mLab->getAddr()) &&
				    !before.contains(oLab->getPos())) {
					visitError("The allocating operation (malloc()) "
						   "does not happen-before the memory access!",
						   oLab->getPos(), DE_AccessNonMalloc);
				}
			}
	}
	return;
}

std::vector<Event> GenMCDriver::filterAcquiredLocks(const llvm::GenericValue *ptr,
						    const std::vector<Event> &stores,
						    const View &before)
{
	auto &g = getGraph();
	std::vector<Event> valid, conflicting;

	for (auto &s : stores) {
		if (auto *wLab = llvm::dyn_cast<CasWriteLabel>(g.getEventLabel(s))) {
			if (wLab->isLock())
				continue;
		}

		if (g.isStoreReadBySettledRMW(s, ptr, before))
			continue;

		if (g.isStoreReadByExclusiveRead(s, ptr))
			conflicting.push_back(s);
		else
			valid.push_back(s);
	}

	if (valid.empty()) {
		auto lit = std::find_if(stores.begin(), stores.end(), [&](const Event &s) {
				if (auto *wLab = llvm::dyn_cast<CasWriteLabel>(g.getEventLabel(s)))
					if (wLab->isLock())
						return true;
				return false;
			});
		BUG_ON(lit == stores.end());
		prioritizeThread = lit->thread;
		valid.push_back(*lit);
	}
	valid.insert(valid.end(), conflicting.begin(), conflicting.end());
	return valid;
}

std::vector<Event>
GenMCDriver::properlyOrderStores(llvm::Interpreter::InstAttr attr,
				 llvm::Type *typ,
				 const llvm::GenericValue *ptr,
				 llvm::GenericValue &expVal,
				 std::vector<Event> &stores)
{
	if (attr == llvm::Interpreter::IA_None ||
	    attr == llvm::Interpreter::IA_Unlock)
		return stores;

	auto &g = getGraph();
	auto &before = g.getPorfBefore(g.getLastThreadEvent(EE->getCurThr().id));

	if (attr == llvm::Interpreter::IA_Lock)
		return filterAcquiredLocks(ptr, stores, before);

	std::vector<Event> valid, conflicting;
	for (auto &s : stores) {
		auto oldVal = getWriteValue(s, ptr, typ);
		if ((attr == llvm::Interpreter::IA_Fai ||
		     EE->compareValues(typ, oldVal, expVal)) &&
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

	if (isExecutionDrivenByGraph()) {
		auto *cLab = static_cast<const ThreadCreateLabel *>(getCurrentLabel());
		return cLab->getChildId();
	}

	Event cur(curThr.id, g.events[curThr.id].size());
	int cid = 0;

	/* First, check if the thread to be created already exists */
	while (cid < (int) g.events.size()) {
		if (!g.events[cid].empty()) {
			if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(
				    g.getEventLabel(Event(cid, 0)))) {
				if (bLab->getParentCreate() == cur)
					break;
			}
		}
		++cid;
	}

	/* Add an event for the thread creation */
	g.addTCreateToGraph(cur.thread, cid);

	/* Prepare the execution context for the new thread */
	llvm::Thread thr(calledFun, cid, cur.thread, SF);
	thr.ECStack.push_back(SF);
	thr.tls = EE->threadLocalVars;

	if (cid == (int) g.events.size()) {
		/* If the thread does not exist in the graph, make an entry for it */
		EE->threads.push_back(thr);
		g.events.push_back({});
		g.addStartToGraph(cid, g.getLastThreadEvent(EE->getCurThr().id)); // need to refetch ref
	} else {
		/* Otherwise, just push the execution context to the interpreter */
		EE->threads[cid] = thr;
	}
	return cid;
}

llvm::GenericValue GenMCDriver::visitThreadJoin(llvm::Function *F, const llvm::GenericValue &arg)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	int cid = arg.IntVal.getLimitedValue(std::numeric_limits<int>::max());
	if (cid < 0 || int (EE->threads.size()) <= cid || cid == thr.id) {
		std::string err = "ERROR: Invalid TID in pthread_join(): " + std::to_string(cid);
		if (cid == thr.id)
			err += " (TID cannot be the same as the calling thread)";
		visitError(err, Event::getInitializer(), DE_InvalidJoin);
	}

	/* If necessary, add a relevant event to the graph */
	if (!isExecutionDrivenByGraph())
		g.addTJoinToGraph(thr.id, cid);

	/* If the update failed (child has not terminated yet) block this thread */
	if (!g.updateJoin(getCurrentLabel()->getPos(), g.getLastThreadEvent(cid)))
		thr.block();

	/*
	 * We always return a success value, so as not to have to update it
	 * when the thread unblocks.
	 */
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

		const EventLabel *lab = g.getLastThreadLabel(thr.id);
		for (auto i = 0u; i < g.events.size(); i++) {
			const EventLabel *pLastLab = g.getLastThreadLabel(i);
			if (auto *pLab = llvm::dyn_cast<ThreadJoinLabel>(pLastLab)) {
				if (pLab->getChildId() != thr.id)
					continue;

				/* If parent thread is waiting for me, relieve it */
				EE->getThrById(i).unblock();
				g.updateJoin(pLab->getPos(), lab->getPos());
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

llvm::GenericValue GenMCDriver::visitLoad(llvm::Interpreter::InstAttr attr,
					  llvm::AtomicOrdering ord,
					  const llvm::GenericValue *addr,
					  llvm::Type *typ,
					  llvm::GenericValue cmpVal,
					  llvm::GenericValue rmwVal,
					  llvm::AtomicRMWInst::BinOp op)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph()) {
		const EventLabel *lab = getCurrentLabel();
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
			return getWriteValue(rLab->getRf(), addr, typ);
		BUG();
	}

	/* Get all stores to this location from which we can read from */
	auto stores = getStoresToLoc(addr);
	BUG_ON(stores.empty());
	auto validStores = properlyOrderStores(attr, typ, addr, cmpVal, stores);

	/* ... and add an appropriate label with a particular rf */
	const ReadLabel *lab = nullptr;
	switch (attr) {
	case llvm::Interpreter::IA_None:
		lab = g.addReadToGraph(thr.id, ord, addr, typ, validStores[0]);
		break;
	case llvm::Interpreter::IA_Fai:
		lab = g.addFaiReadToGraph(thr.id, ord, addr, typ, validStores[0],
					  op, std::move(rmwVal));
		break;
	case llvm::Interpreter::IA_Cas:
	case llvm::Interpreter::IA_Lock:
		lab = g.addCasReadToGraph(thr.id, ord, addr, typ,
					  validStores[0], cmpVal, rmwVal,
					  attr == llvm::Interpreter::IA_Lock);
		break;
	default:
		BUG();
	}

	/* Check for races */
	checkForRaces();

	/* Push all the other alternatives choices to the Stack */
	for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
		addToWorklist(SRead, lab->getPos(), *it, {}, {});
	return getWriteValue(validStores[0], addr, typ);
}

void GenMCDriver::visitStore(llvm::Interpreter::InstAttr attr,
			     llvm::AtomicOrdering ord,
			     const llvm::GenericValue *addr,
			     llvm::Type *typ,
			     llvm::GenericValue &val)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph())
		return;

	auto placesRange =
		getPossibleMOPlaces(addr, attr != llvm::Interpreter::IA_None &&
				    attr != llvm::Interpreter::IA_Unlock);
	auto &begO = placesRange.first;
	auto &endO = placesRange.second;

	/* It is always consistent to add the store at the end of MO */
	const WriteLabel *lab = nullptr;
	switch (attr) {
	case llvm::Interpreter::IA_None:
	case llvm::Interpreter::IA_Unlock:
		lab = g.addStoreToGraph(thr.id, ord, addr, typ, val, endO,
					attr == llvm::Interpreter::IA_Unlock);
		break;
	case llvm::Interpreter::IA_Fai:
		lab = g.addFaiStoreToGraph(thr.id, ord, addr, typ, val, endO);
		break;
	case llvm::Interpreter::IA_Cas:
	case llvm::Interpreter::IA_Lock:
		lab = g.addCasStoreToGraph(thr.id, ord, addr, typ, val, endO,
					   attr == llvm::Interpreter::IA_Lock);
		break;
	}

	/* Check for races */
	checkForRaces();

	auto &locMO = g.modOrder[addr];
	for (auto it = locMO.begin() + begO; it != locMO.begin() + endO; ++it) {

		/* We cannot place the write just before the write of an RMW */
		if (llvm::isa<FaiWriteLabel>(g.getEventLabel(*it)) ||
		    llvm::isa<CasWriteLabel>(g.getEventLabel(*it)))
			continue;

		/* Push the stack item */
		addToWorklist(MOWrite, lab->getPos(), Event::getInitializer(),
			      {}, {}, std::distance(locMO.begin(), it));
	}

	calcRevisits(lab);
	return;
}

llvm::GenericValue GenMCDriver::visitMalloc(const llvm::GenericValue &argSize)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();
	llvm::GenericValue allocBegin;

	if (isExecutionDrivenByGraph()) {
		const EventLabel *lab = getCurrentLabel();
		if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
			allocBegin.PointerVal = (void *) aLab->getAllocAddr();
			return allocBegin;
		}
		BUG();
	}

	WARN_ON_ONCE(argSize.IntVal.getBitWidth() > 64, "malloc-alignment",
		     "WARNING: malloc()'s alignment larger than 64-bit! Limiting...\n");

	/* Get a fresh address and also store the size of this allocation */
	allocBegin.PointerVal = EE->heapAllocas.empty() ? (char *) EE->globalVars.back() + 1
		                                        : (char *) EE->heapAllocas.back() + 1;
	uint64_t size = argSize.IntVal.getLimitedValue();

	/* Track the address allocated*/
	char *ptr = static_cast<char *>(allocBegin.PointerVal);
	for (auto i = 0u; i < size; i++)
		EE->heapAllocas.push_back(ptr + i);

	/* Add a relevant label to the graph */
	g.addMallocToGraph(thr.id, allocBegin.PointerVal, size);

	return allocBegin;
}

void GenMCDriver::visitFree(llvm::GenericValue *ptr)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph())
		return;

	/* Attempt to free a NULL pointer */
	if (ptr == NULL)
		return;

	const MallocLabel *m = nullptr;
	auto &before = g.getHbBefore(g.getLastThreadEvent(thr.id));
	for (auto i = 0u; i < g.events.size(); i++) {
		for (auto j = 1; j <= before[i]; j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
				if (aLab->getAllocAddr() == ptr) {
					m = aLab;
					break;
				}
			}
		}
	}

	if (!m) {
		visitError("", Event::getInitializer(), DE_FreeNonMalloc);
		BUG(); /* visitError() should abort */
	}

	/* FIXME: Check the whole graph */
	/* Check to see whether this memory block has already been freed */
	if (std::find(EE->freedMem.begin(), EE->freedMem.end(), ptr)
	    != EE->freedMem.end()) {
		WARN("ERROR: Double-free error!\n");
		abort();
	}
	EE->freedMem.push_back(ptr);

	/* Add a label with the appropriate store */
	g.addFreeToGraph(thr.id, ptr, m->getAllocSize());
	return;
}

void GenMCDriver::visitError(std::string err, Event confEvent,
			     DriverErrorKind t /* = DE_Safety */ )
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();

	/* If the execution that led to the error is not consistent, block */
	if (!isExecutionValid()) {
		EE->ECStack().clear();
		thr.block();
		return;
	}

	Event errorEvent = g.getLastThreadEvent(thr.id);

	/* Print a basic error message and the graph */
	llvm::dbgs() << "Error detected: " << t << "!\n";
	llvm::dbgs() << "Event " << errorEvent << " ";
	if (!confEvent.isInitializer())
		llvm::dbgs() << "conflicts with event " << confEvent << " ";
	llvm::dbgs() << "in graph:\n";
	printGraph(true);

	/* Print error trace leading up to the violating event(s) */
	if (userConf->printErrorTrace) {
		printTraceBefore(errorEvent);
		if (!confEvent.isInitializer())
			printTraceBefore(confEvent);
	}

	/* Print the specific error message */
	if (!err.empty())
		llvm::dbgs() << err << "\n";

	/* Dump the graph into a file (DOT format) */
	if (userConf->dotFile != "")
		dotPrintToFile(userConf->dotFile, errorEvent, confEvent);

	/* Print results and abort */
	printResults();
	abort();
}

bool graphCoveredByViews(ExecutionGraph &g, const View &v)
{
	for (auto i = 0u; i < g.events.size(); i++) {
		if (v[i] + 1 != (int) g.events[i].size())
			return false;
	}
	return true;
}

bool GenMCDriver::tryToRevisitLock(const CasReadLabel *rLab, const View &preds,
				   const WriteLabel *sLab, const View &before,
				   const std::vector<Event> &writePrefixPos,
				   const std::vector<std::pair<Event, Event> > &moPlacings)
{
	auto &g = execGraph;
	auto v(preds);

	v.update(before);
	if (graphCoveredByViews(g, v) && !g.revisitSetContains(rLab, writePrefixPos, moPlacings)) {
		EE->getThrById(rLab->getThread()).unblock();

		g.changeRf(rLab->getPos(), sLab->getPos());
		auto newVal = rLab->getSwapVal();
		auto offsetMO = g.modOrder.getStoreOffset(rLab->getAddr(), rLab->getRf()) + 1;

		g.addCasStoreToGraph(rLab->getThread(), rLab->getOrdering(), rLab->getAddr(),
				     rLab->getType(), newVal, offsetMO, true /* isLock */);

		prioritizeThread = rLab->getThread();
		if (EE->getThrById(rLab->getThread()).globalInstructions != 0)
			++EE->getThrById(rLab->getThread()).globalInstructions;

		g.addToRevisitSet(rLab, writePrefixPos, moPlacings);
		return true;
	}
	return false;
}

bool GenMCDriver::calcRevisits(const WriteLabel *sLab)
{
	auto &g = getGraph();
	std::vector<Event> loads = getRevisitLoads(sLab);
	std::vector<Event> pendingRMWs = g.getPendingRMWs(sLab); /* empty or singleton */

	if (pendingRMWs.size() > 0)
		loads.erase(std::remove_if(loads.begin(), loads.end(), [&](Event &e)
					{ auto *confLab = g.getEventLabel(pendingRMWs.back());
					  return g.getEventLabel(e)->getStamp() > confLab->getStamp(); }),
			    loads.end());

	for (auto &l : loads) {
		const EventLabel *lab = g.getEventLabel(l);
		BUG_ON(!llvm::isa<ReadLabel>(lab));

		auto *rLab = static_cast<const ReadLabel *>(lab);

		/* Get all events added before the read */
		View preds = g.getViewFromStamp(rLab->getStamp());

		/* Get the prefix of the write to save */
		auto &before = g.getPorfBefore(sLab->getPos());
		auto prefixP = getPrefixToSaveNotBefore(sLab, preds);
		auto &writePrefix = prefixP.first;
		auto &moPlacings = prefixP.second;

		auto writePrefixPos = g.getRfsNotBefore(writePrefix, preds);
		writePrefixPos.insert(writePrefixPos.begin(), sLab->getPos());

		/* Optimize handling of lock operations */
		if (auto *lLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
			if (lLab->isLock() && EE->getThrById(lLab->getThread()).isBlocked &&
			    (int) g.events[lLab->getThread()].size() == lLab->getIndex() + 1) {
				if (tryToRevisitLock(lLab, preds, sLab, before, writePrefixPos, moPlacings))
					continue;
				isMootExecution = true;
			}
		}

		/* If this prefix has revisited the read before, skip */
		if (g.revisitSetContains(rLab, writePrefixPos, moPlacings))
			continue;

		/* Otherwise, add the prefix to the revisit set and the worklist */
		g.addToRevisitSet(rLab, writePrefixPos, moPlacings);
		addToWorklist(SRevisit, rLab->getPos(), sLab->getPos(),
			      std::move(writePrefix), std::move(moPlacings));
	}

	bool consG = !(llvm::isa<CasWriteLabel>(sLab) || llvm::isa<FaiWriteLabel>(sLab)) ||
		pendingRMWs.empty();
	return !isMootExecution && consG;
}

bool GenMCDriver::revisitReads(StackItem &p)
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(p.toRevisit);

	/* Restrict to the predecessors of the event we are revisiting */
	restrictGraph(lab->getStamp());
	restrictWorklist(lab->getStamp());

	/* Restore events that might have been deleted from the graph */
	switch (p.type) {
	case SRead:
	case SReadLibFunc:
		/* Nothing to restore in this case */
		break;
	case SRevisit: {
		/*
		 * Restore the part of the SBRF-prefix of the store that revisits a load,
		 * that is not already present in the graph, the MO edges between that
		 * part and the previous MO, and make that part non-revisitable
		 */
		BUG_ON(!llvm::isa<ReadLabel>(lab));
		auto *rLab = static_cast<const ReadLabel *>(lab);
		g.restoreStorePrefix(rLab, p.writePrefix, p.moPlacings);
		break;
	}
	case MOWrite: {
		/* Try a different MO ordering, and also consider reads to revisit */
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);
		g.modOrder.changeStoreOffset(wLab->getAddr(), wLab->getPos(), p.moPos);
		return calcRevisits(wLab); /* Nothing else to do */
	}
	case MOWriteLib: {
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);
		g.modOrder.changeStoreOffset(wLab->getAddr(), wLab->getPos(), p.moPos);
		return calcLibRevisits(wLab);
	}
	default:
		BUG();
	}

	/* If we reached this point, we are dealing with a read, so casting is safe */
	BUG_ON(!llvm::isa<ReadLabel>(lab));
	auto *rLab = static_cast<const ReadLabel *>(lab);

	/*
	 * For the case where an reads-from is changed, change the respective reads-from label
	 * and check whether a part of an RMW should be added
	 */
	g.changeRf(rLab->getPos(), p.shouldRf);

	/* If the revisited label became an RMW, add the store part and revisit */
	auto rfVal = getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());
	auto offsetMO = g.modOrder.getStoreOffset(rLab->getAddr(), rLab->getRf()) + 1;

	const WriteLabel *sLab = nullptr;
	if (auto *faiLab = llvm::dyn_cast<FaiReadLabel>(rLab)) {
		llvm::GenericValue result;
		EE->executeAtomicRMWOperation(result, rfVal, faiLab->getOpVal(),
					      faiLab->getOp());
		sLab = g.addFaiStoreToGraph(faiLab->getThread(), faiLab->getOrdering(),
					    faiLab->getAddr(), faiLab->getType(), result,
					    offsetMO);
	} else if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		if (EE->compareValues(casLab->getType(), casLab->getExpected(), rfVal)) {
			sLab = g.addCasStoreToGraph(casLab->getThread(), casLab->getOrdering(),
						    casLab->getAddr(), casLab->getType(),
						    casLab->getSwapVal(), offsetMO, casLab->isLock());
		}
	}
	if (sLab)
		return calcRevisits(sLab);

	/* Blocked lock -> prioritize locking thread */
	if (auto *lLab = llvm::dyn_cast<CasReadLabel>(lab)) {
		if (lLab->isLock()) {
			prioritizeThread = lLab->getRf().thread;
			EE->getThrById(p.toRevisit.thread).block();
		}
	}

	if (p.type == SReadLibFunc)
		return calcLibRevisits(lab);

	return true;
}

std::pair<llvm::GenericValue, bool>
GenMCDriver::visitLibLoad(llvm::Interpreter::InstAttr attr,
			  llvm::AtomicOrdering ord,
			  const llvm::GenericValue *addr,
			  llvm::Type *typ,
			  std::string functionName)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);

	if (isExecutionDrivenByGraph()) {
		const EventLabel *lab = getCurrentLabel();
		if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab))
			return std::make_pair(getWriteValue(rLab->getRf(), addr, typ),
					      rLab->getRf().isInitializer());
		BUG();
	}

	/* Add the event to the graph so we'll be able to calculate consistent RFs */
	const LibReadLabel *lab = g.addLibReadToGraph(thr.id, ord, addr, typ,
						      Event::getInitializer(), functionName);

	/* Make sure there exists an initializer event for this memory location */
	auto stores(g.modOrder[addr]);
	auto it = std::find_if(stores.begin(), stores.end(), [&](Event e){
			const EventLabel *eLab = g.getEventLabel(e);
			if (auto *wLab = llvm::dyn_cast<LibWriteLabel>(eLab))
				return wLab->isLibInit();
			BUG();
		});

	if (it == stores.end()) {
		visitError(std::string("Uninitialized memory used by library ") +
			   lib->getName() + ", member " + functionName +
			   std::string(" found"), Event::getInitializer(),
			   DE_UninitializedMem);
		BUG();
	}

	auto preds = g.getViewFromStamp(lab->getStamp());
	auto validStores = g.getLibConsRfsInView(*lib, lab->getPos(), stores, preds);

	/*
	 * If this is a non-functional library, choose one of the available reads-from
	 * options, push the rest to the stack, and return an appropriate value to
	 * the interpreter
	 */
	if (!lib->hasFunctionalRfs()) {
		BUG_ON(validStores.empty());
		g.changeRf(lab->getPos(), validStores[0]);
		for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
			addToWorklist(SRead, lab->getPos(), *it, {}, {});
		return std::make_pair(getWriteValue(lab->getRf(), addr, typ),
				      lab->getRf().isInitializer());
	}

	/* Otherwise, first record all the inconsistent options */
	std::vector<Event> invalid;
	std::copy_if(stores.begin(), stores.end(), std::back_inserter(invalid),
		     [&](Event &e){ return std::find(validStores.begin(), validStores.end(),
						     e) == validStores.end(); });
	g.addInvalidRfs(lab->getPos(), invalid);

	/* Then, partition the stores based on whether they are read */
	auto invIt = std::partition(validStores.begin(), validStores.end(), [&](Event &e){
			const EventLabel *eLab = g.getEventLabel(e);
			if (auto *wLab = llvm::dyn_cast<LibWriteLabel>(eLab))
				return wLab->getReadersList().empty();
			BUG();
		});

	/* Push all options that break RF functionality to the stack */
	for (auto it = invIt; it != validStores.end(); ++it) {
		const EventLabel *iLab = g.getEventLabel(*it);
		BUG_ON(!llvm::isa<LibWriteLabel>(iLab));

		auto *sLab = static_cast<const WriteLabel *>(iLab);
		const std::vector<Event> &readers = sLab->getReadersList();

		BUG_ON(readers.size() > 1);
		auto *rdLab = static_cast<const ReadLabel *>(
			g.getEventLabel(readers.back()));
		if (rdLab->isRevisitable())
			addToWorklist(SReadLibFunc, lab->getPos(), *it, {}, {});
	}

	/* If there is no valid RF, we have to read BOTTOM */
	if (invIt == validStores.begin()) {
		WARN_ONCE("lib-not-always-block", "FIXME: SHOULD NOT ALWAYS BLOCK -- ALSO IN EE\n");
		auto tempRf = Event::getInitializer();
		return std::make_pair(getWriteValue(tempRf, addr, typ), true);
	}

	/*
	 * If BOTTOM is not the only option, push it to inconsistent RFs as well,
	 * choose a valid store to read-from, and push the other alternatives to
	 * the stack
	 */
	WARN_ONCE("lib-check-before-push", "FIXME: CHECK IF IT'S A NON BLOCKING LIB BEFORE PUSHING?\n");
	g.addBottomToInvalidRfs(lab->getPos());
	g.changeRf(lab->getPos(), validStores[0]);

	for (auto it = validStores.begin() + 1; it != invIt; ++it)
		addToWorklist(SRead, lab->getPos(), *it, {}, {});

	return std::make_pair(getWriteValue(lab->getRf(), addr, typ),
			      lab->getRf().isInitializer());
}

void GenMCDriver::visitLibStore(llvm::Interpreter::InstAttr attr,
				llvm::AtomicOrdering ord,
				const llvm::GenericValue *addr,
				llvm::Type *typ,
				llvm::GenericValue &val,
				std::string functionName,
				bool isInit)
{
	auto &g = getGraph();
	auto &thr = EE->getCurThr();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);

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

	/* If there was not any store previously, check if this location was initialized.
	 * We only set a flag here and report an error after the relevant event is added   */
	bool isUninitialized = false;
	if (endO == 0 && !isInit)
		isUninitialized = true;

	/* It is always consistent to add a new event at the end of MO */
	const LibWriteLabel *sLab =
		g.addLibStoreToGraph(thr.id, ord, addr, typ, val, endO,
				     functionName, isInit);

	if (isUninitialized) {
		visitError(std::string("Uninitialized memory used by library \"") +
			   lib->getName() + "\", member \"" + functionName +
			   std::string("\" found"), Event::getInitializer(),
			   DE_UninitializedMem);
		BUG();
	}


	calcLibRevisits(sLab);

	if (lib && !lib->tracksCoherence())
		return;

	/*
	 * Check for alternative MO placings. Temporarily remove sLab from
	 * MO, find all possible alternatives, and push them to the workqueue
	 */
	locMO.pop_back();
	for (auto i = begO; i < endO; ++i) {
		locMO.insert(locMO.begin() + i, sLab->getPos());

		/* Check consistency for the graph with this MO */
		auto preds = g.getViewFromStamp(sLab->getStamp());
		if (g.isLibConsistentInView(*lib, preds))
			addToWorklist(MOWriteLib, sLab->getPos(),
				      Event::getInitializer(), {}, {}, i);
		locMO.erase(locMO.begin() + i);
	}
	locMO.push_back(sLab->getPos());
	return;
}

bool GenMCDriver::calcLibRevisits(const EventLabel *lab)
{
	auto &g = getGraph();
	auto valid = true; /* Suppose we are in a valid state */
	std::vector<Event> loads, stores;

	/* First, get the library of the event causing the revisit */
	const Library *lib = nullptr;
	if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab))
		lib = Library::getLibByMemberName(getGrantedLibs(), rLab->getFunctionName());
	else if (auto *wLab = llvm::dyn_cast<LibWriteLabel>(lab))
		lib = Library::getLibByMemberName(getGrantedLibs(), wLab->getFunctionName());
	else
		BUG();

	/*
	 * If this is a read of a functional library causing the revisit,
	 * then this is a functionality violation: we need to find the conflicting
	 * event, and find an alternative reads-from edge for it
	 */
	if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab)) {
		/* Since a read is causing a revisit, this has to be a functional lib */
		BUG_ON(!lib->hasFunctionalRfs());
		valid = false;
		Event conf = g.getPendingLibRead(rLab);
		loads = {conf};
		const EventLabel *confLab = g.getEventLabel(conf);
		if (auto *conflLab = llvm::dyn_cast<LibReadLabel>(confLab))
			stores = conflLab->getInvalidRfs();
		else
			BUG();
	} else if (auto *wLab = llvm::dyn_cast<LibWriteLabel>(lab)) {
		/* It is a normal store -- we need to find revisitable loads */
		loads = g.getRevisitable(wLab);
		stores = {wLab->getPos()};
	} else {
		BUG();
	}

	/* Next, find which of the 'stores' can be read by 'loads' */
	auto &before = g.getPorfBefore(lab->getPos());
	for (auto &l : loads) {
		const EventLabel *revLab = g.getEventLabel(l);
		BUG_ON(!llvm::isa<ReadLabel>(revLab));

		/* Get the read label to revisit */
		auto *rLab = static_cast<const ReadLabel *>(revLab);
		auto preds = g.getViewFromStamp(rLab->getStamp());

		/* Calculate the view of the resulting graph */
		auto v = preds;
		v.update(before);

		/* Check if this the resulting graph is consistent */
		auto rfs = g.getLibConsRfsInView(*lib, rLab->getPos(), stores, v);

		for (auto &rf : rfs) {
			/* Push consistent options to stack */
			auto writePrefix = g.getPrefixLabelsNotBefore(before, preds);
			auto moPlacings = (lib->tracksCoherence()) ? g.getMOPredsInBefore(writePrefix, preds)
				                                   : std::vector<std::pair<Event, Event> >();
			auto writePrefixPos = g.getRfsNotBefore(writePrefix, preds);
			writePrefixPos.insert(writePrefixPos.begin(), lab->getPos());

			if (g.revisitSetContains(rLab, writePrefixPos, moPlacings))
				continue;

			g.addToRevisitSet(rLab, writePrefixPos, moPlacings);
			addToWorklist(SRevisit, rLab->getPos(), rf,
				      std::move(writePrefix), std::move(moPlacings));

		}
	}
	return valid;
}


/************************************************************
 ** Printing facilities
 ***********************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s,
			      const GenMCDriver::DriverErrorKind &e)
{
	switch (e) {
	case GenMCDriver::DE_Safety:
		return s << "Safety violation";
	case GenMCDriver::DE_UninitializedMem:
		return s << "Uninitialized memory location";
	case GenMCDriver::DE_RaceNotAtomic:
		return s << "Non-Atomic race";
	case GenMCDriver::DE_RaceFreeMalloc:
		return s << "Malloc-Free race";
	case GenMCDriver::DE_FreeNonMalloc:
		return s << "Attempt to free non-allocated memory";
	case GenMCDriver::DE_AccessNonMalloc:
		return s << "Attempt to access non-allocated memory";
	case GenMCDriver::DE_AccessFreed:
		return s << "Attempt to access freed memory";
	case GenMCDriver::DE_DoubleFree:
		return s << "Double-free error";
	case GenMCDriver::DE_InvalidJoin:
		return s << "Invalid join() operation";
	default:
		return s << "Uknown error";
	}
}

#define IMPLEMENT_INTEGER_PRINT(OS, TY)			\
	case llvm::Type::IntegerTyID:			\
	        OS << val.IntVal;			\
		break;

#define IMPLEMENT_FLOAT_PRINT(OS, TY)			\
	case llvm::Type::FloatTyID:			\
	        OS << val.FloatVal;			\
		break;

#define IMPLEMENT_DOUBLE_PRINT(OS, TY)			\
	case llvm::Type::DoubleTyID:			\
	        OS << val.DoubleVal;			\
		break;

#define IMPLEMENT_VECTOR_INTEGER_PRINT(OS, TY)				\
	case llvm::Type::VectorTyID: {					\
		OS << "[";						\
		for (uint32_t _i=0;_i<val.AggregateVal.size();_i++) {	\
			OS << val.AggregateVal[_i].IntVal << " ";	\
		}							\
		OS << "]";						\
	} break;

#define IMPLEMENT_POINTER_PRINT(OS, TY)					\
	case llvm::Type::PointerTyID:					\
	        OS << (void*)(intptr_t)val.PointerVal;	\
		break;

static void executeGVPrint(const llvm::GenericValue &val, const llvm::Type *typ,
			   llvm::raw_ostream &s = llvm::dbgs())
{
	switch (typ->getTypeID()) {
		IMPLEMENT_INTEGER_PRINT(s, typ);
		IMPLEMENT_FLOAT_PRINT(s, typ);
		IMPLEMENT_DOUBLE_PRINT(s, typ);
		IMPLEMENT_VECTOR_INTEGER_PRINT(s, typ);
		IMPLEMENT_POINTER_PRINT(s, typ);
	default:
		WARN("Unhandled type for GVPrint predicate!\n");
		BUG();
	}
	return;
}

#define PRINT_AS_RF(s, e)			\
do {					        \
	if (e.isInitializer())			\
		s << "INIT";			\
	else					\
		s << e;				\
} while (0)

static void executeRLPrint(const ReadLabel *rLab,
			   const std::string &varName,
			   const llvm::GenericValue &val,
			   llvm::raw_ostream &s = llvm::dbgs())
{
	s << rLab->getPos() << ": ";
	s << rLab->getKind() << rLab->getOrdering()
	  << " (" << varName << ", ";
	executeGVPrint(val, rLab->getType(), s);
	s << ")";
	s << " [";
	PRINT_AS_RF(s, rLab->getRf());
	s << "]";
}

static void executeWLPrint(const WriteLabel *wLab,
			   const std::string &varName,
			   llvm::raw_ostream &s = llvm::dbgs())
{
	s << wLab->getPos() << ": ";
	s << wLab->getKind() << wLab->getOrdering()
	  << " (" << varName << ", ";
	executeGVPrint(wLab->getVal(), wLab->getType(), s);
	s << ")";
}

static void executeMDPrint(const EventLabel *lab,
			   const std::pair<int, std::string> &locAndFile,
			   std::string inputFile,
			   llvm::raw_ostream &os = llvm::dbgs())
{
	os << " L." << locAndFile.first;
	std::string errPath = locAndFile.second;
	Parser::stripSlashes(errPath);
	Parser::stripSlashes(inputFile);
	if (errPath != inputFile)
		os << ": " << errPath;
}

void GenMCDriver::printGraph(bool getMetadata /* false */)
{
	auto &g = getGraph();

	if (getMetadata)
		EE->replayExecutionBefore(g.getViewFromStamp(g.nextStamp()));
	for (auto i = 0u; i < g.events.size(); i++) {
		auto &thr = EE->getThrById(i);
		llvm::dbgs() << thr << ":\n";
		for (auto j = 0u; j < g.events[i].size(); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			llvm::dbgs() << "\t";
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				auto name = EE->getGlobalName(rLab->getAddr());
				auto val = getWriteValue(rLab->getRf(), rLab->getAddr(),
							 rLab->getType());
				executeRLPrint(rLab, name, val);
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				auto name = EE->getGlobalName(wLab->getAddr());
				executeWLPrint(wLab, name);
			} else {
				llvm::dbgs() << *lab;
			}
			if (getMetadata &&
			    !llvm::isa<ThreadStartLabel>(lab) &&
			    !llvm::isa<ThreadFinishLabel>(lab)) {
				executeMDPrint(lab, thr.prefixLOC[j], getConf()->inputFile);
			}
			llvm::dbgs() << "\n";
		}
	}
	llvm::dbgs() << "\n";
}

void GenMCDriver::prettyPrintGraph()
{
	auto &g = getGraph();
	for (auto i = 0u; i < g.events.size(); i++) {
		auto &thr = EE->getThrById(i);
		llvm::dbgs() << thr << ": ";
		for (auto j = 0u; j < g.events[i].size(); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->isRevisitable())
					llvm::dbgs().changeColor(llvm::raw_ostream::Colors::GREEN);
				auto val = getWriteValue(rLab->getRf(), rLab->getAddr(),
							 rLab->getType());
				llvm::dbgs() << "R" << EE->getGlobalName(rLab->getAddr()) << ","
					     << val.IntVal << " ";
				llvm::dbgs().resetColor();
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				llvm::dbgs() << "W" << EE->getGlobalName(wLab->getAddr()) << ","
					     << wLab->getVal().IntVal << " ";
			}
		}
		llvm::dbgs() << "\n";
	}
	llvm::dbgs() << "\n";
}

void GenMCDriver::dotPrintToFile(const std::string &filename,
				 Event errorEvent, Event confEvent)
{
	ExecutionGraph &g = getGraph();
	std::ofstream fout(filename);
	llvm::raw_os_ostream ss(fout);

	View before(g.getPorfBefore(errorEvent));
	if (!confEvent.isInitializer())
		before.update(g.getPorfBefore(confEvent));

	EE->replayExecutionBefore(before);

	/* Create a directed graph graph */
	ss << "strict digraph {\n";
	/* Specify node shape */
	ss << "\tnode [shape=box]\n";
	/* Left-justify labels for clusters */
	ss << "\tlabeljust=l\n";

	/* Print all nodes with each thread represented by a cluster */
	for (auto i = 0u; i < before.size(); i++) {
		auto &thr = EE->getThrById(i);
		ss << "subgraph cluster_" << thr.id << "{\n";
		ss << "\tlabel=\"" << thr.threadFun->getName().str() << "()\"\n";
		for (auto j = 1; j <= before[i]; j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));

			ss << "\t\"" << lab->getPos() << "\" [label=\"";

			/* First, print the graph label for this node */
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				auto name = EE->getGlobalName(rLab->getAddr());
				auto val = getWriteValue(rLab->getRf(), rLab->getAddr(),
							 rLab->getType());
				executeRLPrint(rLab, name, val, ss);
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				auto name = EE->getGlobalName(wLab->getAddr());
				executeWLPrint(wLab, name, ss);
			} else {
				ss << *lab;
			}

			/* And then, print the corresponding source-code line */
			ss << "\\n";
			Parser::parseInstFromMData(thr.prefixLOC[j], "", ss);

			ss << "\""
			   << (lab->getPos() == errorEvent  || lab->getPos() == confEvent ?
			       ",style=filled,fillcolor=yellow" : "")
			   << "]\n";
		}
		ss << "}\n";
	}

	/* Print relations between events (po U rf) */
	for (auto i = 0u; i < before.size(); i++) {
		auto &thr = EE->getThrById(i);
		for (auto j = 0; j <= before[i]; j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));

			/* Print a po-edge, but skip dummy start events for
			 * all threads except for the first one */
			if (j < before[i] && !(llvm::isa<ThreadStartLabel>(lab)))
				ss << "\"" << lab->getPos() << "\" -> \""
				   << lab->getPos().next() << "\"\n";
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				/* Do not print RFs from the INIT event */
				if (!rLab->getRf().isInitializer()) {
					ss << "\t\"" << rLab->getRf() << "\" -> \""
					   << rLab->getPos() << "\"[color=green]\n";
				}
			}
			if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab)) {
				if (thr.id == 0)
					continue;
				ss << "\t\"" << bLab->getParentCreate() << "\" -> \""
				   << bLab->getPos().next() << "\"[color=blue]\n";
			}
			if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab))
				ss << "\t\"" << jLab->getChildLast() << "\" -> \""
				   << jLab->getPos() << "\"[color=blue]\n";
		}
	}

	ss << "}\n";
}

void GenMCDriver::recPrintTraceBefore(const Event &e, View &a,
				      llvm::raw_ostream &ss /* llvm::dbgs() */)
{
	auto &g = getGraph();

	if (a.contains(e))
		return;

	auto ai = a[e.thread];
	a[e.thread] = e.index;
	auto &thr = EE->getThrById(e.thread);
	for (int i = ai; i <= e.index; i++) {
		const EventLabel *lab = g.getEventLabel(Event(e.thread, i));
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
			recPrintTraceBefore(rLab->getRf(), a, ss);
		if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab))
			recPrintTraceBefore(jLab->getChildLast(), a, ss);
		if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab))
			if (!bLab->getParentCreate().isInitializer())
				recPrintTraceBefore(bLab->getParentCreate(), a, ss);

		/* Do not print the line if it is an RMW write, since it will
		 * be the same as the previous one */
		if (llvm::isa<CasWriteLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
			continue;
		Parser::parseInstFromMData(thr.prefixLOC[i], thr.threadFun->getName().str(), ss);
	}
	return;
}

void GenMCDriver::printTraceBefore(Event e)
{
	View before(getGraph().getPorfBefore(e));

	llvm::dbgs() << "Trace to " << e << ":\n";

	/* Replay the execution up to the error event (collects mdata) */
	EE->replayExecutionBefore(before);

	/* Linearizate (po U rf) and print trace */
	View a;
	recPrintTraceBefore(e, a);
}
