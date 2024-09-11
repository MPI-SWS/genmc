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

#include "SCDriver.hpp"
#include "Static/ModuleInfo.hpp"

SCDriver::SCDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		std::unique_ptr<ModuleInfo> MI, GenMCDriver::Mode mode /* = GenMCDriver::VerificationMode{} */)
	: GenMCDriver(conf, std::move(mod), std::move(MI), mode) {}

bool SCDriver::isDepTracking() const
{
	return 0;
}

bool SCDriver::visitCalc57_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();




	return true;
}

bool SCDriver::visitCalc57_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = lab; true)if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc57_0(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool SCDriver::visitCalc57_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc57_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc57_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc57_3(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool SCDriver::visitCalc57_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = tc_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc57_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc57_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc57_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tc_pred(g, lab); pLab) {
			if (!visitCalc57_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab) {
			if (!visitCalc57_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab) {
			if (!visitCalc57_1(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

View SCDriver::visitCalc57(const EventLabel *lab) const
{
	auto &g = getGraph();
	View calcRes;


	visitCalc57_2(lab, calcRes);
	return calcRes;
}
auto SCDriver::checkCalc57(const EventLabel *lab) const
{
	auto &g = getGraph();

	return visitCalc57(lab);
}
bool SCDriver::visitCalc62_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();




	return true;
}

bool SCDriver::visitCalc62_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = lab; true)if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc62_0(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool SCDriver::visitCalc62_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc62_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCalc62_3(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

bool SCDriver::visitCalc62_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = getGraph();


	if (auto pLab = tc_pred(g, lab); pLab) {
			if (!visitCalc62_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab) {
			if (!visitCalc62_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab) {
			if (!visitCalc62_1(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tc_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = tj_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)){
				return false;
		}

	}
	if (auto pLab = rf_pred(g, lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)){
				return false;
		}

	}

	return true;
}

View SCDriver::visitCalc62(const EventLabel *lab) const
{
	auto &g = getGraph();
	View calcRes;


	visitCalc62_2(lab, calcRes);
	return calcRes;
}
auto SCDriver::checkCalc62(const EventLabel *lab) const
{
	auto &g = getGraph();

	return visitCalc62(lab);
}
void SCDriver::calculateSaved(EventLabel *lab)
{
}

void SCDriver::calculateViews(EventLabel *lab)
{
	lab->addView(checkCalc57(lab));
	lab->addView(checkCalc62(lab));
}

void SCDriver::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
	lab->setPrefixView(calculatePrefixView(lab));
}

const View &SCDriver::getHbView(const EventLabel *lab) const
{
	return lab->view(0);
}


bool SCDriver::isWriteRfBefore(Event a, Event b)
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
SCDriver::getInitRfsAtLoc(SAddr addr)
{
	std::vector<Event> result;

	for (const auto &lab : getGraph().labels()) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab))
			if (rLab->getRf()->getPos().isInitializer() && rLab->getAddr() == addr)
				result.push_back(rLab->getPos());
	}
	return result;
}

bool SCDriver::isHbOptRfBefore(const Event e, const Event write)
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
SCDriver::splitLocMOBefore(SAddr addr, Event e)
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
SCDriver::splitLocMOAfterHb(SAddr addr, const Event read)
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
SCDriver::splitLocMOAfter(SAddr addr, const Event e)
{
	auto &g = getGraph();
	return std::find_if(g.co_begin(addr), g.co_end(addr), [&](auto &lab){
		return isHbOptRfBefore(e, lab.getPos());
	});
}

std::vector<Event>
SCDriver::getCoherentStores(SAddr addr, Event read)
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
SCDriver::getMOOptRfAfter(const WriteLabel *sLab)
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
SCDriver::getMOInvOptRfAfter(const WriteLabel *sLab)
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
SCDriver::getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf)
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
SCDriver::getCoherentPlacings(SAddr addr, Event store, bool isRMW)
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
bool SCDriver::visitCoherence_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedCoherenceAccepting;


	--visitedCoherenceAccepting;
	return true;
}

bool SCDriver::visitCoherence_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedCoherence_1[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::entered };

	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCoherence_2(pLab)){
				return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
			if (!visitCoherence_0(pLab)){
				return false;
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
	visitedCoherence_1[lab->getStamp().get()] = { visitedCoherenceAccepting, NodeStatus::left };
	return true;
}

bool SCDriver::visitCoherence_2(const EventLabel *lab) const
{
	auto &g = getGraph();


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
	if (auto pLab = rf_pred(g, lab); pLab) {
			if (!visitCoherence_0(pLab)){
				return false;
		}
	}
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
	if (auto pLab = rf_pred(g, lab); pLab) {
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
	return true;
}

bool SCDriver::visitCoherenceFull() const
{
	auto &g = getGraph();

	visitedCoherenceAccepting = 0;
	visitedCoherence_1.clear();
	visitedCoherence_1.resize(g.getMaxStamp().get() + 1);
	return true
		&& std::ranges::all_of(g.labels(), [&](auto &lab){ return visitedCoherence_1[lab.getStamp().get()].status != NodeStatus::unseen || visitCoherence_1(&lab); });
}

bool SCDriver::visitConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_0[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };


	if (auto pLab = tc_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = tj_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : rf_succs(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = co_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = fr_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}
		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	--visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_0[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };
	return true;
}

bool SCDriver::visitConsAcyclic1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	return true
		&& (visitedConsAcyclic1_0[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_0(lab));
}

bool SCDriver::visitConsAcyclic1Full() const
{
	auto &g = getGraph();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	return true
		&& std::ranges::all_of(g.labels(), [&](auto &lab){ return visitedConsAcyclic1_0[lab.getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_0(&lab); });
}

bool SCDriver::checkConsAcyclic1(const EventLabel *lab) const
{
	auto &g = getGraph();


	return visitConsAcyclic1(lab);
}
bool SCDriver::visitError2(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessError2_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool SCDriver::visitLHSUnlessError2_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (auto pLab = alloc_pred(g, lab); pLab) {
			if (!visitLHSUnlessError2_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessError2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessError2Accepting.clear();
	visitedLHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true
		&& visitLHSUnlessError2_1(lab, v);
}

bool SCDriver::checkError2(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessError2(lab))
		return true;

	return visitError2(lab);
}
bool SCDriver::visitError3(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessError3_0(const EventLabel *lab) const
{
	auto &g = getGraph();


	return false;


	return true;
}

bool SCDriver::visitLHSUnlessError3_1(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError3_0(pLab)){
			return false;
		}

	}
	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError3_0(pLab)){
			return false;
		}

	}
	if (true && llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError3_0(pLab)){
			return false;
		}

	}
	if (true && llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError3_0(pLab)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessError3(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessError3Accepting.clear();
	visitedLHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError3Accepting.clear();
	visitedRHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError3_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError3Accepting.size(); i++) {
		if (visitedLHSUnlessError3Accepting[i] && !visitedRHSUnlessError3Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool SCDriver::checkError3(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessError3(lab))
		return true;

	return visitError3(lab);
}
bool SCDriver::visitError4(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessError4_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool SCDriver::visitLHSUnlessError4_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	for (auto &tmp : alloc_succs(g, lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError4_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitLHSUnlessError4_2(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError4_1(pLab, v)){
			return false;
		}

	}
	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError4_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessError4(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessError4Accepting.clear();
	visitedLHSUnlessError4Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true
		&& visitLHSUnlessError4_2(lab, v);
}

bool SCDriver::checkError4(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}
bool SCDriver::visitError5(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessError5_0(const EventLabel *lab) const
{
	auto &g = getGraph();


	return false;


	return true;
}

bool SCDriver::visitLHSUnlessError5_1(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = free_succ(g, lab); pLab)if (true && llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError5_0(pLab)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitLHSUnlessError5_2(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = alloc_pred(g, lab); pLab) {
			if (!visitLHSUnlessError5_1(pLab)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessError5(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessError5Accepting.clear();
	visitedLHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError5Accepting.clear();
	visitedRHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError5_2(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError5Accepting.size(); i++) {
		if (visitedLHSUnlessError5Accepting[i] && !visitedRHSUnlessError5Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool SCDriver::checkError5(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessError5(lab))
		return true;

	return visitError5(lab);
}
bool SCDriver::visitError6(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessError6_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool SCDriver::visitLHSUnlessError6_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	for (auto &tmp : alloc_succs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<MemAccessLabel>(pLab) && llvm::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() && !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(pLab))) {
			if (!visitLHSUnlessError6_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitLHSUnlessError6_2(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (true && llvm::isa<HpRetireLabel>(lab))if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError6_1(pLab, v)){
			return false;
		}

	}
	if (true && llvm::isa<HpRetireLabel>(lab))if (auto pLab = free_pred(g, lab); pLab)if (true && llvm::isa<MemAccessLabel>(pLab) && llvm::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() && !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(pLab))) {
			if (!visitLHSUnlessError6_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessError6(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessError6Accepting.clear();
	visitedLHSUnlessError6Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true
		&& visitLHSUnlessError6_2(lab, v);
}

bool SCDriver::checkError6(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessError6(lab))
		return true;

	return visitError6(lab);
}
bool SCDriver::visitError7(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessError7_0(const EventLabel *lab) const
{
	auto &g = getGraph();


	return false;


	return true;
}

bool SCDriver::visitLHSUnlessError7_1(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (auto pLab = free_succ(g, lab); pLab)if (true && llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError7_0(pLab)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitLHSUnlessError7_2(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (true && llvm::isa<MemAccessLabel>(lab) && llvm::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() && !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(lab)))if (auto pLab = alloc_pred(g, lab); pLab) {
			if (!visitLHSUnlessError7_1(pLab)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessError7(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessError7Accepting.clear();
	visitedLHSUnlessError7Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError7Accepting.clear();
	visitedRHSUnlessError7Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError7_2(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError7Accepting.size(); i++) {
		if (visitedLHSUnlessError7Accepting[i] && !visitedRHSUnlessError7Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool SCDriver::checkError7(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessError7(lab))
		return true;

	return visitError7(lab);
}
bool SCDriver::visitError8(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessError8_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool SCDriver::visitLHSUnlessError8_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (true && lab->isNotAtomic() && llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<WriteLabel>(pLab)) {
			if (!visitLHSUnlessError8_0(pLab, v)){
			return false;
		}

	}
	if (true && lab->isNotAtomic() && llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<ReadLabel>(pLab)) {
			if (!visitLHSUnlessError8_0(pLab, v)){
			return false;
		}

	}
	if (true && lab->isNotAtomic() && llvm::isa<ReadLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<WriteLabel>(pLab)) {
			if (!visitLHSUnlessError8_0(pLab, v)){
			return false;
		}

	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isNotAtomic() && llvm::isa<WriteLabel>(pLab)) {
			if (!visitLHSUnlessError8_0(pLab, v)){
			return false;
		}

	}
	if (true && llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isNotAtomic() && llvm::isa<ReadLabel>(pLab)) {
			if (!visitLHSUnlessError8_0(pLab, v)){
			return false;
		}

	}
	if (true && llvm::isa<ReadLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && pLab->isNotAtomic() && llvm::isa<WriteLabel>(pLab)) {
			if (!visitLHSUnlessError8_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessError8(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessError8Accepting.clear();
	visitedLHSUnlessError8Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true
		&& visitLHSUnlessError8_1(lab, v);
}

bool SCDriver::checkError8(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessError8(lab))
		return true;

	return visitError8(lab);
}
bool SCDriver::visitWarning9(const EventLabel *lab) const
{
	return false;
}

bool SCDriver::visitLHSUnlessWarning9_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool SCDriver::visitLHSUnlessWarning9_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();


	if (true && llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (true && llvm::isa<WriteLabel>(pLab)) {
			if (!visitLHSUnlessWarning9_0(pLab, v)){
			return false;
		}

	}

	return true;
}

bool SCDriver::visitUnlessWarning9(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedLHSUnlessWarning9Accepting.clear();
	visitedLHSUnlessWarning9Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true
		&& visitLHSUnlessWarning9_1(lab, v);
}

bool SCDriver::checkWarning9(const EventLabel *lab) const
{
	auto &g = getGraph();


	if (visitUnlessWarning9(lab))
		return true;

	return visitWarning9(lab);
}
VerificationError SCDriver::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	if (!checkError2(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessNonMalloc;
	}

	if (!checkError3(lab)) {
		race = cexLab;
		return VerificationError::VE_DoubleFree;
	}

	if (!checkError4(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError5(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError6(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError7(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError8(lab)) {
		race = cexLab;
		return VerificationError::VE_RaceNotAtomic;
	}

	return VerificationError::VE_OK;
}

std::vector<VerificationError> SCDriver::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings, std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning9(lab)) {
		racyLabs.push_back(cexLab);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool SCDriver::isConsistent(const EventLabel *lab) const
{

	return true
		&& checkConsAcyclic1(lab);
}

View SCDriver::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = getGraph();
	View pporf;
	pporf.updateIdx(lab->getPos());

	auto *pLab = g.getPreviousLabel(lab);
	if (!pLab)
		return pporf;
	pporf.update(pLab->getPrefixView());
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(pLab))
		pporf.update(rLab->getRf()->getPrefixView());
	if (auto *tsLab = llvm::dyn_cast<ThreadStartLabel>(pLab))
		pporf.update(g.getEventLabel(tsLab->getParentCreate())->getPrefixView());
	if (auto *tjLab = llvm::dyn_cast<ThreadJoinLabel>(pLab))
		pporf.update(g.getLastThreadLabel(tjLab->getChildId())->getPrefixView());
	return pporf;
}
std::unique_ptr<VectorClock> SCDriver::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}
