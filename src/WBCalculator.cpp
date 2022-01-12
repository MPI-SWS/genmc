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

#include "WBCalculator.hpp"
#include "LabelIterator.hpp"
#include "WBIterator.hpp"

bool WBCalculator::isLocOrderedRestricted(SAddr addr, const VectorClock &v) const
{
	if (isLocOrdered(addr))
		return true;

	auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);
	auto prev = Event::getInitializer();

	for (const auto &s : stores) {
		if (!v.contains(s))
			continue;
		if (!g.isWriteRfBefore(prev, s))
			return false;
		prev = s;
	}
	return true;
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
std::vector<unsigned int> WBCalculator::calcRMWLimits(const GlobalRelation &wb) const
{
	auto &g = getGraph();
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
		auto *wLab = static_cast<const WriteLabel *>(g.getEventLabel(s[i]));
		if (llvm::isa<FaiWriteLabel>(wLab) || llvm::isa<CasWriteLabel>(wLab)) {
			auto *pLab = static_cast<const ReadLabel *>(
				g.getPreviousLabel(wLab));
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

Calculator::GlobalRelation
WBCalculator::calcWbRestricted(SAddr addr, const VectorClock &v) const
{
	auto &g = getGraph();
	auto &locMO = getStoresToLoc(addr);
	std::vector<Event> storesInView;

	std::copy_if(locMO.begin(), locMO.end(), std::back_inserter(storesInView),
		     [&](const Event &s){ return v.contains(s); });

	GlobalRelation matrix(std::move(storesInView));
	auto &stores = matrix.getElems();

	/* Optimization */
	if (stores.size() <= 1)
		return matrix;

	auto upperLimit = calcRMWLimits(matrix);
	if (upperLimit.empty()) {
		for (auto i = 0u; i < stores.size(); i++)
			matrix.addEdge(i,i);
		return matrix;
	}

	auto lowerLimit = upperLimit.begin() + stores.size();

	for (auto i = 0u; i < stores.size(); i++) {
		auto *wLab = static_cast<const WriteLabel *>(g.getEventLabel(stores[i]));

		std::vector<Event> es;
		std::copy_if(wLab->readers_begin(), wLab->readers_end(), std::back_inserter(es),
			     [&](const Event &r){ return v.contains(r); });

		es.push_back(wLab->getPos());
		auto upi = upperLimit[i];
		for (auto j = 0u; j < stores.size(); j++) {
			if (i == j || std::none_of(es.begin(), es.end(), [&](Event e)
				      { return g.isWriteRfBefore(stores[j], e); }))
				continue;
			matrix.addEdge(j, i);

			if (upi == stores.size() || upi == upperLimit[j])
				continue;
			matrix.addEdge(lowerLimit[j], upi);
		}

		if (lowerLimit[stores.size()] == stores.size() || upi == stores.size())
			continue;
		matrix.addEdge(lowerLimit[stores.size()], i);
	}
	matrix.transClosure();
	return matrix;
}

Calculator::GlobalRelation WBCalculator::calcWb(SAddr addr) const
{
	auto &g = getGraph();
	GlobalRelation matrix(getStoresToLoc(addr));
	auto &stores = matrix.getElems();

	/* Optimization */
	if (stores.size() <= 1)
		return matrix;

	auto upperLimit = calcRMWLimits(matrix);
	if (upperLimit.empty()) {
		for (auto i = 0u; i < stores.size(); i++)
			matrix.addEdge(i, i);
		return matrix;
	}

	auto lowerLimit = upperLimit.begin() + stores.size();

	for (auto i = 0u; i < stores.size(); i++) {
		auto *wLab = static_cast<const WriteLabel *>(g.getEventLabel(stores[i]));
		std::vector<Event> es(wLab->getReadersList());
		es.push_back(wLab->getPos());

		auto upi = upperLimit[i];
		for (auto j = 0u; j < stores.size(); j++) {
			if (i == j || std::none_of(es.begin(), es.end(), [&](Event e)
				      { return g.isWriteRfBefore(stores[j], e); }))
				continue;
			matrix.addEdge(j, i);
			if (upi == stores.size() || upi == upperLimit[j])
				continue;
			matrix.addEdge(lowerLimit[j], upi);
		}

		if (lowerLimit[stores.size()] == stores.size() || upi == stores.size())
			continue;
		matrix.addEdge(lowerLimit[stores.size()], i);
	}
	matrix.transClosure();
	return matrix;
}

const Calculator::GlobalRelation &
WBCalculator::calcCacheWb(SAddr addr)
{
	if (getCache().getKind() != WBCalculator::Cache::InternalCalc)
		getCache().addCalcInfo(calcWb(addr));
	return getCache().getCalcInfo();
}

const Calculator::GlobalRelation &
WBCalculator::calcCacheWbRestricted(SAddr addr, const VectorClock &v)
{
	if (getCache().getKind() != WBCalculator::Cache::InternalCalc)
		getCache().addCalcInfo(calcWbRestricted(addr, v));
	return getCache().getCalcInfo();
}

void WBCalculator::trackCoherenceAtLoc(SAddr addr)
{
	if (!stores_.count(addr))
		setLocOrderedStatus(addr, true);
	stores_[addr];
}


std::pair<int, int>
WBCalculator::getPossiblePlacings(SAddr addr, Event store, bool isRMW)
{
	auto locMOSize = getStoresToLoc(addr).size();
	return std::make_pair(locMOSize, locMOSize);
}

void WBCalculator::addStoreToLoc(SAddr addr, Event store, int offset)
{
	auto &g = getGraph();
	if (isLocOrdered(addr)) {
		auto &stores = getStoresToLoc(addr);
		auto status = stores.empty() || g.isWriteRfBefore(stores.back(), store);
		setLocOrderedStatus(addr, status);
	}

	/* The offset given is ignored */
	stores_[addr].push_back(store);
}

void WBCalculator::addStoreToLocAfter(SAddr addr, Event store, Event pred)
{
	/* Again the offset given is ignored */
	addStoreToLoc(addr, store, 0);
}

bool WBCalculator::isCoMaximal(SAddr addr, Event store)
{
	auto &stores = getStoresToLoc(addr);
	if (store.isInitializer())
		return stores.empty();
	if (isLocOrdered(addr))
		return !stores.empty() && stores.back() == store;

	auto wb = calcWb(addr);
	return wb.adj_begin(store) == wb.adj_end(store);
}

bool WBCalculator::isCachedCoMaximal(SAddr addr, Event store)
{
	auto &stores = getStoresToLoc(addr);
	if (store.isInitializer())
		return stores.empty();

	if (isLocOrdered(addr))
		return !stores.empty() && stores.back() == store;
	return cache_.isMaximal(store);
}

const std::vector<Event>&
WBCalculator::getStoresToLoc(SAddr addr) const
{
	BUG_ON(stores_.count(addr) == 0);
	return stores_.at(addr);
}

/*
 * Checks which of the stores are (rf?;hb)-before some event e, given the
 * hb-before view of e
 */
View WBCalculator::getRfOptHbBeforeStores(const std::vector<Event> &stores, const View &hbBefore)
{
	const auto &g = getGraph();
	View result;

	for (const auto &w : stores) {
		/* Check if w itself is in the hb view */
		if (hbBefore.contains(w)) {
			result.updateIdx(w);
			continue;
		}

		const EventLabel *lab = g.getEventLabel(w);
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);

		/* Check whether [w];rf;[r] is in the hb view, for some r */
		for (const auto &r : wLab->getReadersList()) {
			if (r.thread != w.thread && hbBefore.contains(r)) {
				result.updateIdx(w);
				result.updateIdx(r);
				break;
			}
		}
	}
	return result;
}

void WBCalculator::expandMaximalAndMarkOverwritten(const std::vector<Event> &stores,
						   View &storeView)
{
	const auto &g = getGraph();

	/* Expand view for maximal stores */
	for (const auto &w : stores) {
		/* If the store is not maximal, skip */
		if (w.index != storeView[w.thread])
			continue;

		const EventLabel *lab = g.getEventLabel(w);
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);

		for (const auto &r : wLab->getReadersList()) {
			if (r.thread != w.thread)
				storeView.updateIdx(r);
		}
	}

	/* Check if maximal writes have been overwritten */
	for (const auto &w : stores) {
		/* If the store is not maximal, skip*/
		if (w.index != storeView[w.thread])
			continue;

		const EventLabel *lab = g.getEventLabel(w);
		BUG_ON(!llvm::isa<WriteLabel>(lab));
		auto *wLab = static_cast<const WriteLabel *>(lab);

		for (const auto &r : wLab->getReadersList()) {
			if (r.thread != w.thread && r.index < storeView[r.thread]) {
				const EventLabel *lab = g.getEventLabel(Event(r.thread, storeView[r.thread]));
				if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
					if (rLab->getRf() != w) {
						storeView[w.thread] = std::min(storeView[w.thread] + 1,
									       int(g.getThreadSize(w.thread)) - 1);
						break;
					}
				}
			}
		}
	}
	return;
}

bool WBCalculator::tryOptimizeWBCalculation(SAddr addr,
					    Event read,
					    std::vector<Event> &result)
{
	auto &g = getGraph();
	auto &allStores = getStoresToLoc(addr);
	auto &hbBefore = g.getEventLabel(read.prev())->getHbView();
	auto view = getRfOptHbBeforeStores(allStores, hbBefore);

	/* Can we read from the initializer event? */
	if (std::none_of(view.begin(), view.end(), [](int i){ return i > 0; }))
		result.push_back(Event::getInitializer());

	expandMaximalAndMarkOverwritten(allStores, view);

	int count = 0;
	for (const auto &w : allStores) {
		if (w.index >= view[w.thread]) {
			if (count++ > 0) {
				result.pop_back();
				break;
			}
			result.push_back(w);

			/* Also set the cache */
			BUG_ON(result.size() > 2);
			getCache().addMaximalInfo(result);
		}
	}
	return (count <= 1);
}

bool WBCalculator::hasSuccRfBefore(SAddr addr, Event a, Event b)
{
	auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);

	if (isLocOrdered(addr))
		return std::any_of(wb_to_succ_begin(stores, a), wb_to_succ_end(stores, a),
				   [&](const Event &s){ return g.isWriteRfBefore(s, b); });

	auto &wb = calcCacheWb(addr);
	return std::any_of(wb_po_succ_begin(wb, a), wb_po_succ_end(wb, a),
			   [&](const Event &s){ return g.isWriteRfBefore(s, b); });
}

bool WBCalculator::hasSuccRfBeforeRestricted(SAddr addr, Event a, Event b,
					     const VectorClock &v, bool orderedInView)
{
	auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);

	if (orderedInView)
		return std::any_of(wb_to_succ_begin(stores, a), wb_to_succ_end(stores, a),
				   [&](const Event &s){ return v.contains(s) && g.isWriteRfBefore(s, b); });

	auto &wb = calcCacheWbRestricted(addr, v);
	return std::any_of(wb_po_succ_begin(wb, a), wb_po_succ_end(wb, a),
			   [&](const Event &s){ return g.isWriteRfBefore(s, b); });
}

bool WBCalculator::hasPredHbOptRfAfter(SAddr addr, Event a, Event b)
{
	auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);

	if (isLocOrdered(addr))
		return std::any_of(wb_to_pred_begin(stores, a), wb_to_pred_end(stores, a),
				   [&](const Event &s){ return g.isHbOptRfBefore(b, s); });

	auto &wb = calcCacheWb(addr);
	return std::any_of(wb_po_pred_begin(stores, a), wb_po_pred_end(stores, a),
			   [&](const Event &s){ return g.isHbOptRfBefore(b, s); });
}

bool WBCalculator::hasPredHbOptRfAfterRestricted(SAddr addr, Event a, Event b,
						 const VectorClock &v, bool orderedInView)
{
	auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);

	if (orderedInView)
		return std::any_of(wb_to_pred_begin(stores, a), wb_to_pred_end(stores, a),
				   [&](const Event &s){ return v.contains(s) && g.isHbOptRfBeforeInView(b, s, v); });

	auto &wb = calcCacheWbRestricted(addr, v);
	return std::any_of(wb_po_pred_begin(stores, a), wb_po_pred_end(stores, a),
			   [&](const Event &s){ return g.isHbOptRfBeforeInView(b, s, v); });
}

bool WBCalculator::isCoherentRf(SAddr addr, Event read, Event store)
{
	auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);

	/* First, check whether it is wb;rf?;hb-before the read */
	if (hasSuccRfBefore(addr, store, read.prev()))
		return false;

	/* If OOO execution is _not_ supported no need to do extra checks */
	if (!supportsOutOfOrder())
		return true;

	/* For the OOO case, we have to do extra checks */
	if (hasPredHbOptRfAfter(addr, store, read))
		return false;

	/* We cannot read from hb-after stores... */
	if (g.getEventLabel(store)->getHbView().contains(read))
		return false;

	/* Also check for violations against the initializer */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
				if (rLab->getRf().isInitializer() &&
				    rLab->getAddr() == addr &&
				    g.getEventLabel(rLab->getPos())->getHbView().contains(read))
					return false;
		}
	}
	return true;
}

bool WBCalculator::isInitCoherentRf(SAddr addr, Event read)
{
	auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);

	for (auto j = 0u; j < stores.size(); j++)
		if (g.isWriteRfBefore(stores[j], read.prev()))
			return false;
	return true;
}

std::vector<Event>
WBCalculator::getCoherentStores(SAddr addr, Event read)
{
	const auto &g = getGraph();
	auto &stores = getStoresToLoc(addr);
	std::vector<Event> result;

	/*  For the in-order execution case:
	 *
	 * Check whether calculating WB is necessary. As a byproduct, if the
	 * initializer is a valid RF, it is pushed into result */
	getCache().invalidate();
	if (!supportsOutOfOrder() && tryOptimizeWBCalculation(addr, read, result))
		return result;

	/* Find the stores from which we can read-from */
	for (auto i = 0u; i < stores.size(); i++) {
		if (isCoherentRf(addr, read, stores[i]))
			result.push_back(stores[i]);
	}

	/* Ensure function contract is satisfied */
	if (!isLocOrdered(addr)) {
		auto &wb = calcCacheWb(addr);
		std::sort(result.begin(), result.end(), [&g, &wb](const Event &a, const Event &b){
			if (b.isInitializer())
				return false;
			return a.isInitializer() || wb(a, b) ||
				(!wb(b, a) && g.getEventLabel(a)->getStamp() < g.getEventLabel(b)->getStamp());
		});
	}

	/* For the OOO execution case:
	 *
	 * We check whether the initializer is a valid RF as well */
	if (supportsOutOfOrder() && isInitCoherentRf(addr, read))
		result.insert(result.begin(), Event::getInitializer());

	return result;
}

bool WBCalculator::isWbMaximal(const WriteLabel *sLab, const std::vector<Event> &ls) const
{
	/* Optimization:
	 * Since sLab is a porf-maximal store, unless it is an RMW, it is
	 * wb-maximal (and so, all revisitable loads can read from it).
	 */
	if (!getGraph().isRMWStore(sLab))
		return true;

	/* Optimization:
	 * If sLab is maximal in WB, then all revisitable loads can read
	 * from it.
	 */
	if (ls.size() > 1) {
		if (isLocOrdered(sLab->getAddr()))
			return sLab->getPos() == getStoresToLoc(sLab->getAddr()).back();
		auto wb = calcWb(sLab->getAddr());
		auto i = wb.getIndex(sLab->getPos());
		bool allowed = true;
		for (auto j = 0u; j < wb.getElems().size(); j++)
			if (wb(i, j)) {
				return false;
			}
		return true;
	}
	return false;
}

bool WBCalculator::isCoherentRevisit(const WriteLabel *sLab, Event read)
{
	auto &g = getGraph();
	auto *rLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(read));
	BUG_ON(!rLab);
	auto v = g.getRevisitView(rLab, sLab);

	auto ordered = isLocOrderedRestricted(sLab->getAddr(), *v);
	if (hasSuccRfBeforeRestricted(sLab->getAddr(), sLab->getPos(),
				      g.getPreviousNonEmptyLabel(read)->getPos(), *v, ordered))
		return false;

	/* If OOO is _not_ supported, no more checks are required */
	if (!supportsOutOfOrder())
		return true;

	if (hasPredHbOptRfAfterRestricted(sLab->getAddr(), sLab->getPos(), read, *v, ordered))
		return false;

	/* Do not revisit hb-before loads... */
	if (g.getEventLabel(sLab->getPos())->getHbView().contains(read))
		return false;

	/* Also check for violations against the initializer */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (lab->getPos() == read)
				continue;
			if (!v->contains(lab->getPos()))
				continue;
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
				if (rLab->getRf().isInitializer() &&
				    rLab->getAddr() == sLab->getAddr() &&
				    g.getEventLabel(rLab->getPos())->getHbView().contains(read))
					return false;
		}
	}
	return true;
}

std::vector<Event>
WBCalculator::getCoherentRevisits(const WriteLabel *sLab)
{
	const auto &g = getGraph();
	auto ls = g.getRevisitable(sLab);

	if (!supportsOutOfOrder() && isWbMaximal(sLab, ls))
		return ls;

	std::vector<Event> result;

	/*
	 * We calculate WB again, in order to filter-out inconsistent
	 * revisit options. For example, if sLab is an RMW, we cannot
	 * revisit a read r for which:
	 * \exists c_a in C_a .
	 *         (c_a, r) \in (hb;[\lW_x];\lRF^?;hb;po)
	 *
	 * since this will create a cycle in WB
	 */
	for (auto &l : ls) {
		getCache().invalidate();
		if (isCoherentRevisit(sLab, l))
			result.push_back(l);
	}
	return result;
}

const Calculator::GlobalRelation &
WBCalculator::getOrInsertWbCalc(SAddr addr, const VectorClock &v, Calculator::PerLocRelation &cache)
{
	if (!cache.count(addr))
		cache[addr] = calcWbRestricted(addr, v);
	return cache.at(addr);
}

bool WBCalculator::isCoAfterRemoved(const ReadLabel *rLab, const WriteLabel *sLab,
				    const EventLabel *lab, Calculator::PerLocRelation &wbs)
{
	auto &g = getGraph();
	if (!llvm::isa<WriteLabel>(lab) || g.isRMWStore(lab))
		return false;

	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	BUG_ON(!wLab);

	auto p = g.getPredsView(sLab->getPos());
	if (auto *dv = llvm::dyn_cast<DepView>(&*p))
		dv->addHole(sLab->getPos());
	else
		--(*p)[sLab->getThread()];

	if (isLocOrderedRestricted(wLab->getAddr(), *p)) {
		auto &stores = getStoresToLoc(wLab->getAddr());
		return std::any_of(wb_to_pred_begin(stores, wLab->getPos()),
				   wb_to_pred_end(stores, wLab->getPos()), [&](const Event &s){
					   auto *slab = g.getEventLabel(s);
					   return g.revisitDeletesEvent(rLab, sLab, slab) &&
						   s.index > wLab->getPPoRfView()[s.thread] && /* no holes */
						   slab->getStamp() < wLab->getStamp() &&
						   !(g.isRMWStore(slab) && slab->getPos().prev() == rLab->getPos());
				   });
	}

	auto &wb = getOrInsertWbCalc(wLab->getAddr(), *p, wbs);
	return std::any_of(wb_po_pred_begin(wb, wLab->getPos()),
			   wb_po_pred_end(wb, wLab->getPos()), [&](const Event &s){
				   auto *slab = g.getEventLabel(s);
				   return g.revisitDeletesEvent(rLab, sLab, slab) &&
					   s.index > wLab->getPPoRfView()[s.thread] && /* no holes */
					   slab->getStamp() < wLab->getStamp() &&
					   !(g.isRMWStore(slab) && slab->getPos().prev() == rLab->getPos());
			   });
}

bool WBCalculator::isRbBeforeSavedPrefix(const ReadLabel *revLab, const WriteLabel *wLab,
					   const EventLabel *lab, Calculator::PerLocRelation &wbs)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
	if (!rLab)
		return false;

	auto &g = getGraph();
        auto &v = g.getPrefixView(wLab->getPos());
	auto p = g.getPredsView(wLab->getPos());

	if (auto *dv = llvm::dyn_cast<DepView>(&*p))
		dv->addHole(wLab->getPos());
	else
		--(*p)[wLab->getThread()];

	if (isLocOrderedRestricted(rLab->getAddr(), *p)) {
		auto &stores = getStoresToLoc(rLab->getAddr());
		return std::any_of(wb_to_succ_begin(stores, rLab->getRf()),
				   wb_to_succ_end(stores, rLab->getRf()), [&](const Event &s){
					   auto *sLab = g.getEventLabel(s);
					   return v.contains(sLab->getPos()) &&
						   rLab->getIndex() > sLab->getPPoRfView()[rLab->getThread()] &&
						   sLab->getPos() != wLab->getPos() &&
						   sLab->getStamp() > revLab->getStamp();
				   });
	}

	auto &wb = getOrInsertWbCalc(rLab->getAddr(), *p, wbs);
	return std::any_of(wb_po_succ_begin(wb, rLab->getRf()),
			   wb_po_succ_end(wb, rLab->getRf()), [&](const Event &s){
				   auto *sLab = g.getEventLabel(s);
				   return v.contains(sLab->getPos()) &&
					   rLab->getIndex() > sLab->getPPoRfView()[rLab->getThread()] &&
					   sLab->getPos() != wLab->getPos() &&
					   sLab->getStamp() > revLab->getStamp();
			   });
}

bool WBCalculator::coherenceSuccRemainInGraph(const ReadLabel *rLab, const WriteLabel *wLab)
{
	auto &g = getGraph();
	if (g.isRMWStore(wLab))
		return true;

	if (isLocOrdered(rLab->getAddr())) {
		auto &stores = getStoresToLoc(rLab->getAddr());
		return wb_to_succ_begin(stores, wLab->getPos()) == wb_to_succ_end(stores, wLab->getPos());
	}

	auto wb = calcWb(rLab->getAddr());
	auto &stores = wb.getElems();

	/* Find the "immediate" successor of wLab */
	std::vector<Event> succs;
	for (auto it = wb.adj_begin(wLab->getPos()), ie = wb.adj_end(wLab->getPos()); it != ie; ++it)
		succs.push_back(stores[*it]);
	if (succs.empty())
		return true;

	std::sort(succs.begin(), succs.end(), [&g](const Event &a, const Event &b)
		{ return g.getEventLabel(a)->getStamp() < g.getEventLabel(b)->getStamp(); });
	auto *sLab = g.getEventLabel(succs[0]);
	return sLab->getStamp() <= rLab->getStamp() || g.getPrefixView(wLab->getPos()).contains(sLab->getPos());
}

Event WBCalculator::getOrInsertWbMaximal(const ReadLabel *lab, const VectorClock &v,
					 std::unordered_map<SAddr, Event> &cache)
{
	if (cache.count(lab->getAddr()))
		return cache.at(lab->getAddr());

	auto &g = getGraph();
	if (isLocOrderedRestricted(lab->getAddr(), v)) {
		auto &stores = getStoresToLoc(lab->getAddr());
		auto maxIt = std::find_if(stores.rbegin(), stores.rend(), [&](const Event &s){
			return v.contains(s);
		});
		return cache[lab->getAddr()] = (maxIt == stores.rend() ? Event::getInitializer() : *maxIt);
	}

	auto wb = calcWbRestricted(lab->getAddr(), v);
	auto &stores = wb.getElems();

	if (stores.empty())
		return cache[lab->getAddr()] = Event::getInitializer();

	std::vector<Event> maximals;
	for (auto &s : stores) {
		if (wb.adj_begin(s) == wb.adj_end(s))
			maximals.push_back(s);
	}

	std::sort(maximals.begin(), maximals.end(), [&g, &v](const Event &a, const Event &b)
		{ return g.getEventLabel(a)->getStamp() < g.getEventLabel(b)->getStamp(); });
	return cache[lab->getAddr()] = maximals.back();
}

Event WBCalculator::getTiebraker(const ReadLabel *rLab, const WriteLabel *wLab, const ReadLabel *lab) const
{
	auto &g = getGraph();
	auto &locMO = getStoresToLoc(lab->getAddr());

	auto *tbLab = g.getEventLabel(lab->getRf());
	for (const auto &s : locMO) {
		auto *sLab = g.getEventLabel(s);
		if (sLab->getStamp() > tbLab->getStamp() &&
		    g.revisitDeletesEvent(rLab, wLab, sLab) &&
		    sLab->getStamp() < lab->getStamp() &&
		    (!g.hasBAM() || !llvm::isa<BIncFaiWriteLabel>(sLab)))
			tbLab = sLab;
	}
	return tbLab->getPos();
}

bool WBCalculator::ignoresDeletedStore(const ReadLabel *rLab, const WriteLabel *wLab, const ReadLabel *lab) const
{
	auto &g = getGraph();
	auto &locMO = getStoresToLoc(lab->getAddr());

	return std::any_of(locMO.begin(), locMO.end(), [&](const Event &s){
		auto *sLab = g.getEventLabel(s);
		return g.revisitDeletesEvent(rLab, wLab, sLab) && sLab->getStamp() < lab->getStamp();
	});
}

/*
 * A store is deemed "maximal" if
 *     (1) it is contained in V
 *     (2) it is not porf-after LAB, and
 *     (3) all of its wb-successors are porf-after LAB
 */
Event WBCalculator::getMaximalOOO(const ReadLabel *rLab, const WriteLabel *wLab, const ReadLabel *lab)
{
	auto &g = getGraph();

	/* It is very important to call getRevisitView(): that way we
	 * take into account some internal rfis that are forced and
	 * that are crucial when calculating wb.
	 *
	 * Take viktor-relseq as an example and the (3,1) <-- (1,4) revisit:
	 *
	 * Rx3   Rx0   Rx4   Rx1
	 * Wx4   Wx1   Wx5   Wx2
	 * Rx4   Rx2
	 * Wx5   Wx3
	 *
	 * Here, the first thread is solely composed of rlx accesses,
	 * and thus the Rx3 Wx4 sequence is not a part of (1,4)'s pporf
	 * view. This creates an issue when trying to revisit (3,1),
	 * as the wb calculation is different. */
	auto p = g.getRevisitView(lab, wLab); /* as if wLab revisits lab */
	if (auto *dv = llvm::dyn_cast<DepView>(&*p)) {
		dv->addHole(lab->getPos());
		dv->addHole(wLab->getPos());
	} else
		BUG();

	/* CAUTION: In the case where the location is ordered, we take
	 * special care for locks: it may well be the case that a lock
	 * reads from an unlock that will be deleted and not part of
	 * WLAB's prefix. In this case, we _want_ the revisit to happen
	 * even though ULAB is not in the cut graph, because the execution
	 * where the lock reads from the cut graph will be deemed moot. */
	if (isLocOrderedRestricted(lab->getAddr(), *p)) {
		auto &stores = getStoresToLoc(lab->getAddr());
		auto maxIt = std::find_if(stores.rbegin(), stores.rend(), [&](const Event &s){
			auto *sLab = g.getEventLabel(s);
			if (s == wLab->getPos() || /* separate case due to matching locks */
			    !(p->contains(s) || g.prefixContainsMatchingLock(sLab, wLab)) ||
			    lab->getIndex() <= sLab->getPPoRfView()[lab->getThread()])
				return false;
			return std::all_of(wb_to_succ_begin(stores, s), wb_to_succ_end(stores, s),
					   [&](const Event &ss){
						   auto *ssLab = g.getEventLabel(ss);
						   return ss == wLab->getPos() ||
							   !(p->contains(ss) ||
							     g.prefixContainsMatchingLock(ssLab, wLab)) ||
							   lab->getIndex() <= ssLab->getPPoRfView()[lab->getThread()];
					   });
		});
		return maxIt == stores.rend() ? Event::getInitializer() : *maxIt;
	}

	auto wb = calcWbRestricted(lab->getAddr(), *p);
	auto &stores = wb.getElems();

	std::vector<Event> maximals;
	for (auto &s : stores) {
		if (lab->getIndex() <= g.getEventLabel(s)->getPPoRfView()[lab->getThread()])
			continue;
		if (std::all_of(wb_po_succ_begin(wb, s), wb_po_succ_end(wb, s), [&](const Event &ss){
			return lab->getIndex() <= g.getEventLabel(ss)->getPPoRfView()[lab->getThread()];
		}))
			maximals.push_back(s);
	}

	std::sort(maximals.begin(), maximals.end(), [&g, &p](const Event &a, const Event &b){
		return (p->contains(a) && !p->contains(b)) ||
			(!(p->contains(b) && !p->contains(a)) &&
			 g.getEventLabel(a)->getStamp() < g.getEventLabel(b)->getStamp());
	});
	return (maximals.empty() ? Event::getInitializer() : maximals.back());
}

bool WBCalculator::wasAddedMaximally(const ReadLabel *rLab, const WriteLabel *wLab,
				     const EventLabel *eLab, std::unordered_map<SAddr, Event> &cache)
{
	auto &g = getGraph();
	auto *lab = llvm::dyn_cast<ReadLabel>(eLab);
	if (!lab || (g.hasBAM() && llvm::isa<BWaitReadLabel>(eLab)))
		return true;

	/* Handle the OOO case specially... */
	if (supportsOutOfOrder())
		return lab->getRf() == getMaximalOOO(rLab, wLab, lab);

	auto p = g.getRevisitView(rLab, wLab);

	/* If it reads from the deleted events, it must be reading
	 * from the deleted event added latest before it */
	if (!p->contains(lab->getRf()))
		return lab->getRf() == getTiebraker(rLab, wLab, lab);

	/* If there is a store that will not remain in the graph (but
	 * added after RLAB), LAB should be reading from there */
	if (ignoresDeletedStore(rLab, wLab, lab))
		return false;

	/* Otherwise, it needs to be reading from the maximal write in
	 * the remaining graph */
	if (auto *dv = llvm::dyn_cast<DepView>(&*p)) {
		dv->addHole(rLab->getPos());
		dv->addHole(wLab->getPos());
	} else {
		--(*p)[rLab->getThread()];
		--(*p)[wLab->getThread()];
	}
	return lab->getRf() == getOrInsertWbMaximal(lab, *p, cache);
}

bool WBCalculator::inMaximalPath(const ReadLabel *rLab, const WriteLabel *wLab)
{
	if (!coherenceSuccRemainInGraph(rLab, wLab))
		return false;

	auto &g = getGraph();
        auto &v = g.getPrefixView(wLab->getPos());
	Calculator::PerLocRelation wbs;
	std::unordered_map<SAddr, Event> initMaximals;

	for (const auto *lab : labels(g)) {
		if (lab->getStamp() < rLab->getStamp())
			continue;
		if (v.contains(lab->getPos()) || g.prefixContainsSameLoc(rLab, wLab, lab) ||
		    g.isOptBlockedLock(lab))
			continue;

		if (isRbBeforeSavedPrefix(rLab, wLab, lab, wbs))
			return false;
		if (g.hasBeenRevisitedByDeleted(rLab, wLab, lab))
			return false;
		if (!wasAddedMaximally(rLab, wLab, lab, initMaximals))
			return false;
	}
	return true;
}

#ifdef ENABLE_GENMC_DEBUG
std::vector<std::pair<Event, Event> >
WBCalculator::saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &labs,
				  const ReadLabel *rLab) const
{
	std::vector<std::pair<Event, Event> > pairs;

	for (const auto &lab : labs) {
		/* Only store MO pairs for write labels */
		if (!llvm::isa<WriteLabel>(lab.get()))
			continue;

		auto *wLab = static_cast<const WriteLabel *>(lab.get());
		auto &locMO = getStoresToLoc(wLab->getAddr());
		auto moPos = std::find(locMO.begin(), locMO.end(), wLab->getPos());

		/* We are not actually saving anything, but we do need to make sure
		 * that the store is in this location's MO */
		BUG_ON(moPos == locMO.end());
	}
	return pairs;
}
#endif

void WBCalculator::initCalc()
{
	auto &g = getGraph();
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);

	coRelation.clear(); /* in case this is called directly (e.g., from PersChecker) */
	for (auto it = stores_.begin(); it != stores_.end(); ++it)
		coRelation[it->first] = calcWb(it->first);
	return;
}

Calculator::CalculationResult WBCalculator::doCalc()
{
	auto &g = getGraph();
	auto &hbRelation = g.getGlobalRelation(ExecutionGraph::RelationId::hb);
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);

	bool changed = false;
	for (auto locIt = stores_.begin(); locIt != stores_.end(); ++locIt) {
		auto &matrix = coRelation[locIt->first];
		auto &stores = matrix.getElems();

		/* If it is empty, nothing to do */
		if (stores.empty())
			continue;

		auto upperLimit = calcRMWLimits(matrix);
		if (upperLimit.empty()) {
			for (auto i = 0u; i < stores.size(); i++)
				matrix.addEdge(i, i);
			return Calculator::CalculationResult(true, false);
		}

		auto lowerLimit = upperLimit.begin() + stores.size();
		for (auto i = 0u; i < stores.size(); i++) {
			auto *wLab = static_cast<const WriteLabel *>(g.getEventLabel(stores[i]));
			std::vector<Event> es(wLab->getReadersList());
			es.push_back(wLab->getPos());

			auto upi = upperLimit[i];
			for (auto j = 0u; j < stores.size(); j++) {
				if (i == j ||
				    std::none_of(es.begin(), es.end(), [&](Event e)
						 { return g.isWriteRfBeforeRel(hbRelation, stores[j], e); }))
					continue;

				if (!matrix(j, i)) {
					changed = true;
					matrix.addEdge(j, i);
				}
				if (upi == stores.size() || upi == upperLimit[j])
					continue;

				if (!matrix(lowerLimit[j], upi)) {
					matrix.addEdge(lowerLimit[j], upi);
					changed = true;
				}
			}

			if (lowerLimit[stores.size()] == stores.size() || upi == stores.size())
				continue;

			if (!matrix(lowerLimit[stores.size()], i)) {
				matrix.addEdge(lowerLimit[stores.size()], i);
				changed = true;
			}
		}
		matrix.transClosure();

		/* Check for consistency */
		if (!matrix.isIrreflexive())
			return Calculator::CalculationResult(changed, false);

	}
	return CalculationResult(changed, true);
}

void WBCalculator::removeAfter(const VectorClock &preds)
{
	auto &g = getGraph();
	VSet<SAddr> keep;

	/* Check which locations should be kept */
	for (auto i = 0u; i < preds.size(); i++) {
		for (auto j = 0u; j <= preds[i]; j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab))
				keep.insert(mLab->getAddr());
		}
	}

	for (auto it = stores_.begin(); it != stores_.end(); /* empty */) {
		it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
						[&](Event &e)
						{ return !preds.contains(e); }),
				 it->second.end());

		/* Should we keep this memory location lying around? */
		if (!keep.count(it->first)) {
			BUG_ON(!it->second.empty());
			ordered_.erase(it->first);
			it = stores_.erase(it);
		} else {
			++it;
		}
	}
}
