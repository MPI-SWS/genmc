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
#include "PSCCalculator.hpp"

RC11Driver::RC11Driver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
		       std::vector<Library> &granted, std::vector<Library> &toVerify,
		       clock_t start)
	: GenMCDriver(std::move(conf), std::move(mod), granted, toVerify, start)
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

void RC11Driver::calcBasicReadViews(ReadLabel *lab)
{
	const auto &g = getGraph();
	const EventLabel *rfLab = g.getEventLabel(lab->getRf());
	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	porf.update(rfLab->getPorfView());
	if (lab->isAtLeastAcquire()) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
			hb.update(wLab->getMsgView());
	}
	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
}

void RC11Driver::calcBasicWriteViews(WriteLabel *lab)
{
	const auto &g = getGraph();

	/* First, we calculate the hb and (po U rf) views */
	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
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
		msg = g.getHbBefore(g.getLastThreadReleaseAtLoc(lab->getPos(),
								lab->getAddr()));
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
		msg.update(g.getHbBefore(g.getLastThreadReleaseAtLoc(lab->getPos(),
								     lab->getAddr())));

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


void RC11Driver::calcBasicFenceViews(FenceLabel *lab)
{
	const auto &g = getGraph();
	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	if (lab->isAtLeastAcquire())
		calcFenceRelRfPoBefore(lab->getPos().prev(), hb);

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
}

std::unique_ptr<ReadLabel>
RC11Driver::createReadLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createFaiReadLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createCasReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			       const llvm::GenericValue *ptr, const llvm::Type *typ,
			       Event rf, const llvm::GenericValue &expected,
			       const llvm::GenericValue &swap,
			       bool isLock)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<CasReadLabel>(g.nextStamp(), ord, pos, ptr, typ,
						   rf, expected, swap, isLock);

	calcBasicReadViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<LibReadLabel>
RC11Driver::createLibReadLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createDskReadLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createFaiStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createCasStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createLibStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createDskWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createDskMdWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createDskDirWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createDskJnlWriteLabel(int tid, int index, llvm::AtomicOrdering ord,
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
RC11Driver::createFenceLabel(int tid, int index, llvm::AtomicOrdering ord)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<FenceLabel>(g.nextStamp(), ord, pos);

	calcBasicFenceViews(lab.get());
	return std::move(lab);
}

std::unique_ptr<MallocLabel>
RC11Driver::createMallocLabel(int tid, int index, const void *addr,
			      unsigned int size, Storage s, AddressSpace spc)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<MallocLabel>(g.nextStamp(),
						  llvm::AtomicOrdering::NotAtomic,
						  pos, addr, size, s, spc);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
}

std::unique_ptr<FreeLabel>
RC11Driver::createFreeLabel(int tid, int index, const void *addr)
{
	auto &g = getGraph();
	Event pos(tid, index);
	std::unique_ptr<FreeLabel> lab(
		new FreeLabel(g.nextStamp(), llvm::AtomicOrdering::NotAtomic,
			      pos, addr));

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());


	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
}

std::unique_ptr<DskOpenLabel>
RC11Driver::createDskOpenLabel(int tid, int index, const char *fileName,
			       const llvm::GenericValue &fd)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskOpenLabel>(g.nextStamp(),
						   llvm::AtomicOrdering::NotAtomic,
						   pos, fileName, fd);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
}

std::unique_ptr<DskFsyncLabel>
RC11Driver::createDskFsyncLabel(int tid, int index, const void *inode,
				unsigned int size)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskFsyncLabel>(g.nextStamp(),
						    llvm::AtomicOrdering::Release,
						    pos, inode, size);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());
	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));

	if (getConf()->persevere)
		g.getPersChecker()->calcFsyncPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskSyncLabel>
RC11Driver::createDskSyncLabel(int tid, int index)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskSyncLabel>(g.nextStamp(),
						   llvm::AtomicOrdering::Release,
						   pos);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());
	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));

	if (getConf()->persevere)
		g.getPersChecker()->calcSyncPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<DskPbarrierLabel>
RC11Driver::createDskPbarrierLabel(int tid, int index)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<DskPbarrierLabel>(g.nextStamp(),
						       llvm::AtomicOrdering::Release,
						       pos);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());
	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));

	if (getConf()->persevere)
		g.getPersChecker()->calcPbarrierPbView(lab.get());
	return std::move(lab);
}

std::unique_ptr<ThreadCreateLabel>
RC11Driver::createTCreateLabel(int tid, int index, int cid)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ThreadCreateLabel>(getGraph().nextStamp(),
							llvm::AtomicOrdering::Release, pos, cid);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));

	return std::move(lab);
}

std::unique_ptr<ThreadJoinLabel>
RC11Driver::createTJoinLabel(int tid, int index, int cid)
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
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
}

std::unique_ptr<ThreadStartLabel>
RC11Driver::createStartLabel(int tid, int index, Event tc, int symm /* = -1 */)
{
	auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ThreadStartLabel>(g.nextStamp(),
						      llvm::AtomicOrdering::Acquire,
						      pos, tc, symm);

	/* Thread start has Acquire semantics */
	View hb(g.getHbBefore(tc));
	View porf(g.getPorfBefore(tc));

	hb[tid] = pos.index;
	porf[tid] = pos.index;

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
}

std::unique_ptr<ThreadFinishLabel>
RC11Driver::createFinishLabel(int tid, int index)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<ThreadFinishLabel>(getGraph().nextStamp(),
							llvm::AtomicOrdering::Release,
							pos);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
}

std::unique_ptr<LockLabelLAPOR>
RC11Driver::createLockLabelLAPOR(int tid, int index, const llvm::GenericValue *addr)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<LockLabelLAPOR>(getGraph().nextStamp(),
						     llvm::AtomicOrdering::Acquire,
						     pos, addr);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
}

std::unique_ptr<UnlockLabelLAPOR>
RC11Driver::createUnlockLabelLAPOR(int tid, int index, const llvm::GenericValue *addr)
{
	const auto &g = getGraph();
	Event pos(tid, index);
	auto lab = LLVM_MAKE_UNIQUE<UnlockLabelLAPOR>(getGraph().nextStamp(),
						       llvm::AtomicOrdering::Release,
						       pos, addr);

	View hb = calcBasicHbView(lab->getPos());
	View porf = calcBasicPorfView(lab->getPos());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return std::move(lab);
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

std::vector<Event> RC11Driver::getStoresToLoc(const llvm::GenericValue *addr)
{
	return getGraph().getCoherentStores(addr, getEE()->getCurrentPosition());
}

std::vector<Event> RC11Driver::getRevisitLoads(const WriteLabel *sLab)
{
	return getGraph().getCoherentRevisits(sLab);
}

void RC11Driver::changeRf(Event read, Event store)
{
	auto &g = getGraph();

	/* Change the reads-from relation in the graph */
	g.changeRf(read, store);

	/* And update the views of the load */
	auto *rLab = static_cast<ReadLabel *>(g.getEventLabel(read));
	calcBasicReadViews(rLab);
	if (getConf()->persevere && llvm::isa<DskReadLabel>(rLab))
		g.getPersChecker()->calcMemAccessPbView(rLab);
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
