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

/*******************************************************************************
 * CAUTION: This file is generated automatically by Kater -- DO NOT EDIT.
 *******************************************************************************/

#ifndef GENMC_IMM_CHECKER_HPP
#define GENMC_IMM_CHECKER_HPP

#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include <concepts>
#include <cstdint>
#include <vector>

// NOLINTBEGIN
class IMMChecker : public ConsistencyChecker {

private:
	enum class NodeStatus : uint32_t { unseen = 0, entered = 1, left = 2 };

	struct DFSWorklistEntry {
		DFSWorklistEntry() = default;
		DFSWorklistEntry(uint32_t state, const EventLabel *lab, bool finishing = false)
			: nfaState(state), lab(lab), isFinishing(finishing)
		{}
		uint32_t nfaState;
		const EventLabel *lab;
		bool isFinishing;
	};

	template <std::unsigned_integral T> class NodeStatusVector {
	public:
		NodeStatusVector() = default;
		NodeStatusVector(size_t size) : visited(size), baseStatus(0) {}

		[[nodiscard]] auto getStatus(size_t i) const -> NodeStatus
		{
			auto status = visited[i];
			if (status < baseStatus)
				return NodeStatus::unseen;
			return static_cast<NodeStatus>(status - baseStatus);
		}

		void setStatus(size_t i, NodeStatus s)
		{
			visited[i] = baseStatus + static_cast<T>(s);
		}

		void maybeClearResize(size_t newSize)
		{
			if (baseStatus >= std::numeric_limits<T>::max() - 6) {
				visited.clear();
				baseStatus = 0;
			} else {
				baseStatus += 3;
			}
			visited.resize(newSize);
		}

	private:
		mutable std::vector<T> visited;
		T baseStatus{};
	};

	class NodeVisitStatusVector {
	private:
		struct NodeVisitStatus {
			NodeVisitStatus() = default;
			NodeVisitStatus(uint32_t count, uint32_t status)
				: count(count), status(status)
			{}
			uint32_t count{};
			uint32_t status{};
		};

	public:
		NodeVisitStatusVector() = default;
		NodeVisitStatusVector(size_t size)
			: visited(size), baseStatus(0), baseCount(0), currIterationMaxCount(0)
		{}

		[[nodiscard]] auto getStatus(size_t i) const -> NodeStatus
		{
			auto status = visited[i].status;
			if (status < baseStatus)
				return NodeStatus::unseen;
			return static_cast<NodeStatus>(status - baseStatus);
		}

		void setStatus(size_t i, NodeStatus s)
		{
			visited[i].status = baseStatus + static_cast<uint32_t>(s);
		}

		[[nodiscard]] auto getCount(size_t i) const -> uint32_t
		{
			auto count = visited[i].count;
			if (count < baseCount)
				return 0;
			return count - baseCount;
		}

		auto setCountIncr(size_t i, uint32_t count)
		{
			const uint32_t newCount = baseCount + count;
			visited[i].count = newCount;
			currIterationMaxCount = std::max(currIterationMaxCount, newCount);
		}

		void setIncr(size_t i, uint32_t count, NodeStatus status)
		{
			setCountIncr(i, count);
			setStatus(i, status);
		}

		void set(size_t i, uint32_t count, NodeStatus status)
		{
			visited[i].count = baseCount + count;
			setStatus(i, status);
		}

		void maybeClearResize(size_t newSize)
		{
			if (baseStatus >= UINT32_MAX - 6 ||
			    currIterationMaxCount > UINT32_MAX / 2) {
				visited.clear();
				baseStatus = 0;
				baseCount = 0;
			} else {
				baseStatus += 3;
				baseCount = currIterationMaxCount;
			}
			currIterationMaxCount = 0;
			visited.resize(newSize);
		}

	private:
		mutable std::vector<NodeVisitStatus> visited;
		uint32_t baseStatus{};
		uint32_t baseCount{};
		uint32_t currIterationMaxCount{};
	};

public:
	IMMChecker(const Config *conf) : ConsistencyChecker(conf) {};

private:
	bool isConsistent(const EventLabel *lab) const override;
	bool isConsistent(const ExecutionGraph &g) const override;
	bool isCoherentRelinche(const ExecutionGraph &g) const override;
	std::optional<VerificationError> checkErrors(const EventLabel *lab,
						     const EventLabel *&race) const override;
	std::vector<VerificationError>
	checkWarnings(const EventLabel *lab, const VSet<VerificationError> &reported,
		      std::vector<const EventLabel *> &races) const override;
	std::vector<EventLabel *> getCoherentStores(ReadLabel *rLab) override;
	void filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls) override;
	std::vector<EventLabel *> getCoherentPlacings(WriteLabel *sLab) override;
	void updateMMViews(EventLabel *lab) override;
	std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const override;
	bool isDepTracking() const override;
	void calculateSaved(EventLabel *lab);
	void calculateViews(EventLabel *lab);
	mutable const EventLabel *cexLab{};
	void recomputeCacheCounters(const ExecutionGraph &g) const override;
	void resetCacheCounters() const override;
	void maybeDecreaseCacheCounters(const EventLabel *lab) const override;
	void maybeIncreaseCacheCounters(const EventLabel *lab) const override;

	mutable NodeStatusVector<uint32_t> visitedCalc75_0;
	mutable NodeStatusVector<uint32_t> visitedCalc75_1;
	mutable NodeStatusVector<uint32_t> visitedCalc75_2;
	mutable NodeStatusVector<uint32_t> visitedCalc75_3;
	mutable NodeStatusVector<uint32_t> visitedCalc75_4;
	mutable NodeStatusVector<uint32_t> visitedCalc75_5;
	mutable NodeStatusVector<uint32_t> visitedCalc75_6;

	bool visitCalc75Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const;

	View visitCalc75(const EventLabel *lab) const;
	auto getHbView(const EventLabel *lab) const -> const View & { return lab->view(0); }

	auto checkCalc75(const EventLabel *lab) const;
	mutable NodeStatusVector<uint32_t> visitedCalc76_0;
	mutable NodeStatusVector<uint32_t> visitedCalc76_1;
	mutable NodeStatusVector<uint32_t> visitedCalc76_2;
	mutable NodeStatusVector<uint32_t> visitedCalc76_3;
	mutable NodeStatusVector<uint32_t> visitedCalc76_4;
	mutable NodeStatusVector<uint32_t> visitedCalc76_5;
	mutable NodeStatusVector<uint32_t> visitedCalc76_6;

	bool visitCalc76Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const;

	View visitCalc76(const EventLabel *lab) const;
	auto getHbRelincheView(const EventLabel *lab) const -> const View & { return lab->view(1); }

	auto checkCalc76(const EventLabel *lab) const;
	mutable NodeStatusVector<uint32_t> visitedCalc87_0;
	mutable NodeStatusVector<uint32_t> visitedCalc87_1;
	mutable NodeStatusVector<uint32_t> visitedCalc87_2;

	bool visitCalc87Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const;

	View visitCalc87(const EventLabel *lab) const;
	auto getPorfView(const EventLabel *lab) const -> const View & { return lab->view(2); }

	auto checkCalc87(const EventLabel *lab) const;
	mutable NodeStatusVector<uint32_t> visitedCoherence_0;
	mutable NodeStatusVector<uint32_t> visitedCoherence_1;
	mutable NodeStatusVector<uint32_t> visitedCoherence_2;
	mutable NodeStatusVector<uint32_t> visitedCoherence_3;
	mutable NodeStatusVector<uint32_t> visitedCoherence_4;
	mutable NodeStatusVector<uint32_t> visitedCoherence_5;
	mutable NodeStatusVector<uint32_t> visitedCoherence_6;
	mutable NodeStatusVector<uint32_t> visitedCoherence_7;
	mutable NodeStatusVector<uint32_t> visitedCoherence_8;
	mutable NodeStatusVector<uint32_t> visitedCoherence_9;
	mutable NodeStatusVector<uint32_t> visitedCoherence_10;
	mutable NodeStatusVector<uint32_t> visitedCoherence_11;
	mutable NodeStatusVector<uint32_t> visitedCoherence_12;

	bool visitCoherenceIterative(std::vector<DFSWorklistEntry> &worklist,
				     const EventLabel *initLab) const;

	bool visitCoherenceRelinche(const ExecutionGraph &g) const;

	mutable NodeVisitStatusVector visitedConsAcyclic1_0;
	mutable NodeVisitStatusVector visitedConsAcyclic1_1;
	mutable NodeVisitStatusVector visitedConsAcyclic1_2;
	mutable NodeVisitStatusVector visitedConsAcyclic1_3;
	mutable NodeVisitStatusVector visitedConsAcyclic1_4;
	mutable NodeVisitStatusVector visitedConsAcyclic1_5;
	mutable NodeVisitStatusVector visitedConsAcyclic1_6;
	mutable NodeVisitStatusVector visitedConsAcyclic1_7;
	mutable NodeVisitStatusVector visitedConsAcyclic1_8;
	mutable NodeVisitStatusVector visitedConsAcyclic1_9;
	mutable NodeVisitStatusVector visitedConsAcyclic1_10;
	mutable NodeVisitStatusVector visitedConsAcyclic1_11;
	mutable NodeVisitStatusVector visitedConsAcyclic1_12;
	mutable NodeVisitStatusVector visitedConsAcyclic1_13;
	mutable NodeVisitStatusVector visitedConsAcyclic1_14;
	mutable NodeVisitStatusVector visitedConsAcyclic1_15;
	mutable NodeVisitStatusVector visitedConsAcyclic1_16;
	mutable NodeVisitStatusVector visitedConsAcyclic1_17;
	mutable NodeVisitStatusVector visitedConsAcyclic1_18;
	mutable NodeVisitStatusVector visitedConsAcyclic1_19;
	mutable uint32_t visitedConsAcyclic1Accepting;

	bool visitConsAcyclic1Iterative(std::vector<DFSWorklistEntry> &worklist) const;

	bool visitConsAcyclic1(const EventLabel *lab) const;

	bool visitConsAcyclic1Full(const ExecutionGraph &g) const;

	mutable int cacheCounterUnlessConsAcyclic1 = 0;
	void recomputeUnlessConsAcyclic1(const ExecutionGraph &g) const;
	bool visitUnlessConsAcyclic1(const EventLabel *lab) const;
	bool checkConsAcyclic1(const EventLabel *lab) const;
	bool checkConsAcyclic1(const ExecutionGraph &g) const;
	mutable NodeVisitStatusVector visitedConsAcyclic2_0;
	mutable NodeVisitStatusVector visitedConsAcyclic2_1;
	mutable NodeVisitStatusVector visitedConsAcyclic2_2;
	mutable NodeVisitStatusVector visitedConsAcyclic2_3;
	mutable NodeVisitStatusVector visitedConsAcyclic2_4;
	mutable NodeVisitStatusVector visitedConsAcyclic2_5;
	mutable NodeVisitStatusVector visitedConsAcyclic2_6;
	mutable NodeVisitStatusVector visitedConsAcyclic2_7;
	mutable NodeVisitStatusVector visitedConsAcyclic2_8;
	mutable NodeVisitStatusVector visitedConsAcyclic2_9;
	mutable NodeVisitStatusVector visitedConsAcyclic2_10;
	mutable NodeVisitStatusVector visitedConsAcyclic2_11;
	mutable NodeVisitStatusVector visitedConsAcyclic2_12;
	mutable NodeVisitStatusVector visitedConsAcyclic2_13;
	mutable NodeVisitStatusVector visitedConsAcyclic2_14;
	mutable NodeVisitStatusVector visitedConsAcyclic2_15;
	mutable NodeVisitStatusVector visitedConsAcyclic2_16;
	mutable NodeVisitStatusVector visitedConsAcyclic2_17;
	mutable uint32_t visitedConsAcyclic2Accepting;

	bool visitConsAcyclic2Iterative(std::vector<DFSWorklistEntry> &worklist) const;

	bool visitConsAcyclic2(const EventLabel *lab) const;

	bool visitConsAcyclic2Full(const ExecutionGraph &g) const;

	bool checkUnlessConsAcyclic2([[maybe_unused]] const EventLabel *lab) { return false; }
	bool checkConsAcyclic2(const EventLabel *lab) const;
	bool checkConsAcyclic2(const ExecutionGraph &g) const;
	bool visitWarning3(const EventLabel *lab) const;
	mutable NodeStatusVector<uint32_t> visitedLHSUnlessWarning3_0;
	mutable NodeStatusVector<uint32_t> visitedLHSUnlessWarning3_1;

	bool visitLHSUnlessWarning3Iterative(std::vector<DFSWorklistEntry> &worklist,
					     const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessWarning3Accepting;
	bool visitUnlessWarning3(const EventLabel *lab) const;
	bool checkWarning3(const EventLabel *lab) const;

	void visitPPoRf0(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf1(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf2(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf3(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf4(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf5(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf6(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf7(const EventLabel *lab, DepView &pporf) const;

	DepView calcPPoRfBefore(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedPPoRf0;
	mutable std::vector<NodeStatus> visitedPPoRf1;
	mutable std::vector<NodeStatus> visitedPPoRf2;
	mutable std::vector<NodeStatus> visitedPPoRf3;
	mutable std::vector<NodeStatus> visitedPPoRf4;
	mutable std::vector<NodeStatus> visitedPPoRf5;
	mutable std::vector<NodeStatus> visitedPPoRf6;
	mutable std::vector<NodeStatus> visitedPPoRf7;
};

// NOLINTEND

#endif /* GENMC_IMM_CHECKER_HPP */
