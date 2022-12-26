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

#ifndef __MO_CALCULATOR_HPP__
#define __MO_CALCULATOR_HPP__

#include "CoherenceCalculator.hpp"
#include <unordered_map>

/*******************************************************************************
 **                        MOCalculator Class
 ******************************************************************************/

/*
 * An implementation of the CoherenceCalculator API, that tracks coherence
 * by recording the modification order. Should be used along with the "mo"
 * version of the driver.
 */
class MOCalculator : public CoherenceCalculator {

public:
	/* Constructor */
	MOCalculator(ExecutionGraph &g, bool ooo)
		: CoherenceCalculator(CC_ModificationOrder, g, ooo) {}

	/* Iterates over the successors of STORE.
	 * Pre: STORE needs to be in mo_ADDR */
	CoherenceCalculator::const_store_iterator
	succ_begin(SAddr addr, Event store) const;
	CoherenceCalculator::const_store_iterator
	succ_end(SAddr addr, Event store) const;

	/* Iterates over the predecessors of STORE, excluding the
	 * initializer.
	 * Pre: STORE needs to be in mo_ADDR */
	CoherenceCalculator::const_store_iterator
	pred_begin(SAddr addr, Event store) const;
	CoherenceCalculator::const_store_iterator
	pred_end(SAddr addr, Event store) const;

	/* Changes the offset of "store" to "newOffset" */
	void changeStoreOffset(SAddr addr, Event store, int newOffset);

	/* Track coherence at location addr */
	void
	trackCoherenceAtLoc(SAddr addr) override;

	/* Returns the range of all the possible (i.e., not violating coherence
	 * places a store can be inserted without inserting it */
	std::pair<int, int>
	getPossiblePlacings(SAddr addr, Event store, bool isRMW) override;

	/* Inserts a store in the appropriate offset in coherence.
	 * The offset should have been a valid (non-RMW) placing returned
	 * from getPossiblePlacings() */
	void
	addStoreToLoc(SAddr addr, Event store, int offset) override;

	/* Inserts "store" after "pred" in coherence order */
	void
	addStoreToLocAfter(SAddr addr, Event store, Event pred) override;

	/* Returns whether STORE is maximal in LOC */
	bool isCoMaximal(SAddr addr, Event store) override;
	bool isCachedCoMaximal(SAddr addr, Event store) override;

	/* Returns all the stores for which if "read" reads-from, coherence
	 * is not violated */
	std::vector<Event>
	getCoherentStores(SAddr addr, Event read) override;

	/* Returns all the reads that "wLab" can revisit without violating
	 * coherence */
	std::vector<Event>
	getCoherentRevisits(const WriteLabel *wLab) override;

	bool inMaximalPath(const BackwardRevisit &r) override;

	/* Overrided Calculator methods */

	void initCalc() override;

	Calculator::CalculationResult doCalc() override;

	/* Stops tracking all stores not included in "preds" in the graph */
	void removeAfter(const VectorClock &preds) override;

#ifdef ENABLE_GENMC_DEBUG
	/* Saves the coherence status for all write labels in prefix.
	 * This means that for each write we save a predecessor in preds (or within
	 * the prefix itself), which will be present when the prefix is restored. */
	std::vector<std::pair<Event, Event> >
	saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
			    const ReadLabel *rLab) const override;
#endif

	/* FIXME: When copying coherence calcs, OOO should be decided based on G */
	std::unique_ptr<Calculator> clone(ExecutionGraph &g) const override {
		return std::make_unique<MOCalculator>(g, outOfOrder);
	}

	static bool classof(const CoherenceCalculator *cohTracker) {
		return cohTracker->getKind() == CC_ModificationOrder;
	}

private:
	/* Returns the offset for a particular store */
	int getStoreOffset(SAddr addr, Event e) const;

	/* Returns the index of the first store that is _not_ (rf?;hb)-before
	 * the event "read". If no such stores exist (i.e., all stores are
	 * concurrent in models that do not support out-of-order execution),
	 * it returns 0. */
	int splitLocMOBefore(SAddr addr, Event read);

	/* Returns the index of the first store that is hb-after "read",
	 * or the next index of the first store that is read by a read that
	 * is hb-after "read". Returns 0 if the latter condition holds for
	 * the initializer event, and the number of the stores in "addr"
	 * if the conditions do not hold for any store in that location. */
	int splitLocMOAfterHb(SAddr addr, const Event read);

	/* Similar to splitLocMOAfterHb(), but used for calculating possible MO
	 * placings. This means it does not take into account reads-from the
	 * initializer, and also returns the index (as opposed to index+1) of
	 * the first store that is hb-after "s" or is read by a read that is
	 * hb-after "s" */
	int splitLocMOAfter(SAddr addr, const Event s);

	/* Returns the events that are mo;rf?-after sLab */
	std::vector<Event> getMOOptRfAfter(const WriteLabel *sLab);

	/* Returns the events that are mo^-1;rf?-after sLab */
	std::vector<Event> getMOInvOptRfAfter(const WriteLabel *sLab);

	bool coherenceSuccRemainInGraph(const BackwardRevisit &r);

	bool wasAddedMaximally(const EventLabel *lab);

	/* Returns true if LAB is co-before any event that would be part
	 * of the saved prefix triggered by the revisit R */
	bool isCoBeforeSavedPrefix(const BackwardRevisit &r, const EventLabel *lab);

	/* Returns true if the location "loc" contains the event "e" */
	bool locContains(SAddr loc, Event e) const;
};

#endif /* __MO_CALCULATOR_HPP__ */
