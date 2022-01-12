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

#ifndef __LB_CALCULATOR_LAPOR_HPP__
#define __LB_CALCULATOR_LAPOR_HPP__

#include "Calculator.hpp"
#include "Matrix2D.hpp"
#include <unordered_map>
#include <vector>

class LBCalculatorLAPOR : public Calculator {

public:
	/* Default constructor */
	LBCalculatorLAPOR(ExecutionGraph &g) : Calculator(g) {}

	/* Adds a lock to the maintained list */
	void addLockToList(SAddr addr, const Event lock);

	/* Returns the first non-trivial event in the critical section
	 * that "lock" opens */
	Event getFirstNonTrivialInCS(const Event lock) const;

	/* Returns the last non-trivial event in the critical section
	 * that "lock" opens */
	Event getLastNonTrivialInCS(const Event lock) const;

	/* Returns a linear extension of LB */
	std::vector<Event> getLbOrdering() const;

	/* TODO: Add comments for relations and move to protected */
	void calcLbFromLoad(const ReadLabel *rLab, const LockLabelLAPOR *lLab);
	void calcLbFromStore(const WriteLabel *wLab, const LockLabelLAPOR *lLab);
	void calcLbRelation();
	bool addLbConstraints();
	bool calcLbFixpoint();

	/* Overrided Calculator methods */

	/* Initialize necessary matrices */
	void initCalc() override;

	/* Performs a step of the LB calculation */
	Calculator::CalculationResult doCalc() override;

	/* The calculator is informed about the removal of some events */
	void removeAfter(const VectorClock &preds) override;

	std::unique_ptr<Calculator> clone(ExecutionGraph &g) const override {
		return LLVM_MAKE_UNIQUE<LBCalculatorLAPOR>(g);
	}

private:
	/* A per-location list of all locks currently present in the graph */
	std::unordered_map<SAddr, std::vector<Event> > locks;
};

#endif /* __LB_CALCULATOR_LAPOR_HPP__ */
