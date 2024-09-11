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

/*******************************************************************************
 * CAUTION: This file is generated automatically by Kater -- DO NOT EDIT.
 *******************************************************************************/

#include "IMMDriver.hpp"
#include "Static/ModuleInfo.hpp"

IMMDriver::IMMDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		std::unique_ptr<ModuleInfo> MI, GenMCDriver::Mode mode /* = GenMCDriver::VerificationMode{} */)
	: GenMCDriver(conf, std::move(mod), std::move(MI), mode) {}

bool IMMDriver::isDepTracking() const
{
	return 1;
}

bool IMMDriver::visitCalc61_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();




	return true;
}

bool IMMDriver::visitCalc61_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = lab; true)if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool IMMDriver::visitCalc61_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();

	if (visitedCalc61_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc61_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc61_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_2(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab))if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab))if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			if (!visitCalc61_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc61_1(pLab, calcRes)){
				return false;
		}

	}

	visitedCalc61_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMDriver::visitCalc61_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();

	if (visitedCalc61_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc61_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc61_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_2(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedCalc61_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_4(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc61_1(pLab, calcRes)){
				return false;
		}

	}

	visitedCalc61_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMDriver::visitCalc61_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();

	if (visitedCalc61_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc61_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedCalc61_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_3(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCalc61_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMDriver::visitCalc61_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();

	if (visitedCalc61_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc61_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc61_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_5(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc61_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_2(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedCalc61_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_4(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc61_1(pLab, calcRes)){
				return false;
		}

	}

	visitedCalc61_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMDriver::visitCalc61_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (true && lab->isAtLeastAcquire() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc61_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_5(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc61_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_5(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCalc61_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc61_3(pLab, calcRes)){
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc61_7(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc61_1(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool IMMDriver::visitCalc61_7(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = tc_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tc_pred(g, lab); pLab) {
			if (!visitCalc61_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab) {
			if (!visitCalc61_1(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

View IMMDriver::visitCalc61(const EventLabel *lab) const
{
	auto &g = getGraph();
	View calcRes;

	visitedCalc61_2.clear();
	visitedCalc61_2.resize(g.getMaxStamp().get() + 1);
	visitedCalc61_3.clear();
	visitedCalc61_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc61_4.clear();
	visitedCalc61_4.resize(g.getMaxStamp().get() + 1);
	visitedCalc61_5.clear();
	visitedCalc61_5.resize(g.getMaxStamp().get() + 1);

	visitCalc61_6(lab, calcRes);
	return calcRes;
}
auto IMMDriver::checkCalc61(const EventLabel *lab) const
{
	auto &g = getGraph();

	return visitCalc61(lab);
}
bool IMMDriver::visitCalc73_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();




	return true;
}

bool IMMDriver::visitCalc73_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = lab; true)if (calcRes.update(pLab->view(1)); true) {
			if (!visitCalc73_0(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool IMMDriver::visitCalc73_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc73_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc73_3(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc73_1(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool IMMDriver::visitCalc73_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = tc_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc73_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc73_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc73_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tc_pred(g, lab); pLab) {
			if (!visitCalc73_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab) {
			if (!visitCalc73_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab) {
			if (!visitCalc73_1(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

View IMMDriver::visitCalc73(const EventLabel *lab) const
{
	auto &g = getGraph();
	View calcRes;


	visitCalc73_2(lab, calcRes);
	return calcRes;
}
auto IMMDriver::checkCalc73(const EventLabel *lab) const
{
	auto &g = getGraph();

	return visitCalc73(lab);
}
void IMMDriver::calculateSaved(EventLabel *lab)
{
}

void IMMDriver::calculateViews(EventLabel *lab)
{
	lab->addView(checkCalc61(lab));
	lab->addView(checkCalc73(lab));
}

void IMMDriver::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
}

const View &IMMDriver::getHbView(const EventLabel *lab) const
{
	return lab->view(0);
}


bool IMMDriver::isWriteRfBefore(Event a, Event b)
{
	auto &g = getGraph();
	auto &before = g.getEventLabel(b)->view(0);
	if (before.contains(a))
		return true;

	const EventLabel *lab = g.getEventLabel(a);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (auto &rLab : wLab->readers())
		if (before.contains(rLab.getPos()))
			return true;
	return false;
}

std::vector<Event>
IMMDriver::getInitRfsAtLoc(SAddr addr)
{
	std::vector<Event> result;

	for (const auto &lab : getGraph().labels()) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab))
			if (rLab->getRf()->getPos().isInitializer() && rLab->getAddr() == addr)
				result.push_back(rLab->getPos());
	}
	return result;
}

bool IMMDriver::isHbOptRfBefore(const Event e, const Event write)
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(write);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *sLab = static_cast<const WriteLabel *>(lab);
	if (sLab->view(0).contains(e))
		return true;

	for (auto &rLab : sLab->readers()) {
		if (rLab.view(0).contains(e))
			return true;
	}
	return false;
}

ExecutionGraph::co_iterator
IMMDriver::splitLocMOBefore(SAddr addr, Event e)
{
	auto &g = getGraph();
	auto rit = std::find_if(g.co_rbegin(addr), g.co_rend(addr), [&](auto &lab){
		return isWriteRfBefore(lab.getPos(), e);
	});
	/* Convert to forward iterator, but be _really_ careful */
	if (rit == g.co_rend(addr))
		return g.co_begin(addr);
	return ++ExecutionGraph::co_iterator(*rit);
}

ExecutionGraph::co_iterator
IMMDriver::splitLocMOAfterHb(SAddr addr, const Event read)
{
	auto &g = getGraph();

	auto initRfs = g.getInitRfsAtLoc(addr);
	if (std::any_of(initRfs.begin(), initRfs.end(), [&read,&g](const Event &rf){
		return g.getEventLabel(rf)->view(0).contains(read);
	}))
		return g.co_begin(addr);

	auto it = std::find_if(g.co_begin(addr), g.co_end(addr), [&](auto &lab){
		return isHbOptRfBefore(read, lab.getPos());
	});
	if (it == g.co_end(addr) || it->view(0).contains(read))
		return it;
	return ++it;
}

ExecutionGraph::co_iterator
IMMDriver::splitLocMOAfter(SAddr addr, const Event e)
{
	auto &g = getGraph();
	return std::find_if(g.co_begin(addr), g.co_end(addr), [&](auto &lab){
		return isHbOptRfBefore(e, lab.getPos());
	});
}

std::vector<Event>
IMMDriver::getCoherentStores(SAddr addr, Event read)
{
	auto &g = getGraph();
	std::vector<Event> stores;

	/* Fastpath: co_max(G) is po-before R */
	auto comax = g.co_rbegin(addr) == g.co_rend(addr) ? Event::getInit() :
		     g.co_rbegin(addr)->getPos();
	if (comax.thread == read.thread && comax.index < read.index)
		return {comax};

	/*
	 * If there are no stores (rf?;hb)-before the current event
	 * then we can read read from all concurrent stores and the
	 * initializer store. Otherwise, we can read from all concurrent
	 * stores and the mo-latest of the (rf?;hb)-before stores.
	 */
	auto begIt = splitLocMOBefore(addr, read);
	if (begIt == g.co_begin(addr))
		stores.push_back(Event::getInit());
	else {
		stores.push_back((--begIt)->getPos());
		++begIt;
	}

	/*
	 * If the model supports out-of-order execution we have to also
	 * account for the possibility the read is hb-before some other
	 * store, or some read that reads from a store.
	 */
	auto endIt = (isDepTracking()) ? splitLocMOAfterHb(addr, read) : g.co_end(addr);
	std::transform(begIt, endIt, std::back_inserter(stores), [&](auto &lab){
		return lab.getPos();
	});
	return stores;
}

std::vector<Event>
IMMDriver::getMOOptRfAfter(const WriteLabel *sLab)
{
	std::vector<Event> after;
	std::vector<const ReadLabel *> rfAfter;

	const auto &g = getGraph();
	std::for_each(g.co_succ_begin(sLab), g.co_succ_end(sLab),
		      [&](auto &wLab){
			      after.push_back(wLab.getPos());
			      std::transform(wLab.readers_begin(), wLab.readers_end(), std::back_inserter(rfAfter),
			      [&](auto &rLab){ return &rLab; });
	});
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after), [](auto *rLab){
		return rLab->getPos();
	});
	return after;
}

std::vector<Event>
IMMDriver::getMOInvOptRfAfter(const WriteLabel *sLab)
{
	auto &g = getGraph();
	std::vector<Event> after;
	std::vector<const ReadLabel *> rfAfter;

	/* First, add (mo;rf?)-before */
	std::for_each(g.co_pred_begin(sLab),
		      g.co_pred_end(sLab), [&](auto &wLab){
			      after.push_back(wLab.getPos());
			      std::transform(wLab.readers_begin(), wLab.readers_end(), std::back_inserter(rfAfter),
			      [&](auto &rLab){ return &rLab; });
	});
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after), [](auto *rLab){
		return rLab->getPos();
	});

	/* Then, we add the reader list for the initializer */
	auto initRfs = g.getInitRfsAtLoc(sLab->getAddr());
	after.insert(after.end(), initRfs.begin(), initRfs.end());
	return after;
}

static std::vector<Event>
getRevisitableFrom(const ExecutionGraph &g, const WriteLabel *sLab,
		   const VectorClock &pporf, const WriteLabel *coPred)
{
	auto pendingRMW = g.getPendingRMW(sLab);
	std::vector<Event> loads;

	for (auto &rLab : coPred->readers()) {
		if (!pporf.contains(rLab.getPos()) && rLab.getAddr() == sLab->getAddr() &&
		    rLab.isRevisitable() && rLab.wasAddedMax())
			loads.push_back(rLab.getPos());
	}
	if (!pendingRMW.isInitializer())
		loads.erase(std::remove_if(loads.begin(), loads.end(),
					   [&](Event &e) {
						   auto *confLab = g.getEventLabel(pendingRMW);
						   return g.getEventLabel(e)->getStamp() >
							  confLab->getStamp();
					   }),
			    loads.end());
	return loads;
}

std::vector<Event>
IMMDriver::getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf)
{
	auto &g = getGraph();
	std::vector<Event> ls;

	/* Fastpath: previous co-max is ppo-before SLAB */
	auto prevCoMaxIt = std::find_if(g.co_rbegin(sLab->getAddr()), g.co_rend(sLab->getAddr()),
					[&](auto &lab) { return lab.getPos() != sLab->getPos(); });
	if (prevCoMaxIt != g.co_rend(sLab->getAddr()) && pporf.contains(prevCoMaxIt->getPos())) {
		ls = getRevisitableFrom(g, sLab, pporf, &*prevCoMaxIt);
	} else {
		ls = g.getRevisitable(sLab, pporf);
	}

	/* If this store is po- and mo-maximal then we are done */
	if (!isDepTracking() && g.isCoMaximal(sLab->getAddr(), sLab->getPos()))
		return ls;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
				{ const View &before = g.getEventLabel(e)->view(0);
				  return std::any_of(optRfs.begin(), optRfs.end(),
					 [&](Event ev)
					 { return before.contains(ev); });
				}), ls.end());

	/* If out-of-order event addition is not supported, then we are done
	 * due to po-maximality */
	if (!isDepTracking())
		return ls;

	/* Otherwise, we also have to exclude hb-before loads */
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
		{ return g.getEventLabel(sLab->getPos())->view(0).contains(e); }),
		ls.end());

	/* ...and also exclude (mo^-1; rf?; (hb^-1)?; sb^-1)-after reads in
	 * the resulting graph */
	auto &before = pporf;
	auto moInvOptRfs = getMOInvOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
				{ auto *eLab = g.getEventLabel(e);
				  auto v = g.getViewFromStamp(eLab->getStamp());
				  v->update(before);
				  return std::any_of(moInvOptRfs.begin(),
						     moInvOptRfs.end(),
						     [&](Event ev)
						     { return v->contains(ev) &&
						       g.getEventLabel(ev)->view(0).contains(e); });
				}),
		 ls.end());

	return ls;
}

std::vector<Event>
IMMDriver::getCoherentPlacings(SAddr addr, Event store, bool isRMW)
{
	auto &g = getGraph();
	std::vector<Event> result;

	/* If it is an RMW store, there is only one possible position in MO */
	if (isRMW) {
		auto *rLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(store.prev()));
		BUG_ON(!rLab);
		auto *rfLab = rLab->getRf();
		BUG_ON(!rfLab);
		result.push_back(rfLab->getPos());
		return result;
	}

	/* Otherwise, we calculate the full range and add the store */
	auto rangeBegin = splitLocMOBefore(addr, store);
	auto rangeEnd = (isDepTracking()) ? splitLocMOAfter(addr, store) : g.co_end(addr);
	auto cos = llvm::iterator_range(rangeBegin, rangeEnd) |
		   std::views::filter([&](auto &sLab) { return !g.isRMWStore(sLab.getPos()); }) |
		   std::views::transform([&](auto &sLab) {
			   auto *pLab = g.co_imm_pred(&sLab);
			   return pLab ? pLab->getPos() : Event::getInit();
		   });
	std::ranges::copy(cos, std::back_inserter(result));
	result.push_back(rangeEnd == g.co_end(addr)   ? g.co_max(addr)->getPos()
			 : !g.co_imm_pred(&*rangeEnd) ? Event::getInit()
						      : g.co_imm_pred(&*rangeEnd)->getPos());
	return result;
}
bool IMMDriver::visitCoherence_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedCoherenceAccepting;


	--visitedCoherenceAccepting;
	return true;
}

bool IMMDriver::visitCoherence_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedCoherence_1[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedCoherence_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_3(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedCoherence_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_5(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedCoherence_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_5(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedCoherence_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCoherence_6(pLab)){
				return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
	visitedCoherence_1[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitCoherence_2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedCoherence_2[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedCoherence_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedCoherence_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedCoherence_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
	visitedCoherence_2[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitCoherence_3(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedCoherence_3[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedCoherence_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedCoherence_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedCoherence_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedCoherence_3[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitCoherence_4(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedCoherence_4[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedCoherence_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_3(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedCoherence_4[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitCoherence_5(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedCoherence_5[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedCoherence_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_5(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedCoherence_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedCoherence_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedCoherence_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedCoherence_5[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitCoherence_6(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedCoherence_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedCoherence_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitCoherence_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedCoherenceAccepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
	return true;
}

bool IMMDriver::visitCoherenceFull() const
{
	auto &g = getGraph();

	visitedCoherenceAccepting = 0;
	visitedCoherence_1.clear();
	visitedCoherence_1.resize(g.getMaxStamp().get() + 1);
	visitedCoherence_2.clear();
	visitedCoherence_2.resize(g.getMaxStamp().get() + 1);
	visitedCoherence_3.clear();
	visitedCoherence_3.resize(g.getMaxStamp().get() + 1);
	visitedCoherence_4.clear();
	visitedCoherence_4.resize(g.getMaxStamp().get() + 1);
	visitedCoherence_5.clear();
	visitedCoherence_5.resize(g.getMaxStamp().get() + 1);
	return true
		&& std::ranges::all_of(g.labels(), [&](auto &lab){ return visitedCoherence_1[lab.getStamp().get()].status != NodeStatus::unseen || visitCoherence_1(&lab); });
}

bool IMMDriver::visitConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_0[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_0[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_1[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_1[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_2[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			if (!visitConsAcyclic1_3(pLab)){
				return false;
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_2[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_3(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	return true;
}

bool IMMDriver::visitConsAcyclic1_4(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_4[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_4[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_5(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	return true;
}

bool IMMDriver::visitConsAcyclic1_6(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_6[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitConsAcyclic1_5(pLab)){
				return false;
		}
	}
	visitedConsAcyclic1_6[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_7(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_7[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_7[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_8(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_8[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_8[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_9(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_9[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_9[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_10(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_10[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_10[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_11(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	return true;
}

bool IMMDriver::visitConsAcyclic1_12(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_12[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_12[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_13(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_13[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_16(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_13[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_14(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_14[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = co_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_14[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_15(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_15[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_15[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_16(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_16[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			if (!visitConsAcyclic1_17(pLab)){
				return false;
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_16[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_17(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_16(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	return true;
}

bool IMMDriver::visitConsAcyclic1_18(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_18[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_16(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_18[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_19(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_19[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_19[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_20(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1_20[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_20[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1_21(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_21[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };


	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_16(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_16(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_21(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	--visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_21[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_4.clear();
	visitedConsAcyclic1_4.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.clear();
	visitedConsAcyclic1_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.clear();
	visitedConsAcyclic1_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.clear();
	visitedConsAcyclic1_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.clear();
	visitedConsAcyclic1_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.clear();
	visitedConsAcyclic1_10.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.clear();
	visitedConsAcyclic1_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.clear();
	visitedConsAcyclic1_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.clear();
	visitedConsAcyclic1_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.clear();
	visitedConsAcyclic1_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_16.clear();
	visitedConsAcyclic1_16.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.clear();
	visitedConsAcyclic1_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.clear();
	visitedConsAcyclic1_19.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_20.clear();
	visitedConsAcyclic1_20.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_21.clear();
	visitedConsAcyclic1_21.resize(g.getMaxStamp().get() + 1);
	return true
		&& (visitedConsAcyclic1_6[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_6(lab))
		&& (visitedConsAcyclic1_7[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_7(lab))
		&& (visitedConsAcyclic1_8[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_8(lab))
		&& (visitedConsAcyclic1_10[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_10(lab))
		&& (visitedConsAcyclic1_13[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_13(lab))
		&& (visitedConsAcyclic1_14[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_14(lab))
		&& (visitedConsAcyclic1_20[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_20(lab))
		&& (visitedConsAcyclic1_21[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_21(lab));
}

bool IMMDriver::visitConsAcyclic1Full() const
{
	auto &g = getGraph();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_4.clear();
	visitedConsAcyclic1_4.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.clear();
	visitedConsAcyclic1_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.clear();
	visitedConsAcyclic1_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.clear();
	visitedConsAcyclic1_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.clear();
	visitedConsAcyclic1_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.clear();
	visitedConsAcyclic1_10.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.clear();
	visitedConsAcyclic1_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.clear();
	visitedConsAcyclic1_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.clear();
	visitedConsAcyclic1_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.clear();
	visitedConsAcyclic1_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_16.clear();
	visitedConsAcyclic1_16.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.clear();
	visitedConsAcyclic1_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.clear();
	visitedConsAcyclic1_19.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_20.clear();
	visitedConsAcyclic1_20.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_21.clear();
	visitedConsAcyclic1_21.resize(g.getMaxStamp().get() + 1);
	return true
		&& std::ranges::all_of(g.labels(), [&](auto &lab){ return visitedConsAcyclic1_21[lab.getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_21(&lab); });
}

bool IMMDriver::visitLHSUnlessConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = getGraph();


	return false;


	return true;
}

bool IMMDriver::visitLHSUnlessConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = getGraph();


	for (auto &tmp : other_labels(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isSC()) {
			if (!visitLHSUnlessConsAcyclic1_0(pLab)){
			return false;
		}

	}

	return true;
}

bool IMMDriver::visitUnlessConsAcyclic1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessConsAcyclic1Accepting.clear();
	visitedLHSUnlessConsAcyclic1Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessConsAcyclic1Accepting.clear();
	visitedRHSUnlessConsAcyclic1Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessConsAcyclic1_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessConsAcyclic1Accepting.size(); i++) {
		if (visitedLHSUnlessConsAcyclic1Accepting[i] && !visitedRHSUnlessConsAcyclic1Accepting[i]) {
			return false;
		}
	}
	return true;
}

bool IMMDriver::checkConsAcyclic1(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessConsAcyclic1(lab))
		return true;

	return visitConsAcyclic1(lab);
}
bool IMMDriver::visitConsAcyclic2_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_0[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_0[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_1[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_1[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_2[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rfi_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			if (!visitConsAcyclic2_3(pLab)){
				return false;
		}
	}
	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_2[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_3(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	return true;
}

bool IMMDriver::visitConsAcyclic2_4(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_4[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_4[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_5(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_5[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = poloc_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_5(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_5[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_6(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_6[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_6[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_7(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_7[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_11(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_11(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_7[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_8(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_8[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_11(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_8[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_9(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_9[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_8(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			if (!visitConsAcyclic2_10(pLab)){
				return false;
		}
	}
	visitedConsAcyclic2_9[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_10(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	return true;
}

bool IMMDriver::visitConsAcyclic2_11(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_11[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_11(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_11[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_12(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_12[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_9(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_6(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_12[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_13(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_13[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_15(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_13[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_14(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_14[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_14[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_15(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_15[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			if (!visitConsAcyclic2_16(pLab)){
				return false;
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_14(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_15[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_16(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_15(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	return true;
}

bool IMMDriver::visitConsAcyclic2_17(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_17[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_15(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_17[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_18(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2_18[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic2_18[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2_19(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedConsAcyclic2Accepting;
	visitedConsAcyclic2_19[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::entered };


	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : detour_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable()) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_18(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_1(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_15(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_15(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic2_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_4(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = poloc_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_5(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	--visitedConsAcyclic2Accepting;
	visitedConsAcyclic2_19[lab->getStamp().get()] = { visitedConsAcyclic2Accepting, NodeStatus::left };
	return true;
}

bool IMMDriver::visitConsAcyclic2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic2Accepting = 0;
	visitedConsAcyclic2_0.clear();
	visitedConsAcyclic2_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_1.clear();
	visitedConsAcyclic2_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_2.clear();
	visitedConsAcyclic2_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_4.clear();
	visitedConsAcyclic2_4.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_5.clear();
	visitedConsAcyclic2_5.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_6.clear();
	visitedConsAcyclic2_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_7.clear();
	visitedConsAcyclic2_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_8.clear();
	visitedConsAcyclic2_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_9.clear();
	visitedConsAcyclic2_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_11.clear();
	visitedConsAcyclic2_11.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_12.clear();
	visitedConsAcyclic2_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_13.clear();
	visitedConsAcyclic2_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_14.clear();
	visitedConsAcyclic2_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_15.clear();
	visitedConsAcyclic2_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_17.clear();
	visitedConsAcyclic2_17.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_18.clear();
	visitedConsAcyclic2_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_19.clear();
	visitedConsAcyclic2_19.resize(g.getMaxStamp().get() + 1);
	return true
		&& (visitedConsAcyclic2_6[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic2_6(lab))
		&& (visitedConsAcyclic2_7[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic2_7(lab))
		&& (visitedConsAcyclic2_9[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic2_9(lab))
		&& (visitedConsAcyclic2_12[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic2_12(lab))
		&& (visitedConsAcyclic2_13[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic2_13(lab));
}

bool IMMDriver::visitConsAcyclic2Full() const
{
	auto &g = getGraph();

	visitedConsAcyclic2Accepting = 0;
	visitedConsAcyclic2_0.clear();
	visitedConsAcyclic2_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_1.clear();
	visitedConsAcyclic2_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_2.clear();
	visitedConsAcyclic2_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_4.clear();
	visitedConsAcyclic2_4.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_5.clear();
	visitedConsAcyclic2_5.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_6.clear();
	visitedConsAcyclic2_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_7.clear();
	visitedConsAcyclic2_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_8.clear();
	visitedConsAcyclic2_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_9.clear();
	visitedConsAcyclic2_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_11.clear();
	visitedConsAcyclic2_11.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_12.clear();
	visitedConsAcyclic2_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_13.clear();
	visitedConsAcyclic2_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_14.clear();
	visitedConsAcyclic2_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_15.clear();
	visitedConsAcyclic2_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_17.clear();
	visitedConsAcyclic2_17.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_18.clear();
	visitedConsAcyclic2_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_19.clear();
	visitedConsAcyclic2_19.resize(g.getMaxStamp().get() + 1);
	return true
		&& std::ranges::all_of(g.labels(), [&](auto &lab){ return visitedConsAcyclic2_19[lab.getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic2_19(&lab); });
}

bool IMMDriver::checkConsAcyclic2(const EventLabel *lab) const
{
	auto &g = getGraph();


	return visitConsAcyclic2(lab);
}
bool IMMDriver::visitWarning3(const EventLabel *lab) const
{
	return false;
}

bool IMMDriver::visitLHSUnlessWarning3_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool IMMDriver::visitLHSUnlessWarning3_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (true && llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<WriteLabel>(pLab)) {
			if (!visitLHSUnlessWarning3_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool IMMDriver::visitUnlessWarning3(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessWarning3Accepting.clear();
	visitedLHSUnlessWarning3Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true
		&& visitLHSUnlessWarning3_1(lab, v);
}

bool IMMDriver::checkWarning3(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessWarning3(lab))
		return true;

	return visitWarning3(lab);
}
VerificationError IMMDriver::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	return VerificationError::VE_OK;
}

std::vector<VerificationError> IMMDriver::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings, std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning3(lab)) {
		racyLabs.push_back(cexLab);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool IMMDriver::isConsistent(const EventLabel *lab) const
{

	return true
		&& checkConsAcyclic1(lab)
		&& checkConsAcyclic2(lab);
}

void IMMDriver::visitPPoRf0(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf1(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<FenceLabel>(pLab)) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<FenceLabel>(pLab))if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<FenceLabel>(pLab)) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf2(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf2(pLab, pporf);
	}
	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf3(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::entered;
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = poloc_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	for (auto &tmp : detour_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && lab->isAtLeastRelease())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (auto pLab = tc_pred(g, lab); pLab)if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (auto pLab = tj_pred(g, lab); pLab)if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (auto pLab = rfe_pred(g, lab); pLab)if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	for (auto &tmp : detour_preds(g, lab)) if (auto *pLab = &tmp; true)if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab))if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (true && lab->isAtLeastRelease())if (auto pLab = po_imm_pred(g, lab); pLab)if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf2(pLab, pporf);
	}
	if (true && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf2(pLab, pporf);
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	for (auto &tmp : detour_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (true && lab->isAtLeastRelease())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab) && ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) || (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (true && llvm::isa<FenceLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf4(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = poloc_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab))if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (auto pLab = poloc_imm_pred(g, lab); pLab)if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf5(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (true && llvm::isa<ReadLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf6(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = rfi_pred(g, lab); pLab)if (true && llvm::isa<WriteLabel>(pLab) && ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) || (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
		auto status = visitedPPoRf5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf5(pLab, pporf);
	}
	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	for (auto &p : ctrl_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	for (auto &p : data_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf7(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf7[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable())if (pporf.updateIdx(pLab->getPos()); true) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	for (auto &p : addr_preds(g, lab)) if (auto *pLab = g.getEventLabel(p); true)if (true && pLab->isDependable()) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	visitedPPoRf7[lab->getStamp().get()] = NodeStatus::left;
}

DepView IMMDriver::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = getGraph();
	DepView pporf;
	pporf.updateIdx(lab->getPos());
	visitedPPoRf0.clear();
	visitedPPoRf0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf1.clear();
	visitedPPoRf1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf2.clear();
	visitedPPoRf2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf3.clear();
	visitedPPoRf3.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf4.clear();
	visitedPPoRf4.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf5.clear();
	visitedPPoRf5.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf6.clear();
	visitedPPoRf6.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf7.clear();
	visitedPPoRf7.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);

	visitPPoRf1(lab, pporf);
	visitPPoRf3(lab, pporf);
	return pporf;
}
std::unique_ptr<VectorClock> IMMDriver::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<DepView>(calcPPoRfBefore(lab));
}
