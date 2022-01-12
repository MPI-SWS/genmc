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
#include "RC11Driver.hpp"
#include "Interpreter.h"
#include "ExecutionGraph.hpp"
#include "PSCCalculator.hpp"
#include "PersistencyChecker.hpp"

RC11Driver::RC11Driver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		       std::unique_ptr<ModuleInfo> MI)
	: GenMCDriver(conf, std::move(mod), std::move(MI))
{
	auto &g = getGraph();

	/* RC11 requires the calculation of PSC */
	g.addCalculator(LLVM_MAKE_UNIQUE<PSCCalculator>(g),
			ExecutionGraph::RelationId::psc, false);
	return;
}

/* Calculates a minimal hb vector clock based on po for a given label */
View RC11Driver::calcBasicHbView(Event e) const
{
	View v(getGraph().getPreviousLabel(e)->getHbView());

	++v[e.thread];
	return v;
}

/* Calculates a minimal (po U rf) vector clock based on po for a given label */
View RC11Driver::calcBasicPorfView(Event e) const
{
	View v(getGraph().getPreviousLabel(e)->getPorfView());

	++v[e.thread];
	return v;
}

void RC11Driver::calcBasicViews(EventLabel *lab)
{
	const auto &g = getGraph();

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
}

void RC11Driver::calcReadViews(ReadLabel *lab)
{
	const auto &g = getGraph();
	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	if (!lab->getRf().isBottom()) {
		const auto *rfLab = g.getEventLabel(lab->getRf());
		porf.update(rfLab->getPorfView());
		if (lab->isAtLeastAcquire()) {
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
				hb.update(wLab->getMsgView());
		}
	}

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
}

void RC11Driver::calcWriteViews(WriteLabel *lab)
{
	calcBasicViews(lab);
	if (llvm::isa<FaiWriteLabel>(lab) || llvm::isa<CasWriteLabel>(lab))
		calcRMWWriteMsgView(lab);
	else
		calcWriteMsgView(lab);
}

void RC11Driver::calcWriteMsgView(WriteLabel *lab)
{
	const auto &g = getGraph();
	View msg;

	/* Should only be called with plain writes */
	BUG_ON(llvm::isa<FaiWriteLabel>(lab) || llvm::isa<CasWriteLabel>(lab));

	if (lab->isAtLeastRelease())
		msg = lab->getHbView();
	else if (lab->getOrdering() == llvm::AtomicOrdering::Monotonic ||
		 lab->getOrdering() == llvm::AtomicOrdering::Acquire)
		msg = g.getEventLabel(g.getLastThreadReleaseAtLoc(lab->getPos(),
								  lab->getAddr()))->getHbView();
	lab->setMsgView(std::move(msg));
}

void RC11Driver::calcRMWWriteMsgView(WriteLabel *lab)
{
	const auto &g = getGraph();
	View msg;

	/* Should only be called with RMW writes */
	BUG_ON(!llvm::isa<FaiWriteLabel>(lab) && !llvm::isa<CasWriteLabel>(lab));

	const EventLabel *pLab = g.getPreviousLabel(lab);

	BUG_ON(pLab->getOrdering() == llvm::AtomicOrdering::NotAtomic);
	BUG_ON(!llvm::isa<ReadLabel>(pLab));

	const ReadLabel *rLab = static_cast<const ReadLabel *>(pLab);
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(rLab->getRf())))
		msg.update(wLab->getMsgView());

	if (rLab->isAtLeastRelease())
		msg.update(lab->getHbView());
	else
		msg.update(g.getEventLabel(g.getLastThreadReleaseAtLoc(lab->getPos(),
								       lab->getAddr()))->getHbView());

	lab->setMsgView(std::move(msg));
}

void RC11Driver::calcFenceRelRfPoBefore(Event last, View &v)
{
	const auto &g = getGraph();
	for (auto i = last.index; i > 0; i--) {
		const EventLabel *lab = g.getEventLabel(Event(last.thread, i));
		if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire())
			return;
		if (!llvm::isa<ReadLabel>(lab))
			continue;
		auto *rLab = static_cast<const ReadLabel *>(lab);
		if (rLab->getOrdering() == llvm::AtomicOrdering::Monotonic ||
		    rLab->getOrdering() == llvm::AtomicOrdering::Release) {
			const EventLabel *rfLab = g.getEventLabel(rLab->getRf());
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
				v.update(wLab->getMsgView());
		}
	}
}


void RC11Driver::calcFenceViews(FenceLabel *lab)
{
	const auto &g = getGraph();
	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	if (lab->isAtLeastAcquire())
		calcFenceRelRfPoBefore(lab->getPos().prev(), hb);

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
}

void RC11Driver::calcStartViews(ThreadStartLabel *lab)
{
	const auto &g = getGraph();

	/* Thread start has Acquire semantics */
	View hb(g.getEventLabel(lab->getParentCreate())->getHbView());
	View porf(g.getEventLabel(lab->getParentCreate())->getPorfView());

	hb[lab->getThread()] = lab->getIndex();
	porf[lab->getThread()] = lab->getIndex();

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
}

void RC11Driver::calcJoinViews(ThreadJoinLabel *lab)
{
	const auto &g = getGraph();
	auto *fLab = g.getLastThreadLabel(lab->getChildId());

	/* Thread joins have acquire semantics -- but we have to wait
	 * for the other thread to finish before synchronizing */
	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	if (llvm::isa<ThreadFinishLabel>(fLab)) {
		hb.update(fLab->getHbView());
		porf.update(fLab->getPorfView());
	}

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
}

void RC11Driver::updateLabelViews(EventLabel *lab, const EventDeps *deps) /* deps ignored */
{
	const auto &g = getGraph();

	switch (lab->getKind()) {
	case EventLabel::EL_Read:
	case EventLabel::EL_BWaitRead:
	case EventLabel::EL_DskRead:
	case EventLabel::EL_CasRead:
	case EventLabel::EL_LockCasRead:
	case EventLabel::EL_FaiRead:
	case EventLabel::EL_BIncFaiRead:
		calcReadViews(llvm::dyn_cast<ReadLabel>(lab));
		if (getConf()->persevere && llvm::isa<DskReadLabel>(lab))
			g.getPersChecker()->calcDskMemAccessPbView(llvm::dyn_cast<DskReadLabel>(lab));
		break;
	case EventLabel::EL_Write:
	case EventLabel::EL_BInitWrite:
	case EventLabel::EL_BDestroyWrite:
	case EventLabel::EL_UnlockWrite:
	case EventLabel::EL_CasWrite:
	case EventLabel::EL_LockCasWrite:
	case EventLabel::EL_FaiWrite:
	case EventLabel::EL_BIncFaiWrite:
	case EventLabel::EL_DskWrite:
	case EventLabel::EL_DskMdWrite:
	case EventLabel::EL_DskDirWrite:
	case EventLabel::EL_DskJnlWrite:
		calcWriteViews(llvm::dyn_cast<WriteLabel>(lab));
		if (getConf()->persevere && llvm::isa<DskWriteLabel>(lab))
			g.getPersChecker()->calcDskMemAccessPbView(llvm::dyn_cast<DskWriteLabel>(lab));
		break;
	case EventLabel::EL_Fence:
	case EventLabel::EL_DskFsync:
	case EventLabel::EL_DskSync:
	case EventLabel::EL_DskPbarrier:
		calcFenceViews(llvm::dyn_cast<FenceLabel>(lab));
		if (getConf()->persevere && llvm::isa<DskAccessLabel>(lab))
			g.getPersChecker()->calcDskFencePbView(llvm::dyn_cast<FenceLabel>(lab));
		break;
	case EventLabel::EL_ThreadStart:
		calcStartViews(llvm::dyn_cast<ThreadStartLabel>(lab));
		break;
	case EventLabel::EL_ThreadJoin:
		calcJoinViews(llvm::dyn_cast<ThreadJoinLabel>(lab));
		break;
	case EventLabel::EL_ThreadCreate:
	case EventLabel::EL_ThreadFinish:
	case EventLabel::EL_LoopBegin:
	case EventLabel::EL_SpinStart:
	case EventLabel::EL_FaiZNESpinEnd:
	case EventLabel::EL_LockZNESpinEnd:
	case EventLabel::EL_Malloc:
	case EventLabel::EL_Free:
	case EventLabel::EL_LockLabelLAPOR:
	case EventLabel::EL_UnlockLabelLAPOR:
	case EventLabel::EL_DskOpen:
		calcBasicViews(lab);
		break;
	case EventLabel::EL_SmpFenceLKMM:
		ERROR("LKMM fences can only be used with -lkmm!\n");
		break;
	case EventLabel::EL_RCULockLKMM:
	case EventLabel::EL_RCUUnlockLKMM:
	case EventLabel::EL_RCUSyncLKMM:
		ERROR("RCU primitives can only be used with -lkmm!\n");
		break;
	default:
		BUG();
	}
}

bool RC11Driver::areInDataRace(const MemAccessLabel *aLab, const MemAccessLabel *bLab)
{
	/* If there is an HB ordering between the two events, there is no race */
	if (isHbBefore(aLab->getPos(), bLab->getPos()) ||
	    isHbBefore(bLab->getPos(), aLab->getPos()) || aLab == bLab)
		return false;

	/* If both accesses are atomic, there is no race */
	/* Note: one check suffices because a variable is either
	 * atomic or not atomic, but we have not checked the address yet */
	if (!aLab->isNotAtomic() && !bLab->isNotAtomic())
		return false;

	/* If they access a different address, there is no race */
	if (aLab->getAddr() != bLab->getAddr())
		return false;

	/* If LAPOR is disabled, we are done */
	if (!getConf()->LAPOR)
		return true;

	/* Otherwise, we have to make sure that the two accesses do _not_
	 * belong in critical sections of the same lock */
	const auto &g = getGraph();
	auto aLock = g.getLastThreadUnmatchedLockLAPOR(aLab->getPos());
	auto bLock = g.getLastThreadUnmatchedLockLAPOR(bLab->getPos());

	/* If any of the two is _not_ in a CS, it is a race */
	if (aLock.isInitializer() || bLock.isInitializer())
		return true;

	/* If both are in a CS, being in CSs of different locks is a race */
	return llvm::dyn_cast<LockLabelLAPOR>(g.getEventLabel(aLock))->getLockAddr() !=
	       llvm::dyn_cast<LockLabelLAPOR>(g.getEventLabel(bLock))->getLockAddr();
}

Event RC11Driver::findRaceForNewLoad(const ReadLabel *rLab)
{
	const auto &g = getGraph();
	const View &before = g.getPreviousNonEmptyLabel(rLab)->getHbView();
	const auto &stores = g.getStoresToLoc(rLab->getAddr());

	/* If there are not any events hb-before the read, there is nothing to do */
	if (before.empty())
		return Event::getInitializer();

	/* Check for events that race with the current load */
	for (auto &s : stores) {
		if (before.contains(s))
			continue;

		auto *sLab = static_cast<const WriteLabel *>(g.getEventLabel(s));
		if (areInDataRace(rLab, sLab))
			return s; /* Race detected! */
	}
	return Event::getInitializer(); /* Race not found */
}

Event RC11Driver::findRaceForNewStore(const WriteLabel *wLab)
{
	const auto &g = getGraph();
	auto &before = g.getPreviousNonEmptyLabel(wLab)->getHbView();

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = before[i] + 1u; j < g.getThreadSize(i); j++) {
			const EventLabel *oLab = g.getEventLabel(Event(i, j));
			if (!llvm::isa<MemAccessLabel>(oLab))
				continue;

			auto *mLab = static_cast<const MemAccessLabel *>(oLab);
			if (areInDataRace(wLab, mLab))
				return mLab->getPos(); /* Race detected */
		}
	}
	return Event::getInitializer(); /* Race not found */
}

Event RC11Driver::findDataRaceForMemAccess(const MemAccessLabel *mLab)
{
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(mLab))
		return findRaceForNewLoad(rLab);
	else if (auto *wLab = llvm::dyn_cast<WriteLabel>(mLab))
		return findRaceForNewStore(wLab);
	BUG();
	return Event::getInitializer();
}

void RC11Driver::changeRf(Event read, Event store)
{
	auto &g = getGraph();

	/* Change the reads-from relation in the graph */
	g.changeRf(read, store);

	/* And update the views of the load */
	auto *rLab = static_cast<ReadLabel *>(g.getEventLabel(read));
	calcReadViews(rLab);
	if (getConf()->persevere && llvm::isa<DskReadLabel>(rLab))
		g.getPersChecker()->calcDskMemAccessPbView(rLab);
	return;
}

void RC11Driver::updateStart(Event create, Event start)
{
	auto &g = getGraph();
	auto *bLab = g.getEventLabel(start);

	View hb(g.getEventLabel(create)->getHbView());
	View porf(g.getEventLabel(create)->getPorfView());

	hb[start.thread] = 0;
	porf[start.thread] = 0;

	bLab->setHbView(std::move(hb));
	bLab->setPorfView(std::move(porf));
	return;
}

bool RC11Driver::updateJoin(Event join, Event childLast)
{
	auto &g = getGraph();

	if (!g.updateJoin(join, childLast))
		return false;

	EventLabel *jLab = g.getEventLabel(join);
	EventLabel *fLab = g.getEventLabel(childLast);

       /* Since the pporf view may contain elements from threads joined
	* in previous explorations, we have to reset it to a po-local one */
	View porf = calcBasicPorfView(jLab->getPos());
	View hb = calcBasicHbView(jLab->getPos());

	hb.update(fLab->getHbView());
	porf.update(fLab->getPorfView());

        jLab->setHbView(std::move(hb));
	jLab->setPorfView(std::move(porf));
	return true;
}

void RC11Driver::initConsCalculation()
{
	return;
}
