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
#include "IMMDriver.hpp"
#include "Interpreter.h"
#include "ExecutionGraph.hpp"
#include "ARCalculator.hpp"
#include "PSCCalculator.hpp"
#include "PersistencyChecker.hpp"

IMMDriver::IMMDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		     std::unique_ptr<ModuleInfo> MI)
	: GenMCDriver(conf, std::move(mod), std::move(MI))
{
	auto &g = getGraph();

	/* IMM requires acyclicity checks for both PSC and AR */
	g.addCalculator(std::make_unique<PSCCalculator>(g),
			ExecutionGraph::RelationId::psc, false);
	g.addCalculator(std::make_unique<ARCalculator>(g),
			ExecutionGraph::RelationId::ar, false);
	return;
}

/* Calculates a minimal hb vector clock based on po for a given label */
View IMMDriver::calcBasicHbView(Event e) const
{
	View v(getGraph().getPreviousLabel(e)->getHbView());

	++v[e.thread];
	return v;
}

DepView IMMDriver::getDepsAsView(const EventDeps *deps)
{
	DepView v;

	if (!deps)
		return v;

	auto &g = getGraph();
	for (auto &adep : deps->addr)
		v.update(g.getPPoRfBefore(adep));
	for (auto &ddep : deps->data)
		v.update(g.getPPoRfBefore(ddep));
	for (auto &cdep : deps->ctrl)
		v.update(g.getPPoRfBefore(cdep));
	for (auto &apdep : deps->addrPo)
		v.update(g.getPPoRfBefore(apdep));
	for (auto &csdep : deps->cas)
		v.update(g.getPPoRfBefore(csdep));
	return v;
}

DepView IMMDriver::calcPPoView(Event e, const EventDeps *deps) /* not const */
{
	/* Update ppo based on dependencies (addr, data, ctrl, addr;po, cas) */
	auto v = getDepsAsView(deps);

	/* This event does not depend on anything else */
	DepView wv;
	wv[e.thread] = e.index;
	wv.addHolesInRange(Event(e.thread, 0), e.index);
	v.update(wv);

	/* Update based on the views of the acquires of the thread */
	auto &g = getGraph();
	for (auto &ev : g.getThreadAcquiresAndFences(e))
		v.update(g.getPPoRfBefore(ev));
	return v;
}

void IMMDriver::updateRelView(DepView &pporf, EventLabel *lab)
{
	if (!lab->isAtLeastRelease())
		return;

	const auto &g = getGraph();

	pporf.removeAllHoles(lab->getThread());

	Event rel = g.getLastThreadRelease(lab->getPos());
	pporf.update(g.getEventLabel(rel)->getPPoRfView());

	if (llvm::isa<FaiWriteLabel>(g.getEventLabel(rel)) ||
	    llvm::isa<CasWriteLabel>(g.getEventLabel(rel)))
		--rel.index;
	for (auto i = rel.index; i < lab->getIndex(); i++) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(
			    g.getEventLabel(Event(lab->getThread(), i)))) {
			pporf.update(rLab->getPPoRfView());
		}
	}
	return;
}

void IMMDriver::calcBasicViews(EventLabel *lab, const EventDeps *deps)
{
	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos(), deps);

	if (lab->isAtLeastRelease())
		updateRelView(pporf, lab);

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
}

void IMMDriver::updateReadViewsFromRf(DepView &pporf, View &hb, const ReadLabel *lab)
{
	if (lab->getRf().isBottom())
		return;

	auto &g = getGraph();
	const EventLabel *rfLab = g.getEventLabel(lab->getRf());

	if (rfLab->getThread() == lab->getThread() || rfLab->getPos().isInitializer()) {
		auto rfiDepend = pporf.contains(rfLab->getPos());
		pporf.update(rfLab->getPPoView()); /* Account for dep; rfi dependencies */
		if (!rfiDepend) /* Should we depend on rfi? */
			pporf.addHole(rfLab->getPos());
	} else {
		pporf.update(rfLab->getPPoRfView());
		for (auto i = 0u; i < lab->getIndex(); i++) {
			auto *eLab = g.getEventLabel(Event(lab->getThread(), i));
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(eLab)) {
				if (wLab->getAddr() == lab->getAddr())
					pporf.update(wLab->getPPoRfView());
			}
		}
	}
	if (lab->isAtLeastAcquire()) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
			hb.update(wLab->getMsgView());
	}
	return;
}

void IMMDriver::calcReadViews(ReadLabel *lab, const EventDeps *deps)
{
	const auto &g = getGraph();
	View hb = calcBasicHbView(lab->getPos());
	DepView ppo = calcPPoView(lab->getPos(), deps);
	DepView pporf(ppo);

	updateReadViewsFromRf(pporf, hb, lab);

	lab->setHbView(std::move(hb));
	lab->setPPoView(std::move(ppo));
	lab->setPPoRfView(std::move(pporf));
}

void IMMDriver::calcWriteViews(WriteLabel *lab, const EventDeps *deps)
{
	const auto &g = getGraph();

	/* First, we calculate the hb and porf view */
	View hb = calcBasicHbView(lab->getPos());
	lab->setHbView(std::move(hb));

	/* Then, we calculate the ppo and (ppo U rf) views.
	 * The first is important because we have to take dep;rfi
	 * dependencies into account for subsequent reads. */
	DepView ppo = calcPPoView(lab->getPos(), deps);
	DepView pporf(ppo);

	if (llvm::isa<CasWriteLabel>(lab) || llvm::isa<FaiWriteLabel>(lab)) {
		ppo.update(g.getPreviousLabel(lab)->getPPoRfView());
		pporf.update(g.getPreviousLabel(lab)->getPPoRfView());
	}
	if (lab->isAtLeastRelease())
		updateRelView(pporf, lab);
	pporf.update(g.getPPoRfBefore(g.getLastThreadReleaseAtLoc(lab->getPos(),
								  lab->getAddr())));
	lab->setPPoView(std::move(ppo));
	lab->setPPoRfView(std::move(pporf));

	/* Finally, calculate the write's message views */
	if (llvm::isa<CasWriteLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
		calcRMWWriteMsgView(lab);
	else
		calcWriteMsgView(lab);
}

void IMMDriver::calcWriteMsgView(WriteLabel *lab)
{
	const auto &g = getGraph();
	View msg;

	/* Should only be called with plain writes */
	BUG_ON(llvm::isa<FaiWriteLabel>(lab) || llvm::isa<CasWriteLabel>(lab));

	if (lab->isAtLeastRelease())
		msg = lab->getHbView();
	else if (lab->isAtMostAcquire())
		msg = g.getEventLabel(
			g.getLastThreadReleaseAtLoc(lab->getPos(), lab->getAddr()))->getHbView();
	lab->setMsgView(std::move(msg));
}

void IMMDriver::calcRMWWriteMsgView(WriteLabel *lab)
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

void IMMDriver::calcFenceRelRfPoBefore(Event last, View &v)
{
	const auto &g = getGraph();
	for (auto i = last.index; i > 0; i--) {
		const EventLabel *lab = g.getEventLabel(Event(last.thread, i));
		if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire())
			return;
		if (!llvm::isa<ReadLabel>(lab))
			continue;
		auto *rLab = static_cast<const ReadLabel *>(lab);
		if (rLab->isAtMostRelease()) {
			const EventLabel *rfLab = g.getEventLabel(rLab->getRf());
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
				v.update(wLab->getMsgView());
		}
	}
}


void IMMDriver::calcFenceViews(FenceLabel *lab, const EventDeps *deps)
{
	const auto &g = getGraph();
	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos(), deps);

	if (lab->isAtLeastAcquire())
		calcFenceRelRfPoBefore(lab->getPos().prev(), hb);
	if (lab->isAtLeastRelease())
		updateRelView(pporf, lab);

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
}

void IMMDriver::calcJoinViews(ThreadJoinLabel *lab, const EventDeps *deps)
{
	const auto &g = getGraph();
	auto *fLab = g.getLastThreadLabel(lab->getChildId());


       /* Since the pporf view may contain elements from threads joined
	* in previous explorations, we have to reset it to the ppo one,
	* and then update it */
	View hb = calcBasicHbView(lab->getPos());
	DepView ppo = calcPPoView(lab->getPos(), deps);
	DepView pporf(ppo);

	if (llvm::isa<ThreadFinishLabel>(fLab)) {
		hb.update(fLab->getHbView());
		pporf.update(fLab->getPPoRfView());
	}

	lab->setHbView(std::move(hb));
	lab->setPPoView(std::move(ppo));
	lab->setPPoRfView(std::move(pporf));
	return;
}

void IMMDriver::calcStartViews(ThreadStartLabel *lab)
{
	const auto &g = getGraph();

	/* Thread start has Acquire semantics */
	View hb(g.getEventLabel(lab->getParentCreate())->getHbView());
	DepView pporf(g.getEventLabel(lab->getParentCreate())->getPPoRfView());

	hb[lab->getThread()] = lab->getIndex();
	pporf[lab->getThread()] = lab->getIndex();

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return;
}

void IMMDriver::calcLockLAPORViews(LockLabelLAPOR *lab, const EventDeps *deps)
{
	const auto &g = getGraph();
	auto hb = calcBasicHbView(lab->getPos());
	auto pporf = calcPPoView(lab->getPos(), deps);

	auto prevUnlock = g.getLastThreadUnlockAtLocLAPOR(lab->getPos().prev(), lab->getLockAddr());
	if (!prevUnlock.isInitializer())
		pporf.update(g.getPPoRfBefore(prevUnlock));

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return;
}

void IMMDriver::updateLabelViews(EventLabel *lab, const EventDeps *deps)
{
	const auto &g = getGraph();

	switch (lab->getKind()) {
	case EventLabel::EL_Read:
	case EventLabel::EL_BWaitRead:
	case EventLabel::EL_SpeculativeRead:
	case EventLabel::EL_ConfirmingRead:
	case EventLabel::EL_DskRead:
	case EventLabel::EL_CasRead:
	case EventLabel::EL_LockCasRead:
	case EventLabel::EL_TrylockCasRead:
	case EventLabel::EL_HelpedCasRead:
	case EventLabel::EL_ConfirmingCasRead:
	case EventLabel::EL_FaiRead:
	case EventLabel::EL_BIncFaiRead:
		calcReadViews(llvm::dyn_cast<ReadLabel>(lab), deps);
		if (getConf()->persevere && llvm::isa<DskReadLabel>(lab))
			g.getPersChecker()->calcDskMemAccessPbView(llvm::dyn_cast<DskReadLabel>(lab));
		break;
	case EventLabel::EL_Write:
	case EventLabel::EL_BInitWrite:
	case EventLabel::EL_BDestroyWrite:
	case EventLabel::EL_UnlockWrite:
	case EventLabel::EL_CasWrite:
	case EventLabel::EL_LockCasWrite:
	case EventLabel::EL_TrylockCasWrite:
	case EventLabel::EL_HelpedCasWrite:
	case EventLabel::EL_ConfirmingCasWrite:
	case EventLabel::EL_FaiWrite:
	case EventLabel::EL_BIncFaiWrite:
	case EventLabel::EL_DskWrite:
	case EventLabel::EL_DskMdWrite:
	case EventLabel::EL_DskDirWrite:
	case EventLabel::EL_DskJnlWrite:
		calcWriteViews(llvm::dyn_cast<WriteLabel>(lab), deps);
		if (getConf()->persevere && llvm::isa<DskWriteLabel>(lab))
			g.getPersChecker()->calcDskMemAccessPbView(llvm::dyn_cast<DskWriteLabel>(lab));
		break;
	case EventLabel::EL_Fence:
	case EventLabel::EL_DskFsync:
	case EventLabel::EL_DskSync:
	case EventLabel::EL_DskPbarrier:
		calcFenceViews(llvm::dyn_cast<FenceLabel>(lab), deps);
		if (getConf()->persevere && llvm::isa<DskAccessLabel>(lab))
			g.getPersChecker()->calcDskFencePbView(llvm::dyn_cast<FenceLabel>(lab));
		break;
	case EventLabel::EL_ThreadStart:
		calcStartViews(llvm::dyn_cast<ThreadStartLabel>(lab));
		break;
	case EventLabel::EL_ThreadJoin:
		calcJoinViews(llvm::dyn_cast<ThreadJoinLabel>(lab), deps);
		break;
	case EventLabel::EL_ThreadCreate:
	case EventLabel::EL_ThreadFinish:
	case EventLabel::EL_Optional:
	case EventLabel::EL_LoopBegin:
	case EventLabel::EL_SpinStart:
	case EventLabel::EL_FaiZNESpinEnd:
	case EventLabel::EL_LockZNESpinEnd:
	case EventLabel::EL_Malloc:
	case EventLabel::EL_Free:
	case EventLabel::EL_HpRetire:
	case EventLabel::EL_UnlockLAPOR:
	case EventLabel::EL_DskOpen:
	case EventLabel::EL_HelpingCas:
	case EventLabel::EL_HpProtect:
		calcBasicViews(lab, deps);
		break;
	case EventLabel::EL_LockLAPOR: /* special case */
		calcLockLAPORViews(llvm::dyn_cast<LockLabelLAPOR>(lab), deps);
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

Event IMMDriver::findDataRaceForMemAccess(const MemAccessLabel *mLab)
{
	/* IMM does not define a concept of a race */
	return Event::getInitializer();
}

void IMMDriver::changeRf(Event read, Event store)
{
	auto &g = getGraph();

	/* Change the reads-from relation in the graph */
	g.changeRf(read, store);

	/* And update the views of the load */
	auto *rLab = static_cast<ReadLabel *>(g.getEventLabel(read));
	View hb = calcBasicHbView(rLab->getPos());
	DepView pporf(rLab->getPPoView());

	updateReadViewsFromRf(pporf, hb, rLab);

	rLab->setHbView(std::move(hb));
	rLab->setPPoRfView(std::move(pporf));

	if (getConf()->persevere && llvm::isa<DskReadLabel>(rLab))
		g.getPersChecker()->calcDskMemAccessPbView(rLab);
}

void IMMDriver::updateStart(Event create, Event start)
{
	auto &g = getGraph();
	auto *bLab = g.getEventLabel(start);

	/* Re-synchronize views */
	View hb(g.getEventLabel(create)->getHbView());
	DepView pporf(g.getEventLabel(create)->getPPoRfView());

	hb[start.thread] = 0;
	pporf[start.thread] = 0;

	bLab->setHbView(std::move(hb));
	bLab->setPPoRfView(std::move(pporf));
	return;
}

bool IMMDriver::updateJoin(Event join, Event childLast)
{
	auto &g = getGraph();

	if (!g.updateJoin(join, childLast))
		return false;

	EventLabel *jLab = g.getEventLabel(join);
	EventLabel *fLab = g.getEventLabel(childLast);

       /* Since the pporf view may contain elements from threads joined
	* in previous explorations, we have to reset it to the ppo one,
	* and then update it */
	DepView pporf(jLab->getPPoView());
	View hb = calcBasicHbView(jLab->getPos());

	hb.update(fLab->getHbView());
	pporf.update(fLab->getPPoRfView());

        jLab->setHbView(std::move(hb));
	jLab->setPPoRfView(std::move(pporf));
	return true;
}

void IMMDriver::initConsCalculation()
{
	return;
}
