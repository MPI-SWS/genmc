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

	/* Track coherence at location addr */
	void
	trackCoherenceAtLoc(const llvm::GenericValue *addr) override;

	/* Returns the range of all the possible (i.e., not violating coherence
	 * places a store can be inserted without inserting it */
	std::pair<int, int>
	getPossiblePlacings(const llvm::GenericValue *addr,
			    Event store, bool isRMW) override;

	/* Inserts a store in the appropriate offset in coherence.
	 * The offset should have been a valid (non-RMW) placing returned
	 * from getPossiblePlacings() */
	void
	addStoreToLoc(const llvm::GenericValue *addr, Event store, int offset) override;

	/* Inserts "store" after "pred" in coherence order */
	void
	addStoreToLocAfter(const llvm::GenericValue *addr, Event store, Event pred) override;

	/* Returns whether STORE is maximal in LOC */
	bool isCoMaximal(const llvm::GenericValue *addr, Event store) override;
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

	/* Changes the offset of "store" to "newOffset" */
	void changeStoreOffset(const llvm::GenericValue *addr,
			       Event store, int newOffset);

	/* Returns all stores in "addr" that are mo-before "e" */
	std::vector<Event>
	getMOBefore(const llvm::GenericValue *addr, Event e) const;

	/* Returns all stores in "addr" that are mo-after "e" */
	std::vector<Event>
	getMOAfter(const llvm::GenericValue *addr, Event e) const;

	const std::vector<Event> &getModOrderAtLoc(const llvm::GenericValue *addr) const;

	/* Overrided Calculator methods */

	void initCalc() override;

	Calculator::CalculationResult doCalc() override;

	/* Restores a previously saved coherence status */
	void
	restorePrefix(const ReadLabel *rLab,
		      const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
		      const std::vector<std::pair<Event, Event> > &status) override;

	/* Stops tracking all stores not included in "preds" in the graph */
	void removeAfter(const VectorClock &preds) override;

	static bool classof(const CoherenceCalculator *cohTracker) {
		return cohTracker->getKind() == CC_ModificationOrder;
	}

private:

	/* Returns the offset for a particular store */
	int getStoreOffset(const llvm::GenericValue *addr, Event e) const;

	/* Returns the index of the first store that is _not_ (rf?;hb)-before
	 * the event "read". If no such stores exist (i.e., all stores are
	 * concurrent in models that do not support out-of-order execution),
	 * it returns 0. */
	int splitLocMOBefore(const llvm::GenericValue *addr, Event read);

	/* Returns the index of the first store that is hb-after "read",
	 * or the next index of the first store that is read by a read that
	 * is hb-after "read". Returns 0 if the latter condition holds for
	 * the initializer event, and the number of the stores in "addr"
	 * if the conditions do not hold for any store in that location. */
	int splitLocMOAfterHb(const llvm::GenericValue *addr, const Event read);

	/* Similar to splitLocMOAfterHb(), but used for calculating possible MO
	 * placings. This means it does not take into account reads-from the
	 * initializer, and also returns the index (as opposed to index+1) of
	 * the first store that is hb-after "s" or is read by a read that is
	 * hb-after "s" */
	int splitLocMOAfter(const llvm::GenericValue *addr, const Event s);

	/* Returns the events that are mo;rf?-after sLab */
	std::vector<Event> getMOOptRfAfter(const WriteLabel *sLab);

	/* Returns the events that are mo^-1;rf?-after sLab */
	std::vector<Event> getMOInvOptRfAfter(const WriteLabel *sLab);

	/* Returns true if the location "loc" contains the event "e" */
	bool locContains(const llvm::GenericValue *loc, Event e) const;


	typedef std::unordered_map<const llvm::GenericValue *,
				   std::vector<Event> > ModifOrder;
	ModifOrder mo_;
};

#endif /* __MO_CALCULATOR_HPP__ */
