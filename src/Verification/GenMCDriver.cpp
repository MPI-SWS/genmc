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

#include "GenMCDriver.hpp"
#include "Config/Config.hpp"
#include "ExecutionGraph/Consistency/BoundDecider.hpp"
#include "ExecutionGraph/Consistency/ConsistencyChecker.hpp"
#include "ExecutionGraph/Consistency/SymmetryChecker.hpp"
#include "ExecutionGraph/DepExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/GraphUtils.hpp"
#include "ExecutionGraph/LabelVisitor.hpp"
#include "Runtime/Interpreter.h"
#include "Static/LLVMModule.hpp"
#include "Support/Error.hpp"
#include "Support/Logger.hpp"
#include "Support/Parser.hpp"
#include "Support/SExprVisitor.hpp"
#include "Support/ThreadPool.hpp"
#include "Verification/DriverHandlerDispatcher.hpp"
#include "config.h"
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
			 std::unique_ptr<ModuleInfo> modInfo, ThreadPool *pool /* = nullptr */,
			 Mode mode /* = VerificationMode{} */)
	: userConf(std::move(conf)), pool(pool), mode(mode)
{
	/* Set up the execution context */
	auto initValGetter = [this](const auto &access) { return getEE()->getLocInitVal(access); };
	auto execGraph = userConf->isDepTrackingModel
				 ? std::make_unique<DepExecutionGraph>(initValGetter)
				 : std::make_unique<ExecutionGraph>(initValGetter);
	execStack.emplace_back(std::move(execGraph), std::move(LocalQueueT()),
			       std::move(ChoiceMap()), std::move(SAddrAllocator()),
			       Event::getInit());

	consChecker = ConsistencyChecker::create(getConf()->model);
	symmChecker = SymmetryChecker::create();
	auto hasBounder = userConf->bound.has_value();
	GENMC_DEBUG(hasBounder |= userConf->boundsHistogram;);
	if (hasBounder)
		bounder = BoundDecider::create(getConf()->boundType);

	/* Create an interpreter for the program's instructions */
	std::string buf;
	EE = llvm::Interpreter::create(std::move(mod), std::move(modInfo), this, getConf(),
				       getExec().getAllocator(), &buf);

	/* Set up a random-number generator (for the scheduler) */
	std::random_device rd;
	auto seedVal = (!userConf->randomScheduleSeed.empty())
			       ? (MyRNG::result_type)stoull(userConf->randomScheduleSeed)
			       : rd();
	if (userConf->printRandomScheduleSeed) {
		PRINT(VerbosityLevel::Error) << "Seed: " << seedVal << "\n";
	}
	rng.seed(seedVal);
	estRng.seed(rd());

	/*
	 * Make sure we can resolve symbols in the program as well. We use 0
	 * as an argument in order to load the program, not a library. This
	 * is useful as it allows the executions of external functions in the
	 * user code.
	 */
	std::string ErrorStr;
	if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr, &ErrorStr)) {
		WARN("Could not resolve symbols in the program: " + ErrorStr);
	}
}

GenMCDriver::~GenMCDriver() = default;

GenMCDriver::Execution::Execution(std::unique_ptr<ExecutionGraph> g, LocalQueueT &&w, ChoiceMap &&m,
				  SAddrAllocator &&alloctor, Event lastAdded)
	: graph(std::move(g)), workqueue(std::move(w)), choices(std::move(m)),
	  alloctor(std::move(alloctor)), lastAdded(lastAdded)
{}
GenMCDriver::Execution::~Execution() = default;

static void repairRead(ExecutionGraph &g, ReadLabel *lab)
{
	auto *maxLab = g.co_max(lab->getAddr());
	lab->setRf(maxLab);
	lab->setAddedMax(true);
	lab->setIPRStatus(maxLab->getStamp() > lab->getStamp());
}

static void repairDanglingReads(ExecutionGraph &g)
{
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *rLab = llvm::dyn_cast<ReadLabel>(g.getLastThreadLabel(i));
		if (rLab && !rLab->getRf()) {
			repairRead(g, rLab);
		}
	}
}

static auto createAllocView(const ExecutionGraph &g) -> View
{
	View v;
	for (auto t : g.thr_ids()) {
		v.setMax({t, 1});
		for (auto &lab : g.po(t)) {
			auto *mLab = llvm::dyn_cast<MallocLabel>(&lab);
			if (!mLab)
				continue;
			if (mLab->getAllocAddr().isDynamic())
				v.setMax({t, static_cast<int>((mLab->getAllocAddr().get() &
							       SAddr::indexMask) +
							      mLab->getAllocSize())});
		}
	}
	return v;
}

void GenMCDriver::Execution::restrict(Stamp stamp)
{
	auto &g = getGraph();
	g.cutToStamp(stamp);
	repairDanglingReads(g);
	getChoiceMap().cut(*g.getViewFromStamp(stamp));
	getAllocator().restrict(createAllocView(g));
}

void GenMCDriver::pushExecution(Execution &&e) { execStack.push_back(std::move(e)); }

bool GenMCDriver::popExecution()
{
	if (execStack.empty())
		return false;
	execStack.pop_back();
	return !execStack.empty();
}

void GenMCDriver::initFromState(std::unique_ptr<Execution> exec)
{
	execStack.clear();
	execStack.emplace_back(std::move(exec->graph), LocalQueueT(), std::move(exec->choices),
			       std::move(exec->alloctor), exec->lastAdded);

	/* We have to also reset the initvalgetter */
	getExec().getGraph().setInitValGetter(
		[&](auto &access) { return getEE()->getLocInitVal(access); });
}

std::unique_ptr<GenMCDriver::Execution> GenMCDriver::extractState()
{
	return std::make_unique<Execution>(GenMCDriver::Execution(
		getExec().getGraph().clone(), LocalQueueT(), ChoiceMap(getExec().getChoiceMap()),
		SAddrAllocator(getExec().getAllocator()), getExec().getLastAdded()));
}

/* Returns a fresh address to be used from the interpreter */
SAddr GenMCDriver::getFreshAddr(const MallocLabel *aLab)
{
	/* The arguments to getFreshAddr() need to be well-formed;
	 * make sure the alignment is positive and a power of 2 */
	auto alignment = aLab->getAlignment();
	BUG_ON(alignment <= 0 || (alignment & (alignment - 1)) != 0);
	switch (aLab->getStorageDuration()) {
	case StorageDuration::SD_Automatic:
		return getExec().getAllocator().allocAutomatic(
			aLab->getThread(), aLab->getAllocSize(), alignment,
			aLab->getStorageType() == StorageType::ST_Durable,
			aLab->getAddressSpace() == AddressSpace::AS_Internal);
	case StorageDuration::SD_Heap:
		return getExec().getAllocator().allocHeap(
			aLab->getThread(), aLab->getAllocSize(), alignment,
			aLab->getStorageType() == StorageType::ST_Durable,
			aLab->getAddressSpace() == AddressSpace::AS_Internal);
	case StorageDuration::SD_Static: /* Cannot ask for fresh static addresses */
	default:
		BUG();
	}
	BUG();
	return SAddr();
}

void GenMCDriver::resetThreadPrioritization() { threadPrios.clear(); }

bool GenMCDriver::isSchedulable(int thread) const
{
	auto &thr = getEE()->getThrById(thread);
	auto *lab = getExec().getGraph().getLastThreadLabel(thread);
	return !thr.ECStack.empty() && !llvm::isa<TerminatorLabel>(lab);
}

bool GenMCDriver::schedulePrioritized()
{
	/* Return false if no thread is prioritized */
	if (threadPrios.empty())
		return false;

	BUG_ON(getConf()->bound.has_value());

	const auto &g = getExec().getGraph();
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
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	for (auto i = 0U; i < g.getNumThreads(); i++) {
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
	auto *ci = llvm::dyn_cast<llvm::CallInst>(I);
	return llvm::isa<llvm::LoadInst>(I) || llvm::isa<llvm::AtomicCmpXchgInst>(I) ||
	       llvm::isa<llvm::AtomicRMWInst>(I) ||
	       (ci && ci->getCalledFunction() &&
		hasGlobalLoadSemantics(ci->getCalledFunction()->getName().str()));
}

bool GenMCDriver::scheduleNextWF()
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	/* First, schedule based on the EG */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (!isSchedulable(i))
			continue;

		if (g.containsPos(Event(i, EE->getThrById(i).globalInstructions + 1))) {
			EE->scheduleThread(i);
			return true;
		}
	}

	/* Try and find a thread that satisfies the policy.
	 * Keep an LTR fallback option in case this fails */
	long fallback = -1;
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (!isSchedulable(i))
			continue;

		if (fallback == -1)
			fallback = i;
		if (!isNextThreadInstLoad(i)) {
			EE->scheduleThread(getFirstSchedulableSymmetric(i));
			return true;
		}
	}

	/* Otherwise, try to schedule the fallback thread */
	if (fallback != -1) {
		EE->scheduleThread(getFirstSchedulableSymmetric(fallback));
		return true;
	}
	return false;
}

int GenMCDriver::getFirstSchedulableSymmetric(int tid)
{
	if (!getConf()->symmetryReduction)
		return tid;

	auto &g = getExec().getGraph();
	auto firstSched = tid;
	auto symm = g.getFirstThreadLabel(tid)->getSymmPredTid();
	while (symm != -1) {
		if (isSchedulable(symm))
			firstSched = symm;
		symm = g.getFirstThreadLabel(symm)->getSymmPredTid();
	}
	return firstSched;
}

bool GenMCDriver::scheduleNextWFR()
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	/* First, schedule based on the EG */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (!isSchedulable(i))
			continue;

		if (g.containsPos(Event(i, EE->getThrById(i).globalInstructions + 1))) {
			EE->scheduleThread(i);
			return true;
		}
	}

	std::vector<int> nonwrites;
	std::vector<int> writes;
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (!isSchedulable(i))
			continue;

		if (!isNextThreadInstLoad(i)) {
			writes.push_back(i);
		} else {
			nonwrites.push_back(i);
		}
	}

	std::vector<int> &selection = !writes.empty() ? writes : nonwrites;
	if (selection.empty())
		return false;

	MyDist dist(0, selection.size() - 1);
	auto candidate = selection[dist(rng)];
	EE->scheduleThread(getFirstSchedulableSymmetric(static_cast<int>(candidate)));
	return true;
}

bool GenMCDriver::scheduleNextRandom()
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	/* Check if randomize scheduling is enabled and schedule some thread */
	MyDist dist(0, g.getNumThreads());
	auto random = dist(rng);
	for (auto j = 0u; j < g.getNumThreads(); j++) {
		auto i = (j + random) % g.getNumThreads();

		if (!isSchedulable(i))
			continue;

		/* Found a not-yet-complete thread; schedule it */
		EE->scheduleThread(getFirstSchedulableSymmetric(static_cast<int>(i)));
		return true;
	}

	/* No schedulable thread found */
	return false;
}

void GenMCDriver::resetExplorationOptions()
{
	unmoot();
	setRescheduledRead(Event::getInit());
	resetThreadPrioritization();
}

void GenMCDriver::handleExecutionStart()
{
	const auto &g = getExec().getGraph();

	/* Set-up (optimize) the interpreter for the new exploration */
	for (auto i = 1u; i < g.getNumThreads(); i++) {

		/* Skip not-yet-created threads */
		BUG_ON(g.isThreadEmpty(i));

		auto *labFst = g.getFirstThreadLabel(i);
		auto parent = labFst->getParentCreate();

		/* Skip if parent create does not exist yet (or anymore) */
		if (!g.containsPos(parent) ||
		    !llvm::isa<ThreadCreateLabel>(g.getEventLabel(parent)))
			continue;

		/* Skip finished threads */
		auto *labLast = g.getLastThreadLabel(i);
		if (llvm::isa<ThreadFinishLabel>(labLast))
			continue;

		/* Skip the recovery thread, if it exists.
		 * It will be scheduled separately afterwards */
		if (i == g.getRecoveryRoutineId())
			continue;

		/* Otherwise, initialize ECStacks in interpreter */
		auto &thr = getEE()->getThrById(i);
		BUG_ON(!thr.ECStack.empty());
		thr.ECStack = thr.initEC;
	}
}

std::pair<std::vector<SVal>, std::vector<Event>> GenMCDriver::extractValPrefix(Event pos)
{
	auto &g = getExec().getGraph();
	std::vector<SVal> vals;
	std::vector<Event> events;

	for (auto i = 0u; i < pos.index; i++) {
		auto *lab = g.getEventLabel(Event(pos.thread, i));
		if (lab->returnsValue()) {
			vals.push_back(lab->getReturnValue());
			events.push_back(lab->getPos());
		}
	}
	return {vals, events};
}

Event findNextLabelToAdd(const ExecutionGraph &g, Event pos)
{
	const auto *firstLab = g.getFirstThreadLabel(pos.thread);
	auto succs = po_succs(g, firstLab);
	auto it =
		std::ranges::find_if(succs, [&](auto &lab) { return llvm::isa<EmptyLabel>(&lab); });
	return it == succs.end() ? g.getLastThreadLabel(pos.thread)->getPos().next()
				 : (*it).getPos();
}

bool GenMCDriver::tryOptimizeScheduling(Event pos)
{
	if (!getConf()->instructionCaching || inEstimationMode())
		return false;

	auto next = findNextLabelToAdd(getExec().getGraph(), pos);
	auto [vals, last] = extractValPrefix(next);
	auto *res = retrieveCachedSuccessors(pos.thread, vals);
	if (res == nullptr || res->empty() || res->back()->getIndex() < next.index)
		return false;

	for (auto &vlab : *res) {
		BUG_ON(vlab->hasStamp());

		DriverHandlerDispatcher dispatcher(this);
		dispatcher.visit(vlab);
		if (llvm::isa<BlockLabel>(
			    getExec().getGraph().getLastThreadLabel(vlab->getThread())) ||
		    isMoot() || getEE()->getCurThr().isBlocked() || isHalting())
			return true;
	}
	return true;
}

void GenMCDriver::checkHelpingCasAnnotation()
{
	/* If we were waiting for a helped CAS that did not appear, complain */
	auto &g = getExec().getGraph();
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (llvm::isa<HelpedCASBlockLabel>(g.getLastThreadLabel(i)))
			ERROR("Helped/Helping CAS annotation error! Does helped CAS always "
			      "execute?\n");
	}

	/* Next, we need to check whether there are any extraneous
	 * stores, not visible to the helped/helping CAS */
	for (auto &lab : g.labels() | std::views::filter([](auto &lab) {
				 return llvm::isa<HelpingCasLabel>(&lab);
			 })) {
		auto *hLab = llvm::dyn_cast<HelpingCasLabel>(&lab);

		/* Check that all stores that would make this helping
		 * CAS succeed are read by a helped CAS.
		 * We don't need to check the swap value of the helped CAS */
		if (std::any_of(g.co_begin(hLab->getAddr()), g.co_end(hLab->getAddr()),
				[&](auto &sLab) {
					return hLab->getExpected() == sLab.getVal() &&
					       std::none_of(
						       sLab.readers_begin(), sLab.readers_end(),
						       [&](auto &rLab) {
							       return llvm::isa<HelpedCasReadLabel>(
								       &rLab);
						       });
				}))
			ERROR("Helped/Helping CAS annotation error! "
			      "Unordered store to helping CAS location!\n");

		/* Special case for the initializer (as above) */
		if (hLab->getAddr().isStatic() &&
		    hLab->getExpected() == getEE()->getLocInitVal(hLab->getAccess())) {
			auto rsView = g.labels() | std::views::filter([hLab](auto &lab) {
					      auto *rLab = llvm::dyn_cast<ReadLabel>(&lab);
					      return rLab && rLab->getAddr() == hLab->getAddr();
				      });
			if (std::ranges::none_of(rsView, [&](auto &lab) {
				    return llvm::isa<HelpedCasReadLabel>(&lab);
			    }))
				ERROR("Helped/Helping CAS annotation error! "
				      "Unordered store to helping CAS location!\n");
		}
	}
	return;
}

#ifdef ENABLE_GENMC_DEBUG
void GenMCDriver::trackExecutionBound()
{
	auto bound = bounder->calculate(getExec().getGraph());
	result.exploredBounds.grow(bound);
	result.exploredBounds[bound]++;
}
#endif

bool GenMCDriver::isExecutionBlocked() const
{
	return std::any_of(
		getEE()->threads_begin(), getEE()->threads_end(), [this](const llvm::Thread &thr) {
			// FIXME: was thr.isBlocked()
			auto &g = getExec().getGraph();
			if (thr.id >= g.getNumThreads() || g.isThreadEmpty(thr.id)) // think rec
				return false;
			return llvm::isa<BlockLabel>(g.getLastThreadLabel(thr.id));
		});
}

void GenMCDriver::updateStSpaceEstimation()
{
	/* Calculate current sample */
	auto &choices = getExec().getChoiceMap();
	auto sample = std::accumulate(choices.begin(), choices.end(), 1.0L,
				      [](auto sum, auto &kv) { return sum *= kv.second.size(); });

	/* This is the (i+1)-th exploration */
	auto totalExplored = (long double)result.explored + result.exploredBlocked + 1L;

	/* As the estimation might stop dynamically, we can't just
	 * normalize over the max samples to avoid overflows. Instead,
	 * use Welford's online algorithm to calculate mean and
	 * variance. */
	auto prevM = result.estimationMean;
	auto prevV = result.estimationVariance;
	result.estimationMean += (sample - prevM) / totalExplored;
	result.estimationVariance +=
		(sample - prevM) / totalExplored * (sample - result.estimationMean) -
		prevV / totalExplored;
}

void GenMCDriver::handleExecutionEnd()
{
	if (isMoot()) {
		GENMC_DEBUG(++result.exploredMoot;);
		return;
	}

	/* Helper: Check helping CAS annotation */
	if (getConf()->helper)
		checkHelpingCasAnnotation();

	/* If under estimation mode, guess the total.
	 * (This may run a few times, but that's OK.)*/
	if (inEstimationMode()) {
		updateStSpaceEstimation();
		if (!shouldStopEstimating())
			getExec().getWorkqueue().add(std::make_unique<RerunForwardRevisit>());
	}

	/* Ignore the execution if some assume has failed */
	if (isExecutionBlocked()) {
		++result.exploredBlocked;
		if (getConf()->printBlockedExecs)
			printGraph();
		if (getConf()->checkLiveness)
			checkLiveness();
		return;
	}

	if (getConf()->warnUnfreedMemory)
		checkUnfreedMemory();
	if (getConf()->printExecGraphs && !getConf()->persevere)
		printGraph(); /* Delay printing if persevere is enabled */

	GENMC_DEBUG(if (getConf()->boundsHistogram && !inEstimationMode()) trackExecutionBound(););

	++result.explored;
	if (fullExecutionExceedsBound())
		++result.boundExceeding;
}

void GenMCDriver::handleRecoveryStart()
{
	BUG();
	// if (isExecutionBlocked())
	// 	return;

	// auto &g = getExec().getGraph();
	// auto *EE = getEE();

	// /* Make sure that a thread for the recovery routine is
	//  * added only once in the execution graph*/
	// if (g.getRecoveryRoutineId() == -1)
	// 	g.addRecoveryThread();

	// /* We will create a start label for the recovery thread.
	//  * We synchronize with a persistency barrier, if one exists,
	//  * otherwise, we synchronize with nothing */
	// auto tid = g.getRecoveryRoutineId();
	// auto psb = g.collectAllEvents(
	// 	[&](const EventLabel *lab) { return llvm::isa<DskPbarrierLabel>(lab); });
	// if (psb.empty())
	// 	psb.push_back(Event::getInit());
	// ERROR_ON(psb.size() > 1, "Usage of only one persistency barrier is allowed!\n");

	// auto tsLab = ThreadStartLabel::create(Event(tid, 0), psb.back(),
	// 				      ThreadInfo(tid, psb.back().thread, 0, 0));
	// auto *lab = addLabelToGraph(std::move(tsLab));

	// /* Create a thread for the interpreter, and appropriately
	//  * add it to the thread list (pthread_create() style) */
	// EE->createAddRecoveryThread(tid);

	// /* Finally, do all necessary preparations in the interpreter */
	// getEE()->setupRecoveryRoutine(tid);
	return;
}

void GenMCDriver::handleRecoveryEnd()
{
	/* Print the graph with the recovery routine */
	if (getConf()->printExecGraphs)
		printGraph();
	getEE()->cleanupRecoveryRoutine(getExec().getGraph().getRecoveryRoutineId());
	return;
}

void GenMCDriver::run()
{
	/* Explore all graphs and print the results */
	explore();
}

bool GenMCDriver::isHalting() const
{
	auto *tp = getThreadPool();
	return shouldHalt || (tp && tp->shouldHalt());
}

void GenMCDriver::halt(VerificationError status)
{
	shouldHalt = true;
	result.status = status;
	if (getThreadPool())
		getThreadPool()->halt();
}

GenMCDriver::Result GenMCDriver::verify(std::shared_ptr<const Config> conf,
					std::unique_ptr<llvm::Module> mod,
					std::unique_ptr<ModuleInfo> modInfo)
{
	/* Spawn a single or multiple drivers depending on the configuration */
	if (conf->threads == 1) {
		auto driver = GenMCDriver::create(conf, std::move(mod), std::move(modInfo));
		driver->run();
		return driver->getResult();
	}

	std::vector<std::future<GenMCDriver::Result>> futures;
	{
		/* Then, fire up the drivers */
		ThreadPool pool(conf, mod, modInfo);
		futures = pool.waitForTasks();
	}

	GenMCDriver::Result res;
	for (auto &f : futures) {
		res += f.get();
	}
	return res;
}

GenMCDriver::Result GenMCDriver::estimate(std::shared_ptr<const Config> conf,
					  const std::unique_ptr<llvm::Module> &mod,
					  const std::unique_ptr<ModuleInfo> &modInfo)
{
	auto estCtx = std::make_unique<llvm::LLVMContext>();
	auto newmod = LLVMModule::cloneModule(mod, estCtx);
	auto newMI = modInfo->clone(*newmod);
	auto driver = GenMCDriver::create(conf, std::move(newmod), std::move(newMI), nullptr,
					  GenMCDriver::EstimationMode{conf->estimationMax});
	driver->run();
	return driver->getResult();
}

/************************************************************
 ** Scheduling methods
 ***********************************************************/

void GenMCDriver::blockThread(std::unique_ptr<BlockLabel> bLab)
{
	/* There are a couple of reasons we don't call Driver::addLabelToGraph() here:
	 *   1) It's redundant to update the views of the block label
	 *   2) If addLabelToGraph() does extra stuff (e.g., event caching) we absolutely
	 *      don't want to do that here. blockThread() should be safe to call from
	 *      anywhere in the code, with no unexpected side-effects */
	auto &g = getExec().getGraph();
	if (bLab->getPos() == g.getLastThreadLabel(bLab->getThread())->getPos())
		g.removeLast(bLab->getThread());
	g.addLabelToGraph(std::move(bLab));
}

void GenMCDriver::blockThreadTryMoot(std::unique_ptr<BlockLabel> bLab)
{
	auto pos = bLab->getPos();
	blockThread(std::move(bLab));
	mootExecutionIfFullyBlocked(pos);
}

void GenMCDriver::unblockThread(Event pos)
{
	auto *bLab = getExec().getGraph().getLastThreadLabel(pos.thread);
	BUG_ON(!llvm::isa<BlockLabel>(bLab));
	getExec().getGraph().removeLast(pos.thread);
}

bool GenMCDriver::scheduleAtomicity()
{
	auto *lastLab = getExec().getGraph().getEventLabel(getExec().getLastAdded());
	if (llvm::isa<FaiReadLabel>(lastLab)) {
		getEE()->scheduleThread(lastLab->getThread());
		return true;
	}
	if (auto *casLab = llvm::dyn_cast<CasReadLabel>(lastLab)) {
		if (casLab->getAccessValue(casLab->getAccess()) == casLab->getExpected()) {
			getEE()->scheduleThread(lastLab->getThread());
			return true;
		}
	}
	return false;
}

bool GenMCDriver::scheduleNormal()
{
	if (inEstimationMode())
		return scheduleNextWFR();

	switch (getConf()->schedulePolicy) {
	case SchedulePolicy::ltr:
		return scheduleNextLTR();
	case SchedulePolicy::wf:
		return scheduleNextWF();
	case SchedulePolicy::wfr:
		return scheduleNextWFR();
	case SchedulePolicy::arbitrary:
		return scheduleNextRandom();
	default:
		BUG();
	}
	BUG();
}

bool GenMCDriver::rescheduleReads()
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	for (auto i = 0u; i < g.getNumThreads(); ++i) {
		auto *bLab = llvm::dyn_cast<ReadOptBlockLabel>(g.getLastThreadLabel(i));
		if (!bLab)
			continue;

		BUG_ON(getConf()->bound.has_value());
		setRescheduledRead(bLab->getPos());
		unblockThread(bLab->getPos());
		EE->scheduleThread(i);
		return true;
	}
	return false;
}

bool GenMCDriver::scheduleNext()
{
	if (isMoot() || isHalting())
		return false;

	auto &g = getExec().getGraph();
	auto *EE = getEE();

	/* 1. Ensure atomicity. This needs to here because of weird interactions with in-place
	 * revisiting and thread priotitization. For example, consider the following scenario:
	 *     - restore @ T2, in-place rev @ T1, prioritize rev @ T1,
	 *       restore FAIR @ T2, schedule T1, atomicity violation */
	if (scheduleAtomicity())
		return true;

	/* Check if we should prioritize some thread */
	if (schedulePrioritized())
		return true;

	/* Schedule the next thread according to the chosen policy */
	if (scheduleNormal())
		return true;

	/* Finally, check if any reads needs to be rescheduled */
	return rescheduleReads();
}

std::vector<ThreadInfo> createExecutionContext(const ExecutionGraph &g)
{
	std::vector<ThreadInfo> tis;
	for (auto i = 1u; i < g.getNumThreads(); i++) { // skip main
		auto *bLab = g.getFirstThreadLabel(i);
		BUG_ON(!bLab);
		tis.push_back(bLab->getThreadInfo());
	}
	return tis;
}

void GenMCDriver::explore()
{
	auto *EE = getEE();

	resetExplorationOptions();
	EE->setExecutionContext(createExecutionContext(getExec().getGraph()));
	while (!isHalting()) {
		EE->reset();

		/* Get main program function and run the program */
		EE->runAsMain(getConf()->programEntryFun);
		if (getConf()->persevere)
			EE->runRecovery();

		auto validExecution = false;
		while (!validExecution) {
			/*
			 * restrictAndRevisit() might deem some execution infeasible,
			 * so we have to reset all exploration options before
			 * calling it again
			 */
			resetExplorationOptions();

			auto item = getExec().getWorkqueue().getNext();
			if (!item) {
				if (popExecution())
					continue;
				return;
			}
			auto pos = item->getPos();
			validExecution = restrictAndRevisit(item) && isRevisitValid(*item);
		}
	}
}

bool isUninitializedAccess(const SAddr &addr, const Event &pos)
{
	return addr.isDynamic() && pos.isInitializer();
}

bool GenMCDriver::isExecutionValid(const EventLabel *lab)
{

	return getSymmChecker().isSymmetryOK(lab) && getConsChecker().isConsistent(lab) &&
	       !partialExecutionExceedsBound();
}

bool GenMCDriver::isRevisitValid(const Revisit &revisit)
{
	auto &g = getExec().getGraph();
	auto pos = revisit.getPos();
	auto *mLab = llvm::dyn_cast<MemAccessLabel>(g.getEventLabel(pos));

	/* E.g., for optional revisits, do nothing */
	if (!mLab)
		return true;

	if (!isExecutionValid(mLab))
		return false;

	auto *rLab = llvm::dyn_cast<ReadLabel>(mLab);
	if (rLab && checkInitializedMem(rLab) != VerificationError::VE_OK)
		return false;

	/* If an extra event is added, re-check consistency */
	auto *nLab = g.po_imm_succ(mLab);
	return !rLab || !rLab->isRMW() ||
	       (isExecutionValid(nLab) && checkForRaces(nLab) == VerificationError::VE_OK);
}

bool GenMCDriver::isExecutionDrivenByGraph(const EventLabel *lab)
{
	const auto &g = getExec().getGraph();
	auto curr = lab->getPos();
	return (curr.index < g.getThreadSize(curr.thread)) &&
	       !llvm::isa<EmptyLabel>(g.getEventLabel(curr));
}

bool GenMCDriver::executionExceedsBound(BoundCalculationStrategy strategy) const
{
	if (!getConf()->bound.has_value() || inEstimationMode())
		return false;

	return bounder->doesExecutionExceedBound(getExec().getGraph(), *getConf()->bound, strategy);
}

bool GenMCDriver::fullExecutionExceedsBound() const
{
	return executionExceedsBound(BoundCalculationStrategy::NonSlacked);
}

bool GenMCDriver::partialExecutionExceedsBound() const
{
	return executionExceedsBound(BoundCalculationStrategy::Slacked);
}

bool GenMCDriver::inRecoveryMode() const
{
	return getEE()->getProgramState() == llvm::ProgramState::Recovery;
}

bool GenMCDriver::inReplay() const
{
	return getEE()->getExecState() == llvm::ExecutionState::Replay;
}

EventLabel *GenMCDriver::addLabelToGraph(std::unique_ptr<EventLabel> lab)
{
	auto &g = getExec().getGraph();

	/* Cache the event before updating views (inits are added w/ tcreate) */
	cacheEventLabel(&*lab);

	/* Add and update views */
	auto *addedLab = g.addLabelToGraph(std::move(lab));
	updateLabelViews(addedLab);
	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(addedLab))
		g.addAlloc(findAllocatingLabel(g, mLab->getAddr()), mLab);

	getExec().getLastAdded() = addedLab->getPos();
	if (addedLab->getIndex() >= getConf()->warnOnGraphSize) {
		LOG_ONCE("large-graph", VerbosityLevel::Tip)
			<< "The execution graph seems quite large. "
			<< "Consider bounding all loops or using -unroll\n";
	}
	return addedLab;
}

void GenMCDriver::updateLabelViews(EventLabel *lab)
{
	getConsChecker().updateMMViews(lab);
	if (!getConf()->symmetryReduction)
		return;

	auto &v = lab->getPrefixView(); // FIXME: called for sideeffects
	getSymmChecker().updatePrefixWithSymmetries(lab);
}

VerificationError GenMCDriver::checkForRaces(const EventLabel *lab)
{
	if (getConf()->disableRaceDetection || inEstimationMode())
		return VerificationError::VE_OK;

	/* Bounding: extensibility not guaranteed; RD should be disabled */
	if (llvm::isa<WriteLabel>(lab) && !checkAtomicity(llvm::dyn_cast<WriteLabel>(lab))) {
		BUG_ON(!getConf()->bound.has_value());
		return VerificationError::VE_OK;
	}

	/* Check for hard errors */
	const EventLabel *racyLab = nullptr;
	auto err = getConsChecker().checkErrors(lab, racyLab);
	if (err != VerificationError::VE_OK) {
		reportError({lab->getPos(), err, "", racyLab});
		return err;
	}

	/* Check whether there are any unreported warnings... */
	std::vector<const EventLabel *> races;
	auto newWarnings = getConsChecker().checkWarnings(lab, getResult().warnings, races);

	/* ... and report them */
	auto i = 0U;
	for (auto &wcode : newWarnings) {
		if (reportWarningOnce(lab->getPos(), wcode, races[i++]))
			return wcode;
	}
	return VerificationError::VE_OK;
}

void GenMCDriver::cacheEventLabel(const EventLabel *lab)
{
	if (!getConf()->instructionCaching || inEstimationMode())
		return;

	auto &g = getExec().getGraph();

	/* Helper that copies and resets the prefix of LAB starting from index FROM. */
	auto copyPrefix = [&](auto from, auto &lab) {
		std::vector<std::unique_ptr<EventLabel>> labs;
		for (auto i = from; i <= lab->getIndex(); i++) {
			auto cLab = (i == lab->getIndex())
					    ? lab->clone()
					    : g.getEventLabel(Event(lab->getThread(), i))->clone();
			cLab->reset();
			labs.push_back(std::move(cLab));
		}
		return labs;
	};

	/* Extract value prefix and find how much of it has already been cached */
	auto [vals, indices] = extractValPrefix(lab->getPos());
	auto commonPrefixLen = seenPrefixes[lab->getThread()].findLongestCommonPrefix(vals);
	std::vector<SVal> seenVals(vals.begin(), vals.begin() + commonPrefixLen);
	auto *data = retrieveCachedSuccessors(lab->getThread(), seenVals);
	BUG_ON(!data);

	/*
	 * Fastpath: if there are no new data to cache, return.
	 * (For dep-tracking, we could optimize toIdx and collect until
	 * a new (non-empty) label with a value is found.)
	 */
	if (!data->empty() && data->back()->getIndex() >= lab->getIndex())
		return;

	/*
	 * Fetch the labels to cache. We try to copy as little as possible,
	 * by inspecting what's already cached.
	 */
	auto fromIdx = commonPrefixLen == 0 ? 0 : indices[commonPrefixLen - 1].index + 1;
	if (!data->empty())
		fromIdx = std::max(data->back()->getIndex() + 1, fromIdx);
	auto labs = copyPrefix(fromIdx, lab);

	/* Go ahead and copy */
	for (auto i = 0U; i < labs.size(); i++) {
		/* Ensure label has not already been cached */
		BUG_ON(!data->empty() && data->back()->getIndex() >= labs[i]->getIndex());
		/* If the last cached label returns a value, then we need
		 * to cache to a different bucket */
		if (!data->empty() && data->back()->returnsValue()) {
			seenVals.push_back(vals[seenVals.size()]);
			auto res = seenPrefixes[lab->getThread()].addSeq(seenVals, {});
			BUG_ON(!res);
			data = retrieveCachedSuccessors(lab->getThread(), seenVals);
		}
		data->push_back(std::move(labs[i]));
	}
}

std::optional<SVal> GenMCDriver::getReadRetValue(const ReadLabel *rLab)
{
	/* Bottom is an acceptable re-option only @ replay */
	if (!rLab->getRf()) {
		BUG_ON(!inReplay());
		return std::nullopt;
	}

	/* Reading a non-init barrier value means that the thread should block */
	auto res = rLab->getAccessValue(rLab->getAccess());
	BUG_ON(llvm::isa<BWaitReadLabel>(rLab) &&
	       res != findBarrierInitValue(getExec().getGraph(), rLab->getAccess()) &&
	       !llvm::isa<TerminatorLabel>(
		       getExec().getGraph().getLastThreadLabel(rLab->getThread())));
	return {res};
}

SVal GenMCDriver::getRecReadRetValue(const ReadLabel *rLab)
{
	auto &g = getExec().getGraph();

	/* Find and read from the latest sameloc store */
	auto preds = po_preds(g, rLab);
	auto wLabIt = std::ranges::find_if(preds, [rLab](auto &lab) {
		auto *wLab = llvm::dyn_cast<WriteLabel>(&lab);
		return wLab && wLab->getAddr() == rLab->getAddr();
	});
	BUG_ON(wLabIt == std::ranges::end(preds));
	return (*wLabIt).getAccessValue(rLab->getAccess());
}

VerificationError GenMCDriver::checkAccessValidity(const MemAccessLabel *lab)
{
	/* Static variable validity is handled by the interpreter. *
	 * Dynamic accesses are valid if they access allocated memory */
	if ((!lab->getAddr().isDynamic() && !getEE()->isStaticallyAllocated(lab->getAddr())) ||
	    (lab->getAddr().isDynamic() && !lab->getAlloc())) {
		reportError({lab->getPos(), VerificationError::VE_AccessNonMalloc});
		return VerificationError::VE_AccessNonMalloc;
	}
	return VerificationError::VE_OK;
}

VerificationError GenMCDriver::checkInitializedMem(const ReadLabel *rLab)
{
	// FIXME: Have label for mutex-destroy and check type instead of val.
	//        Also for barriers.

	/* Locks should not read from destroyed mutexes */
	const auto *lLab = llvm::dyn_cast<LockCasReadLabel>(rLab);
	if (lLab && lLab->getAccessValue(lLab->getAccess()) == SVal(-1)) {
		reportError({lLab->getPos(), VerificationError::VE_UninitializedMem,
			     "Called lock() on destroyed mutex!", lLab->getRf()});
		return VerificationError::VE_UninitializedMem;
	}

	/* Barriers should read initialized, not-destroyed memory */
	const auto *bLab = llvm::dyn_cast<BIncFaiReadLabel>(rLab);
	if (bLab && bLab->getRf()->getPos().isInitializer()) {
		reportError({rLab->getPos(), VerificationError::VE_UninitializedMem,
			     "Called barrier_wait() on uninitialized barrier!"});
		return VerificationError::VE_UninitializedMem;
	}
	if (bLab && bLab->getAccessValue(bLab->getAccess()) == SVal(0)) {
		reportError({rLab->getPos(), VerificationError::VE_AccessFreed,
			     "Called barrier_wait() on destroyed barrier!", bLab->getRf()});
		return VerificationError::VE_UninitializedMem;
	}

	/* Plain events should read initialized memory if they are dynamic accesses */
	if (isUninitializedAccess(rLab->getAddr(), rLab->getRf()->getPos())) {
		reportError({rLab->getPos(), VerificationError::VE_UninitializedMem});
		return VerificationError::VE_UninitializedMem;
	}

	/* Slightly unrelated check, but ensure there are no mixed-size accesses */
	if (rLab->getRf() && !rLab->getRf()->getPos().isInitializer() &&
	    llvm::dyn_cast<WriteLabel>(rLab->getRf())->getSize() != rLab->getSize())
		reportError({rLab->getPos(), VerificationError::VE_MixedSize,
			     "Mixed-size accesses detected: tried to read with a " +
				     std::to_string(rLab->getSize().get() * 8) + "-bit access!\n" +
				     "Please check the LLVM-IR.\n"});
	return VerificationError::VE_OK;
}

VerificationError GenMCDriver::checkInitializedMem(const WriteLabel *wLab)
{
	auto &g = getExec().getGraph();

	/* Unlocks should unlock mutexes locked by the same thread */
	const auto *uLab = llvm::dyn_cast<UnlockWriteLabel>(wLab);
	if (uLab && !findMatchingLock(uLab)) {
		reportError({uLab->getPos(), VerificationError::VE_InvalidUnlock,
			     "Called unlock() on mutex not locked by the same thread!"});
		return VerificationError::VE_InvalidUnlock;
	}

	/* Barriers should be initialized once, with a proper value */
	const auto *bLab = llvm::dyn_cast<BInitWriteLabel>(wLab);
	if (bLab && wLab->getVal() == SVal(0)) {
		reportError({wLab->getPos(), VerificationError::VE_InvalidBInit,
			     "Called barrier_init() with 0!"});
		return VerificationError::VE_InvalidBInit;
	}
	if (bLab &&
	    std::any_of(g.co_begin(bLab->getAddr()), g.co_end(bLab->getAddr()), [&](auto &sLab) {
		    return &sLab != wLab && sLab.getAddr() == wLab->getAddr() &&
			   llvm::isa<BInitWriteLabel>(sLab);
	    })) {
		reportError({wLab->getPos(), VerificationError::VE_InvalidBInit,
			     "Called barrier_init() multiple times!"});
		return VerificationError::VE_InvalidBInit;
	}
	return VerificationError::VE_OK;
}

VerificationError GenMCDriver::checkFinalAnnotations(const WriteLabel *wLab)
{
	if (!getConf()->helper)
		return VerificationError::VE_OK;

	auto &g = getExec().getGraph();

	if (g.hasLocMoreThanOneStore(wLab->getAddr()))
		return VerificationError::VE_OK;
	if ((wLab->isFinal() &&
	     std::any_of(g.co_begin(wLab->getAddr()), g.co_end(wLab->getAddr()),
			 [&](auto &sLab) {
				 return !getConsChecker().getHbView(wLab).contains(sLab.getPos());
			 })) ||
	    (!wLab->isFinal() && std::any_of(g.co_begin(wLab->getAddr()), g.co_end(wLab->getAddr()),
					     [&](auto &sLab) { return sLab.isFinal(); }))) {
		reportError({wLab->getPos(), VerificationError::VE_Annotation,
			     "Multiple stores at final location!"});
		return VerificationError::VE_Annotation;
	}
	return VerificationError::VE_OK;
}

VerificationError GenMCDriver::checkIPRValidity(const ReadLabel *rLab)
{
	if (!rLab->getAnnot() || !getConf()->ipr)
		return VerificationError::VE_OK;

	auto &g = getExec().getGraph();
	auto racyIt = std::find_if(g.co_begin(rLab->getAddr()), g.co_end(rLab->getAddr()),
				   [&](auto &wLab) { return wLab.hasAttr(WriteAttr::WWRacy); });
	if (racyIt == g.co_end(rLab->getAddr()))
		return VerificationError::VE_OK;

	auto msg = "Unordered writes do not constitute a bug per se, though they often "
		   "indicate faulty design.\n"
		   "This warning is treated as an error due to in-place revisiting (IPR).\n"
		   "You can use -disable-ipr to disable this feature."s;
	reportError({racyIt->getPos(), VerificationError::VE_WWRace, msg, nullptr, true});
	return VerificationError::VE_WWRace;
}

bool threadReadsMaximal(const ExecutionGraph &g, int tid)
{
	/*
	 * Depending on whether this is a DSA loop or not, we have to
	 * adjust the detection starting point: DSA-blocked threads
	 * will have a SpinStart as their last event.
	 */
	BUG_ON(!llvm::isa<BlockLabel>(g.getLastThreadLabel(tid)));
	auto *lastLab = g.po_imm_pred(g.getLastThreadLabel(tid));
	auto start = llvm::isa<SpinStartLabel>(lastLab) ? lastLab->getPos().prev()
							: lastLab->getPos();
	for (auto j = start.index; j > 0; j--) {
		auto *lab = g.getEventLabel(Event(tid, j));
		BUG_ON(llvm::isa<LoopBeginLabel>(lab));
		if (llvm::isa<SpinStartLabel>(lab))
			return true;
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (rLab->getRf() != g.co_max(rLab->getAddr()))
				return false;
		}
	}
	BUG();
}

void GenMCDriver::checkLiveness()
{
	if (isHalting())
		return;

	const auto &g = getExec().getGraph();

	/* Collect all threads blocked at spinloops */
	std::vector<int> spinBlocked;
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (llvm::isa<SpinloopBlockLabel>(g.getLastThreadLabel(i)))
			spinBlocked.push_back(i);
	}

	if (spinBlocked.empty())
		return;

	/* And check whether all of them are live or not */
	auto nonTermTID = 0u;
	if (std::all_of(spinBlocked.begin(), spinBlocked.end(), [&](int tid) {
		    nonTermTID = tid;
		    return threadReadsMaximal(g, tid);
	    })) {
		/* Print some TID blocked by a spinloop */
		reportError({g.getLastThreadLabel(nonTermTID)->getPos(),
			     VerificationError::VE_Liveness,
			     "Non-terminating spinloop: thread " + std::to_string(nonTermTID)});
	}
	return;
}

void GenMCDriver::checkUnfreedMemory()
{
	if (isHalting())
		return;

	auto &g = getExec().getGraph();
	const MallocLabel *unfreedAlloc = nullptr;
	if (std::ranges::any_of(g.labels(), [&](auto &lab) {
		    unfreedAlloc = llvm::dyn_cast<MallocLabel>(&lab);
		    return unfreedAlloc && unfreedAlloc->getFree() == nullptr;
	    })) {
		reportWarningOnce(unfreedAlloc->getPos(), VerificationError::VE_UnfreedMemory);
	}
}

void GenMCDriver::filterConflictingBarriers(const ReadLabel *lab, std::vector<EventLabel *> &stores)
{
	if (getConf()->disableBAM ||
	    (!llvm::isa<BIncFaiReadLabel>(lab) && !llvm::isa<BWaitReadLabel>(lab)))
		return;

	/* barrier_wait()'s plain load should read maximally */
	if (auto *rLab = llvm::dyn_cast<BWaitReadLabel>(lab)) {
		std::swap(stores[0], stores.back());
		stores.resize(1);
		return;
	}

	/* barrier_wait()'s FAI loads should not read from conflicting stores */
	auto &g = getExec().getGraph();
	auto isReadByExclusiveRead = [&](auto *oLab) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(oLab))
			return std::ranges::any_of(wLab->readers(),
						   [&](auto &rLab) { return rLab.isRMW(); });
		if (auto *iLab = llvm::dyn_cast<InitLabel>(oLab))
			return std::ranges::any_of(iLab->rfs(lab->getAddr()),
						   [&](auto &rLab) { return rLab.isRMW(); });
		BUG();
	};
	stores.erase(std::remove_if(stores.begin(), stores.end(),
				    [&](auto &sLab) { return isReadByExclusiveRead(sLab); }),
		     stores.end());
}

void GenMCDriver::filterSymmetricStoresSR(const ReadLabel *rLab,
					  std::vector<EventLabel *> &stores) const
{
	auto &g = getExec().getGraph();
	auto t = g.getFirstThreadLabel(rLab->getThread())->getSymmPredTid();

	/* If there is no symmetric thread, exit */
	if (t == -1)
		return;

	/* Check whether the po-prefixes of the two threads match */
	if (!getSymmChecker().sharePrefixSR(t, rLab))
		return;

	/* Get the symmetric event and make sure it matches as well */
	auto *lab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(Event(t, rLab->getIndex())));
	if (!lab || lab->getAddr() != rLab->getAddr() || lab->getSize() != lab->getSize())
		return;

	if (!lab->isRMW())
		return;

	/* Remove stores that will be explored symmetrically */
	auto rfStamp = lab->getRf()->getStamp();
	stores.erase(std::remove_if(
			     stores.begin(), stores.end(),
			     [&](auto &sLab) { return lab->getRf()->getPos() == sLab->getPos(); }),
		     stores.end());
}

void GenMCDriver::filterValuesFromAnnotSAVER(const ReadLabel *rLab,
					     std::vector<EventLabel *> &validStores)
{
	/* Locks are treated as annotated CASes */
	if (!rLab->getAnnot())
		return;

	using Evaluator = SExprEvaluator<ModuleID::ID>;

	auto &g = getExec().getGraph();

	/* Ensure we keep the maximal store around even if Helper messed with it */
	BUG_ON(validStores.empty());
	auto maximal = validStores.back();
	validStores.erase(std::remove_if(validStores.begin(), validStores.end(),
					 [&](auto *wLab) {
						 auto val = wLab->getAccessValue(rLab->getAccess());
						 return wLab != maximal &&
							wLab != g.co_max(rLab->getAddr()) &&
							!Evaluator().evaluate(rLab->getAnnot(),
									      val);
					 }),
			  validStores.end());
	BUG_ON(validStores.empty());
}

void GenMCDriver::unblockWaitingHelping(const WriteLabel *lab)
{
	if (!llvm::isa<HelpedCasWriteLabel>(lab))
		return;

	/* FIXME: We have to wake up all threads waiting on helping CASes,
	 * as we don't know which ones are from the same CAS */
	for (auto i = 0u; i < getExec().getGraph().getNumThreads(); i++) {
		auto *bLab = llvm::dyn_cast<HelpedCASBlockLabel>(
			getExec().getGraph().getLastThreadLabel(i));
		if (bLab)
			getExec().getGraph().removeLast(bLab->getThread());
	}
}

bool GenMCDriver::writesBeforeHelpedContainedInView(const HelpedCasReadLabel *lab, const View &view)
{
	auto &g = getExec().getGraph();
	auto &hb = getConsChecker().getHbView(lab);

	for (auto i = 0u; i < hb.size(); i++) {
		auto j = hb.getMax(i);
		while (!llvm::isa<WriteLabel>(g.getEventLabel(Event(i, j))) && j > 0)
			--j;
		if (j > 0 && !view.contains(Event(i, j)))
			return false;
	}
	return true;
}

bool GenMCDriver::checkHelpingCasCondition(const HelpingCasLabel *hLab)
{
	auto &g = getExec().getGraph();

	auto hsView = g.labels() | std::views::filter([&g, hLab](auto &lab) {
			      auto *rLab = llvm::dyn_cast<HelpedCasReadLabel>(&lab);
			      return rLab && rLab->isRMW() && rLab->getAddr() == hLab->getAddr() &&
				     rLab->getType() == hLab->getType() &&
				     rLab->getSize() == hLab->getSize() &&
				     rLab->getOrdering() == hLab->getOrdering() &&
				     rLab->getExpected() == hLab->getExpected() &&
				     rLab->getSwapVal() == hLab->getSwapVal();
		      });

	if (std::ranges::any_of(hsView, [&g, this](auto &lab) {
		    auto *hLab = llvm::dyn_cast<HelpedCasReadLabel>(&lab);
		    auto &view = getConsChecker().getHbView(hLab);
		    return !writesBeforeHelpedContainedInView(hLab, view);
	    }))
		ERROR("Helped/Helping CAS annotation error! "
		      "Not all stores before helped-CAS are visible to helping-CAS!\n");
	return std::ranges::begin(hsView) != std::ranges::end(hsView);
}

bool GenMCDriver::checkAtomicity(const WriteLabel *wLab)
{
	if (getExec().getGraph().violatesAtomicity(wLab)) {
		moot();
		return false;
	}
	return true;
}

std::optional<EventLabel *> GenMCDriver::findConsistentRf(ReadLabel *rLab,
							  std::vector<EventLabel *> &rfs)
{
	auto &g = getExec().getGraph();

	/* For the non-bounding case, maximal extensibility guarantees consistency */
	if (!getConf()->bound.has_value()) {
		rLab->setRf(rfs.back());
		return {rfs.back()};
	}

	/* Otherwise, search for a consistent rf */
	while (!rfs.empty()) {
		rLab->setRf(rfs.back());
		if (isExecutionValid(rLab))
			return {rfs.back()};
		rfs.erase(rfs.end() - 1);
	}

	/* If none is found, tough luck */
	moot();
	return std::nullopt;
}

std::optional<EventLabel *> GenMCDriver::findConsistentCo(WriteLabel *wLab,
							  std::vector<EventLabel *> &cos)
{
	auto &g = getExec().getGraph();

	/* Similarly to the read case: rely on extensibility */
	g.addStoreToCOAfter(wLab, cos.back());
	if (!getConf()->bound.has_value())
		return {cos.back()};

	/* In contrast to the read case, we need to be a bit more careful:
	 * the consistent choice might not satisfy atomicity, but we should
	 * keep it around to try revisits */
	while (!cos.empty()) {
		g.moveStoreCOAfter(wLab, cos.back());
		if (isExecutionValid(wLab))
			return {cos.back()};
		cos.erase(cos.end() - 1);
	}
	moot();
	return std::nullopt;
}

void GenMCDriver::handleThreadKill(std::unique_ptr<ThreadKillLabel> kLab)
{
	BUG_ON(isExecutionDrivenByGraph(&*kLab));
	addLabelToGraph(std::move(kLab));
}

bool GenMCDriver::isSymmetricToSR(int candidate, Event parent, const ThreadInfo &info) const
{
	auto &g = getExec().getGraph();
	auto cParent = g.getFirstThreadLabel(candidate)->getParentCreate();
	auto &cInfo = g.getFirstThreadLabel(candidate)->getThreadInfo();

	/* A tip to print to the user in case two threads look
	 * symmetric, but we cannot deem it */
	auto tipSymmetry = [&]() {
		LOG_ONCE("possible-symmetry", VerbosityLevel::Tip)
			<< "Threads (" << getEE()->getThrById(cInfo.id) << ") and ("
			<< getEE()->getThrById(info.id)
			<< ") could benefit from symmetry reduction."
			<< " Consider using __VERIFIER_spawn_symmetric().\n";
	};

	/* First, check that the two threads are actually similar */
	if (cInfo.id == info.id || cInfo.parentId != info.parentId || cInfo.funId != info.funId ||
	    cInfo.arg != info.arg) {
		if (cInfo.funId == info.funId && cInfo.parentId == info.parentId)
			tipSymmetry();
		return false;
	}

	/* Then make sure that there is no memory access in between the spawn events */
	auto mm = std::minmax(parent.index, cParent.index);
	auto minI = mm.first;
	auto maxI = mm.second;
	for (auto j = minI; j < maxI; j++) {
		if (llvm::isa<MemAccessLabel>(g.getEventLabel(Event(parent.thread, j)))) {
			tipSymmetry();
			return false;
		}
	}
	return true;
}

int GenMCDriver::getSymmetricTidSR(const ThreadCreateLabel *tcLab,
				   const ThreadInfo &childInfo) const
{
	if (!getConf()->symmetryReduction)
		return -1;

	/* Has the user provided any info? */
	if (childInfo.symmId != -1)
		return childInfo.symmId;

	auto &g = getExec().getGraph();

	for (auto i = childInfo.id - 1; i > 0; i--)
		if (isSymmetricToSR(i, tcLab->getPos(), childInfo))
			return i;
	return -1;
}

int GenMCDriver::handleThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab)
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	if (isExecutionDrivenByGraph(&*tcLab))
		return llvm::dyn_cast<ThreadCreateLabel>(g.getEventLabel(tcLab->getPos()))
			->getChildId();

	/* First, check if the thread to be created already exists */
	int cid = 0;
	while (cid < (long)g.getNumThreads()) {
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
	auto *lab = llvm::dyn_cast<ThreadCreateLabel>(addLabelToGraph(std::move(tcLab)));

	/* Prepare the execution context for the new thread */
	EE->constructAddThreadFromInfo(lab->getChildInfo());

	/* If the thread does not exist in the graph, make an entry for it */
	if (cid == (long)g.getNumThreads()) {
		g.addNewThread();
		BUG_ON(EE->getNumThreads() != g.getNumThreads());
	} else {
		BUG_ON(g.getThreadSize(cid) != 1);
		g.removeLast(cid);
	}

	/* Create a label and add it to the graph; is the thread symmetric to another one? */
	auto symm = getSymmetricTidSR(lab, lab->getChildInfo());
	auto *tsLab = addLabelToGraph(
		ThreadStartLabel::create(Event(cid, 0), lab->getPos(), lab->getChildInfo(), symm));
	if (symm != -1)
		g.getFirstThreadLabel(symm)->setSymmSuccTid(cid);
	return cid;
}

std::optional<SVal> GenMCDriver::handleThreadJoin(std::unique_ptr<ThreadJoinLabel> lab)
{
	auto &g = getExec().getGraph();

	if (isExecutionDrivenByGraph(&*lab))
		return {g.getEventLabel(lab->getPos())->getReturnValue()};

	if (!llvm::isa<ThreadFinishLabel>(g.getLastThreadLabel(lab->getChildId()))) {
		blockThread(JoinBlockLabel::create(lab->getPos(), lab->getChildId()));
		return std::nullopt;
	}

	auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(addLabelToGraph(std::move(lab)));
	auto cid = jLab->getChildId();

	auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(g.getLastThreadLabel(cid));
	BUG_ON(!eLab);
	eLab->setParentJoin(jLab);

	if (cid < 0 || long(g.getNumThreads()) <= cid || cid == jLab->getThread()) {
		std::string err = "ERROR: Invalid TID in pthread_join(): " + std::to_string(cid);
		if (cid == jLab->getThread())
			err += " (TID cannot be the same as the calling thread)";
		reportError({jLab->getPos(), VerificationError::VE_InvalidJoin, err});
		return {SVal(0)};
	}

	if (partialExecutionExceedsBound()) {
		moot();
		return std::nullopt;
	}

	return jLab->getReturnValue();
}

void GenMCDriver::handleThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab)
{
	auto &g = getExec().getGraph();

	if (isExecutionDrivenByGraph(&*eLab))
		return;

	auto *lab = addLabelToGraph(std::move(eLab));
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *pLab = llvm::dyn_cast<JoinBlockLabel>(g.getLastThreadLabel(i));
		if (pLab && pLab->getChildId() == lab->getThread()) {
			/* If parent thread is waiting for me, relieve it */
			unblockThread(pLab->getPos());
		}
	}
	if (partialExecutionExceedsBound())
		moot();
}

void GenMCDriver::handleFence(std::unique_ptr<FenceLabel> fLab)
{
	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
}

void GenMCDriver::checkReconsiderFaiSpinloop(const MemAccessLabel *lab)
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		/* Is there any thread blocked on a potential spinloop? */
		auto *eLab = llvm::dyn_cast<FaiZNEBlockLabel>(g.getLastThreadLabel(i));
		if (!eLab)
			continue;

		/* Check whether this access affects the spinloop variable */
		auto epreds = po_preds(g, eLab);
		auto faiLabIt = std::ranges::find_if(
			epreds, [](auto &lab) { return llvm::isa<FaiWriteLabel>(&lab); });
		BUG_ON(faiLabIt == std::ranges::end(epreds));

		auto *faiLab = llvm::dyn_cast<FaiWriteLabel>(&*faiLabIt);
		if (faiLab->getAddr() != lab->getAddr())
			continue;

		/* FAIs on the same variable are OK... */
		if (llvm::isa<FaiReadLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
			continue;

		/* If it does, and also breaks the assumptions, unblock thread */
		if (!getConsChecker().getHbView(faiLab).contains(lab->getPos())) {
			auto pos = eLab->getPos();
			unblockThread(pos);
			addLabelToGraph(FaiZNESpinEndLabel::create(pos));
		}
	}
}

const VectorClock &GenMCDriver::getPrefixView(const EventLabel *lab) const
{
	// FIXME
	if (!lab->hasPrefixView())
		lab->setPrefixView(const_cast<ConsistencyChecker &>(getConsChecker())
					   .calculatePrefixView(lab));
	return lab->getPrefixView();
}

std::vector<EventLabel *> GenMCDriver::getRfsApproximation(ReadLabel *lab)
{
	auto &g = getExec().getGraph();
	auto &cc = getConsChecker();
	auto rfs = cc.getCoherentStores(lab);
	if (!llvm::isa<CasReadLabel>(lab) && !llvm::isa<FaiReadLabel>(lab))
		return rfs;

	/* Remove atomicity violations */
	auto &before = getPrefixView(lab);
	auto isSettledRMWInView = [](auto &rLab, auto &before) {
		auto &g = *rLab.getParent();
		return rLab.isRMW() &&
		       ((!rLab.isRevisitable() && !llvm::dyn_cast<WriteLabel>(g.po_imm_succ(&rLab))
							   ->hasAttr(WriteAttr::RevBlocker)) ||
			before.contains(rLab.getPos()));
	};
	auto storeReadBySettledRMWInView = [&isSettledRMWInView](auto *sLab, auto &before,
								 SAddr addr) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(sLab)) {
			return std::ranges::any_of(wLab->readers(), [&](auto &rLab) {
				return isSettledRMWInView(rLab, before);
			});
		};

		auto *iLab = llvm::dyn_cast<InitLabel>(sLab);
		BUG_ON(!iLab);
		return std::ranges::any_of(iLab->rfs(addr), [&](auto &rLab) {
			return isSettledRMWInView(rLab, before);
		});
	};
	rfs.erase(std::remove_if(rfs.begin(), rfs.end(),
				 [&](auto &sLab) {
					 auto oldVal = sLab->getAccessValue(lab->getAccess());
					 return lab->valueMakesRMWSucceed(oldVal) &&
						storeReadBySettledRMWInView(sLab, before,
									    lab->getAddr());
				 }),
		  rfs.end());
	return rfs;
}

void GenMCDriver::filterOptimizeRfs(const ReadLabel *lab, std::vector<EventLabel *> &stores)
{
	/* Symmetry reduction */
	if (getConf()->symmetryReduction)
		filterSymmetricStoresSR(lab, stores);

	/* BAM */
	if (!getConf()->disableBAM)
		filterConflictingBarriers(lab, stores);

	/* Keep values that do not lead to blocking */
	filterValuesFromAnnotSAVER(lab, stores);
}

void GenMCDriver::filterAtomicityViolations(const ReadLabel *rLab,
					    std::vector<EventLabel *> &stores)
{
	auto &g = getExec().getGraph();
	if (!llvm::isa<CasReadLabel>(rLab) && !llvm::isa<FaiReadLabel>(rLab))
		return;

	const auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab);
	auto valueMakesSuccessfulRMW = [&casLab, rLab](auto &&val) {
		return !casLab || val == casLab->getExpected();
	};
	stores.erase(
		std::remove_if(
			stores.begin(), stores.end(),
			[&](auto *sLab) {
				if (auto *iLab = llvm::dyn_cast<InitLabel>(sLab))
					return std::any_of(
						iLab->rf_begin(rLab->getAddr()),
						iLab->rf_end(rLab->getAddr()), [&](auto &rLab) {
							return rLab.isRMW() &&
							       valueMakesSuccessfulRMW(
								       rLab.getAccessValue(
									       rLab.getAccess()));
						});
				return std::any_of(
					rf_succ_begin(g, sLab), rf_succ_end(g, sLab),
					[&](auto &rLab) {
						return rLab.isRMW() &&
						       valueMakesSuccessfulRMW(rLab.getAccessValue(
							       rLab.getAccess()));
					});
			}),
		stores.end());
}

EventLabel *GenMCDriver::pickRandomRf(ReadLabel *rLab, std::vector<EventLabel *> &stores)
{
	auto &g = getExec().getGraph();

	stores.erase(std::remove_if(stores.begin(), stores.end(),
				    [&](auto &sLab) {
					    rLab->setRf(sLab);
					    return !isExecutionValid(rLab);
				    }),
		     stores.end());

	MyDist dist(0, stores.size() - 1);
	auto random = dist(estRng);
	rLab->setRf(stores[random]);
	return stores[random];
}

std::optional<SVal> GenMCDriver::handleLoad(std::unique_ptr<ReadLabel> rLab)
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();

	if (inRecoveryMode() && rLab->getAddr().isVolatile())
		return {getRecReadRetValue(rLab.get())};

	if (isExecutionDrivenByGraph(&*rLab))
		return getReadRetValue(llvm::dyn_cast<ReadLabel>(g.getEventLabel(rLab->getPos())));

	if (!rLab->getAnnot())
		rLab->setAnnot(EE->getCurrentAnnotConcretized());
	auto *lab = llvm::dyn_cast<ReadLabel>(addLabelToGraph(std::move(rLab)));

	if (checkAccessValidity(lab) != VerificationError::VE_OK ||
	    checkForRaces(lab) != VerificationError::VE_OK ||
	    checkIPRValidity(lab) != VerificationError::VE_OK)
		return std::nullopt; /* This execution will be blocked */

	/* Check whether the load forces us to reconsider some existing event */
	checkReconsiderFaiSpinloop(lab);

	/* If a CAS read cannot be added maximally, reschedule */
	if (!isRescheduledRead(lab->getPos()) &&
	    removeCASReadIfBlocks(lab, g.co_max(lab->getAddr())))
		return std::nullopt;
	if (isRescheduledRead(lab->getPos()))
		setRescheduledRead(Event::getInit());

	/* Get an approximation of the stores we can read from */
	auto stores = getRfsApproximation(lab);
	BUG_ON(stores.empty());
	GENMC_DEBUG(LOG(VerbosityLevel::Debug3) << "Rfs: " << format(stores) << "\n";);
	filterOptimizeRfs(lab, stores);
	GENMC_DEBUG(LOG(VerbosityLevel::Debug3) << "Rfs (optimized): " << format(stores) << "\n";);

	std::optional<EventLabel *> rf = std::nullopt;
	if (inEstimationMode()) {
		getExec().getChoiceMap().update(lab, stores);
		filterAtomicityViolations(lab, stores);
		rf = pickRandomRf(lab, stores);
	} else {
		rf = findConsistentRf(lab, stores);
		/* Push all the other alternatives choices to the Stack (many maximals for wb) */
		for (const auto &sLab : stores | std::views::take(stores.size() - 1)) {
			auto status = false; /* MO messes with the status */
			getExec().getWorkqueue().add(std::make_unique<ReadForwardRevisit>(
				lab->getPos(), sLab->getPos(), status));
		}
	}

	/* Ensured the selected rf comes from an initialized memory location */
	if (!rf.has_value() || checkInitializedMem(lab) != VerificationError::VE_OK)
		return std::nullopt;

	GENMC_DEBUG(LOG(VerbosityLevel::Debug2) << "--- Added load " << lab->getPos() << "\n"
						<< getExec().getGraph(););

	/* If this is the last part of barrier_wait() check whether we should block */
	auto retVal = (*rf)->getAccessValue(lab->getAccess());
	if (llvm::isa<BWaitReadLabel>(lab) && retVal != findBarrierInitValue(g, lab->getAccess())) {
		blockThread(BarrierBlockLabel::create(lab->getPos().next()));
	}
	return {retVal};
}

void GenMCDriver::annotateStoreHELPER(WriteLabel *wLab)
{
	auto &g = getExec().getGraph();

	/* Don't bother with lock ops */
	if (!getConf()->helper || !wLab->isRMW() || llvm::isa<LockCasWriteLabel>(wLab) ||
	    llvm::isa<TrylockCasWriteLabel>(wLab))
		return;

	/* Check whether we can mark it as RevBlocker */
	auto *pLab = g.po_imm_pred(wLab);
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
}

std::vector<ReadLabel *> GenMCDriver::getRevisitableApproximation(WriteLabel *sLab)
{
	auto &g = getExec().getGraph();
	auto &prefix = getPrefixView(sLab);
	auto loads = getConsChecker().getCoherentRevisits(sLab, prefix);
	std::sort(loads.begin(), loads.end(),
		  [&g](auto &lab1, auto &lab2) { return lab1->getStamp() > lab2->getStamp(); });
	return loads;
}

void GenMCDriver::pickRandomCo(WriteLabel *sLab, std::vector<EventLabel *> &cos)
{
	auto &g = getExec().getGraph();

	g.addStoreToCOAfter(sLab, cos.back());
	cos.erase(std::remove_if(cos.begin(), cos.end(),
				 [&](auto &wLab) {
					 g.moveStoreCOAfter(sLab, wLab);
					 return !isExecutionValid(sLab);
				 }),
		  cos.end());

	/* Extensibility is not guaranteed if an RMW read is not reading maximally
	 * (during estimation, reads read from arbitrary places anyway).
	 * If that is the case, we have to ensure that estimation won't stop. */
	if (cos.empty()) {
		moot();
		getExec().getWorkqueue().add(std::make_unique<RerunForwardRevisit>());
		return;
	}

	MyDist dist(0, cos.size() - 1);
	auto random = dist(estRng);
	g.moveStoreCOAfter(sLab, cos[random]);
}

void GenMCDriver::calcCoOrderings(WriteLabel *lab, const std::vector<EventLabel *> &cos)
{
	for (auto &predLab : cos | std::views::take(cos.size() - 1)) {
		getExec().getWorkqueue().add(
			std::make_unique<WriteForwardRevisit>(lab->getPos(), predLab->getPos()));
	}
}

void GenMCDriver::handleStore(std::unique_ptr<WriteLabel> wLab)
{
	if (isExecutionDrivenByGraph(&*wLab))
		return;

	auto &g = getExec().getGraph();

	if (getConf()->helper && wLab->isRMW())
		annotateStoreHELPER(&*wLab);
	if (llvm::isa<BIncFaiWriteLabel>(&*wLab) && wLab->getVal() == SVal(0))
		wLab->setVal(findBarrierInitValue(g, wLab->getAccess()));

	auto *lab = llvm::dyn_cast<WriteLabel>(addLabelToGraph(std::move(wLab)));

	if (checkAccessValidity(lab) != VerificationError::VE_OK ||
	    checkInitializedMem(lab) != VerificationError::VE_OK ||
	    checkFinalAnnotations(lab) != VerificationError::VE_OK ||
	    checkForRaces(lab) != VerificationError::VE_OK)
		return;

	checkReconsiderFaiSpinloop(lab);
	unblockWaitingHelping(lab);
	checkReconsiderReadOpts(lab);

	/* Find all possible placings in coherence for this store, and
	 * print a WW-race warning if appropriate (if this moots,
	 * exploration will anyway be cut) */
	auto cos = getConsChecker().getCoherentPlacings(lab);
	if (cos.size() > 1) {
		reportWarningOnce(lab->getPos(), VerificationError::VE_WWRace, cos[0]);
	}

	std::optional<EventLabel *> co;
	if (inEstimationMode()) {
		pickRandomCo(lab, cos);
		getExec().getChoiceMap().update(lab, cos);
	} else {
		co = findConsistentCo(lab, cos);
		calcCoOrderings(lab, cos);
	}

	GENMC_DEBUG(LOG(VerbosityLevel::Debug2) << "--- Added store " << lab->getPos() << "\n"
						<< getExec().getGraph(););

	if (inRecoveryMode() || inReplay())
		return;

	calcRevisits(lab);
}

SVal GenMCDriver::handleMalloc(std::unique_ptr<MallocLabel> aLab)
{
	auto &g = getExec().getGraph();

	if (isExecutionDrivenByGraph(&*aLab)) {
		auto *lab = llvm::dyn_cast<MallocLabel>(g.getEventLabel(aLab->getPos()));
		BUG_ON(!lab);
		return SVal(lab->getAllocAddr().get());
	}

	/* Fix and add label to the graph. Cached labels might already have an address,
	 * but we enforce that's the same with the new one dispensed (for non-dep-tracking) */
	auto oldAddr = aLab->getAllocAddr();
	BUG_ON(!getConf()->isDepTrackingModel && oldAddr != SAddr() &&
	       oldAddr != aLab->getAllocAddr());
	if (oldAddr == SAddr())
		aLab->setAllocAddr(getFreshAddr(&*aLab));
	auto *lab = llvm::dyn_cast<MallocLabel>(addLabelToGraph(std::move(aLab)));
	return SVal(lab->getAllocAddr().get());
}

void GenMCDriver::handleFree(std::unique_ptr<FreeLabel> dLab)
{
	auto &g = getExec().getGraph();

	if (isExecutionDrivenByGraph(&*dLab))
		return;

	/* Find the size of the area deallocated */
	auto size = 0u;
	auto alloc = findAllocatingLabel(g, dLab->getFreedAddr());
	if (alloc) {
		size = alloc->getAllocSize();
	}

	/* Add a label with the appropriate store */
	dLab->setFreedSize(size);
	dLab->setAlloc(alloc);
	auto *lab = addLabelToGraph(std::move(dLab));
	alloc->setFree(llvm::dyn_cast<FreeLabel>(lab));

	/* Check whether there is any memory race */
	checkForRaces(lab);
}

const MemAccessLabel *GenMCDriver::getPreviousVisibleAccessLabel(Event start) const
{
	auto &g = getExec().getGraph();
	std::vector<Event> finalReads;

	for (auto pos = start.prev(); pos.index > 0; --pos) {
		auto *lab = g.getEventLabel(pos);
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (getConf()->helper && rLab->isConfirming())
				continue;
			if (rLab->getRf()) {
				auto *wLab = llvm::dyn_cast<WriteLabel>(rLab->getRf());
				if (wLab && wLab->isLocal())
					continue;
				if (wLab && wLab->isFinal()) {
					finalReads.push_back(rLab->getPos());
					continue;
				}
				if (std::any_of(finalReads.begin(), finalReads.end(),
						[&](const Event &l) {
							auto *lLab = llvm::dyn_cast<ReadLabel>(
								g.getEventLabel(l));
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
	auto &g = getExec().getGraph();

	auto *lab = getPreviousVisibleAccessLabel(pos);
	if (auto *rLab = llvm::dyn_cast_or_null<ReadLabel>(lab))
		if (!rLab->isRevisitable() || !rLab->wasAddedMax())
			moot();
	return;
}

void GenMCDriver::handleBlock(std::unique_ptr<BlockLabel> lab)
{
	if (isExecutionDrivenByGraph(&*lab))
		return;

	/* Call addLabelToGraph first to cache the label */
	addLabelToGraph(lab->clone());
	blockThreadTryMoot(std::move(lab));
}

std::unique_ptr<VectorClock> GenMCDriver::getReplayView() const
{
	auto &g = getExec().getGraph();
	auto v = g.getViewFromStamp(g.getMaxStamp());

	/* handleBlock() is usually only called during normal execution
	 * and hence not reproduced during replays.
	 * We have to remove BlockLabels so that these will not lead
	 * to the execution of extraneous instructions */
	for (auto i = 0u; i < g.getNumThreads(); i++)
		if (llvm::isa<BlockLabel>(g.getLastThreadLabel(i)))
			v->setMax(Event(i, v->getMax(i) - 1));
	return v;
}

void GenMCDriver::reportError(const ErrorDetails &details)
{
	auto &g = getExec().getGraph();

	/* If we have already detected an error, no need to report another */
	if (isHalting())
		return;

	/* If we this is a replay (might happen if one LLVM instruction
	 * maps to many MC events), do not get into an infinite loop... */
	if (inReplay())
		return;

	/* Ignore soft errors under estimation mode.
	 * These are going to be reported later on anyway */
	if (!details.shouldHalt && inEstimationMode())
		return;

	/* If this is an invalid access, change the RF of the offending
	 * event to BOTTOM, so that we do not try to get its value.
	 * Don't bother updating the views */
	auto *errLab = g.getEventLabel(details.pos);
	if (isInvalidAccessError(details.type) && llvm::isa<ReadLabel>(errLab))
		llvm::dyn_cast<ReadLabel>(errLab)->setRf(nullptr);

	/* Print a basic error message and the graph.
	 * We have to save the interpreter state as replaying will
	 * destroy the current execution stack */
	auto iState = getEE()->saveState();

	getEE()->replayExecutionBefore(*getReplayView());

	llvm::raw_string_ostream out(result.message);

	out << (isHardError(details.type) ? "Error: " : "Warning: ") << details.type << "!\n";
	out << "Event " << errLab->getPos() << " ";
	if (details.racyLab != nullptr)
		out << "conflicts with event " << details.racyLab->getPos() << " ";
	out << "in graph:\n";
	printGraph(true, out);

	/* Print error trace leading up to the violating event(s) */
	if (getConf()->printErrorTrace) {
		printTraceBefore(errLab, out);
		if (details.racyLab != nullptr)
			printTraceBefore(details.racyLab, out);
	}

	/* Print the specific error message */
	if (!details.msg.empty())
		out << details.msg << "\n";

	/* Dump the graph into a file (DOT format) */
	if (!getConf()->dotFile.empty())
		dotPrintToFile(getConf()->dotFile, errLab, details.racyLab);

	getEE()->restoreState(std::move(iState));

	if (details.shouldHalt)
		halt(details.type);
}

bool GenMCDriver::reportWarningOnce(Event pos, VerificationError wcode,
				    const EventLabel *racyLab /* = nullptr */)
{
	/* Helper function to determine whether the warning should be treated as an error */
	auto shouldUpgradeWarning = [&](auto &wcode) {
		if (wcode != VerificationError::VE_WWRace)
			return std::make_pair(false, ""s);
		if (!getConf()->symmetryReduction && !getConf()->ipr)
			return std::make_pair(false, ""s);

		auto &g = getExec().getGraph();
		auto *lab = g.getEventLabel(pos);
		auto upgrade =
			(getConf()->symmetryReduction &&
			 std::ranges::any_of(
				 g.thr_ids(),
				 [&](auto tid) {
					 return g.getFirstThreadLabel(tid)->getSymmPredTid() != -1;
				 })) ||
			(getConf()->ipr &&
			 std::any_of(sameloc_begin(g, lab), sameloc_end(g, lab), [&](auto &oLab) {
				 auto *rLab = llvm::dyn_cast<ReadLabel>(&oLab);
				 return rLab && rLab->getAnnot();
			 }));
		auto [cause, cli] =
			getConf()->ipr
				? std::make_pair("in-place revisiting (IPR)"s, "-disable-ipr"s)
				: std::make_pair("symmetry reduction (SR)"s, "-disable-sr"s);
		auto msg = "Unordered writes do not constitute a bug per se, though they often "
			   "indicate faulty design.\n" +
			   (upgrade ? ("This warning is treated as an error due to " + cause +
				       ".\n"
				       "You can use " +
				       cli + " to disable these features."s)
				    : ""s);
		return std::make_pair(upgrade, msg);
	};

	/* If the warning has been seen before, only report it if it's an error */
	auto [upgradeWarning, msg] = shouldUpgradeWarning(wcode);
	auto &knownWarnings = getResult().warnings;
	if (upgradeWarning || knownWarnings.count(wcode) == 0) {
		reportError({pos, wcode, msg, racyLab, upgradeWarning});
	}
	if (knownWarnings.count(wcode) == 0)
		knownWarnings.insert(wcode);
	if (wcode == VerificationError::VE_WWRace)
		getExec().getGraph().getWriteLabel(pos)->setAttr(WriteAttr::WWRacy);
	return upgradeWarning;
}

bool GenMCDriver::tryOptimizeBarrierRevisits(BIncFaiWriteLabel *sLab,
					     std::vector<ReadLabel *> &loads)
{
	if (getConf()->disableBAM)
		return false;

	/* If the barrier_wait() does not write the initial value, nothing to do */
	auto iVal = findBarrierInitValue(getExec().getGraph(), sLab->getAccess());
	if (sLab->getVal() != iVal)
		return true;

	/* Otherwise, revisit in place */
	auto &g = getExec().getGraph();
	auto bsView = g.labels() | std::views::filter([&g, sLab](auto &lab) {
			      if (!llvm::isa<BarrierBlockLabel>(&lab))
				      return false;
			      auto *pLab = llvm::dyn_cast<BIncFaiWriteLabel>(
				      g.po_imm_pred(g.po_imm_pred(&lab)));
			      return pLab->getAddr() == sLab->getAddr();
		      }) |
		      std::views::transform([](auto &lab) { return lab.getPos(); });
	std::vector<Event> bs(std::ranges::begin(bsView), std::ranges::end(bsView));
	auto unblockedLoads = std::count_if(loads.begin(), loads.end(), [&](auto *rLab) {
		auto *nLab = llvm::dyn_cast_or_null<BlockLabel>(g.po_imm_succ(rLab));
		return !nLab;
	});
	if (bs.size() > iVal.get() || unblockedLoads > 0)
		WARN_ONCE("bam-well-formed", "Execution not barrier-well-formed!\n");

	for (auto &b : bs) {
		auto *pLab = llvm::dyn_cast<BIncFaiWriteLabel>(
			g.po_imm_pred(g.po_imm_pred(g.getEventLabel(b))));
		BUG_ON(!pLab);
		unblockThread(b);
		g.removeLast(b.thread);
		auto *rLab = llvm::dyn_cast<ReadLabel>(addLabelToGraph(
			BWaitReadLabel::create(b.prev(), pLab->getOrdering(), pLab->getAddr(),
					       pLab->getSize(), pLab->getType(), pLab->getDeps())));
		rLab->setRf(sLab);
		rLab->setAddedMax(rLab->getRf() == g.co_max(rLab->getAddr()));
	}
	return true;
}

void GenMCDriver::tryOptimizeIPRs(const WriteLabel *sLab, std::vector<ReadLabel *> &loads)
{
	if (!getConf()->ipr)
		return;

	auto &g = getExec().getGraph();

	std::vector<ReadLabel *> toIPR;
	loads.erase(std::remove_if(loads.begin(), loads.end(),
				   [&](auto *rLab) {
					   /* Treatment of blocked CASes is different */
					   auto blocked =
						   !llvm::isa<CasReadLabel>(rLab) &&
						   rLab->getAnnot() &&
						   !rLab->valueMakesAssumeSucceed(
							   rLab->getAccessValue(rLab->getAccess()));
					   if (blocked)
						   toIPR.push_back(rLab);
					   return blocked;
				   }),
		    loads.end());

	for (auto *rLab : toIPR)
		revisitInPlace(*constructBackwardRevisit(rLab, sLab));

	/* We also have to filter out some regular revisits */
	auto pending = g.getPendingRMW(sLab);
	if (!pending.isInitializer()) {
		loads.erase(std::remove_if(loads.begin(), loads.end(),
					   [&](auto *rLab) {
						   auto *rfLab = rLab->getRf();
						   return rLab->getAnnot() && // must be like that
							  rfLab->getStamp() > rLab->getStamp() &&
							  !getPrefixView(sLab).contains(
								  rfLab->getPos());
					   }),
			    loads.end());
	}
}

bool GenMCDriver::removeCASReadIfBlocks(const ReadLabel *rLab, const EventLabel *sLab)
{
	/* This only affects annotated CASes */
	if (!rLab->getAnnot() || !llvm::isa<CasReadLabel>(rLab) ||
	    (!getConf()->ipr && !llvm::isa<LockCasReadLabel>(rLab)))
		return false;
	/* Skip if bounding is enabled or the access is uninitialized */
	if (isUninitializedAccess(rLab->getAddr(), sLab->getPos()) || getConf()->bound.has_value())
		return false;

	/* If the CAS blocks, block thread altogether */
	auto val = sLab->getAccessValue(rLab->getAccess());
	if (rLab->valueMakesAssumeSucceed(val))
		return false;

	blockThread(ReadOptBlockLabel::create(rLab->getPos(), rLab->getAddr()));
	return true;
}

void GenMCDriver::checkReconsiderReadOpts(const WriteLabel *sLab)
{
	auto &g = getExec().getGraph();
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *bLab = llvm::dyn_cast<ReadOptBlockLabel>(g.getLastThreadLabel(i));
		if (!bLab || bLab->getAddr() != sLab->getAddr())
			continue;
		unblockThread(bLab->getPos());
	}
}

void GenMCDriver::optimizeUnconfirmedRevisits(const WriteLabel *sLab,
					      std::vector<ReadLabel *> &loads)
{
	if (!getConf()->helper)
		return;

	auto &g = getExec().getGraph();

	/* If there is already a write with the same value, report a possible ABA */
	auto valid = std::count_if(
		g.co_begin(sLab->getAddr()), g.co_end(sLab->getAddr()), [&](auto &wLab) {
			return wLab.getPos() != sLab->getPos() && wLab.getVal() == sLab->getVal();
		});
	if (sLab->getAddr().isStatic() &&
	    g.getInitLabel()->getAccessValue(sLab->getAccess()) == sLab->getVal())
		++valid;
	WARN_ON_ONCE(valid > 0, "helper-aba-found",
		     "Possible ABA pattern! Consider running without -helper.\n");

	/* Do not bother with revisits that will be unconfirmed/lead to ABAs */
	loads.erase(std::remove_if(loads.begin(), loads.end(),
				   [&](auto *lab) {
					   if (!lab->isConfirming())
						   return false;

					   const EventLabel *scLab = nullptr;
					   auto *pLab = findMatchingSpeculativeRead(lab, scLab);
					   ERROR_ON(!pLab, "Confirming CAS annotation error! "
							   "Does a speculative read precede the "
							   "confirming operation?\n");

					   return !scLab;
				   }),
		    loads.end());
}

bool GenMCDriver::isConflictingNonRevBlocker(const EventLabel *pLab, const WriteLabel *sLab,
					     const Event &s)
{
	auto &g = getExec().getGraph();
	auto *sLab2 = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
	if (sLab2->getPos() == sLab->getPos() || !sLab2->isRMW())
		return false;
	auto &prefix = getPrefixView(sLab);
	if (prefix.contains(sLab2->getPos()) && !(pLab && pLab->getStamp() < sLab2->getStamp()))
		return false;
	if (sLab2->getThread() <= sLab->getThread())
		return false;
	return std::any_of(sLab2->readers_begin(), sLab2->readers_end(), [&](auto &rLab) {
		return rLab.getStamp() < sLab2->getStamp() && !prefix.contains(rLab.getPos());
	});
}

bool GenMCDriver::tryOptimizeRevBlockerAddition(const WriteLabel *sLab,
						std::vector<ReadLabel *> &loads)
{
	if (!sLab->hasAttr(WriteAttr::RevBlocker))
		return false;

	auto &g = getExec().getGraph();
	auto *pLab = getPreviousVisibleAccessLabel(sLab->getPos().prev());
	if (std::find_if(g.co_begin(sLab->getAddr()), g.co_end(sLab->getAddr()),
			 [this, pLab, sLab](auto &lab) {
				 return isConflictingNonRevBlocker(pLab, sLab, lab.getPos());
			 }) != g.co_end(sLab->getAddr())) {
		moot();
		loads.clear();
		return true;
	}
	return false;
}

bool GenMCDriver::tryOptimizeRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &loads)
{
	auto &g = getExec().getGraph();

	/* BAM */
	if (!getConf()->disableBAM) {
		if (auto *faiLab = llvm::dyn_cast<BIncFaiWriteLabel>(sLab)) {
			if (tryOptimizeBarrierRevisits(faiLab, loads))
				return true;
		}
	}

	/* IPR + locks */
	tryOptimizeIPRs(sLab, loads);

	/* Helper: 1) Do not bother with revisits that will lead to unconfirmed reads
		   2) Do not bother exploring if a RevBlocker is being re-added	*/
	if (getConf()->helper) {
		optimizeUnconfirmedRevisits(sLab, loads);
		if (sLab->hasAttr(WriteAttr::RevBlocker) &&
		    tryOptimizeRevBlockerAddition(sLab, loads))
			return true;
	}
	return false;
}

void GenMCDriver::revisitInPlace(const BackwardRevisit &br)
{
	BUG_ON(getConf()->bound.has_value());

	auto &g = getExec().getGraph();
	auto *rLab = g.getReadLabel(br.getPos());
	auto *sLab = g.getWriteLabel(br.getRev());

	BUG_ON(!llvm::isa<ReadLabel>(rLab));
	if (g.po_imm_succ(rLab))
		g.removeLast(rLab->getThread());
	rLab->setRf(sLab);
	rLab->setAddedMax(true); // always true for atomicity violations
	rLab->setIPRStatus(true);

	completeRevisitedRMW(rLab);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug1) << "--- In-place revisiting " << rLab->getPos()
						<< " <-- " << sLab->getPos() << "\n"
						<< getExec().getGraph(););

	EE->resetThread(rLab->getThread());
	EE->getThrById(rLab->getThread()).ECStack = EE->getThrById(rLab->getThread()).initEC;
	threadPrios = {rLab->getPos()};
}

void updatePredsWithPrefixView(const ExecutionGraph &g, VectorClock &preds,
			       const VectorClock &pporf)
{
	/* In addition to taking (preds U pporf), make sure pporf includes rfis */
	preds.update(pporf);

	if (!dynamic_cast<const DepExecutionGraph *>(&g))
		return;
	auto &predsD = *llvm::dyn_cast<DepView>(&preds);
	for (auto i = 0u; i < pporf.size(); i++) {
		for (auto j = 1; j <= pporf.getMax(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (preds.contains(rLab->getPos()) &&
				    !preds.contains(rLab->getRf())) {
					if (rLab->getRf()->getThread() == rLab->getThread())
						predsD.removeHole(rLab->getRf()->getPos());
				}
			}
			auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
			if (wLab && wLab->isRMW() && pporf.contains(lab->getPos().prev()))
				predsD.removeHole(lab->getPos());
		}
	}
	return;
}

std::unique_ptr<VectorClock>
GenMCDriver::getRevisitView(const ReadLabel *rLab, const WriteLabel *sLab,
			    const WriteLabel *midLab /* = nullptr */) const
{
	auto &g = getExec().getGraph();
	auto preds = g.getPredsView(rLab->getPos());

	updatePredsWithPrefixView(g, *preds, getPrefixView(sLab));
	if (midLab)
		updatePredsWithPrefixView(g, *preds, getPrefixView(midLab));
	return std::move(preds);
}

std::unique_ptr<BackwardRevisit> GenMCDriver::constructBackwardRevisit(const ReadLabel *rLab,
								       const WriteLabel *sLab) const
{
	if (!getConf()->helper)
		return std::make_unique<BackwardRevisit>(rLab, sLab, getRevisitView(rLab, sLab));

	auto &g = getExec().getGraph();

	/* Check whether there is a conflicting RevBlocker */
	auto pending = g.getPendingRMW(sLab);
	auto *pLab = llvm::dyn_cast_or_null<WriteLabel>(g.po_imm_succ(g.getEventLabel(pending)));
	pending = (!pending.isInitializer() && pLab->hasAttr(WriteAttr::RevBlocker))
			  ? pending.next()
			  : Event::getInit();

	/* If there is, do an optimized backward revisit */
	auto &prefix = getPrefixView(sLab);
	if (!pending.isInitializer() &&
	    !getPrefixView(g.getEventLabel(pending)).contains(rLab->getPos()) &&
	    rLab->getStamp() < g.getEventLabel(pending)->getStamp() && !prefix.contains(pending))
		return std::make_unique<BackwardRevisitHELPER>(
			rLab->getPos(), sLab->getPos(),
			getRevisitView(rLab, sLab, g.getWriteLabel(pending)), pending);
	return std::make_unique<BackwardRevisit>(rLab, sLab, getRevisitView(rLab, sLab));
}

bool isFixedHoleInView(const ExecutionGraph &g, const EventLabel *lab, const DepView &v)
{
	if (auto *wLabB = llvm::dyn_cast<WriteLabel>(lab))
		return std::any_of(wLabB->readers_begin(), wLabB->readers_end(),
				   [&v](auto &oLab) { return v.contains(oLab.getPos()); });

	auto *rLabB = llvm::dyn_cast<ReadLabel>(lab);
	if (!rLabB)
		return false;

	/* If prefix has same address load, we must read from the same write */
	for (auto i = 0u; i < v.size(); i++) {
		for (auto j = 0u; j <= v.getMax(i); j++) {
			if (!v.contains(Event(i, j)))
				continue;
			if (auto *mLab = g.getReadLabel(Event(i, j)))
				if (mLab->getAddr() == rLabB->getAddr() &&
				    mLab->getRf() == rLabB->getRf())
					return true;
		}
	}

	if (rLabB->isRMW()) {
		auto *wLabB = g.getWriteLabel(rLabB->getPos().next());
		return std::any_of(wLabB->readers_begin(), wLabB->readers_end(),
				   [&v](auto &oLab) { return v.contains(oLab.getPos()); });
	}
	return false;
}

bool GenMCDriver::prefixContainsSameLoc(const BackwardRevisit &r, const EventLabel *lab) const
{
	if (!getConf()->isDepTrackingModel)
		return false;

	/* Some holes need to be treated specially. However, it is _wrong_ to keep
	 * porf views around. What we should do instead is simply check whether
	 * an event is "part" of WLAB's pporf view (even if it is not contained in it).
	 * Similar actions are taken in {WB,MO}Calculator */
	auto &g = getExec().getGraph();
	auto &v = *llvm::dyn_cast<DepView>(&getPrefixView(g.getEventLabel(r.getRev())));
	if (lab->getIndex() <= v.getMax(lab->getThread()) && isFixedHoleInView(g, lab, v))
		return true;
	if (auto *br = llvm::dyn_cast<BackwardRevisitHELPER>(&r)) {
		auto &hv = *llvm::dyn_cast<DepView>(&getPrefixView(g.getEventLabel(br->getMid())));
		return lab->getIndex() <= hv.getMax(lab->getThread()) &&
		       isFixedHoleInView(g, lab, hv);
	}
	return false;
}

bool GenMCDriver::hasBeenRevisitedByDeleted(const BackwardRevisit &r, const EventLabel *eLab)
{
	auto *lab = llvm::dyn_cast<ReadLabel>(eLab);
	if (!lab || lab->isIPR())
		return false;

	auto *rfLab = lab->getRf();
	auto &v = *r.getViewNoRel();
	return !v.contains(rfLab->getPos()) && rfLab->getStamp() > lab->getStamp() &&
	       !prefixContainsSameLoc(r, rfLab);
}

bool GenMCDriver::isCoBeforeSavedPrefix(const BackwardRevisit &r, const EventLabel *lab)
{
	auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab);
	if (!mLab)
		return false;

	auto &g = getExec().getGraph();
	auto &v = r.getViewNoRel();
	auto w = llvm::isa<ReadLabel>(mLab) ? llvm::dyn_cast<ReadLabel>(mLab)->getRf()->getPos()
					    : mLab->getPos();
	auto succIt = g.getWriteLabel(w) ? g.co_succ_begin(g.getWriteLabel(w))
					 : g.co_begin(mLab->getAddr());
	auto succE = g.getWriteLabel(w) ? g.co_succ_end(g.getWriteLabel(w))
					: g.co_end(mLab->getAddr());
	return any_of(succIt, succE, [&](auto &sLab) {
		return v->contains(sLab.getPos()) &&
		       (!getConf()->isDepTrackingModel ||
			mLab->getIndex() > getPrefixView(&sLab).getMax(mLab->getThread())) &&
		       sLab.getPos() != r.getRev();
	});
}

bool GenMCDriver::coherenceSuccRemainInGraph(const BackwardRevisit &r)
{
	auto &g = getExec().getGraph();
	auto *wLab = g.getWriteLabel(r.getRev());
	if (wLab->isRMW())
		return true;

	auto succIt = g.co_succ_begin(wLab);
	auto succE = g.co_succ_end(wLab);
	if (succIt == succE)
		return true;

	return r.getViewNoRel()->contains(succIt->getPos());
}

bool wasAddedMaximally(const EventLabel *lab)
{
	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab))
		return mLab->wasAddedMax();
	if (auto *oLab = llvm::dyn_cast<OptionalLabel>(lab))
		return !oLab->isExpanded();
	return true;
}

bool GenMCDriver::isMaximalExtension(const BackwardRevisit &r)
{
	if (!coherenceSuccRemainInGraph(r))
		return false;

	auto &g = getExec().getGraph();
	auto &v = r.getViewNoRel();

	for (const auto &lab : g.labels()) {
		if ((lab.getPos() != r.getPos() && v->contains(lab.getPos())) ||
		    prefixContainsSameLoc(r, &lab))
			continue;

		if (!wasAddedMaximally(&lab))
			return false;
		if (isCoBeforeSavedPrefix(r, &lab))
			return false;
		if (hasBeenRevisitedByDeleted(r, &lab))
			return false;
	}
	return true;
}

bool GenMCDriver::revisitModifiesGraph(const BackwardRevisit &r) const
{
	auto &g = getExec().getGraph();
	auto &v = r.getViewNoRel();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (v->getMax(i) + 1 != (long)g.getThreadSize(i) &&
		    !llvm::isa<TerminatorLabel>(g.getEventLabel(Event(i, v->getMax(i) + 1))))
			return true;
		if (!getConf()->isDepTrackingModel)
			continue;
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (!v->contains(lab->getPos()) && !llvm::isa<EmptyLabel>(lab) &&
			    !llvm::isa<TerminatorLabel>(lab))
				return true;
		}
	}
	return false;
}

std::unique_ptr<ExecutionGraph> GenMCDriver::copyGraph(const BackwardRevisit *br,
						       VectorClock *v) const
{
	auto &g = getExec().getGraph();

	/* Adjust the view that will be used for copying */
	auto &prefix = getPrefixView(g.getEventLabel(br->getRev()));
	if (auto *brh = llvm::dyn_cast<BackwardRevisitHELPER>(br)) {
		if (auto *dv = llvm::dyn_cast<DepView>(v)) {
			dv->addHole(brh->getMid());
			dv->addHole(brh->getMid().prev());
		} else {
			auto prev = v->getMax(brh->getMid().thread);
			v->setMax(Event(brh->getMid().thread, prev - 2));
		}
	}

	auto og = g.getCopyUpTo(*v);

	/* Ensure the prefix of the write will not be revisitable */
	auto *revLab = og->getReadLabel(br->getPos());

	for (auto &lab : og->labels()) {
		if (prefix.contains(lab.getPos()))
			lab.setRevisitStatus(false);
	}
	return og;
}

bool GenMCDriver::checkRevBlockHELPER(const WriteLabel *sLab, const std::vector<ReadLabel *> &loads)
{
	if (!getConf()->helper || !sLab->hasAttr(WriteAttr::RevBlocker))
		return true;

	auto &g = getExec().getGraph();
	if (std::any_of(loads.begin(), loads.end(), [this, &g, sLab](const auto *rLab) {
		    auto *lLab = g.getLastThreadLabel(rLab->getThread());
		    auto *pLab = getPreviousVisibleAccessLabel(lLab->getPos());
		    return llvm::isa<BlockLabel>(lLab) && pLab && pLab->getPos() == rLab->getPos();
	    })) {
		moot();
		return false;
	}
	return true;
}

bool GenMCDriver::calcRevisits(WriteLabel *sLab)
{
	auto &g = getExec().getGraph();
	auto loads = getRevisitableApproximation(sLab);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug3) << "Revisitable: " << format(loads) << "\n";);
	if (tryOptimizeRevisits(sLab, loads))
		return true;

	/* If operating in estimation mode, don't actually revisit */
	if (inEstimationMode()) {
		getExec().getChoiceMap().update(loads, sLab);
		return checkAtomicity(sLab) && checkRevBlockHELPER(sLab, loads) && !isMoot();
	}

	GENMC_DEBUG(LOG(VerbosityLevel::Debug3)
			    << "Revisitable (optimized): " << format(loads) << "\n";);
	for (auto *rLab : loads) {
		auto br = constructBackwardRevisit(rLab, sLab);
		if (!isMaximalExtension(*br))
			break;

		getExec().getWorkqueue().add(std::move(br));
	}

	return checkAtomicity(sLab) && checkRevBlockHELPER(sLab, loads) && !isMoot();
}

WriteLabel *GenMCDriver::completeRevisitedRMW(const ReadLabel *rLab)
{
	/* Handle non-RMW cases first */
	if (!llvm::isa<CasReadLabel>(rLab) && !llvm::isa<FaiReadLabel>(rLab))
		return nullptr;
	if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		if (rLab->getAccessValue(rLab->getAccess()) != casLab->getExpected())
			return nullptr;
	}

	SVal result;
	WriteAttr wattr = WriteAttr::None;
	if (auto *faiLab = llvm::dyn_cast<FaiReadLabel>(rLab)) {
		/* Need to get the rf value within the if, as rLab might be a disk op,
		 * and we cannot get the value in that case (but it will also not be an RMW)
		 */
		auto rfVal = rLab->getAccessValue(rLab->getAccess());
		result = getEE()->executeAtomicRMWOperation(rfVal, faiLab->getOpVal(),
							    faiLab->getSize(), faiLab->getOp());
		if (llvm::isa<BIncFaiReadLabel>(faiLab) && result == SVal(0))
			result = findBarrierInitValue(getExec().getGraph(), rLab->getAccess());
		wattr = faiLab->getAttr();
	} else if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		result = casLab->getSwapVal();
		wattr = casLab->getAttr();
	} else
		BUG();

	auto &g = getExec().getGraph();
	std::unique_ptr<WriteLabel> wLab = nullptr;

#define CREATE_COUNTERPART(name)                                                                   \
	case EventLabel::name##Read:                                                               \
		wLab = name##WriteLabel::create(rLab->getPos().next(), rLab->getOrdering(),        \
						rLab->getAddr(), rLab->getSize(), rLab->getType(), \
						result, wattr);                                    \
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
	auto *lab = llvm::dyn_cast<WriteLabel>(addLabelToGraph(std::move(wLab)));
	BUG_ON(!rLab->getRf());
	g.addStoreToCOAfter(lab, rLab->getRf());
	return lab;
}

bool GenMCDriver::revisitWrite(const WriteForwardRevisit &ri)
{
	auto &g = getExec().getGraph();
	auto *wLab = g.getWriteLabel(ri.getPos());
	BUG_ON(!wLab);

	g.moveStoreCOAfter(wLab, g.getEventLabel(ri.getPred()));
	wLab->setAddedMax(false);
	return calcRevisits(wLab);
}

bool GenMCDriver::revisitOptional(const OptionalForwardRevisit &oi)
{
	auto &g = getExec().getGraph();
	auto *oLab = llvm::dyn_cast<OptionalLabel>(g.getEventLabel(oi.getPos()));

	--result.exploredBlocked;
	BUG_ON(!oLab);
	oLab->setExpandable(false);
	oLab->setExpanded(true);
	return true;
}

bool GenMCDriver::revisitRead(const Revisit &ri)
{
	BUG_ON(!llvm::isa<ReadRevisit>(&ri));

	/* We are dealing with a read: change its reads-from and also check
	 * whether a part of an RMW should be added */
	auto &g = getExec().getGraph();
	auto *rLab = g.getReadLabel(ri.getPos());
	auto *revLab = g.getEventLabel(llvm::dyn_cast<ReadRevisit>(&ri)->getRev());

	rLab->setRf(revLab);
	auto *fri = llvm::dyn_cast<ReadForwardRevisit>(&ri);
	rLab->setAddedMax(fri ? fri->isMaximal() : revLab == g.co_max(rLab->getAddr()));
	rLab->setIPRStatus(false);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug1)
			    << "--- " << (llvm::isa<BackwardRevisit>(ri) ? "Backward" : "Forward")
			    << " revisiting " << ri.getPos() << " <-- " << revLab->getPos() << "\n"
			    << getExec().getGraph(););

	/*  Try to remove the read from the execution */
	if (removeCASReadIfBlocks(rLab, revLab))
		return true;

	/* If the revisited label became an RMW, add the store part and revisit */
	if (auto *sLab = completeRevisitedRMW(rLab))
		return calcRevisits(sLab);

	/* Blocked barrier: block thread */
	if (llvm::isa<BWaitReadLabel>(rLab) &&
	    rLab->getAccessValue(rLab->getAccess()) != findBarrierInitValue(g, rLab->getAccess())) {
		blockThread(BarrierBlockLabel::create(rLab->getPos().next()));
	}

	/* Blocked lock -> prioritize locking thread */
	if (llvm::isa<LockCasReadLabel>(rLab)) {
		blockThread(LockNotAcqBlockLabel::create(rLab->getPos().next()));
		if (!getConf()->bound.has_value())
			threadPrios = {rLab->getRf()->getPos()};
	}
	auto rpreds = po_preds(g, rLab);
	auto oLabIt = std::ranges::find_if(
		rpreds, [&](auto &oLab) { return llvm::isa<SpeculativeReadLabel>(&oLab); });
	if (getConf()->helper && (llvm::isa<SpeculativeReadLabel>(rLab) || oLabIt != rpreds.end()))
		threadPrios = {rLab->getPos()};
	return true;
}

bool GenMCDriver::forwardRevisit(const ForwardRevisit &fr)
{
	auto &g = getExec().getGraph();
	auto *lab = g.getEventLabel(fr.getPos());
	if (auto *mi = llvm::dyn_cast<WriteForwardRevisit>(&fr))
		return revisitWrite(*mi);
	if (auto *oi = llvm::dyn_cast<OptionalForwardRevisit>(&fr))
		return revisitOptional(*oi);
	if (auto *rr = llvm::dyn_cast<RerunForwardRevisit>(&fr))
		return true;
	auto *ri = llvm::dyn_cast<ReadForwardRevisit>(&fr);
	BUG_ON(!ri);
	return revisitRead(*ri);
}

bool GenMCDriver::backwardRevisit(const BackwardRevisit &br)
{
	auto &g = getExec().getGraph();

	/* Recalculate the view because some B labels might have been
	 * removed */
	auto *brh = llvm::dyn_cast<BackwardRevisitHELPER>(&br);
	auto v = getRevisitView(g.getReadLabel(br.getPos()), g.getWriteLabel(br.getRev()),
				brh ? g.getWriteLabel(brh->getMid()) : nullptr);

	auto og = copyGraph(&br, &*v);
	auto cmap = ChoiceMap(getExec().getChoiceMap());
	cmap.cut(*v);
	auto alloctor = SAddrAllocator(getExec().getAllocator());
	alloctor.restrict(createAllocView(*og));

	pushExecution(
		{std::move(og), LocalQueueT(), std::move(cmap), std::move(alloctor), br.getPos()});

	repairDanglingReads(getExec().getGraph());
	auto ok = revisitRead(br);
	BUG_ON(!ok);

	/* If there are idle workers in the thread pool,
	 * try submitting the job instead */
	auto *tp = getThreadPool();
	if (tp && tp->getRemainingTasks() < 8 * tp->size()) {
		if (isRevisitValid(br))
			tp->submit(extractState());
		return false;
	}
	return true;
}

bool GenMCDriver::restrictAndRevisit(const WorkList::ItemT &item)
{
	/* First, appropriately restrict the worklist and the graph */
	auto &g = getExec().getGraph();
	auto *br = llvm::dyn_cast<BackwardRevisit>(&*item);
	auto stamp = g.getEventLabel(br ? br->getRev() : item->getPos())->getStamp();
	getExec().restrict(stamp);

	getExec().getLastAdded() = item->getPos();
	if (auto *fr = llvm::dyn_cast<ForwardRevisit>(&*item))
		return forwardRevisit(*fr);
	if (auto *br = llvm::dyn_cast<BackwardRevisit>(&*item)) {
		return backwardRevisit(*br);
	}
	BUG();
	return false;
}

bool GenMCDriver::handleHelpingCas(std::unique_ptr<HelpingCasLabel> hLab)
{
	if (isExecutionDrivenByGraph(&*hLab))
		return true;

	/* Ensure that the helped CAS exists */
	auto *lab = llvm::dyn_cast<HelpingCasLabel>(addLabelToGraph(std::move(hLab)));
	if (!checkHelpingCasCondition(lab)) {
		blockThread(HelpedCASBlockLabel::create(lab->getPos()));
		return false;
	}
	return true;
}

bool GenMCDriver::handleOptional(std::unique_ptr<OptionalLabel> lab)
{
	auto &g = getExec().getGraph();

	if (isExecutionDrivenByGraph(&*lab))
		return llvm::dyn_cast<OptionalLabel>(g.getEventLabel(lab->getPos()))->isExpanded();

	if (std::any_of(g.label_begin(), g.label_end(), [&](auto &lab) {
		    auto *oLab = llvm::dyn_cast<OptionalLabel>(&lab);
		    return oLab && !oLab->isExpandable();
	    }))
		lab->setExpandable(false);

	auto *oLab = llvm::dyn_cast<OptionalLabel>(addLabelToGraph(std::move(lab)));

	if (!inEstimationMode() && oLab->isExpandable())
		getExec().getWorkqueue().add(
			std::make_unique<OptionalForwardRevisit>(oLab->getPos()));
	return false; /* should not be expanded yet */
}

void GenMCDriver::handleSpinStart(std::unique_ptr<SpinStartLabel> lab)
{
	auto &g = getExec().getGraph();

	/* If it has not been added to the graph, do so */
	if (isExecutionDrivenByGraph(&*lab))
		return;

	auto *stLab = addLabelToGraph(std::move(lab));

	/* Check whether we can detect some spinloop dynamically */
	auto stpreds = po_preds(g, stLab);
	auto lbLabIt = std::ranges::find_if(
		stpreds, [](auto &lab) { return llvm::isa<LoopBeginLabel>(lab); });

	/* If we did not find a loop-begin, this a manual instrumentation(?); report to user
	 */
	ERROR_ON(lbLabIt == stpreds.end(), "No loop-beginning found!\n");

	auto *lbLab = &*lbLabIt;
	auto pLabIt = std::ranges::find_if(stpreds, [lbLab](auto &lab) {
		return llvm::isa<SpinStartLabel>(&lab) && lab.getIndex() > lbLab->getIndex();
	});
	if (pLabIt == stpreds.end())
		return;

	auto *pLab = &*pLabIt;
	for (auto i = pLab->getIndex() + 1; i < stLab->getIndex(); i++) {
		auto *wLab =
			llvm::dyn_cast<WriteLabel>(g.getEventLabel(Event(stLab->getThread(), i)));
		if (wLab && wLab->isEffectful() && wLab->isObservable())
			return; /* found event w/ side-effects */
	}
	/* Spinloop detected */
	blockThreadTryMoot(SpinloopBlockLabel::create(stLab->getPos()));
}

bool GenMCDriver::areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab)
{
	auto &g = getExec().getGraph();

	/* Check that there are no other side-effects since the previous iteration.
	 * We don't have to look for a BEGIN label since ZNE labels are always
	 * preceded by a spin-start */
	auto preds = po_preds(g, lab);
	auto ssLabIt = std::ranges::find_if(
		preds, [](auto &lab) { return llvm::isa<SpinStartLabel>(&lab); });
	BUG_ON(ssLabIt == preds.end());
	auto *ssLab = &*ssLabIt;
	for (auto i = ssLab->getIndex() + 1; i < lab->getIndex(); ++i) {
		auto *oLab = g.getEventLabel(Event(ssLab->getThread(), i));
		if (llvm::isa<WriteLabel>(oLab) && !llvm::isa<FaiWriteLabel>(oLab))
			return false;
	}

	auto wLabIt = std::ranges::find_if(
		preds, [](auto &lab) { return llvm::isa<FaiWriteLabel>(&lab); });
	BUG_ON(wLabIt == preds.end());

	/* All stores in the RMW chain need to be read from at most 1 read,
	 * and there need to be no other stores that are not hb-before lab */
	auto *wLab = llvm::dyn_cast<FaiWriteLabel>(&*wLabIt);
	for (auto &lab : g.labels()) {
		if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(&lab)) {
			if (mLab->getAddr() == wLab->getAddr() && !llvm::isa<FaiReadLabel>(mLab) &&
			    !llvm::isa<FaiWriteLabel>(mLab) &&
			    !getConsChecker().getHbView(wLab).contains(mLab->getPos()))
				return false;
		}
	}
	return true;
}

void GenMCDriver::handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab)
{
	auto &g = getExec().getGraph();

	/* If we are actually replaying this one, it is not a spin loop*/
	if (isExecutionDrivenByGraph(&*lab))
		return;

	auto *zLab = llvm::dyn_cast<FaiZNESpinEndLabel>(addLabelToGraph(std::move(lab)));
	if (areFaiZNEConstraintsSat(zLab))
		blockThreadTryMoot(FaiZNEBlockLabel::create(zLab->getPos()));
}

void GenMCDriver::handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab)
{
	if (isExecutionDrivenByGraph(&*lab))
		return;

	auto *zLab = addLabelToGraph(std::move(lab));
	blockThreadTryMoot(LockZNEBlockLabel::create(zLab->getPos()));
}

void GenMCDriver::handleDummy(std::unique_ptr<EventLabel> lab)
{
	if (!isExecutionDrivenByGraph(&*lab))
		addLabelToGraph(std::move(lab));
}

/************************************************************
 ** Printing facilities
 ***********************************************************/

static void executeMDPrint(const EventLabel *lab, const std::pair<int, std::string> &locAndFile,
			   std::string inputFile, llvm::raw_ostream &os = llvm::outs())
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
	if (llvm::isa<ThreadStartLabel>(lab) || llvm::isa<ThreadFinishLabel>(lab))
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

	auto &g = getExec().getGraph();
	auto *aLab = findAllocatingLabel(g, addr);

	if (!aLab)
		return "???";
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

void GenMCDriver::printGraph(bool printMetadata /* false */,
			     llvm::raw_ostream &s /* = llvm::dbgs() */)
{
	auto &g = getExec().getGraph();
	LabelPrinter printer([this](const SAddr &saddr) { return getVarName(saddr); },
			     [this](const ReadLabel &lab) {
				     return lab.getRf() ? lab.getAccessValue(lab.getAccess())
							: SVal();
			     });

	/* Print the graph */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);
		s << thr;
		if (getConf()->symmetryReduction) {
			if (auto *bLab = g.getFirstThreadLabel(i)) {
				auto symm = bLab->getSymmPredTid();
				if (symm != -1)
					s << " symmetric with " << symm;
			}
		}
		s << ":\n";
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			s << "\t";
			GENMC_DEBUG(if (getConf()->colorAccesses)
					    s.changeColor(getLabelColor(lab)););
			s << printer.toString(*lab);
			GENMC_DEBUG(s.resetColor(););
			GENMC_DEBUG(if (getConf()->printStamps) s << " @ " << lab->getStamp(););
			if (printMetadata && thr.prefixLOC[j].first && shouldPrintLOC(lab)) {
				executeMDPrint(lab, thr.prefixLOC[j], getConf()->inputFile, s);
			}
			s << "\n";
		}
	}

	/* MO: Print coherence information */
	auto header = false;
	for (auto locIt = g.loc_begin(), locE = g.loc_end(); locIt != locE; ++locIt) {
		/* Skip empty and single-store locations */
		if (g.hasLocMoreThanOneStore(locIt->first)) {
			if (!header) {
				s << "Coherence:\n";
				header = true;
			}
			auto *wLab = &*g.co_begin(locIt->first);
			s << getVarName(wLab->getAddr()) << ": [ ";
			for (const auto &w : g.co(locIt->first))
				s << w << " ";
			s << "]\n";
		}
	}
	s << "\n";
}

void GenMCDriver::dotPrintToFile(const std::string &filename, const EventLabel *errLab,
				 const EventLabel *confLab)
{
	auto &g = getExec().getGraph();
	auto *EE = getEE();
	std::ofstream fout(filename);
	llvm::raw_os_ostream ss(fout);
	DotPrinter printer([this](const SAddr &saddr) { return getVarName(saddr); },
			   [this](const ReadLabel &lab) {
				   return lab.getRf() ? lab.getAccessValue(lab.getAccess())
						      : SVal();
			   });

	auto before = getPrefixView(errLab).clone();
	if (confLab)
		before->update(getPrefixView(confLab));

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
		for (auto j = 1; j <= before->getMax(i); j++) {
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
			   << (lab->getPos() == errLab->getPos() ||
					       lab->getPos() == confLab->getPos()
				       ? ",style=filled,fillcolor=yellow"
				       : "")
			   << "]\n";
		}
		ss << "}\n";
	}

	/* Print relations between events (po U rf) */
	for (auto i = 0u; i < before->size(); i++) {
		auto &thr = EE->getThrById(i);
		for (auto j = 0; j <= before->getMax(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));

			/* Print a po-edge, but skip dummy start events for
			 * all threads except for the first one */
			if (j < before->getMax(i) && !llvm::isa<ThreadStartLabel>(lab))
				ss << "\"" << lab->getPos() << "\" -> \"" << lab->getPos().next()
				   << "\"\n";
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				/* Do not print RFs from INIT, BOTTOM, and same thread */
				if (llvm::dyn_cast_or_null<WriteLabel>(rLab) &&
				    rLab->getRf()->getThread() != lab->getThread()) {
					ss << "\"" << rLab->getRf() << "\" -> \"" << rLab->getPos()
					   << "\"[color=green, constraint=false]\n";
				}
			}
			if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab)) {
				if (thr.id == 0)
					continue;
				ss << "\"" << bLab->getParentCreate() << "\" -> \""
				   << bLab->getPos().next() << "\"[color=blue, constraint=false]\n";
			}
			if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab))
				ss << "\"" << g.getLastThreadLabel(jLab->getChildId())->getPos()
				   << "\" -> \"" << jLab->getPos()
				   << "\"[color=blue, constraint=false]\n";
		}
	}

	ss << "}\n";
}

void GenMCDriver::recPrintTraceBefore(const Event &e, View &a,
				      llvm::raw_ostream &ss /* llvm::outs() */)
{
	const auto &g = getExec().getGraph();

	if (a.contains(e))
		return;

	auto ai = a.getMax(e.thread);
	a.setMax(e);
	auto &thr = getEE()->getThrById(e.thread);
	for (int i = ai; i <= e.index; i++) {
		const EventLabel *lab = g.getEventLabel(Event(e.thread, i));
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
			if (rLab->getRf())
				recPrintTraceBefore(rLab->getRf()->getPos(), a, ss);
		if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab))
			recPrintTraceBefore(g.getLastThreadLabel(jLab->getChildId())->getPos(), a,
					    ss);
		if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab))
			if (!bLab->getParentCreate().isInitializer())
				recPrintTraceBefore(bLab->getParentCreate(), a, ss);

		/* Do not print the line if it is an RMW write, since it will be
		 * the same as the previous one */
		if (llvm::isa<CasWriteLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
			continue;
		/* Similarly for a Wna just after the creation of a thread
		 * (it is the store of the PID) */
		if (i > 0 && llvm::isa<ThreadCreateLabel>(g.po_imm_pred(lab)))
			continue;
		Parser::parseInstFromMData(thr.prefixLOC[i], thr.threadFun->getName().str(), ss);
	}
	return;
}

void GenMCDriver::printTraceBefore(const EventLabel *lab, llvm::raw_ostream &s /* = llvm::dbgs() */)
{
	s << "Trace to " << lab->getPos() << ":\n";

	/* Linearize (po U rf) and print trace */
	View a;
	recPrintTraceBefore(lab->getPos(), a, s);
}
