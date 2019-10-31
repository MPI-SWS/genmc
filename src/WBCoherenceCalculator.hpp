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

#ifndef __WB_COHERENCE_CALCULATOR_HPP__
#define __WB_COHERENCE_CALCULATOR_HPP__

#include "CoherenceCalculator.hpp"
#include <unordered_map>

/*******************************************************************************
 **                    WBCoherenceCalculator Class
 ******************************************************************************/

/*
 * An implementation of the CoherenceCalculator API, that tracks coherence
 * by calculating the writes-before relation in the execution graph.
 * Should be used along with the "wb" version of the driver.
 */
class WBCoherenceCalculator : public CoherenceCalculator {

public:

	/* Constructor */
	WBCoherenceCalculator(ExecutionGraph &g, bool ooo)
		: CoherenceCalculator(CC_WritesBefore, g, ooo) {}

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

	/* Restores a previously saved coherence status */
	void
	restoreCoherenceStatus(std::vector<std::pair<Event, Event> > &status) override;

	void removeStoresAfter(VectorClock &preds) override;

	/* Calculates WB */
	Matrix2D<Event> calcWb(const llvm::GenericValue *addr) const;

	/* Calculates WB restricted in v */
	Matrix2D<Event> calcWbRestricted(const llvm::GenericValue *addr,
					 const VectorClock &v) const;

	static bool classof(const CoherenceCalculator *cohTracker) {
		return cohTracker->getKind() == CC_WritesBefore;
	}

private:
	std::vector<unsigned int> calcRMWLimits(const Matrix2D<Event> &wb) const;

	View getRfOptHbBeforeStores(const std::vector<Event> &stores,
				    const View &hbBefore);
	void expandMaximalAndMarkOverwritten(const std::vector<Event> &stores,
					     View &storeView);

	bool tryOptimizeWBCalculation(const llvm::GenericValue *addr,
				      Event read, std::vector<Event> &result);

	bool isWbMaximal(const WriteLabel *wLab, const std::vector<Event> &ls) const;

	bool isCoherentRf(const llvm::GenericValue *addr,
			  const Matrix2D<Event> &wb, Event read,
			  Event store, int storeWbIdx);
	bool isInitCoherentRf(const Matrix2D<Event> &wb, Event read);

	bool isCoherentRevisit(const WriteLabel *sLab, Event read) const;



	typedef std::unordered_map<const llvm::GenericValue *,
				   std::vector<Event> > StoresList;
	StoresList stores_;
};

#endif /* __COHERENCE_CALCULATOR_HPP__ */
