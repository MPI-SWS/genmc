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

/*******************************************************************************
 * CAUTION: This file is generated automatically by Kater -- DO NOT EDIT.
 *******************************************************************************/

#ifndef __TSO_DRIVER_HPP__
#define __TSO_DRIVER_HPP__

#include "ExecutionGraph.hpp"
#include "GenMCDriver.hpp"
#include "GraphIterators.hpp"
#include "MaximalIterator.hpp"
#include "VSet.hpp"
#include "VerificationError.hpp"
#include "config.h"
#include <cstdint>
#include <vector>

class TSODriver : public GenMCDriver {

private:
	enum class NodeStatus : unsigned char { unseen, entered, left };

	struct NodeCountStatus {
		NodeCountStatus() = default;
		NodeCountStatus(uint16_t c, NodeStatus s) : count(c), status(s) {}
		uint16_t count = 0;
		NodeStatus status = NodeStatus::unseen;
	};

public:
	TSODriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		  std::unique_ptr<ModuleInfo> MI,
		  GenMCDriver::Mode mode = GenMCDriver::VerificationMode{});

	std::vector<VSet<Event>> calculateSaved(const EventLabel *lab);
	std::vector<View> calculateViews(const EventLabel *lab);
	void updateMMViews(EventLabel *lab) override;
	bool isDepTracking() const override;
	bool isConsistent(const EventLabel *lab) const override;
	VerificationError checkErrors(const EventLabel *lab,
				      const EventLabel *&race) const override;
	std::vector<VerificationError>
	checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings,
		      std::vector<const EventLabel *> &racyLabs) const;
	bool isRecoveryValid(const EventLabel *lab) const override;
	std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const override;
	const View &getHbView(const EventLabel *lab) const override;
	std::vector<Event> getCoherentStores(SAddr addr, Event read) override;
	std::vector<Event> getCoherentRevisits(const WriteLabel *sLab,
					       const VectorClock &pporf) override;
	llvm::iterator_range<ExecutionGraph::co_iterator>
	getCoherentPlacings(SAddr addr, Event store, bool isRMW) override;

private:
	bool isWriteRfBefore(Event a, Event b);
	std::vector<Event> getInitRfsAtLoc(SAddr addr);
	bool isHbOptRfBefore(const Event e, const Event write);
	ExecutionGraph::co_iterator splitLocMOBefore(SAddr addr, Event e);
	ExecutionGraph::co_iterator splitLocMOAfterHb(SAddr addr, const Event read);
	ExecutionGraph::co_iterator splitLocMOAfter(SAddr addr, const Event e);
	std::vector<Event> getMOOptRfAfter(const WriteLabel *sLab);
	std::vector<Event> getMOInvOptRfAfter(const WriteLabel *sLab);

	void visitCalc0_0(const EventLabel *lab, View &calcRes);
	void visitCalc0_1(const EventLabel *lab, View &calcRes);
	void visitCalc0_2(const EventLabel *lab, View &calcRes);

	View calculate0(const EventLabel *lab);

	mutable std::vector<NodeStatus> visitedCalc0_0;
	mutable std::vector<NodeStatus> visitedCalc0_1;
	mutable std::vector<NodeStatus> visitedCalc0_2;

	void visitCalc1_0(const EventLabel *lab, View &calcRes);
	void visitCalc1_1(const EventLabel *lab, View &calcRes);
	void visitCalc1_2(const EventLabel *lab, View &calcRes);

	View calculate1(const EventLabel *lab);

	mutable std::vector<NodeStatus> visitedCalc1_0;
	mutable std::vector<NodeStatus> visitedCalc1_1;
	mutable std::vector<NodeStatus> visitedCalc1_2;

	bool visitInclusionLHS0_0(const EventLabel *lab, const View &v) const;
	bool visitInclusionLHS0_1(const EventLabel *lab, const View &v) const;

	bool checkInclusion0(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS0_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS0_1;
	mutable std::vector<NodeStatus> visitedInclusionRHS0_0;
	mutable std::vector<NodeStatus> visitedInclusionRHS0_1;

	mutable std::vector<bool> lhsAccept0;
	mutable std::vector<bool> rhsAccept0;

	mutable const EventLabel *racyLab0 = nullptr;

	void visitInclusionLHS1_0(const EventLabel *lab) const;
	void visitInclusionLHS1_1(const EventLabel *lab) const;

	bool checkInclusion1(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS1_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS1_1;

	mutable std::vector<bool> lhsAccept1;
	mutable std::vector<bool> rhsAccept1;

	mutable const EventLabel *racyLab1 = nullptr;

	bool visitInclusionLHS2_0(const EventLabel *lab, const View &v) const;
	bool visitInclusionLHS2_1(const EventLabel *lab, const View &v) const;
	bool visitInclusionLHS2_2(const EventLabel *lab, const View &v) const;

	bool checkInclusion2(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS2_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS2_1;
	mutable std::vector<NodeStatus> visitedInclusionLHS2_2;
	mutable std::vector<NodeStatus> visitedInclusionRHS2_0;
	mutable std::vector<NodeStatus> visitedInclusionRHS2_1;

	mutable std::vector<bool> lhsAccept2;
	mutable std::vector<bool> rhsAccept2;

	mutable const EventLabel *racyLab2 = nullptr;

	void visitInclusionLHS3_0(const EventLabel *lab) const;
	void visitInclusionLHS3_1(const EventLabel *lab) const;
	void visitInclusionLHS3_2(const EventLabel *lab) const;

	bool checkInclusion3(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS3_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS3_1;
	mutable std::vector<NodeStatus> visitedInclusionLHS3_2;

	mutable std::vector<bool> lhsAccept3;
	mutable std::vector<bool> rhsAccept3;

	mutable const EventLabel *racyLab3 = nullptr;

	bool visitInclusionLHS4_0(const EventLabel *lab, const View &v) const;
	bool visitInclusionLHS4_1(const EventLabel *lab, const View &v) const;
	bool visitInclusionLHS4_2(const EventLabel *lab, const View &v) const;

	bool checkInclusion4(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS4_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS4_1;
	mutable std::vector<NodeStatus> visitedInclusionLHS4_2;
	mutable std::vector<NodeStatus> visitedInclusionRHS4_0;
	mutable std::vector<NodeStatus> visitedInclusionRHS4_1;

	mutable std::vector<bool> lhsAccept4;
	mutable std::vector<bool> rhsAccept4;

	mutable const EventLabel *racyLab4 = nullptr;

	void visitInclusionLHS5_0(const EventLabel *lab) const;
	void visitInclusionLHS5_1(const EventLabel *lab) const;
	void visitInclusionLHS5_2(const EventLabel *lab) const;

	bool checkInclusion5(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS5_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS5_1;
	mutable std::vector<NodeStatus> visitedInclusionLHS5_2;

	mutable std::vector<bool> lhsAccept5;
	mutable std::vector<bool> rhsAccept5;

	mutable const EventLabel *racyLab5 = nullptr;

	bool visitInclusionLHS6_0(const EventLabel *lab, const View &v) const;
	bool visitInclusionLHS6_1(const EventLabel *lab, const View &v) const;

	bool checkInclusion6(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS6_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS6_1;
	mutable std::vector<NodeStatus> visitedInclusionRHS6_0;
	mutable std::vector<NodeStatus> visitedInclusionRHS6_1;

	mutable std::vector<bool> lhsAccept6;
	mutable std::vector<bool> rhsAccept6;

	mutable const EventLabel *racyLab6 = nullptr;

	bool visitInclusionLHS7_0(const EventLabel *lab, const View &v) const;
	bool visitInclusionLHS7_1(const EventLabel *lab, const View &v) const;

	bool checkInclusion7(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedInclusionLHS7_0;
	mutable std::vector<NodeStatus> visitedInclusionLHS7_1;
	mutable std::vector<NodeStatus> visitedInclusionRHS7_0;
	mutable std::vector<NodeStatus> visitedInclusionRHS7_1;

	mutable std::vector<bool> lhsAccept7;
	mutable std::vector<bool> rhsAccept7;

	mutable const EventLabel *racyLab7 = nullptr;

	bool visitAcyclic0_0(const EventLabel *lab) const;
	bool visitAcyclic0_1(const EventLabel *lab) const;
	bool visitAcyclic0_2(const EventLabel *lab) const;
	bool visitAcyclic0_3(const EventLabel *lab) const;

	bool isAcyclic0(const EventLabel *lab) const;

	mutable std::vector<NodeCountStatus> visitedAcyclic0_0;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_1;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_2;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_3;

	mutable uint16_t visitedAccepting0 = 0;
	bool shouldVisitAcyclic0(void) const { return true; };

	bool isRecAcyclic(const EventLabel *lab) const;

	mutable uint16_t visitedRecAccepting = 0;
	void visitPPoRf0(const EventLabel *lab, View &pporf) const;

	View calcPPoRfBefore(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedPPoRf0;

	mutable std::vector<VSet<Event>> saved;
	mutable std::vector<View> views;
};

#endif /* __TSO_DRIVER_HPP__ */
