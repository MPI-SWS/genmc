/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#include "GenMCDriver.hpp"
#include "genmc/Execution/Consistency/BoundDecider.hpp"
#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/Consistency/SymmetryChecker.hpp"
#include "genmc/Execution/DepExecutionGraph.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/GraphUtils.hpp"
#include "genmc/Execution/LabelVisitor.hpp"
#include "genmc/Support/Cast.hpp"
#include "genmc/Support/DotPrint.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Support/Logger.hpp"
#include "genmc/Support/Parser.hpp"
#include "genmc/Support/SExprVisitor.hpp"
#include "genmc/Support/ThreadPool.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/DriverHandlerDispatcher.hpp"
#include "genmc/Verification/Relinche/LinearizabilityChecker.hpp"
#include "genmc/Verification/Relinche/Specification.hpp"
#include "genmc/Verification/Scheduler.hpp"
#include "genmc/Verification/VerificationResult.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <system_error>

using namespace std::string_literals;

/************************************************************
 ** GENERIC MODEL CHECKING DRIVER
 ***********************************************************/

GenMCDriver::GenMCDriver(std::shared_ptr<const Config> conf, ThreadPool *pool /* = nullptr */,
			 Mode mode /* = VerificationMode{} */)
	: mode(mode), pool(pool), userConf(std::move(conf))
{
	/* Set up the execution context */
	consChecker = ConsistencyChecker::create(getConf());
	auto cfg = ExecutionGraph::Config{.execState = &execState_,
					  .consChecker = &*consChecker,
					  .emitNALabels = Config::emitNALabels};
	auto execGraph = userConf->isDepTrackingModel ? std::make_unique<DepExecutionGraph>(cfg)
						      : std::make_unique<ExecutionGraph>(cfg);
	execStack.emplace_back(std::move(execGraph), std::move(LocalQueueT()),
			       std::move(ChoiceMap()));

	symmChecker = SymmetryChecker::create();
	auto hasBounder = userConf->bound.has_value();
	GENMC_DEBUG(hasBounder |= userConf->boundsHistogram;);
	if (hasBounder)
		bounder = BoundDecider::create(getConf()->boundType);

	scheduler_ = (inEstimationMode() || inRandomMode())
			     ? std::make_unique<WFRScheduler>(getConf())
			     : Scheduler::create(getConf());

	/* Set up a random-number generator (for the scheduler) */
	if (userConf->printRandomScheduleSeed)
		PRINT(VerbosityLevel::Error, "Seed: {}\n", *userConf->randomScheduleSeed);
	estRng.seed(*userConf->randomScheduleSeed);

	if (userConf->collectLinSpec)
		result.specification = std::make_unique<Specification>(userConf->maxExtSize
#ifdef ENABLE_GENMC_DEBUG
								       ,
								       userConf->relincheDebug
#endif
		);
	if (userConf->checkLinSpec)
		relinche =
			LinearizabilityChecker::create(&getConsChecker(), *getConf()->checkLinSpec);
}

GenMCDriver::~GenMCDriver() = default;

GenMCDriver::Execution::Execution(std::unique_ptr<ExecutionGraph> g, LocalQueueT &&w, ChoiceMap &&m)
	: graph(std::move(g)), workqueue(std::move(w)), choices(std::move(m))
{}
GenMCDriver::Execution::~Execution() = default;

static void repairRead(ExecutionGraph &g, ReadLabel *lab)
{
	auto *maxLab = g.co_max(lab->getAddr());
	lab->setRf(maxLab);
}

void GenMCDriver::repairDanglingReads(ExecutionGraph &g)
{
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *rLab = genmc::dyn_cast<ReadLabel>(g.getLastThreadLabel(i));
		if (rLab && !rLab->getRf()) {
			repairRead(g, rLab);
			updateLabelViews(rLab);
		}
	}
}

void GenMCDriver::Execution::restrict(Stamp stamp)
{
	auto &g = getGraph();
	g.cutToStamp(stamp);
	getChoiceMap().cut(*g.getViewFromStamp(stamp));
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
	execStack.emplace_back(std::move(exec->graph), LocalQueueT(), std::move(exec->choices));

	getExec().getGraph().setConsChecker(&getConsChecker());
	getExec().getGraph().setState(&getExecState());
}

std::unique_ptr<GenMCDriver::Execution> GenMCDriver::extractState()
{
	return std::make_unique<Execution>(GenMCDriver::Execution(
		getExec().getGraph().clone(), LocalQueueT(), ChoiceMap(getExec().getChoiceMap())));
}

static void reconstructState(EventLabel *eLab, View &a, ExecutionState &state)
{
	if (!eLab || a.contains(eLab->getPos()))
		return;

	auto i = a.getMax(eLab->getThread());
	a.updateIdx(eLab->getPos());

	auto &g = *eLab->getParent();
	for (++i; i <= eLab->getIndex(); i++) {
		const auto *lab = g.getEventLabel(Event(eLab->getThread(), i));
		if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab))
			reconstructState(rLab->getRf(), a, state);
		if (const auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(lab))
			reconstructState(g.getLastThreadLabel(jLab->getChildId()), a, state);
		if (const auto *bLab = genmc::dyn_cast<ThreadStartLabel>(lab)) {
			reconstructState(bLab->getCreate(), a, state);
		}

		if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab)) {
			if (rLab->isNotAtomic()) {
				g.getState().onNALoad(
					rLab->getPos(), rLab->getAccess(),
					eLab->getParent()->getConsChecker()->getHbView(rLab));
			} else {
				/* TODO: If we support emitNA + lazy NA value, we should notify
				 * the corresponding value here */
				g.getState().onATLoad(rLab->getPos(), rLab->getAccess(),
						      g.getConsChecker()->getHbView(rLab));
			}
		} else if (const auto *wLab = genmc::dyn_cast<WriteLabel>(lab)) {
			if (wLab->isNotAtomic())
				g.getState().onNAStore(wLab->getPos(), wLab->getAccess(),
						       g.getConsChecker()->getHbView(wLab),
						       wLab->getVal());
			else {
				/* TODO: If we support emitNA + lazy NA value, we should notify
				 * the corresponding value here */
				g.getState().onATStore(wLab->getPos(), wLab->getAccess(),
						       g.getConsChecker()->getHbView(wLab),
						       wLab->isEffectful(),
						       wLab->isAtLeastRelease());
			}
		} else if (const auto *aLab = genmc::dyn_cast<MallocLabel>(lab)) {
			state.onAlloc(aLab->getPos(), aLab->getSize(), aLab->getAlignment(),
				      aLab->getStorageDuration(), aLab->getStorageType(),
				      aLab->getAddressSpace());
		} else if (const auto *dLab = genmc::dyn_cast<FreeLabel>(lab)) {
			state.onFree(dLab->getPos(), dLab->getAddr(),
				     genmc::isa<HpRetireLabel>(dLab));
		} else if (const auto *ssLab = genmc::dyn_cast<SpinStartLabel>(lab)) {
			state.onSpinStart(ssLab->getPos());
		} else if (const auto *lbLab = genmc::dyn_cast<LoopBeginLabel>(lab)) {
			state.onLoopBegin(lbLab->getPos());
		} else if (const auto *hpLab = genmc::dyn_cast<HpProtectLabel>(lab)) {
			state.onProtect(hpLab->getHpAddr(), hpLab->getProtectedAddr());
		}
	}
}

static void reconstructState(ExecutionGraph &g, ExecutionState &state)
{
	View a;
	for (auto i = 1; i < (int)g.getNumThreads(); i++)
		a.setMax(Event(i, -1));
	for (auto i = 0U; i < g.getNumThreads(); i++)
		reconstructState(g.getLastThreadLabel(i), a, state);
}

bool GenMCDriver::handleExecutionStart()
{
	auto &g = getExec().getGraph();
	auto &state = getExec().getGraph().getState();

	/* Set various exploration options for this execution */
	unmoot();
	state.clear();
	if constexpr (Config::emitNALabels)
		reconstructState(g, state);
	getScheduler().resetExplorationOptions(g);
	return getScheduler().inErrorReplay();
}

void GenMCDriver::checkHelpingCasAnnotation()
{
	/* If we were waiting for a helped CAS that did not appear, complain */
	auto &g = getExec().getGraph();
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (genmc::isa<HelpedCASBlockLabel>(g.getLastThreadLabel(i)))
			ERROR("Helped/Helping CAS annotation error! Does helped CAS always "
			      "execute?");
	}

	/* Next, we need to check whether there are any extraneous
	 * stores, not visible to the helped/helping CAS */
	for (auto &lab : g.labels() | std::views::filter([](auto &lab) {
				 return genmc::isa<HelpingCasLabel>(&lab);
			 })) {
		auto *hLab = genmc::dyn_cast<HelpingCasLabel>(&lab);

		/* Check that all stores that would make this helping
		 * CAS succeed are read by a helped CAS.
		 * We don't need to check the swap value of the helped CAS */
		if (std::ranges::any_of(g.co(hLab->getAddr()), [&](auto &sLab) {
			    return hLab->getExpected() == sLab.getVal() &&
				   std::ranges::none_of(sLab.readers(), [&](auto &rLab) {
					   return genmc::isa<HelpedCasReadLabel>(&rLab);
				   });
		    }))
			ERROR("Helped/Helping CAS annotation error! "
			      "Unordered store to helping CAS location!");

		/* Special case for the initializer (as above) */
		if (hLab->getAddr().isStatic() &&
		    hLab->getExpected() == g.getInitVal(hLab->getAccess())) {
			auto rsView = g.labels() | std::views::filter([hLab](auto &lab) {
					      auto *rLab = genmc::dyn_cast<ReadLabel>(&lab);
					      return rLab && rLab->getAddr() == hLab->getAddr();
				      });
			if (std::ranges::none_of(rsView, [&](auto &lab) {
				    return genmc::isa<HelpedCasReadLabel>(&lab);
			    }))
				ERROR("Helped/Helping CAS annotation error! "
				      "Unordered store to helping CAS location!");
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

static const auto maybeTimeRelinche = [](auto &&relinche, auto &&g) {
#ifdef ENABLE_GENMC_DEBUG
	const auto &start = std::chrono::high_resolution_clock::now();
#endif
	auto res = relinche.refinesSpec(g);
#ifdef ENABLE_GENMC_DEBUG
	const auto &stop = std::chrono::high_resolution_clock::now();
	res.analysisTime = stop - start;
#endif
	return res;
};

static void printGraph(const ExecutionGraph &g, const GenMCDriver::GraphDbgInfo &dbgInfo,
		       std::ostream &s = std::cerr);

auto GenMCDriver::handleExecutionEnd() -> std::optional<VerificationError>
{
	auto &g = getExec().getGraph();

	if (isMoot()) {
		GENMC_DEBUG(++result.exploredMoot;);
		return {};
	}

	/* Helper: Check helping CAS annotation */
	if (getConf()->helper)
		checkHelpingCasAnnotation();

	/* If in estimation mode, guess the total.
	 * (This may run a few times, but that's OK.)*/
	if (inEstimationMode()) {
		updateStSpaceEstimation();
		if (!shouldStopEstimating())
			getExec().getWorkqueue().add(std::make_unique<RerunForwardRevisit>());
	}

	/* If in random mode, check if the budget has been depleted */
	if (inRandomMode()) {
		if (!shouldStopRandom())
			getExec().getWorkqueue().add(std::make_unique<RerunForwardRevisit>());
	}

	/* Ignore the execution if some assume has failed */
	if (g.isBlocked()) {
		++result.exploredBlocked;
		if (getConf()->printBlockedExecs)
			printGraph(g, dbgInfo_);
		if (getConf()->checkLiveness)
			return checkLiveness();
		return {};
	}

	if (getConf()->warnUnfreedMemory)
		if (const auto err = checkUnfreedMemory())
			return err;
	if (getConf()->printExecGraphs)
		printGraph(g, dbgInfo_);

	GENMC_DEBUG(if (getConf()->boundsHistogram && inVerificationMode()) trackExecutionBound(););

	++result.explored;
	if (fullExecutionExceedsBound())
		++result.boundExceeding;

	if (isHalting() || g.isBlocked() || isMoot())
		return {};

	if (!inVerificationMode())
		return {};

	/* TODO: figure out how to add linearization to random mode */
	/* Relinche: Collect/check abstract behavior */
	if (getConf()->collectLinSpec)
		result.specification->add(g, &getConsChecker(), getConf()->symmetryReduction);
	if (getConf()->checkLinSpec) {
		result.relincheResult += maybeTimeRelinche(getRelinche(), getExec().getGraph());
		if (result.relincheResult.status) {
			result.status = VerificationError::VE_LinearizabilityError;
			reportError(std::ranges::begin(g.rlabels())->getPos(),
				    {std::nullopt, *result.status,
				     result.relincheResult.status->toString()});
			return {VerificationError::VE_LinearizabilityError};
		}
	}
	return {};
}

bool GenMCDriver::done()
{
	auto validExecution = false;
	while (!isHalting() && !validExecution) {
		auto item = getExec().getWorkqueue().getNext();
		if (!item) {
			if (popExecution())
				continue;
			return true;
		}
		validExecution = restrictAndRevisit(item) && isRevisitValid(*item);
	}
	return isHalting();
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

/************************************************************
 ** Scheduling methods
 ***********************************************************/

void GenMCDriver::blockThreadTryMoot(std::unique_ptr<BlockLabel> bLab)
{
	auto &g = getExec().getGraph();
	auto pos = bLab->getPos();
	blockThread(g, std::move(bLab));
	auto *lab = g.getLastThreadLabel(pos.thread);
	mootExecutionIfFullyBlocked(lab);
}

auto GenMCDriver::scheduleNext(std::span<Action> runnable) -> ScheduleResult
{
	if (isHalting())
		return Error{};
	if (isMoot())
		return Blocked{};

	auto &g = getExec().getGraph();
	if (auto next = getScheduler().schedule(g, runnable); next)
		return *next;
	return g.isBlocked() ? ScheduleResult(Blocked{}) : ScheduleResult(Finished{});
}

auto GenMCDriver::runFromCache() -> bool
{
	if (!getConf()->instructionCaching || !inVerificationMode())
		return false;

	do {
		auto toAdd = getScheduler().scheduleFromCache(getExec().getGraph());
		if (!toAdd)
			return true;
		if (*toAdd == nullptr)
			return false;

		addLabelsToGraph(**toAdd);
	} while (!isMoot() && !isHalting());
	return true;
}

bool GenMCDriver::isExecutionValid(const EventLabel *lab)
{
	auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
	return (!getConf()->symmetryReduction || getSymmChecker().isSymmetryOK(lab)) &&
	       (rLab && rLab->getRf() == getExec().getGraph().co_max(rLab->getAddr()) ||
		getConsChecker().isConsistent(lab)) &&
	       !partialExecutionExceedsBound();
}

bool GenMCDriver::isRevisitValid(const Revisit &revisit)
{
	auto &g = getExec().getGraph();
	auto pos = revisit.getPos();
	auto *mLab = genmc::dyn_cast<MemAccessLabel>(g.getEventLabel(pos));

	/* For replay/optional revisits, do nothing.
	 * (For replays, it is crucial: the graph might not be well-formed; e.g., invalid access) */
	if (genmc::isa<ReplayForwardRevisit>(&revisit) || !mLab)
		return true;

	if (!isExecutionValid(mLab))
		return false;

	/* If an extra event is added, re-check consistency */
	auto *rLab = genmc::dyn_cast<ReadLabel>(mLab);
	auto *nLab = g.po_imm_succ(mLab);
	return !rLab || !rLab->isRMW() ||
	       (isExecutionValid(nLab) // && !checkForRaces(nLab) // FIXME
	       );
}

bool GenMCDriver::isExecutionDrivenByGraph(Event curr)
{
	const auto &g = getExec().getGraph();
	return (curr.index < g.getThreadSize(curr.thread)) &&
	       !genmc::isa<EmptyLabel>(g.getEventLabel(curr));
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

EventLabel *GenMCDriver::addLabelToGraph(std::unique_ptr<EventLabel> lab)
{
	auto &g = getExec().getGraph();

	/* Cache the event before updating views (inits are added w/ tcreate) */
	if (getConf()->instructionCaching && inVerificationMode())
		getScheduler().cacheEventLabel(g, &*lab);

	/* Add and update views */
	auto *addedLab = g.add(std::move(lab));
	updateLabelViews(addedLab);

	if (addedLab->getIndex() >= getConf()->warnOnGraphSize) {
		LOG_ONCE("large-graph", VerbosityLevel::Tip,
			 "The execution graph seems quite large. Consider bounding all loops or "
			 "using -unroll\n");
	}
	return addedLab;
}

void GenMCDriver::addLabelsToGraph(const std::vector<std::unique_ptr<EventLabel>> &labs)
{
	auto &g = getExec().getGraph();
	DriverHandlerDispatcher dispatcher(this);

	for (const auto &vlab : labs) {
		VERIFY(!vlab->hasStamp());

		if (!isExecutionDrivenByGraph(vlab->getPos()))
			dispatcher.visit(vlab);

		if (isMoot() || genmc::isa<BlockLabel>(g.getLastThreadLabel(vlab->getThread())))
			break;
	}

	/* Graph well-formedness: ensure RMWs events are scheduled as one.
	 * (Cannot rely on next round scheduling the same thread.) */
	auto *lastLab = &*getExec().getGraph().rlabels().begin();
	if (auto *rLab = genmc::dyn_cast<ReadLabel>(lastLab)) {
		if (auto wLab = createRMWWriteLabel(g, rLab))
			dispatcher.visit(*wLab);
	}
}

void GenMCDriver::updateLabelViews(EventLabel *lab)
{
	getConsChecker().updateMMViews(lab);
	if (!getConf()->symmetryReduction)
		return;

	getSymmChecker().updatePrefixWithSymmetries(lab);
}

void GenMCDriver::configureProbe(MemLabel *rLab, Event pos, SAddr addr, ASize size)
{
	rLab->reset();

	rLab->setPos(pos);
	rLab->setAddr(addr);
	rLab->setSize(size);

	auto *predLab = getExec().getGraph().getLastThreadLabel(pos.thread);
	VERIFY(predLab->getIndex() == pos.index - 1);
	for (const auto &v : predLab->views()) {
		auto newV = View(v);
		newV.updateIdx(pos);
		rLab->addView(std::move(newV));
	}
	rLab->setPrefixView(predLab->getPrefixView().clone());
}

std::optional<VerificationError> GenMCDriver::checkForRaces(const EventLabel *lab)
{
	if (getConf()->disableRaceDetection || inEstimationMode())
		return {};

	/* Check for hard errors */
	const EventLabel *racyLab = nullptr;
	if (auto err = getConsChecker().checkErrors(lab, racyLab); err) {
		reportError(lab->getPos(), {lab->getPos(), *err, "", racyLab});
		return err;
	}

	/* Check whether there are any unreported warnings... */
	std::vector<const EventLabel *> races;
	auto newWarnings = getConsChecker().checkWarnings(lab, getResult().warnings, races);

	/* ... and report them */
	auto i = 0U;
	for (auto &wcode : newWarnings) {
		if (reportWarningOnce(lab->getPos(), wcode, races[i++]))
			return {wcode};
	}
	return {};
}

GenMCDriver::HandleResult<SVal> GenMCDriver::getReadRetValue(const ReadLabel *rLab)
{
	/* Bottom is an acceptable re-option only @ replay */
	auto &scheduler = getScheduler();
	if (!rLab->getRf()) {
		VERIFY(scheduler.inErrorReplay());
		return {Invalid{}, 1U};
	}

	using Result = GenMCDriver::HandleResult<SVal>;
	using Evaluator = SExprEvaluator<ModuleVarID>;
	auto res = rLab->getAccessValue(rLab->getAccess());
	auto &g = getExec().getGraph();

	/* Return nullopt for reads that require an interpreter reset */
	if (getConf()->ipr && rLab->getAnnot() &&
	    !Evaluator().evaluate(&*rLab->getAnnot()->expr, res)) {
		blockThread(g, BlockLabel::createAssumeBlock(rLab->getPos().next(),
							     rLab->getAnnot()->type));
		return scheduler.inErrorReplay() ? Result(Invalid(), 1U) : Result(Reset(), 0U);
	}
	if (genmc::isa<BWaitReadLabel>(rLab) &&
	    !readsBarrierUnblockingValue(genmc::cast<BWaitReadLabel>(rLab))) {
		blockThread(g, BlockLabel::createAssumeBlock(rLab->getPos().next(),
							     AssumeType::Barrier));
		return scheduler.inErrorReplay() ? Result(Invalid(), 1U) : Result(Reset(), 0U);
	}
	return {res, 1U};
}

std::optional<VerificationError> GenMCDriver::checkAccessValidity(Event pos, const AAccess &access)
{
	/* Check that static accesses hit a registered global range,
	 * and dynamic accesses hit allocated memory */
	auto &state = getExec().getGraph().getState();
	if ((!getConf()->disableStaticValidityChecks && !access.addr.isDynamic() &&
	     !state.isStatic(access.addr)) ||
	    (access.addr.isDynamic() && !state.isAllocated(access.addr))) {
		reportError(pos, {pos, VerificationError::VE_AccessNonMalloc});
		return {VerificationError::VE_AccessNonMalloc};
	}
	return {};
}

std::optional<VerificationError> GenMCDriver::checkInitializedMem(const ReadLabel *rLab)
{
	// FIXME: Have label for mutex-destroy and check type instead of val.
	//        Also for barriers.

	// /* Locks should not read from destroyed mutexes */
	// const auto *lLab = genmc::dyn_cast<LockCasReadLabel>(rLab);
	// if (lLab && lLab->getAccessValue(lLab->getAccess()) == SVal(-1)) {
	// 	reportError(lLab->getPos(), {lLab->getPos(), VerificationError::VE_UninitializedMem,
	// 				     "Called lock() on destroyed mutex!", lLab->getRf()});
	// 	return {VerificationError::VE_UninitializedMem};
	// }

	/* Allow frontends to skip uninit/msa checks */
	if (getConf()->disableInitializationChecks)
		return {};

	/* Dynamic accesses should read initialized memory (if reading from INIT).
	 * We use two separate checks for atomics and NAs, since mixed-size NAs don't have an rf */
	if (rLab->getAddr().isDynamic() &&
	    (!rLab->getParent()->getState().isInitialized(rLab->getAccess()) ||
	     !getConsChecker().getHbView(rLab).contains(
		     rLab->getParent()->getState().getInitView(rLab->getAccess())))) {
		reportError(rLab->getPos(),
			    {rLab->getPos(), VerificationError::VE_UninitializedMem});
		return {VerificationError::VE_UninitializedMem};
	}

	/* Slightly unrelated check, but ensure there are no mixed-size accesses */
	if (rLab->getRf() && !rLab->getRf()->getPos().isInitializer() &&
	    genmc::dyn_cast<WriteLabel>(rLab->getRf())->getSize() != rLab->getSize()) {
		reportError(rLab->getPos(),
			    {rLab->getPos(), VerificationError::VE_MixedSize,
			     "Mixed-size accesses detected: tried to read with a " +
				     std::to_string(rLab->getSize().get() * 8) + "-bit access!\n" +
				     "Please check the LLVM-IR.\n"});
		return {VerificationError::VE_MixedSize};
	}
	return {};
}

std::optional<VerificationError> GenMCDriver::checkInitializedMem(const WriteLabel *wLab)
{
	auto &g = getExec().getGraph();

	/* Unlocks should unlock mutexes locked by the same thread */
	const auto *uLab = genmc::dyn_cast<UnlockWriteLabel>(wLab);
	if (uLab && !findMatchingLock(uLab)) {
		reportError(uLab->getPos(),
			    {uLab->getPos(), VerificationError::VE_InvalidUnlock,
			     "Called unlock() on mutex not locked by the same thread!"});
		return {VerificationError::VE_InvalidUnlock};
	}
	return {};
}

std::optional<VerificationError> GenMCDriver::checkFinalAnnotations(const WriteLabel *wLab)
{
	if (!getConf()->finalWrite)
		return {};

	auto &g = getExec().getGraph();

	if (!g.hasLocMoreThanOneStore(wLab->getAddr()))
		return {};
	if ((wLab->isFinal() &&
	     std::ranges::any_of(g.co(wLab->getAddr()),
				 [&](auto &sLab) {
					 return !getConsChecker().getHbView(wLab).contains(
						 sLab.getPos());
				 })) ||
	    (!wLab->isFinal() && std::ranges::any_of(g.co(wLab->getAddr()),
						     [&](auto &sLab) { return sLab.isFinal(); }))) {
		reportError(wLab->getPos(), {wLab->getPos(), VerificationError::VE_Annotation,
					     "Multiple stores at final location!"});
		return {VerificationError::VE_Annotation};
	}
	return {};
}

std::optional<VerificationError> GenMCDriver::checkIPRValidity(const ReadLabel *rLab)
{
	if (!rLab->getAnnot() || !getConf()->ipr)
		return {};

	auto &g = getExec().getGraph();
	auto racyIt = std::ranges::find_if(
		g.co(rLab->getAddr()), [&](auto &wLab) { return wLab.hasAttr(WriteAttr::WWRacy); });
	if (racyIt == std::ranges::end(g.co(rLab->getAddr())))
		return {};

	auto msg = "Unordered writes do not constitute a bug per se, though they often "
		   "indicate faulty design.\n"
		   "This warning is treated as an error due to in-place revisiting (IPR).\n"
		   "You can use -disable-ipr to disable this feature."s;
	reportError(racyIt->getPos(),
		    {racyIt->getPos(), VerificationError::VE_WWRace, msg, nullptr, true});
	return {VerificationError::VE_WWRace};
}

static auto threadReadsMaximal(const ExecutionGraph &g, int tid) -> bool
{
	/* Depending on whether this is a DSA loop or not, we have to
	 * adjust the detection starting point: DSA-blocked threads
	 * will have a SpinStart as their last event */
	VERIFY(genmc::isa<BlockLabel>(g.getLastThreadLabel(tid)));
	const auto *lastLab = g.po_imm_pred(g.getLastThreadLabel(tid));
	auto start = genmc::isa<SpinStartLabel>(lastLab) ? lastLab->getPos().prev()
							 : lastLab->getPos();

	/* Helper to get the co-max label */
	auto getCoMax = [&](auto &rLab) {
		/* Exclude RMW successor for ZNE spinloops */
		auto rco = g.rco(rLab->getAddr());
		auto coIt = std::ranges::find_if(rco, [&](auto &sLab) {
			return !sLab.getPrefixView().contains(rLab->getPos());
		});
		return coIt == std::ranges::end(rco) ? g.getInitLabel()
						     : (const EventLabel *)&*coIt;
	};

	for (auto j = start.index; j > 0; j--) {
		const auto *lab = g.getEventLabel(Event(tid, j));
		VERIFY(!genmc::isa<LoopBeginLabel>(lab));
		if (genmc::isa<SpinStartLabel>(lab))
			return true;
		if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab)) {
			if (rLab->getRf() != getCoMax(rLab))
				return false;
		}
	}
	UNREACHABLE();
}

auto GenMCDriver::checkLiveness() -> std::optional<VerificationError>
{
	if (isHalting())
		return {};

	const auto &g = getExec().getGraph();

	/* Collect all threads blocked at spinloops */
	std::vector<int> spinBlocked;
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		if (genmc::isa<SpinloopBlockLabel>(g.getLastThreadLabel(i)))
			spinBlocked.push_back(i);
	}

	if (spinBlocked.empty())
		return {};

	/* And check whether all of them are live or not */
	auto nonTermTID = 0u;
	if (std::all_of(spinBlocked.begin(), spinBlocked.end(), [&](int tid) {
		    nonTermTID = tid;
		    return threadReadsMaximal(g, tid);
	    })) {
		/* Print some TID blocked by a spinloop */
		auto lastPos = g.getLastThreadLabel(nonTermTID)->getPos();
		reportError(lastPos,
			    {lastPos, VerificationError::VE_Liveness,
			     "Non-terminating spinloop: thread " + std::to_string(nonTermTID)});
		return {VerificationError::VE_Liveness};
	}
	return {};
}

auto GenMCDriver::checkUnfreedMemory() -> std::optional<VerificationError>
{
	if (isHalting())
		return {};

	auto &g = getExec().getGraph();
	Event unfreedAlloc{};
	if (std::ranges::any_of(g.getState().allocations(), [&](const auto &kv) {
		    unfreedAlloc = kv.second.pos;
		    return !kv.second.freePos.has_value();
	    })) {
		/* Return an error only if the warning was upgraded to an error */
		if (reportWarningOnce(unfreedAlloc, VerificationError::VE_UnfreedMemory))
			return {VerificationError::VE_UnfreedMemory};
	}
	return {};
}

void GenMCDriver::filterConflictingBarriers(const ReadLabel *lab, std::vector<EventLabel *> &stores)
{
	if (getConf()->disableBAM ||
	    (!genmc::isa<BIncFaiReadLabel>(lab) && !genmc::isa<BWaitReadLabel>(lab)))
		return;

	/* Helper lambdas */
	auto isReadByExclusiveRead = [&](auto *oLab) {
		if (auto *wLab = genmc::dyn_cast<WriteLabel>(oLab))
			return std::ranges::any_of(wLab->readers(),
						   [&](auto &rLab) { return rLab.isRMW(); });
		if (auto *iLab = genmc::dyn_cast<InitLabel>(oLab))
			return std::ranges::any_of(iLab->rfs(lab->getAddr()),
						   [&](auto &rLab) { return rLab.isRMW(); });
		UNREACHABLE();
	};
	auto findFaiReader = [](BIncFaiWriteLabel *wLab) {
		return std::ranges::find_if(wLab->readers(), [](auto &rLab) {
			return genmc::isa<BIncFaiReadLabel>(&rLab);
		});
	};
	auto findSameRoundMaximal = [&](BIncFaiWriteLabel *wLab) {
		auto &g = *wLab->getParent();
		while (!isLastInBarrierRound(wLab) &&
		       findFaiReader(wLab) != std::ranges::end(wLab->readers())) {
			wLab = genmc::dyn_cast<BIncFaiWriteLabel>(
				g.po_imm_succ(&*findFaiReader(wLab)));
		}
		return wLab;
	};

	/* barrier_wait()'s plain load should read maximally */
	if (auto *rLab = genmc::dyn_cast<BWaitReadLabel>(lab)) {
		auto *wLab = genmc::dyn_cast<BIncFaiWriteLabel>(stores[0]);
		VERIFY(wLab && wLab->getPos().next() == lab->getPos());
		stores[0] = findSameRoundMaximal(wLab);
		stores.resize(1);
		return;
	}

	/* barrier_wait()'s FAI loads should not read from conflicting stores */
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
	auto *lab = genmc::dyn_cast<ReadLabel>(g.getEventLabel(Event(t, rLab->getIndex())));
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

	using Evaluator = SExprEvaluator<ModuleVarID>;

	auto &g = getExec().getGraph();

	/* Ensure we keep the maximal store around even if Helper messed with it */
	VERIFY(!validStores.empty());
	auto maximal = validStores.back();
	validStores.erase(std::remove_if(validStores.begin(), validStores.end(),
					 [&](auto *wLab) {
						 auto val = wLab->getAccessValue(rLab->getAccess());
						 return wLab != maximal &&
							wLab != g.co_max(rLab->getAddr()) &&
							!Evaluator().evaluate(
								&*rLab->getAnnot()->expr, val);
					 }),
			  validStores.end());
	VERIFY(!validStores.empty());
}

void GenMCDriver::unblockWaitingHelping(const WriteLabel *lab)
{
	if (!genmc::isa<HelpedCasWriteLabel>(lab))
		return;

	/* FIXME: We have to wake up all threads waiting on helping CASes,
	 * as we don't know which ones are from the same CAS */
	for (auto i = 0u; i < getExec().getGraph().getNumThreads(); i++) {
		auto *bLab = genmc::dyn_cast_if_present<HelpedCASBlockLabel>(
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
		while (!genmc::isa<WriteLabel>(g.getEventLabel(Event(i, j))) && j > 0)
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
			      auto *rLab = genmc::dyn_cast<HelpedCasReadLabel>(&lab);
			      return rLab && rLab->isRMW() && rLab->getAddr() == hLab->getAddr() &&
				     rLab->getSize() == hLab->getSize() &&
				     rLab->getOrdering() == hLab->getOrdering() &&
				     rLab->getExpected() == hLab->getExpected() &&
				     rLab->getSwapVal() == hLab->getSwapVal();
		      });

	if (std::ranges::any_of(hsView, [&g, this](auto &lab) {
		    auto *hLab = genmc::dyn_cast<HelpedCasReadLabel>(&lab);
		    auto &view = getConsChecker().getHbView(hLab);
		    return !writesBeforeHelpedContainedInView(hLab, view);
	    }))
		ERROR("Helped/Helping CAS annotation error! "
		      "Not all stores before helped-CAS are visible to helping-CAS!");
	return std::ranges::begin(hsView) != std::ranges::end(hsView);
}

EventLabel *GenMCDriver::findConsistentRf(ReadLabel *rLab, std::vector<EventLabel *> &rfs)
{
	auto &g = getExec().getGraph();

	/* Otherwise, search for a consistent rf */
	std::ranges::reverse(rfs);
	while (rfs.size() > 1) {
		auto *back = rfs.back();
		rfs.pop_back();
		rLab->setRf(back);
		if (isExecutionValid(rLab)) {
			updateLabelViews(rLab);
			return back;
		}
	}

	auto *back = rfs.back();
	rfs.pop_back();
	rLab->setRf(back);
	updateLabelViews(rLab);

	/* For the non-bounding case, maximal extensibility guarantees consistency */
	if (!getConf()->bound.has_value() || isExecutionValid(rLab))
		return back;

	/* Extensibility is guaranteed with bounding because
	 * - the consistent choice might lead to a settled atomicity violation and has already been
	 * filtered-out
	 * - context bounding's slack might have changed due to an optimization */
	VERIFY(getConf()->bound.has_value() &&
	       (getConf()->boundType == BoundType::context || genmc::isa<CasReadLabel>(rLab) ||
		genmc::isa<FaiReadLabel>(rLab)));
	return nullptr;
}

EventLabel *GenMCDriver::findConsistentCo(WriteLabel *wLab, std::vector<EventLabel *> &cos)
{
	auto &g = getExec().getGraph();

	/* Similarly to the read case: rely on extensibility */
	auto back = cos.back();
	wLab->addCo(back);
	if (!getConf()->bound.has_value()) {
		cos.pop_back();
		return back;
	}

	// FIXME: This is wrong
	/* In contrast to the read case, we need to be a bit more careful:
	 * the consistent choice might not satisfy atomicity, but we should
	 * keep it around to try revisits */
	while (!cos.empty()) {
		auto back = cos.back();
		cos.pop_back();
		wLab->moveCo(back);
		if (isExecutionValid(wLab))
			return back;
	}
	return nullptr;
}

auto GenMCDriver::handleThreadKill(std::unique_ptr<ThreadKillLabel> kLab)
	-> HandleResult<std::monostate>
{
	addLabelToGraph(std::move(kLab));
	return {std::monostate{}, 1U};
}

auto GenMCDriver::handleThreadKill(const EventDbgInfo *dbg, Event pos)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		VERIFY(getScheduler().inErrorReplay());
		return {std::monostate{}, 1U};
	}
	return handleThreadKill(ThreadKillLabel::create(pos));
}

bool GenMCDriver::isSymmetricToSR(int candidate, Event parent, const ThreadInfo &info) const
{
	auto &g = getExec().getGraph();
	auto cParent = g.getFirstThreadLabel(candidate)->getCreateId();
	auto &cInfo = g.getFirstThreadLabel(candidate)->getThreadInfo();

	/* A tip to print to the user in case two threads look
	 * symmetric, but we cannot deem it */
	auto tipSymmetry = [&]() {
		LOG_ONCE("possible-symmetry", VerbosityLevel::Tip,
			 "Threads {} and {} could benefit from symmetry reduction. Consider using "
			 "__VERIFIER_spawn_symmetric().\n",
			 cInfo.id, info.id);
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
	auto lastMemAccess = g.getState().getLastMemAccess(parent.thread);
	if (lastMemAccess > mm.first && lastMemAccess <= mm.second) {
		tipSymmetry();
		return false;
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

auto GenMCDriver::handleThreadCreate(std::unique_ptr<ThreadCreateLabel> tcLab) -> HandleResult<int>
{
	auto &g = getExec().getGraph();

	/* First, check if the thread to be created already exists */
	int cid = 0;
	while (cid < (long)g.getNumThreads()) {
		if (!g.isThreadEmpty(cid)) {
			auto *bLab = genmc::dyn_cast_if_present<ThreadStartLabel>(
				g.getFirstThreadLabel(cid));
			if (bLab && bLab->getCreateId() == tcLab->getPos())
				break;
		}
		++cid;
	}

	/* Add an event for the thread creation */
	tcLab->setChildId(cid);
	auto *lab = genmc::dyn_cast<ThreadCreateLabel>(addLabelToGraph(std::move(tcLab)));

	/* This tid should not already exist in the graph */
	VERIFY(cid == (long)g.getNumThreads());
	g.addNewThread();

	/* Create a label and add it to the graph; is the thread symmetric to another one? */
	auto symm = getSymmetricTidSR(lab, lab->getChildInfo());
	auto *tsLab = addLabelToGraph(ThreadStartLabel::create(Event(cid, 0), lab->getPos(), lab,
							       lab->getChildInfo(), symm));
	if (symm != -1)
		g.getFirstThreadLabel(symm)->setSymmSuccTid(cid);
	return {cid, 1U};
}

auto GenMCDriver::handleThreadCreate(const EventDbgInfo *dbg, Event pos, ThreadInfo info,
				     const EventDeps &deps) -> HandleResult<int>
{
	auto &g = getExec().getGraph();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {(int)genmc::dyn_cast<ThreadCreateLabel>(g.getEventLabel(pos))->getChildId(),
			1U};
	return handleThreadCreate(ThreadCreateLabel::create(pos, info, deps));
}

GenMCDriver::HandleResult<SVal> GenMCDriver::handleThreadJoin(std::unique_ptr<ThreadJoinLabel> lab)
{
	auto &g = getExec().getGraph();

	if (!genmc::isa_and_present<ThreadFinishLabel>(g.getLastThreadLabel(lab->getChildId()))) {
		blockThread(g, JoinBlockLabel::create(lab->getPos(), lab->getChildId()));
		return {Reset{}, 0U};
	}

	auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(addLabelToGraph(std::move(lab)));
	auto cid = jLab->getChildId();

	auto *eLab = genmc::dyn_cast<ThreadFinishLabel>(g.getLastThreadLabel(cid));
	VERIFY(eLab);
	eLab->setParentJoin(jLab);

	if (cid < 0 || long(g.getNumThreads()) <= cid || cid == jLab->getThread()) {
		std::string err = "ERROR: Invalid TID in pthread_join(): " + std::to_string(cid);
		if (cid == jLab->getThread())
			err += " (TID cannot be the same as the calling thread)";
		reportError(jLab->getPos(),
			    {jLab->getPos(), VerificationError::VE_InvalidJoin, err});
		return {VerificationError::VE_InvalidJoin};
	}

	if (partialExecutionExceedsBound()) {
		moot();
		return {Invalid{}, 1U};
	}

	return {jLab->getReturnValue(), 1U};
}

GenMCDriver::HandleResult<SVal> GenMCDriver::handleThreadJoin(const EventDbgInfo *dbg, Event pos,
							      unsigned int childTid,
							      const EventDeps &deps)
{
	auto &g = getExec().getGraph();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {g.getEventLabel(pos)->getReturnValue(), 1U};
	return handleThreadJoin(ThreadJoinLabel::create(pos, childTid, deps));
}

auto GenMCDriver::handleThreadFinish(std::unique_ptr<ThreadFinishLabel> eLab)
	-> HandleResult<std::monostate>
{
	auto &g = getExec().getGraph();

	auto *lab = addLabelToGraph(std::move(eLab));
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *pLab = genmc::dyn_cast_if_present<JoinBlockLabel>(g.getLastThreadLabel(i));
		if (pLab && pLab->getChildId() == lab->getThread()) {
			/* If parent thread is waiting for me, relieve it */
			unblockThread(g, pLab->getPos());
		}
	}
	if (partialExecutionExceedsBound())
		moot();
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleThreadFinish(const EventDbgInfo *dbg, Event pos, SVal val)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleThreadFinish(ThreadFinishLabel::create(pos, val));
}

auto GenMCDriver::handleFence(std::unique_ptr<FenceLabel> fLab) -> HandleResult<std::monostate>
{
	auto *lab = addLabelToGraph(std::move(fLab));
	getExec().getGraph().getState().onFence(lab->getPos(), lab->isAtLeastRelease());
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleFence(const EventDbgInfo *dbg, Event pos, MemOrdering ord,
			      const EventDeps &deps) -> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		if constexpr (!Config::emitNALabels)
			getExec().getGraph().getState().onFence(
				pos, isAtLeastOrStrongerThan(ord, MemOrdering::Release));
		return {std::monostate{}, 1U};
	}
	return handleFence(FenceLabel::create(pos, ord, deps));
}

void GenMCDriver::checkReconsiderFaiSpinloop(const MemAccessLabel *lab)
{
	auto &g = getExec().getGraph();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		/* Is there any thread blocked on a potential spinloop? */
		auto *eLab = genmc::dyn_cast_if_present<FaiZNEBlockLabel>(g.getLastThreadLabel(i));
		if (!eLab)
			continue;

		/* Check whether this access affects the spinloop variable */
		auto epreds = g.po_preds(eLab);
		auto faiLabIt = std::ranges::find_if(
			epreds, [](auto &lab) { return genmc::isa<FaiWriteLabel>(&lab); });
		VERIFY(faiLabIt != std::ranges::end(epreds));

		auto *faiLab = genmc::dyn_cast<FaiWriteLabel>(&*faiLabIt);
		if (faiLab->getAddr() != lab->getAddr())
			continue;

		/* FAIs on the same variable are OK... */
		if (genmc::isa<FaiReadLabel>(lab) || genmc::isa<FaiWriteLabel>(lab))
			continue;

		/* If it does, and also breaks the assumptions, unblock thread */
		if (!getConsChecker().getHbView(faiLab).contains(lab->getPos())) {
			auto pos = eLab->getPos();
			unblockThread(g, pos);
			addLabelToGraph(FaiZNESpinEndLabel::create(pos));
		}
	}
}

const VectorClock &GenMCDriver::getPrefixView(const EventLabel *lab) const
{
	if (!lab->hasPrefixView())
		lab->setPrefixView(getConsChecker().calculatePrefixView(lab));
	return lab->getPrefixView();
}

std::vector<EventLabel *> GenMCDriver::getRfsApproximation(ReadLabel *lab)
{
	auto &g = getExec().getGraph();
	auto &cc = getConsChecker();
	auto rfs = cc.getCoherentStores(lab);
	if (!genmc::isa<CasReadLabel>(lab) && !genmc::isa<FaiReadLabel>(lab))
		return rfs;

	/* Remove atomicity violations */
	auto &before = getPrefixView(lab);
	auto isSettledRMWInView = [&before](auto &rLab) {
		auto &g = *rLab.getParent();
		return rLab.isRMW() && (!rLab.isRevisitable() || before.contains(rLab.getPos()));
	};
	auto atomicityViolationInView = [&isSettledRMWInView, lab](auto *sLab) {
		if (auto *wLab = genmc::dyn_cast<WriteLabel>(sLab)) {
			return lab->valueMakesRMWSucceed(wLab->getAccessValue(wLab->getAccess())) &&
			       std::ranges::any_of(wLab->readers(), isSettledRMWInView);
		};

		auto *iLab = genmc::cast<InitLabel>(sLab);
		auto addr = lab->getAddr();
		/* Reads to dynamic addresses cannot have read from Init */
		return // !addr.isDynamic() &&
			lab->valueMakesRMWSucceed(iLab->getAccessValue(lab->getAccess())) &&
			std::ranges::any_of(iLab->rfs(addr), isSettledRMWInView);
	};
	rfs.erase(std::remove_if(rfs.begin(), rfs.end(), atomicityViolationInView), rfs.end());
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
	if (!genmc::isa<CasReadLabel>(rLab) && !genmc::isa<FaiReadLabel>(rLab))
		return;

	const auto *casLab = genmc::dyn_cast<CasReadLabel>(rLab);
	auto valueMakesSuccessfulRMW = [&casLab, rLab](auto &&val) {
		return !casLab || val == casLab->getExpected();
	};
	stores.erase(
		std::remove_if(
			stores.begin(), stores.end(),
			[&](auto *sLab) {
				if (auto *iLab = genmc::dyn_cast<InitLabel>(sLab))
					return std::any_of(
						iLab->rf_begin(rLab->getAddr()),
						iLab->rf_end(rLab->getAddr()), [&](auto &rLab) {
							return rLab.isRMW() &&
							       valueMakesSuccessfulRMW(
								       rLab.getAccessValue(
									       rLab.getAccess()));
						});
				return std::ranges::any_of(g.rf_succs(sLab), [&](auto &rLab) {
					return rLab.isRMW() &&
					       valueMakesSuccessfulRMW(
						       rLab.getAccessValue(rLab.getAccess()));
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

	/* There is no bounding during estimation; reads are always extensible */
	VERIFY(!stores.empty());

	MyDist dist(0, stores.size() - 1);
	auto random = dist(estRng);
	rLab->setRf(stores[random]);
	updateLabelViews(rLab);
	return stores[random];
}

std::optional<VerificationError> GenMCDriver::checkForMixedSize(MemAccessLabel *lab)
{
	auto *g = lab->getParent();

	if (lab->isNotAtomic())
		return {};

	if (g->getState().isAtomicAccessConsistent(lab->getAccess()))
		return {};

	reportError(lab->getPos(),
		    {lab->getPos(), VerificationError::VE_MixedSize,
		     "Mixed-size accesses detected: tried to read with a " +
			     std::to_string(lab->getSize().get() * 8) + "-bit access!\n"});
	return {VerificationError::VE_MixedSize};
}

GenMCDriver::HandleResult<SVal> GenMCDriver::handleLoad(std::unique_ptr<ReadLabel> rLab,
							std::optional<SVal> oldVal)
{
	auto &g = getExec().getGraph();
	auto &frontier = g.getState();

	auto *lab = genmc::dyn_cast<ReadLabel>(addLabelToGraph(std::move(rLab)));
	auto err = checkAccessValidity(lab->getPos(), lab->getAccess())
			   .or_else([&] { return checkInitializedMem(lab); })
			   .or_else([&] { return checkForMixedSize(lab); })
			   .or_else([&] { return checkIPRValidity(lab); })
			   .or_else([&] { return checkForRaces(lab); });
	if (err)
		return {*err}; /* This execution will be blocked */

	VERIFY(!lab->isNotAtomic());
	/* Runtime sends the NA value lazily when adding next atomic access */
	if (oldVal)
		g.updateDeferredValue(lab->getAccess(), *oldVal);
	frontier.onATLoad(lab->getPos(), lab->getAccess(), getConsChecker().getHbView(lab));

	/* Check whether the load forces us to reconsider some existing event */
	checkReconsiderFaiSpinloop(lab);

	/* If a CAS read cannot be added maximally, reschedule */
	if (!getScheduler().isRescheduledRead(lab->getPos()) &&
	    removeCASReadIfBlocks(lab, g.co_max(lab->getAddr())))
		return {Reset{}, 0U};
	if (getScheduler().isRescheduledRead(lab->getPos()))
		getScheduler().setRescheduledRead(Event::getInit());

	/* Get an approximation of the stores we can read from */
	auto stores = getRfsApproximation(lab);
	VERIFY(!stores.empty());
	GENMC_DEBUG(LOG(VerbosityLevel::Debug3, "Rfs: {}", stores););
	filterOptimizeRfs(lab, stores);
	GENMC_DEBUG(LOG(VerbosityLevel::Debug3, "Rfs (optimized): {}", stores););

	EventLabel *rf = nullptr;

	if (inEstimationMode() || inRandomMode()) {
		if (inEstimationMode())
			getExec().getChoiceMap().update(lab, stores);
		filterAtomicityViolations(lab, stores);
		rf = pickRandomRf(lab, stores);
	} else {
		rf = findConsistentRf(lab, stores);
		/* Push all the other alternatives choices to the Stack */
		for (const auto &sLab : stores) {
			getExec().getWorkqueue().add(std::make_unique<ReadForwardRevisit>(
				lab->getPos(), sLab->getPos(), sLab == g.co_max(lab->getAddr())));
		}
	}

	if (!rf) {
		moot();
		return {Invalid{}, 1U};
	}

	GENMC_DEBUG(LOG(VerbosityLevel::Debug2, "--- Added load {}\n{}", lab->getPos(),
			getExec().getGraph()););

	return getReadRetValue(lab);
}

/************************************************************
 ** Memory operation handlers
 ***********************************************************/

#define LOAD_INTERCEPTOR_LOGIC(LabelT, OLD_VAL, ...)                                               \
	auto &g = getExec().getGraph();                                                            \
	auto &state = g.getState();                                                                \
	++pos;                                                                                     \
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)                          \
		return {*err};                                                                     \
	if (isExecutionDrivenByGraph(pos)) {                                                       \
		auto *lab = genmc::dyn_cast<ReadLabel>(g.getEventLabel(pos));                      \
		if constexpr (!Config::emitNALabels) {                                             \
			if (OLD_VAL)                                                               \
				g.updateDeferredValue(lab->getAccess(), *OLD_VAL);                 \
			state.onATLoad(lab->getPos(), lab->getAccess(),                            \
				       getConsChecker().getHbView(lab));                           \
		}                                                                                  \
		return getReadRetValue(lab);                                                       \
	}                                                                                          \
	return handleLoad(LabelT::create(pos, __VA_ARGS__), OLD_VAL);

#define STORE_INTERCEPTOR_LOGIC(LabelT, OLD_VAL, ...)                                              \
	auto &g = getExec().getGraph();                                                            \
	auto &state = g.getState();                                                                \
	++pos;                                                                                     \
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)                          \
		return {*err};                                                                     \
	if (isExecutionDrivenByGraph(pos)) {                                                       \
		auto *lab = genmc::dyn_cast<WriteLabel>(g.getEventLabel(pos));                     \
		if constexpr (!Config::emitNALabels) {                                             \
			if (OLD_VAL)                                                               \
				g.updateDeferredValue(lab->getAccess(), *OLD_VAL);                 \
			state.onATStore(lab->getPos(), lab->getAccess(),                           \
					getConsChecker().getHbView(lab), lab->isEffectful(),       \
					lab->isAtLeastRelease());                                  \
		}                                                                                  \
		return {lab->wasAddedMax(), 1U};                                                   \
	}                                                                                          \
	return handleStore(LabelT::create(pos, __VA_ARGS__), OLD_VAL);

#if EMIT_NA_LABELS
#define GENMC_OLD_VAL_PARAM
#define GENMC_OLD_VAL_PASS std::optional<SVal>()
#else
#define GENMC_OLD_VAL_PARAM std::optional<SVal> oldVal,
#define GENMC_OLD_VAL_PASS oldVal
#endif

#define HANDLE_LOAD_LABEL(name)                                                                    \
	GenMCDriver::HandleResult<SVal> GenMCDriver::handle##name(                                 \
		const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_PARAM MemOrdering ord,           \
		SAddr addr, ASize size, EventLabel *rfLab, std::optional<Annotation> annot,        \
		const EventDeps &deps)                                                             \
	{                                                                                          \
		LOAD_INTERCEPTOR_LOGIC(name##Label, GENMC_OLD_VAL_PASS, ord, addr, size, rfLab,    \
				       annot, deps);                                               \
	}

#define HANDLE_CAS_LOAD_LABEL(name)                                                                \
	GenMCDriver::HandleResult<SVal> GenMCDriver::handle##name(                                 \
		const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_PARAM MemOrdering ord,           \
		SAddr addr, ASize size, SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,   \
		std::optional<Annotation> annot, const EventDeps &deps)                            \
	{                                                                                          \
		LOAD_INTERCEPTOR_LOGIC(name##Label, GENMC_OLD_VAL_PASS, ord, addr, size, exp,      \
				       swap, wattr, rfLab, annot, deps);                           \
	}

#define HANDLE_LOCK_LOAD_LABEL(name)                                                               \
	GenMCDriver::HandleResult<SVal> GenMCDriver::handle##name(                                 \
		const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_PARAM SAddr addr, ASize size,    \
		std::optional<Annotation> annot, const EventDeps &deps)                            \
	{                                                                                          \
		LOAD_INTERCEPTOR_LOGIC(name##Label, GENMC_OLD_VAL_PASS, addr, size, annot, deps);  \
	}

#define HANDLE_FAI_LOAD_LABEL(name)                                                                \
	GenMCDriver::HandleResult<SVal> GenMCDriver::handle##name(                                 \
		const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_PARAM MemOrdering ord,           \
		SAddr addr, ASize size, RMWBinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab, \
		std::optional<Annotation> annot, const EventDeps &deps)                            \
	{                                                                                          \
		LOAD_INTERCEPTOR_LOGIC(name##Label, GENMC_OLD_VAL_PASS, ord, addr, size, op, val,  \
				       wattr, rfLab, annot, deps);                                 \
	}

#define HANDLE_STORE_LABEL(name)                                                                   \
	GenMCDriver::HandleResult<bool> GenMCDriver::handle##name(                                 \
		const EventDbgInfo *dbg, Event pos, GENMC_OLD_VAL_PARAM MemOrdering ord,           \
		SAddr addr, ASize size, SVal val, WriteAttr wattr, const EventDeps &deps)          \
	{                                                                                          \
		STORE_INTERCEPTOR_LOGIC(name##Label, GENMC_OLD_VAL_PASS, ord, addr, size, val,     \
					wattr, deps);                                              \
	}

#define HANDLE_LOCK_STORE_LABEL(name)                                                              \
	GenMCDriver::HandleResult<bool> GenMCDriver::handle##name(                                 \
		const EventDbgInfo *dbg, Event pos, SAddr addr, ASize size, const EventDeps &deps) \
	{                                                                                          \
		STORE_INTERCEPTOR_LOGIC(name##Label, std::optional<SVal>(), addr, size, deps);     \
	}

#define HANDLE_CAS_STORE_LABEL(name) HANDLE_STORE_LABEL(name)
#define HANDLE_FAI_STORE_LABEL(name) HANDLE_STORE_LABEL(name)

#include "genmc/Execution/EventLabel.def"

#undef GENMC_OLD_VAL_PARAM
#undef GENMC_OLD_VAL_PASS
#undef LOAD_INTERCEPTOR_LOGIC
#undef STORE_INTERCEPTOR_LOGIC

GenMCDriver::NALoadResult GenMCDriver::handleNALoad(Event pos, SAddr loc, ASize size)
{
	auto &g = getExec().getGraph();

	if (!readProbeLab_)
		readProbeLab_ = std::make_unique<ReadLabel>(pos, MemOrdering::NotAtomic, loc, size);
	configureProbe(readProbeLab_.get(), pos, loc, size);

	/* Cache the event before updating views (inits are added w/ tcreate) */
	if (getConf()->instructionCaching && inVerificationMode())
		getScheduler().cacheEventLabel(g, readProbeLab_.get());

	auto guard = g.addScoped(readProbeLab_);
	auto *lab = guard.get();

	/* Update views */
	updateLabelViews(lab);

	auto err = checkAccessValidity(lab->getPos(), lab->getAccess())
			   .or_else([&] { return checkInitializedMem(lab); })
			   .or_else([&] { return checkForRaces(lab); });
	if (err) {
		guard.commit();
		return {*err};
	}

	g.getState().onNALoad(pos, {loc, size}, getConsChecker().getHbView(lab));
#if EMIT_NA_LABELS
	lab->setRf(g.co_max(lab->getAddr()));
	guard.commit();
	return getReadRetValue(lab);
#else
	return {std::monostate(), 0U};
#endif
}

GenMCDriver::NALoadResult GenMCDriver::handleNALoad(const EventDbgInfo *dbg, Event pos, SAddr loc,
						    ASize size, const EventDeps &deps)
{
	auto &g = getExec().getGraph();
	auto &state = g.getState();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, true, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		/* TODO: case analysis for view calculation (+updateIdx?) */
#if EMIT_NA_LABELS
		return getReadRetValue(g.getReadLabel(pos));
#else
		state.onNALoad(pos, {loc, size},
			       getConsChecker().getHbView(g.getEventLabel(pos.prev())));
		return {std::monostate(), 0U};
#endif
	}
	return handleNALoad(pos, loc, size);
}

GenMCDriver::NAStoreResult GenMCDriver::handleNAStore(Event pos, SAddr loc, ASize size,
						      std::optional<SVal> val)
{
	auto &g = getExec().getGraph();

	if (!writeProbeLab_)
		writeProbeLab_ = std::make_unique<WriteLabel>(pos, MemOrdering::NotAtomic, loc,
							      size, SVal(0));
	configureProbe(writeProbeLab_.get(), pos, loc, size);
	if (val)
		writeProbeLab_->setVal(*val);

	/* Cache the event before updating views (inits are added w/ tcreate) */
	if (getConf()->instructionCaching && inVerificationMode())
		getScheduler().cacheEventLabel(g, writeProbeLab_.get());

	auto guard = g.addScoped(writeProbeLab_);
	auto *lab = guard.get();

	updateLabelViews(lab);

	auto err = checkAccessValidity(lab->getPos(), lab->getAccess()).or_else([&] {
		return checkForRaces(lab);
	});
	if (err) {
		guard.commit();
		return {*err};
	}

	g.getState().onNAStore(pos, {loc, size}, getConsChecker().getHbView(lab), val);
#if EMIT_NA_LABELS
	lab->addCo(g.co_max(lab->getAddr()));
	guard.commit();
	return {true, 1U};
#else
	return {std::monostate(), 0U};
#endif
}

#if EMIT_NA_LABELS
#define GENMC_NA_STORE_VAL_PARAM SVal val,
#else
#define GENMC_NA_STORE_VAL_PARAM
#endif

GenMCDriver::NAStoreResult
GenMCDriver::handleNAStore(const EventDbgInfo *dbg, Event pos, SAddr loc, ASize size,
			   GENMC_NA_STORE_VAL_PARAM const EventDeps &deps)
{
	auto &g = getExec().getGraph();
	auto &state = g.getState();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, true, dbg); err)
		return {*err};

#if EMIT_NA_LABELS
	const std::optional<SVal> naVal = val;
#else
	const std::optional<SVal> naVal = std::nullopt;
#endif

	if (isExecutionDrivenByGraph(pos)) {
		/* TODO: case analysis for view calculation (+updateIdx?) */
#if EMIT_NA_LABELS
		return {true, 1U};
#else
		state.onNAStore(pos, {loc, size},
				getConsChecker().getHbView(g.getEventLabel(pos.prev())), naVal);
		return {std::monostate(), 0U};
#endif
	}
	return handleNAStore(pos, loc, size, naVal);
}

static auto getRevisitable(WriteLabel *sLab, const VectorClock &before) -> std::vector<ReadLabel *>
{
	auto &g = *sLab->getParent();
	std::vector<ReadLabel *> loads;

	/* Helper function to erase loads in between conflicting RMWs */
	auto eraseConflictingLoads = [](auto &sLab, auto &loads) {
		auto *confLab = findPendingRMW(sLab);
		if (!confLab)
			return;
		loads.erase(std::remove_if(loads.begin(), loads.end(),
					   [&](auto &eLab) {
						   return eLab->getStamp() > confLab->getStamp();
					   }),
			    loads.end());
	};

	/* Fastpath: previous co-max is ppo-before SLAB */
	auto prevCoMaxIt = std::ranges::find_if(
		g.rco(sLab->getAddr()), [&](auto &lab) { return lab.getPos() != sLab->getPos(); });
	if (prevCoMaxIt != std::ranges::end(g.rco(sLab->getAddr())) &&
	    before.contains(prevCoMaxIt->getPos())) {
		for (auto &rLab : prevCoMaxIt->readers()) {
			if (!rLab.isStable() && !before.contains(rLab.getPos()))
				loads.push_back(&rLab);
		}
		eraseConflictingLoads(sLab, loads);
		return loads;
	}

	/* Slowpath: iterate over all same-location reads */
	for (auto it = ++ExecutionGraph::reverse_label_iterator(sLab);
	     it != ExecutionGraph::reverse_label_iterator(g.getInitLabel()); ++it) {
		auto *rLab = genmc::dyn_cast<ReadLabel>(&*it);
		if (rLab && rLab->getAddr() == sLab->getAddr() && !rLab->isStable() &&
		    !before.contains(rLab->getPos()))
			loads.push_back(rLab);
	}
	eraseConflictingLoads(sLab, loads);
	return loads;
}

std::vector<ReadLabel *> GenMCDriver::getRevisitableApproximation(WriteLabel *sLab)
{
	auto &g = getExec().getGraph();
	const auto &prefix = getPrefixView(sLab);
	auto loads = getRevisitable(sLab, prefix);
	getConsChecker().filterCoherentRevisits(sLab, loads);
	std::ranges::sort(loads, [&g](auto &lab1, auto &lab2) {
		return lab1->getStamp() > lab2->getStamp();
	});
	return loads;
}

EventLabel *GenMCDriver::pickRandomCo(WriteLabel *sLab, std::vector<EventLabel *> &cos)
{
	auto &g = getExec().getGraph();

	sLab->addCo(cos.back());
	cos.erase(std::remove_if(cos.begin(), cos.end() - 1,
				 [&](auto &wLab) {
					 sLab->moveCo(wLab);
					 return !isExecutionValid(sLab);
				 }),
		  cos.end() - 1);

	/* Extensibility is not guaranteed if an RMW read is not reading maximally
	 * (during estimation, reads read from arbitrary places anyway).
	 * If that is the case, we have to ensure that estimation won't stop. */
	if (cos.empty()) {
		VERIFY(sLab->isRMW());
		getExec().getWorkqueue().add(std::make_unique<RerunForwardRevisit>());
		return nullptr;
	}

	MyDist dist(0, cos.size() - 1);
	auto random = dist(estRng);
	sLab->moveCo(cos[random]);
	return cos[random];
}

void GenMCDriver::calcCoOrderings(WriteLabel *lab, const std::vector<EventLabel *> &cos)
{
	for (auto &predLab : cos) {
		getExec().getWorkqueue().add(
			std::make_unique<WriteForwardRevisit>(lab->getPos(), predLab->getPos()));
	}
}

GenMCDriver::HandleResult<bool> GenMCDriver::handleStore(std::unique_ptr<WriteLabel> wLab,
							 std::optional<SVal> oldVal)
{
	auto &g = getExec().getGraph();

	auto *lab = genmc::dyn_cast<WriteLabel>(addLabelToGraph(std::move(wLab)));

	/* Stores cannot cause atomicity violation:
	 * - In normal mode, non-maximal RMW are completed elsewhere
	 * - In estimation mode, we have already filtered violations on the read part */
	auto err = checkAccessValidity(lab->getPos(), lab->getAccess())
			   .or_else([&] { return checkInitializedMem(lab); })
			   .or_else([&] { return checkForMixedSize(lab); });
	if (err)
		return {*err};

	/* Runtime sends the NA value lazily when adding next atomic access */
	if (oldVal)
		g.updateDeferredValue(lab->getAccess(), *oldVal);

	checkReconsiderFaiSpinloop(lab);
	unblockWaitingHelping(lab);
	checkReconsiderReadOpts(lab);

	/* Find all possible placings in coherence for this store, and
	 * print a WW-race warning if appropriate (if this moots,
	 * exploration will anyway be cut) */
	auto cos = getConsChecker().getCoherentPlacings(lab);
	if (cos.size() > 1)
		reportWarningOnce(lab->getPos(), VerificationError::VE_WWRace, cos[0]);

	EventLabel *co = nullptr;
	if (inEstimationMode() || inRandomMode()) {
		co = pickRandomCo(lab, cos);
		if (inEstimationMode())
			getExec().getChoiceMap().update(lab, cos);
	} else {
		co = findConsistentCo(lab, cos);
		calcCoOrderings(lab, cos);
	}

	GENMC_DEBUG(LOG(VerbosityLevel::Debug2, "--- Added store {}\n{}", lab->getPos(),
			getExec().getGraph()););

	if (getScheduler().inErrorReplay())
		return {lab->wasAddedMax(), 1U};

	calcRevisits(lab);
	if (!co || violatesAtomicity(lab)) {
		moot();
		return {Invalid{}, 1U};
	}
	if (auto err = checkFinalAnnotations(lab).or_else([&] { return checkForRaces(lab); }); err)
		return {*err};

	VERIFY(!lab->isNotAtomic());
	g.getState().onATStore(lab->getPos(), lab->getAccess(), getConsChecker().getHbView(lab),
			       lab->isEffectful(), lab->isAtLeastRelease());
	return {lab->wasAddedMax(), 1U};
}

auto GenMCDriver::handleMalloc(Event pos, ASize size, uint64_t alignment, StorageDuration sdur,
			       StorageType styp, AddressSpace spc, const NameInfo *info,
			       const std::string &name, const EventDeps &deps) -> HandleResult<SVal>
{
	auto &g = getExec().getGraph();

	if (!mallocProbeLab_)
		mallocProbeLab_ = std::make_unique<MallocLabel>(pos, size.get(), alignment, sdur,
								styp, spc, info, name, deps);

	auto addr = g.getState().onAlloc(pos, size, alignment, sdur, styp, spc);
	configureProbe(mallocProbeLab_.get(), pos, addr, size);

	auto guard = g.addScoped(mallocProbeLab_);
	if constexpr (Config::emitNALabels)
		guard.commit();

	if (addr == SAddr()) {
		reportError(pos, {pos, VerificationError::VE_Allocation,
				  "Allocator is out of memory\n", nullptr});
		guard.commit();
		return {VerificationError::VE_Allocation, 1U};
	}
	return {SVal(addr.get()), Config::emitNALabels ? 1U : 0U};
}

auto GenMCDriver::handleMalloc(const EventDbgInfo *dbg, Event pos, ASize size, uint64_t alignment,
			       StorageDuration sdur, StorageType styp, AddressSpace spc,
			       const NameInfo *info, const std::string &name, const EventDeps &deps)
	-> HandleResult<SVal>
{
	auto &g = getExec().getGraph();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, true, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		SAddr addr;
		if constexpr (Config::emitNALabels) {
			auto *lab = genmc::dyn_cast<MallocLabel>(g.getEventLabel(pos));
			VERIFY(lab);
			auto oldAddr = lab->getAddr();
			addr = lab->getAddr();
			VERIFY(getConf()->isDepTrackingModel || oldAddr == SAddr() ||
			       oldAddr == lab->getAddr());
		} else {
			addr = g.getState().onAlloc(pos, size, alignment, sdur, styp, spc);
		}
		return {SVal(addr.get()), Config::emitNALabels ? 1U : 0U};
	}

	return handleMalloc(pos, size, alignment, sdur, styp, spc, info, name, deps);
}

auto GenMCDriver::handleRetire(Event pos, SAddr loc, const EventDeps &deps)
	-> HandleResult<std::monostate>
{
	auto &g = getExec().getGraph();

	if (!retireProbeLab_)
		retireProbeLab_ = std::make_unique<HpRetireLabel>(pos, loc, deps);

	configureProbe(retireProbeLab_.get(), pos, loc, g.getState().getAllocAccess(loc).size);

	auto guard = g.addScoped(retireProbeLab_);
	if constexpr (Config::emitNALabels)
		guard.commit();

	auto *lab = guard.get();
	if (auto err = checkForRaces(lab); err) {
		guard.commit();
		return {*err};
	}
	g.getState().onFree(pos, loc, true);
	return {std::monostate(), Config::emitNALabels ? 1U : 0U};
}

auto GenMCDriver::handleRetire(const EventDbgInfo *dbg, Event pos, SAddr loc, const EventDeps &deps)
	-> HandleResult<std::monostate>
{
	auto &g = getExec().getGraph();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, true, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		if constexpr (!Config::emitNALabels)
			g.getState().onFree(pos, loc, true);
		return {std::monostate(), Config::emitNALabels ? 1U : 0U};
	}

	return handleRetire(pos, loc, deps);
}

auto GenMCDriver::handleFree(Event pos, SAddr loc, const EventDeps &deps)
	-> HandleResult<std::monostate>
{
	auto &g = getExec().getGraph();

	auto size = g.getState().getAllocAccess(loc).size;
	if (!freeProbeLab_)
		freeProbeLab_ = std::make_unique<FreeLabel>(pos, loc, size, deps);

	configureProbe(freeProbeLab_.get(), pos, loc, size);

	auto guard = g.addScoped(freeProbeLab_);
	if constexpr (Config::emitNALabels)
		guard.commit();

	auto *lab = guard.get();
	if (auto err = checkForRaces(lab); err) {
		guard.commit();
		return {*err};
	}
	g.getState().onFree(pos, loc, false);
	return {std::monostate(), Config::emitNALabels ? 1U : 0U};
}

auto GenMCDriver::handleFree(const EventDbgInfo *dbg, Event pos, SAddr loc, const EventDeps &deps)
	-> HandleResult<std::monostate>
{
	auto &g = getExec().getGraph();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, true, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		if constexpr (!Config::emitNALabels)
			g.getState().onFree(pos, loc, false);
		return {std::monostate(), Config::emitNALabels ? 1U : 0U};
	}
	return handleFree(pos, loc, deps);
}

#if EMIT_NA_LABELS
auto GenMCDriver::allocateGlobal(ASize size, uint64_t alignment, const void *initData,
				 bool persistent, bool internal) -> SAddr
#else
auto GenMCDriver::allocateGlobal(ASize size, uint64_t alignment, bool persistent, bool internal)
	-> SAddr
#endif
{
	auto &state = getExec().getGraph().getState();
	auto addr = state.addStaticRange(size, alignment, persistent, internal);
#if EMIT_NA_LABELS
	state.addStaticInitData(addr, size, initData);
#endif
	return addr;
}

const MemAccessLabel *GenMCDriver::getPreviousVisibleAccessLabel(const EventLabel *start) const
{
	auto &g = getExec().getGraph();
	std::vector<Event> finalReads;

	for (const auto &lab : g.po_preds(start)) {
		if (auto *rLab = genmc::dyn_cast<ReadLabel>(&lab)) {
			if (rLab->isConfirming())
				continue;
			if (rLab->getRf()) {
				auto *wLab = genmc::dyn_cast<WriteLabel>(rLab->getRf());
				if (wLab && wLab->isLocal())
					continue;
				if (wLab && wLab->isFinal()) {
					finalReads.push_back(rLab->getPos());
					continue;
				}
				if (std::any_of(finalReads.begin(), finalReads.end(),
						[&](const Event &l) {
							auto *lLab = genmc::dyn_cast<ReadLabel>(
								g.getEventLabel(l));
							return lLab->getAddr() == rLab->getAddr() &&
							       lLab->getSize() == rLab->getSize();
						}))
					continue;
			}
			return rLab;
		}
		if (auto *wLab = genmc::dyn_cast<WriteLabel>(&lab))
			if (!wLab->isFinal() && !wLab->isLocal())
				return wLab;
	}
	return nullptr; /* none found */
}

void GenMCDriver::mootExecutionIfFullyBlocked(EventLabel *bLab)
{
	auto &g = getExec().getGraph();

	auto *lab = getPreviousVisibleAccessLabel(bLab);
	/* This part tries to match original check when NA are not present
	 * (but both checks are likely not sound when Local/Final attributes are present)
	 */
	if constexpr (!Config::emitNALabels) {
		auto previousNA = g.getState().getPreviousNA(lab->getThread());
		if (previousNA.first != 0 && (!lab || previousNA.first > lab->getIndex())) {
			/* Previous NA is a write */
			if (!previousNA.second)
				return;
			/* Previous NA is a read: check whether it is revisitable */
			auto *mLab =
				g.getEventLabel(Event(bLab->getThread(), previousNA.first - 1));
			VERIFY(mLab);
			if (!mLab->isRevisitable())
				moot();
			return;
		}
	}
	if (const auto *rLab = genmc::dyn_cast_if_present<ReadLabel>(lab))
		if (!rLab->isRevisitable() || !rLab->wasAddedMax())
			moot();
}

auto GenMCDriver::handleBlock(std::unique_ptr<BlockLabel> lab) -> HandleResult<std::monostate>
{
	/* Call addLabelToGraph first to cache the label */
	addLabelToGraph(lab->clone());
	blockThreadTryMoot(std::move(lab));
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleAssume(const EventDbgInfo *dbg, Event pos, AssumeType type)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleBlock(BlockLabel::createAssumeBlock(pos, type));
}

void GenMCDriver::initiateErrorReplay(const ErrorDetails &details)
{
	auto &g = getExec().getGraph();

	/* Initiate an exploration, and block the current one if it's a hard error */
	getExec().getWorkqueue().add(std::make_unique<ReplayForwardRevisit>(
		std::ranges::begin(g.rlabels())->getPos(), details));
	if (details.shouldHalt)
		moot();
}

void GenMCDriver::haltErrorReplay()
{
	getScheduler().setErrorReplayEvent(std::nullopt);
	dbgInfo_.clear();
	bufferedError_ = std::nullopt;

	/* The error-collecting execution finished; we have to drop it */
	moot();
}

static auto parseInstFromMData(int line, std::string absPath, const std::string &functionName)
	-> std::string
{
	/* If line is default-valued or malformed, skip... */
	if (line <= 0)
		return "";

	std::string result;

	return result;
}

static void recPrintTraceBefore(const GenMCDriver::GraphDbgInfo &dbgInfo, const EventLabel *eLab,
				View &a, std::ostream &ss /* std::cout */)
{
	const auto &g = *eLab->getParent();

	if (a.contains(eLab->getPos()))
		return;

	auto ai = a.getMax(eLab->getThread());
	a.setMax(eLab->getPos());
	for (int i = ai; i <= eLab->getIndex(); i++) {
		const auto *lab = g.getEventLabel(Event(eLab->getThread(), i));
		if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab))
			if (rLab->getRf())
				recPrintTraceBefore(dbgInfo, rLab->getRf(), a, ss);
		if (const auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(lab))
			recPrintTraceBefore(dbgInfo, g.getLastThreadLabel(jLab->getChildId()), a,
					    ss);
		if (const auto *bLab = genmc::dyn_cast<ThreadStartLabel>(lab))
			if (!bLab->getCreateId().isInitializer())
				recPrintTraceBefore(dbgInfo, bLab->getCreate(), a, ss);

		/* Do not print the line if it is an RMW write, since it will be
		 * the same as the previous one */
		if (genmc::isa<CasWriteLabel>(lab) || genmc::isa<FaiWriteLabel>(lab))
			continue;
		/* Similarly for a Wna just after the creation of a thread
		 * (it is the store of the PID) */
		if (i > 0 && genmc::isa<ThreadCreateLabel>(g.po_imm_pred(lab)))
			continue;

		if (dbgInfo.contains(lab->getPos())) {
			const auto &info = dbgInfo.at(lab->getPos());
			if (info.functionName != "")
				ss << "[" << info.functionName << "] ";
			ss << info.file << ": " << info.line << ": " << info.source << "\n";
		}
	}
}

static void printTraceBefore(const GenMCDriver::GraphDbgInfo &dbgInfo, const EventLabel *lab,
			     std::ostream &s /* = std::cerr */)
{
	if (dbgInfo.empty())
		return;

	s << std::format("Trace to {}:\n", lab->getPos());

	/* Linearize (po U rf) and print trace */
	View a;
	recPrintTraceBefore(dbgInfo, lab, a, s);
}

static void executeMDPrint(const EventLabel *lab, const GenMCDriver::EventDbgInfo &dbg,
			   std::ostream &os = std::cout)
{
	std::string errPath = dbg.file;
	genmc::extractFilename(errPath);
	os << " " << errPath << ":" << dbg.line;
}

/* Returns true if the corresponding LOC should be printed for this label type */
bool shouldPrintLOC(const EventLabel *lab)
{
	/* Begin/End labels don't have a corresponding LOC */
	if (genmc::isa<ThreadStartLabel>(lab) || genmc::isa<ThreadFinishLabel>(lab))
		return false;

	/* Similarly for allocations that don't come from malloc() */
	if (auto *mLab = genmc::dyn_cast<MallocLabel>(lab))
		return mLab->getAddr().isHeap() && !mLab->getAddr().isInternal();
	return true;
}

std::string printVarName(const MemAccessLabel &lab, const GenMCDriver::GraphDbgInfo &dbgInfo)
{
	auto &g = *lab.getParent();
	if (!lab.getAddr().isStatic() && !g.getState().isAllocated(lab.getAddr()))
		return "???";
	return dbgInfo.contains(lab.getPos()) ? dbgInfo.at(lab.getPos()).accessedVarName : "";
}

/** Outputs the full graph.
 * If printMetadata is set, it outputs debugging information
 * (these should have been collected beforehand) */
static void printGraph(const ExecutionGraph &g, const GenMCDriver::GraphDbgInfo &dbgInfo,
		       std::ostream &s /* = std::cerr */)
{
	LabelPrinter printer(
		[&dbgInfo](const MemAccessLabel &lab) { return printVarName(lab, dbgInfo); },
		[](const ReadLabel &lab) {
			return lab.getRf() ? lab.getAccessValue(lab.getAccess()) : SVal();
		});

	/* Print the graph */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		const auto &thrInfo = g.getFirstThreadLabel(i)->getThreadInfo();
		s << std::format("<{}, {}> {}", thrInfo.parentId, thrInfo.id, thrInfo.name);
		if (auto *bLab = g.getFirstThreadLabel(i)) {
			auto symm = bLab->getSymmPredTid();
			if (symm != -1)
				s << " symmetric with " << symm;
		}
		s << ":\n";
		for (auto &lab : g.po(i)) {
			if (genmc::isa<ThreadStartLabel>(&lab))
				continue;
			s << "\t" << printer.toString(lab);
			if (dbgInfo.contains(lab.getPos()) && shouldPrintLOC(&lab))
				executeMDPrint(&lab, dbgInfo.at(lab.getPos()), s);
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
			auto *wLab = &*std::ranges::begin(g.co(locIt->first));
			s << printVarName(*wLab, dbgInfo) << ": [ ";
			for (const auto &w : g.co(locIt->first))
				s << std::format("{} ", w);
			s << "]\n";
		}
	}
	s << "\n";
}

/** Outputs the current graph into a file (DOT format),
 * and visually marks events e and c (conflicting).
 * Assumes debugging information have already been collected  */
void dotPrintToFile(const std::string &filename, EventLabel *errLab,
		    std::unique_ptr<VectorClock> errView, const EventLabel *confLab,
		    std::unique_ptr<VectorClock> confView, const ConsistencyChecker *checker,
		    const GenMCDriver::GraphDbgInfo &dbgInfo, bool printObservation)
{
	auto &g = *errLab->getParent();

	std::ofstream ss(filename);
	if (!ss) {
		std::error_code ec = std::make_error_code(std::io_errc::stream);
		handleFSError(ec, "Failed to open dot file " + filename);
	}
	DotPrinter printer(
		[&dbgInfo](const MemAccessLabel &lab) { return printVarName(lab, dbgInfo); },
		[](const ReadLabel &lab) {
			return lab.getRf() ? lab.getAccessValue(lab.getAccess()) : SVal();
		});

	std::unique_ptr<VectorClock> before;
	if (&*errView)
		before = std::move(errView);
	else
		before = g.getViewFromStamp(g.getMaxStamp());
	if (confLab)
		before->update(*confView);

	/* Create a directed graph */
	ss << "strict digraph {\n";
	/* Specify node shape */
	ss << "node [shape=plaintext]\n";
	/* Left-justify labels for clusters */
	ss << "labeljust=l\n";
	/* Draw straight lines */
	ss << "splines=false\n";

	/* Print all nodes with each thread represented by a cluster */
	for (auto i = 0u; i < before->size(); i++) {
		bool inMethod = false;
		const auto &tInfo = g.getFirstThreadLabel(i)->getThreadInfo();
		ss << "subgraph cluster_" << i << "{\n";
		ss << "\tlabel=\"<" << tInfo.parentId << ", " << tInfo.id << "> " << tInfo.name
		   << ">\"\n";
		ss << "\ttooltip=\"thread #" << i << "\"\n";
		for (auto j = 1; j <= before->getMax(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));

			if (printObservation) {
				if (genmc::isa<MethodBeginLabel>(lab))
					inMethod = true;
				else if (genmc::isa<MethodEndLabel>(lab))
					inMethod = false;
				else if (inMethod)
					continue;
			}
			ss << std::format("\t\"{}\" [label=<", lab->getPos());

			/* First, print the graph label for this node */
			ss << printer.toString(*lab);

			/* And then, print the corresponding line number */
			if (dbgInfo.contains(lab->getPos()) && shouldPrintLOC(lab)) {
				ss << " <FONT COLOR=\"gray\">";
				executeMDPrint(lab, dbgInfo.at(lab->getPos()), ss);
				ss << "</FONT>";
			}
			ss << ">";

			if (errLab && lab->getPos() == errLab->getPos())
				ss << ", style=filled, fillcolor=yellow";
			if (confLab && lab->getPos() == confLab->getPos())
				ss << ", style=filled, fillcolor=yellow";

			ss << std::format(", tooltip=\"{}\"]\n", lab->getPos());
		}
		ss << "}\n";
	}

	/* Print relations between events (po U rf) */
	for (auto i = 0u; i < before->size(); i++) {
		bool inMethod = false;
		EventLabel const *lastLab = nullptr;
		for (auto j = 0; j <= before->getMax(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));

			if (printObservation) {
				if (genmc::isa<MethodBeginLabel>(lab))
					inMethod = true;
				else if (genmc::isa<MethodEndLabel>(lab))
					inMethod = false;
				else if (inMethod)
					continue;
			}

			/* Print a po-edge, but skip dummy start events for
			 * all threads except for the first one */
			if (lastLab)
				printlnDotEdge(ss, lastLab->getPos(), lab->getPos());
			if (!genmc::isa<ThreadStartLabel>(lab))
				lastLab = lab;

			if (auto *rLab = genmc::dyn_cast<ReadLabel>(lab)) {
				/* Do not print RFs from INIT, BOTTOM, and same thread */
				if (genmc::dyn_cast_if_present<WriteLabel>(rLab->getRf()) &&
				    rLab->getRf()->getThread() != lab->getThread()) {
					printlnDotEdge(
						ss, rLab->getRf()->getPos(), rLab->getPos(),
						{{"color", "green"}, {"constraint", "false"}});
				}
			}
			if (auto *bLab = genmc::dyn_cast<ThreadStartLabel>(lab)) {
				if (i == 0)
					continue;
				printlnDotEdge(ss, bLab->getCreate()->getPos(),
					       bLab->getPos().next(),
					       {{"color", "blue"}, {"constraint", "false"}});
			}
			if (auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(lab))
				printlnDotEdge(ss,
					       g.getLastThreadLabel(jLab->getChildId())->getPos(),
					       jLab->getPos(),
					       {{"color", "blue"}, {"constraint", "false"}});

			// print extension edges
			for (auto begLab : g.lin_succs(lab))
				printlnDotEdge(ss, lab->getPos(), begLab.getPos(),
					       {{"color", "red"}, {"constraint", "false"}});
		}
	}

	if (printObservation) {
		Observation obs(g, checker);

		for (auto const &[op1, op2] : obs.hb()) {
			auto src = obs.getCall(op1).beginLab->getPos();
			auto dst = obs.getCall(op2).endLab->getPos();
			if (src.thread == dst.thread)
				continue;
			printlnDotEdge(ss, src, dst, {{"color", "blue"}, {"constraint", "false"}});
		}
	}

	ss << "}\n";
}

void GenMCDriver::reportError(Event pos, ErrorDetails &&details)
{
	auto &g = getExec().getGraph();
	auto &scheduler = getScheduler();

	/* If anyone has already detected an error, no need to report another */
	if (isHalting())
		return;

	/* If under estimation, ignore soft errors; they're gonna be reported later on
	 * anyway */
	if (inEstimationMode() && !details.shouldHalt)
		return;

	/* If this is an error replay (e.g., when one instruction maps to many events, or
	 * under IMM), do not get into an infinite loop... */
	if (scheduler.inErrorReplay() && !scheduler.isErrorReplayEvent(pos))
		return;

	/* Before printing an error message, do an extra run to collect error metadata */
	if (!scheduler.inErrorReplay()) {
		initiateErrorReplay(details);
		return;
	}

	/* Metadata run is over: if the error is an invalid access, change the RF of the
	 * offending event to BOTTOM, so that we do not try to get its value. */
	auto *errLab = details.pos ? g.getEventLabel(*details.pos) : nullptr;
	if (errLab && isInvalidAccessError(details.type) && genmc::isa<ReadLabel>(errLab))
		genmc::dyn_cast<ReadLabel>(errLab)->setRf(nullptr);

	/* Print basic error message (graph) */
	std::ostringstream out;
	out << std::format("{}: {}!\n", isHardError(details.type) ? "Error" : "Warning",
			   details.type);
	if (errLab)
		out << std::format("Event {} ", errLab->getPos());
	if (details.racyLab != nullptr)
		out << std::format("conflicts with event {} ", details.racyLab->getPos());
	out << "in graph:\n";
	printGraph(g, dbgInfo_, out);

	/* Print an error trace (if desired), and the specific error message */
	if (getConf()->printErrorTrace && errLab) {
		printTraceBefore(dbgInfo_, errLab, out);
		if (details.racyLab != nullptr)
			printTraceBefore(dbgInfo_, details.racyLab, out);
	}
	if (!details.msg.empty())
		out << details.msg;
	result.message += out.str();

	/* Dump the graph into a file (DOT format) */
	if (!getConf()->dotFile.empty())
		dotPrintToFile(getConf()->dotFile, errLab,
			       errLab ? getPrefixView(errLab).clone() : nullptr, details.racyLab,
			       details.racyLab ? getPrefixView(details.racyLab).clone() : nullptr,
			       &getConsChecker(), dbgInfo_, getConf()->dotPrintOnlyClientEvents);

	/* Stop the error-collecting execution */
	haltErrorReplay();

	/* If this was a hard error, stop altogether */
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
			(getConf()->ipr && std::ranges::any_of(g.samelocs(lab), [&](auto &oLab) {
				 auto *rLab = genmc::dyn_cast<ReadLabel>(&oLab);
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
		reportError(pos, {pos, wcode, msg, racyLab, upgradeWarning});
	}
	if (knownWarnings.count(wcode) == 0)
		knownWarnings.insert(wcode);
	if (wcode == VerificationError::VE_WWRace)
		getExec().getGraph().getWriteLabel(pos)->setAttr(WriteAttr::WWRacy);
	return upgradeWarning;
}

bool GenMCDriver::checkBarrierWellFormedness(BIncFaiWriteLabel *sLab)
{
	if (getConf()->disableBAM)
		return true;

	/* Find the latest round completion (or init, if none exists) */
	auto &g = getExec().getGraph();
	auto lastIt = std::ranges::find_if(g.rco(sLab->getAddr()), [sLab](const auto &lab) {
		return &lab != sLab &&
		       ((genmc::isa<BIncFaiWriteLabel>(&lab) &&
			 isLastInBarrierRound(genmc::dyn_cast<BIncFaiWriteLabel>(&lab))) ||
			lab.getOrdering() == MemOrdering::Relaxed);
	});

	/* Check whether the last barrier completion is hb;po;po-before SLAB */
	auto ok = getConsChecker().getHbView(g.po_imm_pred(sLab)).contains(lastIt->getPos());
	if (!ok) {
		reportError(sLab->getPos(),
			    {sLab->getPos(), VerificationError::VE_BarrierWellFormedness,
			     "Execution not barrier-well-formed!\n"});
	}
	return ok;
}

bool GenMCDriver::tryOptimizeBarrierRevisits(BIncFaiWriteLabel *sLab,
					     std::vector<ReadLabel *> &loads)
{
	if (getConf()->disableBAM)
		return false;

	if (!checkBarrierWellFormedness(sLab) || !isLastInBarrierRound(sLab))
		return true;

	/* Find reads to revisit. `loads` is disregarded because it
	 * might not contain some valid revisits (e.g., discarded due to
	 * maximality-related optimizations) */
	auto &g = *sLab->getParent();
	auto *wLab = genmc::dyn_cast<ReadLabel>(g.po_imm_pred(sLab))->getRf();
	VERIFY(wLab);
	std::vector<ReadLabel *> toRevisit;

	while (genmc::isa<BIncFaiWriteLabel>(wLab) &&
	       !isLastInBarrierRound(genmc::dyn_cast<BIncFaiWriteLabel>(wLab))) {
		auto *nLab = g.po_imm_succ(wLab);
		if (nLab) {
			VERIFY(genmc::isa<BWaitReadLabel>(nLab));
			toRevisit.push_back(genmc::cast<ReadLabel>(nLab));
		}
		wLab = genmc::dyn_cast<ReadLabel>(g.po_imm_pred(wLab))->getRf();
	}

	/* Finally, revisit in place */
	for (auto *lab : toRevisit) {
		VERIFY(genmc::isa<BWaitReadLabel>(lab));
		revisitInPlace(*constructBackwardRevisit(lab, sLab));
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
						   !genmc::isa<CasReadLabel>(rLab) &&
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
	auto *confLab = findPendingRMW(sLab);
	if (!confLab)
		return;

	loads.erase(std::remove_if(loads.begin(), loads.end(),
				   [&](auto *rLab) {
					   auto *rfLab = rLab->getRf();
					   return rLab->getAnnot() && // must be like that
						  rfLab->getStamp() > rLab->getStamp() &&
						  !getPrefixView(sLab).contains(rfLab->getPos());
				   }),
		    loads.end());
}

bool GenMCDriver::removeCASReadIfBlocks(const ReadLabel *rLab, const EventLabel *sLab)
{
	auto &g = getExec().getGraph();
	/* This only affects annotated CASes */
	if (!rLab->getAnnot() || !genmc::isa<CasReadLabel>(rLab) ||
	    (!getConf()->ipr && !genmc::isa<LockCasReadLabel>(rLab)))
		return false;
	/* Skip if bounding is enabled */
	if (getConf()->bound.has_value())
		return false;

	/* If the CAS blocks, block thread altogether */
	auto val = sLab->getAccessValue(rLab->getAccess());
	if (rLab->valueMakesAssumeSucceed(val))
		return false;

	blockThread(g, ReadOptBlockLabel::create(rLab->getPos(), rLab->getAddr()));
	return true;
}

void GenMCDriver::checkReconsiderReadOpts(const WriteLabel *sLab)
{
	auto &g = getExec().getGraph();
	for (auto i = 0U; i < g.getNumThreads(); i++) {
		auto *bLab = genmc::dyn_cast_if_present<ReadOptBlockLabel>(g.getLastThreadLabel(i));
		if (!bLab || bLab->getAddr() != sLab->getAddr())
			continue;
		unblockThread(g, bLab->getPos());
	}
}

void GenMCDriver::optimizeUnconfirmedRevisits(const WriteLabel *sLab,
					      std::vector<ReadLabel *> &loads)
{
	if (!getConf()->confirmation)
		return;

	auto &g = getExec().getGraph();

	/* If there is already a write with the same value, report a possible ABA */
	auto valid = std::ranges::count_if(g.co(sLab->getAddr()), [&](auto &wLab) {
		return wLab.getPos() != sLab->getPos() && wLab.getVal() == sLab->getVal();
	});
	if (sLab->getAddr().isStatic() &&
	    g.getInitLabel()->getAccessValue(sLab->getAccess()) == sLab->getVal())
		++valid;
	WARN_ON_ONCE(valid > 0 && std::ranges::count_if(
					  loads, [](auto *lab) { return lab->isConfirming(); }),
		     "confirmation-aba-found",
		     "Possible ABA pattern! Consider running without -confirmation.");

	/* Do not bother with revisits that will be unconfirmed/lead to ABAs */
	loads.erase(std::remove_if(loads.begin(), loads.end(),
				   [&](auto *lab) {
					   if (!lab->isConfirming())
						   return false;

					   const EventLabel *scLab = nullptr;
					   auto *pLab = findMatchingSpeculativeRead(lab, scLab);
					   ERROR_ON(!pLab, "Confirming CAS annotation error! "
							   "Does a speculative read precede the "
							   "confirming operation?");

					   return !scLab;
				   }),
		    loads.end());
}

bool GenMCDriver::tryOptimizeRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &loads)
{
	auto &g = getExec().getGraph();

	/* BAM */
	if (!getConf()->disableBAM) {
		if (auto *faiLab = genmc::dyn_cast<BIncFaiWriteLabel>(sLab)) {
			if (tryOptimizeBarrierRevisits(faiLab, loads))
				return true;
		}
	}

	/* IPR */
	tryOptimizeIPRs(sLab, loads);

	/* Confirmation: Do not bother with revisits that will lead to unconfirmed reads */
	if (getConf()->confirmation)
		optimizeUnconfirmedRevisits(sLab, loads);
	return false;
}

void GenMCDriver::revisitInPlace(const BackwardRevisit &br)
{
	VERIFY(!getConf()->bound.has_value());

	auto &g = getExec().getGraph();
	auto *rLab = g.getReadLabel(br.getPos());
	auto *sLab = g.getWriteLabel(br.getRev());

	VERIFY(genmc::isa<ReadLabel>(rLab));
	if (g.po_imm_succ(rLab))
		g.removeLast(rLab->getThread());
	rLab->setRf(sLab);
	updateLabelViews(rLab);
	rLab->setAddedMax(true); /* explicitly set for atomicity violations */

	/* CASes shouldn't be handled via IPRs */
	VERIFY(!rLab->valueMakesRMWSucceed(rLab->getReturnValue()));

	GENMC_DEBUG(LOG(VerbosityLevel::Debug1, "--- In-place revisiting {} <-- {}\n{}",
			rLab->getPos(), sLab->getPos(), getExec().getGraph()););
}

void updatePredsWithPrefixView(const ExecutionGraph &g, VectorClock &preds,
			       const VectorClock &pporf)
{
	/* In addition to taking (preds U pporf), make sure pporf includes rfis */
	preds.update(pporf);

	if (!dynamic_cast<const DepExecutionGraph *>(&g))
		return;
	auto &predsD = *genmc::dyn_cast<DepView>(&preds);
	for (auto i = 0u; i < pporf.size(); i++) {
		for (auto j = 1; j <= pporf.getMax(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (auto *rLab = genmc::dyn_cast<ReadLabel>(lab)) {
				if (preds.contains(rLab->getPos()) &&
				    !preds.contains(rLab->getRf())) {
					if (rLab->getRf()->getThread() == rLab->getThread())
						predsD.removeHole(rLab->getRf()->getPos());
				}
			}
			auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
			if (wLab && wLab->isRMW() && pporf.contains(lab->getPos().prev()))
				predsD.removeHole(lab->getPos());
		}
	}
	return;
}

std::unique_ptr<VectorClock> GenMCDriver::getRevisitView(const ReadLabel *rLab,
							 const WriteLabel *sLab) const
{
	auto &g = getExec().getGraph();
	auto preds = g.getPredsView(rLab->getPos());

	updatePredsWithPrefixView(g, *preds, getPrefixView(sLab));
	return preds;
}

auto GenMCDriver::constructBackwardRevisit(const ReadLabel *rLab, const WriteLabel *sLab) const
	-> std::unique_ptr<BackwardRevisit>
{
	return std::make_unique<BackwardRevisit>(rLab, sLab, getRevisitView(rLab, sLab));
}

bool isFixedHoleInView(const ExecutionGraph &g, const EventLabel *lab, const DepView &v)
{
	if (auto *wLabB = genmc::dyn_cast<WriteLabel>(lab))
		return std::ranges::any_of(wLabB->readers(),
					   [&v](auto &oLab) { return v.contains(oLab.getPos()); });

	auto *rLabB = genmc::dyn_cast<ReadLabel>(lab);
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
		const auto *wLabB = g.getWriteLabel(rLabB->getPos().next());
		return std::ranges::any_of(wLabB->readers(),
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
	 * an event is "part" of WLAB's pporf view (even if it is not contained in it). */
	auto &g = getExec().getGraph();
	auto &v = *genmc::dyn_cast<DepView>(&getPrefixView(g.getEventLabel(r.getRev())));
	if (lab->getIndex() <= v.getMax(lab->getThread()) && isFixedHoleInView(g, lab, v))
		return true;
	return false;
}

bool GenMCDriver::isCoBeforeSavedPrefix(const BackwardRevisit &r, const EventLabel *lab)
{
	auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab);
	if (!mLab)
		return false;

	auto &g = getExec().getGraph();
	auto &v = r.getViewNoRel();
	auto rLab = genmc::dyn_cast<ReadLabel>(mLab);
	auto wLab = g.getWriteLabel(rLab ? rLab->getRf()->getPos() : mLab->getPos());

	auto succs = wLab ? g.co_succs(wLab) : g.co(mLab->getAddr());
	return std::ranges::any_of(succs, [&](auto &sLab) {
		/* Exclude the write that revisits from the prefix */
		return sLab.getPos() != r.getRev() && v->contains(sLab.getPos()) &&
		       (!getConf()->isDepTrackingModel ||
			mLab->getIndex() > getPrefixView(&sLab).getMax(mLab->getThread()));
	});
}

bool GenMCDriver::coherenceSuccRemainInGraph(const BackwardRevisit &r)
{
	auto &g = getExec().getGraph();
	auto *wLab = g.getWriteLabel(r.getRev());
	if (wLab->isRMW())
		return true;

	auto succs = g.co_succs(wLab);
	if (std::ranges::empty(succs))
		return true;

	return r.getViewNoRel()->contains(&*std::ranges::begin(succs));
}

bool wasAddedMaximally(const EventLabel *lab)
{
	if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab))
		return mLab->wasAddedMax();
	if (auto *oLab = genmc::dyn_cast<OptionalLabel>(lab))
		return !oLab->isExpanded();
	return true;
}

bool GenMCDriver::isMaximalExtension(const BackwardRevisit &r)
{
	/* Only revisit when the write's direct successor (if any) remains in the graph;
	 * revisits should not only differ on the write's placement */
	if (!coherenceSuccRemainInGraph(r))
		return false;

	auto &g = getExec().getGraph();
	auto &v = r.getViewNoRel();

	for (const auto &lab : g.labels()) {
		/* Exclude events unaffected by the revisit */
		if ((lab.getPos() != r.getPos() && v->contains(lab.getPos())) ||
		    prefixContainsSameLoc(r, &lab))
			continue;

		if (!lab.isRevisitable())
			return false;
		if (!wasAddedMaximally(&lab))
			return false;
		if (isCoBeforeSavedPrefix(r, &lab))
			return false;
	}
	return true;
}

std::unique_ptr<ExecutionGraph> GenMCDriver::copyGraph(const BackwardRevisit *br,
						       VectorClock *v) const
{
	auto &g = getExec().getGraph();

	/* Adjust the view that will be used for copying */
	auto &prefix = getPrefixView(g.getEventLabel(br->getRev()));
	auto og = g.getCopyUpTo(*v);

	/** Ensure the prefix of the write will not be revisitable.
	 * This is also used to check whether a write has revisited,
	 * and appropriately prevent some revisits */
	auto *revLab = og->getReadLabel(br->getPos());

	for (auto &lab : og->labels()) {
		if (prefix.contains(lab.getPos()))
			lab.setRevisitStatus(false);
	}
	return og;
}

void GenMCDriver::calcRevisits(WriteLabel *sLab)
{
	auto &g = getExec().getGraph();
	auto loads = getRevisitableApproximation(sLab);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug3, "Revisitable: {}", loads););
	if (tryOptimizeRevisits(sLab, loads))
		return;

	/* If operating in estimation/random mode, don't actually revisit */
	if (inEstimationMode() || inRandomMode()) {
		if (inEstimationMode())
			getExec().getChoiceMap().update(loads, sLab);
		return;
	}

	GENMC_DEBUG(LOG(VerbosityLevel::Debug3, "Revisitable (optimized): {}", loads););
	for (auto *rLab : loads) {
		auto br = constructBackwardRevisit(rLab, sLab);
		if (!isMaximalExtension(*br))
			break;

		getExec().getWorkqueue().add(std::move(br));
	}
}

auto GenMCDriver::completeRevisitedRMW(const ReadLabel *rLab) -> WriteLabel *
{
	auto wLab = createRMWWriteLabel(getExec().getGraph(), rLab);
	if (!wLab)
		return nullptr;

	auto *lab = genmc::dyn_cast<WriteLabel>(addLabelToGraph(std::move(wLab)));
	VERIFY(rLab->getRf());
	lab->addCo(rLab->getRf());
	return lab;
}

bool GenMCDriver::revisitWrite(const WriteForwardRevisit &ri)
{
	auto &g = getExec().getGraph();
	auto *wLab = g.getWriteLabel(ri.getPos());
	VERIFY(wLab);

	wLab->moveCo(g.getEventLabel(ri.getPred()));
	calcRevisits(wLab);
	return !violatesAtomicity(wLab);
}

bool GenMCDriver::revisitOptional(const OptionalForwardRevisit &oi)
{
	auto &g = getExec().getGraph();
	auto *oLab = genmc::dyn_cast<OptionalLabel>(g.getEventLabel(oi.getPos()));

	VERIFY(oLab);
	oLab->setExpandable(false);
	oLab->setExpanded(true);
	return true;
}

bool GenMCDriver::revisitRead(const Revisit &ri)
{
	VERIFY(genmc::isa<ReadRevisit>(&ri));

	/* We are dealing with a read: change its reads-from and also check
	 * whether a part of an RMW should be added */
	auto &g = getExec().getGraph();
	auto *rLab = g.getReadLabel(ri.getPos());
	auto *revLab = g.getEventLabel(genmc::dyn_cast<ReadRevisit>(&ri)->getRev());

	rLab->setRf(revLab);
	updateLabelViews(rLab);
	auto *fri = genmc::dyn_cast<ReadForwardRevisit>(&ri);

	GENMC_DEBUG(LOG(VerbosityLevel::Debug1, "--- {} revisiting {} <-- {}\n{}",
			(genmc::isa<BackwardRevisit>(&ri) ? "Backward" : "Forward"), ri.getPos(),
			revLab->getPos(), getExec().getGraph()););

	/* If the revisited read is a blocking CAS, try to remove it.
	 * We have to be careful not to do this in the "forward" path (when the CAS is
	 * re-scheduled): in that case, the CAS does need to block. This path doesn't typically
	 * go through revisitRead(). However, if we examine rfs in a non-standard order,
	 * it might (hence the first clause below). */
	if ((!fri || !fri->isMaximal()) && removeCASReadIfBlocks(rLab, revLab))
		return true;

	/* If the revisited label became an RMW, add the store part and revisit */
	if (auto *sLab = completeRevisitedRMW(rLab)) {
		calcRevisits(sLab);
		return !violatesAtomicity(sLab);
	}

	/* Blocked barrier or blocked lock: block thread */
	if (genmc::isa<BWaitReadLabel>(rLab) &&
	    !readsBarrierUnblockingValue(genmc::cast<BWaitReadLabel>(rLab)))
		blockThread(g, BarrierBlockLabel::create(rLab->getPos().next()));
	return true;
}

bool GenMCDriver::forwardRevisit(const ForwardRevisit &fr)
{
	auto &g = getExec().getGraph();
	auto *lab = g.getEventLabel(fr.getPos());
	if (auto *mi = genmc::dyn_cast<WriteForwardRevisit>(&fr))
		return revisitWrite(*mi);
	if (auto *oi = genmc::dyn_cast<OptionalForwardRevisit>(&fr))
		return revisitOptional(*oi);
	if (auto *rr = genmc::dyn_cast<RerunForwardRevisit>(&fr))
		return true;
	if (auto *rr = genmc::dyn_cast<ReplayForwardRevisit>(&fr)) {
		auto *errLab = g.getEventLabel(fr.getPos());
		getScheduler().setErrorReplayEvent(genmc::isa<BlockLabel>(errLab)
							   ? errLab->getPos().prev()
							   : errLab->getPos());
		bufferedError_ = rr->getDetails();
		return true;
	}
	auto *ri = genmc::dyn_cast<ReadForwardRevisit>(&fr);
	VERIFY(ri);
	return revisitRead(*ri);
}

bool GenMCDriver::backwardRevisit(const BackwardRevisit &br)
{
	auto &g = getExec().getGraph();

	/* Recalculate the view because some B labels might have been
	 * removed */
	auto v = getRevisitView(g.getReadLabel(br.getPos()), g.getWriteLabel(br.getRev()));

	auto og = copyGraph(&br, &*v);
	auto cmap = ChoiceMap(getExec().getChoiceMap());
	cmap.cut(*v);

	pushExecution({std::move(og), LocalQueueT(), std::move(cmap)});

	repairDanglingReads(getExec().getGraph());
	auto ok = revisitRead(br);
	VERIFY(ok);

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
	auto *br = genmc::dyn_cast<BackwardRevisit>(&*item);
	auto stamp = g.getEventLabel(br ? br->getRev() : item->getPos())->getStamp();
	getExec().restrict(stamp);
	repairDanglingReads(g);

	if (auto *fr = genmc::dyn_cast<ForwardRevisit>(&*item))
		return forwardRevisit(*fr);
	if (auto *br = genmc::dyn_cast<BackwardRevisit>(&*item)) {
		return backwardRevisit(*br);
	}
	UNREACHABLE();
	return false;
}

auto GenMCDriver::handleHelpingCas(std::unique_ptr<HelpingCasLabel> hLab)
	-> HandleResult<std::monostate>
{
	VERIFY(getConf()->helper);

	/* Ensure that the helped CAS exists */
	auto &g = getExec().getGraph();
	auto *lab = genmc::dyn_cast<HelpingCasLabel>(addLabelToGraph(std::move(hLab)));
	if (!checkHelpingCasCondition(lab)) {
		blockThread(g, HelpedCASBlockLabel::create(lab->getPos()));
		return {Reset{}, 0U};
	}
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleHelpingCas(const EventDbgInfo *dbg, Event pos, MemOrdering ord, SAddr loc,
				   ASize size, SVal cmpVal, SVal newVal, const EventDeps &deps)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleHelpingCas(HelpingCasLabel::create(pos, ord, loc, size, cmpVal, newVal, deps));
}

auto GenMCDriver::handleOptional(std::unique_ptr<OptionalLabel> lab) -> HandleResult<bool>
{
	auto &g = getExec().getGraph();

	if (std::any_of(g.label_begin(), g.label_end(), [&](auto &lab) {
		    auto *oLab = genmc::dyn_cast<OptionalLabel>(&lab);
		    return oLab && !oLab->isExpandable();
	    }))
		lab->setExpandable(false);

	auto *oLab = genmc::dyn_cast<OptionalLabel>(addLabelToGraph(std::move(lab)));

	/* Always expand in random exploration */
	if (inEstimationMode() || inRandomMode()) {
		oLab->setExpandable(false);
		oLab->setExpanded(true);
		return {true, 1U};
	}

	/* Otherwise, check whether we should expand */
	if (oLab->isExpandable()) {
		getExec().getWorkqueue().add(
			std::make_unique<OptionalForwardRevisit>(oLab->getPos()));
	}
	return {false, 1U}; /* not expanded in this exploration */
}

auto GenMCDriver::handleOptional(const EventDbgInfo *dbg, Event pos) -> HandleResult<bool>
{
	auto &g = getExec().getGraph();

	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {genmc::dyn_cast<OptionalLabel>(g.getEventLabel(pos))->isExpanded(), 1U};
	return handleOptional(OptionalLabel::create(pos));
}

auto GenMCDriver::handleSpinStart(std::unique_ptr<SpinStartLabel> lab)
	-> HandleResult<std::monostate>
{
	auto &g = getExec().getGraph();
	auto &frontier = g.getState();
	auto *stLab = addLabelToGraph(std::move(lab));

	/* Check for side-effects only if it's not the first spin-start */
	if (genmc::isa<LoopBeginLabel>(g.po_imm_pred(stLab)) ||
	    frontier.onSpinStart(stLab->getPos()))
		return {std::monostate(), 1U};

	/* Spinloop detected */
	blockThreadTryMoot(SpinloopBlockLabel::create(stLab->getPos()));
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleSpinStart(const EventDbgInfo *dbg, Event pos)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		if constexpr (!Config::emitNALabels)
			getExec().getGraph().getState().onSpinStart(pos);
		return {std::monostate(), 1U};
	}
	return handleSpinStart(SpinStartLabel::create(pos));
}

bool GenMCDriver::areFaiZNEConstraintsSat(const FaiZNESpinEndLabel *lab)
{
	auto &g = getExec().getGraph();

	/* Check that there are no other side-effects since the previous iteration.
	 * We don't have to look for a BEGIN label since ZNE labels are always
	 * preceded by a spin-start */
	auto preds = g.po_preds(lab);
	auto ssLabIt = std::ranges::find_if(
		preds, [](auto &lab) { return genmc::isa<SpinStartLabel>(&lab); });
	VERIFY(ssLabIt != preds.end());
	auto *ssLab = &*ssLabIt;
	for (auto i = ssLab->getIndex() + 1; i < lab->getIndex(); ++i) {
		auto *oLab = g.getEventLabel(Event(ssLab->getThread(), i));
		if (genmc::isa<WriteLabel>(oLab) && !genmc::isa<FaiWriteLabel>(oLab))
			return false;
	}

	auto wLabIt = std::ranges::find_if(
		preds, [](auto &lab) { return genmc::isa<FaiWriteLabel>(&lab); });
	VERIFY(wLabIt != preds.end());

	/* All stores in the RMW chain need to be read from at most 1 read,
	 * and there need to be no other stores that are not hb-before lab */
	auto *wLab = genmc::dyn_cast<FaiWriteLabel>(&*wLabIt);
	for (auto &lab : g.labels()) {
		if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(&lab)) {
			if (mLab->getAddr() == wLab->getAddr() && !genmc::isa<FaiReadLabel>(mLab) &&
			    !genmc::isa<FaiWriteLabel>(mLab) &&
			    !getConsChecker().getHbView(wLab).contains(mLab->getPos()))
				return false;
		}
	}
	return true;
}

auto GenMCDriver::handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel> lab)
	-> HandleResult<std::monostate>
{
	auto &g = getExec().getGraph();

	auto *zLab = genmc::dyn_cast<FaiZNESpinEndLabel>(addLabelToGraph(std::move(lab)));
	if (areFaiZNEConstraintsSat(zLab))
		blockThread(g, FaiZNEBlockLabel::create(zLab->getPos())); /* no moot desired */
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleFaiZNESpinEnd(const EventDbgInfo *dbg, Event pos)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleFaiZNESpinEnd(FaiZNESpinEndLabel::create(pos));
}

auto GenMCDriver::handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel> lab)
	-> HandleResult<std::monostate>
{
	auto *zLab = addLabelToGraph(std::move(lab));
	blockThreadTryMoot(LockZNEBlockLabel::create(zLab->getPos()));
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleLockZNESpinEnd(const EventDbgInfo *dbg, Event pos)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleLockZNESpinEnd(LockZNESpinEndLabel::create(pos));
}

auto GenMCDriver::handleDummy(std::unique_ptr<EventLabel> lab) -> HandleResult<std::monostate>
{
	addLabelToGraph(std::move(lab));
	return {std::monostate(), 1U};
}

auto GenMCDriver::handleLoopBegin(std::unique_ptr<LoopBeginLabel> lab)
	-> HandleResult<std::monostate>
{
	getExec().getGraph().getState().onLoopBegin(lab->getPos());
	return handleDummy(std::move(lab));
}

auto GenMCDriver::handleLoopBegin(const EventDbgInfo *dbg, Event pos)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		if constexpr (!Config::emitNALabels)
			getExec().getGraph().getState().onLoopBegin(pos);
		return {std::monostate(), 1U};
	}
	return handleLoopBegin(LoopBeginLabel::create(pos));
}
auto GenMCDriver::handleHpProtect(const EventDbgInfo *dbg, Event pos, SAddr hpAddr, SAddr protAddr)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos)) {
		if constexpr (!Config::emitNALabels)
			getExec().getGraph().getState().onProtect(hpAddr, protAddr);
		return {std::monostate(), 1U};
	}
	getExec().getGraph().getState().onProtect(hpAddr, protAddr);
	return handleDummy(HpProtectLabel::create(pos, hpAddr, protAddr));
}
auto GenMCDriver::handleMethodBegin(const EventDbgInfo *dbg, Event pos, std::string methodName,
				    int32_t argVal) -> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleDummy(MethodBeginLabel::create(pos, methodName, argVal));
}

auto GenMCDriver::handleMethodEnd(const EventDbgInfo *dbg, Event pos, std::string methodName,
				  int32_t retVal) -> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleDummy(MethodEndLabel::create(pos, methodName, retVal));
}

auto GenMCDriver::handleOutput(const EventDbgInfo *dbg, Event pos, std::string msg)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	return handleDummy(OutputLabel::create(pos, std::move(msg)));
}

auto GenMCDriver::handleError(const EventDbgInfo *dbg, Event pos, std::string msg)
	-> HandleResult<std::monostate>
{
	++pos;
	if (auto err = updateErrorInfoAndMaybeExit(pos, false, dbg); err)
		return {*err};
	if (isExecutionDrivenByGraph(pos))
		return {std::monostate(), 1U};
	auto result = handleDummy(ErrorLabel::create(pos, std::move(msg)));
	VERIFY(std::holds_alternative<std::monostate>(result.result));
	reportError(pos, {pos, VerificationError::VE_Safety, std::move(msg)});
	return {VerificationError::VE_Safety, 1U};
}

/************************************************************
 ** Printing facilities
 ***********************************************************/

std::optional<VerificationError> GenMCDriver::updateErrorInfoAndMaybeExit(Event pos, bool isNA,
									  const EventDbgInfo *dbg)
{
	/* If possible, update metadata */
	if (!getScheduler().inErrorReplay())
		return std::nullopt;
	if (dbg && (!isNA || Config::emitNALabels)) {
		VERIFY(!dbgInfo_.contains(pos));
		dbgInfo_[pos] = *dbg;
	}

	/* Check whether error replay is complete */
	if (bufferedError_ && getScheduler().isErrorReplayEvent(pos)) {
		auto code = bufferedError_.value().type;       /* will be erased */
		auto halt = bufferedError_.value().shouldHalt; /* will be erased */
		reportError(pos, std::move(*bufferedError_));
		return halt ? std::optional(code) : std::nullopt;
	}
	return std::nullopt;
}
const GenMCDriver::EventDbgInfo *GenMCDriver::getDbgInfo(Event pos)
{
	if (!dbgInfo_.contains(pos))
		return nullptr;
	return &dbgInfo_.at(pos);
}
