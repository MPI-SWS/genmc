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
#include "BoundDecider.hpp"
#include "Config.hpp"
#include "DepExecutionGraph.hpp"
#include "DriverHandlerDispatcher.hpp"
#include "Error.hpp"
#include "GraphIterators.hpp"
#include "Interpreter.h"
#include "LLVMModule.hpp"
#include "LabelVisitor.hpp"
#include "Logger.hpp"
#include "MaximalIterator.hpp"
#include "Parser.hpp"
#include "SExprVisitor.hpp"
#include "ThreadPool.hpp"
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
			 std::unique_ptr<ModuleInfo> modInfo, Mode mode /* = VerificationMode{} */)
	: userConf(std::move(conf)), mode(mode)
{
	/* Set up the execution context */
	auto execGraph = userConf->isDepTrackingModel ? std::make_unique<DepExecutionGraph>()
						      : std::make_unique<ExecutionGraph>();
	execStack.emplace_back(std::move(execGraph), std::move(LocalQueueT()),
			       std::move(ChoiceMap()));

	auto hasBounder = userConf->bound.has_value();
	GENMC_DEBUG(hasBounder |= userConf->boundsHistogram;);
	if (hasBounder)
		bounder = BoundDecider::create(getConf()->boundType);

	/* Create an interpreter for the program's instructions */
	std::string buf;
	EE = llvm::Interpreter::create(std::move(mod), std::move(modInfo), this, getConf(),
				       getAddrAllocator(), &buf);

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

GenMCDriver::Execution::Execution(std::unique_ptr<ExecutionGraph> g, LocalQueueT &&w, ChoiceMap &&m)
	: graph(std::move(g)), workqueue(std::move(w)), choices(std::move(m))
{}
GenMCDriver::Execution::~Execution() = default;

void repairRead(ExecutionGraph &g, ReadLabel *lab)
{
	auto last = (store_rbegin(g, lab->getAddr()) == store_rend(g, lab->getAddr()))
			    ? Event::getInit()
			    : store_rbegin(g, lab->getAddr())->getPos();
	g.changeRf(lab->getPos(), last);
	lab->setAddedMax(true);
	lab->setIPRStatus(g.getEventLabel(last)->getStamp() > lab->getStamp());
}

void repairDanglingReads(ExecutionGraph &g)
{
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *rLab = llvm::dyn_cast<ReadLabel>(g.getLastThreadLabel(i));
		if (!rLab)
			continue;
		if (!rLab->getRf()) {
			repairRead(g, rLab);
		}
	}
}

void GenMCDriver::Execution::restrictGraph(Stamp stamp)
{
	/* Restrict the graph (and relations). It can be the case that
	 * events with larger stamp remain in the graph (e.g.,
	 * BEGINs). Fix their stamps too. */
	auto &g = getGraph();
	g.cutToStamp(stamp);
	g.compressStampsAfter(stamp);
	repairDanglingReads(g);
}

void GenMCDriver::Execution::restrictWorklist(Stamp stamp)
{
	std::vector<Stamp> idxsToRemove;

	auto &workqueue = getWorkqueue();
	for (auto rit = workqueue.rbegin(); rit != workqueue.rend(); ++rit)
		if (rit->first > stamp && rit->second.empty())
			idxsToRemove.push_back(rit->first); // TODO: break out of loop?

	for (auto &i : idxsToRemove)
		workqueue.erase(i);
}

void GenMCDriver::Execution::restrictChoices(Stamp stamp)
{
	auto &choices = getChoiceMap();
	for (auto cit = choices.begin(); cit != choices.end();) {
		if (cit->first > stamp.get()) {
			cit = choices.erase(cit);
		} else {
			++cit;
		}
	}
}

void GenMCDriver::Execution::restrict(Stamp stamp)
{
	restrictGraph(stamp);
	restrictWorklist(stamp);
	restrictChoices(stamp);
}

void GenMCDriver::pushExecution(Execution &&e) { execStack.push_back(std::move(e)); }

bool GenMCDriver::popExecution()
{
	if (execStack.empty())
		return false;
	execStack.pop_back();
	return !execStack.empty();
}

GenMCDriver::State::State(std::unique_ptr<ExecutionGraph> g, ChoiceMap &&m, SAddrAllocator &&a,
			  llvm::BitVector &&fds, ValuePrefixT &&c, Event la)
	: graph(std::move(g)), choices(std::move(m)), alloctor(std::move(a)), fds(std::move(fds)),
	  cache(std::move(c)), lastAdded(la)
{}
GenMCDriver::State::~State() = default;

void GenMCDriver::initFromState(std::unique_ptr<State> s)
{
	execStack.clear();
	execStack.emplace_back(std::move(s->graph), LocalQueueT(), std::move(s->choices));
	alloctor = std::move(s->alloctor);
	fds = std::move(s->fds);
	seenPrefixes = std::move(s->cache);
	lastAdded = s->lastAdded;
}

std::unique_ptr<GenMCDriver::State> GenMCDriver::extractState()
{
	auto cache = std::move(seenPrefixes);
	seenPrefixes.clear();
	return std::make_unique<State>(getGraph().clone(), ChoiceMap(getChoiceMap()),
				       SAddrAllocator(alloctor), llvm::BitVector(fds),
				       std::move(cache), lastAdded);
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
		return getAddrAllocator().allocAutomatic(
			aLab->getAllocSize(), alignment,
			aLab->getStorageType() == StorageType::ST_Durable,
			aLab->getAddressSpace() == AddressSpace::AS_Internal);
	case StorageDuration::SD_Heap:
		return getAddrAllocator().allocHeap(
			aLab->getAllocSize(), alignment,
			aLab->getStorageType() == StorageType::ST_Durable,
			aLab->getAddressSpace() == AddressSpace::AS_Internal);
	case StorageDuration::SD_Static: /* Cannot ask for fresh static addresses */
	default:
		BUG();
	}
	BUG();
	return SAddr();
}

int GenMCDriver::getFreshFd()
{
	int fd = fds.find_first_unset();

	/* If no available descriptor found, grow fds and try again */
	if (fd == -1) {
		fds.resize(2 * fds.size() + 1);
		return getFreshFd();
	}

	/* Otherwise, mark the file descriptor as used */
	markFdAsUsed(fd);
	return fd;
}

void GenMCDriver::markFdAsUsed(int fd)
{
	if (fd > fds.size())
		fds.resize(fd);
	fds.set(fd);
}

void GenMCDriver::resetThreadPrioritization() { threadPrios.clear(); }

bool GenMCDriver::isSchedulable(int thread) const
{
	auto &thr = getEE()->getThrById(thread);
	auto *lab = getGraph().getLastThreadLabel(thread);
	return !thr.ECStack.empty() && !lab->isTerminator();
}

bool GenMCDriver::schedulePrioritized()
{
	/* Return false if no thread is prioritized */
	if (threadPrios.empty())
		return false;

	BUG_ON(getConf()->bound.has_value());

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
	auto *ci = llvm::dyn_cast<CallInst>(I);
	return llvm::isa<llvm::LoadInst>(I) || llvm::isa<llvm::AtomicCmpXchgInst>(I) ||
	       llvm::isa<llvm::AtomicRMWInst>(I) ||
	       (ci && ci->getCalledFunction() &&
		hasGlobalLoadSemantics(ci->getCalledFunction()->getName().str()));
}

bool GenMCDriver::scheduleNextWF()
{
	auto &g = getGraph();
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

	auto firstSched = tid;
	auto symm = getSymmPredTid(tid);
	while (symm != -1) {
		if (isSchedulable(symm))
			firstSched = symm;
		symm = getSymmPredTid(symm);
	}
	return firstSched;
}

bool GenMCDriver::scheduleNextWFR()
{
	auto &g = getGraph();
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
	auto &g = getGraph();
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
	const auto &g = getGraph();

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

std::pair<std::vector<SVal>, Event> GenMCDriver::extractValPrefix(Event pos)
{
	auto &g = getGraph();
	std::vector<SVal> vals;
	Event last;

	for (auto i = 0u; i < pos.index; i++) {
		auto *lab = g.getEventLabel(Event(pos.thread, i));
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			auto *drLab = llvm::dyn_cast<DskReadLabel>(rLab);
			vals.push_back(drLab ? getDskReadValue(drLab) : getReadValue(rLab));
			last = lab->getPos();
		} else if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab)) {
			vals.push_back(getJoinValue(jLab));
			last = lab->getPos();
		} else if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab)) {
			vals.push_back(getStartValue(bLab));
			last = lab->getPos();
		} else if (auto *oLab = llvm::dyn_cast<OptionalLabel>(lab)) {
			vals.push_back(SVal(oLab->isExpanded()));
			last = lab->getPos();
		} else {
			BUG_ON(lab->hasValue());
		}
	}
	return {vals, last};
}

Event findNextLabelToAdd(const ExecutionGraph &g, Event pos)
{
	auto first = Event(pos.thread, 0);
	auto it = std::find_if(po_succ_begin(g, first), po_succ_end(g, first),
			       [&](auto &lab) { return llvm::isa<EmptyLabel>(&lab); });
	return it == po_succ_end(g, first) ? g.getLastThreadEvent(pos.thread).next() : it->getPos();
}

bool GenMCDriver::tryOptimizeScheduling(Event pos)
{
	if (!getConf()->instructionCaching || inEstimationMode())
		return false;

	auto next = findNextLabelToAdd(getGraph(), pos);
	auto [vals, last] = extractValPrefix(next);
	auto *res = retrieveCachedSuccessors(pos.thread, vals);
	if (res == nullptr || res->empty() || res->back()->getIndex() < next.index)
		return false;

	for (auto &vlab : *res) {
		BUG_ON(vlab->hasStamp());

		DriverHandlerDispatcher dispatcher(this);
		dispatcher.visit(vlab);
		if (llvm::isa<BlockLabel>(getGraph().getLastThreadLabel(vlab->getThread())) ||
		    isMoot() || getEE()->getCurThr().isBlocked() || isHalting())
			return true;
	}
	return true;
}

void GenMCDriver::checkHelpingCasAnnotation()
{
	/* If we were waiting for a helped CAS that did not appear, complain */
	auto &g = getGraph();
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (llvm::isa<HelpedCASBlockLabel>(g.getLastThreadLabel(i)))
			ERROR("Helped/Helping CAS annotation error! Does helped CAS always "
			      "execute?\n");
	}

	/* Next, we need to check whether there are any extraneous
	 * stores, not visible to the helped/helping CAS */
	auto hs = g.collectAllEvents(
		[&](const EventLabel *lab) { return llvm::isa<HelpingCasLabel>(lab); });
	if (hs.empty())
		return;

	for (auto &h : hs) {
		auto *hLab = llvm::dyn_cast<HelpingCasLabel>(g.getEventLabel(h));
		BUG_ON(!hLab);

		/* Check that all stores that would make this helping
		 * CAS succeed are read by a helped CAS.
		 * We don't need to check the swap value of the helped CAS */
		if (std::any_of(store_begin(g, hLab->getAddr()), store_end(g, hLab->getAddr()),
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
			auto rs = g.collectAllEvents([&](const EventLabel *lab) {
				auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
				return rLab && rLab->getAddr() == hLab->getAddr();
			});
			if (std::none_of(rs.begin(), rs.end(), [&](const Event &r) {
				    return llvm::isa<HelpedCasReadLabel>(g.getEventLabel(r));
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
	auto bound = bounder->calculate(getGraph());
	result.exploredBounds.grow(bound);
	result.exploredBounds[bound]++;
}
#endif

bool GenMCDriver::isExecutionBlocked() const
{
	return std::any_of(
		getEE()->threads_begin(), getEE()->threads_end(), [this](const llvm::Thread &thr) {
			// FIXME: was thr.isBlocked()
			auto &g = getGraph();
			if (thr.id >= g.getNumThreads() || g.isThreadEmpty(thr.id)) // think rec
				return false;
			return llvm::isa<BlockLabel>(g.getLastThreadLabel(thr.id));
		});
}

void GenMCDriver::updateStSpaceEstimation()
{
	/* Calculate current sample */
	auto &choices = getChoiceMap();
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
			addToWorklist(0, std::make_unique<RerunForwardRevisit>());
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

	if (fullExecutionExceedsBound())
		++result.boundExceeding;

	if (getConf()->printExecGraphs && !getConf()->persevere)
		printGraph(); /* Delay printing if persevere is enabled */

	GENMC_DEBUG(if (getConf()->boundsHistogram && !inEstimationMode()) trackExecutionBound(););

	++result.explored;
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
	auto psb = g.collectAllEvents(
		[&](const EventLabel *lab) { return llvm::isa<DskPbarrierLabel>(lab); });
	if (psb.empty())
		psb.push_back(Event::getInit());
	ERROR_ON(psb.size() > 1, "Usage of only one persistency barrier is allowed!\n");

	auto tsLab = ThreadStartLabel::create(Event(tid, 0), psb.back(),
					      ThreadInfo(tid, psb.back().thread, 0, 0));
	auto *lab = addLabelToGraph(std::move(tsLab));

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
		auto driver = DriverFactory::create(conf, std::move(mod), std::move(modInfo));
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
	auto driver = DriverFactory::create(conf, std::move(newmod), std::move(newMI),
					    GenMCDriver::EstimationMode{conf->estimationMax});
	driver->run();
	return driver->getResult();
}

void GenMCDriver::addToWorklist(Stamp stamp, WorkSet::ItemT item)
{
	getWorkqueue()[stamp].add(std::move(item));
}

std::pair<Stamp, WorkSet::ItemT> GenMCDriver::getNextItem()
{
	auto &workqueue = getWorkqueue();
	for (auto rit = workqueue.rbegin(); rit != workqueue.rend(); ++rit) {
		if (rit->second.empty()) {
			continue;
		}

		return {rit->first, rit->second.getNext()};
	}
	return {0, nullptr};
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
	getGraph().addLabelToGraph(std::move(bLab));
}

void GenMCDriver::blockThreadTryMoot(std::unique_ptr<BlockLabel> bLab)
{
	auto pos = bLab->getPos();
	blockThread(std::move(bLab));
	mootExecutionIfFullyBlocked(pos);
}

void GenMCDriver::unblockThread(Event pos)
{
	auto *bLab = getGraph().getLastThreadLabel(pos.thread);
	BUG_ON(!llvm::isa<BlockLabel>(bLab));
	getGraph().removeLast(pos.thread);
}

bool GenMCDriver::scheduleAtomicity()
{
	auto *lastLab = getGraph().getEventLabel(lastAdded);
	if (llvm::isa<FaiReadLabel>(lastLab)) {
		getEE()->scheduleThread(lastAdded.thread);
		return true;
	}
	if (auto *casLab = llvm::dyn_cast<CasReadLabel>(lastLab)) {
		if (getReadValue(casLab) == casLab->getExpected()) {
			getEE()->scheduleThread(lastAdded.thread);
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
	auto &g = getGraph();
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

	auto &g = getGraph();
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
	EE->setExecutionContext(createExecutionContext(getGraph()));
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

			auto [stamp, item] = getNextItem();
			if (!item) {
				if (popExecution())
					continue;
				return;
			}
			auto pos = item->getPos();
			validExecution = restrictAndRevisit(stamp, item) && isRevisitValid(*item);
		}
	}
}

bool isUninitializedAccess(const SAddr &addr, const Event &pos)
{
	return addr.isDynamic() && pos.isInitializer();
}

bool readsUninitializedMem(const ReadLabel *lab)
{
	return isUninitializedAccess(lab->getAddr(), lab->getRf()->getPos());
}

bool GenMCDriver::isRevisitValid(const Revisit &revisit)
{
	auto &g = getGraph();
	auto pos = revisit.getPos();
	auto *mLab = llvm::dyn_cast<MemAccessLabel>(g.getEventLabel(pos));

	/* E.g., for optional revisits, do nothing */
	if (!mLab)
		return true;

	if (!isExecutionValid(mLab))
		return false;

	auto *rLab = llvm::dyn_cast<ReadLabel>(mLab);
	if (rLab && readsUninitializedMem(rLab)) {
		reportError(pos, VerificationError::VE_UninitializedMem);
		return false;
	}

	/* If an extra event is added, re-check consistency */
	auto *nLab = g.getNextLabel(pos);
	return !g.isRMWLoad(pos) ||
	       (isExecutionValid(nLab) && checkForRaces(nLab) == VerificationError::VE_OK);
}

bool GenMCDriver::isExecutionDrivenByGraph(const EventLabel *lab)
{
	const auto &g = getGraph();
	auto curr = lab->getPos();
	auto replay = (curr.index < g.getThreadSize(curr.thread)) &&
		      !llvm::isa<EmptyLabel>(g.getEventLabel(curr));
	if (!replay && !llvm::isa<MallocLabel>(lab) && !llvm::isa<ReadLabel>(lab))
		cacheEventLabel(lab);
	return replay;
}

bool GenMCDriver::executionExceedsBound(BoundCalculationStrategy strategy) const
{
	if (!getConf()->bound.has_value() || inEstimationMode())
		return false;

	return bounder->doesExecutionExceedBound(getGraph(), *getConf()->bound, strategy);
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
	auto &g = getGraph();
	auto *addedLab = g.addLabelToGraph(std::move(lab));
	updateLabelViews(addedLab);
	lastAdded = addedLab->getPos();
	if (addedLab->getIndex() >= getConf()->warnOnGraphSize) {
		LOG_ONCE("large-graph", VerbosityLevel::Tip)
			<< "The execution graph seems quite large. "
			<< "Consider bounding all loops or using -unroll\n";
	}
	return addedLab;
}

void GenMCDriver::updateLabelViews(EventLabel *lab)
{
	updateMMViews(lab);
	if (!getConf()->symmetryReduction)
		return;

	auto &v = lab->getPrefixView();
	updatePrefixWithSymmetriesSR(lab);
}

VerificationError GenMCDriver::checkForRaces(const EventLabel *lab)
{
	if (getConf()->disableRaceDetection || inEstimationMode())
		return VerificationError::VE_OK;

	/* Check for hard errors */
	const EventLabel *racyLab = nullptr;
	auto err = checkErrors(lab, racyLab);
	if (err != VerificationError::VE_OK) {
		reportError(lab->getPos(), err, "", racyLab);
		return err;
	}

	/* Check whether there are any unreported warnings... */
	std::vector<const EventLabel *> races;
	auto newWarnings = checkWarnings(lab, getResult().warnings, races);

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

	auto &g = getGraph();

	/* Extract value prefix and cached data */
	auto [vals, last] = extractValPrefix(lab->getPos());
	auto *data = retrieveCachedSuccessors(lab->getThread(), vals);

	/*
	 * Check if there are any new data to cache.
	 * (For dep-tracking, we could optimize toIdx and collect until
	 * a new (non-empty) label with a value is found.)
	 */
	auto fromIdx = (!data || data->empty()) ? last.index : data->back()->getIndex();
	auto toIdx = lab->getIndex();
	if (data && !data->empty() && data->back()->getIndex() >= toIdx)
		return;

	/*
	 * Go ahead and collect the new data. We have to be careful when
	 * cloning LAB because it has not been added to the graph yet.
	 */
	std::vector<std::unique_ptr<EventLabel>> labs;
	for (auto i = fromIdx + 1; i <= toIdx; i++) {
		auto cLab = (i == lab->getIndex())
				    ? lab->clone()
				    : g.getEventLabel(Event(lab->getThread(), i))->clone();
		cLab->reset();
		labs.push_back(std::move(cLab));
	}

	/* Is there an existing entry? */
	if (!data) {
		auto res = seenPrefixes[lab->getThread()].addSeq(vals, std::move(labs));
		BUG_ON(!res);
		return;
	}

	BUG_ON(data->empty() && last.index >= lab->getIndex());
	BUG_ON(!data->empty() && data->back()->getIndex() + 1 != lab->getIndex());

	data->reserve(data->size() + labs.size());
	std::move(std::begin(labs), std::end(labs), std::back_inserter(*data));
	labs.clear();
}

/* Given an event in the graph, returns the value of it */
SVal GenMCDriver::getWriteValue(const EventLabel *lab, const AAccess &access)
{
	/* If the even represents an invalid access, return some value */
	if (!lab)
		return SVal();

	/* If the event is the initializer, ask the interpreter about
	 * the initial value of that memory location */
	if (lab->getPos().isInitializer())
		return getEE()->getLocInitVal(access);

	/* Otherwise, we will get the value from the execution graph */
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	BUG_ON(!wLab);

	/* It can be the case that the load's type is different than
	 * the one the write's (see troep.c).  In any case though, the
	 * sizes should match */
	if (wLab->getSize() != access.getSize())
		reportError(wLab->getPos(), VerificationError::VE_MixedSize,
			    "Mixed-size accesses detected: tried to read event with a " +
				    std::to_string(access.getSize().get() * 8) + "-bit access!\n" +
				    "Please check the LLVM-IR.\n");

	/* If the size of the R and the W are the same, we are done */
	return wLab->getVal();
}

/* Same as above, but the data of a file are not explicitly initialized
 * so as not to pollute the graph with events, since a file can be large.
 * Thus, we treat the case where WRITE reads INIT specially. */
SVal GenMCDriver::getDskWriteValue(const EventLabel *lab, const AAccess &access)
{
	if (lab->getPos().isInitializer())
		return SVal();
	return getWriteValue(lab, access);
}

SVal GenMCDriver::getJoinValue(const ThreadJoinLabel *jLab) const
{
	auto &g = getGraph();
	auto *lLab = llvm::dyn_cast<ThreadFinishLabel>(g.getLastThreadLabel(jLab->getChildId()));
	BUG_ON(!lLab);
	return lLab->getRetVal();
}

SVal GenMCDriver::getStartValue(const ThreadStartLabel *bLab) const
{
	auto &g = getGraph();
	if (bLab->getPos().isInitializer() || bLab->getThread() == g.getRecoveryRoutineId())
		return SVal();

	return bLab->getThreadInfo().arg;
}

SVal GenMCDriver::getBarrierInitValue(const AAccess &access)
{
	const auto &g = getGraph();
	auto sIt = std::find_if(store_begin(g, access.getAddr()), store_end(g, access.getAddr()),
				[&access, &g](auto &bLab) {
					BUG_ON(!llvm::isa<WriteLabel>(bLab));
					return bLab.getAddr() == access.getAddr() &&
					       bLab.isNotAtomic();
				});

	/* All errors pertinent to initialization should be captured elsewhere */
	BUG_ON(sIt == store_end(g, access.getAddr()));
	return getWriteValue(&*sIt, access);
}

std::optional<SVal> GenMCDriver::getReadRetValue(const ReadLabel *rLab)
{
	/* Bottom is an acceptable re-option only @ replay */
	if (!rLab->getRf()) {
		BUG_ON(!inReplay());
		return std::nullopt;
	}

	/* Reading a non-init barrier value means that the thread should block */
	auto res = getReadValue(rLab);
	BUG_ON(llvm::isa<BWaitReadLabel>(rLab) && res != getBarrierInitValue(rLab->getAccess()) &&
	       !getGraph().getLastThreadLabel(rLab->getThread())->isTerminator());
	return {res};
}

SVal GenMCDriver::getRecReadRetValue(const ReadLabel *rLab)
{
	auto &g = getGraph();
	auto rf = g.getLastThreadStoreAtLoc(rLab->getPos(), rLab->getAddr());
	BUG_ON(rf.isInitializer());
	return getWriteValue(g.getEventLabel(rf), rLab->getAccess());
}

bool GenMCDriver::isCoMaximal(SAddr addr, Event e, bool checkCache /* = false */)
{
	return getGraph().isCoMaximal(addr, e, checkCache);
}

bool GenMCDriver::isHazptrProtected(const MemAccessLabel *mLab) const
{
	auto &g = getGraph();
	BUG_ON(!mLab->getAddr().isDynamic());

	auto *aLab = mLab->getAlloc();
	BUG_ON(!aLab);
	auto *pLab = llvm::dyn_cast_or_null<HpProtectLabel>(
		g.getPreviousLabelST(mLab, [&](const EventLabel *lab) {
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

MallocLabel *findAllocatingLabel(const ExecutionGraph &g, const SAddr &addr)
{
	auto labIt = std::find_if(label_begin(g), label_end(g), [&](auto &lab) {
		auto *mLab = llvm::dyn_cast<MallocLabel>(&lab);
		return mLab && mLab->contains(addr);
	});
	if (labIt != label_end(g))
		return llvm::dyn_cast<MallocLabel>(&*labIt);
	return nullptr;
}

bool GenMCDriver::isAccessValid(const MemAccessLabel *lab) const
{
	/* Make sure that the interperter is aware of this static variable */
	if (!lab->getAddr().isDynamic())
		return getEE()->isStaticallyAllocated(lab->getAddr());

	/* Dynamic accesses are valid if they access allocated memory */
	auto &g = getGraph();
	return !lab->getAddr().isNull() && findAllocatingLabel(g, lab->getAddr());
}

void GenMCDriver::checkLockValidity(const ReadLabel *rLab, const std::vector<Event> &rfs)
{
	auto *lLab = llvm::dyn_cast<LockCasReadLabel>(rLab);
	if (!lLab)
		return;

	/* Should not read from destroyed mutex */
	auto rfIt = std::find_if(rfs.cbegin(), rfs.cend(), [this, lLab](const Event &rf) {
		auto rfVal = getWriteValue(getGraph().getEventLabel(rf), lLab->getAccess());
		return rfVal == SVal(-1);
	});
	if (rfIt != rfs.cend())
		reportError(rLab->getPos(), VerificationError::VE_UninitializedMem,
			    "Called lock() on destroyed mutex!", getGraph().getEventLabel(*rfIt));
}

void GenMCDriver::checkUnlockValidity(const WriteLabel *wLab)
{
	auto *uLab = llvm::dyn_cast<UnlockWriteLabel>(wLab);
	if (!uLab)
		return;

	/* Unlocks should unlock mutexes locked by the same thread */
	if (getGraph().getMatchingLock(uLab->getPos()).isInitializer()) {
		reportError(uLab->getPos(), VerificationError::VE_InvalidUnlock,
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
				[&g, wLab](auto &sLab) {
					return &sLab != wLab && sLab.getAddr() == wLab->getAddr() &&
					       llvm::isa<BInitWriteLabel>(sLab);
				});

	if (sIt != store_end(g, wLab->getAddr()))
		reportError(wLab->getPos(), VerificationError::VE_InvalidBInit,
			    "Called barrier_init() multiple times!", &*sIt);
	else if (wLab->getVal() == SVal(0))
		reportError(wLab->getPos(), VerificationError::VE_InvalidBInit,
			    "Called barrier_init() with 0!");
	return;
}

void GenMCDriver::checkBIncValidity(const ReadLabel *rLab, const std::vector<Event> &rfs)
{
	auto *bLab = llvm::dyn_cast<BIncFaiReadLabel>(rLab);
	if (!bLab)
		return;

	if (std::any_of(rfs.cbegin(), rfs.cend(),
			[](const Event &rf) { return rf.isInitializer(); }))
		reportError(rLab->getPos(), VerificationError::VE_UninitializedMem,
			    "Called barrier_wait() on uninitialized barrier!");
	else if (std::any_of(rfs.cbegin(), rfs.cend(), [this, bLab](const Event &rf) {
			 auto rfVal =
				 getWriteValue(getGraph().getEventLabel(rf), bLab->getAccess());
			 return rfVal == SVal(0);
		 }))
		reportError(rLab->getPos(), VerificationError::VE_AccessFreed,
			    "Called barrier_wait() on destroyed barrier!", bLab->getRf());
}

void GenMCDriver::checkFinalAnnotations(const WriteLabel *wLab)
{
	if (!getConf()->helper)
		return;

	auto &g = getGraph();

	if (g.hasLocMoreThanOneStore(wLab->getAddr()))
		return;
	if ((wLab->isFinal() &&
	     std::any_of(store_begin(g, wLab->getAddr()), store_end(g, wLab->getAddr()),
			 [&](auto &sLab) { return !getHbView(wLab).contains(sLab.getPos()); })) ||
	    (!wLab->isFinal() &&
	     std::any_of(store_begin(g, wLab->getAddr()), store_end(g, wLab->getAddr()),
			 [&](auto &sLab) { return sLab.isFinal(); }))) {
		reportError(wLab->getPos(), VerificationError::VE_Annotation,
			    "Multiple stores at final location!");
		return;
	}
	return;
}

void GenMCDriver::checkIPRValidity(const ReadLabel *rLab)
{
	if (!rLab->getAnnot() || !getConf()->ipr)
		return;

	auto &g = getGraph();
	auto racyIt = std::find_if(store_begin(g, rLab->getAddr()), store_end(g, rLab->getAddr()),
				   [&](auto &wLab) { return wLab.hasAttr(WriteAttr::WWRacy); });
	if (racyIt != store_end(g, rLab->getAddr())) {
		auto msg = "Warning treated as an error due to in-place revisiting.\n"
			   "You can use -disable-ipr to disable this feature."s;
		reportError(racyIt->getPos(), VerificationError::VE_WWRace, msg, nullptr, true);
	}
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
	auto start = llvm::isa<SpinStartLabel>(lastLab) ? lastLab->getPos().prev()
							: lastLab->getPos();
	for (auto j = start.index; j > 0; j--) {
		auto *lab = g.getEventLabel(Event(tid, j));
		BUG_ON(llvm::isa<LoopBeginLabel>(lab));
		if (llvm::isa<SpinStartLabel>(lab))
			return true;
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (!isCoMaximal(rLab->getAddr(), rLab->getRf()->getPos()))
				return false;
		}
	}
	BUG();
}

void GenMCDriver::checkLiveness()
{
	if (isHalting())
		return;

	const auto &g = getGraph();

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
		    return threadReadsMaximal(tid);
	    })) {
		/* Print some TID blocked by a spinloop */
		reportError(g.getLastThreadEvent(nonTermTID), VerificationError::VE_Liveness,
			    "Non-terminating spinloop: thread " + std::to_string(nonTermTID));
	}
	return;
}

void GenMCDriver::filterConflictingBarriers(const ReadLabel *lab, std::vector<Event> &stores)
{
	if (getConf()->disableBAM || !llvm::isa<BIncFaiReadLabel>(lab))
		return;

	/* barrier_wait()'s FAI loads should not read from conflicting stores */
	auto &g = getGraph();
	stores.erase(std::remove_if(stores.begin(), stores.end(),
				    [&](const Event &s) {
					    return g.isStoreReadByExclusiveRead(s, lab->getAddr());
				    }),
		     stores.end());
	return;
}

int GenMCDriver::getSymmPredTid(int tid) const
{
	auto &g = getGraph();
	return g.getFirstThreadLabel(tid)->getSymmetricTid();
}

int GenMCDriver::getSymmSuccTid(int tid) const
{
	auto &g = getGraph();
	auto symm = tid;

	/* Check if there is anyone else symmetric to SYMM */
	for (auto i = tid + 1; i < g.getNumThreads(); i++)
		if (g.getFirstThreadLabel(i)->getSymmetricTid() == symm)
			return i;
	return -1; /* no one else */
}

bool GenMCDriver::isEcoBefore(const EventLabel *lab, int tid) const
{
	auto &g = getGraph();
	if (!llvm::isa<MemAccessLabel>(lab))
		return false;

	auto symmPos = Event(tid, lab->getIndex());
	// if (auto *wLab = rf_pred(g, lab); wLab) {
	// 	return wLab.getPos() == symmPos;
	// }))
	// 	return true;
	if (std::any_of(co_succ_begin(g, lab), co_succ_end(g, lab), [&](auto &sLab) {
		    return sLab.getPos() == symmPos ||
			   std::any_of(sLab.readers_begin(), sLab.readers_end(),
				       [&](auto &rLab) { return rLab.getPos() == symmPos; });
	    }))
		return true;
	if (std::any_of(fr_succ_begin(g, lab), fr_succ_end(g, lab), [&](auto &sLab) {
		    return sLab.getPos() == symmPos ||
			   std::any_of(sLab.readers_begin(), sLab.readers_end(),
				       [&](auto &rLab) { return rLab.getPos() == symmPos; });
	    }))
		return true;
	return false;
}

bool GenMCDriver::isEcoSymmetric(const EventLabel *lab, int tid) const
{
	auto &g = getGraph();

	auto *symmLab = g.getEventLabel(Event(tid, lab->getIndex()));
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
		return rLab->getRf() == llvm::dyn_cast<ReadLabel>(symmLab)->getRf();
	}

	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	BUG_ON(!wLab);
	return g.co_imm_succ(wLab) == llvm::dyn_cast<WriteLabel>(symmLab);
}

bool GenMCDriver::isPredSymmetryOK(const EventLabel *lab, int symm)
{
	auto &g = getGraph();

	BUG_ON(symm == -1);
	if (!sharePrefixSR(symm, lab->getPos()) || !g.containsPos(Event(symm, lab->getIndex())))
		return true;

	auto *symmLab = g.getEventLabel(Event(symm, lab->getIndex()));
	if (symmLab->getKind() != lab->getKind())
		return true;

	return !isEcoBefore(lab, symm);
}

bool GenMCDriver::isPredSymmetryOK(const EventLabel *lab)
{
	auto &g = getGraph();
	std::vector<int> preds;

	auto symm = getSymmPredTid(lab->getThread());
	while (symm != -1) {
		preds.push_back(symm);
		symm = getSymmPredTid(symm);
	}
	return std::all_of(preds.begin(), preds.end(),
			   [&](auto &symm) { return isPredSymmetryOK(lab, symm); });
}

bool GenMCDriver::isSuccSymmetryOK(const EventLabel *lab, int symm)
{
	auto &g = getGraph();

	BUG_ON(symm == -1);
	if (!sharePrefixSR(symm, lab->getPos()) || !g.containsPos(Event(symm, lab->getIndex())))
		return true;

	auto *symmLab = g.getEventLabel(Event(symm, lab->getIndex()));
	if (symmLab->getKind() != lab->getKind())
		return true;

	return !isEcoBefore(symmLab, lab->getThread());
}

bool GenMCDriver::isSuccSymmetryOK(const EventLabel *lab)
{
	auto &g = getGraph();
	std::vector<int> succs;

	auto symm = getSymmSuccTid(lab->getThread());
	while (symm != -1) {
		succs.push_back(symm);
		symm = getSymmSuccTid(symm);
	}
	return std::all_of(succs.begin(), succs.end(),
			   [&](auto &symm) { return isSuccSymmetryOK(lab, symm); });
}

bool GenMCDriver::isSymmetryOK(const EventLabel *lab)
{
	auto &g = getGraph();
	return isPredSymmetryOK(lab) && isSuccSymmetryOK(lab);
}

void GenMCDriver::updatePrefixWithSymmetriesSR(EventLabel *lab)
{
	auto t = getSymmPredTid(lab->getThread());
	if (t == -1)
		return;

	auto &v = lab->getPrefixView();
	auto si = calcLargestSymmPrefixBeforeSR(t, lab->getPos());
	auto *symmLab = getGraph().getEventLabel({t, si});
	v.update(getPrefixView(symmLab));
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(symmLab)) {
		v.update(getPrefixView(rLab->getRf()));
	}
}

int GenMCDriver::calcLargestSymmPrefixBeforeSR(int tid, Event pos) const
{
	auto &g = getGraph();

	if (tid < 0 || tid >= g.getNumThreads())
		return -1;

	auto limit = std::min((long)pos.index, (long)g.getThreadSize(tid) - 1);
	for (auto j = 0; j < limit; j++) {
		auto *labA = g.getEventLabel(Event(tid, j));
		auto *labB = g.getEventLabel(Event(pos.thread, j));

		if (labA->getKind() != labB->getKind())
			return j - 1;
		if (auto *rLabA = llvm::dyn_cast<ReadLabel>(labA)) {
			auto *rLabB = llvm::dyn_cast<ReadLabel>(labB);
			if (rLabA->getRf()->getThread() == tid &&
			    rLabB->getRf()->getThread() == pos.thread &&
			    rLabA->getRf()->getIndex() == rLabB->getRf()->getIndex())
				continue;
			if (rLabA->getRf() != rLabB->getRf())
				return j - 1;
		}
		if (auto *wLabA = llvm::dyn_cast<WriteLabel>(labA))
			if (!wLabA->isLocal())
				return j - 1;
	}
	return limit;
}

bool GenMCDriver::sharePrefixSR(int tid, Event pos) const
{
	return calcLargestSymmPrefixBeforeSR(tid, pos) == pos.index;
}

void GenMCDriver::filterSymmetricStoresSR(const ReadLabel *rLab, std::vector<Event> &stores) const
{
	auto &g = getGraph();
	auto t = getSymmPredTid(rLab->getThread());

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

	if (!g.isRMWLoad(lab))
		return;

	/* Remove stores that will be explored symmetrically */
	auto rfStamp = lab->getRf()->getStamp();
	stores.erase(std::remove_if(stores.begin(), stores.end(),
				    [&](auto s) { return lab->getRf()->getPos() == s; }),
		     stores.end());
	return;
}

void GenMCDriver::filterValuesFromAnnotSAVER(const ReadLabel *rLab, std::vector<Event> &validStores)
{
	/* Locks are treated as annotated CASes */
	if (!rLab->getAnnot())
		return;

	using Evaluator = SExprEvaluator<ModuleID::ID>;

	auto &g = getGraph();

	/* Ensure we keep the maximal store around even if Helper messed with it */
	BUG_ON(validStores.empty());
	auto maximal = validStores.back();
	validStores.erase(std::remove_if(validStores.begin(), validStores.end(),
					 [&](Event w) {
						 auto val = getWriteValue(g.getEventLabel(w),
									  rLab->getAccess());
						 return w != maximal &&
							!isCoMaximal(rLab->getAddr(), w, true) &&
							!Evaluator().evaluate(rLab->getAnnot(),
									      val);
					 }),
			  validStores.end());
	BUG_ON(validStores.empty());
}

void GenMCDriver::unblockWaitingHelping()
{
	/* FIXME: We have to wake up all threads waiting on helping CASes,
	 * as we don't know which ones are from the same CAS */
	for (auto i = 0u; i < getGraph().getNumThreads(); i++) {
		auto *bLab = llvm::dyn_cast<HelpedCASBlockLabel>(getGraph().getLastThreadLabel(i));
		if (bLab)
			getGraph().removeLast(bLab->getThread());
	}
}

bool GenMCDriver::writesBeforeHelpedContainedInView(const HelpedCasReadLabel *lab, const View &view)
{
	auto &g = getGraph();
	auto &hb = getHbView(lab);

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
	auto &g = getGraph();

	auto hs = g.collectAllEvents([&](const EventLabel *lab) {
		auto *rLab = llvm::dyn_cast<HelpedCasReadLabel>(lab);
		return rLab && g.isRMWLoad(rLab) && rLab->getAddr() == hLab->getAddr() &&
		       rLab->getType() == hLab->getType() && rLab->getSize() == hLab->getSize() &&
		       rLab->getOrdering() == hLab->getOrdering() &&
		       rLab->getExpected() == hLab->getExpected() &&
		       rLab->getSwapVal() == hLab->getSwapVal();
	});

	if (hs.empty())
		return false;

	if (std::any_of(hs.begin(), hs.end(), [&g, this](const Event &h) {
		    auto *hLab = llvm::dyn_cast<HelpedCasReadLabel>(g.getEventLabel(h));
		    auto &view = getHbView(hLab);
		    return !writesBeforeHelpedContainedInView(hLab, view);
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
	auto &g = getGraph();

	while (!rfs.empty()) {
		g.changeRf(rLab->getPos(), rfs.back());
		if (isExecutionValid(rLab))
			break;
		rfs.erase(rfs.end() - 1);
	}

	if (!rfs.empty())
		return true;

	BUG_ON(!getConf()->bound.has_value());
	moot();
	return false;
}

bool GenMCDriver::ensureConsistentStore(const WriteLabel *wLab)
{
	if (!checkAtomicity(wLab) || !isExecutionValid(wLab)) {
		moot();
		return false;
	}
	return true;
}

void GenMCDriver::filterInvalidRecRfs(const ReadLabel *rLab, std::vector<Event> &rfs)
{
	auto &g = getGraph();
	rfs.erase(std::remove_if(rfs.begin(), rfs.end(),
				 [&](Event &r) {
					 g.changeRf(rLab->getPos(), r);
					 return !isRecoveryValid(rLab);
				 }),
		  rfs.end());
	BUG_ON(rfs.empty());
	g.changeRf(rLab->getPos(), rfs[0]);
	return;
}

void GenMCDriver::handleThreadKill(std::unique_ptr<ThreadKillLabel> kLab)
{
	BUG_ON(isExecutionDrivenByGraph(&*kLab));
	addLabelToGraph(std::move(kLab));
	return;
}

bool GenMCDriver::isSymmetricToSR(int candidate, Event parent, const ThreadInfo &info) const
{
	auto &g = getGraph();
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

	auto &g = getGraph();

	for (auto i = childInfo.id - 1; i > 0; i--)
		if (isSymmetricToSR(i, tcLab->getPos(), childInfo))
			return i;
	return -1;
}

int GenMCDriver::handleThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab)
{
	auto &g = getGraph();
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
	auto symm = getSymmetricTidSR(lab, lab->getChildInfo());
	auto tsLab =
		ThreadStartLabel::create(Event(cid, 0), lab->getPos(), lab->getChildInfo(), symm);
	addLabelToGraph(std::move(tsLab));
	return cid;
}

std::optional<SVal> GenMCDriver::handleThreadJoin(std::unique_ptr<ThreadJoinLabel> lab)
{
	auto &g = getGraph();

	if (isExecutionDrivenByGraph(&*lab))
		return {getJoinValue(
			llvm::dyn_cast<ThreadJoinLabel>(g.getEventLabel(lab->getPos())))};

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
		reportError(jLab->getPos(), VerificationError::VE_InvalidJoin, err);
		return {SVal(0)};
	}

	if (partialExecutionExceedsBound()) {
		moot();
		return std::nullopt;
	}

	return {getJoinValue(jLab)};
}

void GenMCDriver::handleThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab)
{
	auto &g = getGraph();

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

void GenMCDriver::handleFenceLKMM(std::unique_ptr<FenceLabel> fLab)
{
	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
}

void GenMCDriver::handleFence(std::unique_ptr<FenceLabel> fLab)
{
	if (llvm::isa<SmpFenceLabelLKMM>(&*fLab)) {
		handleFenceLKMM(std::move(fLab));
		return;
	}

	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::handleCLFlush(std::unique_ptr<CLFlushLabel> fLab)
{
	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::checkReconsiderFaiSpinloop(const MemAccessLabel *lab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);

		/* Is there any thread blocked on a potential spinloop? */
		auto *eLab = llvm::dyn_cast<FaiZNEBlockLabel>(g.getLastThreadLabel(i));
		if (!eLab)
			continue;

		/* Check whether this access affects the spinloop variable */
		auto *faiLab = llvm::dyn_cast<FaiWriteLabel>(g.getPreviousLabelST(
			eLab, [](const EventLabel *lab) { return llvm::isa<FaiWriteLabel>(lab); }));
		if (faiLab->getAddr() != lab->getAddr())
			continue;
		/* FAIs on the same variable are OK... */
		if (llvm::isa<FaiReadLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
			continue;

		/* If it does, and also breaks the assumptions, unblock thread */
		if (!getHbView(faiLab).contains(lab->getPos())) {
			auto pos = eLab->getPos();
			unblockThread(pos);
			addLabelToGraph(FaiZNESpinEndLabel::create(pos));
		}
	}
	return;
}

std::vector<Event> GenMCDriver::getRfsApproximation(const ReadLabel *lab)
{
	auto rfs = getCoherentStores(lab->getAddr(), lab->getPos());
	if (!llvm::isa<CasReadLabel>(lab) && !llvm::isa<FaiReadLabel>(lab))
		return rfs;

	/* Remove atomicity violations */
	auto &g = getGraph();
	auto &before = getPrefixView(lab);
	rfs.erase(std::remove_if(
			  rfs.begin(), rfs.end(),
			  [&](const Event &s) {
				  auto oldVal = getWriteValue(g.getEventLabel(s), lab->getAccess());
				  if (llvm::isa<FaiReadLabel>(lab) &&
				      g.isStoreReadBySettledRMW(s, lab->getAddr(), before))
					  return true;
				  if (auto *rLab = llvm::dyn_cast<CasReadLabel>(lab)) {
					  if (oldVal == rLab->getExpected() &&
					      g.isStoreReadBySettledRMW(s, rLab->getAddr(), before))
						  return true;
				  }
				  return false;
			  }),
		  rfs.end());
	return rfs;
}

bool GenMCDriver::filterOptimizeRfs(const ReadLabel *lab, std::vector<Event> &stores)
{
	/* Symmetry reduction */
	if (getConf()->symmetryReduction)
		filterSymmetricStoresSR(lab, stores);

	/* BAM */
	if (!getConf()->disableBAM)
		filterConflictingBarriers(lab, stores);

	/* Keep values that do not lead to blocking */
	filterValuesFromAnnotSAVER(lab, stores);

	if (!isRescheduledRead(lab->getPos())) {
		if (removeCASReadIfBlocks(lab, getGraph().getEventLabel(stores.back())))
			return false;
	}
	return true;
}

void GenMCDriver::filterAtomicityViolations(const ReadLabel *rLab, std::vector<Event> &stores)
{
	auto &g = getGraph();
	if (!llvm::isa<CasReadLabel>(rLab) && !llvm::isa<FaiReadLabel>(rLab))
		return;

	const auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab);
	auto valueMakesSuccessfulRMW = [&casLab, rLab](auto &&val) {
		return !casLab || val == casLab->getExpected();
	};
	stores.erase(std::remove_if(
			     stores.begin(), stores.end(),
			     [&](auto &s) {
				     auto *sLab = g.getEventLabel(s);
				     if (auto *iLab = llvm::dyn_cast<InitLabel>(sLab))
					     return std::any_of(
						     iLab->rf_begin(rLab->getAddr()),
						     iLab->rf_end(rLab->getAddr()),
						     [&](auto &rLab) {
							     return g.isRMWLoad(&rLab) &&
								    valueMakesSuccessfulRMW(
									    getReadValue(&rLab));
						     });
				     return std::any_of(rf_succ_begin(g, sLab),
							rf_succ_end(g, sLab), [&](auto &rLab) {
								return g.isRMWLoad(&rLab) &&
								       valueMakesSuccessfulRMW(
									       getReadValue(&rLab));
							});
			     }),
		     stores.end());
}

void GenMCDriver::updateStSpaceChoices(const ReadLabel *rLab, const std::vector<Event> &stores)
{
	auto &choices = getChoiceMap();
	choices[rLab->getStamp()] = stores;
}

std::optional<SVal> GenMCDriver::pickRandomRf(ReadLabel *rLab, std::vector<Event> &stores)
{
	auto &g = getGraph();

	stores.erase(std::remove_if(stores.begin(), stores.end(),
				    [&](auto &s) {
					    g.changeRf(rLab->getPos(), s);
					    return !isExecutionValid(rLab);
				    }),
		     stores.end());

	MyDist dist(0, stores.size() - 1);
	auto random = dist(estRng);
	g.changeRf(rLab->getPos(), stores[random]);

	if (readsUninitializedMem(rLab)) {
		reportError(rLab->getPos(), VerificationError::VE_UninitializedMem);
		return std::nullopt;
	}
	return getWriteValue(rLab->getRf(), rLab->getAccess());
}

std::optional<SVal> GenMCDriver::handleLoad(std::unique_ptr<ReadLabel> rLab)
{
	auto &g = getGraph();
	auto *EE = getEE();

	if (inRecoveryMode() && rLab->getAddr().isVolatile())
		return {getRecReadRetValue(rLab.get())};

	if (isExecutionDrivenByGraph(&*rLab))
		return getReadRetValue(llvm::dyn_cast<ReadLabel>(g.getEventLabel(rLab->getPos())));

	/* First, we have to check whether the access is valid. This has to
	 * happen here because we may query the interpreter for this location's
	 * value in order to determine whether this load is going to be an RMW.
	 * Coherence needs to be tracked before validity is established, as
	 * consistency checks may be triggered if the access is invalid */
	g.trackCoherenceAtLoc(rLab->getAddr());

	if (!rLab->getAnnot())
		rLab->setAnnot(EE->getCurrentAnnotConcretized());
	cacheEventLabel(&*rLab);
	auto *lab = llvm::dyn_cast<ReadLabel>(addLabelToGraph(std::move(rLab)));

	if (!isAccessValid(lab)) {
		reportError(lab->getPos(), VerificationError::VE_AccessNonMalloc);
		return std::nullopt; /* This execution will be blocked */
	}
	g.addAlloc(findAllocatingLabel(g, lab->getAddr()), lab);

	if (checkForRaces(lab) != VerificationError::VE_OK)
		return std::nullopt;

	/* Get an approximation of the stores we can read from */
	auto stores = getRfsApproximation(lab);
	BUG_ON(stores.empty());
	GENMC_DEBUG(LOG(VerbosityLevel::Debug3) << "Rfs: " << format(stores) << "\n";);

	/* Try to minimize the number of rfs */
	if (!filterOptimizeRfs(lab, stores))
		return std::nullopt;

	/* ... add an appropriate label with a random rf */
	g.changeRf(lab->getPos(), stores.back());
	GENMC_DEBUG(LOG(VerbosityLevel::Debug3) << "Rfs (optimized): " << format(stores) << "\n";);

	/* ... and make sure that the rf we end up with is consistent and respects the bound */
	if (!ensureConsistentRf(lab, stores))
		return std::nullopt;

	if (readsUninitializedMem(lab)) {
		reportError(lab->getPos(), VerificationError::VE_UninitializedMem);
		return std::nullopt;
	}

	/* If this is the last part of barrier_wait() check whether we should block */
	auto retVal = getWriteValue(g.getEventLabel(stores.back()), lab->getAccess());
	if (llvm::isa<BWaitReadLabel>(lab) && retVal != getBarrierInitValue(lab->getAccess())) {
		blockThread(BarrierBlockLabel::create(lab->getPos().next()));
		if (!getConf()->disableBAM)
			return {retVal};
	}

	if (isRescheduledRead(lab->getPos()))
		setRescheduledRead(Event::getInit());
	if (lab->getAnnot())
		checkIPRValidity(lab);

	if (inEstimationMode()) {
		updateStSpaceChoices(lab, stores);
		filterAtomicityViolations(lab, stores);
		return pickRandomRf(lab, stores);
	}

	GENMC_DEBUG(LOG(VerbosityLevel::Debug2) << "--- Added load " << lab->getPos() << "\n"
						<< getGraph(););

	/* Check whether the load forces us to reconsider some existing event */
	checkReconsiderFaiSpinloop(lab);

	/* Check for races and reading from uninitialized memory */
	if (llvm::isa<LockCasReadLabel>(lab))
		checkLockValidity(lab, stores);
	if (llvm::isa<BIncFaiReadLabel>(lab))
		checkBIncValidity(lab, stores);

	/* Push all the other alternatives choices to the Stack (many maximals for wb) */
	std::for_each(stores.begin(), stores.end() - 1, [&](const Event &s) {
		auto status = false; /* MO messes with the status */
		addToWorklist(lab->getStamp(),
			      std::make_unique<ReadForwardRevisit>(lab->getPos(), s, status));
	});
	return {retVal};
}

void GenMCDriver::annotateStoreHELPER(WriteLabel *wLab)
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
}

std::vector<Event> GenMCDriver::getRevisitableApproximation(const WriteLabel *sLab)
{
	auto &g = getGraph();
	auto &prefix = getPrefixView(sLab);
	auto loads = getCoherentRevisits(sLab, prefix);
	std::sort(loads.begin(), loads.end(), [&g](const Event &l1, const Event &l2) {
		return g.getEventLabel(l1)->getStamp() > g.getEventLabel(l2)->getStamp();
	});
	return loads;
}

void GenMCDriver::pickRandomCo(WriteLabel *sLab,
			       const llvm::iterator_range<ExecutionGraph::co_iterator> &placesRange)
{
	auto &g = getGraph();

	MyDist dist(0, std::distance(placesRange.begin(), placesRange.end()));
	auto random = dist(estRng);
	g.addStoreToCO(sLab, std::next(placesRange.begin(), (long long)random));
}

void GenMCDriver::updateStSpaceChoices(const WriteLabel *wLab, const std::vector<Event> &stores)
{
	auto &choices = getChoiceMap();
	choices[wLab->getStamp()] = stores;
}

void GenMCDriver::calcCoOrderings(WriteLabel *lab)
{
	/* Find all possible placings in coherence for this store.
	 * If appropriate, print a WW-race warning (if this moots, exploration will anyway be cut).
	 * Printing happens after choices are updated, to not invalidate iterators */
	auto &g = getGraph();
	auto placesRange = getCoherentPlacings(lab->getAddr(), lab->getPos(), g.isRMWStore(lab));
	auto *racyWrite = placesRange.begin() != placesRange.end() ? &*placesRange.begin()
								   : nullptr;

	if (inEstimationMode()) {
		std::vector<Event> cos;
		std::transform(placesRange.begin(), placesRange.end(), std::back_inserter(cos),
			       [&](auto &lab) { return lab.getPos(); });
		cos.push_back(Event::getBottom());
		pickRandomCo(lab, placesRange);
		updateStSpaceChoices(lab, cos);
		if (racyWrite)
			reportWarningOnce(lab->getPos(), VerificationError::VE_WWRace, racyWrite);
		return;
	}

	/* We cannot place the write just before the write of an RMW or during recovery */
	for (auto &succLab : placesRange) {
		if (!g.isRMWStore(succLab.getPos()) && !inRecoveryMode())
			addToWorklist(lab->getStamp(), std::make_unique<WriteForwardRevisit>(
							       lab->getPos(), succLab.getPos()));
	}
	g.addStoreToCO(lab, placesRange.end());
	if (racyWrite)
		reportWarningOnce(lab->getPos(), VerificationError::VE_WWRace, racyWrite);
}

void GenMCDriver::handleStore(std::unique_ptr<WriteLabel> wLab)
{
	if (isExecutionDrivenByGraph(&*wLab))
		return;

	auto &g = getGraph();

	/* If it's a valid access, track coherence for this location */
	g.trackCoherenceAtLoc(wLab->getAddr());

	if (getConf()->helper && g.isRMWStore(&*wLab))
		annotateStoreHELPER(&*wLab);

	auto *lab = llvm::dyn_cast<WriteLabel>(addLabelToGraph(std::move(wLab)));

	if (!isAccessValid(lab)) {
		reportError(lab->getPos(), VerificationError::VE_AccessNonMalloc);
		return;
	}
	g.addAlloc(findAllocatingLabel(g, lab->getAddr()), lab);

	/* It is always consistent to add the store at the end of MO */
	if (llvm::isa<BIncFaiWriteLabel>(lab) && lab->getVal() == SVal(0))
		lab->setVal(getBarrierInitValue(lab->getAccess()));

	calcCoOrderings(lab);

	/* If the graph is not consistent (e.g., w/ LAPOR) stop the exploration */
	bool cons = ensureConsistentStore(lab);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug2) << "--- Added store " << lab->getPos() << "\n"
						<< getGraph(););

	if (cons && checkForRaces(lab) != VerificationError::VE_OK)
		return;

	if (!inRecoveryMode() && !inReplay())
		calcRevisits(lab);

	if (!cons)
		return;

	checkReconsiderFaiSpinloop(lab);
	if (llvm::isa<HelpedCasWriteLabel>(lab))
		unblockWaitingHelping();
	checkReconsiderReadOpts(lab);

	/* Check for races */
	if (llvm::isa<UnlockWriteLabel>(lab))
		checkUnlockValidity(lab);
	if (llvm::isa<BInitWriteLabel>(lab))
		checkBInitValidity(lab);
	checkFinalAnnotations(lab);
}

void GenMCDriver::handleHpProtect(std::unique_ptr<HpProtectLabel> hpLab)
{
	if (isExecutionDrivenByGraph(&*hpLab))
		return;

	addLabelToGraph(std::move(hpLab));
}

SVal GenMCDriver::handleMalloc(std::unique_ptr<MallocLabel> aLab)
{
	auto &g = getGraph();

	if (isExecutionDrivenByGraph(&*aLab)) {
		auto *lab = llvm::dyn_cast<MallocLabel>(g.getEventLabel(aLab->getPos()));
		BUG_ON(!lab);
		return SVal(lab->getAllocAddr().get());
	}

	/* Fix and add label to the graph; return the new address */
	if (aLab->getAllocAddr() == SAddr())
		aLab->setAllocAddr(getFreshAddr(&*aLab));
	cacheEventLabel(&*aLab);
	auto *lab = llvm::dyn_cast<MallocLabel>(addLabelToGraph(std::move(aLab)));
	return SVal(lab->getAllocAddr().get());
}

void GenMCDriver::handleFree(std::unique_ptr<FreeLabel> dLab)
{
	auto &g = getGraph();

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

void GenMCDriver::handleRCULockLKMM(std::unique_ptr<RCULockLabelLKMM> lLab)
{
	if (isExecutionDrivenByGraph(&*lLab))
		return;

	addLabelToGraph(std::move(lLab));
}

void GenMCDriver::handleRCUUnlockLKMM(std::unique_ptr<RCUUnlockLabelLKMM> uLab)
{
	if (isExecutionDrivenByGraph(&*uLab))
		return;

	addLabelToGraph(std::move(uLab));
}

void GenMCDriver::handleRCUSyncLKMM(std::unique_ptr<RCUSyncLabelLKMM> fLab)
{
	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
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
	auto &g = getGraph();

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

	auto &g = getGraph();
	blockThreadTryMoot(std::move(lab));
}

std::unique_ptr<VectorClock> GenMCDriver::getReplayView() const
{
	auto &g = getGraph();
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

void GenMCDriver::reportError(Event pos, VerificationError s, const std::string &err /* = "" */,
			      const EventLabel *racyLab /* = nullptr */,
			      bool shouldHalt /* = true */)
{
	auto &g = getGraph();

	/* If we have already detected an error, no need to report another */
	if (isHalting())
		return;

	/* If we this is a replay (might happen if one LLVM instruction
	 * maps to many MC events), do not get into an infinite loop... */
	if (inReplay())
		return;

	/* Ignore soft errors under estimation mode.
	 * These are going to be reported later on anyway */
	if (!shouldHalt && inEstimationMode())
		return;

	/* If this is an invalid access, change the RF of the offending
	 * event to BOTTOM, so that we do not try to get its value.
	 * Don't bother updating the views */
	auto *errLab = g.getEventLabel(pos);
	if (isInvalidAccessError(s) && llvm::isa<ReadLabel>(errLab))
		g.changeRf(errLab->getPos(), Event::getBottom());

	/* Print a basic error message and the graph.
	 * We have to save the interpreter state as replaying will
	 * destroy the current execution stack */
	auto iState = getEE()->saveState();

	getEE()->replayExecutionBefore(*getReplayView());

	llvm::raw_string_ostream out(result.message);

	out << (isHardError(s) ? "Error: " : "Warning: ") << s << "!\n";
	out << "Event " << errLab->getPos() << " ";
	if (racyLab != nullptr)
		out << "conflicts with event " << racyLab->getPos() << " ";
	out << "in graph:\n";
	printGraph(true, out);

	/* Print error trace leading up to the violating event(s) */
	if (getConf()->printErrorTrace) {
		printTraceBefore(errLab, out);
		if (racyLab != nullptr)
			printTraceBefore(racyLab, out);
	}

	/* Print the specific error message */
	if (!err.empty())
		out << err << "\n";

	/* Dump the graph into a file (DOT format) */
	if (!getConf()->dotFile.empty())
		dotPrintToFile(getConf()->dotFile, errLab, racyLab);

	getEE()->restoreState(std::move(iState));

	if (shouldHalt)
		halt(s);
}

bool GenMCDriver::reportWarningOnce(Event pos, VerificationError wcode,
				    const EventLabel *racyLab /* = nullptr */)
{
	/* Helper function to determine whether the warning should be treated as an error */
	auto isHardError = [&](auto &wcode) {
		if (wcode != VerificationError::VE_WWRace)
			return std::make_pair(false, ""s);
		if (!getConf()->symmetryReduction && !getConf()->ipr)
			return std::make_pair(false, ""s);

		auto &g = getGraph();
		auto *lab = g.getEventLabel(pos);
		auto hardError =
			(getConf()->symmetryReduction &&
			 std::any_of(g.getThreadList().begin(), g.getThreadList().end(),
				     [&](auto &thr) {
					     return llvm::dyn_cast<ThreadStartLabel>(
							    thr.begin()->get())
							    ->getSymmetricTid() != -1;
				     })) ||
			(getConf()->ipr &&
			 std::any_of(sameloc_begin(g, lab), sameloc_end(g, lab), [&](auto &oLab) {
				 auto *rLab = llvm::dyn_cast<ReadLabel>(&oLab);
				 return rLab && rLab->getAnnot();
			 }));
		auto [cause, cli] =
			getConf()->ipr ? std::make_pair("in-place revisiting"s, "-disable-ipr"s)
				       : std::make_pair("symmetry reduction"s, "-disable-sr"s);
		auto msg = hardError ? ("Warning treated as an error due to " + cause +
					".\n"
					"You can use " +
					cli + " to disable these features."s)
				     : ""s;
		return std::make_pair(hardError, msg);
	};

	/* If the warning has been seen before, only report it if it's an error */
	auto [hardError, msg] = isHardError(wcode);
	auto &knownWarnings = getResult().warnings;
	if (hardError || knownWarnings.count(wcode) == 0) {
		reportError(pos, wcode, msg, racyLab, hardError);
	}
	if (knownWarnings.count(wcode) == 0)
		knownWarnings.insert(wcode);
	if (wcode == VerificationError::VE_WWRace)
		getGraph().getWriteLabel(pos)->setAttr(WriteAttr::WWRacy);
	return hardError;
}

bool GenMCDriver::tryOptimizeBarrierRevisits(const BIncFaiWriteLabel *sLab,
					     std::vector<Event> &loads)
{
	if (getConf()->disableBAM)
		return false;

	/* If the barrier_wait() does not write the initial value, nothing to do */
	auto iVal = getBarrierInitValue(sLab->getAccess());
	if (sLab->getVal() != iVal)
		return true;

	/* Otherwise, revisit in place */
	auto &g = getGraph();
	auto bs = g.collectAllEvents([&](const EventLabel *lab) {
		if (!llvm::isa<BarrierBlockLabel>(lab))
			return false;
		auto *pLab =
			llvm::dyn_cast<BIncFaiWriteLabel>(g.getPreviousLabel(lab->getPos().prev()));
		return pLab->getAddr() == sLab->getAddr();
	});
	auto unblockedLoads = std::count_if(loads.begin(), loads.end(), [&](auto &l) {
		auto *nLab = llvm::dyn_cast_or_null<BlockLabel>(g.getNextLabel(l));
		return !nLab;
	});
	if (bs.size() > iVal.get() || unblockedLoads > 0)
		WARN_ONCE("bam-well-formed", "Execution not barrier-well-formed!\n");

	std::for_each(bs.begin(), bs.end(), [&](const Event &b) {
		auto *pLab = llvm::dyn_cast<BIncFaiWriteLabel>(g.getPreviousLabel(b.prev()));
		BUG_ON(!pLab);
		unblockThread(b);
		g.removeLast(b.thread);
		auto *rLab = llvm::dyn_cast<ReadLabel>(addLabelToGraph(
			BWaitReadLabel::create(b.prev(), pLab->getOrdering(), pLab->getAddr(),
					       pLab->getSize(), pLab->getType(), pLab->getDeps())));
		g.changeRf(rLab->getPos(), sLab->getPos());
		rLab->setAddedMax(isCoMaximal(rLab->getAddr(), rLab->getRf()->getPos()));
	});
	return true;
}

void GenMCDriver::tryOptimizeIPRs(const WriteLabel *sLab, std::vector<Event> &loads)
{
	if (!getConf()->ipr)
		return;

	auto &g = getGraph();

	std::vector<Event> toIPR;
	loads.erase(std::remove_if(loads.begin(), loads.end(),
				   [&](auto &l) {
					   auto *rLab = g.getReadLabel(l);
					   /* Treatment of blocked CASes is different */
					   auto blocked =
						   !llvm::isa<CasReadLabel>(rLab) &&
						   willBeAssumeBlocked(rLab, getReadValue(rLab));
					   if (blocked)
						   toIPR.push_back(l);
					   return blocked;
				   }),
		    loads.end());

	for (auto &l : toIPR)
		revisitInPlace(*constructBackwardRevisit(g.getReadLabel(l), sLab));

	/* We also have to filter out some regular revisits */
	auto pending = g.getPendingRMW(sLab);
	if (!pending.isInitializer()) {
		loads.erase(std::remove_if(loads.begin(), loads.end(),
					   [&](auto &l) {
						   auto *rLab = g.getReadLabel(l);
						   auto *rfLab = rLab->getRf();
						   return rLab->getAnnot() && // must be like that
							  rfLab->getStamp() > rLab->getStamp() &&
							  !getPrefixView(sLab).contains(
								  rfLab->getPos());
					   }),
			    loads.end());
	}

	return;
}

bool GenMCDriver::removeCASReadIfBlocks(const ReadLabel *rLab, const EventLabel *sLab)
{
	if (isUninitializedAccess(rLab->getAddr(), sLab->getPos()) ||
	    getConf()->bound.has_value() || !llvm::isa<CasReadLabel>(rLab))
		return false;

	auto val = getWriteValue(sLab, rLab->getAccess());
	if (!willBeAssumeBlocked(rLab, val))
		return false;

	auto &g = getGraph();
	auto pos = rLab->getPos();
	auto addr = rLab->getAddr();
	g.removeLast(pos.thread);
	blockThread(ReadOptBlockLabel::create(pos, addr));
	return true;
}

void GenMCDriver::checkReconsiderReadOpts(const WriteLabel *sLab)
{
	auto &g = getGraph();
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *bLab = llvm::dyn_cast<ReadOptBlockLabel>(g.getLastThreadLabel(i));
		if (!bLab || bLab->getAddr() != sLab->getAddr())
			continue;
		unblockThread(bLab->getPos());
	}
}

void GenMCDriver::optimizeUnconfirmedRevisits(const WriteLabel *sLab, std::vector<Event> &loads)
{
	if (!getConf()->helper)
		return;

	auto &g = getGraph();

	/* If there is already a write with the same value, report a possible ABA */
	auto valid = std::count_if(
		store_begin(g, sLab->getAddr()), store_end(g, sLab->getAddr()), [&](auto &wLab) {
			return wLab.getPos() != sLab->getPos() && wLab.getVal() == sLab->getVal();
		});
	if (sLab->getAddr().isStatic() &&
	    getWriteValue(g.getEventLabel(Event::getInit()), sLab->getAccess()) == sLab->getVal())
		++valid;
	WARN_ON_ONCE(valid > 0, "helper-aba-found",
		     "Possible ABA pattern! Consider running without -helper.\n");

	/* Do not bother with revisits that will be unconfirmed/lead to ABAs */
	loads.erase(
		std::remove_if(loads.begin(), loads.end(),
			       [&](const Event &l) {
				       auto *lab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(l));
				       if (!g.isConfirming(lab))
					       return false;

				       auto sc = Event::getInit();
				       auto *pLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(
					       g.getMatchingSpeculativeRead(lab->getPos(), &sc)));
				       ERROR_ON(!pLab, "Confirming CAS annotation error! "
						       "Does a speculative read precede the "
						       "confirming operation?\n");

				       return sc.isInitializer();
			       }),
		loads.end());
}

bool GenMCDriver::isConflictingNonRevBlocker(const EventLabel *pLab, const WriteLabel *sLab,
					     const Event &s)
{
	auto &g = getGraph();
	auto *sLab2 = llvm::dyn_cast<WriteLabel>(g.getEventLabel(s));
	if (sLab2->getPos() == sLab->getPos() || !g.isRMWStore(sLab2))
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

bool GenMCDriver::tryOptimizeRevBlockerAddition(const WriteLabel *sLab, std::vector<Event> &loads)
{
	if (!sLab->hasAttr(WriteAttr::RevBlocker))
		return false;

	auto &g = getGraph();
	auto *pLab = getPreviousVisibleAccessLabel(sLab->getPos().prev());
	if (std::find_if(store_begin(g, sLab->getAddr()), store_end(g, sLab->getAddr()),
			 [this, pLab, sLab](auto &lab) {
				 return isConflictingNonRevBlocker(pLab, sLab, lab.getPos());
			 }) != store_end(g, sLab->getAddr())) {
		moot();
		loads.clear();
		return true;
	}
	return false;
}

bool GenMCDriver::tryOptimizeRevisits(const WriteLabel *sLab, std::vector<Event> &loads)
{
	auto &g = getGraph();

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

bool GenMCDriver::willBeAssumeBlocked(const ReadLabel *rLab, SVal val)
{
	auto &g = getGraph();
	using Evaluator = SExprEvaluator<ModuleID::ID>;

	/* CAS annotations are only respected when ipr is enabled  */
	return (getConf()->ipr || llvm::isa<LockCasReadLabel>(rLab)) && rLab->getAnnot() &&
	       !Evaluator().evaluate(rLab->getAnnot(), val);
}

void GenMCDriver::revisitInPlace(const BackwardRevisit &br)
{
	BUG_ON(getConf()->bound.has_value());

	auto &g = getGraph();
	auto *rLab = g.getReadLabel(br.getPos());
	const auto *sLab = g.getWriteLabel(br.getRev());

	BUG_ON(!llvm::isa<ReadLabel>(rLab));
	if (g.getNextLabel(rLab))
		g.removeLast(rLab->getThread());
	g.changeRf(rLab->getPos(), sLab->getPos());
	rLab->setAddedMax(true); // always true for atomicity violations
	rLab->setIPRStatus(true);

	completeRevisitedRMW(rLab);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug1) << "--- In-place revisiting " << rLab->getPos()
						<< " <-- " << sLab->getPos() << "\n"
						<< getGraph(););

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
			if (g.isRMWStore(lab) && pporf.contains(lab->getPos().prev()))
				predsD.removeHole(lab->getPos());
		}
	}
	return;
}

std::unique_ptr<VectorClock>
GenMCDriver::getRevisitView(const ReadLabel *rLab, const WriteLabel *sLab,
			    const WriteLabel *midLab /* = nullptr */) const
{
	auto &g = getGraph();
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

	auto &g = getGraph();

	/* Check whether there is a conflicting RevBlocker */
	auto pending = g.getPendingRMW(sLab);
	auto *pLab = llvm::dyn_cast_or_null<WriteLabel>(g.getNextLabel(pending));
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

bool GenMCDriver::prefixContainsMatchingLock(const BackwardRevisit &r, const EventLabel *lab)
{
	if (!llvm::isa<UnlockWriteLabel>(lab))
		return false;
	auto l = getGraph().getMatchingLock(lab->getPos());
	if (l.isInitializer())
		return false;
	if (getPrefixView(getGraph().getWriteLabel(r.getRev())).contains(l))
		return true;
	if (auto *br = llvm::dyn_cast<BackwardRevisitHELPER>(&r))
		return getPrefixView(getGraph().getWriteLabel(br->getMid())).contains(l);
	return false;
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

	if (g.isRMWLoad(rLabB)) {
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
	auto &g = getGraph();
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

	auto &g = getGraph();
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
	auto &g = getGraph();
	auto *wLab = g.getWriteLabel(r.getRev());
	if (g.isRMWStore(wLab))
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

	auto &g = getGraph();
	auto &v = r.getViewNoRel();

	for (const auto &lab : labels(g)) {
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
	auto &g = getGraph();
	auto &v = r.getViewNoRel();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		if (v->getMax(i) + 1 != (long)g.getThreadSize(i) &&
		    !g.getEventLabel(Event(i, v->getMax(i) + 1))->isTerminator())
			return true;
		if (!getConf()->isDepTrackingModel)
			continue;
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (!v->contains(lab->getPos()) && !llvm::isa<EmptyLabel>(lab) &&
			    !lab->isTerminator())
				return true;
		}
	}
	return false;
}

std::unique_ptr<ExecutionGraph> GenMCDriver::copyGraph(const BackwardRevisit *br,
						       VectorClock *v) const
{
	auto &g = getGraph();

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

	og->compressStampsAfter(revLab->getStamp());
	for (auto &lab : labels(*og)) {
		if (prefix.contains(lab.getPos()))
			lab.setRevisitStatus(false);
	}
	return og;
}

GenMCDriver::ChoiceMap GenMCDriver::createChoiceMapForCopy(const ExecutionGraph &og) const
{
	const auto &g = getGraph();
	const auto &choices = getChoiceMap();
	ChoiceMap result;

	for (auto &lab : labels(g)) {
		if (!og.containsPos(lab.getPos()) || !choices.count(lab.getStamp()))
			continue;

		auto oldStamp = lab.getStamp();
		auto newStamp = og.getEventLabel(lab.getPos())->getStamp();
		for (const auto &e : choices.at(oldStamp)) {
			if (og.containsPos(e))
				result[newStamp.get()].insert(e);
		}
	}
	return result;
}

bool GenMCDriver::checkRevBlockHELPER(const WriteLabel *sLab, const std::vector<Event> &loads)
{
	if (!getConf()->helper || !sLab->hasAttr(WriteAttr::RevBlocker))
		return true;

	auto &g = getGraph();
	if (std::any_of(loads.begin(), loads.end(), [this, &g, sLab](const Event &l) {
		    auto *lLab = g.getLastThreadLabel(l.thread);
		    auto *pLab = getPreviousVisibleAccessLabel(lLab->getPos());
		    return llvm::isa<BlockLabel>(lLab) && pLab && pLab->getPos() == l;
	    })) {
		moot();
		return false;
	}
	return true;
}

void GenMCDriver::updateStSpaceChoices(const std::vector<Event> &loads, const WriteLabel *sLab)
{
	auto &g = getGraph();
	auto &choices = getChoiceMap();
	for (const auto &l : loads) {
		const auto *rLab = g.getReadLabel(l);
		choices[rLab->getStamp()].insert(sLab->getPos());
	}
}

bool GenMCDriver::calcRevisits(const WriteLabel *sLab)
{
	auto &g = getGraph();
	auto loads = getRevisitableApproximation(sLab);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug3) << "Revisitable: " << format(loads) << "\n";);
	if (tryOptimizeRevisits(sLab, loads))
		return true;

	/* If operating in estimation mode, don't actually revisit */
	if (inEstimationMode()) {
		updateStSpaceChoices(loads, sLab);
		return checkAtomicity(sLab) && checkRevBlockHELPER(sLab, loads) && !isMoot();
	}

	GENMC_DEBUG(LOG(VerbosityLevel::Debug3)
			    << "Revisitable (optimized): " << format(loads) << "\n";);
	for (auto &l : loads) {
		auto *rLab = g.getReadLabel(l);
		BUG_ON(!rLab);

		auto br = constructBackwardRevisit(rLab, sLab);
		if (!isMaximalExtension(*br))
			break;

		addToWorklist(sLab->getStamp(), std::move(br));
	}

	return checkAtomicity(sLab) && checkRevBlockHELPER(sLab, loads) && !isMoot();
}

WriteLabel *GenMCDriver::completeRevisitedRMW(const ReadLabel *rLab)
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
			result = getBarrierInitValue(rLab->getAccess());
		wattr = faiLab->getAttr();
	} else if (auto *casLab = llvm::dyn_cast<CasReadLabel>(rLab)) {
		result = casLab->getSwapVal();
		wattr = casLab->getAttr();
	} else
		BUG();

	auto &g = getGraph();
	std::unique_ptr<WriteLabel> wLab = nullptr;

#define CREATE_COUNTERPART(name)                                                                   \
	case EventLabel::EL_##name##Read:                                                          \
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
	cacheEventLabel(&*wLab);
	auto *lab = llvm::dyn_cast<WriteLabel>(addLabelToGraph(std::move(wLab)));
	BUG_ON(!rLab->getRf());
	if (auto *rfLab = llvm::dyn_cast<WriteLabel>(rLab->getRf())) {
		g.addStoreToCO(lab, ExecutionGraph::co_iterator(g.co_succ_begin(rfLab)));
	} else {
		g.addStoreToCO(lab, g.co_begin(lab->getAddr()));
	}
	g.addAlloc(findAllocatingLabel(g, lab->getAddr()), lab);
	return lab;
}

bool GenMCDriver::revisitWrite(const WriteForwardRevisit &ri)
{
	auto &g = getGraph();
	auto *wLab = g.getWriteLabel(ri.getPos());
	BUG_ON(!wLab);

	g.removeStoreFromCO(wLab);
	g.addStoreToCO(wLab, ExecutionGraph::co_iterator(g.getWriteLabel(ri.getSucc())));
	wLab->setAddedMax(false);
	return calcRevisits(wLab);
}

bool GenMCDriver::revisitOptional(const OptionalForwardRevisit &oi)
{
	auto &g = getGraph();
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
	auto &g = getGraph();
	auto *rLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(ri.getPos()));
	auto rev = llvm::dyn_cast<ReadRevisit>(&ri)->getRev();
	BUG_ON(!rLab);

	g.changeRf(rLab->getPos(), rev);
	auto *fri = llvm::dyn_cast<ReadForwardRevisit>(&ri);
	rLab->setAddedMax(fri ? fri->isMaximal() : isCoMaximal(rLab->getAddr(), rev));
	rLab->setIPRStatus(false);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug1)
			    << "--- " << (llvm::isa<BackwardRevisit>(ri) ? "Backward" : "Forward")
			    << " revisiting " << ri.getPos() << " <-- " << rev << "\n"
			    << getGraph(););

	/*  Try to remove the read from the execution */
	if (removeCASReadIfBlocks(rLab, g.getEventLabel(rev)))
		return true;

	/* If the revisited label became an RMW, add the store part and revisit */
	if (auto *sLab = completeRevisitedRMW(rLab))
		return calcRevisits(sLab);

	/* Blocked barrier: block thread */
	if (llvm::isa<BWaitReadLabel>(rLab) &&
	    getReadValue(rLab) != getBarrierInitValue(rLab->getAccess())) {
		blockThread(BarrierBlockLabel::create(rLab->getPos().next()));
	}

	/* Blocked lock -> prioritize locking thread */
	if (llvm::isa<LockCasReadLabel>(rLab)) {
		blockThread(LockNotAcqBlockLabel::create(rLab->getPos().next()));
		if (!getConf()->bound.has_value())
			threadPrios = {rLab->getRf()->getPos()};
	}
	auto *oLab = g.getPreviousLabelST(rLab, [&](const EventLabel *oLab) {
		return llvm::isa<SpeculativeReadLabel>(oLab);
	});

	if (getConf()->helper && (llvm::isa<SpeculativeReadLabel>(rLab) || oLab))
		threadPrios = {rLab->getPos()};
	return true;
}

bool GenMCDriver::forwardRevisit(const ForwardRevisit &fr)
{
	auto &g = getGraph();
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
	auto &g = getGraph();

	/* Recalculate the view because some B labels might have been
	 * removed */
	auto *brh = llvm::dyn_cast<BackwardRevisitHELPER>(&br);
	auto v = getRevisitView(g.getReadLabel(br.getPos()), g.getWriteLabel(br.getRev()),
				brh ? g.getWriteLabel(brh->getMid()) : nullptr);

	auto og = copyGraph(&br, &*v);
	auto m = createChoiceMapForCopy(*og);

	pushExecution({std::move(og), LocalQueueT(), std::move(m)});

	repairDanglingReads(getGraph());
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

bool GenMCDriver::restrictAndRevisit(Stamp stamp, const WorkSet::ItemT &item)
{
	/* First, appropriately restrict the worklist and the graph */
	getExecution().restrict(stamp);

	lastAdded = item->getPos();
	if (auto *fr = llvm::dyn_cast<ForwardRevisit>(&*item))
		return forwardRevisit(*fr);
	if (auto *br = llvm::dyn_cast<BackwardRevisit>(&*item)) {
		return backwardRevisit(*br);
	}
	BUG();
	return false;
}

SVal GenMCDriver::handleDskRead(std::unique_ptr<DskReadLabel> drLab)
{
	auto &g = getGraph();

	if (isExecutionDrivenByGraph(&*drLab)) {
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
	auto *lab = llvm::dyn_cast<DskReadLabel>(addLabelToGraph(std::move(drLab)));
	g.changeRf(lab->getPos(), validStores[0]);

	/* ... filter out all option that make the recovery invalid */
	filterInvalidRecRfs(lab, validStores);

	/* Push all the other alternatives choices to the Stack */
	for (auto it = validStores.begin() + 1; it != validStores.end(); ++it)
		addToWorklist(lab->getStamp(),
			      std::make_unique<ReadForwardRevisit>(lab->getPos(), *it));
	return getDskWriteValue(g.getEventLabel(validStores[0]), lab->getAccess());
}

void GenMCDriver::handleDskWrite(std::unique_ptr<DskWriteLabel> wLab)
{
	if (isExecutionDrivenByGraph(&*wLab))
		return;

	auto &g = getGraph();

	g.trackCoherenceAtLoc(wLab->getAddr());

	/* Disk writes should always be hb-ordered */
	auto placesRange = getCoherentPlacings(wLab->getAddr(), wLab->getPos(), false);
	BUG_ON(placesRange.begin() != placesRange.end());

	/* Safe to _only_ add it at the end of MO */
	auto *lab = llvm::dyn_cast<WriteLabel>(addLabelToGraph(std::move(wLab)));
	g.addStoreToCO(lab, placesRange.end());

	calcRevisits(lab);
	return;
}

SVal GenMCDriver::handleDskOpen(std::unique_ptr<DskOpenLabel> oLab)
{
	auto &g = getGraph();

	if (isExecutionDrivenByGraph(&*oLab)) {
		auto *lab = llvm::dyn_cast<DskOpenLabel>(g.getEventLabel(oLab->getPos()));
		BUG_ON(!lab);
		return lab->getFd();
	}

	/* We get a fresh file descriptor for this open() */
	auto fd = getFreshFd();
	ERROR_ON(fd == -1, "Too many calls to open()!\n");

	oLab->setFd(SVal(fd));
	auto *lab = llvm::dyn_cast<DskOpenLabel>(addLabelToGraph(std::move(oLab)));
	return lab->getFd();
}

void GenMCDriver::handleDskFsync(std::unique_ptr<DskFsyncLabel> fLab)
{
	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::handleDskSync(std::unique_ptr<DskSyncLabel> fLab)
{
	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
	return;
}

void GenMCDriver::handleDskPbarrier(std::unique_ptr<DskPbarrierLabel> fLab)
{
	if (isExecutionDrivenByGraph(&*fLab))
		return;

	addLabelToGraph(std::move(fLab));
	return;
}

bool GenMCDriver::handleHelpingCas(std::unique_ptr<HelpingCasLabel> hLab)
{
	if (isExecutionDrivenByGraph(&*hLab))
		return true;

	/* Before adding it to the graph, ensure that the helped CAS exists */
	if (!checkHelpingCasCondition(&*hLab)) {
		blockThread(HelpedCASBlockLabel::create(hLab->getPos()));
		return false;
	}
	addLabelToGraph(std::move(hLab));
	return true;
}

bool GenMCDriver::handleOptional(std::unique_ptr<OptionalLabel> lab)
{
	auto &g = getGraph();

	if (isExecutionDrivenByGraph(&*lab))
		return llvm::dyn_cast<OptionalLabel>(g.getEventLabel(lab->getPos()))->isExpanded();

	if (std::any_of(label_begin(g), label_end(g), [&](auto &lab) {
		    auto *oLab = llvm::dyn_cast<OptionalLabel>(&lab);
		    return oLab && !oLab->isExpandable();
	    }))
		lab->setExpandable(false);

	auto *oLab = llvm::dyn_cast<OptionalLabel>(addLabelToGraph(std::move(lab)));

	if (!inEstimationMode() && oLab->isExpandable())
		addToWorklist(oLab->getStamp(),
			      std::make_unique<OptionalForwardRevisit>(oLab->getPos()));
	return false; /* should not be expanded yet */
}

void GenMCDriver::handleLoopBegin(std::unique_ptr<LoopBeginLabel> bLab)
{
	if (isExecutionDrivenByGraph(&*bLab))
		return;

	addLabelToGraph(std::move(bLab));
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
	auto *mLab = g.getPreviousLabelST(wLab, [wLab](const EventLabel *lab) {
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

void GenMCDriver::handleSpinStart(std::unique_ptr<SpinStartLabel> lab)
{
	auto &g = getGraph();

	/* If it has not been added to the graph, do so */
	if (isExecutionDrivenByGraph(&*lab))
		return;

	auto *stLab = addLabelToGraph(std::move(lab));

	/* Check whether we can detect some spinloop dynamically */
	auto *lbLab = g.getPreviousLabelST(
		stLab, [](const EventLabel *lab) { return llvm::isa<LoopBeginLabel>(lab); });
	/* If we did not find a loop-begin, this a manual instrumentation(?); report to user */
	ERROR_ON(!lbLab, "No loop-beginning found!\n");

	auto *pLab = g.getPreviousLabelST(stLab, [lbLab](const EventLabel *lab) {
		return llvm::isa<SpinStartLabel>(lab) && lab->getIndex() > lbLab->getIndex();
	});
	if (!pLab)
		return;

	for (auto i = pLab->getIndex() + 1; i < stLab->getIndex(); i++) {
		auto *wLab =
			llvm::dyn_cast<WriteLabel>(g.getEventLabel(Event(stLab->getThread(), i)));
		if (wLab && isWriteEffectful(wLab) && isWriteObservable(wLab))
			return; /* found event w/ side-effects */
	}
	/* Spinloop detected */
	auto stPos = stLab->getPos();
	g.removeLast(stPos.thread);
	blockThreadTryMoot(SpinloopBlockLabel::create(stPos));
}

bool GenMCDriver::areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab)
{
	auto &g = getGraph();

	/* Check that there are no other side-effects since the previous iteration.
	 * We don't have to look for a BEGIN label since ZNE labels are always
	 * preceded by a spin-start */
	auto *ssLab = g.getPreviousLabelST(
		lab, [](const EventLabel *lab) { return llvm::isa<SpinStartLabel>(lab); });
	BUG_ON(!ssLab);
	for (auto i = ssLab->getIndex() + 1; i < lab->getIndex(); ++i) {
		auto *oLab = g.getEventLabel(Event(ssLab->getThread(), i));
		if (llvm::isa<WriteLabel>(oLab) && !llvm::isa<FaiWriteLabel>(oLab))
			return false;
	}

	auto *wLab = llvm::dyn_cast<FaiWriteLabel>(g.getPreviousLabelST(
		lab, [](const EventLabel *lab) { return llvm::isa<FaiWriteLabel>(lab); }));
	BUG_ON(!wLab);

	/* All stores in the RMW chain need to be read from at most 1 read,
	 * and there need to be no other stores that are not hb-before lab */
	for (auto &lab : labels(g)) {
		if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(&lab)) {
			if (mLab->getAddr() == wLab->getAddr() && !llvm::isa<FaiReadLabel>(mLab) &&
			    !llvm::isa<FaiWriteLabel>(mLab) &&
			    !getHbView(wLab).contains(mLab->getPos()))
				return false;
		}
	}
	return true;
}

void GenMCDriver::handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab)
{
	auto &g = getGraph();

	/* If we are actually replaying this one, it is not a spin loop*/
	if (isExecutionDrivenByGraph(&*lab))
		return;

	auto *zLab = llvm::dyn_cast<FaiZNESpinEndLabel>(addLabelToGraph(std::move(lab)));
	if (areFaiZNEConstraintsSat(&*zLab)) {
		auto pos = zLab->getPos();
		g.removeLast(pos.thread);
		blockThreadTryMoot(FaiZNEBlockLabel::create(pos));
	}
	return;
}

void GenMCDriver::handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab)
{
	if (isExecutionDrivenByGraph(&*lab))
		return;

	blockThreadTryMoot(LockZNEBlockLabel::create(lab->getPos()));
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

void GenMCDriver::printGraph(bool printMetadata /* false */,
			     llvm::raw_ostream &s /* = llvm::dbgs() */)
{
	auto &g = getGraph();
	LabelPrinter printer([this](const SAddr &saddr) { return getVarName(saddr); },
			     [this](const ReadLabel &lab) {
				     return llvm::isa<DskReadLabel>(&lab)
						    ? getDskReadValue(
							      llvm::dyn_cast<DskReadLabel>(&lab))
						    : getReadValue(&lab);
			     });

	/* Print the graph */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		auto &thr = EE->getThrById(i);
		s << thr;
		if (getConf()->symmetryReduction) {
			if (auto *bLab = g.getFirstThreadLabel(i)) {
				auto symm = bLab->getSymmetricTid();
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
			for (const auto &w : stores(g, locIt->first))
				s << w << " ";
			s << "]\n";
		}
	}
	s << "\n";
}

void GenMCDriver::dotPrintToFile(const std::string &filename, const EventLabel *errLab,
				 const EventLabel *confLab)
{
	auto &g = getGraph();
	auto *EE = getEE();
	std::ofstream fout(filename);
	llvm::raw_os_ostream ss(fout);
	DotPrinter printer([this](const SAddr &saddr) { return getVarName(saddr); },
			   [this](const ReadLabel &lab) {
				   return llvm::isa<DskReadLabel>(&lab)
						  ? getDskReadValue(
							    llvm::dyn_cast<DskReadLabel>(&lab))
						  : getReadValue(&lab);
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
				ss << "\"" << g.getLastThreadEvent(jLab->getChildId()) << "\" -> \""
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

	auto ai = a.getMax(e.thread);
	a.setMax(e);
	auto &thr = getEE()->getThrById(e.thread);
	for (int i = ai; i <= e.index; i++) {
		const EventLabel *lab = g.getEventLabel(Event(e.thread, i));
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
			if (rLab->getRf())
				recPrintTraceBefore(rLab->getRf()->getPos(), a, ss);
		if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab))
			recPrintTraceBefore(g.getLastThreadEvent(jLab->getChildId()), a, ss);
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

void GenMCDriver::printTraceBefore(const EventLabel *lab, llvm::raw_ostream &s /* = llvm::dbgs() */)
{
	s << "Trace to " << lab->getPos() << ":\n";

	/* Linearize (po U rf) and print trace */
	View a;
	recPrintTraceBefore(lab->getPos(), a, s);
}
