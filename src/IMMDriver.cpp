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

#include "IMMDriver.hpp"

/* Calculates a minimal hb vector clock based on po for a given label */
View IMMDriver::calcBasicHbView(Event e) const
{
	View v(getGraph().getPreviousLabel(e)->getHbView());

	++v[e.thread];
	return v;
}

/* Calculates a minimal (po U rf) vector clock based on po for a given label */
View IMMDriver::calcBasicPorfView(Event e) const
{
	View v(getGraph().getPreviousLabel(e)->getPorfView());

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

	if (lab->isAtLeastRelease()) {
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
	}
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
	if (lab->isAtLeastRelease()) {
		pporf.removeAllHoles(lab->getThread());
		Event rel = g.getLastThreadRelease(lab->getPos());
		for (auto i = rel.index; i < lab->getIndex(); i++) {
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(
				    g.getEventLabel(Event(lab->getThread(), i)))) {
				pporf.update(rLab->getPPoRfView());
			}
		}
	}

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
	auto lab = llvm::make_unique<ReadLabel>(g.nextStamp(), ord, pos, ptr, typ, rf);

	calcBasicReadViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<FaiReadLabel>
IMMDriver::createFaiReadLabel(int tid, int index, llvm::AtomicOrdering ord,
				const llvm::GenericValue *ptr, const llvm::Type *typ,
				Event rf, llvm::AtomicRMWInst::BinOp op,
				llvm::GenericValue &&opValue)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = llvm::make_unique<FaiReadLabel>(g.nextStamp(), ord, pos, ptr, typ,
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
	auto lab = llvm::make_unique<CasReadLabel>(g.nextStamp(), ord, pos, ptr, typ,
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
	auto lab = llvm::make_unique<LibReadLabel>(g.nextStamp(), ord, pos, ptr,
						   typ, rf, functionName);
	calcBasicReadViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<WriteLabel>
IMMDriver::createStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    const llvm::GenericValue &val, bool isUnlock)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = llvm::make_unique<WriteLabel>(g.nextStamp(), ord, pos, ptr,
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
	auto lab = llvm::make_unique<FaiWriteLabel>(g.nextStamp(), ord, pos,
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
	auto lab = llvm::make_unique<CasWriteLabel>(g.nextStamp(), ord, pos, ptr,
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
	auto lab = llvm::make_unique<LibWriteLabel>(g.nextStamp(), ord, pos, ptr,
						    typ, val, functionName, isInit);

	calcBasicWriteViews(lab.get());
	calcWriteMsgView(lab.get());
	return std::move(lab);
}

std::unique_ptr<FenceLabel>
IMMDriver::createFenceLabel(int tid, int index, llvm::AtomicOrdering ord)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = llvm::make_unique<FenceLabel>(g.nextStamp(), ord, pos);

	calcBasicFenceViews(lab.get());
	return std::move(lab);
}


std::unique_ptr<MallocLabel>
IMMDriver::createMallocLabel(int tid, int index, const void *addr,
			       unsigned int size, bool isLocal)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = llvm::make_unique<MallocLabel>(g.nextStamp(),
						  llvm::AtomicOrdering::NotAtomic,
						  pos, addr, size, isLocal);

	View hb = calcBasicHbView(lab->getPos());
	DepView pporf = calcPPoView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPPoRfView(std::move(pporf));
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
	auto lab = llvm::make_unique<ThreadCreateLabel>(getGraph().nextStamp(),
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
	auto lab = llvm::make_unique<ThreadJoinLabel>(g.nextStamp(),
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
IMMDriver::createStartLabel(int tid, int index, Event tc)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = llvm::make_unique<ThreadStartLabel>(g.nextStamp(),
						       llvm::AtomicOrdering::Acquire,
						       pos, tc);

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
	auto lab = llvm::make_unique<ThreadFinishLabel>(getGraph().nextStamp(),
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

std::vector<Event> IMMDriver::collectAllEvents()
{
	const auto &g = getGraph();
	std::vector<Event> result;

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (llvm::isa<MemAccessLabel>(lab) ||
			    llvm::isa<FenceLabel>(lab))
				result.push_back(lab->getPos());
		}
	}
	return result;
}

/* TODO: Add function in VC interface that transforms VC to Matrix? */
void IMMDriver::fillMatrixFromView(const Event e, const DepView &v,
				     Matrix2D<Event> &matrix)
{
	const auto &g = getGraph();
 	auto eI = matrix.getIndex(e);

	for (auto i = 0u; i < v.size(); i++) {
                if ((int) i == e.thread)
			continue;
		for (auto j = v[i]; j > 0; j--) {
			auto curr = Event(i, j);
			if (!v.contains(curr))
				continue;

			auto *lab = g.getEventLabel(curr);
                        if (!llvm::isa<MemAccessLabel>(lab) &&
			    !llvm::isa<FenceLabel>(lab))
                                continue;

			matrix(lab->getPos(), eI) = true;
                        break;
                }
 	}
}

Matrix2D<Event> IMMDriver::getARMatrix()
{
	Matrix2D<Event> ar(collectAllEvents());
	const std::vector<Event> &es = ar.getElems();
        auto len = ar.size();
	const auto &g = getGraph();

        /* First, add ppo edges */
        for (auto i = 0u; i < len; i++) {
                for (auto j = 0u; j < len; j++) {
			if (es[i].thread == es[j].thread &&
			    es[i].index < es[j].index &&
			    g.getPPoRfBefore(es[j]).contains(es[i]))
				ar(i, j) = true;
                }
        }

        /* Then, add the remaining ar edges */
        for (const auto &e : es) {
                auto *lab = g.getEventLabel(e);
                BUG_ON(!llvm::isa<MemAccessLabel>(lab) && !llvm::isa<FenceLabel>(lab));
                fillMatrixFromView(e, g.getPPoRfBefore(e), ar);
        }
	ar.transClosure();
	return ar;
}

bool IMMDriver::isExecutionValid()
{
	const auto &g = getGraph();
	Matrix2D<Event> ar = getARMatrix();
	auto scs = g.getSCs();
	auto &fcs = scs.second;

	std::function<bool(const Matrix2D<Event>&)> check =
		[&](const Matrix2D<Event> &psc) -> bool {
		auto basicAr = ar;
		if (psc.isReflexive())
			return false;
		for (auto &f1 : fcs) {
			for (auto &f2 : fcs)
				if (psc(f1, f2))
					basicAr(f1, f2) = true;
		}
		basicAr.transClosure();
		return !basicAr.isReflexive();
	};

	return g.checkPscCondition(CheckPSCType::full, check);
}
