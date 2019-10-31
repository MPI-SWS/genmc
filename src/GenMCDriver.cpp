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
#include "GraphBuilder.hpp"
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
	  isMootExecution(false), prioritizeThread(-1),
	  explored(0), exploredBlocked(0), duplicates(0), start(start)
{
	/* Register a signal handler for abort() */
	std::signal(SIGABRT, abortHandler);

	/* Set up an appropriate execution graph */
	execGraph = GraphBuilder(userConf->isDepTrackingModel)
		.withCoherenceType(userConf->coherence).build();

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
	const auto &g = getGraph();

	/* Set-up (optimize) the interpreter for the new exploration */
	for (auto i = 1u; i < g.getNumThreads(); i++) {

		/* Skip not-yet-created threads */
		BUG_ON(g.isThreadEmpty(i));

		const EventLabel *lab = g.getEventLabel(Event(i, 0));
		BUG_ON(!llvm::isa<ThreadStartLabel>(lab));

		auto *labFst = static_cast<const ThreadStartLabel *>(lab);
		Event parent = labFst->getParentCreate();

		/* Skip if parent create does not exist yet */
		if (parent.index >= (int) g.getThreadSize(parent.thread))
			continue;

		/* This could fire for the main() thread */
		BUG_ON(!llvm::isa<ThreadCreateLabel>(g.getEventLabel(parent)));

		/* Skip finished threads */
		const EventLabel *labLast = g.getLastThreadLabel(i);
		if (llvm::isa<ThreadFinishLabel>(labLast))
			continue;

		/* Otherwise, initialize ECStacks in interpreter */
		auto &thr = getEE()->getThrById(i);
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
	if (std::any_of(getEE()->threads.begin(), getEE()->threads.end(),
			[](llvm::Thread &thr){ return thr.isBlocked; })) {
		++exploredBlocked;
		return;
	}

	const auto &g = getGraph();
	if (userConf->checkPscAcyclicity != CheckPSCType::nocheck &&
	    !g.isPscAcyclic(userConf->checkPscAcyclicity))
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
	llvm::VariableInfo VI;

	LLVMModule::transformLLVMModule(*mod, VI, userConf->spinAssume, userConf->unroll);
	if (userConf->transformFile != "")
		LLVMModule::printLLVMModule(*mod, userConf->transformFile);

	/* Create an interpreter for the program's instructions. */
	EE = (llvm::Interpreter *)
	     llvm::Interpreter::create(&*mod, std::move(VI), this, userConf->isDepTrackingModel, &buf);

	/* Create main thread and start event */
	auto mainFun = mod->getFunction(userConf->programEntryFun);
	if (!mainFun) {
		WARN("ERROR: Could not find program's entry point function!\n");
		abort();
	}
	auto main = EE->createMainThread(mainFun);
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
	const auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(e);
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

void GenMCDriver::restrictWorklist(const EventLabel *rLab)
{
	auto stamp = rLab->getStamp();
	std::vector<int> idxsToRemove;

	for (auto rit = workqueue.rbegin(); rit != workqueue.rend(); ++rit)
		if (rit->first > stamp && rit->second.empty())
			idxsToRemove.push_back(rit->first); // TODO: break out of loop?

	for (auto &i : idxsToRemove)
		workqueue.erase(i);
}

void GenMCDriver::restrictGraph(const EventLabel *rLab)
{
	const auto &g = getGraph();
	auto v = g.getPredsView(rLab->getPos());

	/* First, free memory allocated by events that will no longer be in the graph */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (lab->getStamp() <= rLab->getStamp())
				continue;
			if (auto *mLab = llvm::dyn_cast<MallocLabel>(lab))
				getEE()->deallocateBlock(mLab->getAllocAddr(),
							 mLab->getAllocSize(),
							 mLab->isLocal());
		}
	}

	/* Then, restrict the graph */
	getGraph().cutToStamp(rLab->getStamp());
	return;
}

/*
 * Restore the part of the SBRF-prefix of the store that revisits a load,
 * that is not already present in the graph, the MO edges between that
 * part and the previous MO, and make that part non-revisitable
 */
void GenMCDriver::restorePrefix(const EventLabel *lab,
				std::vector<std::unique_ptr<EventLabel> > &&prefix,
				std::vector<std::pair<Event, Event> > &&moPlacings)
{
	const auto &g = getGraph();
	BUG_ON(!llvm::isa<ReadLabel>(lab));
	auto *rLab = static_cast<const ReadLabel *>(lab);

	/* Re-allocate memory for the allocation events in the prefix */
	for (auto &lab : prefix) {
		if (auto *mLab = llvm::dyn_cast<MallocLabel>(&*lab))
			getEE()->allocateBlock(mLab->getAllocAddr(),
					       mLab->getAllocSize(),
					       mLab->isLocal());
	}

	getGraph().restoreStorePrefix(rLab, prefix, moPlacings);
}


/************************************************************
 ** Scheduling methods
 ***********************************************************/

bool GenMCDriver::scheduleNext()
{
	if (isMootExecution)
		return false;

	const auto &g = getGraph();
	auto *EE = getEE();

	/* First, check if we should prioritize some thread */
	if (0 <= prioritizeThread && prioritizeThread < (int) g.getNumThreads()) {
		auto &thr = EE->getThrById(prioritizeThread);
		if (!thr.ECStack.empty() && !thr.isBlocked &&
		    !llvm::isa<ThreadFinishLabel>(g.getLastThreadLabel(prioritizeThread))) {
			EE->currentThread = prioritizeThread;
			return true;
		}
	}
	prioritizeThread = -2;


	/* Check if randomize scheduling is enabled and schedule some thread */
	MyDist dist(0, g.getNumThreads());
	auto random = (userConf->randomizeSchedule) ? dist(rng) : 0;
	for (auto j = 0u; j < g.getNumThreads(); j++) {
		auto i = (j + random) % g.getNumThreads();
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
	auto *EE = getEE();

	while (true) {
		EE->reset();

		/* Get main program function and run the program */
		EE->runStaticConstructorsDestructors(false);
		EE->runFunctionAsMain(mod->getFunction(userConf->programEntryFun), {"prog"}, nullptr);
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
	const auto &g = getGraph();
	auto &thr = getEE()->getCurThr();
	auto curr = Event(thr.id, ++thr.globalInstructions);

	return (curr.index < g.getThreadSize(curr.thread)) &&
		!llvm::isa<EmptyLabel>(g.getEventLabel(curr));
}

const EventLabel *GenMCDriver::getCurrentLabel() const
{
	const auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();

	BUG_ON(pos.index >= g.getThreadSize(pos.thread));
	return g.getEventLabel(pos);
}

/* Given an event in the graph, returns the value of it */
llvm::GenericValue GenMCDriver::getWriteValue(Event write,
					      const llvm::GenericValue *ptr,
					      const llvm::Type *typ)
{
	/* If the even represents an invalid access, return some value */
	if (write.isBottom())
		return llvm::GenericValue();

	/* If the event is the initializer, ask the interpreter about
	 * the initial value of that memory location */
	if (write.isInitializer())
		return getEE()->getLocInitVal((llvm::GenericValue *)ptr,
					      (llvm::Type *) typ);

	/* Otherwise, we will get the value from the execution graph */
	const EventLabel *lab = getGraph().getEventLabel(write);
	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);

	/* If the type of the R and the W are the same, we are done */
	if (wLab->getType() == typ)
		return wLab->getVal();

	/* Otherwise, make sure that we return a value of the expected type.
	 * (Required because of how CAS produces code in LLVM -- see troep.c) */
	llvm::GenericValue result;
	if (typ->isIntegerTy() && wLab->getType()->isPointerTy()) {
		result.IntVal = llvm::APInt(
			getEE()->getTypeSize((llvm::Type *) wLab->getType()) * 8,
			(uint64_t) wLab->getVal().PointerVal);
	} else if (typ->isPointerTy() && wLab->getType()->isIntegerTy()) {
		result.PointerVal = (void *) wLab->getVal().IntVal.getZExtValue();
	} else {
		BUG();
	}
	return result;
}

void GenMCDriver::findMemoryRaceForMemAccess(const MemAccessLabel *mLab)
{
	const auto &g = getGraph();
	const View &before = g.getHbBefore(mLab->getPos().prev());
	for (auto i = 0u; i < g.getNumThreads(); i++)
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *oLab = g.getEventLabel(Event(i, j));
			if (auto *fLab = llvm::dyn_cast<FreeLabel>(oLab)) {
				if (fLab->getFreedAddr() == mLab->getAddr()) {
					visitError("", oLab->getPos(), DE_AccessFreed);
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

void GenMCDriver::findMemoryRaceForAllocAccess(const FreeLabel *fLab)
{
	const MallocLabel *m = nullptr; /* There must be a malloc() before the free() */
	const auto &g = getGraph();
	auto *ptr = fLab->getFreedAddr();
	auto &before = g.getHbBefore(fLab->getPos());
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
				if (aLab->getAllocAddr() == ptr &&
				    before.contains(aLab->getPos())) {
					m = aLab;
				}
			}
			if (auto *dLab = llvm::dyn_cast<FreeLabel>(lab)) {
				if (dLab->getFreedAddr() == ptr &&
				    dLab->getPos() != fLab->getPos()) {
					visitError("", dLab->getPos(), DE_DoubleFree);
					BUG();
				}
			}
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
				if (mLab->getAddr() == ptr &&
				    !before.contains(mLab->getPos())) {
					visitError("", mLab->getPos(), DE_AccessFreed);
					BUG();
				}

			}
		}
	}

	if (!m) {
		visitError("", Event::getInitializer(), DE_FreeNonMalloc);
		BUG(); /* visitError() should abort */
	}
	return;
}

void GenMCDriver::checkForMemoryRaces(const void *addr)
{
	if (userConf->disableRaceDetection)
		return;
	if (!getEE()->isHeapAlloca(addr))
		return;

	const EventLabel *lab = getCurrentLabel();
	BUG_ON(!llvm::isa<MemAccessLabel>(lab) && !llvm::isa<FreeLabel>(lab));

	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab))
		findMemoryRaceForMemAccess(mLab);
	else if (auto *fLab = llvm::dyn_cast<FreeLabel>(lab))
		findMemoryRaceForAllocAccess(fLab);
	return;
}

/*
 * This function is called to check for data races when a new event is added.
 * When a race is detected visit error is called, which will report an error
 * if the execution is valid. This method is memory-model specific since
 * the concept of a "race" (e.g., as in (R)C11) may not be defined on all
 * models, and thus relies on a virtual method.
 */
void GenMCDriver::checkForDataRaces()
{
	if (userConf->disableRaceDetection)
		return;

	/* We only check for data races when reads and writes are added */
	const EventLabel *lab = getCurrentLabel();
	BUG_ON(!llvm::isa<MemAccessLabel>(lab));

	auto racy = findDataRaceForMemAccess(static_cast<const MemAccessLabel *>(lab));

	/* If a race is found and the execution is consistent, return it */
	if (!racy.isInitializer()) {
		visitError("", racy, DE_RaceNotAtomic);
		BUG(); /* visitError() should abort in any case */
	}
	return;
}

void GenMCDriver::checkAccessValidity()
{
	const EventLabel *lab = getCurrentLabel();

	/* Should only be called with reads and writes */
	BUG_ON(!llvm::isa<MemAccessLabel>(lab));

	auto *mLab = static_cast<const MemAccessLabel *>(lab);
	if (!getEE()->isShared(mLab->getAddr())) {
		visitError("", Event::getInitializer(),
			   DE_AccessNonMalloc);
		BUG();
	}
	return;
}

std::vector<Event>
GenMCDriver::getLibConsRfsInView(const Library &lib, Event read,
				 const std::vector<Event> &stores,
				 const View &v)
{
	EventLabel *lab = getGraph().getEventLabel(read);
	BUG_ON(!llvm::isa<LibReadLabel>(lab));

	auto *rLab = static_cast<ReadLabel *>(lab);
	Event oldRf = rLab->getRf();
	std::vector<Event> filtered;

	for (auto &s : stores) {
		changeRf(rLab->getPos(), s);
		if (getGraph().isLibConsistentInView(lib, v))
			filtered.push_back(s);
	}
	/* Restore the old reads-from, and eturn all the valid reads-from options */
	changeRf(rLab->getPos(), oldRf);
	return filtered;
}

std::vector<Event> GenMCDriver::filterAcquiredLocks(const llvm::GenericValue *ptr,
						    const std::vector<Event> &stores,
						    const VectorClock &before)
{
	const auto &g = getGraph();
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

	const auto &g = getGraph();
	auto curr = getEE()->getCurrentPosition().prev();
	auto &before = g.getPrefixView(curr);

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

	result.IntVal = llvm::APInt(typ->getIntegerBitWidth(), getEE()->getCurThr().id);
	return result;
}

int GenMCDriver::visitThreadCreate(llvm::Function *calledFun, const llvm::ExecutionContext &SF)
{
	const auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph()) {
		auto *cLab = static_cast<const ThreadCreateLabel *>(getCurrentLabel());
		return cLab->getChildId();
	}

	Event cur = EE->getCurrentPosition();
	int cid = 0;

	/* First, check if the thread to be created already exists */
	while (cid < (int) g.getNumThreads()) {
		if (!g.isThreadEmpty(cid)) {
			if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(
				    g.getEventLabel(Event(cid, 0)))) {
				if (bLab->getParentCreate() == cur)
					break;
			}
		}
		++cid;
	}

	/* Add an event for the thread creation */
	auto tcLab = createTCreateLabel(cur.thread, cur. index, cid);
	getGraph().addOtherLabelToGraph(std::move(tcLab));

	/* Prepare the execution context for the new thread */
	llvm::Thread thr = EE->createNewThread(calledFun, cid, cur.thread, SF);

	if (cid == (int) g.getNumThreads()) {
		/* If the thread does not exist in the graph, make an entry for it */
		EE->threads.push_back(thr);
		getGraph().addNewThread();
		auto tsLab = createStartLabel(cid, 0, cur);
		getGraph().addOtherLabelToGraph(std::move(tsLab));
	} else {
		/* Otherwise, just push the execution context to the interpreter */
		EE->threads[cid] = thr;
	}
	return cid;
}

llvm::GenericValue GenMCDriver::visitThreadJoin(llvm::Function *F, const llvm::GenericValue &arg)
{
	const auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	int cid = arg.IntVal.getLimitedValue(std::numeric_limits<int>::max());
	if (cid < 0 || int (EE->threads.size()) <= cid || cid == thr.id) {
		std::string err = "ERROR: Invalid TID in pthread_join(): " + std::to_string(cid);
		if (cid == thr.id)
			err += " (TID cannot be the same as the calling thread)";
		visitError(err, Event::getInitializer(), DE_InvalidJoin);
	}

	/* If necessary, add a relevant event to the graph */
	if (!isExecutionDrivenByGraph()) {
		auto jLab = createTJoinLabel(thr.id, thr.globalInstructions, cid);
		getGraph().addOtherLabelToGraph(std::move(jLab));
	}

	/* If the update failed (child has not terminated yet) block this thread */
	if (!updateJoin(getEE()->getCurrentPosition(), g.getLastThreadEvent(cid)))
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
	const auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	if (!isExecutionDrivenByGraph() && /* Make sure that there is not a failed assume... */
	    !thr.isBlocked) {
		auto eLab = createFinishLabel(thr.id, thr.globalInstructions);
		getGraph().addOtherLabelToGraph(std::move(eLab));

		if (thr.id == 0)
			return;

		const EventLabel *lab = g.getLastThreadLabel(thr.id);
		for (auto i = 0u; i < g.getNumThreads(); i++) {
			const EventLabel *pLastLab = g.getLastThreadLabel(i);
			if (auto *pLab = llvm::dyn_cast<ThreadJoinLabel>(pLastLab)) {
				if (pLab->getChildId() != thr.id)
					continue;

				/* If parent thread is waiting for me, relieve it */
				EE->getThrById(i).unblock();
				updateJoin(pLab->getPos(), lab->getPos());
			}
		}
	} /* FIXME: Maybe move view update into thread finish creation? */
	  /* FIXME: Thread return values? */
}

void GenMCDriver::visitFence(llvm::AtomicOrdering ord)
{
	if (isExecutionDrivenByGraph())
		return;

	auto pos = getEE()->getCurrentPosition();
	auto fLab = createFenceLabel(pos.thread, pos.index, ord);
	getGraph().addOtherLabelToGraph(std::move(fLab));
	return;
}

llvm::GenericValue
GenMCDriver::visitLoad(llvm::Interpreter::InstAttr attr,
		       llvm::AtomicOrdering ord,
		       const llvm::GenericValue *addr,
		       llvm::Type *typ,
		       llvm::GenericValue cmpVal,
		       llvm::GenericValue rmwVal,
		       llvm::AtomicRMWInst::BinOp op)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	if (isExecutionDrivenByGraph()) {
		const EventLabel *lab = getCurrentLabel();
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			return getWriteValue(rLab->getRf(), addr, typ);
		}
		BUG();
	}

	/* Make the graph aware of a (potentially) new memory location */
	g.trackCoherenceAtLoc(addr);

	/* Get all stores to this location from which we can read from */
	auto stores = getStoresToLoc(addr);
	BUG_ON(stores.empty());
	auto validStores = properlyOrderStores(attr, typ, addr, cmpVal, stores);

	/* ... and add an appropriate label with a particular rf */
	Event pos = getEE()->getCurrentPosition();
	std::unique_ptr<ReadLabel> rLab = nullptr;
	switch (attr) {
	case llvm::Interpreter::IA_None:
		rLab = std::move(createReadLabel(pos.thread, pos.index, ord,
						 addr, typ, validStores[0]));
		break;
	case llvm::Interpreter::IA_Fai:
		rLab = std::move(createFaiReadLabel(pos.thread, pos.index, ord,
						    addr, typ, validStores[0],
						    op, std::move(rmwVal)));
		break;
	case llvm::Interpreter::IA_Cas:
	case llvm::Interpreter::IA_Lock:
		rLab = std::move(createCasReadLabel(pos.thread, pos.index, ord, addr, typ,
						    validStores[0], cmpVal, rmwVal,
						    attr == llvm::Interpreter::IA_Lock));
		break;
	default:
		BUG();
	}

	const ReadLabel *lab = g.addReadLabelToGraph(std::move(rLab), validStores[0]);

	/* Check whether a valid address is accessed, and whether there are races */
	checkAccessValidity();
	checkForDataRaces();
	checkForMemoryRaces(lab->getAddr());

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
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto *EE = getEE();
	auto pos = EE->getCurrentPosition();

	g.trackCoherenceAtLoc(addr);
	auto placesRange =
		g.getCoherentPlacings(addr, pos, attr != llvm::Interpreter::IA_None &&
				      attr != llvm::Interpreter::IA_Unlock);
	auto &begO = placesRange.first;
	auto &endO = placesRange.second;

	/* It is always consistent to add the store at the end of MO */
	std::unique_ptr<WriteLabel> wLab = nullptr;
	switch (attr) {
	case llvm::Interpreter::IA_None:
	case llvm::Interpreter::IA_Unlock:
		wLab = std::move(createStoreLabel(pos.thread, pos.index, ord,
						  addr, typ, val,
						  attr == llvm::Interpreter::IA_Unlock));
		break;
	case llvm::Interpreter::IA_Fai:
		wLab = std::move(createFaiStoreLabel(pos.thread, pos.index, ord,
						     addr, typ, val));
		break;
	case llvm::Interpreter::IA_Cas:
	case llvm::Interpreter::IA_Lock:
		wLab = std::move(createCasStoreLabel(pos.thread, pos.index, ord,
						     addr, typ, val,
						     attr == llvm::Interpreter::IA_Lock));
		break;
	}

	const WriteLabel *lab = g.addWriteLabelToGraph(std::move(wLab), endO);

	/* Check whether a valid address is accessed, and whether there are races */
	checkAccessValidity();
	checkForDataRaces();
	checkForMemoryRaces(lab->getAddr());

	auto &locMO = g.getStoresToLoc(addr);
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

llvm::GenericValue GenMCDriver::visitMalloc(uint64_t allocSize, bool isLocal /* false */)
{
	auto &g = getGraph();
	auto *EE = getEE();
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

	WARN_ON_ONCE(allocSize > 64, "malloc-alignment",
		     "malloc()'s alignment larger than 64-bit! Limiting...\n");

	/* Get a fresh address and also track this allocation */
	allocBegin.PointerVal = EE->getFreshAddr(allocSize, isLocal);

	/* Add a relevant label to the graph and return the new address */
	Event pos = EE->getCurrentPosition();
	auto aLab = createMallocLabel(pos.thread, pos.index, allocBegin.PointerVal,
				      allocSize, isLocal);
	g.addOtherLabelToGraph(std::move(aLab));
	return allocBegin;
}

void GenMCDriver::visitFree(llvm::GenericValue *ptr)
{
	const auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph())
		return;

	/* Attempt to free a NULL pointer */
	if (ptr == NULL)
		return;

	/* Add a label with the appropriate store */
	Event pos = EE->getCurrentPosition();
	auto dLab = createFreeLabel(pos.thread, pos.index, ptr);
	getGraph().addOtherLabelToGraph(std::move(dLab));

	/* Check whether there is any memory race */
	checkForMemoryRaces(ptr);
	return;
}

void GenMCDriver::visitError(std::string err, Event confEvent,
			     DriverErrorKind t /* = DE_Safety */ )
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	/* If the execution that led to the error is not consistent, block */
	if (!isExecutionValid()) {
		getEE()->ECStack().clear();
		thr.block();
		return;
	}

	const EventLabel *errLab = getCurrentLabel();

	/* If this is an invalid access, change the RF of the offending
	 * event to BOTTOM, so that we do not try to get its value.
	 * Don't bother updating the views */
	if (isInvalidAccessError(t) && llvm::isa<ReadLabel>(errLab))
		g.changeRf(errLab->getPos(), Event::getBottom());

	/* Print a basic error message and the graph */
	llvm::dbgs() << "Error detected: " << t << "!\n";
	llvm::dbgs() << "Event " << errLab->getPos() << " ";
	if (!confEvent.isInitializer())
		llvm::dbgs() << "conflicts with event " << confEvent << " ";
	llvm::dbgs() << "in graph:\n";
	printGraph(true);

	/* Print error trace leading up to the violating event(s) */
	if (userConf->printErrorTrace) {
		printTraceBefore(errLab->getPos());
		if (!confEvent.isInitializer())
			printTraceBefore(confEvent);
	}

	/* Print the specific error message */
	if (!err.empty())
		llvm::dbgs() << err << "\n";

	/* Dump the graph into a file (DOT format) */
	if (userConf->dotFile != "")
		dotPrintToFile(userConf->dotFile, errLab->getPos(), confEvent);

	/* Print results and abort */
	printResults();
	abort();
}

bool GenMCDriver::tryToRevisitLock(const CasReadLabel *rLab, const WriteLabel *sLab,
				   const std::vector<Event> &writePrefixPos,
				   const std::vector<std::pair<Event, Event> > &moPlacings)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (!g.revisitModifiesGraph(rLab, sLab) &&
	    !g.revisitSetContains(rLab, writePrefixPos, moPlacings)) {
		EE->getThrById(rLab->getThread()).unblock();

		changeRf(rLab->getPos(), sLab->getPos());

		completeRevisitedRMW(rLab);

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
	const auto &g = getGraph();
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

		/* Get the prefix of the write to save */
		auto writePrefix = g.getPrefixLabelsNotBefore(sLab, rLab);
		auto moPlacings = g.saveCoherenceStatus(writePrefix, rLab);

		auto writePrefixPos = g.extractRfs(writePrefix);
		writePrefixPos.insert(writePrefixPos.begin(), sLab->getPos());

		/* Optimize handling of lock operations */
		if (auto *lLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
			if (lLab->isLock() && getEE()->getThrById(lLab->getThread()).isBlocked &&
			    (int) g.getThreadSize(lLab->getThread()) == lLab->getIndex() + 1) {
				if (tryToRevisitLock(lLab, sLab, writePrefixPos, moPlacings))
					continue;
				isMootExecution = true;
			}
		}

		/* If this prefix has revisited the read before, skip */
		if (g.revisitSetContains(rLab, writePrefixPos, moPlacings))
			continue;

		/* Otherwise, add the prefix to the revisit set and the worklist */
		getGraph().addToRevisitSet(rLab, writePrefixPos, moPlacings);
		addToWorklist(SRevisit, rLab->getPos(), sLab->getPos(),
			      std::move(writePrefix), std::move(moPlacings));
	}

	bool consG = !(llvm::isa<CasWriteLabel>(sLab) || llvm::isa<FaiWriteLabel>(sLab)) ||
		pendingRMWs.empty();
	return !isMootExecution && consG;
}

const WriteLabel *GenMCDriver::completeRevisitedRMW(const ReadLabel *rLab)
{
	const auto &g = getGraph();
	auto *EE = getEE();
	auto rfVal = getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());

	const WriteLabel *sLab = nullptr;
	std::unique_ptr<WriteLabel> wLab = nullptr;
	if (auto *faiLab = llvm::dyn_cast<FaiReadLabel>(rLab)) {
		llvm::GenericValue result;
		EE->executeAtomicRMWOperation(result, rfVal, faiLab->getOpVal(),
					      faiLab->getOp());
		EE->setCurrentDeps(nullptr, nullptr, nullptr, nullptr, nullptr);
		wLab = std::move(createFaiStoreLabel(faiLab->getThread(),
						     faiLab->getIndex() + 1,
						     faiLab->getOrdering(),
						     faiLab->getAddr(),
						     faiLab->getType(),
						     result));
	} else if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		if (EE->compareValues(casLab->getType(), casLab->getExpected(), rfVal)) {
			EE->setCurrentDeps(nullptr, nullptr, nullptr, nullptr, nullptr);
			wLab = std::move(createCasStoreLabel(casLab->getThread(),
							     casLab->getIndex() + 1,
							     casLab->getOrdering(),
							     casLab->getAddr(),
							     casLab->getType(),
							     casLab->getSwapVal(),
							     casLab->isLock()));
		}
	}
	if (wLab)
		return getGraph().addWriteLabelToGraph(std::move(wLab), rLab->getRf());
	return nullptr;
}

bool GenMCDriver::revisitReads(StackItem &p)
{
	const auto &g = getGraph();
	auto *EE = getEE();
	const EventLabel *lab = g.getEventLabel(p.toRevisit);

	/* First, appropriately restrict the worklist and the graph */
	restrictWorklist(lab);
	restrictGraph(lab);

	if (p.type == SRevisit)
		restorePrefix(lab, std::move(p.writePrefix), std::move(p.moPlacings));

	/* Finally, appropriately modify the graph:
	 * 1) If we are restricting to a write, then change its MO position */
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
		BUG_ON(p.type != MOWrite && p.type != MOWriteLib);
		getGraph().changeStoreOffset(wLab->getAddr(), wLab->getPos(), p.moPos);
		return (p.type == MOWrite) ? calcRevisits(wLab) : calcLibRevisits(wLab);
	}

	/* 2) If we are dealing with a read, change its reads-from,
	 * and check whether a part of an RMW should be added */
	BUG_ON(!llvm::isa<ReadLabel>(lab));
	auto *rLab = static_cast<const ReadLabel *>(lab);

	getEE()->setCurrentDeps(nullptr, nullptr, nullptr, nullptr, nullptr);
	changeRf(rLab->getPos(), p.shouldRf);

	/* If the revisited label became an RMW, add the store part and revisit */
	if (auto *sLab = completeRevisitedRMW(rLab))
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
	const auto &g = getGraph();
	auto &thr = getEE()->getCurThr();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);

	if (isExecutionDrivenByGraph()) {
		const EventLabel *lab = getCurrentLabel();
		if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab))
			return std::make_pair(getWriteValue(rLab->getRf(), addr, typ),
					      rLab->getRf().isInitializer());
		BUG();
	}

	getGraph().trackCoherenceAtLoc(addr);

	/* Add the event to the graph so we'll be able to calculate consistent RFs */
	Event pos = getEE()->getCurrentPosition();
	auto lLab = createLibReadLabel(pos.thread, pos.index, ord, addr, typ,
				       Event::getInitializer(), functionName);
	auto *lab = getGraph().addReadLabelToGraph(std::move(lLab), Event::getInitializer());

	/* Make sure there exists an initializer event for this memory location */
	auto stores(g.getStoresToLoc(addr));
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
	auto validStores = getLibConsRfsInView(*lib, lab->getPos(), stores, preds);

	/*
	 * If this is a non-functional library, choose one of the available reads-from
	 * options, push the rest to the stack, and return an appropriate value to
	 * the interpreter
	 */
	if (!lib->hasFunctionalRfs()) {
		BUG_ON(validStores.empty());
		changeRf(lab->getPos(), validStores[0]);
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
	getGraph().addInvalidRfs(lab->getPos(), invalid);

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
	getGraph().addBottomToInvalidRfs(lab->getPos());
	changeRf(lab->getPos(), validStores[0]);

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
	const auto &g = getGraph();
	auto &thr = getEE()->getCurThr();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);

	if (isExecutionDrivenByGraph())
		return;

	getGraph().trackCoherenceAtLoc(addr);

	/*
	 * We need to try all possible MO placements, but after the initialization write,
	 * which is explicitly included in MO, in the case of libraries.
	 */
	auto begO = 1;
	auto endO = getGraph().getStoresToLoc(addr).size();

	/* If there was not any store previously, check if this location was initialized.
	 * We only set a flag here and report an error after the relevant event is added   */
	bool isUninitialized = false;
	if (endO == 0 && !isInit)
		isUninitialized = true;

	/* It is always consistent to add a new event at the end of MO */
	Event pos = getEE()->getCurrentPosition();
	auto lLab = createLibStoreLabel(pos.thread, pos.index, ord, addr, typ,
					val, functionName, isInit);
	auto *sLab = getGraph().addWriteLabelToGraph(std::move(lLab), endO);

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
	const auto &locMO = getGraph().getStoresToLoc(addr);
	auto oldOffset = std::find(locMO.begin(), locMO.end(), sLab->getPos()) - locMO.begin();
	for (auto i = begO; i < endO; ++i) {
		getGraph().changeStoreOffset(addr, sLab->getPos(), i);

		/* Check consistency for the graph with this MO */
		auto preds = g.getViewFromStamp(sLab->getStamp());
		if (getGraph().isLibConsistentInView(*lib, preds)) {
			addToWorklist(MOWriteLib, sLab->getPos(),
				      Event::getInitializer(), {}, {}, i);
		}
	}
	getGraph().changeStoreOffset(addr, sLab->getPos(), oldOffset);
	return;
}

bool GenMCDriver::calcLibRevisits(const EventLabel *lab)
{
	const auto &g = getGraph();
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
		auto rfs = getLibConsRfsInView(*lib, rLab->getPos(), stores, v);

		for (auto &rf : rfs) {
			/* Push consistent options to stack */
			auto writePrefix = g.getPrefixLabelsNotBefore(lab, rLab);
			auto moPlacings = g.saveCoherenceStatus(writePrefix, rLab);

			auto writePrefixPos = g.extractRfs(writePrefix);
			writePrefixPos.insert(writePrefixPos.begin(), lab->getPos());

			if (g.revisitSetContains(rLab, writePrefixPos, moPlacings))
				continue;

			getGraph().addToRevisitSet(rLab, writePrefixPos, moPlacings);
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
	case GenMCDriver::DE_RaceNotAtomic:
		return s << "Non-Atomic race";
	case GenMCDriver::DE_RaceFreeMalloc:
		return s << "Malloc-Free race";
	case GenMCDriver::DE_FreeNonMalloc:
		return s << "Attempt to free non-allocated memory";
	case GenMCDriver::DE_DoubleFree:
		return s << "Double-free error";
	case GenMCDriver::DE_UninitializedMem:
		return s << "Uninitialized memory location";
	case GenMCDriver::DE_AccessNonMalloc:
		return s << "Attempt to access non-allocated memory";
	case GenMCDriver::DE_AccessFreed:
		return s << "Attempt to access freed memory";

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
	else if (e.isBottom())			\
		s << "BOTTOM";			\
	else					\
		s << e ;			\
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

/* Returns true if the corresponding LOC should be printed for this label type */
bool shouldPrintLOC(const EventLabel *lab)
{
	/* Begin/End labels don't have a corresponding LOC */
	if (llvm::isa<ThreadStartLabel>(lab) ||
	    llvm::isa<ThreadFinishLabel>(lab))
		return false;

	/* Similarly for allocas() (stack variables) */
	if (auto *mLab = llvm::dyn_cast<MallocLabel>(lab))
		return !mLab->isLocal();

	return true;
}

void GenMCDriver::printGraph(bool getMetadata /* false */)
{
	auto &g = getGraph();

	if (getMetadata)
		EE->replayExecutionBefore(g.getViewFromStamp(g.nextStamp()));

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);
		llvm::dbgs() << thr << ":\n";
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			llvm::dbgs() << "\t";
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				auto name = EE->getVarName(rLab->getAddr());
				auto val = getWriteValue(rLab->getRf(), rLab->getAddr(),
							 rLab->getType());
				executeRLPrint(rLab, name, val);
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				auto name = EE->getVarName(wLab->getAddr());
				executeWLPrint(wLab, name);
			} else {
				llvm::dbgs() << *lab;
			}
			if (getMetadata && shouldPrintLOC(lab)) {
				executeMDPrint(lab, thr.prefixLOC[j], getConf()->inputFile);
			}
			llvm::dbgs() << "\n";
		}
	}
	llvm::dbgs() << "\n";
}

void GenMCDriver::prettyPrintGraph()
{
	const auto &g = getGraph();
	auto *EE = getEE();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);
		llvm::dbgs() << "<" << thr.parentId << "," << thr.id
			     << "> " << thr.threadFun->getName() << ": ";
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->isRevisitable())
					llvm::dbgs().changeColor(llvm::raw_ostream::Colors::GREEN);
				auto val = getWriteValue(rLab->getRf(), rLab->getAddr(),
							 rLab->getType());
				llvm::dbgs() << "R" << EE->getVarName(rLab->getAddr()) << ","
					     << val.IntVal << " ";
				llvm::dbgs().resetColor();
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				llvm::dbgs() << "W" << EE->getVarName(wLab->getAddr()) << ","
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
	const ExecutionGraph &g = getGraph();
	llvm::Interpreter *EE = getEE();
	std::ofstream fout(filename);
	llvm::raw_os_ostream ss(fout);

	auto *before = g.getPrefixView(errorEvent).clone();
	if (!confEvent.isInitializer())
		before->update(g.getPrefixView(confEvent));

	EE->replayExecutionBefore(*before);

	/* Create a directed graph graph */
	ss << "strict digraph {\n";
	/* Specify node shape */
	ss << "\tnode [shape=box]\n";
	/* Left-justify labels for clusters */
	ss << "\tlabeljust=l\n";

	/* Print all nodes with each thread represented by a cluster */
	for (auto i = 0u; i < before->size(); i++) {
		auto &thr = EE->getThrById(i);
		ss << "subgraph cluster_" << thr.id << "{\n";
		ss << "\tlabel=\"" << thr.threadFun->getName().str() << "()\"\n";
		for (auto j = 1; j <= (*before)[i]; j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));

			ss << "\t\"" << lab->getPos() << "\" [label=\"";

			/* First, print the graph label for this node */
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				auto name = EE->getVarName(rLab->getAddr());
				auto val = getWriteValue(rLab->getRf(), rLab->getAddr(),
							 rLab->getType());
				executeRLPrint(rLab, name, val, ss);
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				auto name = EE->getVarName(wLab->getAddr());
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
	for (auto i = 0u; i < before->size(); i++) {
		auto &thr = EE->getThrById(i);
		for (auto j = 0; j <= (*before)[i]; j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));

			/* Print a po-edge, but skip dummy start events for
			 * all threads except for the first one */
			if (j < (*before)[i] && !llvm::isa<ThreadStartLabel>(lab))
				ss << "\"" << lab->getPos() << "\" -> \""
				   << lab->getPos().next() << "\"\n";
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				/* Do not print RFs from the INIT/BOTTOM event */
				if (!rLab->getRf().isInitializer() &&
				    !rLab->getRf().isBottom()) {
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
	const auto &g = getGraph();

	if (a.contains(e))
		return;

	auto ai = a[e.thread];
	a[e.thread] = e.index;
	auto &thr = getEE()->getThrById(e.thread);
	for (int i = ai; i <= e.index; i++) {
		const EventLabel *lab = g.getEventLabel(Event(e.thread, i));
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
			if (!rLab->getRf().isBottom())
				recPrintTraceBefore(rLab->getRf(), a, ss);
		if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab))
			recPrintTraceBefore(jLab->getChildLast(), a, ss);
		if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab))
			if (!bLab->getParentCreate().isInitializer())
				recPrintTraceBefore(bLab->getParentCreate(), a, ss);

		/* Do not print the line if it is an RMW write, since it will be
		 * the same as the previous one */
		if (llvm::isa<CasWriteLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
			continue;
		/* Similarly for a Wna just after the creation of a thread
		 * (it is the store of the PID) */
		if (i > 0 && llvm::isa<ThreadCreateLabel>(g.getPreviousLabel(lab)))
			continue;
		Parser::parseInstFromMData(thr.prefixLOC[i], thr.threadFun->getName().str(), ss);
	}
	return;
}

void GenMCDriver::printTraceBefore(Event e)
{
	llvm::dbgs() << "Trace to " << e << ":\n";

	/* Replay the execution up to the error event (collects mdata).
	 * Even if the prefix has holes, replaying will fill them up,
	 * so we end up with a (po U rf) view of the offending execution */
	const VectorClock &before = getGraph().getPrefixView(e);
	getEE()->replayExecutionBefore(before);

	/* Linearize (po U rf) and print trace */
	View a;
	recPrintTraceBefore(e, a);
}
