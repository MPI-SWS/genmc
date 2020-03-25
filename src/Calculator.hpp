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

#ifndef __CALCULATOR_HPP__
#define __CALCULATOR_HPP__

#include "AdjList.hpp"
#include "EventLabel.hpp"
#include "VectorClock.hpp"
#include <vector>
#include <unordered_map>

class ExecutionGraph;

/*******************************************************************************
 **                        Calculator Class (Abstract)
 ******************************************************************************/

/*
 * An abstract class for modeling calculations that take place on the
 * execution graph as part of a fixpoint calculation.
 */
class Calculator {

public:
	/* Represents the result of the performed calculation */
	struct CalculationResult {
		/* Whether any new information were added */
		bool changed;
		/* Whether the respective relation is consistent */
		bool cons;

		CalculationResult() : changed(false), cons(true) {}
		CalculationResult(bool cd, bool cs) : changed(cd), cons(cs) {}

		inline CalculationResult& operator|=(const CalculationResult &o) {
			changed |= o.changed;
			cons &= o.cons;
			return *this;
		};
	};

	/* Represents the structure on which calculations take place */
	using GlobalRelation = AdjList<Event, EventHasher>;
	using PerLocRelation = std::unordered_map<const llvm::GenericValue *,
						  GlobalRelation >;

	/* Constructor */
	Calculator(ExecutionGraph &g) : execGraph(g) {}

	virtual ~Calculator() {}

	ExecutionGraph &getGraph() { return execGraph; }
	ExecutionGraph &getGraph() const { return execGraph; }

	/* Should perform the necessary initializations for the calculation */
	virtual void initCalc() = 0;

	/* Should perform one round of the calculation of the
	 * respective relation */
	virtual CalculationResult doCalc() = 0;

	/* The calculator is informed about the removal of some events */
	virtual void removeAfter(const VectorClock &preds) = 0;

	/* The calculator is informed about the restoration of some events */
	virtual void
	restorePrefix(const ReadLabel *rLab,
		      const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
		      const std::vector<std::pair<Event, Event> > &status) = 0;

private:
	/* A reference to the graph managing object */
	ExecutionGraph &execGraph;
};

#endif /* __CALCULATOR_HPP__ */
