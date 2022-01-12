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

#ifndef __PERSISTENCY_CHECKER_HPP__
#define __PERSISTENCY_CHECKER_HPP__

#include "Calculator.hpp"

class ExecutionGraph;

/*******************************************************************************
 **                    PersistencyChecker Class
 ******************************************************************************/

/*
 * A class that calculates a "persists-before" order (indicating the
 * order in which disk writes may be flushed to disk), as well as
 * a "recovery" relation
 */
class PersistencyChecker {

public:
	/* Constructor */
	PersistencyChecker(ExecutionGraph &g, unsigned int blockSize)
		: execGraph(g), blockSize(blockSize) {}

	/* Helpers that calculate pb-before events for disk accesses */
	void calcDskMemAccessPbView(MemAccessLabel *mLab);
	void calcDskFencePbView(FenceLabel *fLab);

	/* Returns whether the recovery relation is acyclic */
	bool isRecAcyclic();

	std::unique_ptr<PersistencyChecker> clone(ExecutionGraph &g) {
		return LLVM_MAKE_UNIQUE<PersistencyChecker>(g, blockSize);
	}

private:
	/* Return a reference to the execution graph */
	ExecutionGraph &getGraph() { return execGraph; }
	ExecutionGraph &getGraph() const { return execGraph; }

	/* Returns the block size */
	unsigned int getBlockSize() const { return blockSize; }

	/* Returns a reference to the pb relation */
	Calculator::GlobalRelation &getPbRelation() { return pbRelation; }

	/* Helpers for view calculation */
	void calcFsyncPbView(DskFsyncLabel *fLab);
	void calcSyncPbView(DskSyncLabel *fLab);
	void calcPbarrierPbView(DskPbarrierLabel *fLab);

	/* Returns true if S is read from a load
	 * in the recovery routine */
	bool isStoreReadFromRecRoutine(Event s);

	/* Calculates pb-relation. As a side-effect, it also
	 * calculates coherence and populates the respective
	 * relation in the graph */
	void calcPb();

	/* Returns true if the read does not participate in a
	 * cycle in rec */
	bool isRecFromReadValid(const DskReadLabel *rLab);

	/* A reference to the execution graph */
	ExecutionGraph &execGraph;

	/* Block size in bytes */
	unsigned int blockSize;

	/* Persists-before relation.  This can be local since it will
	 * only be modified from the persistency checker */
	Calculator::GlobalRelation pbRelation;
};

#endif /* __PERSISTENCY_CHECKER_HPP__ */
