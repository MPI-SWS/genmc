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

#ifndef __WB_CALCULATOR_HPP__
#define __WB_CALCULATOR_HPP__

#include "CoherenceCalculator.hpp"
#include "ExecutionGraph.hpp"
#include <unordered_map>

/*******************************************************************************
 **                           WBCalculator Class
 ******************************************************************************/

/*
 * An implementation of the CoherenceCalculator API, that tracks coherence
 * by calculating the writes-before relation in the execution graph.
 * Should be used along with the "wb" version of the driver.
 */
class WBCalculator : public CoherenceCalculator {

public:

	/* Constructor */
	WBCalculator(ExecutionGraph &m, bool ooo)
		: CoherenceCalculator(CC_WritesBefore, m, ooo) {}

	/* Track coherence at location addr */
	void
	trackCoherenceAtLoc(const llvm::GenericValue *addr) override;

	/* Returns the range of all the possible (i.e., not violating coherence
	 * places a store can be inserted without inserting it */
	std::pair<int, int>
	getPossiblePlacings(const llvm::GenericValue *addr,
			    Event store, bool isRMW) override;

	/* Tracks a store by inserting it into the appropriate offset */
	void
	addStoreToLoc(const llvm::GenericValue *addr, Event store, int offset) override;
	void
	addStoreToLocAfter(const llvm::GenericValue *addr, Event store, Event pred) override;

	/* Returns whether STORE is maximal in LOC */
	bool isCoMaximal(const llvm::GenericValue *addr, Event store) override;
	/* Returns whether STORE is maximal in LOC */
	bool isCachedCoMaximal(const llvm::GenericValue *addr, Event store) override;

	/* Returns a list of stores to a particular memory location */
	const std::vector<Event>&
	getStoresToLoc(const llvm::GenericValue *addr) const override;

	/* Returns all the stores for which if "read" reads-from, coherence
	 * is not violated */
	std::vector<Event>
	getCoherentStores(const llvm::GenericValue *addr, Event read) override;

	/* Returns all the reads that "wLab" can revisit without violating
	 * coherence */
	std::vector<Event>
	getCoherentRevisits(const WriteLabel *wLab) override;

	/* Saves the coherence status for all write labels in prefix.
	 * This means that for each write we save a predecessor in preds (or within
	 * the prefix itself), which will be present when the prefix is restored. */
	std::vector<std::pair<Event, Event> >
	saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
			    const ReadLabel *rLab) const override;

	/* Calculates WB */
	GlobalRelation calcWb(const llvm::GenericValue *addr) const;

	/* Calculates WB restricted in v */
	GlobalRelation calcWbRestricted(const llvm::GenericValue *addr,
					const VectorClock &v) const;

	/* Populates "wb" so that it represents coherence at loc "addr".
	 * If "prop" is provided, only stores satisfying it are considered.
	 * If "rel" is provided, "rel" is used instead of hb to provide ordering */
	template <typename F>
	Calculator::CalculationResult
	calcWbRelation(const llvm::GenericValue *addr, GlobalRelation &wb,
		       F prop = [](Event e){ return true; });
	template <typename F>
	Calculator::CalculationResult
	calcWbRelation(const llvm::GenericValue *addr, GlobalRelation &wb,
		       const GlobalRelation &rel,
		       F prop = [](Event e){ return true; });


	void initCalc() override;

	Calculator::CalculationResult doCalc() override;

	/* Restores a previously saved coherence status */
	void
	restorePrefix(const ReadLabel *rLab,
		      const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
		      const std::vector<std::pair<Event, Event> > &status) override;

	/* Will remove stores not in preds */
	void removeAfter(const VectorClock &preds) override;

	static bool classof(const CoherenceCalculator *cohTracker) {
		return cohTracker->getKind() == CC_WritesBefore;
	}

private:
	/*
	 * Stores a calculation results so that subsequent queries of particular
	 * kinds will not have to calculate WB again. Currently stores information
	 * about (the most recent) calculations taking place on a single memory location.
	 */
	class Cache {

	public:
		/*
		 * For now, only store whether an internal calculation has been
		 * performed (ignore fixpoints), and whether that calculation
		 * was optimized.
		 */
		enum Kind { Invalid, InternalOpt, InternalCalc };

		Kind getKind() { return k; }

		/*
		 * Caches the fact that E1 and E2 aer maximal;
		 * (assumes that we got this from an optimization)
		 */
		void addMaximalInfo(const std::vector<Event> &es) {
			k = InternalOpt;
			maximals = es;
		}
		void addMaximalInfo(std::vector<Event> &&es) {
			k = InternalOpt;
			maximals = std::move(es);
		}

		/* Caches the calculation for a single memory location */
		void addCalcInfo(GlobalRelation &wb) {
			k = InternalCalc;
			cache = wb;
		}
		void addCalcInfo(GlobalRelation &&wb) {
			k = InternalCalc;
			cache = std::move(wb);
		}

		bool isMaximal(const Event &store) {
			if (getKind() == InternalOpt)
				return std::any_of(maximals.begin(), maximals.end(),
						   [&](Event &m){ return store == m; });
			else if (getKind() == InternalCalc)
				return cache.adj_begin(store) == cache.adj_end(store);
			BUG();
		}

		/* Invalidates the cache */
		void invalidate() { k = Invalid; }

	private:
		Kind k;

		/* When the internal calculation was optimized */
		std::vector<Event> maximals;

		/* When the internal calculation was fully performed */
		GlobalRelation cache;
	};

	Cache &getCache() { return cache_; }

	std::vector<unsigned int> calcRMWLimits(const GlobalRelation &wb) const;

	View getRfOptHbBeforeStores(const std::vector<Event> &stores,
				    const View &hbBefore);
	void expandMaximalAndMarkOverwritten(const std::vector<Event> &stores,
					     View &storeView);

	bool tryOptimizeWBCalculation(const llvm::GenericValue *addr,
				      Event read, std::vector<Event> &result);

	bool isWbMaximal(const WriteLabel *wLab, const std::vector<Event> &ls) const;

	bool isCoherentRf(const llvm::GenericValue *addr,
			  const GlobalRelation &wb, Event read,
			  Event store, int storeWbIdx);
	bool isInitCoherentRf(const GlobalRelation &wb, Event read);

	bool isCoherentRevisit(const WriteLabel *sLab, Event read) const;

	typedef std::unordered_map<const llvm::GenericValue *,
				   std::vector<Event> > StoresList;
	StoresList stores_;

	Cache cache_;
};

template <typename F>
Calculator::CalculationResult
WBCalculator::calcWbRelation(const llvm::GenericValue *addr, GlobalRelation &wb,
			     F prop /* = [](Event e){ return true; } */)
{
	auto &gm = getGraph();
	auto &hbRelation = gm.getGlobalRelation(ExecutionGraph::RelationId::hb);

	return calcWbRelation(addr, wb, hbRelation, prop);
}

template <typename F>
Calculator::CalculationResult
WBCalculator::calcWbRelation(const llvm::GenericValue *addr, GlobalRelation &matrix,
			     const GlobalRelation &rel,
			     F prop /* = [](Event e){ return true; } */)
{
	auto &g = getGraph();

	bool changed = false;
	for (auto locIt = stores_.begin(); locIt != stores_.end(); ++locIt) {
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

			std::vector<Event> es;

			if (prop(wLab->getPos()))
				es.push_back(wLab->getPos());
			auto &readers = wLab->getReadersList();
			std::copy_if(readers.begin(), readers.end(), std::back_inserter(es),
				     [&](const Event &r){ return prop(r); });

			auto upi = upperLimit[i];
			for (auto j = 0u; j < stores.size(); j++) {
				if (// !prop(stores[i]) ||
				    !prop(stores[j]))
					continue;
				if (i == j ||
				    std::none_of(es.begin(), es.end(), [&](Event e)
						 { return g.isWriteRfBeforeRel(rel, stores[j], e, prop); }))
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

#endif /* __WB_CALCULATOR_HPP__ */
