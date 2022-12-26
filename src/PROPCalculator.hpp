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

#ifndef __PROP_CALCULATOR_HPP__
#define __PROP_CALCULATOR_HPP__

#include "Calculator.hpp"
#include "DriverGraphEnumAPI.hpp"
#include "EventLabel.hpp"
#include "Error.hpp"
#include "ExecutionGraph.hpp"

class PROPCalculator : public Calculator {

public:
	PROPCalculator(ExecutionGraph &g) : Calculator(g) {}

	/* Returns true if an event should be considered for prop */
	static bool isNonTrivial(const EventLabel *lab) {
		return (llvm::isa<MemAccessLabel>(lab) ||
			llvm::isa<LockLabelLAPOR>(lab) ||
			llvm::isa<UnlockLabelLAPOR>(lab));
	}

	/* Overrided Calculator methods */

	/* Initialize necessary matrices */
	void initCalc() override;

	/* Performs a step of the LB calculation */
	Calculator::CalculationResult doCalc() override;

	/* The calculator is informed about the removal of some events */
	void removeAfter(const VectorClock &preds) override;

	std::unique_ptr<Calculator> clone(ExecutionGraph &g) const override {
		return std::make_unique<PROPCalculator>(g);
	}

	const std::vector<Event> &getCumulFences() const { return cumulFences; }
	const std::vector<Event> &getStrongFences() const { return strongFences; }

private:
	bool isCumulFenceBefore(Event a, Event b) const;
	bool isPoUnlRfLockPoBefore(Event a, Event b) const;
	std::vector<Event> getExtOverwrites(Event e) const;
	bool addConstraint(Event a, Event b);
	bool addPropConstraints();

	/* All cumulative fences currently in the graph */
	std::vector<Event> cumulFences;

	/* All strong fences currently in the graph */
	std::vector<Event> strongFences;

	/* All locks currently in the graph */
	std::vector<Event> locks;
};

#endif /* __PROP_CALCULATOR_HPP__ */
