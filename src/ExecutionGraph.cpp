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

ExecutionGraph::ExecutionGraph() : timestamp(0)
{
	/* Create an entry for main() and push the "initializer" label */
	events.push_back({ EventLabel(EStart, llvm::AtomicOrdering::Acquire,
				      Event(0, 0), Event::getInitializer()) });
}


/************************************************************
 ** Basic getter methods
 ***********************************************************/

unsigned int ExecutionGraph::nextStamp()
{
	return timestamp++;
}

EventLabel& ExecutionGraph::getEventLabel(const Event &e)
{
	return events[e.thread][e.index];
}

EventLabel& ExecutionGraph::getPreviousLabel(const Event &e)
{
	return events[e.thread][e.index-1];
}

EventLabel& ExecutionGraph::getLastThreadLabel(int thread)
{
	return events[thread][events[thread].size() - 1];
}

Event ExecutionGraph::getLastThreadEvent(int thread)
{
	return Event(thread, events[thread].size() - 1);
}

Event ExecutionGraph::getLastThreadRelease(int thread, llvm::GenericValue *addr)
{
	for (int i = events[thread].size() - 1; i > 0; i--) {
		EventLabel &lab = events[thread][i];
		if (lab.isFence() && lab.isAtLeastRelease())
			return lab.getPos();
		if (lab.isWrite() && lab.isAtLeastRelease() && lab.getAddr() == addr)
			return lab.getPos();
	}
	return events[thread][0].getPos();
}

/*
 * Given an write label sLab that is part of an RMW, return all
 * other RMWs that read from the same write. Of course, there must
 * be _at most_ one other RMW reading from the same write (see [Rex] set)
 */
std::vector<Event> ExecutionGraph::getPendingRMWs(EventLabel &sLab)
{
	std::vector<Event> pending;

	/* This function should be called with a write event */
	BUG_ON(!sLab.isWrite());

	/* If this is _not_ an RMW event, return an empty vector */
	if (!sLab.isRMW())
		return pending;

	/* Otherwise, scan for other RMWs that successfully read the same val */
	auto &pLab = getPreviousLabel(sLab.getPos());
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 1u; j < events[i].size(); j++) { /* Skip thread start */
			EventLabel &lab = events[i][j];
			if (isRMWLoad(lab.getPos()) && lab.rf == pLab.rf &&
			    lab.getAddr() == pLab.getAddr() && lab.getPos() != pLab.getPos())
				pending.push_back(lab.getPos());
		}
	}
	BUG_ON(pending.size() > 1);
	return pending;
}

Event ExecutionGraph::getPendingLibRead(EventLabel &lab)
{
	/* Should only be called with a read of a functional library that doesn't read BOT */
	BUG_ON(!lab.isRead() || lab.rf == Event::getInitializer());

	/* Get the conflicting label */
	auto &sLab = getEventLabel(lab.rf);
	auto it = std::find_if(sLab.rfm1.begin(), sLab.rfm1.end(), [&lab](Event &e){ return e != lab.getPos(); });
	BUG_ON(it == sLab.rfm1.end());
	return *it;
}

std::vector<Event> ExecutionGraph::getRevisitable(const EventLabel &sLab)
{
	std::vector<Event> loads;

	BUG_ON(!sLab.isWrite());
	auto before = getPorfBefore(sLab.getPos());
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = before[i] + 1u; j < events[i].size(); j++) {
			auto &lab = events[i][j];
			if (lab.isRead() && lab.getAddr() == sLab.getAddr() && lab.isRevisitable())
				loads.push_back(lab.getPos());
		}
	}
	return loads;
}

std::vector<Event> ExecutionGraph::getMoOptRfAfter(Event store)
{
	auto ls = modOrder.getMoAfter(getEventLabel(store).getAddr(), store);
	std::vector<Event> rfs;

	/*
	 * We push the RFs to a different vector in order
	 * not to invalidate the iterator
	 */
	for (auto it = ls.begin(); it != ls.end(); ++it) {
		auto &lab = getEventLabel(*it);
		if (lab.isWrite()) {
			for (auto &l : lab.rfm1)
				rfs.push_back(l);
		}
	}
	std::move(rfs.begin(), rfs.end(), std::back_inserter(ls));
	return ls;
}


/************************************************************
 ** Basic setter methods
 ***********************************************************/

void ExecutionGraph::calcLoadPoRfView(EventLabel &lab)
{
	lab.porfView = getPreviousLabel(lab.getPos()).getPorfView();
	++lab.porfView[lab.getThread()];

	lab.porfView.updateMax(getEventLabel(lab.rf).getPorfView());
}

void ExecutionGraph::calcLoadHbView(EventLabel &lab)
{
	lab.hbView = getPreviousLabel(lab.getPos()).getHbView();
	++lab.hbView[lab.getThread()];

	if (lab.isAtLeastAcquire())
		lab.hbView.updateMax(getEventLabel(lab.rf).getMsgView());
}

EventLabel& ExecutionGraph::addEventToGraph(EventLabel &lab)
{
	lab.stamp = nextStamp();
	events[lab.getThread()].push_back(lab);
	return getLastThreadLabel(lab.getThread());
}

EventLabel& ExecutionGraph::addReadToGraphCommon(EventLabel &lab, Event &rf)
{
	lab.makeRevisitable();
	calcLoadHbView(lab);
	calcLoadPoRfView(lab);

	auto &nLab = addEventToGraph(lab);

	if (rf.isInitializer())
		return nLab;

	EventLabel &rfLab = events[rf.thread][rf.index];
	rfLab.rfm1.push_back(lab.getPos());
	return nLab;
}

EventLabel& ExecutionGraph::addReadToGraph(int tid, EventAttr attr, llvm::AtomicOrdering ord,
					   llvm::GenericValue *ptr, llvm::Type *typ, Event rf,
					   llvm::GenericValue &&cmpVal, llvm::GenericValue &&rmwVal,
					   llvm::AtomicRMWInst::BinOp op)
{
	int max = events[tid].size();
	EventLabel lab(ERead, attr, ord, Event(tid, max), ptr, typ, rf, cmpVal, rmwVal, op);
	return addReadToGraphCommon(lab, rf);
}

EventLabel& ExecutionGraph::addLibReadToGraph(int tid, EventAttr attr, llvm::AtomicOrdering ord,
					      llvm::GenericValue *ptr, llvm::Type *typ, Event rf,
					      std::string functionName)
{
	int max = events[tid].size();
	EventLabel lab(ERead, attr, ord, Event(tid, max), ptr, typ, rf, functionName);
	return addReadToGraphCommon(lab, rf);
}

EventLabel& ExecutionGraph::addStoreToGraphCommon(EventLabel &lab)
{
	auto thread = lab.getThread();
	auto last = getLastThreadEvent(thread);

	lab.hbView = getHbBefore(last);
	++lab.hbView[thread];
	lab.porfView = getPorfBefore(last);
	++lab.porfView[thread];
	if (lab.isRMW()) {
		EventLabel &pLab = getEventLabel(last);
		View mV = getMsgView(pLab.rf);
		BUG_ON(pLab.ord == llvm::AtomicOrdering::NotAtomic);
		if (pLab.isAtLeastRelease())
			/* no need for ctor -- getMax copies the View */
			lab.msgView = mV.getMax(lab.hbView);
		else
			lab.msgView = getHbBefore(
				getLastThreadRelease(thread, lab.getAddr())).getMax(mV);
	} else {
		if (lab.isAtLeastRelease())
			lab.msgView = lab.hbView;
		else if (lab.ord == llvm::AtomicOrdering::Monotonic ||
			 lab.ord == llvm::AtomicOrdering::Acquire)
			lab.msgView = getHbBefore(getLastThreadRelease(thread, lab.getAddr()));
	}
	return addEventToGraph(lab);
}

EventLabel& ExecutionGraph::addStoreToGraph(int tid, EventAttr attr, llvm::AtomicOrdering ord,
					    llvm::GenericValue *ptr, llvm::Type *typ,
					    llvm::GenericValue &val, int offsetMO)
{
	int max = events[tid].size();
	EventLabel lab(EWrite, attr, ord, Event(tid, max), ptr, typ, val);
	modOrder[lab.getAddr()].insert(modOrder[lab.getAddr()].begin() + offsetMO, lab.getPos());
	return addStoreToGraphCommon(lab);
}

EventLabel& ExecutionGraph::addLibStoreToGraph(int tid, EventAttr attr, llvm::AtomicOrdering ord,
					       llvm::GenericValue *ptr, llvm::Type *typ,
					       llvm::GenericValue &val, int offsetMO,
					       std::string functionName, bool isInit)
{
	int max = events[tid].size();
	EventLabel lab(EWrite, ATTR_PLAIN, ord, Event(tid, max), ptr, typ, val, functionName, isInit);
	modOrder[lab.getAddr()].insert(modOrder[lab.getAddr()].begin() + offsetMO, lab.getPos());
	return addStoreToGraphCommon(lab);
}

EventLabel& ExecutionGraph::addFenceToGraph(int tid, llvm::AtomicOrdering ord)

{
	int max = events[tid].size();
	EventLabel lab(EFence, ord, Event(tid, max));
	Event last = getLastThreadEvent(tid);

	lab.hbView = getHbBefore(last);
	++lab.hbView[tid];
	lab.porfView = getPorfBefore(last);
	++lab.porfView[tid];
	if (lab.isAtLeastAcquire())
		calcRelRfPoBefore(tid, last.index, lab.hbView);
	return addEventToGraph(lab);
}

EventLabel& ExecutionGraph::addMallocToGraph(int tid, llvm::GenericValue *addr, llvm::GenericValue &val)
{
	int max = events[tid].size();
	EventLabel lab(EMalloc, Event(tid, max), addr, val);
	Event last = getLastThreadEvent(tid);

	lab.hbView = getHbBefore(last);
	++lab.hbView[tid];
	lab.porfView = getPorfBefore(last);
	++lab.porfView[tid];
	return addEventToGraph(lab);
}

EventLabel& ExecutionGraph::addFreeToGraph(int tid, llvm::GenericValue *addr, llvm::GenericValue &val)
{
	int max = events[tid].size();
	EventLabel lab(EFree, Event(tid, max), addr, val);
	Event last = getLastThreadEvent(tid);

	lab.hbView = getHbBefore(last);
	++lab.hbView[tid];
	lab.porfView = getPorfBefore(last);
	++lab.porfView[tid];
	return addEventToGraph(lab);
}

EventLabel& ExecutionGraph::addTCreateToGraph(int tid, int cid)
{
	int max = events[tid].size();
	EventLabel lab(ETCreate, llvm::AtomicOrdering::Release, Event(tid, max), cid);
	Event last = getLastThreadEvent(tid);

	lab.rfm1.push_back(Event(cid, 0));

	lab.porfView = getPorfBefore(last);
	++lab.porfView[tid];

	/* Thread creation has Release semantics */
	lab.hbView = getHbBefore(last);
	++lab.hbView[tid];
	lab.msgView = lab.hbView;
	return addEventToGraph(lab);
}

EventLabel& ExecutionGraph::addTJoinToGraph(int tid, int cid)
{
	int max = events[tid].size();
	EventLabel lab(ETJoin, llvm::AtomicOrdering::Acquire, Event(tid, max), cid);
	Event last = getLastThreadEvent(tid);

	lab.rf = Event::getInitializer();
	lab.porfView = getPorfBefore(last);
	lab.porfView[tid] = max;

	/* Thread joins have acquire semantics -- but we have to wait
	 * for the other thread to finish first, so we do not fully
	 * update the view yet */
	lab.hbView = getHbBefore(last);
	lab.hbView[tid] = max;
	return addEventToGraph(lab);
}

EventLabel& ExecutionGraph::addStartToGraph(int tid, Event tc)
{
	int max = events[tid].size();
	EventLabel lab = EventLabel(EStart, llvm::AtomicOrdering::Acquire, Event(tid, max), tc);

	lab.porfView = getPorfBefore(tc);
	lab.porfView[tid] = max;

	/* Thread start has Acquire semantics */
	lab.hbView = getHbBefore(tc);
	lab.hbView[tid] = max;

	return addEventToGraph(lab);
}

EventLabel& ExecutionGraph::addFinishToGraph(int tid)
{
	int max = events[tid].size();
	EventLabel lab(EFinish, llvm::AtomicOrdering::Release, Event(tid, max));
	Event last = getLastThreadEvent(tid);

	lab.porfView = getPorfBefore(last);
	lab.porfView[tid] = max;

	/* Thread termination has Release semantics */
	lab.hbView = getHbBefore(last);
	lab.hbView[tid] = max;
	lab.msgView = lab.hbView;
	return addEventToGraph(lab);
}

/************************************************************
 ** Calculation of [(po U rf)*] predecessors and successors
 ***********************************************************/

Event ExecutionGraph::getRMWChainUpperLimit(const EventLabel &sLab, const Event upper)
{
	/*
	 * This function should not be called with an event that is not
	 * the store part of an RMW
	 */
	WARN_ON_ONCE(!sLab.isWrite(), "getrmwchainupperlimit-arg",
		     "WARNING: getRMWChainUpperLimit() called with non-write argument!\n");

	/* As long as you find successful RMWs, keep going up the chain */
	auto curr = &sLab;
	auto limit = curr;
	while (curr->isWrite() && curr->pos != upper) {
		limit = curr;
		if (!curr->isRMW())
			break;

		auto &rLab = getPreviousLabel(curr->pos);

		/* Go down one (rmw-1;rf-1)-step */
		curr = &getEventLabel(rLab.rf);
	}

	/* We return the limit (curr may be equal to the upper search bound) */
	return limit->pos;
}

Event ExecutionGraph::getRMWChainLowerLimit(const EventLabel &sLab, const Event lower)
{
	/*
	 * This function should not be called with an event that is not
	 * the store part of an RMW
	 */
	WARN_ON_ONCE(!sLab.isWrite(), "getrmwchainlowerlimit-arg",
		     "WARNING: getRMWChainLowerLimit() called with non-write argument!\n");

	/* As long as you find successful RMWs, keep going down the chain */
	auto curr = &sLab;
	auto limit = curr;
	while (curr->pos != lower) {
		limit = curr;

		/* Check if other successful RMWs are reading from this write (at most one) */
		std::vector<Event> rmwRfs;
		std::copy_if(curr->rfm1.begin(), curr->rfm1.end(), std::back_inserter(rmwRfs),
			     [&](const Event &r){ return isRMWLoad(r); });
		BUG_ON(rmwRfs.size() > 1);

		/* If there is none, we reached the upper limit of the chain */
		if (rmwRfs.size() == 0)
			break;

		/* Otherwise, go up one (rf;rmw)-step */
		curr = &getEventLabel(rmwRfs.back().next());
	}

	/* We return the limit (curr may be equal to the lower search bound) */
	return limit->pos;
}

Event ExecutionGraph::getRMWChainLowerLimitInView(const EventLabel &sLab, const Event lower, View &v)
{
	/*
	 * This function should not be called with an event that is not
	 * the store part of an RMW
	 */
	WARN_ON_ONCE(!sLab.isWrite(), "getrmwchainlowerlimit-arg",
		     "WARNING: getRMWChainLowerLimit() called with non-write argument!\n");

	/* As long as you find successful RMWs, keep going down the chain */
	auto curr = &sLab;
	auto limit = curr;
	while (curr->pos != lower && curr->pos.index <= v[curr->pos.thread]) {
		limit = curr;

		/* Check if other successful RMWs are reading from this write (at most one) */
		std::vector<Event> rmwRfs;
		std::copy_if(curr->rfm1.begin(), curr->rfm1.end(), std::back_inserter(rmwRfs),
			     [&](const Event &r){ return isRMWLoad(r); });
		BUG_ON(rmwRfs.size() > 2);

		/* If there is none, we reached the upper limit of the chain */
		if (rmwRfs.size() == 0)
			break;

		Event next;
		if (rmwRfs.size() == 2) {
			if (rmwRfs[0].index <= v[rmwRfs[0].thread])
				next = rmwRfs[0].next();
			else
				next = rmwRfs[1].next();
		} else {
			next = rmwRfs.back().next();
		}

		/* Otherwise, go up one (rf;rmw)-step */
		curr = &getEventLabel(next);
	}

	/* We return the limit (curr may be equal to the lower search bound) */
	return limit->pos;
}


std::vector<Event> ExecutionGraph::getRMWChain(const EventLabel &sLab)
{

	WARN_ON_ONCE(!sLab.isWrite(), "getrmwchainupto-arg",
		     "WARNING: getRMWChain() called with non-write argument!\n");

	std::vector<Event> chain;

	if (!(sLab.isWrite() && sLab.isRMW()))
		return chain;

	/* As long as you find successful RMWs, keep going up the chain */
	auto curr = &sLab;
	while (curr->isWrite() && curr->isRMW()) {
		chain.push_back(curr->pos);

		/* Go down one (rmw-1;rf-1)-step */
		auto &rLab = getPreviousLabel(curr->pos);
		curr = &getEventLabel(rLab.rf);
	}

	/* We arrived at a non-RMW event so the chain is over */
	chain.push_back(curr->pos);
	return chain;

}

std::vector<Event> ExecutionGraph::getStoresHbAfterStores(llvm::GenericValue *loc,
							  const std::vector<Event> &chain)
{
	auto &stores = modOrder[loc];
	std::vector<Event> result;

	for (auto &s : stores) {
		if (std::find(chain.begin(), chain.end(), s) != chain.end())
			continue;
		auto before = getHbBefore(s);
		if (std::any_of(chain.begin(), chain.end(), [&before](Event e)
				{ return e.index < before[e.thread]; }))
			result.push_back(s);
	}
	return result;
}

View ExecutionGraph::getMsgView(Event e)
{
	return getEventLabel(e).getMsgView();
}

View ExecutionGraph::getPorfBefore(Event e)
{
	return getEventLabel(e).getPorfView();
}
View ExecutionGraph::getHbBefore(Event e)
{
	return getEventLabel(e).getHbView();
}

View ExecutionGraph::getHbBefore(const std::vector<Event> &es)
{
	View v;

	for (auto &e : es) {
		auto o = getHbBefore(e);
		v.updateMax(o);
	}
	return v;
}

View ExecutionGraph::getHbPoBefore(Event e)
{
	return getHbBefore(e.prev());
}

void ExecutionGraph::calcHbRfBefore(Event &e, llvm::GenericValue *addr,
				    View &a)
{
	int ai = a[e.thread];
	if (e.index <= ai)
		return;

	a[e.thread] = e.index;
	for (int i = ai + 1; i <= e.index; i++) {
		EventLabel &lab = events[e.thread][i];
		if (lab.isRead() && !lab.rf.isInitializer() &&
		    (lab.getAddr() == addr || lab.rf.index <= lab.hbView[lab.rf.thread]))
			calcHbRfBefore(lab.rf, addr, a);
	}
	return;
}

View ExecutionGraph::getHbRfBefore(std::vector<Event> &es)
{
	View a;

	for (auto &e : es)
		calcHbRfBefore(e, getEventLabel(e).getAddr(), a);
	return a;
}

void ExecutionGraph::calcRelRfPoBefore(int thread, int index, View &v)
{
	for (auto i = index; i > 0; i--) {
		EventLabel &lab = events[thread][i];
		if (lab.isFence() && lab.isAtLeastAcquire())
			return;
		if (lab.isRead() && (lab.ord == llvm::AtomicOrdering::Monotonic ||
				     lab.ord == llvm::AtomicOrdering::Release)) {
			View o = getMsgView(lab.rf);
			v.updateMax(o);
		}
	}
}


/************************************************************
 ** Calculation of particular sets of events/event labels
 ***********************************************************/

std::vector<EventLabel>
ExecutionGraph::getPrefixLabelsNotBefore(View &prefix, View &before)
{
	std::vector<EventLabel> result;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = before[i] + 1; j <= prefix[i]; j++) {
			EventLabel &lab = events[i][j];
			result.push_back(lab);
			EventLabel &curLab = result.back();
			if (!curLab.hasWriteSem())
				continue;
			curLab.rfm1.erase(std::remove_if(curLab.rfm1.begin(), curLab.rfm1.end(),
							 [&](Event &e)
							 { return e.index > prefix[e.thread] &&
								  e.index > before[e.thread]; }),
					  curLab.rfm1.end());
		}
	}
	return result;
}

std::vector<Event>
ExecutionGraph::getRfsNotBefore(const std::vector<EventLabel> &labs,
				View &before)
{
	std::vector<Event> rfs;

	std::for_each(labs.begin(), labs.end(), [&rfs, &before](const EventLabel &lab)
		      { if (lab.isRead() && lab.getIndex() > before[lab.getThread()])
				      rfs.push_back(lab.rf); });
	return rfs;
}

std::vector<std::pair<Event, Event> >
ExecutionGraph::getMOPredsInBefore(const std::vector<EventLabel> &labs,
				   View &before)
{
	std::vector<std::pair<Event, Event> > pairs;

	for (auto &lab : labs) {
		/* Only store MO pairs for labels that are not in before */
		if (!lab.isWrite() || lab.getIndex() <= before[lab.getThread()])
			continue;

		auto &locMO = modOrder[lab.getAddr()];
		auto moPos = std::find(locMO.begin(), locMO.end(), lab.getPos());

		/* This store must definitely be in this location's MO */
		BUG_ON(moPos == locMO.end());

		/* We need to find the previous MO store that is in before or
		 * in the vector for which we are getting the predecessors */
		std::reverse_iterator<std::vector<Event>::iterator> predPos(moPos);
		auto predFound = false;
		for (auto rit = predPos; rit != locMO.rend(); ++rit) {
			if (rit->index <= before[rit->thread] ||
			    std::find_if(labs.begin(), labs.end(),
					 [rit](const EventLabel &lab)
					 { return lab.getPos() == *rit; })
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

bool ExecutionGraph::isWriteRfBefore(View &before, Event e)
{
	if (e.index <= before[e.thread])
		return true;

	auto &lab = getEventLabel(e);
	BUG_ON(!lab.isWrite());
	for (auto &e : lab.rfm1)
		if (e.index <= before[e.thread])
			return true;
	return false;
}

bool ExecutionGraph::isStoreReadByExclusiveRead(Event &store, llvm::GenericValue *ptr)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			auto &lab = events[i][j];
			if (isRMWLoad(lab.getPos()) && lab.rf == store && lab.getAddr() == ptr)
				return true;
		}
	}
	return false;
}

bool ExecutionGraph::isStoreReadBySettledRMW(Event &store, llvm::GenericValue *ptr, View &porfBefore)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			auto &lab = events[i][j];
			if (isRMWLoad(lab.getPos()) && lab.rf == store && lab.getAddr() == ptr) {
				if (!lab.isRevisitable())
					return true;
				if (lab.getIndex() <= porfBefore[lab.getThread()])
					return true;
			}
		}
	}
	return false;
}

std::vector<Event> ExecutionGraph::findOverwrittenBoundary(llvm::GenericValue *addr, int thread)
{
	std::vector<Event> boundary;
	auto before = getHbBefore(getLastThreadEvent(thread));

	if (before.empty())
		return boundary;

	for (auto &e : modOrder[addr])
		if (isWriteRfBefore(before, e))
			boundary.push_back(e.prev());
	return boundary;
}

/* View before _can_ be implicitly modified */
std::pair<int, int>
ExecutionGraph::splitLocMOBefore(llvm::GenericValue *addr, View &before)
{
	auto &locMO = modOrder[addr];
	for (auto rit = locMO.rbegin(); rit != locMO.rend(); ++rit) {
		if (before.empty() || !isWriteRfBefore(before, *rit))
			continue;
		return std::make_pair(std::distance(rit, locMO.rend()), locMO.size());
	}
	return std::make_pair(0, locMO.size());
}


/************************************************************
 ** Graph modification methods
 ***********************************************************/

void ExecutionGraph::changeRf(EventLabel &lab, Event store)
{
	Event oldRf = lab.rf;
	lab.rf = store;

	/* Make sure that the old write it was reading from still exists */
	if (oldRf.index < (int) events[oldRf.thread].size() && !oldRf.isInitializer()) {
		EventLabel &oldLab = getEventLabel(oldRf);
		oldLab.rfm1.erase(std::remove(oldLab.rfm1.begin(), oldLab.rfm1.end(), lab.getPos()),
				  oldLab.rfm1.end());
	}
	if (!store.isInitializer()) {
		EventLabel &sLab = getEventLabel(store);
		sLab.rfm1.push_back(lab.getPos());
	}
	/* Update the views of the load */
	calcLoadHbView(lab);
	calcLoadPoRfView(lab);
}


void ExecutionGraph::cutToView(View &preds)
{
	/* Restrict the graph according to the view */
	for (auto i = 0u; i < events.size(); i++) {
		auto &thr = events[i];
		thr.erase(thr.begin() + preds[i] + 1, thr.end());
	}

	/* Remove any 'pointers' to events that have been removed */
	for (auto i = 0u; i < events.size(); i++) {
		auto &thr = events[i];
		for (auto j = 0u; j < thr.size(); j++) {
			auto &lab = thr[j];
			/*
			 * If it is a join and the respective Finish has been
			 * removed, renew the Views of this label and continue
			 */
			if (lab.isJoin() &&
			    lab.rf.index >= (int) events[lab.rf.thread].size()) {
				auto init = Event::getInitializer();
				lab.rf = init;
				calcLoadHbView(lab);
				calcLoadPoRfView(lab);
				continue;
			}

			/* If it hasn't write semantics, nothing to do */
			if (!lab.hasWriteSem())
				continue;

			lab.rfm1.erase(std::remove_if(lab.rfm1.begin(), lab.rfm1.end(),
						      [&](Event &e)
						      { return e.index > preds[e.thread]; }),
				       lab.rfm1.end());
		}
	}

	/* Remove cutted events from the modification order as well */
	for (auto it = modOrder.begin(); it != modOrder.end(); ++it)
		it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
						[&preds](Event &e)
						{ return e.index > preds[e.thread]; }),
				 it->second.end());
	return;
}

View ExecutionGraph::getViewFromStamp(unsigned int stamp)
{
	View preds;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = (int) events[i].size() - 1; j >= 0; j--) {
			auto &lab = events[i][j];
			if (lab.getStamp() <= stamp) {
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

void ExecutionGraph::restoreStorePrefix(EventLabel &rLab, std::vector<EventLabel> &storePrefix,
					std::vector<std::pair<Event, Event> > &moPlacings)
{
	for (auto &lab : storePrefix) {
		BUG_ON(lab.getIndex() > (int) events[lab.getThread()].size() &&
		       "Events should be added in order!");
		if (lab.getIndex() == (int) events[lab.getThread()].size())
			events[lab.getThread()].push_back(lab);
	}

	for (auto &lab : storePrefix) {
		EventLabel &curLab = events[lab.getThread()][lab.getIndex()];
		curLab.makeNotRevisitable();
		if (curLab.isRead() && !curLab.rf.isInitializer()) {
			EventLabel &rfLab = getEventLabel(curLab.rf);
			if (std::find(rfLab.rfm1.begin(), rfLab.rfm1.end(), curLab.getPos())
			    == rfLab.rfm1.end())
				rfLab.rfm1.push_back(curLab.getPos());
		}
		curLab.rfm1.erase(std::remove(curLab.rfm1.begin(), curLab.rfm1.end(), rLab.getPos()),
				  curLab.rfm1.end());
	}

	/* If there are no specific mo placings, just insert all stores */
	if (moPlacings.empty()) {
		std::for_each(storePrefix.begin(), storePrefix.end(), [this](EventLabel &lab)
			      { if (lab.isWrite()) this->modOrder.addAtLocEnd(lab.getAddr(), lab.getPos()); });
		return;
	}

	/* Otherwise, insert the writes of storePrefix into the appropriate places */
	auto inserted = 0u;
	while (inserted < moPlacings.size()) {
		for (auto it = moPlacings.begin(); it != moPlacings.end(); ++it) {
			auto &lab = getEventLabel(it->first);
			if (modOrder.locContains(lab.getAddr(), it->second) &&
			    !modOrder.locContains(lab.getAddr(), it->first)) {
				modOrder.addAtLocAfter(lab.getAddr(), it->second, it->first);
				++inserted;
			}
		}
	}
}


/************************************************************
 ** Equivalence checks
 ***********************************************************/

bool ExecutionGraph::equivPrefixes(unsigned int stamp,
				   const std::vector<EventLabel> &oldPrefix,
				   const std::vector<EventLabel> &newPrefix)
{
	for (auto ritN = newPrefix.rbegin(); ritN != newPrefix.rend(); ++ritN) {
		if (std::all_of(oldPrefix.rbegin(), oldPrefix.rend(), [&](const EventLabel &sLab)
				{ return sLab != *ritN; }))
			return false;
	}

	for (auto ritO = oldPrefix.rbegin(); ritO != oldPrefix.rend(); ++ritO) {
		if (std::find(newPrefix.rbegin(), newPrefix.rend(), *ritO) != newPrefix.rend())
			continue;

		if (ritO->getStamp() <= stamp &&
		    ritO->getIndex() < (int) events[ritO->getThread()].size() &&
		    *ritO == events[ritO->getThread()][ritO->getIndex()])
			continue;
		return false;
	}
	return true;
}

bool ExecutionGraph::equivPlacings(unsigned int stamp,
				   const std::vector<std::pair<Event, Event> > &oldPlacings,
				   const std::vector<std::pair<Event, Event> > &newPlacings)
{
	for (auto ritN = newPlacings.rbegin(); ritN != newPlacings.rend(); ++ritN) {
		if (std::all_of(oldPlacings.rbegin(), oldPlacings.rend(),
				[&](const std::pair<Event, Event> &s)
				{ return s != *ritN; }))
			return false;
	}

	for (auto ritO = oldPlacings.rbegin(); ritO != oldPlacings.rend(); ++ritO) {
		if (std::find(newPlacings.rbegin(), newPlacings.rend(), *ritO) != newPlacings.rend())
			continue;

		auto &sLab1 = getEventLabel(ritO->first);
		auto &sLab2 = getEventLabel(ritO->second);
		if (sLab2.getStamp() <= stamp &&
		    modOrder.areOrdered(sLab1.getAddr(), sLab1.getPos(), sLab2.getPos()))
			continue;
		return false;
	}
	return true;
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

Event ExecutionGraph::findRaceForNewLoad(Event e)
{
	auto &lab = getEventLabel(e);
	auto before = getHbBefore(lab.getPos().prev());
	auto &stores = modOrder[lab.getAddr()];

	/* If there are not any events hb-before the read, there is nothing to do */
	if (before.empty())
		return Event::getInitializer();

	/* Check for events that race with the current load */
	for (auto &s : stores) {
		if (s.index > before[s.thread]) {
			EventLabel &sLab = getEventLabel(s);
			if ((lab.isNotAtomic() || sLab.isNotAtomic()) &&
			    lab.getPos() != sLab.getPos())
				return s; /* Race detected! */
		}
	}
	return Event::getInitializer(); /* Race not found */
}

Event ExecutionGraph::findRaceForNewStore(Event e)
{
	auto &lab = getEventLabel(e);
	auto before = getHbBefore(lab.getPos().prev());

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = before[i] + 1u; j < events[i].size(); j++) {
			EventLabel &oLab = events[i][j];
			if ((oLab.isRead() || oLab.isWrite()) &&
			    oLab.getAddr() == lab.getAddr() &&
			    oLab.getPos() != lab.getPos())
				if (lab.isNotAtomic() || oLab.isNotAtomic())
					return oLab.getPos(); /* Race detected! */
		}
	}
	return Event::getInitializer(); /* Race not found */
}


/************************************************************
 ** Calculation utilities
 ***********************************************************/

template <class T>
int binSearch(const std::vector<T> &arr, int len, T what)
{
	int low = 0;
	int high = len - 1;
	while (low <= high) {
		int mid = (low + high) / 2;
		if (arr[mid] > what)
			high = mid - 1;
		else if (arr[mid] < what)
			low = mid + 1;
		else
			return mid;
	}
	return -1; /* not found */
}

void calcTransClosure(std::vector<bool> &matrix, int len)
{
	for (auto i = 1; i < len; i++)
		for (auto k = 0; k < i; k++)
			if (matrix[i * len + k])
				for (auto j = 0; j < len; j++)
					matrix[i * len + j] = matrix[i * len + j] | matrix[k * len + j];
	for (auto i = 0; i < len - 1; i++)
		for (auto k = i + 1; k < len; k++)
			if (matrix[i * len + k])
				for (auto j = 0; j < len; j++)
					matrix[i * len + j] = matrix[i * len + j] | matrix[k * len + j];
}

bool isIrreflexive(std::vector<bool> &matrix, int len)
{
	for (auto i = 0; i < len; i++)
		if (matrix[i * len + i])
			return false;
	return true;
}

/*
 * Get in-degrees for event es, according to adjacency matrix
 */
std::vector<int> getInDegrees(const std::vector<bool> &matrix, const std::vector<Event> &es)
{
	std::vector<int> inDegree(es.size(), 0);

	for (auto i = 0u; i < matrix.size(); i++)
		inDegree[i % es.size()] += (int) matrix[i];
	return inDegree;
}

std::vector<Event> topoSort(const std::vector<bool> &matrix, const std::vector<Event> &es)
{
	std::vector<Event> sorted;
	std::vector<int> stack;

	/* Get in-degrees for es, according to matrix */
	auto inDegree = getInDegrees(matrix, es);

	/* Propagate events with no incoming edges to stack */
	for (auto i = 0u; i < inDegree.size(); i++)
		if (inDegree[i] == 0)
			stack.push_back(i);

	/* Perform topological sorting, filling up sorted */
	while (stack.size() > 0) {
		/* Pop next node-ID, and push node into sorted */
		auto nextI = stack.back();
		sorted.push_back(es[nextI]);
		stack.pop_back();

		for (auto i = 0u; i < es.size(); i++) {
			/* Finds all nodes with incoming edges from nextI */
			if (!matrix[nextI * es.size() + i])
				continue;
			if (--inDegree[i] == 0)
				stack.push_back(i);
		}
	}

	/* Make sure that there is no cycle */
	BUG_ON(std::any_of(inDegree.begin(), inDegree.end(), [](int degI){ return degI > 0; }));
	return sorted;
}

void allTopoSortUtil(std::vector<std::vector<Event> > &sortings, std::vector<Event> &current,
		     std::vector<bool> visited, std::vector<int> &inDegree,
		     const std::vector<bool> &matrix, const std::vector<Event> &es)
{
	/*
	 * The boolean variable 'scheduled' indicates whether this recursive call
	 * has added (scheduled) one event (at least) to the current topological sorting.
	 * If no event was added, a full topological sort has been produced.
	 */
	auto scheduled = false;

	for (auto i = 0u; i < es.size(); i++) {
		/* If ith-event can be added */
		if (inDegree[i] == 0 && !visited[i]) {
			/* Reduce in-degrees of its neighbors */
			for (auto j = 0u; j < es.size(); j++)
				if (matrix[i * es.size() + j])
					--inDegree[j];
			/* Add event in current sorting, mark as visited, and recurse */
			current.push_back(es[i]);
			visited[i] = true;

			allTopoSortUtil(sortings, current, visited, inDegree, matrix, es);

			/* Reset visited, current sorting, and inDegree */
			visited[i] = false;
			current.pop_back();
			for (auto j = 0u; j < es.size(); j++)
				if (matrix[i * es.size() + j])
					++inDegree[j];
			/* Mark that at least one event has been added to the current sorting */
			scheduled = true;
		}
	}

	/*
	 * We reach this point if no events were added in the current sorting, meaning
	 * that this is a complete sorting
	 */
	if (!scheduled)
		sortings.push_back(current);
	return;
}

std::vector<std::vector<Event> > allTopoSort(const std::vector<bool> &matrix, const std::vector<Event> &es)
{
	std::vector<bool> visited(es.size(), false);
	std::vector<std::vector<Event> > sortings;
	std::vector<Event> current;

	auto inDegree = getInDegrees(matrix, es);
	allTopoSortUtil(sortings, current, visited, inDegree, matrix, es);
	return sortings;
}

void addEdgesFromTo(const std::vector<int> &from, const std::vector<int> &to,
		    int len, std::vector<bool> &matrix)
{
	for (auto &i : from)
		for (auto &j: to)
			matrix[i * len + j] = true;
	return;
}


/************************************************************
 ** PSC calculation
 ***********************************************************/

bool ExecutionGraph::isRMWLoad(const Event &e)
{
	EventLabel &lab = getEventLabel(e);
	if (lab.isWrite() || !lab.isRMW())
		return false;
	if (e.index == (int) events[e.thread].size() - 1)
		return false;

	EventLabel &labNext = events[e.thread][e.index + 1];
	if (labNext.isRMW() && labNext.isWrite() && lab.getAddr() == labNext.getAddr())
		return true;
	return false;
}

std::pair<std::vector<Event>, std::vector<Event> >
ExecutionGraph::getSCs()
{
	std::vector<Event> scs, fcs;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			EventLabel &lab = events[i][j];
			if (lab.isSC() && !isRMWLoad(lab.getPos()))
				scs.push_back(lab.getPos());
			if (lab.isFence() && lab.isSC())
				fcs.push_back(lab.getPos());
		}
	}
	return std::make_pair(scs,fcs);
}

std::vector<llvm::GenericValue *> ExecutionGraph::getDoubleLocs()
{
	std::vector<llvm::GenericValue *> singles, doubles;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 1u; j < events[i].size(); j++) { /* Do not consider thread inits */
			EventLabel &lab = events[i][j];
			if (!(lab.isRead() || lab.isWrite()))
				continue;
			if (std::find(doubles.begin(), doubles.end(), lab.getAddr()) != doubles.end())
				continue;
			if (std::find(singles.begin(), singles.end(), lab.getAddr()) != singles.end()) {
				singles.erase(std::remove(singles.begin(), singles.end(), lab.getAddr()),
					      singles.end());
				doubles.push_back(lab.getAddr());
			} else {
				singles.push_back(lab.getAddr());
			}
		}
	}
	return doubles;
}

int calcEventIndex(const std::vector<Event> &scs, Event e)
{
	int idx = binSearch(scs, scs.size(), e);
	BUG_ON(idx == -1);
	return idx;
}

std::vector<int> ExecutionGraph::calcSCFencesSuccs(std::vector<Event> &scs,
						   std::vector<Event> &fcs, const Event &e)
{
	std::vector<int> succs;

	if (isRMWLoad(e))
		return succs;
	for (auto &f : fcs) {
		auto before = getHbBefore(f);
		if (e.index <= before[e.thread])
			succs.push_back(calcEventIndex(scs, f));
	}
	return succs;
}

std::vector<int> ExecutionGraph::calcSCFencesPreds(std::vector<Event> &scs,
						   std::vector<Event> &fcs, const Event &e)
{
	std::vector<int> preds;
	auto before = getHbBefore(e);

	if (isRMWLoad(e))
		return preds;
	for (auto &f : fcs) {
		if (f.index <= before[f.thread])
			preds.push_back(calcEventIndex(scs, f));
	}
	return preds;
}

std::vector<int> ExecutionGraph::calcSCSuccs(std::vector<Event> &scs,
					     std::vector<Event> &fcs, const Event &e)
{
	EventLabel &lab = getEventLabel(e);

	if (isRMWLoad(e))
		return {};
	if (lab.isSC())
		return {calcEventIndex(scs, e)};
	else
		return calcSCFencesSuccs(scs, fcs, e);
}

std::vector<int> ExecutionGraph::calcSCPreds(std::vector<Event> &scs,
					     std::vector<Event> &fcs, const Event &e)
{
	EventLabel &lab = getEventLabel(e);

	if (isRMWLoad(e))
		return {};
	if (lab.isSC())
		return {calcEventIndex(scs, e)};
	else
		return calcSCFencesPreds(scs, fcs, e);
}


std::vector<int> ExecutionGraph::getSCRfSuccs(std::vector<Event> &scs, std::vector<Event> &fcs,
					      EventLabel &lab)
{
	std::vector<int> rfs;

	BUG_ON(!lab.isWrite());
	for (auto &e : lab.rfm1) {
		auto succs = calcSCSuccs(scs, fcs, e);
		rfs.insert(rfs.end(), succs.begin(), succs.end());
	}
	return rfs;
}

std::vector<int> ExecutionGraph::getSCFenceRfSuccs(std::vector<Event> &scs, std::vector<Event> &fcs,
						   EventLabel &lab)
{
	std::vector<int> fenceRfs;

	BUG_ON(!lab.isWrite());
	for (auto &e : lab.rfm1) {
		auto fenceSuccs = calcSCFencesSuccs(scs, fcs, e);
		fenceRfs.insert(fenceRfs.end(), fenceSuccs.begin(), fenceSuccs.end());
	}
	return fenceRfs;
}

void ExecutionGraph::addRbEdges(std::vector<Event> &scs, std::vector<Event> &fcs,
				std::vector<int> &moAfter, std::vector<int> &moRfAfter,
				std::vector<bool> &matrix, EventLabel &lab)
{
	BUG_ON(!lab.isWrite());
	for (auto &e : lab.rfm1) {
		auto preds = calcSCPreds(scs, fcs, e);
		auto fencePreds = calcSCFencesPreds(scs, fcs, e);

		addEdgesFromTo(preds, moAfter, scs.size(), matrix);        /* PSC_base: Adds rb-edges */
		addEdgesFromTo(fencePreds, moRfAfter, scs.size(), matrix); /* PSC_fence: Adds (rb;rf)-edges */
	}
	return;
}

void ExecutionGraph::addMoRfEdges(std::vector<Event> &scs, std::vector<Event> &fcs,
				  std::vector<int> &moAfter, std::vector<int> &moRfAfter,
				  std::vector<bool> &matrix, EventLabel &lab)
{
	auto preds = calcSCPreds(scs, fcs, lab.getPos());
	auto fencePreds = calcSCFencesPreds(scs, fcs, lab.getPos());
	auto rfs = getSCRfSuccs(scs, fcs, lab);

	addEdgesFromTo(preds, moAfter, scs.size(), matrix);        /* PSC_base:  Adds mo-edges */
	addEdgesFromTo(preds, rfs, scs.size(), matrix);            /* PSC_base:  Adds rf-edges */
	addEdgesFromTo(fencePreds, moRfAfter, scs.size(), matrix); /* PSC_fence: Adds (mo;rf)-edges */
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
void ExecutionGraph::addSCEcos(std::vector<Event> &scs, std::vector<Event> &fcs,
			       std::vector<Event> &mo, std::vector<bool> &matrix)
{
	std::vector<int> moAfter;   /* Ids of mo-after SC writes or writes that reach an SC fence */
	std::vector<int> moRfAfter; /* Ids of SC fences that can be reached by (mo;rf)-after reads */

	for (auto rit = mo.rbegin(); rit != mo.rend(); rit++) {
		EventLabel &lab = getEventLabel(*rit);

		addRbEdges(scs, fcs, moAfter, moRfAfter, matrix, lab);
		addMoRfEdges(scs, fcs, moAfter, moRfAfter, matrix, lab);

		auto succs = calcSCSuccs(scs, fcs, lab.getPos());
		auto fenceRfs = getSCFenceRfSuccs(scs, fcs, lab);
		moAfter.insert(moAfter.end(), succs.begin(), succs.end());
		moRfAfter.insert(moRfAfter.end(), fenceRfs.begin(), fenceRfs.end());
	}
}

/*
 * addSCWbEcos is a helper function that calculates a part of PSC_base and PSC_fence,
 * like addSCEcos. The difference between them lies in the fact that addSCEcos
 * uses MO for adding mo and rb edges, addSCWBEcos uses WB for that.
 */
void ExecutionGraph::addSCWbEcos(std::vector<Event> &scs, std::vector<Event> &fcs,
				 std::vector<Event> &stores, std::vector<bool> &wbMatrix,
				 std::vector<bool> &pscMatrix)
{
	for (auto &w : stores) {
		EventLabel &wLab = getEventLabel(w);
		auto wIdx = calcEventIndex(stores, wLab.getPos());

		/*
		 * Calculate which of the stores are wb-after the current
		 * write, and then collect wb-after and (wb;rf)-after SC successors
		 */
		std::vector<int> wbAfter, wbRfAfter;
		for (auto &s : stores) {
			EventLabel &sLab = getEventLabel(s);
			auto sIdx = calcEventIndex(stores, sLab.getPos());
			if (wbMatrix[wIdx * stores.size() + sIdx]) {
				auto succs = calcSCSuccs(scs, fcs, sLab.getPos());
				auto fenceRfs = getSCFenceRfSuccs(scs, fcs, sLab);
				wbAfter.insert(wbAfter.end(), succs.begin(), succs.end());
				wbRfAfter.insert(wbRfAfter.end(), fenceRfs.begin(), fenceRfs.end());
			}
		}

		/* Then, add the proper edges to PSC using wb-after and (wb;rf)-after successors */
		addRbEdges(scs, fcs, wbAfter, wbRfAfter, pscMatrix, wLab);
		addMoRfEdges(scs, fcs, wbAfter, wbRfAfter, pscMatrix, wLab);
	}
}

void ExecutionGraph::addSbHbEdges(std::vector<Event> &scs, std::vector<bool> &matrix)
{
	for (auto i = 0u; i < scs.size(); i++) {
		for (auto j = 0u; j < scs.size(); j++) {
			if (i == j)
				continue;
			EventLabel &ei = getEventLabel(scs[i]);
			EventLabel &ej = getEventLabel(scs[j]);

			/* PSC_base: Adds sb-edges*/
			if (ei.getThread() == ej.getThread()) {
				if (ei.getIndex() < ej.getIndex())
					matrix[i * scs.size() + j] = true;
				continue;
			}

			/* PSC_base: Adds sb_(<>loc);hb;sb_(<>loc) edges
			 * HACK: Also works for PSC_fence: [Fsc];hb;[Fsc] edges
			 * since fences have a null address, different from the
			 * address of all global variables */
			Event prev = ej.getPos().prev();
			EventLabel &ejPrev = getEventLabel(prev);
			if (!ejPrev.hbView.empty() && ej.getAddr() != ejPrev.getAddr() &&
			    ei.getIndex() < ejPrev.hbView[ei.getThread()]) {
				Event next = ei.getPos().next();
				EventLabel &eiNext = getEventLabel(next);
				if (ei.getAddr() != eiNext.getAddr())
					matrix[i * scs.size() + j] = true;
			}
		}
	}
	return;
}

void ExecutionGraph::addInitEdges(std::vector<Event> &scs, std::vector<Event> &fcs,
				  std::vector<bool> &matrix)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			EventLabel &lab = events[i][j];
			/* Consider only reads that read from the initializer write */
			if (!lab.isRead() || !lab.rf.isInitializer() || isRMWLoad(lab.getPos()))
				continue;
			std::vector<int> preds = calcSCPreds(scs, fcs, lab.pos);
			std::vector<int> fencePreds = calcSCFencesPreds(scs, fcs, lab.getPos());
			for (auto &w : modOrder[lab.getAddr()]) {
				std::vector<int> wSuccs = calcSCSuccs(scs, fcs, w);
				addEdgesFromTo(preds, wSuccs, scs.size(), matrix); /* Adds rb-edges */
				for (auto &r : getEventLabel(w).rfm1) {
					std::vector<int> fenceSuccs = calcSCFencesSuccs(scs, fcs, r);
					addEdgesFromTo(fencePreds, fenceSuccs, scs.size(), matrix); /*Adds (rb;rf)-edges */
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

	/* Sort SC accesses for easier look-up, and create PSC matrix */
	std::vector<bool> matrix(scs.size() * scs.size(), false);
	std::sort(scs.begin(), scs.end());

	/* Add edges from the initializer write (special case) */
	addInitEdges(scs, fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(scs, matrix);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence for only
	 * _one_ possible extension of WB for each location
	 */
	std::vector<llvm::GenericValue *> scLocs = getDoubleLocs();
	for (auto loc : scLocs) {
		auto wb = calcWb(loc);
		auto &stores = wb.first;
		auto &wbMatrix = wb.second;
		auto sortedStores = topoSort(wbMatrix, stores);
		addSCEcos(scs, fcs, sortedStores, matrix);
	}

	/* Calculate the transitive closure of the bool matrix */
	calcTransClosure(matrix, scs.size());
	return isIrreflexive(matrix, scs.size());
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

	/* Sort SC accesses for easier look-up, and create PSC matrix */
	std::vector<bool> matrix(scs.size() * scs.size(), false);
	std::sort(scs.begin(), scs.end());

	/* Add edges from the initializer write (special case) */
	addInitEdges(scs, fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(scs, matrix);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence using WB
	 * instead of MO
	 */
	std::vector<llvm::GenericValue *> scLocs = getDoubleLocs();
	for (auto loc : scLocs) {
		auto wb = calcWb(loc);
		auto &stores = wb.first;
		auto &wbMatrix = wb.second;
		addSCWbEcos(scs, fcs, stores, wbMatrix, matrix);
	}

	/* Calculate the transitive closure of the bool matrix */
	calcTransClosure(matrix, scs.size());
	return isIrreflexive(matrix, scs.size());
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

	/* Sort SC accesses for easier look-up, and create PSC matrix */
	std::vector<bool> matrix(scs.size() * scs.size(), false);
	std::sort(scs.begin(), scs.end());

	/* Add edges from the initializer write (special case) */
	addInitEdges(scs, fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(scs, matrix);

	/*
	 * Collect memory locations that have more than one SC
	 * memory access, and then calculate the possible extensions
	 * of writes-before (WB) for these memory locations
	 */
	std::vector<llvm::GenericValue *> scLocs = getDoubleLocs();
	std::vector<std::vector<std::vector<Event> > > topoSorts(scLocs.size());
	for (auto i = 0u; i < scLocs.size(); i++) {
		auto wb = calcWb(scLocs[i]);
		auto &stores = wb.first;
		auto &wbMatrix = wb.second;
		topoSorts[i] = allTopoSort(wbMatrix, stores);
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
			addSCEcos(scs, fcs, topoSorts[i][count[i]], tentativePSC);
		calcTransClosure(tentativePSC, scs.size());
		if (isIrreflexive(tentativePSC, scs.size()))
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

	/* Sort SC accesses for easier look-up, and create PSC matrix */
	std::vector<bool> matrix(scs.size() * scs.size(), false);
	std::sort(scs.begin(), scs.end());

	/* Add edges from the initializer write (special case) */
	addInitEdges(scs, fcs, matrix);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(scs, matrix);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence for only
	 * _one_ possible extension of WB for each location
	 */
	std::vector<llvm::GenericValue *> scLocs = getDoubleLocs();
	for (auto loc : scLocs) {
		auto &stores = modOrder[loc];
		addSCEcos(scs, fcs, stores, matrix);
	}

	/* Calculate the transitive closure of the bool matrix */
	calcTransClosure(matrix, scs.size());
	return isIrreflexive(matrix, scs.size());
}

std::pair<std::vector<Event>, std::vector<bool> >
ExecutionGraph::calcWbRestricted(llvm::GenericValue *addr, View &v)
{
	std::vector<Event> stores;

	std::copy_if(modOrder[addr].begin(), modOrder[addr].end(), std::back_inserter(stores),
		     [&v](Event &s){ return s.index <= v[s.thread]; });
	/* Sort so we can use calcEventIndex() */
	std::sort(stores.begin(), stores.end());

	std::vector<bool> matrix(stores.size() * stores.size(), false);
	for (auto i = 0u; i < stores.size(); i++) {
		auto &lab = getEventLabel(stores[i]);

		std::vector<Event> es;
		std::copy_if(lab.rfm1.begin(), lab.rfm1.end(), std::back_inserter(es),
			     [&v](Event &r){ return r.index <= v[r.thread]; });

		es.push_back(stores[i].prev());
		auto before = getHbBefore(es);
		for (auto j = 0u; j < stores.size(); j++) {
			if (i == j || !isWriteRfBefore(before, stores[j]))
				continue;
			matrix[j * stores.size() + i] = true;

			EventLabel &wLabI = getEventLabel(stores[i]);
			auto upperL = getRMWChainUpperLimit(wLabI, stores[j]);
			int k = calcEventIndex(stores, upperL);

			EventLabel &wLabJ = getEventLabel(stores[j]);
			auto lowerL = getRMWChainLowerLimitInView(wLabJ, upperL, v);
			int l = calcEventIndex(stores, lowerL);
			matrix[l * stores.size() + k] = true;
		}
		/* Add wb-edges for chains of RMWs that read from the initializer */
		for (auto j = 0u; j < stores.size(); j++) {
			EventLabel &wLab = getEventLabel(stores[j]);
			EventLabel &pLab = getPreviousLabel(wLab.getPos());
			if (i == j || !wLab.isRMW() || !pLab.rf.isInitializer())
				continue;

			auto lowerL = getRMWChainLowerLimitInView(wLab, stores[i], v);
			int k = calcEventIndex(stores, lowerL);
			matrix[k * stores.size() + i] = true;
		}
	}
	calcTransClosure(matrix, stores.size());
	return std::make_pair(stores, matrix);
}

std::pair<std::vector<Event>, std::vector<bool> >
ExecutionGraph::calcWb(llvm::GenericValue *addr)
{
	std::vector<Event> stores(modOrder[addr]);
	std::vector<bool> matrix(stores.size() * stores.size(), false);

	/* Sort so we can use calcEventIndex() */
	std::sort(stores.begin(), stores.end());
	for (auto i = 0u; i < stores.size(); i++) {
		auto &lab = getEventLabel(stores[i]);
		std::vector<Event> es(lab.rfm1.begin(), lab.rfm1.end());

		es.push_back(stores[i].prev());
		auto before = getHbBefore(es);
		for (auto j = 0u; j < stores.size(); j++) {
			if (i == j || !isWriteRfBefore(before, stores[j]))
				continue;
			matrix[j * stores.size() + i] = true;

			EventLabel &wLabI = getEventLabel(stores[i]);
			auto upperL = getRMWChainUpperLimit(wLabI, stores[j]);
			int k = calcEventIndex(stores, upperL);

			EventLabel &wLabJ = getEventLabel(stores[j]);
			auto lowerL = getRMWChainLowerLimit(wLabJ, upperL);
			int l = calcEventIndex(stores, lowerL);
			matrix[l * stores.size() + k] = true;
		}
		/* Add wb-edges for chains of RMWs that read from the initializer */
		for (auto j = 0u; j < stores.size(); j++) {
			EventLabel &wLab = getEventLabel(stores[j]);
			EventLabel &pLab = getPreviousLabel(wLab.getPos());
			if (i == j || !wLab.isRMW() || !pLab.rf.isInitializer())
				continue;

			auto lowerL = getRMWChainLowerLimit(wLab, stores[i]);
			int k = calcEventIndex(stores, lowerL);
			matrix[k * stores.size() + i] = true;
		}
	}
	calcTransClosure(matrix, stores.size());
	return std::make_pair(stores, matrix);
}

bool ExecutionGraph::isWbAcyclic(void)
{
	bool acyclic = true;
	for (auto it = modOrder.begin(); it != modOrder.end(); ++it) {
		auto wb = calcWb(it->first);
		auto &stores = wb.first;
		auto &matrix = wb.second;
		for (auto i = 0u; i < stores.size(); i++)
			if (matrix[i * stores.size() + i])
				acyclic = false;
	}
	return acyclic;
}


/************************************************************
 ** Library consistency checking methods
 ***********************************************************/

std::vector<Event> ExecutionGraph::getLibEventsInView(Library &lib, View &v)
{
	std::vector<Event> result;

	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 1; j <= v[i]; j++) { /* Do not consider thread inits */
			EventLabel &lab = events[i][j];
			if (lib.hasMember(lab.functionName))
				result.push_back(lab.getPos());
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
			EventLabel &lab = getEventLabel(froms[i].second[j]);
			if (lab.isRead() && std::find(tos.begin(), tos.end(), lab.rf) != tos.end())
				buf.push_back(lab.rf);
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
			EventLabel &lab = getEventLabel(froms[i].second[j]);
			if (!lab.isWrite())
				continue;
			std::copy_if(lab.rfm1.begin(), lab.rfm1.end(), std::back_inserter(buf),
				     [&tos](Event &e){ return std::find(tos.begin(), tos.end(), e) !=
						     tos.end(); });

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
				auto before = getHbBefore(e);
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
			EventLabel &lab = getEventLabel(froms[i].second[j]);
			if (!lab.isWrite())
				continue;

			View v(getHbBefore(tos[0]));
			for (auto &t : tos) {
				auto o = getHbBefore(t);
				v.updateMax(o);
			}

			auto wb = calcWbRestricted(lab.getAddr(), v);
			auto &ss = wb.first;
			auto &matrix = wb.second;
			// TODO: Make a map with already calculated WBs??

			if (std::find(ss.begin(), ss.end(), lab.getPos()) == ss.end())
				continue;

			/* Collect all wb-after stores that are in "tos" range */
			int k = calcEventIndex(ss, lab.getPos());
			for (auto l = 0u; l < ss.size(); l++)
				if (matrix[k * ss.size() + l])
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
			EventLabel &lab = getEventLabel(froms[i].second[j]);
			if (!lab.isWrite())
				continue;

			/* Collect all mo-after events that are in the "tos" range */
			auto moAfter = modOrder.getMoAfter(lab.getAddr(), lab.getPos());
			for (auto &s : moAfter)
				if (std::find(tos.begin(), tos.end(), s) != tos.end())
					buf.push_back(s);
		}
		froms[i].second = buf;
	}
}

void ExecutionGraph::calcSingleStepPairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
					 std::vector<Event> &tos,
					 llvm::StringMap<std::vector<bool> > &relMap,
					 std::vector<bool> &relMatrix, std::string &step)
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
			  std::vector<Event> &es, std::vector<bool> &matrix)
{
	for (auto &p : pairs) {
		for (auto k = 0u; k < p.second.size(); k++) {
			auto i = calcEventIndex(es, p.first);
			auto j = calcEventIndex(es, p.second[k]);
			matrix[i * es.size() + j] = true;
		}
	}
}

void ExecutionGraph::addStepEdgeToMatrix(std::vector<Event> &es,
					 llvm::StringMap<std::vector<bool> > &relMap,
					 std::vector<bool> &relMatrix,
					 std::vector<std::string> &substeps)
{
	std::vector<std::pair<Event, std::vector<Event> > > edges;

	/* Initialize edges */
	for (auto &e : es)
		edges.push_back(std::make_pair(e, std::vector<Event>({e})));

	for (auto i = 0u; i < substeps.size(); i++)
		calcSingleStepPairs(edges, es, relMap, relMatrix, substeps[i]);
	addEdgePairsToMatrix(edges, es, relMatrix);
}

llvm::StringMap<std::vector<bool> >
ExecutionGraph::calculateAllRelations(Library &lib, std::vector<Event> &es)
{
	llvm::StringMap<std::vector<bool> > relMap;

	std::sort(es.begin(), es.end());
	for (auto &r : lib.getRelations()) {
		std::vector<bool> relMatrix(es.size() * es.size(), false);
		auto &steps = r.getSteps();
		for (auto &s : steps)
			addStepEdgeToMatrix(es, relMap, relMatrix, s);

		if (r.isTransitive())
			calcTransClosure(relMatrix, es.size());
		relMap[r.getName()] = relMatrix;
	}
	return relMap;
}

bool ExecutionGraph::isLibConsistentInView(Library &lib, View &v)
{
	auto es = getLibEventsInView(lib, v);
	auto relations = calculateAllRelations(lib, es);
	auto &constraints = lib.getConstraints();
	if (std::all_of(constraints.begin(), constraints.end(),
			[&relations, &es](Constraint &c)
			{ return isIrreflexive(relations[c.getName()], es.size()); }))
		return true;
	return false;
}

std::vector<Event>
ExecutionGraph::getLibConsRfsInView(Library &lib, EventLabel &rLab,
				    const std::vector<Event> &stores, View &v)
{
	auto oldRf = rLab.rf;
	std::vector<Event> filtered;

	for (auto &s : stores) {
		changeRf(rLab, s);
		if (isLibConsistentInView(lib, v))
			filtered.push_back(s);
	}
	/* Restore the old reads-from, and eturn all the valid reads-from options */
	changeRf(rLab, oldRf);
	return filtered;
}


/************************************************************
 ** Debugging methods
 ***********************************************************/

void ExecutionGraph::validate(void)
{
	for (auto i = 0u; i < events.size(); i++) {
		for (auto j = 0u; j < events[i].size(); j++) {
			EventLabel &lab = events[i][j];
			if (lab.isRead()) {
				Event &rf = lab.rf;
				if (lab.rf.isInitializer())
					continue;
				bool readExists = false;
				for (auto &r : getEventLabel(rf).rfm1)
					if (r == lab.getPos())
						readExists = true;
				if (!readExists) {
					WARN("Read event is not the appropriate rf-1 list!\n");
					llvm::dbgs() << lab.getPos() << "\n";
					llvm::dbgs() << *this << "\n";
					abort();
				}
			} else if (lab.isWrite()) {
				if (lab.isRMW() && std::count_if(lab.rfm1.begin(), lab.rfm1.end(),
						  [&](Event &load){ return isRMWLoad(load); }) > 1) {
					WARN("RMW store is read from more than 1 load!\n");
					llvm::dbgs() << "RMW store: " << lab.getPos() << "\nReads:";
					for (auto &r : lab.rfm1)
						llvm::dbgs() << r << " ";
					llvm::dbgs() << "\n";
					llvm::dbgs() << *this << "\n";
					abort();
				}
				if (lab.rfm1.empty())
					continue;
				bool writeExists = false;
				for (auto &e : lab.rfm1)
					if (getEventLabel(e).rf == lab.getPos())
						writeExists = true;
				if (!writeExists) {
					WARN("Write event is not marked in the read event!\n");
					llvm::dbgs() << lab.getPos() << "\n";
					llvm::dbgs() << *this << "\n";
					abort();
				}
				for (auto &r : lab.rfm1) {
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


/************************************************************
 ** Overloaded operators
 ***********************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ExecutionGraph &g)
{
	for (auto i = 0u; i < g.events.size(); i++) {
		for (auto j = 0u; j < g.events[i].size(); j++) {
			auto &lab = g.events[i][j];
			s << "\t" << lab << "\n";
			if (lab.isRead())
				s << "\t\treads from: " << lab.rf << "\n";
			else if (lab.isStart())
				s << "\t\tforked from: " << lab.rf << "\n";
			else if (lab.isJoin())
				s << "\t\tawaits for: " << lab.cid << ", (rf: "
				  << lab.rf << ")\n";
		}
	}
	s << "Thread sizes:\n\t";
	for (auto i = 0u; i < g.events.size(); i++)
		s << g.events[i].size() << " ";
	s << "\n";
	return s;
}
