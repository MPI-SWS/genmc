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

#ifndef __COHERENCE_CALCULATOR_HPP__
#define __COHERENCE_CALCULATOR_HPP__

#include "Calculator.hpp"
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

class ExecutionGraph;

/*******************************************************************************
 **                      CoherenceCalculator Class (Abstract)
 ******************************************************************************/

/*
 * An abstract class for modeling the different ways we can track coherence
 * (e.g, with the usage of modification orders, or by calculating WB, etc).
 * Defines the minimal interface such implementations should offer.
 */
class CoherenceCalculator : public Calculator {

public:
	/* Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * Used to enable the Driver to perform calculator-specific
	 * actions in the algorithm if necessary. */
	enum CoherenceCalculatorKind {
		CC_ModificationOrder,
		CC_WritesBefore
	};

protected:

	/* Constructor */
	CoherenceCalculator(CoherenceCalculatorKind k, ExecutionGraph &m, bool ooo)
		: Calculator(m), kind(k), outOfOrder(ooo) {}

	/* Returns whether the model we are operating under supports
	 * out-of-order execution */
	bool supportsOutOfOrder() const { return outOfOrder; };

public:
	/* Returns the discriminator of this object */
	CoherenceCalculatorKind getKind() const { return kind; }

	virtual ~CoherenceCalculator() = default;

	/* Track coherence at location addr */
	virtual void
	trackCoherenceAtLoc(const llvm::GenericValue *addr) = 0;

	/* Returns the range of all the possible (i.e., not violating coherence
	 * places a store can be inserted without inserting it */
	virtual std::pair<int, int>
	getPossiblePlacings(const llvm::GenericValue *addr,
			    Event store, bool isRMW) = 0;

	/* Tracks a store by inserting it into the appropriate offset */
	virtual void
	addStoreToLoc(const llvm::GenericValue *addr, Event store, int offset) = 0;
	virtual void
	addStoreToLocAfter(const llvm::GenericValue *addr, Event store, Event pred) = 0;

	/* Returns whether STORE is maximal in LOC */
	virtual bool
	isCoMaximal(const llvm::GenericValue *addr, Event store) = 0;

	/* Returns whether STORE is maximal in LOC.
	 * Pre: Cached information for this location exist. */
	virtual bool
	isCachedCoMaximal(const llvm::GenericValue *addr, Event store) = 0;

	/* Returns a list of stores to a particular memory location */
	virtual const std::vector<Event>&
	getStoresToLoc(const llvm::GenericValue *addr) const = 0;

	/* Returns all the stores for which if "read" reads-from, coherence
	 * is not violated */
	virtual std::vector<Event>
	getCoherentStores(const llvm::GenericValue *addr, Event read) = 0;

	/* Returns all the reads that "wLab" can revisit without violating
	 * coherence */
	virtual std::vector<Event>
	getCoherentRevisits(const WriteLabel *wLab) = 0;

	/* Saves the coherence status for all write labels in prefix.
	 * This means that for each write we save a predecessor in preds (or within
	 * the prefix itself), which will be present when the prefix is restored. */
	virtual std::vector<std::pair<Event, Event> >
	saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
			    const ReadLabel *rLab) const = 0;

private:

	/* Discriminator enum for LLVM-style RTTI */
	const CoherenceCalculatorKind kind;

	/* Whether the model which we are operating under supports out-of-order
	 * execution. This enables some extra optimizations, in certain cases. */
	bool outOfOrder;
};

#endif /* __COHERENCE_CALCULATOR_HPP__ */
