#ifndef GENMC_UNIT_STUB_CHECKER_HPP
#define GENMC_UNIT_STUB_CHECKER_HPP

#include <memory>
#include <vector>

#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"

/* A checker whose every operation is a no-op: a base for unit tests that
 * exercise graph machinery without a memory model */
class StubChecker : public ConsistencyChecker {
public:
	StubChecker() : ConsistencyChecker(nullptr) {}

	[[nodiscard]] auto isConsistent(const EventLabel * /*lab*/) const -> bool override
	{
		return true;
	}
	[[nodiscard]] auto isConsistent(const ExecutionGraph & /*g*/) const -> bool override
	{
		return true;
	}
	auto checkErrors(const EventLabel * /*lab*/, const EventLabel *& /*race*/) const
		-> std::optional<VerificationError> override
	{
		return {};
	}
	auto checkWarnings(const EventLabel * /*lab*/, const VSet<VerificationError> & /*reported*/,
			   std::vector<const EventLabel *> & /*races*/) const
		-> std::vector<VerificationError> override
	{
		return {};
	}
	void filterCoherentRevisits(WriteLabel * /*sLab*/,
				    std::vector<ReadLabel *> & /*ls*/) override
	{}
	auto getCoherentStores(ReadLabel * /*rLab*/) -> std::vector<EventLabel *> override
	{
		return {};
	}
	auto getCoherentPlacings(WriteLabel * /*wLab*/) -> std::vector<EventLabel *> override
	{
		return {};
	}
	void updateMMViews(EventLabel * /*lab*/) override {}
	auto calculatePrefixView(const EventLabel * /*lab*/) const
		-> std::unique_ptr<VectorClock> override
	{
		return std::make_unique<View>();
	}
	auto getHbViewIndex() const -> unsigned override { return 0; }
	[[nodiscard]] auto isDepTracking() const -> bool override { return false; }

	void resetCacheCounters() const override {}
	void recomputeCacheCounters(const ExecutionGraph & /*g*/) const override {}
	void maybeDecreaseCacheCounters(const EventLabel * /*lab*/) const override {}
	void maybeIncreaseCacheCounters(const EventLabel * /*lab*/) const override {}

};

#endif /* GENMC_UNIT_STUB_CHECKER_HPP */
