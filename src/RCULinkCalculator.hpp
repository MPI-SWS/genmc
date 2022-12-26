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

#ifndef __RCU_LINK_CALCULATOR_HPP__
#define __RCU_LINK_CALCULATOR_HPP__

#include "Calculator.hpp"
#include "DriverGraphEnumAPI.hpp"
#include "EventLabel.hpp"
#include "Error.hpp"
#include "ExecutionGraph.hpp"

/*******************************************************************************
 **                           RCULinkCalculator Class
 ******************************************************************************/

class RCULinkCalculator : public Calculator {

public:
	RCULinkCalculator(ExecutionGraph &g) : Calculator(g) {}

	/* Overrided Calculator methods */

	/* Initialize necessary matrices */
	void initCalc() override;

	/* Performs a step of the LB calculation */
	Calculator::CalculationResult doCalc() override;

	/* The calculator is informed about the removal of some events */
	void removeAfter(const VectorClock &preds) override;

	std::unique_ptr<Calculator> clone(ExecutionGraph &g) const override {
		return std::make_unique<RCULinkCalculator>(g);
	}

private:
	bool linksTo(Event e, Event r) const;
	std::vector<Event> getPbOptPropPoLinks(Event e) const;
	bool addRcuLinks(Event e);
	bool addRcuLinkConstraints();
};

#endif /* __RCU_LINK_CALCULATOR_HPP__ */
