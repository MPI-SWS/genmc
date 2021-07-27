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

#include "config.h"
#include "Config.hpp"
#include "Error.hpp"
#include "GraphBuilder.hpp"
#include "LLVMModule.hpp"
#include "GenMCDriver.hpp"
#include "Interpreter.h"
#include "Parser.hpp"
#include "SExprVisitor.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/raw_os_ostream.h>

#include <algorithm>
#include <csignal>

/************************************************************
 ** GENERIC MODEL CHECKING DRIVER
 ***********************************************************/

GenMCDriver::GenMCDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod, clock_t start)
	: userConf(std::move(conf)), isMootExecution(false), explored(0),
	  exploredBlocked(0), duplicates(0), start(start)
{
	ModuleInfo MI;
	std::string buf;

	/* Prepare the module for verification */
	LLVMModule::transformLLVMModule(*mod, getConf(), MI);
	if (userConf->transformFile != "")
		LLVMModule::printLLVMModule(*mod, userConf->transformFile);

	/* Create an interpreter for the program's instructions */
	EE = std::unique_ptr<llvm::Interpreter>((llvm::Interpreter *)
		llvm::Interpreter::create(std::move(mod), std::move(MI), this, getConf(), &buf));

	/* Set up an suitable execution graph with appropriate relations */
	execGraph = GraphBuilder(userConf->isDepTrackingModel, userConf->warnOnGraphSize)
		.withCoherenceType(userConf->coherence)
		.withEnabledLAPOR(userConf->LAPOR)
		.withEnabledPersevere(userConf->persevere, userConf->blockSize).build();

	/* Set up a random-number generator (for the scheduler) */
	std::random_device rd;
	auto seedVal = (userConf->randomScheduleSeed != "") ?
		(MyRNG::result_type) stoull(userConf->randomScheduleSeed) : rd();
	if (userConf->printRandomScheduleSeed)
		llvm::outs() << "Seed: " << seedVal << "\n";
	rng.seed(seedVal);

	/* If a specs file has been specified, parse it */
	if (userConf->specsFile != "") {
		auto res = Parser().parseSpecs(userConf->specsFile);
		std::copy_if(res.begin(), res.end(), std::back_inserter(grantedLibs),
			     [](Library &l){ return l.getType() == Granted; });
		std::copy_if(res.begin(), res.end(), std::back_inserter(toVerifyLibs),
			     [](Library &l){ return l.getType() == ToVerify; });
	}

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

GenMCDriver::~GenMCDriver() = default;

void GenMCDriver::printResults()
{
	std::string dups = " (" + std::to_string(duplicates) + " duplicates)";
	llvm::outs() << "Number of complete executions explored: " << explored
		     << ((userConf->countDuplicateExecs) ? dups : "") << "\n";
	if (exploredBlocked) {
		llvm::outs() << "Number of blocked executions seen: " << exploredBlocked
			     << "\n";
	}
	llvm::outs() << "Total wall-clock time: "
		     << llvm::format("%.2f", ((float) clock() - start)/CLOCKS_PER_SEC)
		     << "s\n";
}

void GenMCDriver::resetThreadPrioritization()
{
	if (!userConf->LAPOR) {
		threadPrios.clear();
		return;
	}

	/*
	 * Check if there is some thread that did not manage to finish its
	 * critical section, and mark this execution as blocked
	 */
	const auto &g = getGraph();
	auto *EE = getEE();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		Event last = g.getLastThreadEvent(i);
		if (!g.getLastThreadUnmatchedLockLAPOR(last).isInitializer())
			EE->getThrById(i).block(llvm::Thread::BlockageType::BT_LockRel);
	}

	/* Clear all prioritization */
	threadPrios.clear();
}

bool GenMCDriver::isLockWellFormedLAPOR() const
{
	if (!getConf()->LAPOR)
		return true;

	/*
	 * Check if there is some thread that did not manage to finish its
	 * critical section, and mark this execution as blocked
	 */
	const auto &g = getGraph();
	auto *EE = getEE();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		Event last = g.getLastThreadEvent(i);
		if (!g.getLastThreadUnmatchedLockLAPOR(last).isInitializer() &&
		    std::none_of(EE->threads.begin(), EE->threads.end(), [](const llvm::Thread &thr){
				    return thr.getBlockageType() == llvm::Thread::BlockageType::BT_Cons; }))
			return false;
	}
	return true;
}

void GenMCDriver::prioritizeThreads()
{
	if (!userConf->LAPOR)
		return;

	const auto &g = getGraph();

	/* Prioritize threads according to lock acquisitions */
	threadPrios = g.getLbOrderingLAPOR();

	/* Remove threads that are executed completely */
	auto remIt = std::remove_if(threadPrios.begin(), threadPrios.end(), [&](Event e)
				    { return llvm::isa<ThreadFinishLabel>(g.getLastThreadLabel(e.thread)); });
	threadPrios.erase(remIt, threadPrios.end());
	return;
}

bool GenMCDriver::isSchedulable(int thread) const
{
	auto &thr = getEE()->getThrById(thread);
	return !thr.ECStack.empty() && !thr.isBlocked() &&
		!llvm::isa<ThreadFinishLabel>(getGraph().getLastThreadLabel(thread));
}

bool GenMCDriver::schedulePrioritized()
{
	/* Return false if no thread is prioritized */
	if (threadPrios.empty())
		return false;

	const auto &g = getGraph();
	auto *EE = getEE();
	for (auto &e : threadPrios) {
		/* Skip unschedulable threads */
		if (!isSchedulable(e.thread))
			continue;

		/* Found a not-yet-complete thread; schedule it */
		EE->currentThread = e.thread;
		return true;
	}
	return false;
}

bool GenMCDriver::scheduleNextLTR()
{
	auto &g = getGraph();
	auto *EE = getEE();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (!isSchedulable(i))
			continue;

		/* Found a not-yet-complete thread; schedule it */
		EE->currentThread = i;
		return true;
	}

	/* No schedulable thread found */
	return false;
}

bool GenMCDriver::isNextThreadInstLoad(int tid)
{
	auto &I = getEE()->getThrById(tid).ECStack.back().CurInst;

	/* Overapproximate with function calls some of which might be modeled as loads */
	return llvm::isa<llvm::LoadInst>(I) || llvm::isa<llvm::AtomicCmpXchgInst>(I) ||
		llvm::isa<llvm::AtomicRMWInst>(I) || llvm::isa<llvm::CallInst>(I);
}

bool GenMCDriver::scheduleNextWF()
{
	auto &g = getGraph();
	auto *EE = getEE();

	/* Try and find a thread that satisfies the policy.
	 * Keep an LTR fallback option in case this fails */
	long fallback = -1;
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (!isSchedulable(i))
			continue;

		if (fallback == -1)
			fallback = i;
		if (!isNextThreadInstLoad(i)) {
			EE->currentThread = i;
			return true;
		}
	}

	/* Otherwise, try to schedule the fallback thread */
	if (fallback != -1) {
		EE->currentThread = fallback;
		return true;
	}
	return false;
}

bool GenMCDriver::scheduleNextRandom()
{
	auto &g = getGraph();
	auto *EE = getEE();

	/* Check if randomize scheduling is enabled and schedule some thread */
	MyDist dist(0, g.getNumThreads());
	auto random = dist(rng);
	for (auto j = 0u; j < g.getNumThreads(); j++) {
		auto i = (j + random) % g.getNumThreads();

		if (!isSchedulable(i))
			continue;

		/* SR: Symmetric threads have to always be executed in order */
		if (getConf()->symmetryReduction) {
			auto *bLab = llvm::dyn_cast<ThreadStartLabel>(g.getEventLabel(Event(i, 0)));
			auto symm = bLab->getSymmetricTid();
			if (symm != -1 && isSchedulable(symm) &&
			    g.getThreadSize(symm) <= g.getThreadSize(i)) {
				EE->currentThread = symm;
				return true;
			}
		}

		/* Found a not-yet-complete thread; schedule it */
		EE->currentThread = i;
		return true;
	}

	/* No schedulable thread found */
	return false;
}

void GenMCDriver::deprioritizeThread()
{
	/* Extra check to make sure the function is properly used */
	if (!userConf->LAPOR)
		return;

	auto &g = getGraph();
	auto *lab = getCurrentLabel();

	BUG_ON(!llvm::isa<UnlockLabelLAPOR>(lab));
	auto *uLab = static_cast<const UnlockLabelLAPOR *>(lab);

	auto delIt = threadPrios.end();
	for (auto it = threadPrios.begin(); it != threadPrios.end(); ++it) {
		const EventLabel *lab = g.getEventLabel(*it);
		BUG_ON(!llvm::isa<LockLabelLAPOR>(lab));

		auto *lLab = static_cast<const LockLabelLAPOR *>(lab);
		if (lLab->getThread() == uLab->getThread() &&
		    lLab->getLockAddr() == uLab->getLockAddr()) {
			delIt = it;
			break;
		}
	}

	if (delIt != threadPrios.end())
		threadPrios.erase(delIt);
}

void GenMCDriver::resetExplorationOptions()
{
	isMootExecution = false;
	resetThreadPrioritization();
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

		/* Skip if parent create does not exist yet (or anymore) */
		if (!g.contains(parent) || !llvm::isa<ThreadCreateLabel>(g.getEventLabel(parent)))
			continue;

		/* Skip finished threads */
		const EventLabel *labLast = g.getLastThreadLabel(i);
		if (llvm::isa<ThreadFinishLabel>(labLast))
			continue;

		/* Skip the recovery thread, if it exists.
		 * It will be scheduled separately afterwards */
		if (i == g.getRecoveryRoutineId())
			continue;

		/* Otherwise, initialize ECStacks in interpreter */
		auto &thr = getEE()->getThrById(i);
		BUG_ON(!thr.ECStack.empty() || thr.isBlocked());
		thr.ECStack.push_back(thr.initSF);

		/* Mark threads that are blocked appropriately */
		if (auto *lLab = llvm::dyn_cast<LockCasReadLabel>(labLast))
			thr.block(llvm::Thread::BlockageType::BT_LockAcq);
	}

	/* Then, set up thread prioritization and interpreter's state */
	prioritizeThreads();
	getEE()->setProgramState(llvm::Interpreter::PS_Main);
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

	/* LAPOR: Check lock-well-formedness */
	if (userConf->LAPOR && !isLockWellFormedLAPOR())
		WARN_ONCE("lapor-not-well-formed", "Execution not lock-well-formed!\n");

	/* Ignore the execution if some assume has failed; check liveness here */
	if (std::any_of(getEE()->threads.begin(), getEE()->threads.end(),
			[](llvm::Thread &thr){ return thr.isBlocked(); })) {
		++exploredBlocked;
		if (userConf->checkLiveness)
			checkLiveness();
		return;
	}

	/* Handle printing and counting */
	const auto &g = getGraph();
	if (userConf->checkConsPoint == ProgramPoint::exec &&
	    !isConsistent(ProgramPoint::exec))
		return;
	if (userConf->printExecGraphs && !userConf->persevere)
		printGraph(); /* Delay printing if persevere is enabled */
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
	return;
}

void GenMCDriver::handleRecoveryStart()
{
	auto &g = getGraph();
	auto *EE = getEE();

	/* Make sure that a thread for the recovery routine is
	 * added only once in the execution graph*/
	if (g.getRecoveryRoutineId() == -1)
		g.addRecoveryThread();

	/* We will create a start label for the recovery thread.
	 * We synchronize with a persistency barrier, if one exists,
	 * otherwise, we synchronize with nothing */
	auto tid = g.getRecoveryRoutineId();
	auto psb = g.collectAllEvents([&](const EventLabel *lab)
				      { return llvm::isa<DskPbarrierLabel>(lab); });
	if (psb.empty())
		psb.push_back(Event::getInitializer());
	ERROR_ON(psb.size() > 1, "Usage of only one persistency barrier is allowed!\n");

	auto tsLab = ThreadStartLabel::create(g.nextStamp(), Event(tid, 0), psb.back());
	updateLabelViews(tsLab.get());
	g.addOtherLabelToGraph(std::move(tsLab));

	/* Create a thread for the interpreter, and appropriately
	 * add it to the thread list (pthread_create() style) */
	auto rec = EE->createRecoveryThread(tid);
	if (tid == (int) g.getNumThreads() - 1) {
		EE->threads.push_back(rec);
	} else {
		EE->threads[tid] = rec;
	}

	/* Finally, do all necessary preparations in the interpreter */
	getEE()->setProgramState(llvm::Interpreter::PS_Recovery);
	getEE()->setupRecoveryRoutine(tid);
	return;
}

void GenMCDriver::handleRecoveryEnd()
{
	/* Print the graph with the recovery routine */
	if (userConf->printExecGraphs)
		printGraph();
	getEE()->cleanupRecoveryRoutine(getGraph().getRecoveryRoutineId());
	return;
}

void GenMCDriver::run()
{
	/* Explore all graphs and print the results */
	explore();
	printResults();
	return;
}

void GenMCDriver::addToWorklist(std::unique_ptr<WorkItem> item)

{
	const auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(item->getPos());

	workqueue[lab->getStamp()].add(std::move(item));
	return;
}

std::unique_ptr<WorkItem> GenMCDriver::getNextItem()
{
	for (auto rit = workqueue.rbegin(); rit != workqueue.rend(); ++rit) {
		if (rit->second.empty())
			continue;

		return rit->second.getNext();
	}
	return nullptr;
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

bool GenMCDriver::revisitSetContains(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
				     const std::vector<std::pair<Event, Event> > &moPlacings)
{
	return revisitSet[rLab->getStamp()].contains(writePrefix, moPlacings);
}

void GenMCDriver::addToRevisitSet(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
				  const std::vector<std::pair<Event, Event> > &moPlacings)
{
	revisitSet[rLab->getStamp()].add(writePrefix, moPlacings);
	return;
}

void GenMCDriver::restrictRevisitSet(const EventLabel *rLab)
{
	auto stamp = rLab->getStamp();
	std::vector<int> idxsToRemove;

	for (auto rit = revisitSet.rbegin(); rit != revisitSet.rend(); ++rit)
		if (rit->first > stamp)
			idxsToRemove.push_back(rit->first);

	for (auto &i : idxsToRemove)
		revisitSet.erase(i);
}

void GenMCDriver::notifyEERemoved(unsigned int cutStamp)
{
	const auto &g = getGraph();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (lab->getStamp() <= cutStamp)
				continue;

			/* Untrack memory if allocation event will be deleted */
			if (auto *mLab = llvm::dyn_cast<MallocLabel>(lab))
				getEE()->untrackAlloca(mLab->getAllocAddr(),
						       mLab->getAllocSize(),
						       mLab->getStorage(),
						       mLab->getAddrSpace());
			/* For persistency, reclaim fds */
			if (auto *oLab = llvm::dyn_cast<DskOpenLabel>(lab))
				getEE()->reclaimUnusedFd(oLab->getFd().IntVal.getLimitedValue());
		}
	}
}

void GenMCDriver::restrictGraph(const EventLabel *rLab)
{
	unsigned int stamp = rLab->getStamp();

	/* Inform the interpreter about deleted events, and then
	 * restrict the graph (and relations) */
	notifyEERemoved(stamp);
	getGraph().cutToStamp(stamp);
	return;
}

void GenMCDriver::notifyEERestored(const std::vector<std::unique_ptr<EventLabel> > &prefix)
{
	for (auto &lab : prefix) {
		if (auto *mLab = llvm::dyn_cast<MallocLabel>(&*lab))
			getEE()->trackAlloca(mLab->getAllocAddr(),
					     mLab->getAllocSize(),
					     mLab->getStorage(),
					     mLab->getAddrSpace());
		if (auto *oLab = llvm::dyn_cast<DskOpenLabel>(&*lab))
			getEE()->markFdAsUsed(oLab->getFd().IntVal.getLimitedValue());
	}
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

	/* Inform the interpreter about events being restored, and then restore them */
	notifyEERestored(prefix);
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
	if (schedulePrioritized())
		return true;

	/* Schedule the next thread according to the chosen policy */
	switch (getConf()->schedulePolicy) {
	case SchedulePolicy::ltr:
		return scheduleNextLTR();
	case SchedulePolicy::wf:
		return scheduleNextWF();
	case SchedulePolicy::random:
		return scheduleNextRandom();
	default:
		BUG();
	}
	BUG();
}

void GenMCDriver::explore()
{
	auto *EE = getEE();

	while (true) {
		EE->reset();

		/* Get main program function and run the program */
		EE->runStaticConstructorsDestructors(false);
		EE->runFunctionAsMain(EE->FindFunctionNamed(userConf->programEntryFun.c_str()), {"prog"}, nullptr);
		EE->runStaticConstructorsDestructors(true);

		auto validExecution = true;
		do {
			/*
			 * revisitReads() might deem some execution infeasible,
			 * so we have to reset all exploration options before
			 * calling it again
			 */
			resetExplorationOptions();

			auto item = getNextItem();
			if (!item) {
				EE->reset();  /* To free memory */
				return;
			}
			validExecution = revisitReads(std::move(item)) && isConsistent(ProgramPoint::step);
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

bool GenMCDriver::inRecoveryMode() const
{
	return getEE()->getProgramState() == llvm::Interpreter::PS_Recovery;
}

const EventLabel *GenMCDriver::getCurrentLabel() const
{
	const auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();

	BUG_ON(!g.contains(pos));
	return g.getEventLabel(pos);
}

/* Given an event in the graph, returns the value of it */
llvm::GenericValue GenMCDriver::getWriteValue(Event write,
					      const llvm::GenericValue *ptr,
					      const llvm::Type *typ)
{
	/* If the even represents an invalid access, return some value */
	if (write.isBottom())
		return GET_ZERO_GV(typ);

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
		ERROR("Mixed-size accesses detected! Please check the LLVM-IR.\n");
	}
	return result;
}

/* Same as above, but the data of a file are not explicitly initialized
 * so as not to pollute the graph with events, since a file can be large.
 * Thus, we treat the case where WRITE reads INIT specially. */
llvm::GenericValue GenMCDriver::getDskWriteValue(Event write,
						 const llvm::GenericValue *ptr,
						 const llvm::Type *typ)
{
	if (write.isInitializer())
		return GET_ZERO_GV(typ);
	return getWriteValue(write, ptr, typ);
}

llvm::GenericValue GenMCDriver::getBarrierInitValue(const llvm::GenericValue *ptr,
						    const llvm::Type *typ)
{
	auto &g = getGraph();
	auto &stores = g.getStoresToLoc(ptr);

	auto sIt = std::find_if(stores.begin(), stores.end(), [ptr,&g](const Event &s){
		auto *bLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
		BUG_ON(!bLab);
		return bLab->getAddr() == ptr && bLab->isNotAtomic();
	});

	/* All errors pertinent to initialization should be captured elsewhere */
	BUG_ON(sIt == stores.end());
	return getWriteValue(*sIt, ptr, typ);
}

llvm::GenericValue GenMCDriver::getReadRetValueAndMaybeBlock(Event read,
							     const llvm::GenericValue *ptr,
							     const llvm::Type *typ)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();
	auto *rLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(read));
	BUG_ON(!rLab);

	/* Fetch appropriate return value */
	auto res = getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());

	/* Check whether we should block */
	if (rLab->getRf().isBottom()) {
		/* Bottom is an acceptable re-option only @ replay; block anyway */
		BUG_ON(getEE()->getExecState() != llvm::Interpreter::ExecutionState::ES_Replay);
		thr.block(llvm::Thread::BlockageType::BT_Error);
	} else if (llvm::isa<BWaitReadLabel>(rLab) &&
		   !getEE()->compareValues(typ, res, getBarrierInitValue(ptr, typ))) {
		/* Reading a non-init barrier value means that the thread should block */
		thr.block(llvm::Thread::BlockageType::BT_Barrier);
	}
	return res;
}

llvm::GenericValue GenMCDriver::getRecReadRetValue(const llvm::GenericValue *addr,
						   const llvm::Type *typ)
{
	auto &g = getGraph();
	auto recLast = getEE()->getCurrentPosition();
	auto rf = g.getLastThreadStoreAtLoc(recLast.next(), addr);
	BUG_ON(rf.isInitializer());
	return getWriteValue(rf, addr, typ);
}

bool GenMCDriver::shouldCheckCons(ProgramPoint p)
{
	/* Always check consistency on error, or at user-specified points */
	if (p <= getConf()->checkConsPoint)
		return true;

	/* LAPOR requires consistency checks at each step, and at the
	 * end of an execution, when popping an alternative rf option */
	if (getConf()->LAPOR)
		return true;

	return false;
}

bool GenMCDriver::shouldCheckFullCons(ProgramPoint p)
{
	if (p == ProgramPoint::error ||
	    (p <= getConf()->checkConsPoint &&
	     getConf()->checkConsType == CheckConsType::full))
		return true;
	return false;
}

bool GenMCDriver::shouldCheckPers(ProgramPoint p)
{
	/* Always check consistency on error, or at user-specified points */
	return p <= getConf()->checkPersPoint;
}

bool GenMCDriver::isHbBefore(Event a, Event b, ProgramPoint p /* = step */)
{
	if (shouldCheckCons(p) == false)
		return getGraph().getEventLabel(b)->getHbView().contains(a);

	return getGraph().getGlobalRelation(ExecutionGraph::RelationId::hb)(a, b);
}

bool GenMCDriver::isCoMaximal(const llvm::GenericValue *addr, Event e,
			      bool checkCache /* = false */, ProgramPoint p /* = step */)
{
	auto &g = getGraph();
	auto *cc = g.getCoherenceCalculator();

	if (checkCache)
		return cc->isCachedCoMaximal(addr, e);
	if (!shouldCheckCons(p))
		return cc->isCoMaximal(addr, e);

	auto &coLoc = g.getPerLocRelation(ExecutionGraph::RelationId::co)[addr];
	return (e.isInitializer() && coLoc.empty()) ||
	       (!e.isInitializer() && coLoc.adj_begin(e) == coLoc.adj_end(e));
}

void GenMCDriver::findMemoryRaceForMemAccess(const MemAccessLabel *mLab)
{
	const auto &g = getGraph();
	const View &before = g.getHbBefore(mLab->getPos().prev());
	const MallocLabel *allocLab = nullptr;

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *oLab = g.getEventLabel(Event(i, j));
			/* Find preceding malloc */
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(oLab)) {
				if (aLab->getAllocAddr() <= mLab->getAddr() &&
				    ((char *) aLab->getAllocAddr() +
				     aLab->getAllocSize() > (char *) mLab->getAddr())) {
					allocLab = aLab;
					if (!before.contains(oLab->getPos())) {
						visitError(DE_AccessNonMalloc,
							   "The allocating operation (malloc()) "
							   "does not happen-before the memory access!",
							   oLab->getPos());
						return;
					}
				}
			}
		}
	}
	if (!allocLab) {
		visitError(DE_AccessNonMalloc, "No happens-before preceding allocation (malloc()) found!");
		return;
	}

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *oLab = g.getEventLabel(Event(i, j));
			/* Check if the malloc()-address has been freed */
			if (auto *fLab = llvm::dyn_cast<FreeLabel>(oLab)) {
				if (fLab->getFreedAddr() == allocLab->getAllocAddr()) {
					visitError(DE_AccessFreed, "", oLab->getPos());
					return;
				}
			}
		}
	}
	return;
}

void GenMCDriver::findMemoryRaceForAllocAccess(const FreeLabel *fLab)
{
	const auto &g = getGraph();
	auto *ptr = fLab->getFreedAddr();
	auto &before = g.getHbBefore(fLab->getPos());
	const MallocLabel *allocLab = nullptr; /* There must be a malloc() before the free() */

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
				if (aLab->getAllocAddr() == ptr &&
				    before.contains(aLab->getPos())) {
					allocLab = aLab;
				}
			}
		}
	}
	if (!allocLab) {
		visitError(DE_FreeNonMalloc);
		return;
	}

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *dLab = llvm::dyn_cast<FreeLabel>(lab)) {
				if (dLab->getFreedAddr() == ptr &&
				    dLab->getPos() != fLab->getPos()) {
					visitError(DE_DoubleFree, "", dLab->getPos());
				}
			}
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
				if (mLab->getAddr() >= allocLab->getAllocAddr() &&
				    (char *) mLab->getAddr() < (char *) allocLab->getAllocAddr() +
				    allocLab->getAllocSize() &&
				    !before.contains(mLab->getPos())) {
					visitError(DE_AccessFreed, "", mLab->getPos());
				}
			}
		}
	}
	return;
}

void GenMCDriver::checkForMemoryRaces(const void *addr)
{
	if (userConf->disableRaceDetection)
		return;
	if (!getEE()->isDynamic(addr) || getEE()->isInternal(addr))
		return;

	const EventLabel *lab = getCurrentLabel();
	BUG_ON(!llvm::isa<MemAccessLabel>(lab) && !llvm::isa<FreeLabel>(lab));

	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab))
		findMemoryRaceForMemAccess(mLab);
	else if (auto *fLab = llvm::dyn_cast<FreeLabel>(lab))
		findMemoryRaceForAllocAccess(fLab);
	return;
}

void GenMCDriver::checkForUninitializedMem(const std::vector<Event> &rfs)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(getCurrentLabel());
	if (!rLab)
		return;

	auto *EE = getEE();
	if (EE->isDynamic(rLab->getAddr()) && !EE->isInternal(rLab->getAddr()) &&
	    std::any_of(rfs.begin(), rfs.end(), [](const Event &rf){ return rf.isInitializer(); }))
		visitError(DE_UninitializedMem);
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
		visitError(DE_RaceNotAtomic, "", racy);
	}
	return;
}

bool GenMCDriver::isAccessValid(const llvm::GenericValue *addr)
{
	/* Only valid memory locations should be accessed */
	return getEE()->isShared(addr);
}

void GenMCDriver::checkLockValidity(const std::vector<Event> &rfs)
{
	auto *lLab = llvm::dyn_cast<LockCasReadLabel>(getCurrentLabel());
	if (!lLab)
		return;

	/* Should not read from destroyed mutex */
	auto rfIt = std::find_if(rfs.cbegin(), rfs.cend(), [this, lLab](const Event &rf){
		auto rfVal = getWriteValue(rf, lLab->getAddr(), lLab->getType());
		return getEE()->compareValues(lLab->getType(), rfVal, INT_TO_GV(lLab->getType(), -1));
	});
	if (rfIt != rfs.cend())
		visitError(DE_UninitializedMem, "Called lock() on destroyed mutex!", *rfIt);
}

void GenMCDriver::checkUnlockValidity()
{
	auto *uLab = llvm::dyn_cast<UnlockWriteLabel>(getCurrentLabel());
	if (!uLab)
		return;

	/* Unlocks should unlock mutexes locked by the same thread */
	if (getGraph().getMatchingLock(uLab->getPos()).isInitializer()) {
		visitError(DE_InvalidUnlock,
			   "Called unlock() on mutex not locked by the same thread!");
	}
}

void GenMCDriver::checkBInitValidity()
{
	auto *wLab = llvm::dyn_cast<BInitWriteLabel>(getCurrentLabel());
	if (!wLab)
		return;

	/* Make sure the barrier hasn't already been initialized, and
	 * that the initializing value is greater than 0 */
	auto &g = getGraph();
	auto &stores = g.getStoresToLoc(wLab->getAddr());
	auto sIt = std::find_if(stores.begin(), stores.end(), [&g, wLab](const Event &s){
		auto *sLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
		return sLab != wLab && sLab->getAddr() == wLab->getAddr() &&
			llvm::isa<BInitWriteLabel>(sLab);
	});

	if (sIt != stores.end())
		visitError(DE_InvalidBInit, "Called barrier_init() multiple times!", *sIt);
	else if (getEE()->compareValues(wLab->getType(), wLab->getVal(), GET_ZERO_GV(wLab->getType())))
		visitError(DE_InvalidBInit, "Called barrier_init() with 0!");
	return;
}

void GenMCDriver::checkBIncValidity(const std::vector<Event> &rfs)
{
	auto *bLab = llvm::dyn_cast<BIncFaiReadLabel>(getCurrentLabel());
	if (!bLab)
		return;

	if (std::any_of(rfs.cbegin(), rfs.cend(), [](const Event &rf){ return rf.isInitializer(); }))
		visitError(DE_UninitializedMem, "Called barrier_wait() on uninitialized barrier!");
	else if (std::any_of(rfs.cbegin(), rfs.cend(), [this, bLab](const Event &rf){
		auto rfVal = getWriteValue(rf, bLab->getAddr(), bLab->getType());
		return getEE()->compareValues(bLab->getType(), rfVal, GET_ZERO_GV(bLab->getType()));
	}))
		visitError(DE_AccessFreed, "Called barrier_wait() on destroyed barrier!", bLab->getRf());
}

void addPerLocRelationToExtend(Calculator::PerLocRelation &rel,
			       std::vector<Calculator::GlobalRelation *> &toExtend,
			       std::vector<const llvm::GenericValue *> &extOrder)

{
	for (auto &loc : rel) {
		toExtend.push_back(&loc.second);
		extOrder.push_back(loc.first);
	}
}

void extendPerLocRelation(Calculator::PerLocRelation &rel,
			  std::vector<std::vector<Event> >::iterator extsBegin,
			  std::vector<std::vector<Event> >::iterator extsEnd,
			  std::vector<const llvm::GenericValue *>::iterator locsBegin,
			  std::vector<const llvm::GenericValue *>::iterator locsEnd)

{
	BUG_ON(std::distance(extsBegin, extsEnd) != std::distance(locsBegin, locsEnd));
	auto locIt = locsBegin;
	for (auto extIt = extsBegin; extIt != extsEnd; ++extIt, ++locIt) {
		rel[*locIt] = Calculator::GlobalRelation(*extIt);
		for (auto j = 1; j < extIt->size(); j++)
			rel[*locIt].addEdge((*extIt)[j - 1], (*extIt)[j]);
	}
}

bool GenMCDriver::doFinalConsChecks(bool checkFull /* = false */)
{
	if (!checkFull)
		return true;

	bool hasLB = getConf()->LAPOR;
	bool hasWB = getConf()->coherence == CoherenceType::wb;
	if (!hasLB && !hasWB)
		return true;

	auto &g = getGraph();

	/* Cache all relations because we will need to restore them
	 * after possible extensions were tried*/
	g.cacheRelations();

	/* We flatten all per-location relations that need to be
	 * extended.  However, we need to ensure that, for each
	 * per-location relation, the order in which the locations of
	 * the relation are added to the extension list is the same as
	 * the order in which the extensions of said locations are
	 * restored.
	 *
	 * This is not the case in all platforms if we are simply
	 * iterating over unordered_maps<> (e.g. macΟS). The iteration
	 * order may differ if, e.g., when saving an extension we
	 * iterate over a relation's cache, while when restoring we
	 * iterate over the relation itself, even though the relation
	 * is the same as its cache just before restoring. */
	auto coSize = 0u;
	auto lbSize = 0u;
	std::vector<Calculator::GlobalRelation *> toExtend;
	std::vector<const llvm::GenericValue *> extOrder;
	if (hasWB) {
		addPerLocRelationToExtend(g.getCachedPerLocRelation(ExecutionGraph::RelationId::co),
					  toExtend, extOrder);
		coSize = g.getCachedPerLocRelation(ExecutionGraph::RelationId::co).size();
	}
	if (hasLB) {
		addPerLocRelationToExtend(g.getCachedPerLocRelation(ExecutionGraph::RelationId::lb),
					  toExtend, extOrder);
		lbSize = g.getCachedPerLocRelation(ExecutionGraph::RelationId::lb).size();
	}

	auto res = Calculator::GlobalRelation::
		combineAllTopoSort(toExtend, [&](std::vector<std::vector<Event>> &sortings){
			g.restoreCached();
			auto count = 0u;
			if (hasWB) {
				extendPerLocRelation(g.getPerLocRelation(ExecutionGraph::RelationId::co),
						     sortings.begin(), sortings.begin() + coSize,
						     extOrder.begin(), extOrder.begin() + coSize);
			}
			count += coSize;
			if (hasLB && count >= coSize) {
				extendPerLocRelation(g.getPerLocRelation(ExecutionGraph::RelationId::lb),
						     sortings.begin() + count,
						     sortings.begin() + count + lbSize,
						     extOrder.begin() + count,
						     extOrder.begin() + count + lbSize);
			}
			count += lbSize;
			return g.doCalcs(true).cons;
		});
	return res;
}

bool GenMCDriver::isConsistent(ProgramPoint p)
{
	/* Fastpath: No fixpoint is required */
	auto check = shouldCheckCons(p);
	if (!check)
		return true;

	auto checkFull = shouldCheckFullCons(p);

	/* The specific instance will populate the necessary entries
	 * in the graph */
	getGraph().doInits(checkFull);
	initConsCalculation();

	/* Fixpoint calculation */
	Calculator::CalculationResult step;
	do {
		step = getGraph().doCalcs(checkFull);
		if (!step.cons)
			return false;
	} while (step.changed);

	/* Do final checks, after the fixpoint is over */
	return doFinalConsChecks(checkFull);
}

bool GenMCDriver::isRecoveryValid(ProgramPoint p)
{
	/* If we are not in the recovery routine, nothing to do */
	if (!inRecoveryMode())
		return true;

	/* Fastpath: No fixpoint is required */
	auto check = shouldCheckPers(p);
	if (!check)
		return true;

	return getGraph().isRecoveryValid();
}

bool GenMCDriver::threadReadsMaximal(int tid)
{
	auto &g = getGraph();

	/*
	 * Depending on whether this is a DSA loop or not, we have to
	 * adjust the detection starting point: DSA-blocked threads
	 * will have a SpinStart as their last event.
	 */
	auto *lastLab = g.getLastThreadLabel(tid);
	auto start = llvm::isa<SpinStartLabel>(lastLab) ? lastLab->getPos().prev() : lastLab->getPos();
	for (auto j = start.index; j > 0; j--) {
		auto *lab = g.getEventLabel(Event(tid, j));
		BUG_ON(llvm::isa<LoopBeginLabel>(lab));
		if (llvm::isa<SpinStartLabel>(lab))
			return true;
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (!isCoMaximal(rLab->getAddr(), rLab->getRf()))
				return false;
		}
	}
	BUG();
}

void GenMCDriver::checkLiveness()
{
	if (shouldCheckCons(ProgramPoint::exec) && !isConsistent(ProgramPoint::exec))
		return;

	const auto &g = getGraph();
	const auto *EE = getEE();
	std::vector<int> spinBlocked;

	/* Collect all threads blocked at spinloops */
	for (const auto &thr : EE->threads) {
		if (thr.getBlockageType() == llvm::Thread::BT_Spinloop)
			spinBlocked.push_back(thr.id);
	}
	/* And check whether all of them are live or not */
	if (!spinBlocked.empty() &&
	    std::all_of(spinBlocked.begin(), spinBlocked.end(),
			[&](int tid){ return threadReadsMaximal(tid); })) {
		/* Print some TID blocked by a spinloop */
		visitError(DE_Liveness, "Non-terminating spinloop: " \
			   "thread " + std::to_string(spinBlocked[0]));
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
		if (auto *wLab = llvm::dyn_cast<LockCasWriteLabel>(g.getEventLabel(s)))
			continue;

		if (g.isStoreReadBySettledRMW(s, ptr, before))
			continue;

		if (g.isStoreReadByExclusiveRead(s, ptr))
			conflicting.push_back(s);
		else
			valid.push_back(s);
	}

	if (valid.empty()) {
		auto lit = std::find_if(stores.begin(), stores.end(), [&](const Event &s) {
			return llvm::isa<LockCasWriteLabel>(g.getEventLabel(s));
		});
		BUG_ON(lit == stores.end());
		threadPrios = {*lit};
		valid.push_back(*lit);
	}
	valid.insert(valid.end(), conflicting.begin(), conflicting.end());
	return valid;
}

std::vector<Event>
GenMCDriver::properlyOrderStores(InstAttr attr,
				 llvm::Type *typ,
				 const llvm::GenericValue *ptr,
				 llvm::GenericValue &expVal,
				 std::vector<Event> &stores)
{
	if (!isRMWAttr(attr))
		return stores;

	const auto &g = getGraph();
	auto curr = getEE()->getCurrentPosition().prev();
	auto &before = g.getPrefixView(curr);

	if (isLockAttr(attr))
		return filterAcquiredLocks(ptr, stores, before);

	std::vector<Event> valid, conflicting;
	for (auto &s : stores) {
		auto oldVal = getWriteValue(s, ptr, typ);
		if ((isFAIAttr(attr) || EE->compareValues(typ, oldVal, expVal)) &&
		    g.isStoreReadBySettledRMW(s, ptr, before))
			continue;

		if (g.isStoreReadByExclusiveRead(s, ptr))
			conflicting.push_back(s);
		else
			valid.push_back(s);
	}

	/* barrier_wait()'s FAI loads should not read from conflicting stores */
	if (!isBPostAttr(attr) || getConf()->disableBAM)
		valid.insert(valid.end(), conflicting.begin(), conflicting.end());
	return valid;
}

bool GenMCDriver::sharePrefixSR(int tid, Event pos) const
{
	auto &g = getGraph();

	if (tid < 0 || tid >= g.getNumThreads())
		return false;
	if (g.getThreadSize(tid) <= pos.index)
		return false;
	for (auto j = 1u; j < pos.index; j++) {
		auto *labA = g.getEventLabel(Event(tid, j));
		auto *labB = g.getEventLabel(Event(pos.thread, j));

		if (auto *rLabA = llvm::dyn_cast<ReadLabel>(labA)) {
			auto *rLabB = llvm::dyn_cast<ReadLabel>(labB);
			if (!rLabB || rLabA->getRf() != rLabB->getRf())
				return false;
		}
	}
	return true;
}

void GenMCDriver::filterSymmetricStoresSR(const llvm::GenericValue *addr, llvm::Type *typ,
					  std::vector<Event> &stores) const
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto pos = EE->getCurrentPosition();
	auto t = llvm::dyn_cast<ThreadStartLabel>(g.getEventLabel(Event(pos.thread, 0)))->getSymmetricTid();

	/* If there is no symmetric thread, exit */
	if (t == -1)
		return;

	/* Check whether the po-prefixes of the two threads match */
	if (!sharePrefixSR(t, pos))
		return;

	/* Get the symmetric event and make sure it matches as well */
	auto *lab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(Event(t, pos.index)));
	if (!lab || lab->getAddr() != addr || lab->getType() != typ)
		return;

	/* Remove stores that will be explored symmetrically */
	auto rfStamp = g.getEventLabel(lab->getRf())->getStamp();
	auto st = (g.isRMWLoad(lab)) ? rfStamp + 1 : rfStamp;
	stores.erase(std::remove_if(stores.begin(), stores.end(), [&](Event s) {
				    auto *sLab = g.getEventLabel(s);
				    return sLab->getStamp() < st;
		     }), stores.end());
	return;
}

bool GenMCDriver::filterValuesFromAnnotSAVER(const llvm::GenericValue *addr, llvm::Type *typ,
					     const SExpr *annot, std::vector<Event> &validStores)
{
	if (!annot)
		return false;

	/* For WB, there might be many maximal ones */
	auto shouldBlock =
		std::any_of(validStores.begin(), validStores.end(),
			    [&](const Event &s){ return isCoMaximal(addr, s, true) &&
					    !SExprEvaluator().evaluate(annot, getWriteValue(s, addr, typ)); });
	validStores.erase(std::remove_if(validStores.begin(), validStores.end(), [&](Event w) {
		return !isCoMaximal(addr, w, true) &&
		       !SExprEvaluator().evaluate(annot, getWriteValue(w, addr, typ)); }),
		validStores.end());
	BUG_ON(validStores.empty());

	return shouldBlock;
}

bool GenMCDriver::ensureConsistentRf(const ReadLabel *rLab, std::vector<Event> &rfs)
{
	bool found = false;
	while (!found) {
		found = true;
		changeRf(rLab->getPos(), rfs[0]);
		if (!isConsistent(ProgramPoint::step)) {
			found = false;
			rfs.erase(rfs.begin());
			BUG_ON(!userConf->LAPOR && rfs.empty());
			if (rfs.empty())
				break;
		}
	}

	if (!found) {
		for (auto i = 0u; i < getGraph().getNumThreads(); i++)
			getEE()->getThrById(i).block(llvm::Thread::BlockageType::BT_Cons);
		return false;
	}
	return true;
}

bool GenMCDriver::ensureConsistentStore(const WriteLabel *wLab)
{
	if (shouldCheckCons(ProgramPoint::step) && !isConsistent(ProgramPoint::step)) {
		for (auto i = 0u; i < getGraph().getNumThreads(); i++)
			getEE()->getThrById(i).block(llvm::Thread::BlockageType::BT_Cons);
		return false;
	}
	return true;
}

void GenMCDriver::filterInvalidRecRfs(const ReadLabel *rLab, std::vector<Event> &rfs)
{
	rfs.erase(std::remove_if(rfs.begin(), rfs.end(), [&](Event &r){
		  changeRf(rLab->getPos(), r);
		  return !isRecoveryValid(ProgramPoint::step);
	}), rfs.end());
	BUG_ON(rfs.empty());
	changeRf(rLab->getPos(), rfs[0]);
	return;
}

llvm::GenericValue GenMCDriver::visitThreadSelf(llvm::Type *typ)
{
	llvm::GenericValue result;

	result.IntVal = llvm::APInt(typ->getIntegerBitWidth(), getEE()->getCurThr().id);
	return result;
}

bool GenMCDriver::isSymmetricToSR(int candidate, int thread, Event parent,
				  llvm::Function *threadFun, const llvm::GenericValue &threadArg) const
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &cThr = EE->getThrById(candidate);
	auto cParent = llvm::dyn_cast<ThreadStartLabel>(g.getEventLabel(Event(candidate, 0)))->getParentCreate();

	/* First, check that the two threads are actually similar */
	if (cThr.id == thread || cThr.threadFun != threadFun ||
	    cThr.threadArg.PointerVal != threadArg.PointerVal ||
	    cParent.thread != parent.thread)
		return false;

	/* Then make sure that there is no memory access in between the spawn events */
	auto mm = std::minmax(parent.index, cParent.index);
	auto minI = mm.first;
	auto maxI = mm.second;
	for (auto j = minI; j < maxI; j++)
		if (llvm::isa<MemAccessLabel>(g.getEventLabel(Event(parent.thread, j))))
			return false;
	return true;
}

int GenMCDriver::getSymmetricTidSR(int thread, Event parent, llvm::Function *threadFun,
				   const llvm::GenericValue &threadArg) const
{
	auto &g = getGraph();
	auto *EE = getEE();

	for (auto i = g.getNumThreads() - 1; i > 0; i--)
		if (i != thread && isSymmetricToSR(i, thread, parent, threadFun, threadArg))
			return i;
	return -1;
}

int GenMCDriver::visitThreadCreate(llvm::Function *calledFun, const llvm::GenericValue &arg,
				   const llvm::ExecutionContext &SF)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph()) {
		auto *cLab = static_cast<const ThreadCreateLabel *>(getCurrentLabel());
		return cLab->getChildId();
	}

	Event cur = EE->getCurrentPosition();
	int cid = 0;

	/* First, check if the thread to be created already exists */
	while (cid < (long) g.getNumThreads()) {
		if (!g.isThreadEmpty(cid)) {
			auto *bLab = llvm::dyn_cast<ThreadStartLabel>(g.getFirstThreadLabel(cid));
			BUG_ON(!bLab);
			if (bLab->getParentCreate() == cur)
				break;
		}
		++cid;
	}

	/* Add an event for the thread creation */
	auto tcLab = ThreadCreateLabel::create(g.nextStamp(), cur, cid);
	updateLabelViews(tcLab.get());
	auto *lab = g.addOtherLabelToGraph(std::move(tcLab));

	/* Prepare the execution context for the new thread */
	llvm::Thread thr = EE->createNewThread(calledFun, arg, cid, cur.thread, SF);

	if (cid == (int) g.getNumThreads()) {
		/* If the thread does not exist in the graph, make an entry for it */
		EE->threads.push_back(thr);
		g.addNewThread();
		auto symm = getConf()->symmetryReduction ? getSymmetricTidSR(cid, cur, calledFun, arg) : -1;
		auto tsLab = ThreadStartLabel::create(g.nextStamp(), Event(cid, 0), cur, symm);
		updateLabelViews(tsLab.get());
		auto *ss = g.addOtherLabelToGraph(std::move(tsLab));
	} else {
		/* Otherwise, push the execution context to the interpreter and update the graph */
		EE->threads[cid] = thr;
		updateStart(lab->getPos(), g.getFirstThreadEvent(cid));
	}

	return cid;
}

llvm::GenericValue GenMCDriver::visitThreadJoin(llvm::Function *F, const llvm::GenericValue &arg)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	int cid = arg.IntVal.getLimitedValue(std::numeric_limits<int>::max());
	if (cid < 0 || int (EE->threads.size()) <= cid || cid == thr.id) {
		std::string err = "ERROR: Invalid TID in pthread_join(): " + std::to_string(cid);
		if (cid == thr.id)
			err += " (TID cannot be the same as the calling thread)";
		visitError(DE_InvalidJoin, err);
	}

	/* If necessary, add a relevant event to the graph */
	if (!isExecutionDrivenByGraph()) {
		auto jLab = ThreadJoinLabel::create(g.nextStamp(), Event(thr.id, thr.globalInstructions), cid);
		updateLabelViews(jLab.get());
		g.addOtherLabelToGraph(std::move(jLab));
	}

	/* If the update failed (child has not terminated yet) block this thread */
	if (!updateJoin(getEE()->getCurrentPosition(), g.getLastThreadEvent(cid)))
		thr.block(llvm::Thread::BlockageType::BT_ThreadJoin);

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
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	if (!isExecutionDrivenByGraph() && /* Make sure that there is not a failed assume... */
	    !thr.isBlocked()) {
		auto eLab = ThreadFinishLabel::create(g.nextStamp(), Event(thr.id, thr.globalInstructions));
		updateLabelViews(eLab.get());
		g.addOtherLabelToGraph(std::move(eLab));

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

static const std::unordered_map<std::string, SmpFenceType> smpFenceTypes = {
	{"mb", SmpFenceType::MB},
	{"rmb", SmpFenceType::RMB},
	{"wmb", SmpFenceType::WMB},
	{"ba", SmpFenceType::MBBA},
	{"aa", SmpFenceType::MBAA},
	{"as", SmpFenceType::MBAS},
	{"aul", SmpFenceType::MBAUL},
};

void GenMCDriver::visitFenceLKMM(llvm::AtomicOrdering ord, const char *lkmmType)
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();

	BUG_ON(smpFenceTypes.count(lkmmType) == 0);
	auto typ = smpFenceTypes.at(lkmmType);

	ord = (isStrong(typ)) ? llvm::AtomicOrdering::SequentiallyConsistent :
		llvm::AtomicOrdering::Monotonic;

	auto fLab = SmpFenceLabelLKMM::create(g.nextStamp(), ord, typ, pos);
	updateLabelViews(fLab.get());
	g.addOtherLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::visitFence(llvm::AtomicOrdering ord, const char *lkmmType /* = nullptr */)
{
	if (lkmmType) {
		visitFenceLKMM(ord, lkmmType);
		return;
	}

	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();

	auto fLab = FenceLabel::create(g.nextStamp(), ord, pos);
	updateLabelViews(fLab.get());
	g.addOtherLabelToGraph(std::move(fLab));
	return;
}

const ReadLabel *
GenMCDriver::createAddReadLabel(InstAttr attr,
				llvm::AtomicOrdering ord,
				const llvm::GenericValue *addr,
				llvm::Type *typ,
				std::unique_ptr<SExpr> annot,
				const llvm::GenericValue &cmpVal,
				const llvm::GenericValue &rmwVal,
				llvm::AtomicRMWInst::BinOp op,
				Event store)
{
	auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();

	std::unique_ptr<ReadLabel> rLab = nullptr;
	switch (attr) {
	case InstAttr::IA_None:
		rLab = ReadLabel::create(g.nextStamp(), ord, pos,
					 addr, typ, store, std::move(annot));
		break;
	case InstAttr::IA_BWait:
		rLab = BWaitReadLabel::create(g.nextStamp(), ord, pos,
					      addr, typ, store, std::move(annot));
		break;
	case InstAttr::IA_Fai:
		rLab = FaiReadLabel::create(g.nextStamp(), ord, pos, addr, typ,
					    store, op, rmwVal, std::move(annot));
		break;
	case InstAttr::IA_NoRetFai:
		rLab = NoRetFaiReadLabel::create(g.nextStamp(), ord, pos, addr, typ,
						 store, op, rmwVal, std::move(annot));
		break;
	case InstAttr::IA_BPost:
		rLab = BIncFaiReadLabel::create(g.nextStamp(), ord, pos, addr, typ,
						store, op, rmwVal, std::move(annot));
		break;
	case InstAttr::IA_Cas:
		rLab = CasReadLabel::create(g.nextStamp(), ord, pos, addr, typ,
					    store, cmpVal, rmwVal, std::move(annot));
		break;
	case InstAttr::IA_Lock:
		rLab = LockCasReadLabel::create(g.nextStamp(), ord, pos, addr, typ,
						store, cmpVal, rmwVal, std::move(annot));
		break;
	case InstAttr::IA_Trylock:
		rLab = TrylockCasReadLabel::create(g.nextStamp(), ord, pos, addr, typ,
						   store, cmpVal, rmwVal, std::move(annot));
		break;
	default:
		BUG();
	}
	updateLabelViews(rLab.get());
	return g.addReadLabelToGraph(std::move(rLab), store);
}

void GenMCDriver::checkReconsiderFaiSpinloop(const MemAccessLabel *lab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);

		/* Is there any thread blocked on a potential spinloop? */
		auto *eLab = llvm::dyn_cast<FaiZNESpinEndLabel>(g.getLastThreadLabel(i));
		if (!eLab)
			continue;

		/* Check whether this access affects the spinloop variable */
		auto *faiLab = llvm::dyn_cast<FaiWriteLabel>(g.getPreviousLabelST(eLab,
			        [](const EventLabel *lab){ return llvm::isa<FaiWriteLabel>(lab); }));
		if (faiLab->getAddr() != lab->getAddr())
			continue;
		if (llvm::isa<FaiWriteLabel>(lab)) /* FAIs on the same variable are OK... */
			continue;

		/* If it does, and also breaks the assumptions, unblock thread */
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			auto *rfLab = g.getEventLabel(rLab->getRf());
			if (auto *wLab = llvm::dyn_cast<FaiWriteLabel>(rfLab)) {
				if (wLab->getReadersList().size() >= 2)
					thr.unblock();
			}
		} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
			auto &stores = g.getStoresToLoc(wLab->getAddr());
			if (std::all_of(stores.rend(), stores.rbegin(), [&](Event s){
				return !isHbBefore(wLab->getPos(), s);
			}))
				thr.unblock();
		}
	}
	return;
}

llvm::GenericValue
GenMCDriver::visitLoad(InstAttr attr,
		       llvm::AtomicOrdering ord,
		       const llvm::GenericValue *addr,
		       llvm::Type *typ,
		       llvm::GenericValue cmpVal,
		       llvm::GenericValue rmwVal,
		       llvm::AtomicRMWInst::BinOp op)
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	if (inRecoveryMode())
		return getRecReadRetValue(addr, typ);

	if (isExecutionDrivenByGraph())
		return getReadRetValueAndMaybeBlock(EE->getCurrentPosition(), addr, typ);

	/* First, we have to check whether the access is valid. This has to
	 * happen here because we may query the interpreter for this location's
	 * value in order to determine whether this load is going to be an RMW.
	 * Coherence needs to be tracked before validity is established, as
	 * consistency checks may be triggered if the access is invalid */
	g.trackCoherenceAtLoc(addr);
	if (!isAccessValid(addr)) {
		createAddReadLabel(attr, ord, addr, typ, nullptr, cmpVal,
				   rmwVal, op, Event::getInitializer());
		visitError(DE_AccessNonMalloc);
		return GET_ZERO_GV(typ); /* Return some value; this execution will be blocked */
	}

	/* Get an approximation of the stores we can read from */
	auto stores = getStoresToLoc(addr);
	BUG_ON(stores.empty());
	auto validStores = properlyOrderStores(attr, typ, addr, cmpVal, stores);
	if (getConf()->symmetryReduction)
		filterSymmetricStoresSR(addr, typ, validStores);

	/* If this load is annotatable, keep values that will not leed to blocking */
	auto annot = EE->getCurrentAnnotConcretized();
	if (annot.get())
		filterValuesFromAnnotSAVER(addr, typ, annot.get(), validStores);

	/* ... add an appropriate label with a random rf */
	const ReadLabel *lab = createAddReadLabel(attr, ord, addr, typ, std::move(annot),
						  cmpVal, rmwVal, op, validStores[0]);

	/* ... and make sure that the rf we end up with is consistent */
	if (!ensureConsistentRf(lab, validStores))
		return GET_ZERO_GV(typ);

	/* Check whether the load forces us to reconsider some potential spinloop */
	checkReconsiderFaiSpinloop(lab);

	/* Check for races and reading from uninitialized memory */
	checkForDataRaces();
	checkForMemoryRaces(lab->getAddr());
	checkForUninitializedMem(validStores);
	if (llvm::isa<LockCasReadLabel>(lab))
		checkLockValidity(validStores);
	if (llvm::isa<BIncFaiReadLabel>(lab))
		checkBIncValidity(validStores);

	/* If this is the last part of barrier_wait() check whether we should block */
	auto retVal = getWriteValue(validStores[0], addr, typ);
	if (llvm::isa<BWaitReadLabel>(lab) &&
	    !EE->compareValues(typ, retVal, getBarrierInitValue(addr, typ)))
		thr.block(llvm::Thread::BlockageType::BT_Barrier);

	/* Push all the other alternatives choices to the Stack */
	for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
		addToWorklist(LLVM_MAKE_UNIQUE<FRevItem>(lab->getPos(), *it));
	return retVal;
}

const WriteLabel *
GenMCDriver::createAddStoreLabel(InstAttr attr,
				 llvm::AtomicOrdering ord,
				 const llvm::GenericValue *addr,
				 llvm::Type *typ,
				 const llvm::GenericValue &val, int moPos)
{
	auto &g = getGraph();
	auto pos = EE->getCurrentPosition();

	std::unique_ptr<WriteLabel> wLab = nullptr;
	switch (attr) {
	case InstAttr::IA_None:
		wLab = WriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_Unlock:
		wLab = UnlockWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_BInit:
		wLab = BInitWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_BDestroy:
		wLab = BDestroyWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_Fai:
		wLab = FaiWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_NoRetFai:
		wLab = NoRetFaiWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_BPost: {
		/* Barrier hack: reset barrier to initial if it reached 0 */
		auto bVal = (val.IntVal == 0) ? getBarrierInitValue(addr, typ) : val;
		wLab = BIncFaiWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, bVal);
		break;
	}
	case InstAttr::IA_Cas:
		wLab = CasWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_Lock:
		wLab = LockCasWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	case InstAttr::IA_Trylock:
		wLab = TrylockCasWriteLabel::create(g.nextStamp(), ord, pos, addr, typ, val);
		break;
	default:
		BUG();
	}
	updateLabelViews(wLab.get());
	return getGraph().addWriteLabelToGraph(std::move(wLab), moPos);
}

void GenMCDriver::visitStore(InstAttr attr,
			     llvm::AtomicOrdering ord,
			     const llvm::GenericValue *addr,
			     llvm::Type *typ,
			     const llvm::GenericValue &val)

{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto *EE = getEE();
	auto pos = EE->getCurrentPosition();

	/* If it's a valid access, track coherence for this location */
	g.trackCoherenceAtLoc(addr);
	if (!isAccessValid(addr)) {
		createAddStoreLabel(attr, ord, addr, typ, val, 0);
		visitError(DE_AccessNonMalloc);
		return;
	}

	/* Find all possible placings in coherence for this store */
	auto placesRange = g.getCoherentPlacings(addr, pos, isRMWAttr(attr));
	auto &begO = placesRange.first;
	auto &endO = placesRange.second;

	/* It is always consistent to add the store at the end of MO */
	const WriteLabel *lab = createAddStoreLabel(attr, ord, addr, typ, val, endO);

	auto &locMO = g.getStoresToLoc(addr);
	for (auto it = locMO.begin() + begO; it != locMO.begin() + endO; ++it) {

		/* We cannot place the write just before the write of an RMW */
		if (llvm::isa<FaiWriteLabel>(g.getEventLabel(*it)) ||
		    llvm::isa<CasWriteLabel>(g.getEventLabel(*it)))
			continue;

		/* Push the stack item */
		if (!inRecoveryMode()) {
			addToWorklist(LLVM_MAKE_UNIQUE<MOItem>(lab->getPos(), std::distance(locMO.begin(), it)));
		}
	}
	if (!inRecoveryMode())
		calcRevisits(lab);

	/* If the graph is not consistent (e.g., w/ LAPOR) stop the exploration */
	if (!ensureConsistentStore(lab))
		return;

	checkReconsiderFaiSpinloop(lab);

	/* Check for races */
	checkForDataRaces();
	checkForMemoryRaces(lab->getAddr());
	if (llvm::isa<UnlockWriteLabel>(lab))
		checkUnlockValidity();
	if (llvm::isa<BInitWriteLabel>(lab))
		checkBInitValidity();
	return;
}

void GenMCDriver::visitLockLAPOR(const llvm::GenericValue *addr)
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();
	auto lLab = LockLabelLAPOR::create(g.nextStamp(), pos, addr);
	updateLabelViews(lLab.get());
	g.addLockLabelToGraphLAPOR(std::move(lLab));

	threadPrios.insert(threadPrios.begin(), pos);
	return;
}

void GenMCDriver::visitLock(const llvm::GenericValue *addr, llvm::Type *typ)
{
	/* No locking when running the recovery routine */
	if (userConf->persevere && inRecoveryMode())
		return;

	/* Treatment of locks based on whether LAPOR is enabled */
	if (userConf->LAPOR) {
		visitLockLAPOR(addr);
		return;
	}

	auto ret = visitLoad(InstAttr::IA_Lock, llvm::AtomicOrdering::Acquire,
			     addr, typ, INT_TO_GV(typ, 0), INT_TO_GV(typ, 1));

	auto *rLab = llvm::dyn_cast<ReadLabel>(getCurrentLabel());
	if (!rLab->getRf().isBottom() && EE->compareValues(typ, INT_TO_GV(typ, 0), ret)) {
		visitStore(InstAttr::IA_Lock, llvm::AtomicOrdering::Acquire,
			   addr, typ, INT_TO_GV(typ, 1));
	} else {
		EE->getCurThr().block(llvm::Thread::BlockageType::BT_LockAcq);
	}
}

void GenMCDriver::visitUnlockLAPOR(const llvm::GenericValue *addr)
{
	if (isExecutionDrivenByGraph()) {
		deprioritizeThread();
		return;
	}

	auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();
	auto lLab = UnlockLabelLAPOR::create(g.nextStamp(), pos, addr);
	updateLabelViews(lLab.get());
	g.addOtherLabelToGraph(std::move(lLab));
	return;
}

void GenMCDriver::visitUnlock(const llvm::GenericValue *addr, llvm::Type *typ)
{
	/* No locking when running the recovery routine */
	if (userConf->persevere && inRecoveryMode())
		return;

	/* Treatment of unlocks based on whether LAPOR is enabled */
	if (userConf->LAPOR) {
		visitUnlockLAPOR(addr);
		return;
	}

	visitStore(InstAttr::IA_Unlock, llvm::AtomicOrdering::Release, addr, typ, INT_TO_GV(typ, 0));
	return;
}

llvm::GenericValue GenMCDriver::visitMalloc(uint64_t allocSize, unsigned int alignment,
					    Storage s, AddressSpace spc)
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

	/* Get a fresh address and also track this allocation */
	allocBegin.PointerVal = EE->getFreshAddr(allocSize, alignment, s, spc);

	/* Add a relevant label to the graph and return the new address */
	Event pos = EE->getCurrentPosition();
	auto aLab = MallocLabel::create(g.nextStamp(), pos,
					allocBegin.PointerVal, allocSize, s, spc);
	updateLabelViews(aLab.get());
	g.addOtherLabelToGraph(std::move(aLab));
	return allocBegin;
}

void GenMCDriver::visitFree(void *ptr)
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	/* Attempt to free a NULL pointer; don't increase counters */
	if (ptr == NULL)
		return;

	if (isExecutionDrivenByGraph())
		return;

	/* Add a label with the appropriate store */
	Event pos = EE->getCurrentPosition();
	auto dLab = FreeLabel::create(g.nextStamp(), pos, ptr);
	updateLabelViews(dLab.get());
	g.addOtherLabelToGraph(std::move(dLab));

	/* Check whether there is any memory race */
	checkForMemoryRaces(ptr);
	return;
}

void GenMCDriver::visitRCULockLKMM()
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph())
		return;

	auto pos = EE->getCurrentPosition();
	auto lLab = RCULockLabelLKMM::create(g.nextStamp(), pos);
	updateLabelViews(lLab.get());
	g.addOtherLabelToGraph(std::move(lLab));
	return;
}

void GenMCDriver::visitRCUUnlockLKMM()
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph())
		return;

	auto pos = EE->getCurrentPosition();
	auto uLab = RCUUnlockLabelLKMM::create(g.nextStamp(), pos);
	updateLabelViews(uLab.get());
	g.addOtherLabelToGraph(std::move(uLab));
	return;
}

void GenMCDriver::visitRCUSyncLKMM()
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph())
		return;

	Event pos = EE->getCurrentPosition();
	auto gpLab = RCUSyncLabelLKMM::create(g.nextStamp(), pos);
	updateLabelViews(gpLab.get());
	g.addOtherLabelToGraph(std::move(gpLab));
	return;
}

void GenMCDriver::visitError(DriverErrorKind t, const std::string &err /* = "" */,
			     Event confEvent /* = INIT */)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	/* If we this is a replay (might happen if one LLVM instruction
	 * maps to many MC events), do not get into an infinite loop... */
	if (getEE()->getExecState() == llvm::Interpreter::ES_Replay)
		return;

	/* If the execution that led to the error is not consistent, block */
	if (!isConsistent(ProgramPoint::error)) {
		thr.block(llvm::Thread::BlockageType::BT_Error);
		return;
	}
	if (inRecoveryMode() && !isRecoveryValid(ProgramPoint::error)) {
		thr.block(llvm::Thread::BlockageType::BT_Error);
		return;
	}

	const EventLabel *errLab = getCurrentLabel();

	/* If this is an invalid access, change the RF of the offending
	 * event to BOTTOM, so that we do not try to get its value.
	 * Don't bother updating the views */
	if (isInvalidAccessError(t) && llvm::isa<ReadLabel>(errLab))
		g.changeRf(errLab->getPos(), Event::getBottom());

	/* Print a basic error message and the graph */
	llvm::outs() << "Error detected: " << t << "!\n";
	llvm::outs() << "Event " << errLab->getPos() << " ";
	if (!confEvent.isInitializer())
		llvm::outs() << "conflicts with event " << confEvent << " ";
	llvm::outs() << "in graph:\n";
	printGraph(true);

	/* Print error trace leading up to the violating event(s) */
	if (userConf->printErrorTrace) {
		printTraceBefore(errLab->getPos());
		if (!confEvent.isInitializer())
			printTraceBefore(confEvent);
	}

	/* Print the specific error message */
	if (!err.empty())
		llvm::outs() << err << "\n";

	/* Dump the graph into a file (DOT format) */
	if (userConf->dotFile != "")
		dotPrintToFile(userConf->dotFile, errLab->getPos(), confEvent);

	/* Print results and abort */
	printResults();
	exit(EVERIFY);
}

bool GenMCDriver::tryToRevisitLock(const CasReadLabel *rLab, const WriteLabel *sLab,
				   const std::vector<Event> &writePrefixPos,
				   const std::vector<std::pair<Event, Event> > &moPlacings)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (!g.revisitModifiesGraph(rLab, sLab) &&
	    !revisitSetContains(rLab, writePrefixPos, moPlacings)) {
		EE->getThrById(rLab->getThread()).unblock();

		changeRf(rLab->getPos(), sLab->getPos());

		completeRevisitedRMW(rLab);

		threadPrios = {rLab->getPos()};
		if (EE->getThrById(rLab->getThread()).globalInstructions != 0)
			++EE->getThrById(rLab->getThread()).globalInstructions;

		addToRevisitSet(rLab, writePrefixPos, moPlacings);
		return true;
	}
	return false;
}

bool GenMCDriver::calcRevisits(const WriteLabel *sLab)
{
	auto &g = getGraph();

	if (getConf()->symmetryReduction) {
		auto *bLab = llvm::dyn_cast<ThreadStartLabel>(g.getEventLabel(Event(sLab->getThread(), 0)));
		auto tid = bLab->getSymmetricTid();
		if (tid != -1 && sharePrefixSR(tid, sLab->getPos()))
			return true;
	}

	if (auto *faiLab = llvm::dyn_cast<BIncFaiWriteLabel>(sLab)) {
		if (!getConf()->disableBAM &&
		    !getEE()->compareValues(sLab->getType(), sLab->getVal(),
					    getBarrierInitValue(sLab->getAddr(), sLab->getType())))
			return true;
	}

	std::vector<Event> loads = getRevisitLoads(sLab);
	std::vector<Event> pendingRMWs = g.getPendingRMWs(sLab); /* empty or singleton */

	if (pendingRMWs.size() > 0)
		loads.erase(std::remove_if(loads.begin(), loads.end(), [&](Event &e)
					{ auto *confLab = g.getEventLabel(pendingRMWs.back());
					  return g.getEventLabel(e)->getStamp() > confLab->getStamp(); }),
			    loads.end());

	for (auto &l : loads) {
		auto *lab = g.getEventLabel(l);
		BUG_ON(!llvm::isa<ReadLabel>(lab));

		auto *rLab = static_cast<const ReadLabel *>(lab);

		/* Optimize barrier revisits */
		if (auto *faiLab = llvm::dyn_cast<BIncFaiWriteLabel>(sLab)) {
			if (!getConf()->disableBAM &&
			    rLab->getPos() == g.getLastThreadEvent(rLab->getThread())) {
				BUG_ON(!llvm::isa<BWaitReadLabel>(rLab));
				changeRf(rLab->getPos(), faiLab->getPos());
				getEE()->getThrById(rLab->getThread()).unblock();
				continue;
			}
		}

		/* Get the prefix of the write to save */
		auto writePrefix = g.getPrefixLabelsNotBefore(sLab, rLab);
		auto moPlacings = g.saveCoherenceStatus(writePrefix, rLab);

		auto writePrefixPos = g.extractRfs(writePrefix);
		writePrefixPos.insert(writePrefixPos.begin(), sLab->getPos());

		/* Optimize handling of lock operations */
		if (auto *lLab = llvm::dyn_cast<LockCasReadLabel>(rLab)) {
			if (getEE()->getThrById(lLab->getThread()).isBlocked() &&
			    (int) g.getThreadSize(lLab->getThread()) == lLab->getIndex() + 1) {
				if (tryToRevisitLock(lLab, sLab, writePrefixPos, moPlacings))
					continue;
				isMootExecution = true;
			}
		}

		/* If this prefix has revisited the read before, skip */
		if (revisitSetContains(rLab, writePrefixPos, moPlacings))
			continue;

		/* Otherwise, add the prefix to the revisit set and the worklist */
		addToRevisitSet(rLab, writePrefixPos, moPlacings);
		addToWorklist(LLVM_MAKE_UNIQUE<BRevItem>(rLab->getPos(), sLab->getPos(),
							 std::move(writePrefix), std::move(moPlacings)));
	}

	bool consG = !(llvm::isa<CasWriteLabel>(sLab) || llvm::isa<FaiWriteLabel>(sLab)) ||
		pendingRMWs.empty();
	return !isMootExecution && consG;
}

void GenMCDriver::repairLock(LockCasReadLabel *lab)
{
	auto &g = getGraph();
	auto &stores = g.getStoresToLoc(lab->getAddr());

	BUG_ON(stores.empty());
	for (auto rit = stores.rbegin(), re = stores.rend(); rit != re; ++rit) {
		auto *posRf = llvm::dyn_cast<WriteLabel>(g.getEventLabel(*rit));
		if (llvm::isa<LockCasWriteLabel>(posRf) || llvm::isa<TrylockCasWriteLabel>(posRf)) {
			auto prev = posRf->getPos().prev();
			if (g.getMatchingUnlock(prev).isInitializer()) {
				changeRf(lab->getPos(), posRf->getPos());
				threadPrios = { posRf->getPos() };
				return;
			}
		}
	}
	BUG();
}

void GenMCDriver::repairDanglingLocks()
{
	auto &g = getGraph();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto *lLab = llvm::dyn_cast<LockCasReadLabel>(g.getEventLabel(g.getLastThreadEvent(i)));
		if (!lLab)
			continue;
		if (!g.contains(lLab->getRf())) {
			repairLock(lLab);
			break; /* Only one such lock may exist at all times */
		}
	}
	return;
}

void GenMCDriver::repairDanglingBarriers()
{
	auto &g = getGraph();

	/* The wait-load of a barrier may lose its rf after cutting the graph.
	 * If this happens, fix the problem by making it read from the barrier's
	 * increment operation. */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto *bLab = llvm::dyn_cast<BWaitReadLabel>(g.getEventLabel(g.getLastThreadEvent(i)));
		if (!bLab)
			continue;
		if (!g.contains(bLab->getRf())) {
			BUG_ON(!llvm::isa<BIncFaiWriteLabel>(g.getPreviousLabel(bLab)));
			BUG_ON(!g.contains(bLab->getPos()));
			changeRf(bLab->getPos(), bLab->getPos().prev());
		}
	}
	return;
}

const WriteLabel *GenMCDriver::completeRevisitedRMW(const ReadLabel *rLab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	const WriteLabel *sLab = nullptr;
	std::unique_ptr<WriteLabel> wLab = nullptr;
	if (auto *faiLab = llvm::dyn_cast<FaiReadLabel>(rLab)) {
		auto isBarrier = false;
		llvm::GenericValue result;

		/* Need to get the rf value within the if, as rLab might be a disk op,
		 * and we cannot get the value in that case (but it will also not be an RMW)  */
		auto rfVal = getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());
		EE->executeAtomicRMWOperation(result, rfVal, faiLab->getOpVal(),
					      faiLab->getOp());
		if (llvm::isa<BIncFaiReadLabel>(faiLab) && result.IntVal == 0) {
			isBarrier = true;
			result = getBarrierInitValue(rLab->getAddr(), rLab->getType());
		}
		EE->setCurrentDeps(nullptr, nullptr, nullptr, nullptr, nullptr);
		wLab = isBarrier ?
			BIncFaiWriteLabel::create(g.nextStamp(),
						  faiLab->getOrdering(),
						  Event(faiLab->getThread(), faiLab->getIndex() + 1),
						  faiLab->getAddr(),
						  faiLab->getType(), result) :
			(llvm::isa<NoRetFaiReadLabel>(faiLab) ?
			 NoRetFaiWriteLabel::create(g.nextStamp(),
						    faiLab->getOrdering(),
						    Event(faiLab->getThread(), faiLab->getIndex() + 1),
						    faiLab->getAddr(),
						    faiLab->getType(), result) :
			 FaiWriteLabel::create(g.nextStamp(),
					       faiLab->getOrdering(),
					       Event(faiLab->getThread(), faiLab->getIndex() + 1),
					       faiLab->getAddr(),
					       faiLab->getType(), result));
	} else if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		auto isLock = llvm::isa<LockCasReadLabel>(casLab);
		auto rfVal = getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());
		if (EE->compareValues(casLab->getType(), casLab->getExpected(), rfVal)) {
			EE->setCurrentDeps(nullptr, nullptr, nullptr, nullptr, nullptr);
			wLab = isLock ?
				LockCasWriteLabel::create(g.nextStamp(),
							  casLab->getOrdering(),
							  Event(casLab->getThread(), casLab->getIndex() + 1),
							  casLab->getAddr(),
							  casLab->getType(),
							  casLab->getSwapVal()) :
				(llvm::isa<TrylockCasReadLabel>(casLab) ?
				 TrylockCasWriteLabel::create(g.nextStamp(),
							      casLab->getOrdering(),
							      Event(casLab->getThread(), casLab->getIndex() + 1),
							      casLab->getAddr(),
							      casLab->getType(),
							      casLab->getSwapVal()) :
				 CasWriteLabel::create(g.nextStamp(),
						       casLab->getOrdering(),
						       Event(casLab->getThread(), casLab->getIndex() + 1),
						       casLab->getAddr(),
						       casLab->getType(),
						       casLab->getSwapVal()));
		}
	}
	if (wLab) {
		updateLabelViews(wLab.get());
		return g.addWriteLabelToGraph(std::move(wLab), rLab->getRf());
	}
	return nullptr;
}

bool GenMCDriver::revisitReads(std::unique_ptr<WorkItem> item)
{
	auto &g = getGraph();
	auto *EE = getEE();
	const EventLabel *lab = g.getEventLabel(item->getPos());

	/* First, appropriately restrict the worklist, the revisit set, and the graph */
	restrictWorklist(lab);
	restrictRevisitSet(lab);
	restrictGraph(lab);

	/* Handle the MO case first: if we are restricting to a write, change its MO position */
	if (auto *mi = llvm::dyn_cast<MOItem>(item.get())) {
		auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
		BUG_ON(!wLab);
		g.changeStoreOffset(wLab->getAddr(), wLab->getPos(), mi->getMOPos());
		repairDanglingLocks();
		repairDanglingBarriers();
		return (llvm::isa<MOLibItem>(mi)) ? calcLibRevisits(wLab) : calcRevisits(wLab);
	}

	/* Otherwise, handle the revisit case */
	auto *ri = llvm::dyn_cast<RevItem>(item.get());
	BUG_ON(!ri);

	/* Restore the prefix, if this is a backward revisit */
	if (auto *bri = llvm::dyn_cast<BRevItem>(ri))
		restorePrefix(lab, bri->getPrefixRel(), bri->getMOPlacingsRel());

	/* We are dealing with a read: change its reads-from and also check
	 * whether a part of an RMW should be added */
	BUG_ON(!llvm::isa<ReadLabel>(lab));
	auto *rLab = static_cast<const ReadLabel *>(lab);

	getEE()->setCurrentDeps(nullptr, nullptr, nullptr, nullptr, nullptr);
	changeRf(rLab->getPos(), ri->getRev());

	/* Repair barriers here, as dangling wait-reads may be part of the prefix */
	repairDanglingBarriers();

	/* If the revisited label became an RMW, add the store part and revisit */
	if (auto *sLab = completeRevisitedRMW(rLab))
		return calcRevisits(sLab);

	/* Blocked lock -> prioritize locking thread */
	repairDanglingLocks();
	if (auto *lLab = llvm::dyn_cast<LockCasReadLabel>(lab)) {
		threadPrios = {lLab->getRf()};
		EE->getThrById(lab->getThread()).block(llvm::Thread::BlockageType::BT_LockAcq);
	}

	/* Special handling for library revs */
	if (auto *fi = llvm::dyn_cast<FRevLibItem>(ri))
		return calcLibRevisits(lab);

	return true;
}

std::pair<llvm::GenericValue, bool>
GenMCDriver::visitLibLoad(InstAttr attr,
			  llvm::AtomicOrdering ord,
			  const llvm::GenericValue *addr,
			  llvm::Type *typ,
			  std::string functionName)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);

	if (isExecutionDrivenByGraph()) {
		const EventLabel *lab = getCurrentLabel();
		if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab))
			return std::make_pair(getWriteValue(rLab->getRf(), addr, typ),
					      rLab->getRf().isInitializer());
		BUG();
	}

	g.trackCoherenceAtLoc(addr);

	/* Add the event to the graph so we'll be able to calculate consistent RFs */
	Event pos = getEE()->getCurrentPosition();
	auto lLab = LibReadLabel::create(g.nextStamp(), ord, pos, addr, typ,
					 Event::getInitializer(), functionName);
	updateLabelViews(lLab.get());
	auto *lab = g.addReadLabelToGraph(std::move(lLab), Event::getInitializer());

	/* Make sure there exists an initializer event for this memory location */
	auto stores(g.getStoresToLoc(addr));
	auto it = std::find_if(stores.begin(), stores.end(), [&](Event e){
			const EventLabel *eLab = g.getEventLabel(e);
			if (auto *wLab = llvm::dyn_cast<LibWriteLabel>(eLab))
				return wLab->isLibInit();
			BUG();
		});

	if (it == stores.end()) {
		visitError(DE_UninitializedMem,
			   std::string("Uninitialized memory used by library ") +
			   lib->getName() + ", member " + functionName + std::string(" found"));
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
			addToWorklist(LLVM_MAKE_UNIQUE<FRevItem>(lab->getPos(), *it));
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
			addToWorklist(LLVM_MAKE_UNIQUE<FRevLibItem>(lab->getPos(), *it));
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
	changeRf(lab->getPos(), validStores[0]);

	for (auto it = validStores.begin() + 1; it != invIt; ++it)
		addToWorklist(LLVM_MAKE_UNIQUE<FRevItem>(lab->getPos(), *it));

	return std::make_pair(getWriteValue(lab->getRf(), addr, typ),
			      lab->getRf().isInitializer());
}

void GenMCDriver::visitLibStore(InstAttr attr,
				llvm::AtomicOrdering ord,
				const llvm::GenericValue *addr,
				llvm::Type *typ,
				llvm::GenericValue &val,
				std::string functionName,
				bool isInit)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();
	auto lib = Library::getLibByMemberName(getGrantedLibs(), functionName);

	if (isExecutionDrivenByGraph())
		return;

	g.trackCoherenceAtLoc(addr);

	/*
	 * We need to try all possible MO placements, but after the initialization write,
	 * which is explicitly included in MO, in the case of libraries.
	 */
	auto begO = 1;
	auto endO = g.getStoresToLoc(addr).size();

	/* If there was not any store previously, check if this location was initialized.
	 * We only set a flag here and report an error after the relevant event is added   */
	bool isUninitialized = false;
	if (endO == 0 && !isInit)
		isUninitialized = true;

	/* It is always consistent to add a new event at the end of MO */
	Event pos = getEE()->getCurrentPosition();
	auto lLab = LibWriteLabel::create(g.nextStamp(), ord, pos, addr, typ,
					  val, functionName, isInit);
	updateLabelViews(lLab.get());
	auto *sLab = g.addWriteLabelToGraph(std::move(lLab), endO);

	if (isUninitialized) {
		visitError(DE_UninitializedMem,
			   std::string("Uninitialized memory used by library \"") +
			   lib->getName() + "\", member \"" + functionName + std::string("\" found"));
	}

	calcLibRevisits(sLab);

	if (lib && !lib->tracksCoherence())
		return;

	/*
	 * Check for alternative MO placings. Temporarily remove sLab from
	 * MO, find all possible alternatives, and push them to the workqueue
	 */
	const auto &locMO = g.getStoresToLoc(addr);
	auto oldOffset = std::find(locMO.begin(), locMO.end(), sLab->getPos()) - locMO.begin();
	for (auto i = begO; i < endO; ++i) {
		g.changeStoreOffset(addr, sLab->getPos(), i);

		/* Check consistency for the graph with this MO */
		auto preds = g.getViewFromStamp(sLab->getStamp());
		if (g.isLibConsistentInView(*lib, preds)) {
			addToWorklist(LLVM_MAKE_UNIQUE<MOLibItem>(sLab->getPos(), i));
		}
	}
	g.changeStoreOffset(addr, sLab->getPos(), oldOffset);
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
		auto rfs = getLibConsRfsInView(*lib, rLab->getPos(), stores, v);

		for (auto &rf : rfs) {
			/* Push consistent options to stack */
			auto writePrefix = g.getPrefixLabelsNotBefore(lab, rLab);
			auto moPlacings = g.saveCoherenceStatus(writePrefix, rLab);

			auto writePrefixPos = g.extractRfs(writePrefix);
			writePrefixPos.insert(writePrefixPos.begin(), lab->getPos());

			if (revisitSetContains(rLab, writePrefixPos, moPlacings))
				continue;

			addToRevisitSet(rLab, writePrefixPos, moPlacings);
			addToWorklist(LLVM_MAKE_UNIQUE<BRevItem>(rLab->getPos(), rf,
								 std::move(writePrefix), std::move(moPlacings)));

		}
	}
	return valid;
}

llvm::GenericValue
GenMCDriver::visitDskRead(const llvm::GenericValue *addr, llvm::Type *typ)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph()) {
		auto *rLab = llvm::dyn_cast<DskReadLabel>(getCurrentLabel());
		BUG_ON(!rLab);
		return getDskWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());
	}

	/* Make the graph aware of a (potentially) new memory location */
	g.trackCoherenceAtLoc(addr);

	/* Get all stores to this location from which we can read from */
	auto validStores = getStoresToLoc(addr);
	BUG_ON(validStores.empty());

	/* ... and add an appropriate label with a particular rf */
	Event pos = getEE()->getCurrentPosition();
	auto ord = (inRecoveryMode()) ? llvm::AtomicOrdering::Monotonic : llvm::AtomicOrdering::Acquire;
	auto rLab = DskReadLabel::create(g.nextStamp(), ord, pos, addr, typ, validStores[0]);

	updateLabelViews(rLab.get());
	const ReadLabel *lab = g.addReadLabelToGraph(std::move(rLab), validStores[0]);

	/* ... filter out all option that make the recovery invalid */
	filterInvalidRecRfs(lab, validStores);

	/* Push all the other alternatives choices to the Stack */
	for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
		addToWorklist(LLVM_MAKE_UNIQUE<FRevItem>(lab->getPos(), *it));
	return getDskWriteValue(validStores[0], lab->getAddr(), lab->getType());
}

void
GenMCDriver::visitDskWrite(const llvm::GenericValue *addr, llvm::Type *typ,
			   const llvm::GenericValue &val, void *mapping,
			   InstAttr attr /* = IA_None */,
			   std::pair<void *, void *> ordDataRange /* = (NULL, NULL) */,
			   void *transInode /* = NULL */)
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto *EE = getEE();
	auto pos = EE->getCurrentPosition();

	g.trackCoherenceAtLoc(addr);

	/* Disk writes should always be hb-ordered */
	auto placesRange = g.getCoherentPlacings(addr, pos, false);
	auto &begO = placesRange.first;
	auto &endO = placesRange.second;
	BUG_ON(begO != endO);

	/* Safe to _only_ add it at the end of MO */
	std::unique_ptr<DskWriteLabel> wLab = nullptr;
	auto ord = llvm::AtomicOrdering::Release;
	switch (attr) {
	case InstAttr::IA_None:
		wLab = DskWriteLabel::create(g.nextStamp(), ord, pos,
					     addr, typ, val, mapping);
		break;
	case InstAttr::IA_DskMdata:
		wLab = DskMdWriteLabel::create(g.nextStamp(), ord, pos,
					       addr, typ, val, mapping, ordDataRange);
		break;
	case InstAttr::IA_DskDirOp:
		wLab = DskDirWriteLabel::create(g.nextStamp(), ord, pos,
						addr, typ, val, mapping);
		break;
	case InstAttr::IA_DskJnlOp:
		wLab = DskJnlWriteLabel::create(g.nextStamp(), ord, pos,
						addr, typ, val, mapping, transInode);
		break;
	default:
		BUG();
	}

	updateLabelViews(wLab.get());
	const WriteLabel *lab = g.addWriteLabelToGraph(std::move(wLab), endO);

	calcRevisits(lab);
	return;
}

llvm::GenericValue
GenMCDriver::visitDskOpen(const char *fileName, llvm::Type *intTyp)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph()) {
		const EventLabel *lab = getCurrentLabel();
		if (auto *oLab = llvm::dyn_cast<DskOpenLabel>(lab)) {
			return oLab->getFd();
		}
		BUG();
	}

	/* We get a fresh file descriptor for this open() */
	auto fd = EE->getFreshFd();
	ERROR_ON(fd == -1, "Too many calls to open()!\n");

	/* We add a relevant label to the graph... */
	llvm::GenericValue fdR;
	fdR.IntVal = llvm::APInt(intTyp->getIntegerBitWidth(), fd);
	Event pos = EE->getCurrentPosition();
	auto oLab = DskOpenLabel::create(g.nextStamp(), pos, fileName, fdR);
	updateLabelViews(oLab.get());
	g.addOtherLabelToGraph(std::move(oLab));

	/* Return the freshly allocated fd */
	return fdR;
}

void GenMCDriver::visitDskFsync(void *inode, unsigned int size)
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();
	auto fLab = DskFsyncLabel::create(g.nextStamp(), pos, inode, size);
	updateLabelViews(fLab.get());
	g.addOtherLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::visitDskSync()
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto pos = getEE()->getCurrentPosition();
	auto sncLab = DskSyncLabel::create(g.nextStamp(), pos);
	updateLabelViews(sncLab.get());
	g.addOtherLabelToGraph(std::move(sncLab));
	return;
}

void GenMCDriver::visitDskPbarrier()
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto  pos = getEE()->getCurrentPosition();
	auto dpLab = DskPbarrierLabel::create(g.nextStamp(), pos);
	updateLabelViews(dpLab.get());
	g.addOtherLabelToGraph(std::move(dpLab));
	return;
}

void GenMCDriver::visitLoopBegin()
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto  pos = getEE()->getCurrentPosition();
	auto lbLab = LoopBeginLabel::create(g.nextStamp(), pos);
	updateLabelViews(lbLab.get());
	g.addOtherLabelToGraph(std::move(lbLab));
	return;
}

bool GenMCDriver::isWriteObservable(const WriteLabel *wLab)
{
	if (wLab->isAtLeastRelease() || !getEE()->isDynamic(wLab->getAddr()))
		return true;

	auto &g = getGraph();
	auto *mLab = g.getPreviousLabelST(wLab, [wLab](const EventLabel *lab){
		if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
			if (aLab->getAllocAddr() <= wLab->getAddr() &&
			    ((char *) aLab->getAllocAddr() + aLab->getAllocSize() >
			     (char *) wLab->getAddr()))
				return true;
		}
		return false;
	});
	if (mLab == nullptr)
		return true;

	for (auto j = mLab->getIndex() + 1; j < wLab->getIndex(); j++)
		if (g.getEventLabel(Event(wLab->getThread(), j))->isAtLeastRelease())
			return true;
	return false;
}

void GenMCDriver::visitSpinStart()
{
	auto &g = getGraph();
	const EventLabel *stLab = nullptr;

	/* If it has not been added to the graph, do so */
	if (!isExecutionDrivenByGraph()) {
		auto pos = getEE()->getCurrentPosition();
		auto lab = SpinStartLabel::create(g.nextStamp(), pos);
		updateLabelViews(lab.get());
		stLab = g.addOtherLabelToGraph(std::move(lab));
	} else {
		stLab = g.getEventLabel(getEE()->getCurrentPosition());
	}

	/* Check whether we can detect some spinloop dynamically */
	auto *lbLab = g.getPreviousLabelST(stLab, [](const EventLabel *lab){
		return llvm::isa<LoopBeginLabel>(lab);
	});
	/* If we did not find a loop-begin, this a manual instrumentation(?); report to user */
	ERROR_ON(!lbLab, "No loop-beginning found!\n");

	auto *pLab = g.getPreviousLabelST(stLab, [lbLab](const EventLabel *lab){
		return llvm::isa<SpinStartLabel>(lab) && lab->getIndex() > lbLab->getIndex();
	});
	if (!pLab)
		return;

	for (auto i = pLab->getIndex() + 1; i < stLab->getIndex(); i++) {
		auto *wLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(Event(stLab->getThread(), i)));
		if (wLab && isWriteObservable(wLab))
			return; /* found event w/ side-effects */
	}
	/* Spinloop detected */
	getEE()->getCurThr().block(llvm::Thread::BlockageType::BT_Spinloop);
	return;
}

bool GenMCDriver::areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab)
{
	auto &g = getGraph();

	auto *wLab = llvm::dyn_cast<FaiWriteLabel>(
		g.getPreviousLabelST(lab, [](const EventLabel *lab){ return llvm::isa<FaiWriteLabel>(lab); }));
	BUG_ON(!wLab);

	auto &stores = g.getStoresToLoc(wLab->getAddr());
	BUG_ON(stores.empty());

	/* All stores in the RMW chain need to be read from at most 1 read,
	 * and there need to be no other stores that are not hb-before lab */
	for (auto it = stores.begin(), ie = stores.end(); it != ie; ++it) {
		auto *sLab = static_cast<const WriteLabel *>(g.getEventLabel(*it));
		if (auto *faiLab = llvm::dyn_cast<FaiWriteLabel>(sLab)) {
			if (faiLab->getReadersList().size() >= 2)
				return false;
		} else {
			if (!isHbBefore(sLab->getPos(), wLab->getPos()))
				return false;
		}
	}
	return true;
}

void GenMCDriver::visitFaiZNESpinEnd()
{
	auto &g = getGraph();
	auto *EE = getEE();

	/* If there are more events after this one, it is not a spin loop*/
	if (isExecutionDrivenByGraph() &&
	    EE->getCurrentPosition().index < g.getLastThreadEvent(EE->getCurrentPosition().thread).index)
		return;

	auto pos = EE->getCurrentPosition();
	auto lab = FaiZNESpinEndLabel::create(g.nextStamp(), pos);
	updateLabelViews(lab.get());
	auto *eLab = g.addOtherLabelToGraph(std::move(lab)); /* might overwrite but that's ok */

	if (areFaiZNEConstraintsSat(llvm::dyn_cast<FaiZNESpinEndLabel>(eLab)))
		getEE()->getCurThr().block(llvm::Thread::BlockageType::BT_ZNESpinloop);
	return;
}

void GenMCDriver::visitLockZNESpinEnd()
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph())
		return;

	auto pos = EE->getCurrentPosition();
	auto lab = LockZNESpinEndLabel::create(g.nextStamp(), pos);
	updateLabelViews(lab.get());
	auto *eLab = g.addOtherLabelToGraph(std::move(lab));

	getEE()->getCurThr().block(llvm::Thread::BlockageType::BT_ZNESpinloop);
	return;
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
	case GenMCDriver::DE_Recovery:
		return s << "Recovery error";
	case GenMCDriver::DE_Liveness:
		return s << "Liveness violation";
	case GenMCDriver::DE_RaceNotAtomic:
		return s << "Non-Atomic race";
	case GenMCDriver::DE_RaceFreeMalloc:
		return s << "Malloc-Free race";
	case GenMCDriver::DE_FreeNonMalloc:
		return s << "Attempt to free non-allocated memory";
	case GenMCDriver::DE_DoubleFree:
		return s << "Double-free error";
	case GenMCDriver::DE_Allocation:
		return s << "Allocation error";
	case GenMCDriver::DE_UninitializedMem:
		return s << "Attempt to read from uninitialized memory";
	case GenMCDriver::DE_AccessNonMalloc:
		return s << "Attempt to access non-allocated memory";
	case GenMCDriver::DE_AccessFreed:
		return s << "Attempt to access freed memory";
	case GenMCDriver::DE_InvalidJoin:
		return s << "Invalid join() operation";
	case GenMCDriver::DE_InvalidUnlock:
		return s << "Invalid unlock() operation";
	case GenMCDriver::DE_InvalidBInit:
		return s << "Invalid barrier_init() operation";
	case GenMCDriver::DE_InvalidRecoveryCall:
		return s << "Invalid function call during recovery";
	case GenMCDriver::DE_InvalidTruncate:
		return s << "Invalid file truncation";
	case GenMCDriver::DE_SystemError:
		return s << errorList.at(systemErrorNumber);
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
	LLVM_VECTOR_TYPEID_CASES {					\
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
			   llvm::raw_ostream &s = llvm::outs())
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
			   llvm::raw_ostream &s = llvm::outs())
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
			   llvm::raw_ostream &s = llvm::outs())
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
			   llvm::raw_ostream &os = llvm::outs())
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

	/* Similarly for allocations that don't come from malloc() */
	if (auto *mLab = llvm::dyn_cast<MallocLabel>(lab))
		return mLab->getStorage() == Storage::ST_Heap &&
		       mLab->getAddrSpace() != AddressSpace::AS_Internal;

	return true;
}

void GenMCDriver::printGraph(bool getMetadata /* false */)
{
	auto &g = getGraph();

	if (getMetadata)
		EE->replayExecutionBefore(g.getViewFromStamp(g.nextStamp()));

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);
		llvm::outs() << thr << ":\n";
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			llvm::outs() << "\t";
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				auto name = EE->getVarName(rLab->getAddr());
				auto val = llvm::isa<DskReadLabel>(rLab) ?
					getDskWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType()) :
					getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());
				executeRLPrint(rLab, name, val);
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				auto name = EE->getVarName(wLab->getAddr());
				executeWLPrint(wLab, name);
			} else {
				llvm::outs() << *lab;
			}
			if (getMetadata && thr.prefixLOC[j].first && shouldPrintLOC(lab)) {
				executeMDPrint(lab, thr.prefixLOC[j], getConf()->inputFile);
			}
			llvm::outs() << "\n";
		}
	}
	llvm::outs() << "\n";
}

void GenMCDriver::prettyPrintGraph()
{
	const auto &g = getGraph();
	auto *EE = getEE();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);
		llvm::outs() << "<" << thr.parentId << "," << thr.id
			     << "> " << thr.threadFun->getName() << ": ";
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->isRevisitable())
					llvm::outs().changeColor(llvm::raw_ostream::Colors::GREEN);
				auto val = llvm::isa<DskReadLabel>(rLab) ?
					getDskWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType()) :
					getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getType());
				llvm::outs() << "R" << EE->getVarName(rLab->getAddr()) << ","
					     << val.IntVal << " ";
				llvm::outs().resetColor();
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				llvm::outs() << "W" << EE->getVarName(wLab->getAddr()) << ","
					     << wLab->getVal().IntVal << " ";
			}
		}
		llvm::outs() << "\n";
	}
	llvm::outs() << "\n";
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
				      llvm::raw_ostream &ss /* llvm::outs() */)
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
	llvm::outs() << "Trace to " << e << ":\n";

	/* Replay the execution up to the error event (collects mdata).
	 * Even if the prefix has holes, replaying will fill them up,
	 * so we end up with a (po U rf) view of the offending execution */
	const VectorClock &before = getGraph().getPrefixView(e);
	getEE()->replayExecutionBefore(before);

	/* Linearize (po U rf) and print trace */
	View a;
	recPrintTraceBefore(e, a);
}
