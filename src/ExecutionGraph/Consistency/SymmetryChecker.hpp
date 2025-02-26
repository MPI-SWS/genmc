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
 * Author: Iason Marmanis <imarmanis@mpi-sws.org>
 */

#ifndef GENMC_SYMMETRY_CHECKER_HPP
#define GENMC_SYMMETRY_CHECKER_HPP

#include <memory>

class EventLabel;
class ExecutionGraph;

/*
 * SPORE utilities --- a collection of MM-independent consistency checks.
 *
 * Currently this does not need to be a class. The only reason it is is
 * that we might decide to keep some state for all these calculations (e.g.,
 * like in the other checkers).
 */
class SymmetryChecker {

public:
	static auto create() -> std::unique_ptr<SymmetryChecker>
	{
		return std::unique_ptr<SymmetryChecker>(new SymmetryChecker());
	}

	~SymmetryChecker() = default;

	SymmetryChecker(const SymmetryChecker &) = delete;
	SymmetryChecker(SymmetryChecker &&) = default;
	auto operator=(const SymmetryChecker &) -> SymmetryChecker & = delete;
	auto operator=(SymmetryChecker &&) -> SymmetryChecker & = default;

	/** Returns whether LAB and its symmetric predecessor in SYMM
	 * share their prefix. Returns false if no predecessor exists */
	auto sharePrefixSR(int symm, const EventLabel *lab) const -> bool;

	/** Check whether a graph is the representative one */
	auto isSymmetryOK(const EventLabel *lab) const -> bool;

	/** Updates the prefix view of a label with its symmetric
	 * predecessors */
	void updatePrefixWithSymmetries(EventLabel *lab);

private:
	SymmetryChecker() = default;

	auto isEcoBefore(const EventLabel *lab, int tid) const -> bool;
	auto isEcoSymmetric(const EventLabel *lab, int tid) const -> bool;
	auto isPredSymmetryOK(const EventLabel *lab, int tid) const -> bool;
	auto isPredSymmetryOK(const EventLabel *lab) const -> bool;
	auto isSuccSymmetryOK(const EventLabel *lab, int tid) const -> bool;
	auto isSuccSymmetryOK(const EventLabel *lab) const -> bool;
};

#endif /* GENMC_SYMMETRY_CHECKER_HPP */
