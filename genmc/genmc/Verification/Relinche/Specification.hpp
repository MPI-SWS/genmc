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

#ifndef GENMC_SPECIFICATION_HPP
#define GENMC_SPECIFICATION_HPP

#include "genmc/ADT/VSet.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Verification/Relinche/Linearization.hpp"
#include "genmc/Verification/Relinche/Observation.hpp"

#include <climits>
#include <istream>
#include <numeric>
#include <ostream>
#include <ranges>
#include <unordered_map>
#include <vector>

/** Extension of happens-before relation that invalidate some outcome of spec */
struct Hint {
	std::vector<std::pair<MethodCall::Id, MethodCall::Id>> edges;

	friend auto operator<<(std::ostream &os, const Hint &hint) -> std::ostream &;
};

/** Serializable collection of projected graphs (observations) of the specification program */
class Specification {
private:
	struct Record {
		VSet<Linearization> lins;
		std::vector<Hint> hints;
	};

public:
	Specification() = default;
	Specification(unsigned maxHintSize
#ifdef ENABLE_GENMC_DEBUG
		      ,
		      bool debug = false
#endif
		      )
		: maxExtSize_(maxHintSize)
#ifdef ENABLE_GENMC_DEBUG
		  ,
		  debug(debug)
#endif
	{}

	/** Iterators */

	/** Iterate over all observations */
	[[nodiscard]] auto observations() const { return std::views::keys(data_); }

	/** Iterate over <obs', <lins, hints>> pairs where obs' refines obs */
	[[nodiscard]] auto refined_observations(const Observation &obs) const
	{
		const auto [begin, end] = data_.equal_range(obs);
		return std::ranges::subrange(begin, end) |
		       std::views::filter([&](auto &kv) { return kv.first.isRefinedBy(obs); });
	}

	/** Statistics */

	[[nodiscard]] auto getNumObservations() const { return data_.size(); }

	[[nodiscard]] auto getNumOutcomes() const
	{
		auto count = 0U;
		for (auto it = data_.begin(), ie = data_.end(); it != ie;) {
			const auto &key = it->first;
			++count;
			while (++it != ie && it->first == key)
				;
		}
		return count;
	}

	[[nodiscard]] auto getNumLins() const
	{
		return std::accumulate(data_.begin(), data_.end(), 0U, [&](auto acc, auto &elem) {
			return acc + elem.second.lins.size();
		});
	}

	[[nodiscard]] auto getNumHints() const
	{
		return std::accumulate(data_.begin(), data_.end(), 0U, [&](auto acc, auto &elem) {
			return acc + elem.second.hints.size();
		});
	}

	[[nodiscard]] auto getNumSynchronizations(const Observation &obs) const
	{
		return data_.count(obs);
	}

	[[nodiscard]] auto containsSameOutput(const Observation &obs) const -> bool
	{
		return data_.contains(obs);
	}

	[[nodiscard]] auto isRefinedBy(const Observation &) const -> bool;
	[[nodiscard]] auto refinementMissingEdges(const Observation &) const
		-> std::vector<std::vector<Edge>>;

	void add(ExecutionGraph &g, const ConsistencyChecker *consChecker, bool symmReduction);
	void merge(Specification &&other);
	void calculateHints();

private:
	using storage_t = std::unordered_multimap<Observation, Record>;

	[[nodiscard]] auto lookupSyncOrInsert(const Observation &obs) -> Record &;
	[[nodiscard]] auto lookupSync(const Observation &obs) const -> const Record &;
	void addHints(const Observation &obs, std::vector<Hint> &&hints);

	[[nodiscard]] auto getMaxExtSize() const -> unsigned { return maxExtSize_; }

	[[nodiscard]]
	auto calculateObservationHints(const Observation &obs) const -> std::vector<Hint>;

	friend void serialize(std::ostream &os, const Specification &spec);
	friend auto deserialize(std::istream &is) -> Specification;

	// split into two maps <obs, lins> + <obs, hints> ?
	storage_t data_;
	unsigned maxExtSize_ = UINT_MAX;

#ifdef ENABLE_GENMC_DEBUG
	mutable unsigned int extensionsExploredInTotal = 0U;
	bool debug = false;
#endif
};

void serialize(std::ostream &os, const Specification &spec);
auto deserialize(std::istream &is) -> Specification;

#endif /* GENMC_SPECIFICATION_HPP */
