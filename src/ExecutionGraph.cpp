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

#include "Library.hpp"
#include "Parser.hpp"
#include "ExecutionGraph.hpp"
#include <llvm/IR/DebugInfo.h>

/************************************************************
 ** Class Constructors
 ***********************************************************/

ExecutionGraph::ExecutionGraph() : timestamp(1)
{
	/* Create an entry for main() and push the "initializer" label */
	events.push_back({});
	events[0].push_back( std::unique_ptr<ThreadStartLabel>(
				     new ThreadStartLabel(
					     0, llvm::AtomicOrdering::Acquire,
					     Event(0, 0),
					     Event::getInitializer() )
				     ) );
}


/************************************************************
 ** Basic getter methods
 ***********************************************************/

unsigned int ExecutionGraph::nextStamp()
{
	return timestamp++;
}

const EventLabel *ExecutionGraph::getEventLabel(Event e) const
{
	return events[e.thread][e.index].get();
}

const EventLabel *ExecutionGraph::getPreviousLabel(Event e) const
{
	return events[e.thread][e.index - 1].get();
}

const EventLabel *ExecutionGraph::getPreviousLabel(const EventLabel *lab) const
{
	return events[lab->getThread()][lab->getIndex() - 1].get();
}

const EventLabel *ExecutionGraph::getLastThreadLabel(int thread) const
{
	return events[thread][events[thread].size() - 1].get();
}

Event ExecutionGraph::getLastThreadEvent(int thread) const
{
	return Event(thread, events[thread].size() - 1);
}

Event ExecutionGraph::getLastThreadRelease(int thread, const llvm::GenericValue *addr) const
{
	for (int i = events[thread].size() - 1; i > 0; i--) {
		const EventLabel *lab = getEventLabel(Event(thread, i));
		if (auto *fLab = llvm::dyn_cast<FenceLabel>(lab)) {
			if (fLab->isAtLeastRelease())
				return Event(thread, i);
		}
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
			if (wLab->isAtLeastRelease() && wLab->getAddr() == addr)
				return Event(thread, i);
		}
	}
	return Event(thread, 0);
}

/*
 * Given an write label sLab that is part of an RMW, return all
 * other RMWs that read from the same write. Of course, there must
 * be _at most_ one other RMW reading from the same write (see [Rex] set)
 */
std::vector<Event> ExecutionGraph::getPendingRMWs(const WriteLabel *sLab)
{
	std::vector<Event> pending;

	/* If this is _not_ an RMW event, return an empty vector */
	if (!llvm::isa<FaiWriteLabel>(sLab) && !llvm::isa<CasWriteLabel>(sLab))
		return pending;

	/* Otherwise, scan for other RMWs that successfully read the same val */
	const auto *pLab = static_cast<const ReadLabel *>(getPreviousLabel(sLab));
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 1u; j < events[i].size(); j++) { /* Skip thread start */
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!isRMWLoad(lab))
				continue;
			const auto *rLab = static_cast<const ReadLabel *>(lab);
			if (rLab->getRf() == pLab->getRf() &&
			    rLab->getAddr() == pLab->getAddr() &&
			    rLab->getPos() != pLab->getPos())
				pending.push_back(lab->getPos());
		}
	}
	BUG_ON(pending.size() > 1);
	return pending;
}

Event ExecutionGraph::getPendingLibRead(const LibReadLabel *lab)
{
	/* Should only be called with a read of a functional library that doesn't read BOT */
	BUG_ON(lab->getRf().isInitializer());

	/* Get the conflicting label */
	const auto *sLab = static_cast<const WriteLabel *>(getEventLabel(lab->getRf()));
	auto it = std::find_if(sLab->getReadersList().begin(),
			       sLab->getReadersList().end(),
			       [&](const Event &e) { return e != lab->getPos(); });
	BUG_ON(it == sLab->getReadersList().end());
	return *it;
}

std::vector<Event> ExecutionGraph::getRevisitable(const WriteLabel *sLab)
{
	std::vector<Event> loads;

	auto &before = getPorfBefore(sLab->getPos());
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = before[i] + 1u; j < events[i].size(); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getAddr() == sLab->getAddr() &&
				    rLab->isRevisitable())
					loads.push_back(rLab->getPos());
			}
		}
	}
	return loads;
}

std::vector<Event> ExecutionGraph::getMoOptRfAfter(const WriteLabel *sLab)
{
	auto ls = modOrder.getMoAfter(sLab->getAddr(), sLab->getPos());
	std::vector<Event> rfs;

	/*
	 * We push the RFs to a different vector in order
	 * not to invalidate the iterator
	 */
	for (auto it = ls.begin(); it != ls.end(); ++it) {
		const EventLabel *lab = getEventLabel(*it);
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
			for (auto &l : wLab->getReadersList())
				rfs.push_back(l);
		}
	}
	std::move(rfs.begin(), rfs.end(), std::back_inserter(ls));
	return ls;
}


/*******************************************************************************
 **                       Label addition methods
 ******************************************************************************/

/* Since 1) std::make_unique<> is not a friend class to EventLabel
 * (which has its constructors protected), and 2) there is no
 * guarantee that std::make_unique<> will not delegate the
 * construction of the object to some internal function, when creating
 * some std::unique_ptr<> in the following methods we have to use the
 * std::unique_ptr<> constructor and not std::make_unique<> */

/* Calculates a minimal hb vector clock based on po for a given label */
View ExecutionGraph::calcBasicHbView(const EventLabel *lab) const
{
	View v(getPreviousLabel(lab)->getHbView());

	++v[lab->getThread()];
	return v;
}

/* Calculates a minimal (po U rf) vector clock based on po for a given label */
View ExecutionGraph::calcBasicPorfView(const EventLabel *lab) const
{
	View v(getPreviousLabel(lab)->getPorfView());

	++v[lab->getThread()];
	return v;
}

const EventLabel *ExecutionGraph::addEventToGraph(std::unique_ptr<EventLabel> lab)
{
	auto thread = lab->getThread();
	events[thread].push_back(std::move(lab));
	return getLastThreadLabel(thread);
}

const ReadLabel *
ExecutionGraph::addReadToGraphCommon(std::unique_ptr<ReadLabel> lab)
{
	EventLabel *rfLab = events[lab->getRf().thread][lab->getRf().index].get();
	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());

	porf.update(rfLab->getPorfView());
	if (lab->isAtLeastAcquire()) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
			hb.update(wLab->getMsgView());
	}
	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));

	const auto *nLab = static_cast<const ReadLabel *>(
		addEventToGraph(std::move(lab)));


	if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab)) {
		wLab->addReader(nLab->getPos());
	}
	return nLab;
}

const ReadLabel *
ExecutionGraph::addReadToGraph(int tid, llvm::AtomicOrdering ord,
			       const llvm::GenericValue *ptr,
			       const llvm::Type *typ, Event rf)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<ReadLabel> lab(
		new ReadLabel(nextStamp(), ord, pos, ptr, typ, rf));
	return addReadToGraphCommon(std::move(lab));
}

const FaiReadLabel *
ExecutionGraph::addFaiReadToGraph(int tid, llvm::AtomicOrdering ord,
				  const llvm::GenericValue *ptr,
				  const llvm::Type *typ, Event rf,
				  llvm::AtomicRMWInst::BinOp op,
				  llvm::GenericValue &&val)

{
	Event pos(tid, events[tid].size());
	std::unique_ptr<FaiReadLabel> lab(
		new FaiReadLabel(nextStamp(), ord, pos, ptr, typ, rf, op, val));

	return static_cast<const FaiReadLabel *>(
		addReadToGraphCommon(std::move(lab)));
}

const CasReadLabel *
ExecutionGraph::addCasReadToGraph(int tid, llvm::AtomicOrdering ord,
				  const llvm::GenericValue *ptr,
				  const llvm::Type *typ, Event rf,
				  const llvm::GenericValue &expected,
				  const llvm::GenericValue &swap,
				  bool isLock /* = false */)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<CasReadLabel> lab(
		new CasReadLabel(nextStamp(), ord, pos, ptr, typ,
				 rf, expected, swap, isLock));

	return static_cast<const CasReadLabel *>(
		addReadToGraphCommon(std::move(lab)));
}

const LibReadLabel *
ExecutionGraph::addLibReadToGraph(int tid, llvm::AtomicOrdering ord,
				  const llvm::GenericValue *ptr,
				  const llvm::Type *typ, Event rf,
				  std::string fName)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<LibReadLabel> lab(
		new LibReadLabel(nextStamp(), ord, pos, ptr, typ, rf, fName));

	return static_cast<const LibReadLabel *>(
		addReadToGraphCommon(std::move(lab)));
}

const WriteLabel *
ExecutionGraph::addStoreToGraphCommon(std::unique_ptr<WriteLabel> lab)
{
	/* First, we calculate the hb and (po U rf) views */
	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));

	/* Then, we calculate the message view for the store */
	View msg;
	if (llvm::isa<FaiWriteLabel>(&*lab) || llvm::isa<CasWriteLabel>(&*lab)) {
		const EventLabel *pLab = getPreviousLabel(lab.get());

		BUG_ON(pLab->getOrdering() == llvm::AtomicOrdering::NotAtomic);
		BUG_ON(!llvm::isa<ReadLabel>(pLab));

		const ReadLabel *rLab = static_cast<const ReadLabel *>(pLab);
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(getEventLabel(rLab->getRf())))
			msg.update(wLab->getMsgView());

		if (rLab->isAtLeastRelease())
			msg.update(lab->getHbView());
		else
			msg.update(getHbBefore(getLastThreadRelease(lab->getThread(),
								    lab->getAddr())));
	} else {
		if (lab->isAtLeastRelease())
			msg = lab->getHbView();
		else if (lab->getOrdering() == llvm::AtomicOrdering::Monotonic ||
			 lab->getOrdering() == llvm::AtomicOrdering::Acquire)
			msg = getHbBefore(getLastThreadRelease(lab->getThread(),
							       lab->getAddr()));
	}

	lab->setMsgView(std::move(msg));
	return static_cast<const WriteLabel *>(addEventToGraph(std::move(lab)));
}

const WriteLabel *
ExecutionGraph::addStoreToGraph(int tid, llvm::AtomicOrdering ord,
				const llvm::GenericValue *ptr,
				const llvm::Type *typ,
				const llvm::GenericValue &val, int offsetMO,
				bool isUnlock)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<WriteLabel> lab(
		new WriteLabel(nextStamp(), ord, pos, ptr, typ, val, isUnlock));
	modOrder[lab->getAddr()].insert(modOrder[lab->getAddr()].begin() + offsetMO,
					lab->getPos());
	return addStoreToGraphCommon(std::move(lab));
}

const FaiWriteLabel *
ExecutionGraph::addFaiStoreToGraph(int tid, llvm::AtomicOrdering ord,
				   const llvm::GenericValue *ptr,
				   const llvm::Type *typ,
				   const llvm::GenericValue &val,
				   int offsetMO)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<FaiWriteLabel> lab(
		new FaiWriteLabel(nextStamp(), ord, pos, ptr, typ, val));
	modOrder[lab->getAddr()].insert(modOrder[lab->getAddr()].begin() + offsetMO,
					lab->getPos());
	return static_cast<const FaiWriteLabel *>(
		addStoreToGraphCommon(std::move(lab)));
}

const CasWriteLabel *
ExecutionGraph::addCasStoreToGraph(int tid, llvm::AtomicOrdering ord,
				   const llvm::GenericValue *ptr,
				   const llvm::Type *typ,
				   const llvm::GenericValue &val,
				   int offsetMO, bool isLock)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<CasWriteLabel> lab(
		new CasWriteLabel(nextStamp(), ord, pos, ptr, typ, val, isLock));
	modOrder[lab->getAddr()].insert(modOrder[lab->getAddr()].begin() + offsetMO,
					lab->getPos());
	return static_cast<const CasWriteLabel *>(
		addStoreToGraphCommon(std::move(lab)));
}

const LibWriteLabel *
ExecutionGraph::addLibStoreToGraph(int tid, llvm::AtomicOrdering ord,
				   const llvm::GenericValue *ptr,
				   const llvm::Type *typ,
				   llvm::GenericValue &val, int offsetMO,
				   std::string functionName, bool isInit)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<LibWriteLabel> lab(
		new LibWriteLabel(nextStamp(), ord, pos, ptr, typ, val,
				  functionName, isInit));
	modOrder[lab->getAddr()].insert(modOrder[lab->getAddr()].begin() + offsetMO,
					lab->getPos());
	return static_cast<const LibWriteLabel *>(
		addStoreToGraphCommon(std::move(lab)));
}

const FenceLabel *
ExecutionGraph::addFenceToGraph(int tid, llvm::AtomicOrdering ord)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<FenceLabel> lab(new FenceLabel(nextStamp(), ord, pos));

	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());
	if (lab->isAtLeastAcquire())
		calcRelRfPoBefore(lab->getPos().prev(), hb);

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return static_cast<const FenceLabel *>(addEventToGraph(std::move(lab)));
}

const MallocLabel *
ExecutionGraph::addMallocToGraph(int tid, const void *addr, unsigned int size)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<MallocLabel> lab(
		new MallocLabel(nextStamp(), llvm::AtomicOrdering::NotAtomic,
				pos, addr, size));

	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return static_cast<const MallocLabel *>(addEventToGraph(std::move(lab)));
}

const FreeLabel *
ExecutionGraph::addFreeToGraph(int tid, const void *addr, unsigned int size)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<FreeLabel> lab(
		new FreeLabel(nextStamp(), llvm::AtomicOrdering::NotAtomic,
			      pos, addr, size));

	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return static_cast<const FreeLabel *>(addEventToGraph(std::move(lab)));
}

const ThreadCreateLabel *
ExecutionGraph::addTCreateToGraph(int tid, int cid)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<ThreadCreateLabel> lab(
		new ThreadCreateLabel(nextStamp(),
				      llvm::AtomicOrdering::Release, pos, cid));

	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return static_cast<const ThreadCreateLabel *>(addEventToGraph(std::move(lab)));
}

const ThreadJoinLabel *
ExecutionGraph::addTJoinToGraph(int tid, int cid)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<ThreadJoinLabel> lab(
		new ThreadJoinLabel(nextStamp(),
				    llvm::AtomicOrdering::Acquire, pos, cid));

	/* Thread joins have acquire semantics -- but we have to wait
	 * for the other thread to finish first, so we do not fully
	 * update the view yet */
	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return static_cast<const ThreadJoinLabel *>(addEventToGraph(std::move(lab)));
}

const ThreadStartLabel *
ExecutionGraph::addStartToGraph(int tid, Event tc)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<ThreadStartLabel> lab(
		new ThreadStartLabel(nextStamp(),
				     llvm::AtomicOrdering::Acquire, pos, tc));

	/* Thread start has Acquire semantics */
	View hb(getHbBefore(tc));
	hb[tid] = pos.index;

	View porf(getPorfBefore(tc));
	porf[tid] = pos.index;

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return static_cast<const ThreadStartLabel *>(addEventToGraph(std::move(lab)));
}

const ThreadFinishLabel *
ExecutionGraph::addFinishToGraph(int tid)
{
	Event pos(tid, events[tid].size());
	std::unique_ptr<ThreadFinishLabel> lab(
		new ThreadFinishLabel(nextStamp(),
				      llvm::AtomicOrdering::Release, pos));

	View hb = calcBasicHbView(lab.get());
	View porf = calcBasicPorfView(lab.get());

	lab->setHbView(std::move(hb));
	lab->setPorfView(std::move(porf));
	return static_cast<const ThreadFinishLabel *>(addEventToGraph(std::move(lab)));
}

/************************************************************
 ** Calculation of [(po U rf)*] predecessors and successors
 ***********************************************************/

std::vector<Event> ExecutionGraph::getStoresHbAfterStores(const llvm::GenericValue *loc,
							  const std::vector<Event> &chain)
{
	auto &stores = modOrder[loc];
	std::vector<Event> result;

	for (auto &s : stores) {
		if (std::find(chain.begin(), chain.end(), s) != chain.end())
			continue;
		auto &before = getHbBefore(s);
		if (std::any_of(chain.begin(), chain.end(), [&before](Event e)
				{ return e.index < before[e.thread]; }))
			result.push_back(s);
	}
	return result;
}

const View &ExecutionGraph::getPorfBefore(Event e)
{
	return getEventLabel(e)->getPorfView();
}

const View &ExecutionGraph::getHbBefore(Event e)
{
	return getEventLabel(e)->getHbView();
}

View ExecutionGraph::getHbBefore(const std::vector<Event> &es)
{
	View v;

	for (auto &e : es)
		v.update(getEventLabel(e)->getHbView());
	return v;
}

const View &ExecutionGraph::getHbPoBefore(Event e)
{
	return getHbBefore(e.prev());
}

void ExecutionGraph::calcHbRfBefore(Event e, const llvm::GenericValue *addr,
				    View &a)
{
	if (a.contains(e))
		return;
	int ai = a[e.thread];
	a[e.thread] = e.index;
	for (int i = ai + 1; i <= e.index; i++) {
		const EventLabel *lab = getEventLabel(Event(e.thread, i));
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			if (rLab->getAddr() == addr ||
			    rLab->getHbView().contains(rLab->getRf()))
				calcHbRfBefore(rLab->getRf(), addr, a);
		} else if (auto *bLab = llvm::dyn_cast<ThreadStartLabel>(lab)) {
			calcHbRfBefore(bLab->getParentCreate(), addr, a);
		} else if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab)) {
			calcHbRfBefore(jLab->getChildLast(), addr, a);
		}
	}
	return;
}

View ExecutionGraph::getHbRfBefore(const std::vector<Event> &es)
{
	View a;

	for (auto &e : es) {
		const EventLabel *lab = getEventLabel(e);
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
			calcHbRfBefore(e, rLab->getAddr(), a);
		} else {
			a.update(lab->getHbView());
		}
	}
	return a;
}

void ExecutionGraph::calcRelRfPoBefore(Event last, View &v)
{
	for (auto i = last.index; i > 0; i--) {
		const EventLabel *lab = getEventLabel(Event(last.thread, i));
		if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire())
			return;
		if (!llvm::isa<ReadLabel>(lab))
			continue;
		auto *rLab = static_cast<const ReadLabel *>(lab);
		if (rLab->getOrdering() == llvm::AtomicOrdering::Monotonic ||
		    rLab->getOrdering() == llvm::AtomicOrdering::Release) {
			const EventLabel *rfLab = getEventLabel(rLab->getRf());
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
				v.update(wLab->getMsgView());
		}
	}
}


/************************************************************
 ** Calculation of particular sets of events/event labels
 ***********************************************************/

std::vector<std::unique_ptr<EventLabel> >
ExecutionGraph::getPrefixLabelsNotBefore(const View &prefix, const View &before)
{
	std::vector<std::unique_ptr<EventLabel> > result;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = before[i] + 1; j <= prefix[i]; j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			result.push_back(std::unique_ptr<EventLabel>(lab->clone()));

			auto &curLab = result.back();
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(curLab.get())) {
				wLab->removeReader([&](Event r) {
						return !prefix.contains(r) &&
						       !before.contains(r);
					});
			} else if (auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(curLab.get())) {
				if (!prefix.contains(eLab->getParentJoin()) &&
				    !before.contains(eLab->getParentJoin()))
					eLab->setParentJoin(Event::getInitializer());
			} else if (auto *cLab = llvm::dyn_cast<ThreadCreateLabel>(curLab.get())) {
				/* We can keep the begin event of the child
				 * the since it will not be deleted */
				;
			}
		}
	}
	return result;
}

std::vector<Event>
ExecutionGraph::getRfsNotBefore(const std::vector<std::unique_ptr<EventLabel> > &labs,
				const View &before)
{
	std::vector<Event> rfs;

	for (auto const &lab : labs) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab.get())) {
			if (!before.contains(rLab->getPos()))
				rfs.push_back(rLab->getRf());
		}
	}
	return rfs;
}

std::vector<std::pair<Event, Event> >
ExecutionGraph::getMOPredsInBefore(const std::vector<std::unique_ptr<EventLabel> > &labs,
				   const View &before)
{
	std::vector<std::pair<Event, Event> > pairs;

	for (const auto &lab : labs) {
		/* Only store MO pairs for labels that are not in before */
		if (!llvm::isa<WriteLabel>(lab.get()) || before.contains(lab->getPos()))
			continue;

		auto *wLab = static_cast<const WriteLabel *>(lab.get());
		auto &locMO = modOrder[wLab->getAddr()];
		auto moPos = std::find(locMO.begin(), locMO.end(), wLab->getPos());

		/* This store must definitely be in this location's MO */
		BUG_ON(moPos == locMO.end());

		/* We need to find the previous MO store that is in before or
		 * in the vector for which we are getting the predecessors */
		std::reverse_iterator<std::vector<Event>::iterator> predPos(moPos);
		auto predFound = false;
		for (auto rit = predPos; rit != locMO.rend(); ++rit) {
			if (before.contains(*rit) ||
			    std::find_if(labs.begin(), labs.end(),
					 [&](const std::unique_ptr<EventLabel> &lab)
					 { return lab->getPos() == *rit; })
			    != labs.end()) {
				pairs.push_back(std::make_pair(*moPos, *rit));
				predFound = true;
				break;
			}
		}
		/* If there is not predecessor in the vector or in before,
		 * then INIT is the only valid predecessor */
		if (!predFound)
			pairs.push_back(std::make_pair(*moPos, Event::getInitializer()));
	}
	return pairs;
}


/************************************************************
 ** Calculation of writes a read can read from
 ***********************************************************/

bool ExecutionGraph::isWriteRfBefore(const View &before, Event e)
{
	if (before.contains(e))
		return true;

	const EventLabel *lab = getEventLabel(e);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (auto &e : wLab->getReadersList())
		if (before.contains(e))
			return true;
	return false;
}

bool ExecutionGraph::isStoreReadByExclusiveRead(Event store, const llvm::GenericValue *ptr)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!isRMWLoad(lab))
				continue;

			auto *rLab = static_cast<const ReadLabel *>(lab);
			if (rLab->getRf() == store && rLab->getAddr() == ptr)
				return true;
		}
	}
	return false;
}

bool ExecutionGraph::isStoreReadBySettledRMW(Event store, const llvm::GenericValue *ptr, const View &porfBefore)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!isRMWLoad(lab))
				continue;

			auto *rLab = static_cast<const ReadLabel *>(lab);
			if (rLab->getRf() != store || rLab->getAddr() != ptr)
				continue;

			if (!rLab->isRevisitable())
				return true;
			if (porfBefore.contains(rLab->getPos()))
				return true;
		}
	}
	return false;
}

std::vector<Event> ExecutionGraph::findOverwrittenBoundary(const llvm::GenericValue *addr, int thread)
{
	std::vector<Event> boundary;
	auto &before = getHbBefore(getLastThreadEvent(thread));

	if (before.empty())
		return boundary;

	for (auto &e : modOrder[addr])
		if (isWriteRfBefore(before, e))
			boundary.push_back(e.prev());
	return boundary;
}


/************************************************************
 ** Graph modification methods
 ***********************************************************/

void ExecutionGraph::changeRf(Event read, Event store)
{
	EventLabel *lab = events[read.thread][read.index].get();
	BUG_ON(!llvm::isa<ReadLabel>(lab));

	/* First, we set the new reads-from edge */
	ReadLabel *rLab = static_cast<ReadLabel *>(lab);
	Event oldRf = rLab->getRf();
	rLab->setRf(store);

	/*
	 * Now, we need to delete the read from the readers list of oldRf.
	 * For that we need to check:
	 *     1) That the old write it was reading from still exists
	 *     2) That oldRf has not been deleted (and a different event is
	 *        now in its place, perhaps after the restoration of some prefix
	 *        during a revisit)
	 *     3) That oldRf is not th initializer */
	if (oldRf.index < (int) events[oldRf.thread].size()) {
		EventLabel *labRef = events[oldRf.thread][oldRf.index].get();
		if (auto *oldLab = llvm::dyn_cast<WriteLabel>(labRef))
			oldLab->removeReader([&](Event r){ return r == rLab->getPos(); });
	}

	EventLabel *rfLab = events[store.thread][store.index].get();
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
		wLab->addReader(rLab->getPos());

	/* Update the views of the load */
	View hb = calcBasicHbView(rLab);
	View porf = calcBasicPorfView(rLab);

	porf.update(rfLab->getPorfView());
	if (rLab->isAtLeastAcquire()) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab))
			hb.update(wLab->getMsgView());
	}
	rLab->setHbView(std::move(hb));
	rLab->setPorfView(std::move(porf));
}

bool ExecutionGraph::updateJoin(Event join, Event childLast)
{
	EventLabel *lab = events[join.thread][join.index].get();
	BUG_ON(!llvm::isa<ThreadJoinLabel>(lab));

	auto *jLab = static_cast<ThreadJoinLabel *>(lab);
	EventLabel *cLastLab = events[childLast.thread][childLast.index].get();

	/* If the child thread has not terminated, do not update anything */
	if (!llvm::isa<ThreadFinishLabel>(cLastLab))
		return false;

	auto *fLab = static_cast<ThreadFinishLabel *>(cLastLab);

	jLab->setChildLast(childLast);
	fLab->setParentJoin(jLab->getPos());
	jLab->hbView.update(fLab->hbView);
	jLab->porfView.update(fLab->porfView);
	return true;
}

void ExecutionGraph::cutToView(const View &preds)
{
	/* Restrict the graph according to the view */
	for (auto i = 0u; i < events.size(); i++) {
		auto &thr = events[i];
		thr.erase(thr.begin() + preds[i] + 1, thr.end());
	}

	/* Remove any 'pointers' to events that have been removed */
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			EventLabel *lab = events[i][j].get();
			/*
			 * If it is a join and the respective Finish has been
			 * removed, renew the Views of this label and continue
			 */
			if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(lab)) {
				Event cl = jLab->getChildLast();
				if (cl.index >= (int) events[cl.thread].size()) {
					jLab->setChildLast(Event::getInitializer());
					View hb = calcBasicHbView(jLab);
					View porf = calcBasicPorfView(jLab);
					jLab->setHbView(std::move(hb));
					jLab->setPorfView(std::move(porf));
				}
			} else if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				wLab->removeReader([&](Event r){
						return !preds.contains(r);
					});
			}
			/* No special action for CreateLabels; we can
			 * keep the begin event of the child the since
			 * it will not be deleted */
		}
	}

	/* Remove cutted events from the modification order as well */
	for (auto it = modOrder.begin(); it != modOrder.end(); ++it)
		it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
						[&](Event &e)
						{ return !preds.contains(e); }),
				 it->second.end());
	return;
}

View ExecutionGraph::getViewFromStamp(unsigned int stamp)
{
	View preds;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = (int) events[i].size() - 1; j >= 0; j--) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (lab->getStamp() <= stamp) {
				preds[i] = j;
				break;
			}
		}
	}
	return preds;
}

void ExecutionGraph::cutToStamp(unsigned int stamp)
{
	auto preds = getViewFromStamp(stamp);
	cutToView(preds);
	return;
}

void ExecutionGraph::restoreStorePrefix(const ReadLabel *rLab,
					std::vector<std::unique_ptr<EventLabel> > &storePrefix,
					std::vector<std::pair<Event, Event> > &moPlacings)
{
	std::vector<Event> inserted;

	for (auto &lab : storePrefix) {
		BUG_ON(lab->getIndex() != (int) events[lab->getThread()].size() &&
		       "Events should be added in order!");
		inserted.push_back(lab->getPos());
		events[lab->getThread()].push_back(std::move(lab));
	}

	for (const auto &e : inserted) {
		EventLabel *curLab = events[e.thread][e.index].get();
		if (auto *curRLab = llvm::dyn_cast<ReadLabel>(curLab)) {
			curRLab->setRevisitStatus(false);
			Event curRf = curRLab->getRf();
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(
				    events[curRf.thread][curRf.index].get())) {
				wLab->addReader(curRLab->getPos());
			}
		} else if (auto *curWLab = llvm::dyn_cast<WriteLabel>(curLab)) {
			curWLab->removeReader([&](Event r){ return r == rLab->getPos(); });
		}
	}

	/* If there are no specific mo placings, just insert all stores */
	if (moPlacings.empty()) {
		for (const auto &e : inserted) {
			const EventLabel *lab = getEventLabel(e);
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab))
				modOrder.addAtLocEnd(wLab->getAddr(), wLab->getPos());
		}
		return;
	}

	/* Otherwise, insert the writes of storePrefix into the appropriate places */
	auto insertedMO = 0u;
	while (insertedMO < moPlacings.size()) {
		for (auto it = moPlacings.begin(); it != moPlacings.end(); ++it) {
			/* it->fist is a WriteLabel by construction */
			auto *lab = static_cast<const WriteLabel *>(getEventLabel(it->first));
			if (modOrder.locContains(lab->getAddr(), it->second) &&
			    !modOrder.locContains(lab->getAddr(), it->first)) {
				modOrder.addAtLocAfter(lab->getAddr(), it->second, it->first);
				++insertedMO;
			}
		}
	}
}

bool ExecutionGraph::revisitSetContains(const ReadLabel *r, const std::vector<Event> &writePrefix,
					const std::vector<std::pair<Event, Event> > &moPlacings) const
{
	EventLabel *lab = events[r->getThread()][r->getIndex()].get();
	BUG_ON(!llvm::isa<ReadLabel>(lab));

	ReadLabel *rLab = static_cast<ReadLabel *>(lab);
	return rLab->revs.contains(writePrefix, moPlacings);
}

void ExecutionGraph::addToRevisitSet(const ReadLabel *r, const std::vector<Event> &writePrefix,
				     const std::vector<std::pair<Event, Event> > &moPlacings)
{
	EventLabel *lab = events[r->getThread()][r->getIndex()].get();
	BUG_ON(!llvm::isa<ReadLabel>(lab));

	ReadLabel *rLab = static_cast<ReadLabel *>(lab);
	return rLab->revs.add(writePrefix, moPlacings);
}


/************************************************************
 ** Consistency checks
 ***********************************************************/

bool ExecutionGraph::isConsistent(void)
{
	return true;
}


/************************************************************
 ** Race detection methods
 ***********************************************************/

Event ExecutionGraph::findRaceForNewLoad(const ReadLabel *rLab)
{
	const View &before = getPreviousLabel(rLab)->getHbView();
	std::vector<Event> &stores = modOrder[rLab->getAddr()];

	/* If there are not any events hb-before the read, there is nothing to do */
	if (before.empty())
		return Event::getInitializer();

	/* Check for events that race with the current load */
	for (auto &s : stores) {
		if (before.contains(s))
			continue;

		auto *sLab = static_cast<const WriteLabel *>(getEventLabel(s));
		if ((rLab->isNotAtomic() || sLab->isNotAtomic()) &&
		    rLab->getPos() != sLab->getPos())
			return s; /* Race detected! */
	}
	return Event::getInitializer(); /* Race not found */
}

Event ExecutionGraph::findRaceForNewStore(const WriteLabel *wLab)
{
	auto &before = getPreviousLabel(wLab)->getHbView();

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = before[i] + 1u; j < events[i].size(); j++) {
			const EventLabel *oLab = getEventLabel(Event(i, j));
			/* If they are both atomics, nothing to check */
			if (!wLab->isNotAtomic() && !oLab->isNotAtomic())
				continue;
			if (!llvm::isa<MemAccessLabel>(oLab))
				continue;

			auto *mLab = static_cast<const MemAccessLabel *>(oLab);
			if (mLab->getAddr() == wLab->getAddr() &&
			    mLab->getPos() != wLab->getPos())
				return mLab->getPos(); /* Race detected */
		}
	}
	return Event::getInitializer(); /* Race not found */
}


/************************************************************
 ** PSC calculation
 ***********************************************************/

bool ExecutionGraph::isRMWLoad(const EventLabel *lab)
{
	if (!llvm::isa<CasReadLabel>(lab) && !llvm::isa<FaiReadLabel>(lab))
		return false;
	const ReadLabel *rLab = static_cast<const ReadLabel *>(lab);

	if (lab->getIndex() == (int) events[lab->getThread()].size() - 1)
		return false;
	const EventLabel *nLab =
		getEventLabel(Event(rLab->getThread(), rLab->getIndex() + 1));

	if (!llvm::isa<MemAccessLabel>(nLab))
		return false;
	auto *mLab = static_cast<const MemAccessLabel *>(nLab);

	if ((llvm::isa<CasWriteLabel>(mLab) || llvm::isa<FaiWriteLabel>(mLab)) &&
	    mLab->getAddr() == rLab->getAddr())
		return true;
	return false;
}

bool ExecutionGraph::isRMWLoad(Event e)
{
	return isRMWLoad(getEventLabel(e));
}

std::pair<std::vector<Event>, std::vector<Event> >
ExecutionGraph::getSCs()
{
	std::vector<Event> scs, fcs;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (lab->isSC() && !isRMWLoad(lab))
				scs.push_back(lab->getPos());
			if (lab->isSC() && llvm::isa<FenceLabel>(lab))
				fcs.push_back(lab->getPos());
		}
	}
	return std::make_pair(scs,fcs);
}

std::vector<const llvm::GenericValue *> ExecutionGraph::getDoubleLocs()
{
	std::vector<const llvm::GenericValue *> singles, doubles;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 1u; j < events[i].size(); j++) { /* Do not consider thread inits */
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (!llvm::isa<MemAccessLabel>(lab))
				continue;

			auto *mLab = static_cast<const MemAccessLabel *>(lab);
			if (std::find(doubles.begin(), doubles.end(),
				      mLab->getAddr()) != doubles.end())
				continue;
			if (std::find(singles.begin(), singles.end(),
				      mLab->getAddr()) != singles.end()) {
				singles.erase(std::remove(singles.begin(),
							  singles.end(),
							  mLab->getAddr()),
					      singles.end());
				doubles.push_back(mLab->getAddr());
			} else {
				singles.push_back(mLab->getAddr());
			}
		}
	}
	return doubles;
}

std::vector<Event> ExecutionGraph::calcSCFencesSuccs(const std::vector<Event> &fcs,
						     const Event e)
{
	std::vector<Event> succs;

	if (isRMWLoad(e))
		return succs;
	for (auto &f : fcs) {
		if (getHbBefore(f).contains(e))
			succs.push_back(f);
	}
	return succs;
}

std::vector<Event> ExecutionGraph::calcSCFencesPreds(const std::vector<Event> &fcs, const Event e)
{
	std::vector<Event> preds;
	auto &before = getHbBefore(e);

	if (isRMWLoad(e))
		return preds;
	for (auto &f : fcs) {
		if (before.contains(f))
			preds.push_back(f);
	}
	return preds;
}

std::vector<Event> ExecutionGraph::calcSCSuccs(const std::vector<Event> &fcs, const Event e)
{
	const EventLabel *lab = getEventLabel(e);

	if (isRMWLoad(lab))
		return {};
	if (lab->isSC())
		return {e};
	else
		return calcSCFencesSuccs(fcs, e);
}

std::vector<Event> ExecutionGraph::calcSCPreds(const std::vector<Event> &fcs,
					       const Event e)
{
	const EventLabel *lab = getEventLabel(e);

	if (isRMWLoad(lab))
		return {};
	if (lab->isSC())
		return {e};
	else
		return calcSCFencesPreds(fcs, e);
}

std::vector<Event> ExecutionGraph::getSCRfSuccs(const std::vector<Event> &fcs,
						const Event ev)
{
	const EventLabel *lab = getEventLabel(ev);
	std::vector<Event> rfs;

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto succs = calcSCSuccs(fcs, e);
		rfs.insert(rfs.end(), succs.begin(), succs.end());
	}
	return rfs;
}

std::vector<Event> ExecutionGraph::getSCFenceRfSuccs(const std::vector<Event> &fcs,
						     const Event ev)
{
	const EventLabel *lab = getEventLabel(ev);
	std::vector<Event> fenceRfs;

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto fenceSuccs = calcSCFencesSuccs(fcs, e);
		fenceRfs.insert(fenceRfs.end(), fenceSuccs.begin(), fenceSuccs.end());
	}
	return fenceRfs;
}

void ExecutionGraph::addRbEdges(std::vector<Event> &fcs, std::vector<Event> &moAfter,
				std::vector<Event> &moRfAfter, Matrix2D<Event> &matrix,
				const Event &ev)
{
	const EventLabel *lab = getEventLabel(ev);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto preds = calcSCPreds(fcs, e);
		auto fencePreds = calcSCFencesPreds(fcs, e);

		matrix.addEdgesFromTo(preds, moAfter);        /* PSC_base: Adds rb-edges */
		matrix.addEdgesFromTo(fencePreds, moRfAfter); /* PSC_fence: Adds (rb;rf)-edges */
	}
	return;
}

void ExecutionGraph::addMoRfEdges(std::vector<Event> &fcs, std::vector<Event> &moAfter,
				  std::vector<Event> &moRfAfter, Matrix2D<Event> &matrix,
				  const Event &ev)
{
	auto preds = calcSCPreds(fcs, ev);
	auto fencePreds = calcSCFencesPreds(fcs, ev);
	auto rfs = getSCRfSuccs(fcs, ev);

	matrix.addEdgesFromTo(preds, moAfter);        /* PSC_base:  Adds mo-edges */
	matrix.addEdgesFromTo(preds, rfs);            /* PSC_base:  Adds rf-edges */
	matrix.addEdgesFromTo(fencePreds, moRfAfter); /* PSC_fence: Adds (mo;rf)-edges */
	return;
}

/*
 * addSCEcos - Helper function that calculates a part of PSC_base and PSC_fence
 *
 * For PSC_base, it adds mo, rb, and hb_loc edges. The procedure for mo and rb
 * is straightforward: at each point, we only need to keep a list of all the
 * mo-after writes that are either SC, or can reach an SC fence. For hb_loc,
 * however, we only consider rf-edges because the other cases are implicitly
 * covered (sb, mo, etc).
 *
 * For PSC_fence, it adds (mo;rf)- and (rb;rf)-edges. Simple cases like
 * mo, rf, and rb are covered by PSC_base, and all other combinations with
 * more than one step either do not compose, or lead to an already added
 * single-step relation (e.g, (rf;rb) => mo, (rb;mo) => rb)
 */
void ExecutionGraph::addSCEcos(std::vector<Event> &fcs,
			       std::vector<Event> &mo, Matrix2D<Event> &matrix)
{
	std::vector<Event> moAfter;   /* mo-after SC writes or writes that reach an SC fence */
	std::vector<Event> moRfAfter; /* SC fences that can be reached by (mo;rf)-after reads */

	for (auto rit = mo.rbegin(); rit != mo.rend(); rit++) {

		addRbEdges(fcs, moAfter, moRfAfter, matrix, *rit);
		addMoRfEdges(fcs, moAfter, moRfAfter, matrix, *rit);

		auto succs = calcSCSuccs(fcs, *rit);
		auto fenceRfs = getSCFenceRfSuccs(fcs, *rit);
		moAfter.insert(moAfter.end(), succs.begin(), succs.end());
		moRfAfter.insert(moRfAfter.end(), fenceRfs.begin(), fenceRfs.end());
	}
}

/*
 * addSCWbEcos is a helper function that calculates a part of PSC_base and PSC_fence,
 * like addSCEcos. The difference between them lies in the fact that addSCEcos
 * uses MO for adding mo and rb edges, addSCWBEcos uses WB for that.
 */
void ExecutionGraph::addSCWbEcos(std::vector<Event> &fcs,
				 Matrix2D<Event> &wbMatrix,
				 Matrix2D<Event> &pscMatrix)
{
	auto &stores = wbMatrix.getElems();
	for (auto i = 0u; i < stores.size(); i++) {

		/*
		 * Calculate which of the stores are wb-after the current
		 * write, and then collect wb-after and (wb;rf)-after SC successors
		 */
		std::vector<Event> wbAfter, wbRfAfter;
		for (auto j = 0u; j < stores.size(); j++) {
			if (wbMatrix(i, j)) {
				auto succs = calcSCSuccs(fcs, stores[j]);
				auto fenceRfs = getSCFenceRfSuccs(fcs, stores[j]);
				wbAfter.insert(wbAfter.end(), succs.begin(), succs.end());
				wbRfAfter.insert(wbRfAfter.end(), fenceRfs.begin(), fenceRfs.end());
			}
		}

		/* Then, add the proper edges to PSC using wb-after and (wb;rf)-after successors */
		addRbEdges(fcs, wbAfter, wbRfAfter, pscMatrix, stores[i]);
		addMoRfEdges(fcs, wbAfter, wbRfAfter, pscMatrix, stores[i]);
	}
}

void ExecutionGraph::addSbHbEdges(Matrix2D<Event> &matrix)
{
	auto &scs = matrix.getElems();
	for (auto i = 0u; i < scs.size(); i++) {
		for (auto j = 0u; j < scs.size(); j++) {
			if (i == j)
				continue;
			const EventLabel *eiLab = getEventLabel(scs[i]);
			const EventLabel *ejLab = getEventLabel(scs[j]);

			/* PSC_base: Adds sb-edges*/
			if (eiLab->getThread() == ejLab->getThread()) {
				if (eiLab->getIndex() < ejLab->getIndex())
					matrix(i, j) = true;
				continue;
			}

			/* PSC_base: Adds sb_(<>loc);hb;sb_(<>loc) edges */
			const EventLabel *ejPrevLab = getPreviousLabel(ejLab);
			if (!llvm::isa<MemAccessLabel>(ejPrevLab) ||
			    !llvm::isa<MemAccessLabel>(ejLab) ||
			    !llvm::isa<MemAccessLabel>(eiLab))
				continue;

			auto *ejPrevMLab = static_cast<const MemAccessLabel *>(ejPrevLab);
			auto *ejMLab = static_cast<const MemAccessLabel *>(ejLab);
			auto *eiMLab = static_cast<const MemAccessLabel *>(eiLab);

			if (ejPrevMLab->getAddr() != ejMLab->getAddr() &&
			    ejPrevMLab->getHbView().contains(eiMLab->getPos())) {
				Event next = eiMLab->getPos().next();
				if (next == getLastThreadEvent(eiMLab->getThread()))
					continue;
				const EventLabel *eiNextLab = getEventLabel(next);
				if (auto *eiNextMLab =
				    llvm::dyn_cast<MemAccessLabel>(eiNextLab)) {
					if (eiMLab->getAddr() != eiNextMLab->getAddr())
						matrix(i, j) = true;
				}
			}
		}
	}
	return;
}

void ExecutionGraph::addInitEdges(std::vector<Event> &fcs, Matrix2D<Event> &matrix)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			/* Consider only reads that read from the initializer write */
			if (!llvm::isa<ReadLabel>(lab) || isRMWLoad(lab))
				continue;
			auto *rLab = static_cast<const ReadLabel *>(lab);
			if (!rLab->getRf().isInitializer())
				continue;

			auto preds = calcSCPreds(fcs, rLab->getPos());
			auto fencePreds = calcSCFencesPreds(fcs, rLab->getPos());
			for (auto &w : modOrder[rLab->getAddr()]) {
				/* Can be casted to WriteLabel by construction */
				auto *wLab = static_cast<const WriteLabel *>(
					getEventLabel(w));
				auto wSuccs = calcSCSuccs(fcs, w);
				matrix.addEdgesFromTo(preds, wSuccs); /* Adds rb-edges */
				for (auto &r : wLab->getReadersList()) {
					auto fenceSuccs = calcSCFencesSuccs(fcs, r);
					matrix.addEdgesFromTo(fencePreds, fenceSuccs); /*Adds (rb;rf)-edges */
				}
			}
		}
	}
	return;
}

bool ExecutionGraph::isPscWeakAcyclicWB()
{
	/* Collect all SC events (except for RMW loads) */
	auto accesses = getSCs();
	auto &scs = accesses.first;
	auto &fcs = accesses.second;

	/* If there are no SC events, it is a valid execution */
	if (scs.empty())
		return true;

	Matrix2D<Event> matrix(scs);

	/* Add edges from the initializer write (special case) */
	addInitEdges(fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(matrix);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence for only
	 * _one_ possible extension of WB for each location
	 */
	std::vector<const llvm::GenericValue *> scLocs = getDoubleLocs();
	for (auto loc : scLocs) {
		auto wb = calcWb(loc);
		auto sortedStores = wb.topoSort();
		addSCEcos(fcs, sortedStores, matrix);
	}

	matrix.transClosure();
	return !matrix.isReflexive();
}

bool ExecutionGraph::isPscWbAcyclicWB()
{
	/* Collect all SC events (except for RMW loads) */
	auto accesses = getSCs();
	auto &scs = accesses.first;
	auto &fcs = accesses.second;

	/* If there are no SC events, it is a valid execution */
	if (scs.empty())
		return true;

	Matrix2D<Event> matrix(scs);

	/* Add edges from the initializer write (special case) */
	addInitEdges(fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(matrix);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence using WB
	 * instead of MO
	 */
	std::vector<const llvm::GenericValue *> scLocs = getDoubleLocs();
	for (auto loc : scLocs) {
		auto wb = calcWb(loc);
		addSCWbEcos(fcs, wb, matrix);
	}

	matrix.transClosure();
	return !matrix.isReflexive();
}

bool ExecutionGraph::isPscAcyclicWB()
{
	/* Collect all SC events (except for RMW loads) */
	auto accesses = getSCs();
	auto &scs = accesses.first;
	auto &fcs = accesses.second;

	/* If there are no SC events, it is a valid execution */
	if (scs.empty())
		return true;

	Matrix2D<Event> matrix(scs);

	/* Add edges from the initializer write (special case) */
	addInitEdges(fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(matrix);

	/*
	 * Collect memory locations that have more than one SC
	 * memory access, and then calculate the possible extensions
	 * of writes-before (WB) for these memory locations
	 */
	std::vector<const llvm::GenericValue *> scLocs = getDoubleLocs();
	std::vector<std::vector<std::vector<Event> > > topoSorts(scLocs.size());
	for (auto i = 0u; i < scLocs.size(); i++) {
		auto wb = calcWb(scLocs[i]);
		topoSorts[i] = wb.allTopoSort();
	}

	unsigned int K = topoSorts.size();
	std::vector<unsigned int> count(K, 0);

	/*
	 * It suffices to find one combination for the WB extensions of all
	 * locations, for which PSC is acyclic. This loop is like an odometer:
	 * given an array that contains K vectors, we keep a counter for each
	 * vector, and proceed by incrementing the rightmost counter. Like in
	 * addition, if a carry is created, this is propagated to the left.
	 */
	while (count[0] < topoSorts[0].size()) {
		/* Process current combination */
		auto tentativePSC(matrix);
		for (auto i = 0u; i < K; i++)
			addSCEcos(fcs, topoSorts[i][count[i]], tentativePSC);

		tentativePSC.transClosure();
		if (!tentativePSC.isReflexive())
			return true;

		/* Find next combination */
		++count[K - 1];
		for (auto i = K - 1; (i > 0) && (count[i] == topoSorts[i].size()); --i) {
			count[i] = 0;
			++count[i - 1];
		}
	}

	/* No valid MO combination found */
	return false;
}

bool ExecutionGraph::isPscAcyclicMO()
{
	/* Collect all SC events (except for RMW loads) */
	auto accesses = getSCs();
	auto &scs = accesses.first;
	auto &fcs = accesses.second;

	/* If there are no SC events, it is a valid execution */
	if (scs.empty())
		return true;

	Matrix2D<Event> matrix(scs);

	/* Add edges from the initializer write (special case) */
	addInitEdges(fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(matrix);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence for only
	 * _one_ possible extension of WB for each location
	 */
	std::vector<const llvm::GenericValue *> scLocs = getDoubleLocs();
	for (auto loc : scLocs) {
		auto &stores = modOrder[loc];
		addSCEcos(fcs, stores, matrix);
	}

	matrix.transClosure();
	return !matrix.isReflexive();
}

/*
 * Given a WB matrix returns a vector that, for each store in the WB
 * matrix, contains the index (in the WB matrix) of the upper and the
 * lower limit of the RMW chain that store belongs to. We use N for as
 * the index of the initializer write, where N is the number of stores
 * in the WB matrix.
 *
 * The vector is partitioned into 3 parts: | UPPER | LOWER | LOWER_I |
 *
 * The first part contains the upper limits, the second part the lower
 * limits and the last part the lower limit for the initiazizer write,
 * which is not part of the WB matrix.
 *
 * If there is an atomicity violation in the graph, the returned
 * vector is empty. (This may happen as part of some optimization,
 * e.g., in getRevisitLoads(), where the view of the resulting graph
 * is not calculated.)
 */
std::vector<unsigned int> ExecutionGraph::calcRMWLimits(const Matrix2D<Event> &wb)
{
	auto &s = wb.getElems();
	auto size = s.size();

	/* upperL is the vector to return, with size (2 * N + 1) */
	std::vector<unsigned int> upperL(2 * size + 1);
	std::vector<unsigned int>::iterator lowerL = upperL.begin() + size;

	/*
	 * First, we initialize the vector. For the upper limit of a
	 * non-RMW store we set the store itself, and for an RMW-store
	 * its predecessor in the RMW chain. For the lower limit of
	 * any store we set the store itself.  For the initializer
	 * write, we use "size" as its index, since it does not exist
	 * in the WB matrix.
	 */
	for (auto i = 0u; i < size; i++) {
		/* static casts below are based on the construction of the EG */
		auto *wLab = static_cast<const WriteLabel *>(getEventLabel(s[i]));
		if (llvm::isa<FaiWriteLabel>(wLab) || llvm::isa<CasWriteLabel>(wLab)) {
			auto *pLab = static_cast<const ReadLabel *>(
				getPreviousLabel(wLab));
			Event prev = pLab->getRf();
			upperL[i] = (prev == Event::getInitializer()) ? size :
				wb.getIndex(prev);
		} else {
			upperL[i] = i;
		}
		lowerL[i] = i;
	}
	lowerL[size] = size;

	/*
	 * Next, we set the lower limit of the upper limit of an RMW
	 * store to be the store itself.
	 */
	for (auto i = 0u; i < size; i++) {
		auto ui = upperL[i];
		if (ui == i)
			continue;
		if (lowerL[ui] != ui) {
			/* If the lower limit of this upper limit has already
			 * been set, we have two RMWs reading from the same write */
			upperL.clear();
			return upperL;
		}
		lowerL[upperL[i]] = i;
	}

	/*
	 * Calculate the actual upper limit, by taking the
	 * predecessor's predecessor as an upper limit, until the end
	 * of the chain is reached.
	 */
        bool changed;
	do {
		changed = false;
		for (auto i = 0u; i < size; i++) {
			auto j = upperL[i];
			if (j == size || j == i)
				continue;

			auto k = upperL[j];
			if (j == k)
				continue;
			upperL[i] = k;
			changed = true;
		}
	} while (changed);

	/* Similarly for the lower limits */
	do {
		changed = false;
		for (auto i = 0u; i <= size; i++) {
			auto j = lowerL[i];
			if (j == i)
				continue;
			auto k = lowerL[j];
			if (j == k)
				continue;
			lowerL[i] = k;
			changed = true;
		}
	} while (changed);
	return upperL;
}

Matrix2D<Event> ExecutionGraph::calcWbRestricted(const llvm::GenericValue *addr, const View &v)
{
	std::vector<Event> storesInView;

	std::copy_if(modOrder[addr].begin(), modOrder[addr].end(),
		     std::back_inserter(storesInView),
		     [&](Event &s){ return v.contains(s); });

	Matrix2D<Event> matrix(std::move(storesInView));
	auto &stores = matrix.getElems();

	/* Optimization */
	if (stores.size() <= 1)
		return matrix;

	auto upperLimit = calcRMWLimits(matrix);
	if (upperLimit.empty()) {
		for (auto i = 0u; i < stores.size(); i++)
			matrix(i,i) = true;
		return matrix;
	}

	auto lowerLimit = upperLimit.begin() + stores.size();

	for (auto i = 0u; i < stores.size(); i++) {
		auto *wLab = static_cast<const WriteLabel *>(getEventLabel(stores[i]));

		std::vector<Event> es;
		const std::vector<Event> readers = wLab->getReadersList();
		std::copy_if(readers.begin(), readers.end(), std::back_inserter(es),
			     [&](const Event &r){ return v.contains(r); });

		auto before = getHbBefore(es).
			update(getPreviousLabel(wLab)->getHbView());
		auto upi = upperLimit[i];
		for (auto j = 0u; j < stores.size(); j++) {
			if (i == j || !isWriteRfBefore(before, stores[j]))
				continue;
			matrix(j, i) = true;

			if (upi == stores.size() || upi == upperLimit[j])
				continue;
			matrix(lowerLimit[j], upi) = true;
		}

		if (lowerLimit[stores.size()] == stores.size() || upi == stores.size())
			continue;
		matrix(lowerLimit[stores.size()], i) = true;
	}
	matrix.transClosure();
	return matrix;
}

Matrix2D<Event> ExecutionGraph::calcWb(const llvm::GenericValue *addr)
{
	Matrix2D<Event> matrix(modOrder[addr]);
	auto &stores = matrix.getElems();

	/* Optimization */
	if (stores.size() <= 1)
		return matrix;

	auto upperLimit = calcRMWLimits(matrix);
	if (upperLimit.empty()) {
		for (auto i = 0u; i < stores.size(); i++)
			matrix(i,i) = true;
		return matrix;
	}

	auto lowerLimit = upperLimit.begin() + stores.size();

	for (auto i = 0u; i < stores.size(); i++) {
		auto *wLab = static_cast<const WriteLabel *>(getEventLabel(stores[i]));
		auto before = getHbBefore(wLab->getReadersList()).
			update(getPreviousLabel(wLab)->getHbView());

		auto upi = upperLimit[i];
		for (auto j = 0u; j < stores.size(); j++) {
			if (i == j || !isWriteRfBefore(before, stores[j]))
				continue;
			matrix(j, i) = true;
			if (upi == stores.size() || upi == upperLimit[j])
				continue;
			matrix(lowerLimit[j], upi) = true;
		}

		if (lowerLimit[stores.size()] == stores.size() || upi == stores.size())
			continue;
		matrix(lowerLimit[stores.size()], i) = true;
	}
	matrix.transClosure();
	return matrix;
}

bool ExecutionGraph::isWbAcyclic(void)
{
	for (auto it = modOrder.begin(); it != modOrder.end(); ++it) {
		auto wb = calcWb(it->first);
		if (wb.isReflexive())
			return false;
	}
	return true;
}


/************************************************************
 ** Library consistency checking methods
 ***********************************************************/

std::vector<Event> ExecutionGraph::getLibEventsInView(const Library &lib, const View &v)
{
	std::vector<Event> result;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 1; j <= v[i]; j++) { /* Do not consider thread inits */
			const EventLabel *lab = getEventLabel(Event(i, j));

			if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab)) {
				if (lib.hasMember(rLab->getFunctionName()))
					result.push_back(rLab->getPos());
			}
			if (auto *wLab = llvm::dyn_cast<LibWriteLabel>(lab)) {
				if (lib.hasMember(wLab->getFunctionName()))
					result.push_back(wLab->getPos());
			}
		}
	}
	return result;
}

void ExecutionGraph::getPoEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			for (auto &e : tos)
				if (froms[i].second[j].thread == e.thread &&
				    froms[i].second[j].index < e.index)
					buf.push_back(e);
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getRfm1EdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				      std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (std::find(tos.begin(), tos.end(), rLab->getRf()) !=
				    tos.end())
					buf.push_back(rLab->getRf());
			}
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getRfEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				const std::vector<Event> readers = wLab->getReadersList();
				std::copy_if(readers.begin(), readers.end(),
					     std::back_inserter(buf),
					     [&](const Event &e) {
						     return std::find(tos.begin(), tos.end(),
								      e) != tos.end();
					     });
			}
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getHbEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			for (auto &e : tos) {
				auto &before = getHbBefore(e);
				if (froms[i].second[j].index <
				    before[froms[i].second[j].thread])
					buf.push_back(e);
			}
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getWbEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (!llvm::isa<WriteLabel>(lab))
				continue;

			auto *wLab = static_cast<const WriteLabel *>(lab);
			View v(getHbBefore(tos[0]));
			for (auto &t : tos)
				v.update(getEventLabel(t)->getHbView());

			auto wb = calcWbRestricted(wLab->getAddr(), v);
			auto &ss = wb.getElems();
			// TODO: Make a map with already calculated WBs??

			if (std::find(ss.begin(), ss.end(), wLab->getPos()) == ss.end())
				continue;

			/* Collect all wb-after stores that are in "tos" range */
			auto k = wb.getIndex(wLab->getPos());
			for (auto l = 0u; l < ss.size(); l++)
				if (wb(k, l))
					buf.push_back(ss[l]);
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::getMoEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				    std::vector<Event> &tos)
{
	std::vector<Event> buf;

	for (auto i = 0u; i < froms.size(); i++) {
		buf = {};
		for (auto j = 0u; j < froms[i].second.size(); j++) {
			const EventLabel *lab = getEventLabel(froms[i].second[j]);
			if (!llvm::isa<WriteLabel>(lab))
				continue;

			/* Collect all mo-after events that are in the "tos" range */
			auto *wLab = static_cast<const WriteLabel *>(lab);
			auto moAfter = modOrder.getMoAfter(wLab->getAddr(), wLab->getPos());
			for (const auto &s : moAfter)
				if (std::find(tos.begin(), tos.end(), s) != tos.end())
					buf.push_back(s);
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::calcSingleStepPairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
					 const std::string &step, std::vector<Event> &tos)
{
	if (step == "po")
		return getPoEdgePairs(froms, tos);
	else if (step == "rf")
		return getRfEdgePairs(froms, tos);
	else if (step == "hb")
		return getHbEdgePairs(froms, tos);
	else if (step == "rf-1")
		return getRfm1EdgePairs(froms, tos);
	else if (step == "wb")
		return getWbEdgePairs(froms, tos);
	else if (step == "mo")
		return getMoEdgePairs(froms, tos);
	else
		BUG();
}

void addEdgePairsToMatrix(std::vector<std::pair<Event, std::vector<Event> > > &pairs,
			  Matrix2D<Event> &matrix)
{
	for (auto &p : pairs) {
		for (auto k = 0u; k < p.second.size(); k++) {
			matrix(p.first, p.second[k]) = true;
		}
	}
}

void ExecutionGraph::addStepEdgeToMatrix(std::vector<Event> &es,
					 Matrix2D<Event> &relMatrix,
					 const std::vector<std::string> &substeps)
{
	std::vector<std::pair<Event, std::vector<Event> > > edges;

	/* Initialize edges */
	for (auto &e : es)
		edges.push_back(std::make_pair(e, std::vector<Event>({e})));

	for (auto i = 0u; i < substeps.size(); i++)
		calcSingleStepPairs(edges, substeps[i], es);
	addEdgePairsToMatrix(edges, relMatrix);
}

llvm::StringMap<Matrix2D<Event> >
ExecutionGraph::calculateAllRelations(const Library &lib, std::vector<Event> &es)
{
	llvm::StringMap<Matrix2D<Event> > relMap;

	for (auto &r : lib.getRelations()) {
		Matrix2D<Event> relMatrix(es);
		auto &steps = r.getSteps();
		for (auto &s : steps)
			addStepEdgeToMatrix(es, relMatrix, s);

		if (r.isTransitive())
			relMatrix.transClosure();
		relMap[r.getName()] = relMatrix;
	}
	return relMap;
}

bool ExecutionGraph::isLibConsistentInView(const Library &lib, const View &v)
{
	auto es = getLibEventsInView(lib, v);
	auto relations = calculateAllRelations(lib, es);
	auto &constraints = lib.getConstraints();
	if (std::all_of(constraints.begin(), constraints.end(),
			[&](const Constraint &c)
			{ return !relations[c.getName()].isReflexive(); }))
		return true;
	return false;
}

std::vector<Event>
ExecutionGraph::getLibConsRfsInView(const Library &lib, Event read,
				    const std::vector<Event> &stores,
				    const View &v)
{
	EventLabel *lab = events[read.thread][read.index].get();
	BUG_ON(!llvm::isa<LibReadLabel>(lab));

	auto *rLab = static_cast<ReadLabel *>(lab);
	Event oldRf = rLab->getRf();
	std::vector<Event> filtered;

	for (auto &s : stores) {
		changeRf(rLab->getPos(), s);
		if (isLibConsistentInView(lib, v))
			filtered.push_back(s);
	}
	/* Restore the old reads-from, and eturn all the valid reads-from options */
	changeRf(rLab->getPos(), oldRf);
	return filtered;
}

void ExecutionGraph::addInvalidRfs(Event read, const std::vector<Event> &rfs)
{
	EventLabel *lab = events[read.thread][read.index].get();
	BUG_ON(!llvm::isa<LibReadLabel>(lab));

	auto *lLab = static_cast<LibReadLabel *>(lab);
	for (auto &s : rfs)
		lLab->addInvalidRf(s);
}

void ExecutionGraph::addBottomToInvalidRfs(Event read)
{
	EventLabel *lab = events[read.thread][read.index].get();

	if (auto *rLab = llvm::dyn_cast<LibReadLabel>(lab)) {
		rLab->addInvalidRf(Event::getInitializer());
		return;
	}
	BUG();
}


/************************************************************
 ** Debugging methods
 ***********************************************************/

static bool containsEvent(const std::vector<Event> &es, const Event e)
{
	return std::find(es.begin(), es.end(), e) != es.end();
}

void ExecutionGraph::validate(void)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			const EventLabel *lab = getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
				if (rLab->getRf().isInitializer())
					continue;

				auto *rfLab = static_cast<const WriteLabel *>(
					getEventLabel(rLab->getRf()));
				if (containsEvent(rfLab->getReadersList(), rLab->getPos()))
					continue;
				WARN("Read event is not the appropriate rf-1 list!\n");
				llvm::dbgs() << rLab->getPos() << "\n";
				llvm::dbgs() << *this << "\n";
				abort();
			}
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
				const std::vector<Event> &rs = wLab->getReadersList();
				if (llvm::isa<FaiWriteLabel>(wLab) ||
				    llvm::isa<CasWriteLabel>(wLab)) {
					if (1 >= std::count_if(rs.begin(), rs.end(),
							       [&](const Event &r) {
								       return isRMWLoad(r);
							       }))
						continue;
					WARN("RMW store is read from more than 1 load!\n");
					llvm::dbgs() << "RMW store: " << wLab->getPos()
						     << "\nReads:";
					for (auto &r : rs)
						llvm::dbgs() << r << " ";
					llvm::dbgs() << "\n";
					llvm::dbgs() << *this << "\n";
					abort();
				}

				if (std::any_of(rs.begin(), rs.end(), [&](Event r) {
				        if (auto *rLab = llvm::dyn_cast<ReadLabel>(
						    getEventLabel(r))) {
						return rLab->getRf() != wLab->getPos();
					} else {
						WARN("Non-read event found in rf-1 list!\n");
						abort();
					}
					})) {
						WARN("Write event is not marked in the read event!\n");
						llvm::dbgs() << wLab->getPos() << "\n";
						llvm::dbgs() << *this << "\n";
						abort();
				}
				for (auto &r : rs) {
					if (r.thread > (int) events.size() ||
					    r.index >= (int) events[r.thread].size()) {
						WARN("Event in write's rf-list does not exist!\n");
						llvm::dbgs() << r << "\n";
						llvm::dbgs() << *this << "\n";
						abort();
					}
				}

			}
		}
	}
	return;
}


/*******************************************************************************
 **                           Overloaded operators
 ******************************************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ExecutionGraph &g)
{
	for (auto i = 0u; i < g.events.size(); i++) {
		s << "Thread " << i << ":\n";
		for (auto j = 0u; j < g.events[i].size(); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			s << "\t" << *lab << "\n";
		}
	}
	s << "Thread sizes:\n\t";
	for (auto i = 0u; i < g.events.size(); i++)
		s << g.events[i].size() << " ";
	s << "\n";
	return s;
}
