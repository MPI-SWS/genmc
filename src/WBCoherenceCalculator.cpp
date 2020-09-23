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

#include "WBCoherenceCalculator.hpp"

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
std::vector<unsigned int> WBCoherenceCalculator::calcRMWLimits(const GlobalRelation &wb) const
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
WBCoherenceCalculator::calcWbRestricted(const llvm::GenericValue *addr,
					const VectorClock &v) const
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
		const std::vector<Event> readers = wLab->getReadersList();
		std::copy_if(readers.begin(), readers.end(), std::back_inserter(es),
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

Calculator::GlobalRelation
WBCoherenceCalculator::calcWb(const llvm::GenericValue *addr) const
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

void
WBCoherenceCalculator::trackCoherenceAtLoc(const llvm::GenericValue *addr)
{
	stores_[addr];
}


std::pair<int, int>
WBCoherenceCalculator::getPossiblePlacings(const llvm::GenericValue *addr,
					   Event store, bool isRMW)
{
	auto locMOSize = getStoresToLoc(addr).size();
	return std::make_pair(locMOSize, locMOSize);
}

void WBCoherenceCalculator::addStoreToLoc(const llvm::GenericValue *addr,
					  Event store, int offset)
{
	/* The offset given is ignored */
	stores_[addr].push_back(store);
}

void WBCoherenceCalculator::addStoreToLocAfter(const llvm::GenericValue *addr,
					  Event store, Event pred)
{
	/* Again the offset given is ignored */
	addStoreToLoc(addr, store, 0);
}

bool WBCoherenceCalculator::isCoMaximal(const llvm::GenericValue *addr, Event store)
{
	auto &stores = getStoresToLoc(addr);
	if (stores.empty() && store.isInitializer())
		return true;

	auto wb = calcWb(addr);
	return !store.isInitializer() &&
	       std::none_of(stores.begin(), stores.end(), [&](Event s){ return wb(store, s); });
}


const std::vector<Event>&
WBCoherenceCalculator::getStoresToLoc(const llvm::GenericValue *addr) const
{
	BUG_ON(stores_.count(addr) == 0);
	return stores_.at(addr);
}

/*
 * Checks which of the stores are (rf?;hb)-before some event e, given the
 * hb-before view of e
 */
View WBCoherenceCalculator::getRfOptHbBeforeStores(const std::vector<Event> &stores,
						   const View &hbBefore)
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

void WBCoherenceCalculator::expandMaximalAndMarkOverwritten(const std::vector<Event> &stores,
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
						storeView[w.thread]++;
						break;
					}
				}
			}
		}
	}
	return;
}

bool WBCoherenceCalculator::tryOptimizeWBCalculation(const llvm::GenericValue *addr,
						     Event read,
						     std::vector<Event> &result)
{
	auto &g = getGraph();
	auto &allStores = getStoresToLoc(addr);
	auto &hbBefore = g.getHbBefore(read.prev());
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
		}
	}
	return (count <= 1);
}

bool WBCoherenceCalculator::isCoherentRf(const llvm::GenericValue *addr,
					 const GlobalRelation &wb,
					 Event read, Event store, int storeWbIdx)
{
	auto &g = getGraph();
	auto &stores = wb.getElems();

	/* First, check whether it is wb;rf?;hb-before the read */
	for (auto j = 0u; j < stores.size(); j++) {
		if (wb(storeWbIdx, j) && g.isWriteRfBefore(stores[j], read.prev()))
			return false;
	}

	/* If OOO execution is _not_ supported no need to do extra checks */
	if (!supportsOutOfOrder())
		return true;

	/* For the OOO case, we have to do extra checks */
	for (auto j = 0u; j < stores.size(); j++) {
		if (wb(j, storeWbIdx) && g.isHbOptRfBefore(read, stores[j]))
			return false;
	}

	/* We cannot read from hb-after stores... */
	if (g.getHbBefore(store).contains(read))
		return false;

	/* Also check for violations against the initializer */
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));
			if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
				if (rLab->getRf().isInitializer() &&
				    rLab->getAddr() == addr &&
				    g.getHbBefore(rLab->getPos()).contains(read))
					return false;
		}
	}
	return true;
}

bool WBCoherenceCalculator::isInitCoherentRf(const GlobalRelation &wb,
					     Event read)
{
	auto &g = getGraph();
	auto &stores = wb.getElems();

	for (auto j = 0u; j < stores.size(); j++)
		if (g.isWriteRfBefore(stores[j], read.prev()))
			return false;
	return true;
}

std::vector<Event>
WBCoherenceCalculator::getCoherentStores(const llvm::GenericValue *addr,
					 Event read)
{
	const auto &g = getGraph();
	std::vector<Event> result;

	/*  For the in-order execution case:
	 *
	 * Check whether calculating WB is necessary. As a byproduct, if the
	 * initializer is a valid RF, it is pushed into result */
	if (!supportsOutOfOrder() && tryOptimizeWBCalculation(addr, read, result))
		return result;

	auto wb = calcWb(addr);
	auto &stores = wb.getElems();

	/* Find the stores from which we can read-from */
	for (auto i = 0u; i < stores.size(); i++) {
		if (isCoherentRf(addr, wb, read, stores[i], i))
			result.push_back(stores[i]);
	}

	/* For the OOO execution case:
	 *
	 * We check whether the initializer is a valid RF as well */
	if (supportsOutOfOrder() && isInitCoherentRf(wb, read))
		result.push_back(Event::getInitializer());

	return result;
}

bool WBCoherenceCalculator::isWbMaximal(const WriteLabel *sLab,
					const std::vector<Event> &ls) const
{
	/* Optimization:
	 * Since sLab is a porf-maximal store, unless it is an RMW, it is
	 * wb-maximal (and so, all revisitable loads can read from it).
	 */
	if (!llvm::isa<FaiWriteLabel>(sLab) && !llvm::isa<CasWriteLabel>(sLab))
		return true;

	/* Optimization:
	 * If sLab is maximal in WB, then all revisitable loads can read
	 * from it.
	 */
	if (ls.size() > 1) {
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

bool WBCoherenceCalculator::isCoherentRevisit(const WriteLabel *sLab,
					      Event read) const
{
	auto &g = getGraph();
	const EventLabel *rLab = g.getEventLabel(read);

	BUG_ON(!llvm::isa<ReadLabel>(rLab));
	auto v = g.getRevisitView(static_cast<const ReadLabel *>(rLab), sLab);
	auto wb = calcWbRestricted(sLab->getAddr(), *v);
	auto &stores = wb.getElems();
	auto i = wb.getIndex(sLab->getPos());

	for (auto j = 0u; j < stores.size(); j++) {
		if (wb(i, j) && g.isWriteRfBefore(stores[j], g.getPreviousNonEmptyLabel(read)->getPos())) {
			return false;
		}
	}

	/* If OOO is _not_ supported, no more checks are required */
	if (!supportsOutOfOrder())
		return true;

	for (auto j = 0u; j < stores.size(); j++) {
		if (wb(j, i) && g.isHbOptRfBeforeInView(read, stores[j], *v))
				return false;
	}

	/* Do not revisit hb-before loads... */
	if (g.getHbBefore(sLab->getPos()).contains(read))
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
				    g.getHbBefore(rLab->getPos()).contains(read))
					return false;
		}
	}
	return true;
}

std::vector<Event>
WBCoherenceCalculator::getCoherentRevisits(const WriteLabel *sLab)
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
		if (isCoherentRevisit(sLab, l))
			result.push_back(l);
	}
	return result;
}

std::vector<std::pair<Event, Event> >
WBCoherenceCalculator::saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &labs,
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

void WBCoherenceCalculator::initCalc()
{
	auto &g = getGraph();
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);

	for (auto it = stores_.begin(); it != stores_.end(); ++it)
		coRelation[it->first] = calcWb(it->first);
	return;
}

Calculator::CalculationResult WBCoherenceCalculator::doCalc()
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

void
WBCoherenceCalculator::restorePrefix(const ReadLabel *rLab,
				     const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
				     const std::vector<std::pair<Event, Event> > &status)
{
	auto &g = getGraph();
	for (const auto &lab : storePrefix) {
		if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab.get()))
			addStoreToLoc(wLab->getAddr(), wLab->getPos(), 0);
	}
}

void WBCoherenceCalculator::removeAfter(const VectorClock &preds)
{
	for (auto it = stores_.begin(); it != stores_.end(); ++it)
		it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
						[&](Event &e)
						{ return !preds.contains(e); }),
				 it->second.end());
}
