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

#ifndef __PSC_CALCULATOR_HPP__
#define __PSC_CALCULATOR_HPP__

#include "Calculator.hpp"

class PSCCalculator : public Calculator {

public:
	PSCCalculator(ExecutionGraph &g) : Calculator(g) {}

	/* Overrided Calculator methods */

	/* Initialize necessary matrices */
	void initCalc() override;

	/* Performs a step of the LB calculation */
	Calculator::CalculationResult doCalc() override;

	/* The calculator is informed about the removal of some events */
	void removeAfter(const VectorClock &preds) override;

	/* The calculator is informed about the restoration of some events */
	void restorePrefix(const ReadLabel *rLab,
			   const std::vector<std::unique_ptr<EventLabel> > &storePrefix,
			   const std::vector<std::pair<Event, Event> > &status) override;

private:
	/* Returns a list with all accesses that are accessed at least twice */
	std::vector<const llvm::GenericValue *> getDoubleLocs() const;

	std::vector<Event> calcSCFencesSuccs(const std::vector<Event> &fcs,
					     const Event e) const;
	std::vector<Event> calcSCFencesPreds(const std::vector<Event> &fcs,
					     const Event e) const;
	std::vector<Event> calcSCSuccs(const std::vector<Event> &fcs,
				       const Event e) const;
	std::vector<Event> calcSCPreds(const std::vector<Event> &fcs,
				       const Event e) const;
	std::vector<Event> calcRfSCSuccs(const std::vector<Event> &fcs,
					 const Event e) const;
	std::vector<Event> calcRfSCFencesSuccs(const std::vector<Event> &fcs,
					       const Event e) const;

	void addRbEdges(const std::vector<Event> &fcs,
			const std::vector<Event> &moAfter,
			const std::vector<Event> &moRfAfter,
			Calculator::GlobalRelation &matrix, const Event &e) const;
	void addMoRfEdges(const std::vector<Event> &fcs,
			  const std::vector<Event> &moAfter,
			  const std::vector<Event> &moRfAfter,
			  Calculator::GlobalRelation &matrix, const Event &e) const;
	void addSCEcosLoc(const std::vector<Event> &fcs,
			  Calculator::GlobalRelation &coMatrix,
			  Calculator::GlobalRelation &pscMatrix) const;

	void addSCEcos(const std::vector<Event> &fcs,
		       const std::vector<const llvm::GenericValue *> &scLocs,
		       Calculator::GlobalRelation &matrix) const;

	void addInitEdges(const std::vector<Event> &fcs,
			  Calculator::GlobalRelation &matrix) const;
	void addSbHbEdges(Calculator::GlobalRelation &matrix) const;

	Calculator::CalculationResult addPscConstraints();
	void calcPscRelation();
};

#endif /* __PSC_CALCULATOR_HPP__ */
