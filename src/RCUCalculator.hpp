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

#ifndef __RCU_CALCULATOR_HPP__
#define __RCU_CALCULATOR_HPP__

#include "Calculator.hpp"
#include "DriverGraphEnumAPI.hpp"
#include "EventLabel.hpp"
#include "Error.hpp"
#include "ExecutionGraph.hpp"

/*******************************************************************************
 **                           RCUCalculator Class
 ******************************************************************************/

/*
 * Calculates LKMM's rcu-order relation. We do not separately calculate rb
 * since it is only used as part of xb in our case. Also, since rcu-link
 * is not seen by any other relations apart from rcu-order, it is stored
 * in a private field of this calculator.
 */
class RCUCalculator : public Calculator {

public:
	RCUCalculator(ExecutionGraph &g) : Calculator(g) {}

	/* Overrided Calculator methods */

	/* Initialize necessary matrices */
	void initCalc() override;

	/* Performs a step of the LB calculation */
	Calculator::CalculationResult doCalc() override;

	/* The calculator is informed about the removal of some events */
	void removeAfter(const VectorClock &preds) override;

	std::unique_ptr<Calculator> clone(ExecutionGraph &g) const override {
		return LLVM_MAKE_UNIQUE<RCUCalculator>(g);
	}

private:
	void incRcuCounter(Event e, unsigned int &gps, unsigned int &css) const;
	void decRcuCounter(Event e, unsigned int &gps, unsigned int &css) const;
	bool checkAddRcuConstraint(Event a, Event b, const unsigned int gps,
				   const unsigned int css);
	bool addRcuConstraints();
};

#endif /* __RCU_CALCULATOR_HPP__ */
