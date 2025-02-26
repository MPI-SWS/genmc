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

#ifndef GENMC_CONSISTENCY_CHECKER_HPP
#define GENMC_CONSISTENCY_CHECKER_HPP

#include "ADT/VSet.hpp"
#include "Verification/VerificationError.hpp"

#include <memory>
#include <vector>

class EventLabel;
class ReadLabel;
class WriteLabel;
class ExecutionGraph;
class Config;
class VectorClock;
class View;
enum class ModelType : std::uint8_t;

/* An abstract class defining the API for checking graph consistency,
 * and for caching memory-model-specific information in the execution graph.
 * Consistency checkers for specific models should override this class.
 * This class is not thread-safe: each thread needs its own checker.
 */
class ConsistencyChecker {
protected:
	ConsistencyChecker() = default;

public:
	virtual ~ConsistencyChecker() = default;

	ConsistencyChecker(const ConsistencyChecker &) = delete;
	ConsistencyChecker(ConsistencyChecker &&) = delete;
	auto operator=(const ConsistencyChecker &) -> ConsistencyChecker & = delete;
	auto operator=(ConsistencyChecker &&) -> ConsistencyChecker & = delete;

	static auto create(ModelType model) -> std::unique_ptr<ConsistencyChecker>;

	/* Returns true if the current graph is consistent when E is added */
	virtual auto isConsistent(const EventLabel *lab) const -> bool = 0;

	virtual auto checkErrors(const EventLabel *lab, const EventLabel *&race) const
		-> VerificationError = 0;

	virtual auto checkWarnings(const EventLabel *lab, const VSet<VerificationError> &reported,
				   std::vector<const EventLabel *> &races) const
		-> std::vector<VerificationError> = 0;

	virtual auto getCoherentRevisits(WriteLabel *sLab, const VectorClock &pporf)
		-> std::vector<ReadLabel *> = 0;

	virtual auto getCoherentStores(ReadLabel *rLab) -> std::vector<EventLabel *> = 0;

	virtual auto getCoherentPlacings(WriteLabel *wLab) -> std::vector<EventLabel *> = 0;

	/* Updates lab with model-specific information.
	 * Needs to be called every time a new label is added to the graph */
	virtual void updateMMViews(EventLabel *lab) = 0;

	virtual auto calculatePrefixView(const EventLabel *lab) const
		-> std::unique_ptr<VectorClock> = 0;

	virtual auto getHbView(const EventLabel *lab) const -> const View & = 0;

	virtual auto isDepTracking() const -> bool = 0;

private:
	/* We don't need any private members but it's conceivable that
	 * we would also like to cache some information to be
	 * reused */
};

#endif /* GENMC_CONSISTENCY_CHECKER_HPP */
