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
#include "GraphIterators.hpp"
#include "LabelVisitor.hpp"
#include "Parser.hpp"
#include "SExprVisitor.hpp"
#include "ThreadPool.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/raw_os_ostream.h>

#include <algorithm>
#include <csignal>

/************************************************************
 ** GENERIC MODEL CHECKING DRIVER
 ***********************************************************/

GenMCDriver::GenMCDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
			 std::unique_ptr<ModuleInfo> MI)
	: userConf(conf), result(), isMootExecution(false), readToReschedule(Event::getInitializer()),
	  shouldHalt(false)
{
	std::string buf;

	/* Create an interpreter for the program's instructions */
	EE = std::unique_ptr<llvm::Interpreter>((llvm::Interpreter *)
		llvm::Interpreter::create(std::move(mod), std::move(MI), this, getConf(), &buf));

	/* Set up an suitable execution graph with appropriate relations */
	execGraph = GraphBuilder(userConf->isDepTrackingModel, userConf->warnOnGraphSize)
		.withCoherenceType(userConf->coherence)
		.withEnabledLAPOR(userConf->LAPOR)
		.withEnabledPersevere(userConf->persevere, userConf->blockSize)
		.withEnabledBAM(!userConf->disableBAM).build();

	/* Set up a random-number generator (for the scheduler) */
	std::random_device rd;
	auto seedVal = (userConf->randomScheduleSeed != "") ?
		(MyRNG::result_type) stoull(userConf->randomScheduleSeed) : rd();
	if (userConf->printRandomScheduleSeed)
		llvm::outs() << "Seed: " << seedVal << "\n";
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

GenMCDriver::~GenMCDriver() = default;

GenMCDriver::LocalState::~LocalState() = default;

GenMCDriver::LocalState::LocalState(std::unique_ptr<ExecutionGraph> g, RevisitSetT &&r, LocalQueueT &&w,
				    std::unique_ptr<llvm::EELocalState> interpState, bool isMootExecution,
				    Event readToReschedule, const std::vector<Event> &threadPrios)
	: graph(std::move(g)), revset(std::move(r)), workqueue(std::move(w)),
	  interpState(std::move(interpState)), isMootExecution(isMootExecution),
	  readToReschedule(readToReschedule), threadPrios(threadPrios) {}

std::unique_ptr<GenMCDriver::LocalState> GenMCDriver::releaseLocalState()
{
	return std::make_unique<GenMCDriver::LocalState>(
		std::move(execGraph), std::move(revisitSet), std::move(workqueue),
		getEE()->releaseLocalState(), isMootExecution, readToReschedule, threadPrios);
}

void GenMCDriver::restoreLocalState(std::unique_ptr<GenMCDriver::LocalState> state)
{
	execGraph = std::move(state->graph);
	revisitSet = std::move(state->revset);
	workqueue = std::move(state->workqueue);
	getEE()->restoreLocalState(std::move(state->interpState));
	isMootExecution = state->isMootExecution;
	readToReschedule = state->readToReschedule;
	threadPrios = std::move(state->threadPrios);
	return;
}

GenMCDriver::SharedState::~SharedState() = default;

GenMCDriver::SharedState::SharedState(std::unique_ptr<ExecutionGraph> g,
				      std::unique_ptr<llvm::EESharedState> interpState)
	: graph(std::move(g)), interpState(std::move(interpState)) {}

std::unique_ptr<GenMCDriver::SharedState> GenMCDriver::getSharedState()
{
	return std::make_unique<GenMCDriver::SharedState>(
		std::move(execGraph), getEE()->getSharedState());
}

void GenMCDriver::setSharedState(std::unique_ptr<GenMCDriver::SharedState> state)
{
	execGraph = std::move(state->graph);
	getEE()->setSharedState(std::move(state->interpState));
	return;
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
			EE->getThrById(i).block(BlockageType::LockNotRel);
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
		    std::none_of(EE->threads_begin(), EE->threads_end(), [](const llvm::Thread &thr){
				    return thr.getBlockageType() == BlockageType::Cons; }))
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
	auto *lab = getGraph().getLastThreadLabel(thread);
	return !thr.ECStack.empty() && !thr.isBlocked() && !EventLabel::denotesThreadEnd(lab);
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
		EE->scheduleThread(e.thread);
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
		EE->scheduleThread(i);
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
			EE->scheduleThread(i);
			return true;
		}
	}

	/* Otherwise, try to schedule the fallback thread */
	if (fallback != -1) {
		EE->scheduleThread(fallback);
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
			auto *bLab = llvm::dyn_cast<const ThreadStartLabel>(g.getEventLabel(Event(i, 0)));
			auto symm = bLab->getSymmetricTid();
			if (symm != -1 && isSchedulable(symm) &&
			    g.getThreadSize(symm) <= g.getThreadSize(i)) {
				EE->scheduleThread(symm);
				return true;
			}
		}

		/* Found a not-yet-complete thread; schedule it */
		EE->scheduleThread(i);
		return true;
	}

	/* No schedulable thread found */
	return false;
}

void GenMCDriver::deprioritizeThread(const UnlockLabelLAPOR *uLab)
{
	/* Extra check to make sure the function is properly used */
	if (!userConf->LAPOR)
		return;

	auto &g = getGraph();

	auto delIt = threadPrios.end();
	for (auto it = threadPrios.begin(); it != threadPrios.end(); ++it) {
		auto *lLab = llvm::dyn_cast<LockLabelLAPOR>(g.getEventLabel(*it));
		BUG_ON(!lLab);

		if (lLab->getThread() == uLab->getThread() &&
		    lLab->getLockAddr() == uLab->getLockAddr()) {
			delIt = it;
			break;
		}
	}

	if (delIt != threadPrios.end())
		threadPrios.erase(delIt);
	return;
}

void GenMCDriver::resetExplorationOptions()
{
	unmoot();
	setRescheduledRead(Event::getInitializer());
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
	}

	/* Then, set up thread prioritization and interpreter's state */
	prioritizeThreads();
}

void GenMCDriver::handleExecutionInProgress()
{
	/* Check if there are checks to be done while running */
	GENMC_DEBUG(
		if (userConf->validateExecGraphs)
			getGraph().validate();
	);
	return;
}

void GenMCDriver::checkHelpingCasAnnotation()
{
	/* If we were waiting for a helped CAS that did not appear, complain */
	if (std::any_of(getEE()->threads_begin(), getEE()->threads_end(),
			 [](const llvm::Thread &thr) {
				return thr.getBlockageType() == BlockageType::HelpedCas;
			 }))
		ERROR("Helped/Helping CAS annotation error! Does helped CAS always execute?\n");

	auto &g = getGraph();
	auto *EE = getEE();

	/* Next, we need to check whether there are any extraneous
	 * stores, not visible to the helped/helping CAS */
	auto hs = g.collectAllEvents([&](const EventLabel *lab){ return llvm::isa<HelpingCasLabel>(lab); });
	if (hs.empty())
		return;

	for (auto &h : hs) {
		auto *hLab = llvm::dyn_cast<HelpingCasLabel>(g.getEventLabel(h));
		BUG_ON(!hLab);

		/* Check that all stores that would make this helping
		 * CAS succeed are read by a helped CAS.
		 * We don't need to check the swap value of the helped CAS */
		if (std::any_of(store_begin(g, hLab->getAddr()), store_end(g, hLab->getAddr()),
				[&](const Event &s){
			auto *sLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
			return hLab->getExpected() == sLab->getVal() &&
				std::none_of(sLab->getReadersList().begin(), sLab->getReadersList().end(),
					    [&](const Event &r){
						    return llvm::isa<HelpedCasReadLabel>(g.getEventLabel(r));
					    });
		}))
			ERROR("Helped/Helping CAS annotation error! "
			      "Unordered store to helping CAS location!\n");

		/* Special case for the initializer (as above) */
		if (hLab->getAddr().isStatic() &&
		    hLab->getExpected() == EE->getLocInitVal(hLab->getAddr(), hLab->getAccess())) {
			auto rs = g.collectAllEvents([&](const EventLabel *lab){
				auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
				return rLab && rLab->getAddr() == hLab->getAddr();
			});
			if (std::none_of(rs.begin(), rs.end(), [&](const Event &r){
				return llvm::isa<HelpedCasReadLabel>(g.getEventLabel(r));
			}))
				ERROR("Helped/Helping CAS annotation error! "
				      "Unordered store to helping CAS location!\n");
		}
	}
	return;
}

bool GenMCDriver::isExecutionBlocked() const
{
	return std::any_of(getEE()->threads_begin(), getEE()->threads_end(),
			   [this](const llvm::Thread &thr){
				   auto *bLab = llvm::dyn_cast<BlockLabel>(getGraph().getLastThreadLabel(thr.id));
				   return bLab || thr.isBlocked(); });
}

void GenMCDriver::handleFinishedExecution()
{
	/* LAPOR: Check lock-well-formedness */
	if (getConf()->LAPOR && !isLockWellFormedLAPOR())
		WARN_ONCE("lapor-not-well-formed", "Execution not lock-well-formed!\n");

	/* Helper: Check helping CAS annotation */
	if (getConf()->helper && !isMoot())
		checkHelpingCasAnnotation();

	/* Ignore the execution if some assume has failed */
	if (isExecutionBlocked() || isMoot()) {
		GENMC_DEBUG(
			if (getConf()->printBlockedExecs)
				printGraph();
		);
		++result.exploredBlocked;
		if (isMoot())
			++result.exploredMoot;
		if (getConf()->checkLiveness && !isMoot())
			checkLiveness();
		return;
	}

	if (getConf()->checkConsPoint == ProgramPoint::exec &&
	    !isConsistent(ProgramPoint::exec))
		return;
	if (getConf()->printExecGraphs && !getConf()->persevere)
		printGraph(); /* Delay printing if persevere is enabled */
	++result.explored;
	return;
}

void GenMCDriver::handleRecoveryStart()
{
	if (isExecutionBlocked())
		return;

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
	updateLabelViews(tsLab.get(), nullptr);
	g.addOtherLabelToGraph(std::move(tsLab));

	/* Create a thread for the interpreter, and appropriately
	 * add it to the thread list (pthread_create() style) */
	EE->createAddRecoveryThread(tid);

	/* Finally, do all necessary preparations in the interpreter */
	getEE()->setupRecoveryRoutine(tid);
	return;
}

void GenMCDriver::handleRecoveryEnd()
{
	/* Print the graph with the recovery routine */
	if (getConf()->printExecGraphs)
		printGraph();
	getEE()->cleanupRecoveryRoutine(getGraph().getRecoveryRoutineId());
	return;
}

void GenMCDriver::run()
{
	/* Explore all graphs and print the results */
	explore();
	return;
}

bool GenMCDriver::isHalting() const
{
	auto *tp = getThreadPool();
	return shouldHalt || (tp && tp->shouldHalt());
}

void GenMCDriver::halt(Status status)
{
	getEE()->block(BlockageType::Error);
	shouldHalt = true;
	result.status = status;
	workqueue.clear();
	if (getThreadPool())
		getThreadPool()->halt();
}

GenMCDriver::Result GenMCDriver::verify(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod)
{
	auto MI = std::make_unique<ModuleInfo>(*mod);

	/* Prepare the module for verification */
	LLVMModule::transformLLVMModule(*mod, *MI, conf);
	if (conf->transformFile != "")
		LLVMModule::printLLVMModule(*mod, conf->transformFile);

	if (conf->threads == 1) {
		auto driver = DriverFactory::create(conf, std::move(mod), std::move(MI));
		driver->run();
		return driver->getResult();
	}

	std::vector<std::future<GenMCDriver::Result>> futures;
	{
		/* Then, fire up the drivers */
		ThreadPool tp(conf, mod, MI);
		futures = tp.waitForTasks();
	}

	GenMCDriver::Result res;
	for (auto &f : futures)
		res += f.get();
	return res;
}

void GenMCDriver::addToWorklist(WorkSet::ItemT item)

{
	const auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(item->getPos());

	workqueue[lab->getStamp()].add(std::move(item));
	return;
}

WorkSet::ItemT GenMCDriver::getNextItem()
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

void GenMCDriver::notifyEERemoved(const VectorClock &v)
{
	const auto &g = getGraph();
	for (auto *lab : labels(g)) {
		if (v.contains(lab->getPos()))
			continue;

		/* For persistency, reclaim fds */
		if (auto *oLab = llvm::dyn_cast<DskOpenLabel>(lab))
			getEE()->reclaimUnusedFd(oLab->getFd().get());
	}
}

void GenMCDriver::restrictGraph(const EventLabel *rLab)
{
	/* Inform the interpreter about deleted events, and then
	 * restrict the graph (and relations) */
	notifyEERemoved(*getGraph().getPredsView(rLab->getPos()));
	getGraph().cutToStamp(rLab->getStamp());
	getGraph().resetStamp(rLab->getStamp() + 1);
	return;
}


/************************************************************
 ** Scheduling methods
 ***********************************************************/

bool GenMCDriver::scheduleNormal()
{
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

bool GenMCDriver::rescheduleReads()
{
	auto &g = getGraph();
	auto *EE = getEE();

	for (auto i = 0u; i < g.getNumThreads(); ++i) {
		auto *bLab = llvm::dyn_cast<BlockLabel>(g.getLastThreadLabel(i));
		if (!bLab || bLab->getType() != BlockageType::ReadOptBlock)
			continue;
		auto *pLab = llvm::dyn_cast<ReadLabel>(g.getPreviousLabel(bLab));
		BUG_ON(!pLab);

		setRescheduledRead(pLab->getPos());
		g.remove(bLab);
		g.remove(pLab);

		EE->resetThread(i);
		EE->getThrById(i).ECStack = { EE->getThrById(i).initSF };
		EE->scheduleThread(i);
		return true;
	}
	return false;
}

bool GenMCDriver::scheduleNext()
{
	if (isMoot() || isHalting())
		return false;

	auto &g = getGraph();
	auto *EE = getEE();

	/* First, check if we should prioritize some thread */
	if (schedulePrioritized())
		return true;

	/* Then, schedule the next thread according to the chosen policy */
	if (scheduleNormal())
		return true;

	/* Finally, check if any reads needs to be rescheduled */
	return rescheduleReads();
}

void GenMCDriver::explore()
{
	auto *EE = getEE();

	unmoot(); /* recursive calls */
	BUG_ON(!readToReschedule.isInitializer());
	while (true) {
		EE->reset();

		/* Get main program function and run the program */
		EE->runAsMain(getConf()->programEntryFun);
		if (getConf()->persevere)
			EE->runRecovery();

		auto validExecution = true;
		do {
			/*
			 * restrictAndRevisit() might deem some execution infeasible,
			 * so we have to reset all exploration options before
			 * calling it again
			 */
			resetExplorationOptions();

			auto item = getNextItem();
			if (!item) {
				EE->reset();  /* To free memory */
				return;
			}
			validExecution = restrictAndRevisit(std::move(item)) && isConsistent(ProgramPoint::step);
		} while (!validExecution);
	}
}

bool GenMCDriver::isExecutionDrivenByGraph()
{
	const auto &g = getGraph();
	auto curr = getEE()->incPos();
	return (curr.index < g.getThreadSize(curr.thread)) &&
		!llvm::isa<EmptyLabel>(g.getEventLabel(curr));
}

bool GenMCDriver::inRecoveryMode() const
{
	return getEE()->getProgramState() == llvm::ProgramState::Recovery;
}

bool GenMCDriver::inReplay() const
{
	return getEE()->getExecState() == llvm::ExecutionState::Replay;
}

const EventLabel *GenMCDriver::getCurrentLabel() const
{
	const auto &g = getGraph();
	auto pos = getEE()->currPos();

	BUG_ON(!g.contains(pos));
	return g.getEventLabel(pos);
}

/* Given an event in the graph, returns the value of it */
SVal GenMCDriver::getWriteValue(Event write, SAddr addr, AAccess access)
{
	/* If the even represents an invalid access, return some value */
	if (write.isBottom())
		return SVal();

	/* If the event is the initializer, ask the interpreter about
	 * the initial value of that memory location */
	if (write.isInitializer())
		return getEE()->getLocInitVal(addr, access);

	/* Otherwise, we will get the value from the execution graph */
	auto *wLab = llvm::dyn_cast<WriteLabel>(getGraph().getEventLabel(write));
	BUG_ON(!wLab);

	/* It can be the case that the load's type is different than
	 * the one the write's (see troep.c).  In any case though, the
	 * sizes should match */
	if (wLab->getSize() != access.getSize())
		visitError(wLab->getPos(), Status::VS_MixedSize,
			   "Mixed-size accesses detected: tried to read event with a " +
			   std::to_string(access.getSize().get() * 8) + "-bit access!\n" +
			   "Please check the LLVM-IR.\n");

	/* If the size of the R and the W are the same, we are done */
	return wLab->getVal();
}

/* Same as above, but the data of a file are not explicitly initialized
 * so as not to pollute the graph with events, since a file can be large.
 * Thus, we treat the case where WRITE reads INIT specially. */
SVal GenMCDriver::getDskWriteValue(Event write, SAddr addr, AAccess access)
{
	if (write.isInitializer())
		return SVal();
	return getWriteValue(write, addr, access);
}

SVal GenMCDriver::getBarrierInitValue(SAddr addr, AAccess access)
{
	const auto &g = getGraph();
	auto sIt = std::find_if(store_begin(g, addr), store_end(g, addr), [&addr,&g](const Event &s){
		auto *bLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
		BUG_ON(!bLab);
		return bLab->getAddr() == addr && bLab->isNotAtomic();
	});

	/* All errors pertinent to initialization should be captured elsewhere */
	BUG_ON(sIt == store_end(g, addr));
	return getWriteValue(*sIt, addr, access);
}

SVal GenMCDriver::getReadRetValueAndMaybeBlock(const ReadLabel *rLab)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	/* Fetch appropriate return value and check whether we should block */
	auto res = getReadValue(rLab);
	if (rLab->getRf().isBottom()) {
		/* Bottom is an acceptable re-option only @ replay; block anyway */
		BUG_ON(!inReplay());
		thr.block(BlockageType::Error);
	} else if (llvm::isa<BWaitReadLabel>(rLab) &&
		   res != getBarrierInitValue(rLab->getAddr(), rLab->getAccess())) {
		/* Reading a non-init barrier value means that the thread should block */
		thr.block(BlockageType::Barrier);
	}
	return res;
}

SVal GenMCDriver::getRecReadRetValue(const ReadLabel *rLab)
{
	auto &g = getGraph();
	auto rf = g.getLastThreadStoreAtLoc(rLab->getPos(), rLab->getAddr());
	BUG_ON(rf.isInitializer());
	return getWriteValue(rf, rLab->getAddr(), rLab->getAccess());
}

CheckConsType GenMCDriver::getCheckConsType(ProgramPoint p) const
{
	/* Always check consistency on error, or at user-specified points.
	 * Assume that extensions that require more extensive checks have
	 * enabled them during config */
	if (p <= getConf()->checkConsPoint)
		return (p == ProgramPoint::error ? CheckConsType::full : getConf()->checkConsType);
	return CheckConsType::fast;
}

bool GenMCDriver::shouldCheckPers(ProgramPoint p)
{
	/* Always check consistency on error, or at user-specified points */
	return p <= getConf()->checkPersPoint;
}

bool GenMCDriver::isHbBefore(Event a, Event b, ProgramPoint p /* = step */)
{
	return getGraph().isHbBefore(a, b, getCheckConsType(p));
}

bool GenMCDriver::isCoMaximal(SAddr addr, Event e, bool checkCache /* = false */,
			      ProgramPoint p /* = step */)
{
	return getGraph().isCoMaximal(addr, e, checkCache, getCheckConsType(p));
}

bool GenMCDriver::isHazptrProtected(const MallocLabel *aLab, const MemAccessLabel *mLab) const
{
	auto &g = getGraph();
	BUG_ON(!mLab->getAddr().isDynamic());

	auto *pLab = llvm::dyn_cast_or_null<HpProtectLabel>(
		g.getPreviousLabelST(mLab, [&](const EventLabel *lab){
			auto *pLab = llvm::dyn_cast<HpProtectLabel>(lab);
			return pLab && aLab->contains(pLab->getProtectedAddr());
	}));
	if (!pLab)
		return false;

	for (auto j = pLab->getIndex() + 1; j < mLab->getIndex(); j++) {
		auto *lab = g.getEventLabel(Event(mLab->getThread(), j));
		if (auto *oLab = dyn_cast<HpProtectLabel>(lab))
			if (oLab->getHpAddr() == pLab->getHpAddr())
				return false;
	}
	return true;
}

bool GenMCDriver::checkForMemoryRaces(const MemAccessLabel *mLab)
{
	if (!mLab->getAddr().isDynamic())
		return false;

	const auto &g = getGraph();
	const View &before = g.getEventLabel(mLab->getPos().prev())->getHbView();
	const MallocLabel *allocLab = nullptr;
	const WriteLabel *initLab = nullptr;
	const FreeLabel *potHazLab = nullptr;

	for (const auto *oLab : labels(g)) {
		if (auto *fLab = llvm::dyn_cast<FreeLabel>(oLab)) {
			if (fLab->contains(mLab->getAddr())) {
				if (!llvm::isa<HpRetireLabel>(fLab)) {
					visitError(mLab->getPos(), Status::VS_AccessFreed, "", oLab->getPos());
					return true;
				}
				potHazLab = fLab;
			}
		}
		if (auto *aLab = llvm::dyn_cast<MallocLabel>(oLab)) {
			if (aLab->contains(mLab->getAddr())) {
				if (!before.contains(aLab->getPos())) {
					visitError(mLab->getPos(), Status::VS_AccessNonMalloc,
						   "The allocating operation (malloc()) "
						   "does not happen-before the memory access!",
						   oLab->getPos());
					return true;
				} else {
					allocLab = aLab;
				}
			}
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(oLab)) {
			if (wLab->getAddr() == mLab->getAddr() && before.contains(wLab->getPos()))
				initLab = wLab;
		}
	}

	/* Also make sure there is an allocating event and some initializer store.
	 * We do this separately for better error messages */
	if (!allocLab) {
		visitError(mLab->getPos(), Status::VS_AccessNonMalloc);
		return true;
	}
	if (llvm::isa<ReadLabel>(mLab) && !initLab) {
		visitError(mLab->getPos(), Status::VS_UninitializedMem);
		return true;
	}
	/* If this access is a potential hazard, make sure it is properly protected */
	if (potHazLab && !isHazptrProtected(allocLab, mLab)) {
		visitError(mLab->getPos(), Status::VS_AccessFreed,
			   "Access not properly protected by hazard pointer!", potHazLab->getPos());
		return true;
	}
	return false;
}

bool GenMCDriver::checkForMemoryRaces(const FreeLabel *fLab)
{
	if (!fLab->getFreedAddr().isDynamic())
		return false;

	const auto &g = getGraph();
	auto ptr = fLab->getFreedAddr();
	auto &before = g.getEventLabel(fLab->getPos())->getHbView();
	const MallocLabel *m = nullptr; /* There must be a malloc() before the free() */
	const MemAccessLabel *potHazLab = nullptr;

	for (const auto *lab : labels(g)) {
		if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
			if (aLab->getAllocAddr() == ptr &&
			    before.contains(aLab->getPos())) {
				m = aLab;
			}
		}
		if (auto *dLab = llvm::dyn_cast<FreeLabel>(lab)) {
			if (dLab->getFreedAddr() == ptr &&
			    dLab->getPos() != fLab->getPos()) {
				visitError(fLab->getPos(), Status::VS_DoubleFree, "", dLab->getPos());
				return true;
			}
		}
		if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
			if (mLab->getAddr() == ptr && !before.contains(mLab->getPos())) {
				if (!llvm::isa<HpRetireLabel>(fLab)) {
					visitError(fLab->getPos(), Status::VS_AccessFreed, "", mLab->getPos());
					return true;
				}
				potHazLab = mLab;
			}

		}
	}

	if (!m) {
		visitError(fLab->getPos(), Status::VS_FreeNonMalloc);
		return true;
	}
	if (potHazLab && !isHazptrProtected(m, potHazLab)) {
		visitError(fLab->getPos(), Status::VS_AccessFreed,
			   "Access not properly protected by hazard pointer!", potHazLab->getPos());
		return true;
	}
	return false;
}

/*
 * This function is called to check for data races when a new event is added.
 * When a race is detected visit error is called, which will report an error
 * if the execution is valid. This method is memory-model specific since
 * the concept of a "race" (e.g., as in (R)C11) may not be defined on all
 * models, and thus relies on a virtual method.
 */
void GenMCDriver::checkForDataRaces(const MemAccessLabel *lab)
{
	if (getConf()->disableRaceDetection)
		return;

	auto racy = findDataRaceForMemAccess(lab);

	/* If a race is found and the execution is consistent, return it */
	if (!racy.isInitializer())
		visitError(lab->getPos(), Status::VS_RaceNotAtomic, "", racy);
	return;
}

bool GenMCDriver::isAccessValid(const MemAccessLabel *lab)
{
	/* Make sure that the interperter is aware of this static variable */
	if (!lab->getAddr().isDynamic())
		return getEE()->isStaticallyAllocated(lab->getAddr());

	/* Validity of dynamic accesses will be checked as part of the race detection mechanism */
	return !lab->getAddr().isNull() && !checkForMemoryRaces(lab);
}

void GenMCDriver::checkLockValidity(const ReadLabel *rLab, const std::vector<Event> &rfs)
{
	auto *lLab = llvm::dyn_cast<LockCasReadLabel>(rLab);
	if (!lLab)
		return;

	/* Should not read from destroyed mutex */
	auto rfIt = std::find_if(rfs.cbegin(), rfs.cend(), [this, lLab](const Event &rf){
		auto rfVal = getWriteValue(rf, lLab->getAddr(), lLab->getAccess());
		return rfVal == SVal(-1);
	});
	if (rfIt != rfs.cend())
		visitError(rLab->getPos(), Status::VS_UninitializedMem,
			   "Called lock() on destroyed mutex!", *rfIt);
}

void GenMCDriver::checkUnlockValidity(const WriteLabel *wLab)
{
	auto *uLab = llvm::dyn_cast<UnlockWriteLabel>(wLab);
	if (!uLab)
		return;

	/* Unlocks should unlock mutexes locked by the same thread */
	if (getGraph().getMatchingLock(uLab->getPos()).isInitializer()) {
		visitError(uLab->getPos(), Status::VS_InvalidUnlock,
			   "Called unlock() on mutex not locked by the same thread!");
	}
}

void GenMCDriver::checkBInitValidity(const WriteLabel *lab)
{
	auto *wLab = llvm::dyn_cast<BInitWriteLabel>(lab);
	if (!wLab)
		return;

	/* Make sure the barrier hasn't already been initialized, and
	 * that the initializing value is greater than 0 */
	auto &g = getGraph();
	auto sIt = std::find_if(store_begin(g, wLab->getAddr()), store_end(g, wLab->getAddr()),
				[&g, wLab](const Event &s){
		auto *sLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
		return sLab != wLab && sLab->getAddr() == wLab->getAddr() &&
			llvm::isa<BInitWriteLabel>(sLab);
	});

	if (sIt != store_end(g, wLab->getAddr()))
		visitError(wLab->getPos(), Status::VS_InvalidBInit, "Called barrier_init() multiple times!", *sIt);
	else if (wLab->getVal() == SVal(0))
		visitError(wLab->getPos(), Status::VS_InvalidBInit, "Called barrier_init() with 0!");
	return;
}

void GenMCDriver::checkBIncValidity(const ReadLabel *rLab, const std::vector<Event> &rfs)
{
	auto *bLab = llvm::dyn_cast<BIncFaiReadLabel>(rLab);
	if (!bLab)
		return;

	if (std::any_of(rfs.cbegin(), rfs.cend(), [](const Event &rf){ return rf.isInitializer(); }))
		visitError(rLab->getPos(), Status::VS_UninitializedMem,
			   "Called barrier_wait() on uninitialized barrier!");
	else if (std::any_of(rfs.cbegin(), rfs.cend(), [this, bLab](const Event &rf){
		auto rfVal = getWriteValue(rf, bLab->getAddr(), bLab->getAccess());
		return rfVal == SVal(0);
	}))
		visitError(rLab->getPos(), Status::VS_AccessFreed,
			   "Called barrier_wait() on destroyed barrier!", bLab->getRf());
}

void GenMCDriver::checkFinalAnnotations(const WriteLabel *wLab)
{
	if (!getConf()->helper)
		return;

	auto &g = getGraph();
	auto *cc = g.getCoherenceCalculator();

	if (!cc->hasMoreThanOneStore(wLab->getAddr()))
		return;
	if ((wLab->isFinal() &&
	     std::any_of(cc->store_begin(wLab->getAddr()), cc->store_end(wLab->getAddr()),
			 [&](const Event &s){ return !wLab->getHbView().contains(s); })) ||
	    (!wLab->isFinal() &&
	     std::any_of(cc->store_begin(wLab->getAddr()), cc->store_end(wLab->getAddr()),
			 [&](const Event &s){ return g.getWriteLabel(s)->isFinal(); }))) {
		visitError(wLab->getPos(), Status::VS_Annotation,
			   "Multiple stores at final location!");
		return;
	}
	return;
}

bool GenMCDriver::isConsistent(ProgramPoint p)
{
	initConsCalculation();
	return getGraph().isConsistent(getCheckConsType(p));
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
	BUG_ON(!llvm::isa<BlockLabel>(g.getLastThreadLabel(tid)));
	auto *lastLab = g.getPreviousLabel(g.getLastThreadLabel(tid));
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
	if (isHalting() || !isConsistent(ProgramPoint::exec))
		return;

	const auto &g = getGraph();
	const auto *EE = getEE();

	/* Collect all threads blocked at spinloops */
	std::vector<int> spinBlocked;
	for (auto thrIt = EE->threads_begin(), thrE = EE->threads_end(); thrIt != thrE; ++thrIt) {
		auto *bLab = llvm::dyn_cast<BlockLabel>(g.getLastThreadLabel(thrIt->id));
		if (thrIt->getBlockageType() == BlockageType::Spinloop ||
		    (bLab && bLab->getType() == BlockageType::Spinloop))
			spinBlocked.push_back(thrIt->id);
	}

	if (spinBlocked.empty())
		return;

	/* And check whether all of them are live or not */
	auto nonTermTID = 0u;
	if (std::all_of(spinBlocked.begin(), spinBlocked.end(), [&](int tid){
		nonTermTID = tid;
		return threadReadsMaximal(tid);
	})) {
		/* Print some TID blocked by a spinloop */
		visitError(g.getLastThreadEvent(nonTermTID), Status::VS_Liveness,
			   "Non-terminating spinloop: thread " + std::to_string(nonTermTID));
	}
	return;
}

void GenMCDriver::filterAcquiredLocks(const ReadLabel *rLab, std::vector<Event> &stores)

{
	const auto &g = getGraph();

	/* The course of action depends on whether we are in repair mode or not */
	if ((llvm::isa<LockCasWriteLabel>(g.getEventLabel(stores.back())) ||
	     llvm::isa<TrylockCasWriteLabel>(g.getEventLabel(stores.back()))) &&
	    !isRescheduledRead(rLab->getPos())) {
		stores = {stores.back()};
		return;
	}

	auto max = stores.back();
	stores.erase(std::remove_if(stores.begin(), stores.end(), [&](const Event &s){
		return (llvm::isa<LockCasWriteLabel>(g.getEventLabel(s)) ||
			llvm::isa<TrylockCasWriteLabel>(g.getEventLabel(s))) && s != max;
	}), stores.end());
	return;
}

void GenMCDriver::filterConflictingBarriers(const ReadLabel *lab, std::vector<Event> &stores)
{
	if (getConf()->disableBAM || !llvm::isa<BIncFaiReadLabel>(lab))
		return;

	/* barrier_wait()'s FAI loads should not read from conflicting stores */
	auto &g = getGraph();
	stores.erase(std::remove_if(stores.begin(), stores.end(), [&](const Event &s){
		return g.isStoreReadByExclusiveRead(s, lab->getAddr());
	}), stores.end());
	return;
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
			if (!rLabB) return false;
		        if (rLabA->getRf().thread == tid && rLabB->getRf().thread == pos.thread
			    && rLabA->getRf().index == rLabB->getRf().index)
				continue;
			if (rLabA->getRf() != rLabB->getRf())
				return false;
		}
	}
	return true;
}

void GenMCDriver::filterSymmetricStoresSR(const ReadLabel *rLab, std::vector<Event> &stores) const
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto t = llvm::dyn_cast<ThreadStartLabel>(
		g.getEventLabel(Event(rLab->getThread(), 0)))->getSymmetricTid();

	/* If there is no symmetric thread, exit */
	if (t == -1)
		return;

	/* Check whether the po-prefixes of the two threads match */
	if (!sharePrefixSR(t, rLab->getPos()))
		return;

	/* Get the symmetric event and make sure it matches as well */
	auto *lab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(Event(t, rLab->getIndex())));
	if (!lab || lab->getAddr() != rLab->getAddr() || lab->getSize() != lab->getSize())
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

bool GenMCDriver::filterValuesFromAnnotSAVER(const ReadLabel *rLab, std::vector<Event> &validStores)
{
	if (!rLab->getAnnot())
		return false;

	using Evaluator = SExprEvaluator<ModuleID::ID>;

	auto &g = getGraph();

	/* Ensure we keep the maximal store around even if Helper messed with it */
	BUG_ON(validStores.empty());
	auto maximal = validStores.back();
	validStores.erase(std::remove_if(validStores.begin(), validStores.end(), [&](Event w){
		auto val = getWriteValue(w, rLab->getAddr(), rLab->getAccess());
		return w != maximal && !isCoMaximal(rLab->getAddr(), w, true) &&
			!Evaluator().evaluate(rLab->getAnnot(), val);
	}), validStores.end());
	BUG_ON(validStores.empty());

	/* Return whether we should block */
	auto maximalVal = getWriteValue(validStores.back(), rLab->getAddr(), rLab->getAccess());
	return !Evaluator().evaluate(rLab->getAnnot(), maximalVal);
}

void GenMCDriver::unblockWaitingHelping()
{
	/* We have to wake up all threads waiting on helping CASes,
	 * as we don't know which ones are from the same CAS */
	std::for_each(getEE()->threads_begin(), getEE()->threads_end(), [](Thread &thr){
			if (thr.isBlocked() && thr.getBlockageType() == BlockageType::HelpedCas)
				thr.unblock();
		});
}

bool writesBeforeHelpedContainedInView(const ExecutionGraph &g, const HelpedCasReadLabel *lab, const View &view)
{
	auto &hb = lab->getHbView();

	for (auto i = 0u; i < hb.size(); i++) {
		auto j = hb[i];
		while (!llvm::isa<WriteLabel>(g.getEventLabel(Event(i, j))) && j > 0)
			--j;
		if (j > 0 && !view.contains(Event(i, j)))
			return false;
	}
	return true;
}

bool GenMCDriver::checkHelpingCasCondition(const HelpingCasLabel *hLab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	auto hs = g.collectAllEvents([&](const EventLabel *lab){
		auto *rLab = llvm::dyn_cast<HelpedCasReadLabel>(lab);
		return rLab && g.isRMWLoad(rLab) && rLab->getAddr() == hLab->getAddr() &&
			rLab->getType() == hLab->getType() && rLab->getSize() == hLab->getSize() &&
			rLab->getOrdering() == hLab->getOrdering() &&
			rLab->getExpected() == hLab->getExpected() &&
			rLab->getSwapVal() == hLab->getSwapVal();
	});

	if (hs.empty())
		return false;

	if (std::any_of(hs.begin(), hs.end(), [&g, EE](const Event &h){
		auto *hLab = llvm::dyn_cast<HelpedCasReadLabel>(g.getEventLabel(h));
		auto &view = g.getPreviousNonEmptyLabel(hLab->getPos())->getHbView();
		return !writesBeforeHelpedContainedInView(g, hLab, view);
	}))
		ERROR("Helped/Helping CAS annotation error! "
		      "Not all stores before helped-CAS are visible to helping-CAS!\n");
	return true;
}

bool GenMCDriver::checkAtomicity(const WriteLabel *wLab)
{
	if (getGraph().violatesAtomicity(wLab)) {
		moot();
		return false;
	}
	return true;
}

bool GenMCDriver::ensureConsistentRf(const ReadLabel *rLab, std::vector<Event> &rfs)
{
	bool found = false;
	while (!found) {
		found = true;
		changeRf(rLab->getPos(), rfs.back());
		if (!isConsistent(ProgramPoint::step)) {
			found = false;
			rfs.erase(rfs.end() - 1);
			BUG_ON(!getConf()->LAPOR && rfs.empty());
			if (rfs.empty())
				break;
		}
	}

	if (!found) {
		getEE()->block(BlockageType::Cons);
		return false;
	}
	return true;
}

bool GenMCDriver::ensureConsistentStore(const WriteLabel *wLab)
{
	if (!checkAtomicity(wLab) || !isConsistent(ProgramPoint::step)) {
		getEE()->block(BlockageType::Cons);
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

SVal GenMCDriver::visitThreadSelf(const EventDeps *deps)
{
	return SVal(getEE()->getCurThr().id);
}

void GenMCDriver::visitThreadKill(std::unique_ptr<ThreadKillLabel> lab)
{
	BUG_ON(isExecutionDrivenByGraph());
	updateLabelViews(lab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(lab));
	return;
}

bool GenMCDriver::isSymmetricToSR(int candidate, int thread, Event parent,
				  llvm::Function *threadFun, SVal threadArg) const
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &cThr = EE->getThrById(candidate);
	auto cParent = llvm::dyn_cast<ThreadStartLabel>(g.getEventLabel(Event(candidate, 0)))->getParentCreate();

	/* First, check that the two threads are actually similar */
	if (cThr.id == thread || cThr.threadFun != threadFun ||
	    cThr.threadArg != threadArg ||
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
				   SVal threadArg) const
{
	auto &g = getGraph();
	auto *EE = getEE();

	for (auto i = g.getNumThreads() - 1; i > 0; i--)
		if (i != thread && isSymmetricToSR(i, thread, parent, threadFun, threadArg))
			return i;
	return -1;
}

int GenMCDriver::visitThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab, const EventDeps *deps,
				   llvm::Function *calledFun, SVal arg, const llvm::ExecutionContext &SF)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph())
		return llvm::dyn_cast<ThreadCreateLabel>(g.getEventLabel(tcLab->getPos()))->getChildId();

	/* First, check if the thread to be created already exists */
	int cid = 0;
	while (cid < (long) g.getNumThreads()) {
		if (!g.isThreadEmpty(cid)) {
			auto *bLab = llvm::dyn_cast<ThreadStartLabel>(g.getFirstThreadLabel(cid));
			BUG_ON(!bLab);
			if (bLab->getParentCreate() == tcLab->getPos())
				break;
		}
		++cid;
	}

	/* Add an event for the thread creation */
	tcLab->setChildId(cid);
	updateLabelViews(tcLab.get(), deps);
	auto *lab = g.addOtherLabelToGraph(std::move(tcLab));

	/* Prepare the execution context for the new thread */
	EE->createAddNewThread(calledFun, arg, cid, lab->getThread(), SF);

	/* If the thread does not exist in the graph, make an entry for it */
	if (cid == (long) g.getNumThreads()) {
		g.addNewThread();
		BUG_ON(EE->getNumThreads() != g.getNumThreads());
		auto symm = getConf()->symmetryReduction ?
			getSymmetricTidSR(cid, lab->getPos(), calledFun, arg) : -1;
		auto tsLab = ThreadStartLabel::create(Event(cid, 0), lab->getPos(), symm);
		updateLabelViews(tsLab.get(), nullptr);
		auto *ss = g.addOtherLabelToGraph(std::move(tsLab));
	} else {
		/* Otherwise, update the existing entry */
		updateStart(lab->getPos(), g.getFirstThreadEvent(cid));
	}

	return cid;
}

SVal GenMCDriver::visitThreadJoin(std::unique_ptr<ThreadJoinLabel> lab, const EventDeps *deps)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	/* If necessary, add a relevant event to the graph */
	const ThreadJoinLabel *jLab = nullptr;
	if (!isExecutionDrivenByGraph()) {
		updateLabelViews(lab.get(), deps);
		jLab = llvm::dyn_cast<ThreadJoinLabel>(g.addOtherLabelToGraph(std::move(lab)));
	} else {
		jLab = llvm::dyn_cast<ThreadJoinLabel>(g.getEventLabel(lab->getPos()));
	}

	auto cid = jLab->getChildId();
	if (cid < 0 || long (g.getNumThreads()) <= cid || cid == thr.id) {
		std::string err = "ERROR: Invalid TID in pthread_join(): " + std::to_string(cid);
		if (cid == thr.id)
			err += " (TID cannot be the same as the calling thread)";
		visitError(jLab->getPos(), Status::VS_InvalidJoin, err);
		return SVal(0);
	}

	/* If the update failed (child has not terminated yet) block this thread */
	if (!updateJoin(jLab->getPos(), g.getLastThreadEvent(cid)))
		thr.block(BlockageType::ThreadJoin);

	/*
	 * We always return a success value, so as not to have to update it
	 * when the thread unblocks.
	 */
	return SVal(0);
}

void GenMCDriver::visitThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab)
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	if (!isExecutionDrivenByGraph() && /* Make sure that there is not a failed assume... */
	    !thr.isBlocked()) {
		updateLabelViews(eLab.get(), nullptr);
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

void GenMCDriver::visitFenceLKMM(std::unique_ptr<FenceLabel> fLab, const EventDeps *deps)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(fLab.get(), deps);
	getGraph().addOtherLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::visitFence(std::unique_ptr<FenceLabel> fLab, const EventDeps *deps)
{
	if (llvm::isa<SmpFenceLabelLKMM>(&*fLab)) {
		visitFenceLKMM(std::move(fLab), deps);
		return;
	}

	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(fLab.get(), deps);
	getGraph().addOtherLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::checkReconsiderFaiSpinloop(const MemAccessLabel *lab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);

		/* Is there any thread blocked on a potential spinloop? */
		auto *eLab = llvm::dyn_cast<BlockLabel>(g.getLastThreadLabel(i));
		if (!eLab || eLab->getType() != BlockageType::FaiZNESpinloop)
			continue;

		/* Check whether this access affects the spinloop variable */
		BUG_ON(!llvm::isa<FaiZNESpinEndLabel>(g.getPreviousLabel(eLab)));
		auto *faiLab = llvm::dyn_cast<FaiWriteLabel>(g.getPreviousLabelST(eLab,
			        [](const EventLabel *lab){ return llvm::isa<FaiWriteLabel>(lab); }));
		if (faiLab->getAddr() != lab->getAddr())
			continue;
		/* FAIs on the same variable are OK... */
		if (llvm::isa<FaiReadLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
			continue;

		/* If it does, and also breaks the assumptions, unblock thread */
		if (!isHbBefore(lab->getPos(), faiLab->getPos())) {
			if (getEE()->getThrById(eLab->getThread()).globalInstructions != 0)
				--getEE()->getThrById(eLab->getThread()).globalInstructions;
			g.remove(eLab->getPos());
			thr.unblock();
		}
	}
	return;
}

std::vector<Event> GenMCDriver::getRfsApproximation(const ReadLabel *lab)
{
	auto rfs = getGraph().getCoherentStores(lab->getAddr(), lab->getPos());
	if (!llvm::isa<CasReadLabel>(lab) && !llvm::isa<FaiReadLabel>(lab))
		return rfs;

	/* Remove atomicity violations */
	auto &g = getGraph();
	auto &before = g.getPrefixView(lab->getPos());
	rfs.erase(std::remove_if(rfs.begin(), rfs.end(), [&](const Event &s){
		auto oldVal = getWriteValue(s, lab->getAddr(), lab->getAccess());
		if (llvm::isa<FaiReadLabel>(lab) && g.isStoreReadBySettledRMW(s, lab->getAddr(), before))
			return true;
		if (auto *rLab = llvm::dyn_cast<CasReadLabel>(lab)) {
			if (oldVal == rLab->getExpected() &&
			    g.isStoreReadBySettledRMW(s, rLab->getAddr(), before))
				return true;
		}
		return false;
	}), rfs.end());
	return rfs;
}

void GenMCDriver::filterConfirmingRfs(const ReadLabel *lab, std::vector<Event> &stores)
{
	auto &g = getGraph();
	if (!getConf()->helper || !g.isConfirming(lab))
		return;

	auto sc = Event::getInitializer();
	auto *rLab = llvm::dyn_cast<ReadLabel>(
		g.getEventLabel(g.getMatchingSpeculativeRead(lab->getPos(), &sc)));
	ERROR_ON(!rLab, "Confirming annotation error! Does the speculative "
		 "read always precede the confirming operation?\n");

	/* Print a warning if there are ABAs */
	auto specVal = getWriteValue(rLab->getRf(), rLab->getAddr(), rLab->getAccess());
	auto valid = std::count_if(stores.begin(), stores.end(), [&](const Event &s){
		return getWriteValue(s, rLab->getAddr(), rLab->getAccess()) == specVal;
	});
	WARN_ON_ONCE(valid > 1, "helper-aba-found",
		     "Possible ABA pattern on variable " + getVarName(rLab->getAddr()) +
		     "! Consider running without -helper.\n");

	/* Do not optimize if there are intervening SC accesses */
	if (!sc.isInitializer())
		return;

	BUG_ON(stores.empty());

	/* Demand that the confirming read reads the speculated value (exact rf) */
	auto maximal = stores.back();
	stores.erase(std::remove_if(stores.begin(), stores.end(), [&](const Event &s){
		return s != rLab->getRf();
	}), stores.end());

	/* ... and if no such value exists, block indefinitely */
	if (stores.empty()) {
		stores.push_back(maximal);
		visitBlock(BlockLabel::create(lab->getPos().next(), BlockageType::Confirmation));
		return;
	}

	/* deprioritize thread upon confirmation */
	if (!threadPrios.empty() &&
	    llvm::isa<SpeculativeReadLabel>(getGraph().getEventLabel(threadPrios[0])))
		threadPrios.clear();
	return;
}

bool GenMCDriver::existsPendingSpeculation(const ReadLabel *lab, const std::vector<Event> &stores)
{
	auto &g = getGraph();
	return (std::any_of(label_begin(g), label_end(g), [&](const EventLabel *oLab){
		auto *orLab = llvm::dyn_cast<SpeculativeReadLabel>(oLab);
		return orLab && orLab->getAddr() == lab->getAddr() &&
		       !g.getPreviousLabel(lab->getPos())->getHbView().contains(orLab->getPos()) &&
			orLab->getPos() != lab->getPos();
	}) &&
		std::find_if(stores.begin(), stores.end(), [&](const Event &s){
			return llvm::isa<ConfirmingCasWriteLabel>(g.getEventLabel(s)) &&
				s.index > lab->getIndex() && s.thread == lab->getThread();
		}) == stores.end());
}

void GenMCDriver::filterUnconfirmedReads(const ReadLabel *lab, std::vector<Event> &stores)
{
	if (!getConf()->helper || !llvm::isa<SpeculativeReadLabel>(lab))
		return;

	if (isRescheduledRead(lab->getPos())) {
		setRescheduledRead(Event::getInitializer());
		return;
	}

	/* If there exist any speculative reads the confirming read of which has not been added,
	 * prioritize those and discard current rfs; otherwise, prioritize ourselves */
	if (existsPendingSpeculation(lab, stores)) {
		std::swap(stores[0], stores.back());
		stores.resize(1);
		visitBlock(BlockLabel::create(lab->getPos().next(), BlockageType::ReadOptBlock));
		return;
	}

	threadPrios = { lab->getPos() };
}

void GenMCDriver::filterOptimizeRfs(const ReadLabel *lab, std::vector<Event> &stores)
{
	/* Symmetry reduction */
	if (getConf()->symmetryReduction)
		filterSymmetricStoresSR(lab, stores);

	/* BAM */
	if (!getConf()->disableBAM)
		filterConflictingBarriers(lab, stores);

	/* Locking */
	if (llvm::isa<LockCasReadLabel>(lab))
		filterAcquiredLocks(lab, stores);

	/* Helper: Try to read speculated value (affects maximality status) */
	if (getConf()->helper && getGraph().isConfirming(lab))
		filterConfirmingRfs(lab, stores);

	/* Helper: If there are pending confirmations, prioritize those */
	if (getConf()->helper && llvm::isa<SpeculativeReadLabel>(lab))
		filterUnconfirmedReads(lab, stores);

	/* If this load is annotatable, keep values that will not leed to blocking */
	if (lab->getAnnot())
		filterValuesFromAnnotSAVER(lab, stores);
}

SVal GenMCDriver::visitLoad(std::unique_ptr<ReadLabel> rLab, const EventDeps *deps)
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	if (inRecoveryMode())
		return getRecReadRetValue(rLab.get());

	if (isExecutionDrivenByGraph())
		return getReadRetValueAndMaybeBlock(llvm::dyn_cast<ReadLabel>(g.getEventLabel(rLab->getPos())));

	/* First, we have to check whether the access is valid. This has to
	 * happen here because we may query the interpreter for this location's
	 * value in order to determine whether this load is going to be an RMW.
	 * Coherence needs to be tracked before validity is established, as
	 * consistency checks may be triggered if the access is invalid */
	g.trackCoherenceAtLoc(rLab->getAddr());

	rLab->setAnnot(EE->getCurrentAnnotConcretized());
	updateLabelViews(rLab.get(), deps);
	auto *lab = g.addReadLabelToGraph(std::move(rLab));

	if (!isAccessValid(lab)) {
		visitError(lab->getPos(), Status::VS_AccessNonMalloc);
		return SVal(0); /* Return some value; this execution will be blocked */
	}

	/* Get an approximation of the stores we can read from */
	auto stores = getRfsApproximation(lab);
	BUG_ON(stores.empty());

	/* Try to minimize the number of rfs */
	filterOptimizeRfs(lab, stores);

	/* ... add an appropriate label with a random rf */
	changeRf(lab->getPos(), stores.back());

	/* ... and make sure that the rf we end up with is consistent */
	if (!ensureConsistentRf(lab, stores))
		return SVal(0);

	GENMC_DEBUG(
		if (getConf()->vLevel >= VerbosityLevel::V3) {
			llvm::dbgs() << "--- Added load " << lab->getPos() << "\n";
			printGraph();
		}
	);

	/* Check whether the load forces us to reconsider some existing event */
	checkReconsiderFaiSpinloop(lab);

	/* Check for races and reading from uninitialized memory */
	checkForDataRaces(lab);
	if (llvm::isa<LockCasReadLabel>(lab))
		checkLockValidity(lab, stores);
	if (llvm::isa<BIncFaiReadLabel>(lab))
		checkBIncValidity(lab, stores);

	if (isRescheduledRead(lab->getPos()) && !llvm::isa<LockCasReadLabel>(lab))
		setRescheduledRead(Event::getInitializer());

	/* If this is the last part of barrier_wait() check whether we should block */
	auto retVal = getWriteValue(stores.back(), lab->getAddr(), lab->getAccess());
	if (llvm::isa<BWaitReadLabel>(lab) &&
	   retVal != getBarrierInitValue(lab->getAddr(), lab->getAccess()))
		visitBlock(BlockLabel::create(lab->getPos().next(), BlockageType::Barrier));

	/* Push all the other alternatives choices to the Stack (many maximals for wb) */
	std::for_each(stores.begin(), stores.end() - 1, [&](const Event &s){
		auto status = llvm::isa<MOCalculator>(g.getCoherenceCalculator()) ? false :
			isCoMaximal(lab->getAddr(), s, true); /* MO messes with the status */
		addToWorklist(std::make_unique<ForwardRevisit>(lab->getPos(), s, status));
	});
	return retVal;
}

void GenMCDriver::annotateStoreHELPER(WriteLabel *wLab) const
{
	auto &g = getGraph();

	/* Don't bother with lock ops */
	if (!getConf()->helper || !g.isRMWStore(wLab) || llvm::isa<LockCasWriteLabel>(wLab) ||
	    llvm::isa<TrylockCasWriteLabel>(wLab))
		return;

	/* Check whether we can mark it as RevBlocker */
	auto *pLab = g.getPreviousLabel(wLab);
	auto *mLab = llvm::dyn_cast_or_null<MemAccessLabel>(
		getPreviousVisibleAccessLabel(pLab->getPos()));
	auto *rLab = llvm::dyn_cast_or_null<ReadLabel>(mLab);
	if (!mLab || (mLab->wasAddedMax() && (!rLab || rLab->isRevisitable())))
		return;

	/* Mark the store and its predecessor */
	if (llvm::isa<FaiWriteLabel>(wLab))
		llvm::dyn_cast<FaiReadLabel>(pLab)->setAttr(WriteAttr::RevBlocker);
	else
		llvm::dyn_cast<CasReadLabel>(pLab)->setAttr(WriteAttr::RevBlocker);
	wLab->setAttr(WriteAttr::RevBlocker);
	return;
}

std::vector<Event> GenMCDriver::getRevisitableApproximation(const WriteLabel *sLab)
{
	auto &g = getGraph();
	auto loads = g.getCoherentRevisits(sLab);
	std::sort(loads.begin(), loads.end(), [&g](const Event &l1, const Event &l2){
		return g.getEventLabel(l1)->getStamp() > g.getEventLabel(l2)->getStamp();
	});
	return loads;
}

void GenMCDriver::visitStore(std::unique_ptr<WriteLabel> wLab, const EventDeps *deps)
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto *EE = getEE();

	/* If it's a valid access, track coherence for this location */
	g.trackCoherenceAtLoc(wLab->getAddr());

	if (getConf()->helper && g.isRMWStore(&*wLab))
		annotateStoreHELPER(&*wLab);
	updateLabelViews(wLab.get(), deps);
	auto *lab = g.addWriteLabelToGraph(std::move(wLab));

	if (!isAccessValid(lab)) {
		visitError(lab->getPos(), Status::VS_AccessNonMalloc);
		return;
	}

	/* Find all possible placings in coherence for this store */
	auto placesRange = g.getCoherentPlacings(lab->getAddr(), lab->getPos(), g.isRMWStore(lab));
	auto &begO = placesRange.first;
	auto &endO = placesRange.second;

	/* It is always consistent to add the store at the end of MO */
	if (llvm::isa<BIncFaiWriteLabel>(lab) && lab->getVal() == SVal(0))
		const_cast<WriteLabel*>(lab)->setVal(getBarrierInitValue(lab->getAddr(), lab->getAccess()));
	g.getCoherenceCalculator()->addStoreToLoc(lab->getAddr(), lab->getPos(), endO);

	for (auto it = store_begin(g, lab->getAddr()) + begO,
		  ie = store_begin(g, lab->getAddr()) + endO; it != ie; ++it) {

		/* We cannot place the write just before the write of an RMW */
		if (g.isRMWStore(*it))
			continue;

		/* Push the stack item */
		if (!inRecoveryMode())
			addToWorklist(std::make_unique<WriteRevisit>(
					      lab->getPos(), std::distance(store_begin(g, lab->getAddr()), it)));
	}

	/* If the graph is not consistent (e.g., w/ LAPOR) stop the exploration */
	bool cons = ensureConsistentStore(lab);

	GENMC_DEBUG(
		if (getConf()->vLevel >= VerbosityLevel::V3) {
			llvm::dbgs() << "--- Added store " << lab->getPos() << "\n";
			printGraph();
		}
	);

	if (!inRecoveryMode() && !inReplay())
		calcRevisits(lab);

	if (!cons)
		return;

	checkReconsiderFaiSpinloop(lab);
	if (llvm::isa<HelpedCasWriteLabel>(lab))
		unblockWaitingHelping();

	/* Check for races */
	checkForDataRaces(lab);
	if (llvm::isa<UnlockWriteLabel>(lab))
		checkUnlockValidity(lab);
	if (llvm::isa<BInitWriteLabel>(lab))
		checkBInitValidity(lab);
	checkFinalAnnotations(lab);
	return;
}

void GenMCDriver::visitLockLAPOR(std::unique_ptr<LockLabelLAPOR> lab, const EventDeps *deps)
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	updateLabelViews(lab.get(), deps);
	g.addLockLabelToGraphLAPOR(std::move(lab));

	/* Only prioritize when first adding a lock; in replays, this
	 * is handled in the setup */
	threadPrios.insert(threadPrios.begin(), lab->getPos());
	return;
}

void GenMCDriver::visitLock(Event pos, SAddr addr, ASize size, const EventDeps *deps)
{
	/* No locking when running the recovery routine */
	if (getConf()->persevere && inRecoveryMode())
		return;

	/* Treatment of locks based on whether LAPOR is enabled */
	if (getConf()->LAPOR) {
		visitLockLAPOR(LockLabelLAPOR::create(pos, addr), deps);
		return;
	}

	auto ret = visitLoad(LockCasReadLabel::create(pos, addr, size), deps);

	/* Check if that was a rescheduled lock; do that here so that
	 * recursive calls always have no locks to be rescheduled */
	auto rescheduled = isRescheduledRead(pos);
	if (isRescheduledRead(pos))
		setRescheduledRead(Event::getInitializer());

	auto *rLab = llvm::dyn_cast<ReadLabel>(getGraph().getEventLabel(pos));
	if (!rLab->getRf().isBottom() && ret == SVal(0))
		visitStore(LockCasWriteLabel::create(pos.next(), addr, size), deps);
	else
		visitBlock(BlockLabel::create(pos.next(),
					      rescheduled ? BlockageType::LockNotAcq :
					      BlockageType::ReadOptBlock));
}

void GenMCDriver::visitUnlockLAPOR(std::unique_ptr<UnlockLabelLAPOR> uLab, const EventDeps *deps)
{
	if (isExecutionDrivenByGraph()) {
		deprioritizeThread(uLab.get());
		return;
	}

	updateLabelViews(uLab.get(), deps);
	getGraph().addOtherLabelToGraph(std::move(uLab));

	/* Ensure we deprioritize also when adding an event, as a
	 * revisit may leave a critical section open */
	deprioritizeThread(uLab.get());
	return;
}

void GenMCDriver::visitUnlock(Event pos, SAddr addr, ASize size, const EventDeps *deps)
{
	/* No locking when running the recovery routine */
	if (getConf()->persevere && inRecoveryMode())
		return;

	/* Treatment of unlocks based on whether LAPOR is enabled */
	if (getConf()->LAPOR) {
		visitUnlockLAPOR(UnlockLabelLAPOR::create(pos, addr), deps);
		return;
	}

	visitStore(UnlockWriteLabel::create(pos, addr, size), deps);
	return;
}

void GenMCDriver::visitHpProtect(std::unique_ptr<HpProtectLabel> hpLab, const EventDeps *deps)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(hpLab.get(), deps);
	getGraph().addOtherLabelToGraph(std::move(hpLab));
	return;
}

SVal GenMCDriver::visitMalloc(std::unique_ptr<MallocLabel> aLab, const EventDeps *deps,
			      unsigned int alignment, Storage s, AddressSpace spc)
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	if (isExecutionDrivenByGraph()) {
		auto *lab = llvm::dyn_cast<MallocLabel>(g.getEventLabel(aLab->getPos()));
		BUG_ON(!lab);
		return SVal(lab->getAllocAddr().get());
	}

	/* Fix and add label to the graph; return the new address */
	aLab->setAllocAddr(EE->getFreshAddr(aLab->getAllocSize(), alignment, s, spc));
	updateLabelViews(aLab.get(), deps);
	auto *lab = llvm::dyn_cast<MallocLabel>(g.addOtherLabelToGraph(std::move(aLab)));
	return SVal(lab->getAllocAddr().get());
}

void GenMCDriver::visitFree(std::unique_ptr<FreeLabel> dLab, const EventDeps *deps)
{
	auto &g = getGraph();
	auto *EE = getEE();
	auto &thr = EE->getCurThr();

	/* Attempt to free a NULL pointer; don't increase counters */
	if (dLab->getFreedAddr().isNull())
		return;

	if (isExecutionDrivenByGraph())
		return;

	/* Find the size of the area deallocated */
	auto size = 0u;
	auto alloc = g.getMallocCounterpart(&*dLab);
	if (!alloc.isInitializer())
		size = llvm::dyn_cast<MallocLabel>(g.getEventLabel(alloc))->getAllocSize();

	/* Add a label with the appropriate store */
	dLab->setFreedSize(size);
	updateLabelViews(dLab.get(), deps);
	auto *lab = g.addOtherLabelToGraph(std::move(dLab));

	/* Check whether there is any memory race */
	checkForMemoryRaces(llvm::dyn_cast<FreeLabel>(lab));
	return;
}

void GenMCDriver::visitRCULockLKMM(std::unique_ptr<RCULockLabelLKMM> lab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(lab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(lab));
	return;
}

void GenMCDriver::visitRCUUnlockLKMM(std::unique_ptr<RCUUnlockLabelLKMM> lab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(lab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(lab));
	return;
}

void GenMCDriver::visitRCUSyncLKMM(std::unique_ptr<RCUSyncLabelLKMM> lab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(lab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(lab));
	return;
}

const MemAccessLabel *GenMCDriver::getPreviousVisibleAccessLabel(Event start) const
{
	auto &g = getGraph();
	std::vector<Event> finalReads;

	for (auto pos = start.prev(); pos.index > 0; --pos) {
		auto *lab = g.getEventLabel(pos);
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (getConf()->helper && g.isConfirming(rLab))
				continue;
			if (!rLab->getRf().isBottom()) {
				auto *rfLab = g.getEventLabel(rLab->getRf());
				auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab);
				if (wLab && wLab->isLocal())
					continue;
				if (wLab && wLab->isFinal()) {
					finalReads.push_back(rLab->getPos());
					continue;
				}
				if (std::any_of(finalReads.begin(), finalReads.end(), [&](const Event &l){
					auto *lLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(l));
					return lLab->getAddr() == rLab->getAddr() &&
					       lLab->getSize() == rLab->getSize();
				}))
					continue;
			}
			return rLab;
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab))
			if (!wLab->isFinal() && !wLab->isLocal())
				return wLab;
	}
	return nullptr; /* none found */
}

void GenMCDriver::mootExecutionIfFullyBlocked(Event pos)
{
	auto &g = getGraph();

	auto *lab = getPreviousVisibleAccessLabel(pos);
	if (auto *rLab = llvm::dyn_cast_or_null<ReadLabel>(lab))
		if (!rLab->isRevisitable() || !rLab->wasAddedMax())
			moot();
	return;
}

void GenMCDriver::visitBlock(std::unique_ptr<BlockLabel> lab)
{
	getEE()->getCurThr().block(BlockageType::User);

	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();
	auto *bLab = llvm::dyn_cast<BlockLabel>(g.addOtherLabelToGraph(std::move(lab)));
	mootExecutionIfFullyBlocked(bLab->getPos());
}

View GenMCDriver::getReplayView() const
{
	auto &g = getGraph();
	auto v = g.getViewFromStamp(g.nextStamp());

	/* visitBlock() is usually only called during normal execution
	 * and hence not reproduced during replays.
	 * We have to remove BlockLabels so that these will not lead
	 * to the execution of extraneous instructions */
	for (auto i = 0u; i < g.getNumThreads(); i++)
		if (llvm::isa<BlockLabel>(g.getLastThreadLabel(i)))
			--v[i];
	return v;
}

void GenMCDriver::visitError(Event pos, Status s, const std::string &err /* = "" */,
			     Event confEvent /* = INIT */)
{
	auto &g = getGraph();
	auto &thr = getEE()->getCurThr();

	/* If we have already detected an error, no need to report another */
	if (isHalting())
		return;

	/* If we this is a replay (might happen if one LLVM instruction
	 * maps to many MC events), do not get into an infinite loop... */
	if (inReplay())
		return;

	/* If the execution that led to the error is not consistent, block */
	if (!isConsistent(ProgramPoint::error)) {
		thr.block(BlockageType::Error);
		return;
	}
	if (inRecoveryMode() && !isRecoveryValid(ProgramPoint::error)) {
		thr.block(BlockageType::Error);
		return;
	}

	const EventLabel *errLab = g.getEventLabel(pos);

	/* If this is an invalid access, change the RF of the offending
	 * event to BOTTOM, so that we do not try to get its value.
	 * Don't bother updating the views */
	if (isInvalidAccessError(s) && llvm::isa<ReadLabel>(errLab))
		g.changeRf(errLab->getPos(), Event::getBottom());

	/* Print a basic error message and the graph.
	 * We have to save the interpreter state as replaying will
	 * destroy the current execution stack */
	auto oldState = getEE()->releaseLocalState();

	getEE()->replayExecutionBefore(getReplayView());

	llvm::raw_string_ostream out(result.message);

	out << "Error detected: " << s << "!\n";
	out << "Event " << errLab->getPos() << " ";
	if (!confEvent.isInitializer())
		out << "conflicts with event " << confEvent << " ";
	out << "in graph:\n";
	printGraph(true, out);

	/* Print error trace leading up to the violating event(s) */
	if (getConf()->printErrorTrace) {
		printTraceBefore(errLab->getPos(), out);
		if (!confEvent.isInitializer())
			printTraceBefore(confEvent, out);
	}

	/* Print the specific error message */
	if (!err.empty())
		out << err << "\n";

	/* Dump the graph into a file (DOT format) */
	if (getConf()->dotFile != "")
		dotPrintToFile(getConf()->dotFile, errLab->getPos(), confEvent);

	getEE()->restoreLocalState(std::move(oldState));

	halt(s);
}

#ifdef ENABLE_GENMC_DEBUG
void GenMCDriver::checkForDuplicateRevisit(const ReadLabel *rLab, const WriteLabel *sLab)
{
	if (!getConf()->countDuplicateExecs)
		return;

	/* Get the prefix of the write to save */
	auto &g = getGraph();
	auto writePrefix = g.getPrefixLabelsNotBefore(sLab, rLab);
	auto moPlacings = g.saveCoherenceStatus(writePrefix, rLab);

	auto writePrefixPos = g.extractRfs(writePrefix);
	writePrefixPos.insert(writePrefixPos.begin(), sLab->getPos());

	/* If this prefix has revisited the read before, skip */
	if (revisitSetContains(rLab, writePrefixPos, moPlacings)) {
		++result.duplicates;
	} else {
		addToRevisitSet(rLab, writePrefixPos, moPlacings);
	}
	return;
}
#endif

bool GenMCDriver::tryOptimizeBarrierRevisits(const BIncFaiWriteLabel *sLab, std::vector<Event> &loads)
{
	if (getConf()->disableBAM)
		return false;

	/* If the barrier_wait() does not write the initial value, nothing to do */
	auto iVal = getBarrierInitValue(sLab->getAddr(), sLab->getAccess());
	if (sLab->getVal() != iVal)
		return true;

	/* Otherwise, revisit in place */
	auto &g = getGraph();
	if (loads.size() > iVal.get() ||
	    std::any_of(loads.begin(), loads.end(), [&g](const Event &l){
		    return !llvm::isa<BWaitReadLabel>(g.getEventLabel(l)) ||
			    l != g.getLastThreadEvent(l.thread).prev(); }))
		WARN_ONCE("bam-well-formed", "Execution not barrier-well-formed!\n");

	std::for_each(loads.begin(), loads.end(), [&](const Event &l){
		auto *rLab = llvm::dyn_cast<BWaitReadLabel>(g.getEventLabel(l));
		changeRf(rLab->getPos(), sLab->getPos());
		rLab->setAddedMax(isCoMaximal(rLab->getAddr(), rLab->getRf()));
		g.remove(g.getLastThreadLabel(l.thread));
		if (getEE()->getThrById(rLab->getThread()).globalInstructions != 0)
			--getEE()->getThrById(rLab->getThread()).globalInstructions;
		getEE()->getThrById(rLab->getThread()).unblock();
	});
	return true;
}

void GenMCDriver::optimizeUnconfirmedRevisits(const WriteLabel *sLab, std::vector<Event> &loads)
{
	if (!getConf()->helper)
		return;

	auto &g = getGraph();

	/* If there is already a write with the same value, report a possible ABA */
	auto valid = std::count_if(store_begin(g, sLab->getAddr()), store_end(g, sLab->getAddr()),
				   [&](const Event &w){
		auto *wLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(w));
		return wLab->getPos() != sLab->getPos() && wLab->getVal() == sLab->getVal();
	});
	if (sLab->getAddr().isStatic() &&
	    getWriteValue(Event::getInitializer(), sLab->getAddr(), sLab->getAccess()) == sLab->getVal())
		++valid;
	WARN_ON_ONCE(valid > 0, "helper-aba-found",
		     "Possible ABA pattern! Consider running without -helper.\n");

	/* Do not bother with revisits that will be unconfirmed/lead to ABAs */
	loads.erase(std::remove_if(loads.begin(), loads.end(), [&](const Event &l){
		auto *lab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(l));
		if (!g.isConfirming(lab))
			return false;

		auto sc = Event::getInitializer();
		auto *pLab = llvm::dyn_cast<ReadLabel>(
			g.getEventLabel(g.getMatchingSpeculativeRead(lab->getPos(), &sc)));
		ERROR_ON(!pLab, "Confirming CAS annotation error! "
			 "Does a speculative read precede the confirming operation?\n");

		return sc.isInitializer();
	}), loads.end());
}

bool isConflictingNonRevBlocker(const ExecutionGraph &g, const EventLabel *pLab,
				const WriteLabel *sLab, const Event &s)
{
	auto *sLab2 = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
	if (sLab2->getPos() == sLab->getPos() || !g.isRMWStore(sLab2))
		return false;
	if (g.getPrefixView(sLab->getPos()).contains(sLab2->getPos()) &&
	    !(pLab && pLab->getStamp() < sLab2->getStamp()))
		return false;
	if (sLab2->getThread() <= sLab->getThread())
		return false;
	return std::any_of(sLab2->readers_begin(), sLab2->readers_end(), [&](const Event &r){
				return g.getEventLabel(r)->getStamp() < sLab2->getStamp() &&
					!g.getPrefixView(sLab->getPos()).contains(r);
		});
}

bool GenMCDriver::tryOptimizeRevBlockerAddition(const WriteLabel *sLab, std::vector<Event> &loads)
{
	if (!sLab->hasAttr(WriteAttr::RevBlocker))
		return false;

	auto &g = getGraph();
	auto *pLab = getPreviousVisibleAccessLabel(sLab->getPos().prev());
	if (std::find_if(store_begin(g, sLab->getAddr()), store_end(g, sLab->getAddr()),
			 [&g, pLab, sLab](const Event &s){
		return isConflictingNonRevBlocker(g, pLab, sLab, s);
	}) != store_end(g, sLab->getAddr())) {
		moot();
		loads.clear();
		return true;
	}
	return false;
}

bool GenMCDriver::tryOptimizeRevisits(const WriteLabel *sLab, std::vector<Event> &loads)
{
	auto &g =getGraph();

	/* Symmetry reduction */
	if (getConf()->symmetryReduction) {
		auto tid = g.getFirstThreadLabel(sLab->getThread())->getSymmetricTid();
		if (tid != -1 && sharePrefixSR(tid, sLab->getPos()))
			return true;
	}

	/* BAM */
	if (!getConf()->disableBAM) {
		if (auto *faiLab = llvm::dyn_cast<BIncFaiWriteLabel>(sLab)) {
			if (tryOptimizeBarrierRevisits(faiLab, loads))
				return true;
		}
	}

	/* Helper: 1) Do not bother with revisits that will lead to unconfirmed reads
	           2) Do not bother exploring if a RevBlocker is being re-added	*/
	if (getConf()->helper) {
		optimizeUnconfirmedRevisits(sLab, loads);
		if (sLab->hasAttr(WriteAttr::RevBlocker) && tryOptimizeRevBlockerAddition(sLab, loads))
			return true;
	}

	/* Optimization-blocked (locking + helper) */
	loads.erase(std::remove_if(loads.begin(), loads.end(), [&g](const Event &l){
		return g.isOptBlockedRead(g.getEventLabel(l));
	}), loads.end());

	return false;
}

bool GenMCDriver::tryRevisitLockInPlace(const BackwardRevisit &r)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (g.getLastThreadEvent(r.getPos().thread).prev() != r.getPos() ||
	    g.revisitModifiesGraph(r))
		return false;

	auto *rLab = g.getReadLabel(r.getPos());
	const auto *sLab = g.getWriteLabel(r.getRev());

	BUG_ON(!llvm::isa<LockCasReadLabel>(rLab) || !llvm::isa<UnlockWriteLabel>(sLab));
	BUG_ON(!llvm::isa<BlockLabel>(g.getEventLabel(rLab->getPos().next())));
	g.remove(rLab->getPos().next());
	changeRf(rLab->getPos(), sLab->getPos());
	rLab->setAddedMax(isCoMaximal(rLab->getAddr(), rLab->getRf()));

	completeRevisitedRMW(rLab);

	GENMC_DEBUG(
		if (getConf()->vLevel >= VerbosityLevel::V2) {
			llvm::dbgs() << "--- In-place revisiting " << rLab->getPos()
			<< " <-- " << sLab->getPos() << "\n";
			printGraph();
		}
	);

	EE->getThrById(rLab->getThread()).unblock();
	threadPrios = {rLab->getPos()};
	return true;
}

std::unique_ptr<BackwardRevisit>
GenMCDriver::constructBackwardRevisit(const ReadLabel *rLab, const WriteLabel *sLab)
{
	if (!getConf()->helper)
		return std::make_unique<BackwardRevisit>(rLab, sLab);

	auto &g = getGraph();

	/* Check whether there is a conflicting RevBlocker */
	auto pending = g.getPendingRMW(sLab);
	auto *pLab = llvm::dyn_cast_or_null<WriteLabel>(g.getNextLabel(pending));
	pending = (!pending.isInitializer() && pLab->hasAttr(WriteAttr::RevBlocker)) ?
		pending.next() : Event::getInitializer();

	/* If there is, do an optimized backward revisit */
	if (!pending.isInitializer() &&
	    !g.getPrefixView(pending).contains(rLab->getPos()) &&
	    rLab->getStamp() < g.getEventLabel(pending)->getStamp() &&
	    !g.getPrefixView(sLab->getPos()).contains(pending))
		return std::make_unique<BackwardRevisitHELPER>(rLab->getPos(), sLab->getPos(), pending);
	return std::make_unique<BackwardRevisit>(rLab, sLab);
}

std::unique_ptr<ExecutionGraph>
GenMCDriver::copyGraph(const BackwardRevisit *br, VectorClock *v) const
{
	auto &g = getGraph();

	/* Adjust the view that will be used for copying */
	if (auto *brh = llvm::dyn_cast<BackwardRevisitHELPER>(br)) {
		if (auto *dv = llvm::dyn_cast<DepView>(v)) {
			dv->addHole(brh->getMid());
			dv->addHole(brh->getMid().prev());
		} else {
			--(*v)[brh->getMid().thread];
			--(*v)[brh->getMid().thread];
		}
	}

	auto og = g.getCopyUpTo(*v);

	/* Adjust stamps in the copy, and ensure the prefix of the
	 * write will not be revisitable */
	auto *revLab = og->getReadLabel(br->getPos());
	auto &prefix = og->getPrefixView(br->getRev());
	og->resetStamp(revLab->getStamp() + 1);

	for (auto *lab : labels(*og)) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (rLab && prefix.contains(rLab->getPos()))
				rLab->setRevisitStatus(false);
		}
		if (lab->getStamp() > revLab->getStamp())
			lab->setStamp(og->nextStamp());
	}
	return og;
}

bool GenMCDriver::checkRevBlockHELPER(const WriteLabel *sLab, const std::vector<Event> &loads)
{
	if (!getConf()->helper || !sLab->hasAttr(WriteAttr::RevBlocker))
		return true;

	auto &g = getGraph();
	if (std::any_of(loads.begin(), loads.end(), [this, &g, sLab](const Event &l){
		auto *lLab = g.getLastThreadLabel(l.thread);
		auto *pLab = this->getPreviousVisibleAccessLabel(lLab->getPos());
		return (llvm::isa<BlockLabel>(lLab) ||
			getEE()->getThrById(lLab->getThread()).isBlocked()) &&
			pLab && pLab->getPos() == l;
	})) {
		moot();
		return false;
	}
	return true;
}

bool GenMCDriver::calcRevisits(const WriteLabel *sLab)
{
	auto &g = getGraph();

	auto loads = getRevisitableApproximation(sLab);
	if (tryOptimizeRevisits(sLab, loads))
		return true;

	for (auto &l : loads) {
		auto *rLab = g.getReadLabel(l);
		BUG_ON(!rLab);

		auto br = constructBackwardRevisit(rLab, sLab);
		if (!g.isMaximalExtension(*br))
			break;

		/* Optimize handling of lock operations */
		if (llvm::isa<LockCasReadLabel>(rLab) && llvm::isa<UnlockWriteLabel>(sLab)) {
			if (tryRevisitLockInPlace(*br))
				break;
			moot();
		}

		GENMC_DEBUG(checkForDuplicateRevisit(rLab, sLab););

		auto v = g.getRevisitView(*br);
		auto og = copyGraph(&*br, &*v);
		auto read = rLab->getPos();
		auto write = sLab->getPos(); /* prefetch since we are gonna change state */

		auto localState = releaseLocalState();
		auto newState = std::make_unique<SharedState>(std::move(og), getEE()->getSharedState());

		setSharedState(std::move(newState));

		notifyEERemoved(*v);
		revisitRead(BackwardRevisit(read, write));

		/* If there are idle workers in the thread pool,
		 * try submitting the job instead */
		auto *tp = getThreadPool();
		if (tp && tp->getRemainingTasks() < 8 * tp->size()) {
			tp->submit(getSharedState());
		} else {
			if (isConsistent(ProgramPoint::step))
				explore();
		}

		restoreLocalState(std::move(localState));
	}

	return checkAtomicity(sLab) && checkRevBlockHELPER(sLab, loads) && !isMoot();
}

void GenMCDriver::repairLock(LockCasReadLabel *lab)
{
	auto &g = getGraph();

	for (auto rit = store_rbegin(g, lab->getAddr()), re = store_rend(g, lab->getAddr()); rit != re; ++rit) {
		auto *posRf = llvm::dyn_cast<WriteLabel>(g.getEventLabel(*rit));
		if (llvm::isa<LockCasWriteLabel>(posRf) || llvm::isa<TrylockCasWriteLabel>(posRf)) {
			auto prev = posRf->getPos().prev();
			if (g.getMatchingUnlock(prev).isInitializer()) {
				changeRf(lab->getPos(), posRf->getPos());
				threadPrios = { posRf->getPos() };
				g.addOtherLabelToGraph(BlockLabel::create(lab->getPos().next(),
									  BlockageType::LockNotAcq));
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
	if (getConf()->disableBAM)
		return;

	/* The wait-load of a barrier may lose its rf after cutting the graph.
	 * If this happens, fix the problem by making it read from the barrier's
	 * increment operation, and add a corresponding block event */
	auto &g = getGraph();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto *bLab = llvm::dyn_cast<BWaitReadLabel>(g.getLastThreadLabel(i));
		if (!bLab)
			continue;
		auto iVal = getBarrierInitValue(bLab->getAddr(), bLab->getAccess());
		if (g.contains(bLab->getRf()) && getReadValue(bLab) == iVal)
			continue;

		BUG_ON(!llvm::isa<BIncFaiWriteLabel>(g.getPreviousLabel(bLab)));
		BUG_ON(!g.contains(bLab->getPos()));
		changeRf(bLab->getPos(), bLab->getPos().prev());
		bLab->setAddedMax(true);
		g.addOtherLabelToGraph(BlockLabel::create(bLab->getPos().next(),
							  BlockageType::Barrier));
	}
	return;
}

const WriteLabel *GenMCDriver::completeRevisitedRMW(const ReadLabel *rLab)
{
	/* Handle non-RMW cases first */
	if (!llvm::isa<CasReadLabel>(rLab) && !llvm::isa<FaiReadLabel>(rLab))
		return nullptr;
	if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		if (getReadValue(rLab) != casLab->getExpected())
			return nullptr;
	}

	SVal result;
	WriteAttr wattr = WriteAttr::None;
	if (auto *faiLab = llvm::dyn_cast<FaiReadLabel>(rLab)) {
		/* Need to get the rf value within the if, as rLab might be a disk op,
		 * and we cannot get the value in that case (but it will also not be an RMW)  */
		auto rfVal = getReadValue(rLab);
		result = getEE()->executeAtomicRMWOperation(rfVal, faiLab->getOpVal(),
							    faiLab->getSize(), faiLab->getOp());
		if (llvm::isa<BIncFaiReadLabel>(faiLab) && result == SVal(0))
			    result = getBarrierInitValue(rLab->getAddr(), rLab->getAccess());
		wattr = faiLab->getAttr();
	} else if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		result = casLab->getSwapVal();
		wattr = casLab->getAttr();
	} else
		BUG();

	auto &g = getGraph();
	std::unique_ptr<WriteLabel> wLab = nullptr;

#define CREATE_COUNTERPART(name)					\
	case EventLabel::EL_## name ## Read:				\
		wLab = name##WriteLabel::create(g.nextStamp(), rLab->getOrdering(), \
						rLab->getPos().next(),	\
						rLab->getAddr(),	\
						rLab->getSize(),	\
						rLab->getType(), result, wattr); \
		break;

	switch (rLab->getKind()) {
		CREATE_COUNTERPART(BIncFai);
		CREATE_COUNTERPART(NoRetFai);
		CREATE_COUNTERPART(Fai);
		CREATE_COUNTERPART(LockCas);
		CREATE_COUNTERPART(TrylockCas);
		CREATE_COUNTERPART(Cas);
		CREATE_COUNTERPART(HelpedCas);
		CREATE_COUNTERPART(ConfirmingCas);
	default:
		BUG();
	}
	BUG_ON(!wLab);
	updateLabelViews(wLab.get(), nullptr);
	return g.addWriteLabelToGraph(std::move(wLab), rLab->getRf());
}

bool GenMCDriver::revisitRead(const ReadRevisit &ri)
{
	/* We are dealing with a read: change its reads-from and also check
	 * whether a part of an RMW should be added */
	auto &g = getGraph();
	auto *rLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(ri.getPos()));
	BUG_ON(!rLab);

	changeRf(rLab->getPos(), ri.getRev());
	auto *fri = llvm::dyn_cast<ForwardRevisit>(&ri);
	rLab->setAddedMax(fri ? fri->isMaximal() : isCoMaximal(rLab->getAddr(), ri.getRev()));

	GENMC_DEBUG(
		if (getConf()->vLevel >= VerbosityLevel::V2) {
			llvm::dbgs() << "--- " << (llvm::isa<BackwardRevisit>(ri) ? "Backward" : "Forward")
			<< " revisiting " << ri.getPos()
			<< " <-- " << ri.getRev() << "\n";
			printGraph();
		}
	);

	/* Repair barriers here, as dangling wait-reads may be part of the prefix */
	repairDanglingBarriers();

	/* If the revisited label became an RMW, add the store part and revisit */
	if (auto *sLab = completeRevisitedRMW(rLab))
		return calcRevisits(sLab);

	/* Blocked lock -> prioritize locking thread */
	repairDanglingLocks();
	if (llvm::isa<LockCasReadLabel>(rLab)) {
		g.addOtherLabelToGraph(BlockLabel::create(rLab->getPos().next(), BlockageType::LockNotAcq));
		threadPrios = {rLab->getRf()};
	}
	auto *oLab = g.getPreviousLabelST(rLab, [&](const EventLabel *oLab){
		return llvm::isa<SpeculativeReadLabel>(oLab);
	});

	if (llvm::isa<SpeculativeReadLabel>(rLab) || oLab)
		threadPrios = {rLab->getPos()};
	return true;
}

bool GenMCDriver::restrictAndRevisit(WorkSet::ItemT item)
{
	auto &g = getGraph();
	auto *EE = getEE();
	EventLabel *lab = g.getEventLabel(item->getPos());

	/* First, appropriately restrict the worklist, the revisit set, and the graph */
	restrictWorklist(lab);
	restrictRevisitSet(lab);
	restrictGraph(lab);

	/* Handle special cases first: if we are restricting to a write, change its MO position */
	if (auto *mi = llvm::dyn_cast<WriteRevisit>(item.get())) {
		auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
		BUG_ON(!wLab);
		g.changeStoreOffset(wLab->getAddr(), wLab->getPos(), mi->getMOPos());
		wLab->setAddedMax(false);
		repairDanglingLocks();
		repairDanglingBarriers();
		return calcRevisits(wLab);
	} else if (auto *oi = llvm::dyn_cast<OptionalRevisit>(item.get())) {
		auto *oLab = llvm::dyn_cast<OptionalLabel>(lab);
		--result.exploredBlocked;
		BUG_ON(!oLab);
		oLab->setExpandable(false);
		oLab->setExpanded(true);
		return true;
	}

	/* Otherwise, handle the read case */
	auto *ri = llvm::dyn_cast<ReadRevisit>(item.get());
	BUG_ON(!ri);
	return revisitRead(*ri);
}

SVal GenMCDriver::visitDskRead(std::unique_ptr<DskReadLabel> drLab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph()) {
		auto *rLab = llvm::dyn_cast<DskReadLabel>(g.getEventLabel(drLab->getPos()));
		BUG_ON(!rLab);
		return getDskReadValue(rLab);
	}

	/* Make the graph aware of a (potentially) new memory location */
	g.trackCoherenceAtLoc(drLab->getAddr());

	/* Get all stores to this location from which we can read from */
	auto validStores = getRfsApproximation(&*drLab);
	BUG_ON(validStores.empty());

	/* ... and add an appropriate label with a particular rf */
	if (inRecoveryMode())
		drLab->setOrdering(llvm::AtomicOrdering::Monotonic);
	updateLabelViews(drLab.get(), nullptr);
	const ReadLabel *lab = g.addReadLabelToGraph(std::move(drLab), validStores[0]);

	/* ... filter out all option that make the recovery invalid */
	filterInvalidRecRfs(lab, validStores);

	/* Push all the other alternatives choices to the Stack */
	for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
		addToWorklist(std::make_unique<ForwardRevisit>(lab->getPos(), *it));
	return getDskWriteValue(validStores[0], lab->getAddr(), lab->getAccess());
}

void GenMCDriver::visitDskWrite(std::unique_ptr<DskWriteLabel> wLab)
{
	if (isExecutionDrivenByGraph())
		return;

	auto &g = getGraph();

	g.trackCoherenceAtLoc(wLab->getAddr());

	/* Disk writes should always be hb-ordered */
	auto placesRange = g.getCoherentPlacings(wLab->getAddr(), wLab->getPos(), false);
	auto &begO = placesRange.first;
	auto &endO = placesRange.second;
	BUG_ON(begO != endO);

	/* Safe to _only_ add it at the end of MO */
	updateLabelViews(wLab.get(), nullptr);
	auto *lab = g.addWriteLabelToGraph(std::move(wLab), endO);

	calcRevisits(lab);
	return;
}

SVal GenMCDriver::visitDskOpen(std::unique_ptr<DskOpenLabel> lab)
{
	auto &g = getGraph();

	if (isExecutionDrivenByGraph()) {
		auto *oLab = llvm::dyn_cast<DskOpenLabel>(g.getEventLabel(lab->getPos()));
		BUG_ON(!oLab);
		return oLab->getFd();
	}

	/* We get a fresh file descriptor for this open() */
	auto fd = EE->getFreshFd();
	ERROR_ON(fd == -1, "Too many calls to open()!\n");

	/* We add a relevant label to the graph... */
	lab->setFd(SVal(fd));
	updateLabelViews(lab.get(), nullptr);
	g.addOtherLabelToGraph(std::move(lab));

	/* Return the freshly allocated fd */
	return SVal(fd);
}

void GenMCDriver::visitDskFsync(std::unique_ptr<DskFsyncLabel> fLab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(fLab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::visitDskSync(std::unique_ptr<DskSyncLabel> fLab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(fLab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::visitDskPbarrier(std::unique_ptr<DskPbarrierLabel> fLab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(fLab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::visitHelpingCas(std::unique_ptr<HelpingCasLabel> hLab, const EventDeps *deps)
{
	if (isExecutionDrivenByGraph())
		return;

	/* Before adding it to the graph, ensure that the helped CAS exists */
	auto &thr = getEE()->getCurThr();
	if (!checkHelpingCasCondition(&*hLab)) {
		thr.block(BlockageType::HelpedCas);
		thr.rollToSnapshot();
		return;
	}

	updateLabelViews(hLab.get(), deps);
	getGraph().addOtherLabelToGraph(std::move(hLab));
	return;
}

bool GenMCDriver::visitOptional(std::unique_ptr<OptionalLabel> lab)
{
	if (isExecutionDrivenByGraph())
		return llvm::dyn_cast<OptionalLabel>(getCurrentLabel())->isExpanded();

	auto &g = getGraph();
	if (std::any_of(label_begin(g), label_end(g), [&](const EventLabel *lab){
		auto *oLab = llvm::dyn_cast<OptionalLabel>(lab);
		return oLab && !oLab->isExpandable();
	}))
		lab->setExpandable(false);

	updateLabelViews(lab.get(), nullptr);
	auto *oLab = llvm::dyn_cast<OptionalLabel>(g.addOtherLabelToGraph(std::move(lab)));

	if (oLab->isExpandable())
		addToWorklist(std::make_unique<OptionalRevisit>(oLab->getPos()));
	return false; /* should not be expanded yet */
}

void GenMCDriver::visitLoopBegin(std::unique_ptr<LoopBeginLabel> lab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(lab.get(), nullptr);
	getGraph().addOtherLabelToGraph(std::move(lab));
	return;
}

bool GenMCDriver::isWriteEffectful(const WriteLabel *wLab)
{
	auto &g = getGraph();
	auto *xLab = llvm::dyn_cast<FaiWriteLabel>(wLab);
	auto *rLab = llvm::dyn_cast<FaiReadLabel>(g.getPreviousLabel(wLab));
	if (!xLab || rLab->getOp() != llvm::AtomicRMWInst::BinOp::Xchg)
		return true;

	return getReadValue(rLab) != xLab->getVal();
}

bool GenMCDriver::isWriteObservable(const WriteLabel *wLab)
{
	if (wLab->isAtLeastRelease() || !wLab->getAddr().isDynamic())
		return true;

	auto &g = getGraph();
	auto *mLab = g.getPreviousLabelST(wLab, [wLab](const EventLabel *lab){
		if (auto *aLab = llvm::dyn_cast<MallocLabel>(lab)) {
			if (aLab->contains(wLab->getAddr()))
				return true;
		}
		return false;
	});
	if (mLab == nullptr)
		return true;

	for (auto j = mLab->getIndex() + 1; j < wLab->getIndex(); j++) {
		auto *lab = g.getEventLabel(Event(wLab->getThread(), j));
		if (lab->isAtLeastRelease())
			return true;
		/* The location must not be read (loop counter) */
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
			if (rLab->getAddr() == wLab->getAddr())
				return true;
	}
	return false;
}

void GenMCDriver::visitSpinStart(std::unique_ptr<SpinStartLabel> lab)
{
	auto &g = getGraph();

	/* If it has not been added to the graph, do so */
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(lab.get(), nullptr);
	auto *stLab = g.addOtherLabelToGraph(std::move(lab));

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
		if (wLab && isWriteEffectful(wLab) && isWriteObservable(wLab))
			return; /* found event w/ side-effects */
	}
	/* Spinloop detected */
	visitBlock(BlockLabel::create(stLab->getPos().next(), BlockageType::Spinloop));
	return;
}

bool GenMCDriver::areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab)
{
	auto &g = getGraph();

	/* Check that there are no other side-effects since the previous iteration.
	 * We don't have to look for a BEGIN label since ZNE labels are always
	 * preceded by a spin-start */
	auto *ssLab = g.getPreviousLabelST(lab, [](const EventLabel *lab){
		return llvm::isa<SpinStartLabel>(lab);
	});
	BUG_ON(!ssLab);
	for (auto i = ssLab->getIndex() + 1; i < lab->getIndex(); ++i) {
		auto *oLab = g.getEventLabel(Event(ssLab->getThread(), i));
		if (llvm::isa<WriteLabel>(oLab) && !llvm::isa<FaiWriteLabel>(oLab))
			return false;
	}

	auto *wLab = llvm::dyn_cast<FaiWriteLabel>(
		g.getPreviousLabelST(lab, [](const EventLabel *lab){ return llvm::isa<FaiWriteLabel>(lab); }));
	BUG_ON(!wLab);

	/* All stores in the RMW chain need to be read from at most 1 read,
	 * and there need to be no other stores that are not hb-before lab */
	for (auto *lab : labels(g)) {
		if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab)) {
			if (mLab->getAddr() == wLab->getAddr() && !llvm::isa<FaiReadLabel>(mLab) &&
			    !llvm::isa<FaiWriteLabel>(mLab) && !isHbBefore(mLab->getPos(), wLab->getPos()))
				return false;
		}
	}
	return true;
}

void GenMCDriver::visitFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	/* If there are more events after this one, it is not a spin loop*/
	if (isExecutionDrivenByGraph() &&
	    lab->getIndex() < g.getLastThreadEvent(lab->getThread()).index)
		return;

	updateLabelViews(lab.get(), nullptr);
	auto *eLab = g.addOtherLabelToGraph(std::move(lab)); /* might overwrite but that's ok */
	if (areFaiZNEConstraintsSat(llvm::dyn_cast<FaiZNESpinEndLabel>(eLab)))
		visitBlock(BlockLabel::create(eLab->getPos().next(), BlockageType::FaiZNESpinloop));
	return;
}

void GenMCDriver::visitLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab)
{
	if (isExecutionDrivenByGraph())
		return;

	updateLabelViews(lab.get(), nullptr);
	auto *eLab = getGraph().addOtherLabelToGraph(std::move(lab));

	visitBlock(BlockLabel::create(eLab->getPos().next(), BlockageType::LockZNESpinloop));
	return;
}


/************************************************************
 ** Printing facilities
 ***********************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s,
			      const GenMCDriver::Status &st)
{
	using Status = GenMCDriver::Status;

	switch (st) {
	case Status::VS_OK:
		return s << "OK";
	case Status::VS_Safety:
		return s << "Safety violation";
	case Status::VS_Recovery:
		return s << "Recovery error";
	case Status::VS_Liveness:
		return s << "Liveness violation";
	case Status::VS_RaceNotAtomic:
		return s << "Non-Atomic race";
	case Status::VS_RaceFreeMalloc:
		return s << "Malloc-Free race";
	case Status::VS_FreeNonMalloc:
		return s << "Attempt to free non-allocated memory";
	case Status::VS_DoubleFree:
		return s << "Double-free error";
	case Status::VS_Allocation:
		return s << "Allocation error";
	case Status::VS_UninitializedMem:
		return s << "Attempt to read from uninitialized memory";
	case Status::VS_AccessNonMalloc:
		return s << "Attempt to access non-allocated memory";
	case Status::VS_AccessFreed:
		return s << "Attempt to access freed memory";
	case Status::VS_InvalidJoin:
		return s << "Invalid join() operation";
	case Status::VS_InvalidUnlock:
		return s << "Invalid unlock() operation";
	case Status::VS_InvalidBInit:
		return s << "Invalid barrier_init() operation";
	case Status::VS_InvalidRecoveryCall:
		return s << "Invalid function call during recovery";
	case Status::VS_InvalidTruncate:
		return s << "Invalid file truncation";
	case Status::VS_Annotation:
		return s << "Annotation error";
	case Status::VS_MixedSize:
		return s << "Mixed-size accesses";
	case Status::VS_SystemError:
		return s << errorList.at(systemErrorNumber);
	default:
		return s << "Uknown status";
	}
}

static void executeMDPrint(const EventLabel *lab,
			   const std::pair<int, std::string> &locAndFile,
			   std::string inputFile,
			   llvm::raw_ostream &os = llvm::outs())
{
	std::string errPath = locAndFile.second;
	Parser::stripSlashes(errPath);
	Parser::stripSlashes(inputFile);

	os << " ";
	if (errPath != inputFile)
		os << errPath << ":";
	else
		os << "L.";
	os << locAndFile.first;
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
		return mLab->getAllocAddr().isHeap() && !mLab->getAllocAddr().isInternal();

	return true;
}

std::string GenMCDriver::getVarName(const SAddr &addr) const
{
	if (addr.isStatic())
		return getEE()->getStaticName(addr);

	const auto &g = getGraph();
	auto a = g.getMalloc(addr);

	if (a.isInitializer())
		return "???";

	auto *aLab = llvm::dyn_cast<MallocLabel>(g.getEventLabel(a));
	BUG_ON(!aLab);
	if (aLab->getNameInfo())
		return aLab->getName() +
		       aLab->getNameInfo()->getNameAtOffset(addr - aLab->getAllocAddr());
	return "";
}

#ifdef ENABLE_GENMC_DEBUG
llvm::raw_ostream::Colors getLabelColor(const EventLabel *lab)
{
	auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab);
	if (!mLab)
		return llvm::raw_ostream::Colors::WHITE;

	if (llvm::isa<ReadLabel>(mLab) && !llvm::dyn_cast<ReadLabel>(mLab)->isRevisitable())
		return llvm::raw_ostream::Colors::RED;
	if (mLab->wasAddedMax())
		return llvm::raw_ostream::Colors::GREEN;
	return llvm::raw_ostream::Colors::WHITE;
}
#endif

void GenMCDriver::printGraph(bool printMetadata /* false */, llvm::raw_ostream &s /* = llvm::dbgs() */)
{
	auto &g = getGraph();
	LabelPrinter printer([this](const SAddr &saddr){ return getVarName(saddr); },
			     [this](const ReadLabel &lab){
				     return llvm::isa<DskReadLabel>(&lab) ?
					     getDskReadValue(llvm::dyn_cast<DskReadLabel>(&lab)) :
					     getReadValue(&lab);
			     });

	/* Print the graph */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);
		s << thr;
		if (getConf()->symmetryReduction) {
			if (auto *bLab = g.getFirstThreadLabel(i)) {
				auto symm = bLab->getSymmetricTid();
				if (symm != -1) s << " symmetric with " << symm;
			}
		}
		s << ":\n";
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			s << "\t";
			GENMC_DEBUG(
				if (getConf()->colorAccesses)
					s.changeColor(getLabelColor(lab));
			);
			s << printer.toString(*lab);
			GENMC_DEBUG(s.resetColor(););
			GENMC_DEBUG(
				if (getConf()->vLevel >= VerbosityLevel::V1)
					s << " @ " << lab->getStamp();
			);
			if (printMetadata && thr.prefixLOC[j].first && shouldPrintLOC(lab)) {
				executeMDPrint(lab, thr.prefixLOC[j], getConf()->inputFile, s);
			}
			s << "\n";
		}
	}

	/* MO: Print coherence information */
	auto header = false;
	if (auto *mm = llvm::dyn_cast<MOCalculator>(g.getCoherenceCalculator())) {
		for (auto locIt = mm->begin(), locE = mm->end(); locIt != locE; ++locIt)
			/* Skip empty and single-store locations */
			if (mm->hasMoreThanOneStore(locIt->first)) {
				if (!header) {
					s << "Coherence:\n";
					header = true;
				}
				auto *wLab = g.getWriteLabel(*mm->store_begin(locIt->first));
				s << getVarName(wLab->getAddr()) << ": [ ";
				for (const auto &w : stores(g, locIt->first))
					s << w << " ";
				s << "]\n";
			}
	}
	s << "\n";
}

void GenMCDriver::dotPrintToFile(const std::string &filename,
				 Event errorEvent, Event confEvent)
{
	auto &g = getGraph();
	auto *EE = getEE();
	std::ofstream fout(filename);
	llvm::raw_os_ostream ss(fout);
	DotPrinter printer([this](const SAddr &saddr){ return getVarName(saddr); },
			     [this](const ReadLabel &lab){
				     return llvm::isa<DskReadLabel>(&lab) ?
					     getDskReadValue(llvm::dyn_cast<DskReadLabel>(&lab)) :
					     getReadValue(&lab);
			     });

	auto *before = g.getPrefixView(errorEvent).clone();
	if (!confEvent.isInitializer())
		before->update(g.getPrefixView(confEvent));

	/* Create a directed graph graph */
	ss << "strict digraph {\n";
	/* Specify node shape */
	ss << "node [shape=plaintext]\n";
	/* Left-justify labels for clusters */
	ss << "labeljust=l\n";
	/* Draw straight lines */
	ss << "splines=false\n";

	/* Print all nodes with each thread represented by a cluster */
	for (auto i = 0u; i < before->size(); i++) {
		auto &thr = EE->getThrById(i);
		ss << "subgraph cluster_" << thr.id << "{\n";
		ss << "\tlabel=\"" << thr.threadFun->getName().str() << "()\"\n";
		for (auto j = 1; j <= (*before)[i]; j++) {
			auto *lab = g.getEventLabel(Event(i, j));

			ss << "\t\"" << lab->getPos() << "\" [label=<";

			/* First, print the graph label for this node */
			ss << printer.toString(*lab);

			/* And then, print the corresponding line number */
			if (thr.prefixLOC[j].first && shouldPrintLOC(lab)) {
				ss << " <FONT COLOR=\"gray\">";
				executeMDPrint(lab, thr.prefixLOC[j], getConf()->inputFile, ss);
				ss << "</FONT>";
			}

			ss << ">"
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
			auto *lab = g.getEventLabel(Event(i, j));

			/* Print a po-edge, but skip dummy start events for
			 * all threads except for the first one */
			if (j < (*before)[i] && !llvm::isa<ThreadStartLabel>(lab))
				ss << "\"" << lab->getPos() << "\" -> \""
				   << lab->getPos().next() << "\"\n";
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				/* Do not print RFs from INIT, BOTTOM, and same thread */
				if (!rLab->getRf().isInitializer() &&
				    !rLab->getRf().isBottom() &&
				    rLab->getRf().thread != lab->getPos().thread) {
					ss << "\"" << rLab->getRf() << "\" -> \""
					   << rLab->getPos() << "\"[color=green, constraint=false]\n";
				}
			}
			if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab)) {
				if (thr.id == 0)
					continue;
				ss << "\"" << bLab->getParentCreate() << "\" -> \""
				   << bLab->getPos().next() << "\"[color=blue, constraint=false]\n";
			}
			if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab))
				ss << "\"" << jLab->getChildLast() << "\" -> \""
				   << jLab->getPos() << "\"[color=blue, constraint=false]\n";
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

void GenMCDriver::printTraceBefore(Event e, llvm::raw_ostream &s /* = llvm::dbgs() */)
{
	s << "Trace to " << e << ":\n";

	/* Linearize (po U rf) and print trace */
	View a;
	recPrintTraceBefore(e, a, s);
}
