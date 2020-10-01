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
#include "ARCalculator.hpp"
#include "PSCCalculator.hpp"

IMMDriver::IMMDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
		     std::vector<Library> &granted, std::vector<Library> &toVerify,
		     clock_t start)
	: GenMCDriver(std::move(conf), std::move(mod), granted, toVerify, start)
{
	auto &g = getGraph();

	/* IMM requires acyclicity checks for both PSC and AR */
	g.addCalculator(LLVM_MAKE_UNIQUE<PSCCalculator>(g),
			ExecutionGraph::RelationId::psc, false);
	g.addCalculator(LLVM_MAKE_UNIQUE<ARCalculator>(g),
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

DepView IMMDriver::calcPPoView(Event e) /* not const */
{
	auto &g = getGraph();
	auto *EE = getEE();
	DepView v;

	/* Update ppo based on dependencies (addr, data, ctrl, addr;po, cas) */
	auto *addr = EE->getCurrentAddrDeps();
	if (addr) {
		for (auto &adep : *addr)
			v.update(g.getPPoRfBefore(adep));
	}
	auto *data = EE->getCurrentDataDeps();
	if (data) {
		for (auto &ddep : *data)
			v.update(g.getPPoRfBefore(ddep));
	}
	auto *ctrl = EE->getCurrentCtrlDeps();
	if (ctrl) {
		for (auto &cdep : *ctrl)
			v.update(g.getPPoRfBefore(cdep));
	}
	auto *addrPo = EE->getCurrentAddrPoDeps();
	if (addrPo) {
		for (auto &apdep : *addrPo)
			v.update(g.getPPoRfBefore(apdep));
	}
	auto *cas = EE->getCurrentCasDeps();
	if (cas) {
		for (auto &csdep : *cas)
			v.update(g.getPPoRfBefore(csdep));
	}

	/* This event does not depend on anything else */
	int oldIdx = v[e.thread];
	v[e.thread] = e.index;
	for (auto i = oldIdx + 1; i < e.index; i++)
		v.addHole(Event(e.thread, i));

	/* Update based on the views of the acquires of the thread */
	std::vector<Event> acqs = g.getThreadAcquiresAndFences(e);
	for (auto &ev : acqs)
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

void IMMDriver::calcBasicReadViews(ReadLabel *lab)
{
	const auto &g = getGraph();
	const EventLabel *rfLab = g.getEventLabel(lab->getRf());
	View hb = calcBasicHbView(lab->getPos());
	DepView ppo = calcPPoView(lab->getPos());
	DepView pporf(ppo);

	pporf.update(rfLab->getPPoRfView());
	if (rfLab->getThread() != lab->getThread()) {
		for (auto i = 0u; i < lab->getIndex(); i++) {
			const EventLabel *eLab = g.getEventLabel(Event(lab->getThread(), i));
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
	lab->setHbView(std::move(hb));
	lab->setPPoView(std::move(ppo));
	lab->setPPoRfView(std::move(pporf));
}

void IMMDriver::calcBasicWriteViews(WriteLabel *lab)
{
	const auto &g = getGraph();

	/* First, we calculate the hb and (po U rf) views */
	View hb = calcBasicHbView(lab->getPos());
	lab->setHbView(std::move(hb));

	/* Then, we calculate the (ppo U rf) view */
	DepView pporf = calcPPoView(lab->getPos());

	if (llvm::isa<CasWriteLabel>(lab) || llvm::isa<FaiWriteLabel>(lab))
		pporf.update(g.getPPoRfBefore(g.getPreviousLabel(lab)->getPos()));
	if (lab->isAtLeastRelease())
		updateRelView(pporf, lab);
	pporf.update(g.getPPoRfBefore(g.getLastThreadReleaseAtLoc(lab->getPos(),
								  lab->getAddr())));
	lab->setPPoRfView(std::move(pporf));
}

void IMMDriver::calcWriteMsgView(WriteLabel *lab)
{
	const auto &g = getGraph();
	View msg;

	/* Should only be called with plain writes */
	BUG_ON(llvm::isa<FaiWriteLabel>(lab) || llvm::isa<CasWriteLabel>(lab));

	if (lab->isAtLeastRelease())
		msg = lab->getHbView();
	else if (lab->getOrdering() == llvm::AtomicOrdering::Monotonic ||
		 lab->getOrdering() == llvm::AtomicOrdering::Acquire)
		msg = g.getHbBefore(g.getLastThreadReleaseAtLoc(lab->getPos(),
								lab->getAddr()));
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
		msg.update(g.getHbBefore(g.getLastThreadReleaseAtLoc(lab->getPos(),
								     lab->getAddr())));

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
		if (rLab->getOrdering() == llvm::AtomicOrdering::Monotonic ||
		    rLab->getOrdering() == llvm::AtomicOrdering::Release) {
			const EventLabel *rfLab = g.getEventLabel(rLab->getRf());
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
				v.update(wLab->getMsgView());
		}
	}
}


void IMMDriver::calcBasicFenceViews(FenceLabel *lab)
{
	const auto &g = getGraph();
	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	if (lab->isAtLeastAcquire())
		calcFenceRelRfPoBefore(lab->getPos().prev(), hb);
	if (lab->isAtLeastRelease())
		updateRelView(pporf, lab);

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
}

std::unique_ptr<ReadLabel>
IMMDriver::createReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			     const llvm::GenericValue *ptr, const llvm::Type *typ,
			     Event rf)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ReadLabel>(g.nextStamp(), ord, pos, ptr, typ, rf);

	calcBasicReadViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<FaiReadLabel>
IMMDriver::createFaiReadLabel(int tid, int index, llvm::AtomicOrdering ord,
				const llvm::GenericValue *ptr, const llvm::Type *typ,
				Event rf, llvm::AtomicRMWInst::BinOp op,
				const llvm::GenericValue &opValue)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<FaiReadLabel>(g.nextStamp(), ord, pos, ptr, typ,
						   rf, op, opValue);

	calcBasicReadViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<CasReadLabel>
IMMDriver::createCasReadLabel(int tid, int index, llvm::AtomicOrdering ord,
				const llvm::GenericValue *ptr, const llvm::Type *typ,
				Event rf, const llvm::GenericValue &expected,
				const llvm::GenericValue &swap, bool isLock)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<CasReadLabel>(g.nextStamp(), ord, pos, ptr, typ,
						   rf, expected, swap, isLock);

	calcBasicReadViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<LibReadLabel>
IMMDriver::createLibReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			      const llvm::GenericValue *ptr, const llvm::Type *typ,
			      Event rf, std::string functionName)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<LibReadLabel>(g.nextStamp(), ord, pos, ptr,
						   typ, rf, functionName);
	calcBasicReadViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskReadLabel>
IMMDriver::createDskReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			      const llvm::GenericValue *ptr, const llvm::Type *typ,
			      Event rf)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskReadLabel>(g.nextStamp(), ord, pos, ptr,
						  typ, rf);
	calcBasicReadViews(lab.get());
	if (getConf()->persevere)
		g.getPersChecker()->calcMemAccessPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<WriteLabel>
IMMDriver::createStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    const llvm::GenericValue &val, bool isUnlock)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<WriteLabel>(g.nextStamp(), ord, pos, ptr,
						 typ, val, isUnlock);
	calcBasicWriteViews(lab.get());
	calcWriteMsgView(lab.get());
	return std::move(lab);
}

std::unique_ptr<FaiWriteLabel>
IMMDriver::createFaiStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
				 const llvm::GenericValue *ptr, const llvm::Type *typ,
				 const llvm::GenericValue &val)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<FaiWriteLabel>(g.nextStamp(), ord, pos,
						    ptr, typ, val);
	calcBasicWriteViews(lab.get());
	calcRMWWriteMsgView(lab.get());
	return std::move(lab);
}

std::unique_ptr<CasWriteLabel>
IMMDriver::createCasStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
				 const llvm::GenericValue *ptr, const llvm::Type *typ,
				 const llvm::GenericValue &val, bool isLock)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<CasWriteLabel>(g.nextStamp(), ord, pos, ptr,
						    typ, val, isLock);

	calcBasicWriteViews(lab.get());
	calcRMWWriteMsgView(lab.get());
	return std::move(lab);
}

std::unique_ptr<LibWriteLabel>
IMMDriver::createLibStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			       const llvm::GenericValue *ptr, const llvm::Type *typ,
			       llvm::GenericValue &val, std::string functionName,
			       bool isInit)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<LibWriteLabel>(g.nextStamp(), ord, pos, ptr,
						    typ, val, functionName, isInit);

	calcBasicWriteViews(lab.get());
	calcWriteMsgView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskWriteLabel>
IMMDriver::createDskWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
			       const llvm::GenericValue *ptr, const llvm::Type *typ,
			       const llvm::GenericValue &val, void *mapping)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskWriteLabel>(
		g.nextStamp(), ord, pos, ptr, typ, val, mapping);

	calcBasicWriteViews(lab.get());
	calcWriteMsgView(lab.get());
	if (getConf()->persevere)
		g.getPersChecker()->calcMemAccessPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskMdWriteLabel>
IMMDriver::createDskMdWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
				 const llvm::GenericValue *ptr, const llvm::Type *typ,
				 const llvm::GenericValue &val, void *mapping,
				 std::pair<void *, void *> ordDataRange)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskMdWriteLabel>(
		g.nextStamp(), ord, pos, ptr, typ, val, mapping, ordDataRange);

	calcBasicWriteViews(lab.get());
	calcWriteMsgView(lab.get());
	if (getConf()->persevere)
		g.getPersChecker()->calcMemAccessPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskDirWriteLabel>
IMMDriver::createDskDirWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
				  const llvm::GenericValue *ptr, const llvm::Type *typ,
				  const llvm::GenericValue &val, void *mapping)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskDirWriteLabel>(
		g.nextStamp(), ord, pos, ptr, typ, val, mapping);

	calcBasicWriteViews(lab.get());
	calcWriteMsgView(lab.get());
	if (getConf()->persevere)
		g.getPersChecker()->calcMemAccessPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskJnlWriteLabel>
IMMDriver::createDskJnlWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
				  const llvm::GenericValue *ptr, const llvm::Type *typ,
				  const llvm::GenericValue &val, void *mapping, void *transInode)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskJnlWriteLabel>(
		g.nextStamp(), ord, pos, ptr, typ, val, mapping, transInode);

	calcBasicWriteViews(lab.get());
	calcWriteMsgView(lab.get());
	if (getConf()->persevere)
		g.getPersChecker()->calcMemAccessPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<FenceLabel>
IMMDriver::createFenceLabel(int tid, int index, llvm::AtomicOrdering ord)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<FenceLabel>(g.nextStamp(), ord, pos);

	calcBasicFenceViews(lab.get());
	return std::move(lab);
}


std::unique_ptr<MallocLabel>
IMMDriver::createMallocLabel(int tid, int index, const void *addr,
			     unsigned int size, Storage s, AddressSpace spc)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<MallocLabel>(g.nextStamp(),
						  llvm::AtomicOrdering::NotAtomic,
						  pos, addr, size, s, spc);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<DskOpenLabel>
IMMDriver::createDskOpenLabel(int tid, int index, const char *fileName,
			      const llvm::GenericValue &fd)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskOpenLabel>(g.nextStamp(),
						   llvm::AtomicOrdering::Release,
						   pos, fileName, fd);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	updateRelView(pporf, lab.get());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<DskFsyncLabel>
IMMDriver::createDskFsyncLabel(int tid, int index, const void *inode,
			       unsigned int size)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskFsyncLabel>(g.nextStamp(),
						    llvm::AtomicOrdering::Release,
						    pos, inode, size);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	updateRelView(pporf, lab.get());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));

	if (getConf()->persevere)
		g.getPersChecker()->calcFsyncPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskSyncLabel>
IMMDriver::createDskSyncLabel(int tid, int index)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskSyncLabel>(g.nextStamp(),
						   llvm::AtomicOrdering::Release,
						   pos);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	updateRelView(pporf, lab.get());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));

	if (getConf()->persevere)
		g.getPersChecker()->calcSyncPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskPbarrierLabel>
IMMDriver::createDskPbarrierLabel(int tid, int index)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskPbarrierLabel>(g.nextStamp(),
						       llvm::AtomicOrdering::Release,
						       pos);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	updateRelView(pporf, lab.get());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));

	if (getConf()->persevere)
		g.getPersChecker()->calcPbarrierPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<FreeLabel>
IMMDriver::createFreeLabel(int tid, int index, const void *addr)
{
	auto &g = getGraph();
	Event pos(tid, index);
	std::unique_ptr<FreeLabel> lab(
		new FreeLabel(g.nextStamp(), llvm::AtomicOrdering::NotAtomic,
			      pos, addr));

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<ThreadCreateLabel>
IMMDriver::createTCreateLabel(int tid, int index, int cid)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ThreadCreateLabel>(getGraph().nextStamp(),
							llvm::AtomicOrdering::Release, pos, cid);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	pporf.removeAllHoles(lab->getThread());
	Event rel = g.getLastThreadRelease(lab->getPos());
	for (auto i = rel.index; i < lab->getIndex(); i++) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(
			    g.getEventLabel(Event(lab->getThread(), i)))) {
			pporf.update(rLab->getPPoRfView());
		}
	}

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<ThreadJoinLabel>
IMMDriver::createTJoinLabel(int tid, int index, int cid)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ThreadJoinLabel>(g.nextStamp(),
						      llvm::AtomicOrdering::Acquire,
						      pos, cid);

	/* Thread joins have acquire semantics -- but we have to wait
	 * for the other thread to finish first, so we do not fully
	 * update the view yet */
	View hb = calcBasicHbView(lab->getPos());
	DepView ppo = calcPPoView(lab->getPos());
	DepView pporf(ppo);

	lab->setHbView(std::move(hb));
	lab->setPPoView(std::move(ppo));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<ThreadStartLabel>
IMMDriver::createStartLabel(int tid, int index, Event tc, int symm /* = -1 */)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ThreadStartLabel>(g.nextStamp(),
						      llvm::AtomicOrdering::Acquire,
						      pos, tc, symm);

	/* Thread start has Acquire semantics */
	View hb(g.getHbBefore(tc));
	DepView pporf(g.getPPoRfBefore(tc));

	hb[tid] = pos.index;
	pporf[tid] = pos.index;

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<ThreadFinishLabel>
IMMDriver::createFinishLabel(int tid, int index)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ThreadFinishLabel>(getGraph().nextStamp(),
							llvm::AtomicOrdering::Release,
							pos);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	pporf.removeAllHoles(lab->getThread());
	Event rel = g.getLastThreadRelease(lab->getPos());
	for (auto i = rel.index; i < lab->getIndex(); i++) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(
			    g.getEventLabel(Event(lab->getThread(), i)))) {
			pporf.update(rLab->getPPoRfView());
		}
	}

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<LockLabelLAPOR>
IMMDriver::createLockLabelLAPOR(int tid, int index, const llvm::GenericValue *addr)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<LockLabelLAPOR>(getGraph().nextStamp(),
						     llvm::AtomicOrdering::Acquire,
						     pos, addr);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	auto prevUnlock = g.getLastThreadUnlockAtLocLAPOR(lab->getPos().prev(),
							  addr);
	if (!prevUnlock.isInitializer())
		pporf.update(g.getPPoRfBefore(prevUnlock));

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

std::unique_ptr<UnlockLabelLAPOR>
IMMDriver::createUnlockLabelLAPOR(int tid, int index, const llvm::GenericValue *addr)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<UnlockLabelLAPOR>(getGraph().nextStamp(),
						       llvm::AtomicOrdering::Release,
						       pos, addr);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	updateRelView(pporf, lab.get());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
	return std::move(lab);
}

Event IMMDriver::findDataRaceForMemAccess(const MemAccessLabel *mLab)
{
	return Event::getInitializer(); /* Race detection disabled for IMM */
}

std::vector<Event> IMMDriver::getStoresToLoc(const llvm::GenericValue *addr)
{
	return getGraph().getCoherentStores(addr, getEE()->getCurrentPosition());
}

std::vector<Event> IMMDriver::getRevisitLoads(const WriteLabel *sLab)
{
	return getGraph().getCoherentRevisits(sLab);
}

void IMMDriver::changeRf(Event read, Event store)
{
	auto &g = getGraph();

	/* Change the reads-from relation in the graph */
	g.changeRf(read, store);

	/* And update the views of the load */
	EventLabel *lab = g.getEventLabel(read);
	ReadLabel *rLab = static_cast<ReadLabel *>(lab);
	EventLabel *rfLab = g.getEventLabel(store);
	View hb = calcBasicHbView(rLab->getPos());
	DepView pporf(rLab->getPPoView());

	pporf.update(rfLab->getPPoRfView());
	if (rfLab->getThread() != rLab->getThread()) {
		for (auto i = 0u; i < rLab->getIndex(); i++) {
			const EventLabel *eLab = g.getEventLabel(Event(rLab->getThread(), i));
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(eLab)) {
				if (wLab->getAddr() == rLab->getAddr())
					pporf.update(wLab->getPPoRfView());
			}
		}
	}

	if (rLab->isAtLeastAcquire()) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
			hb.update(wLab->getMsgView());
	}
	rLab->setHbView(std::move(hb));
	rLab->setPPoRfView(std::move(pporf));

	if (getConf()->persevere && llvm::isa<DskReadLabel>(rLab))
		g.getPersChecker()->calcMemAccessPbView(rLab);
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
