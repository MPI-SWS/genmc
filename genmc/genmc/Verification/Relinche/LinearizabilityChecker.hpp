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

#ifndef GENMC_RELINCHE_HPP
#define GENMC_RELINCHE_HPP

#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Verification/Relinche/Specification.hpp"

#include <chrono>
#include <fstream>
#include <string>

class ExecutionGraph;
class ConsistencyChecker;

/** Abstract class for linearizability checking errors */
struct LinearizabilityError { // NOLINT(cppcoreguidelines-special-member-functions)
	[[nodiscard]] virtual auto toString() const -> std::string = 0;

	virtual ~LinearizabilityError() = default;
};

/** Checks that a library implementation refines its (linearizability) specification */
class LinearizabilityChecker {
public:
	struct Result {
		std::unique_ptr<LinearizabilityError> status;
		uint32_t hintsChecked = 0;
#ifdef ENABLE_GENMC_DEBUG
		std::chrono::high_resolution_clock::duration analysisTime{};
#endif

		// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
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
