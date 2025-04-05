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
 * Author: Pavel Golovin <pgolovin@mpi-sws.org>
 */

#ifndef GENMC_RELINCHE_HPP
#define GENMC_RELINCHE_HPP

#include "ExecutionGraph/EventLabel.hpp"
#include "Support/Error.hpp"
#include "Verification/Relinche/Specification.hpp"

#include <chrono>
#include <fstream>
#include <string>

class ExecutionGraph;
class ConsistencyChecker;

/** Abstract class for linearizability checking errors */
struct LinearizabilityError {
	virtual auto toString() const -> std::string = 0;

	virtual ~LinearizabilityError() = default;
};

/** Checks that a library implementation refines its (linearizability) specification */
class LinearizabilityChecker {
public:
	struct Result {
		std::unique_ptr<LinearizabilityError> status;
		uint32_t hintsChecked = 0;
#ifdef ENABLE_GENMC_DEBUG
		std::chrono::high_resolution_clock::duration analysisTime;
#endif

		auto operator+=(Result &&other) -> Result &
		{
			/* Propagate latest error */
			if (other.status)
				status = std::move(other.status);
			hintsChecked += other.hintsChecked;
			GENMC_DEBUG(analysisTime += other.analysisTime;);
			return *this;
		}
	};

	/** Create checker by loading specification from specFile */
	static auto create(const ConsistencyChecker *consChecker, const std::string &specFile)
	{
		return std::unique_ptr<LinearizabilityChecker>(
			new LinearizabilityChecker(consChecker, specFile));
	}

	/** Check that an implementation graph satisfies the library specification */
	auto refinesSpec(ExecutionGraph &graph) -> Result;

private:
	LinearizabilityChecker(const ConsistencyChecker *consChecker, const std::string &specFile)
		: consChecker(consChecker)
	{
		std::ifstream istream(specFile);
		specification = std::make_unique<Specification>(deserialize(istream));
	}

	const ConsistencyChecker *consChecker{};
	std::unique_ptr<Specification> specification;
};

#endif /* GENMC_RELINCHE_HPP */
