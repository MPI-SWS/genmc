/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#ifndef GENMC_SYMMETRY_CHECKER_HPP
#define GENMC_SYMMETRY_CHECKER_HPP

#include <memory>

class EventLabel;
class ExecutionGraph;

/**
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
