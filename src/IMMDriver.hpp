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

#ifndef __IMM_DRIVER_HPP__
#define __IMM_DRIVER_HPP__

#include "ExecutionGraph.hpp"
#include "GenMCDriver.hpp"
#include "GraphIterators.hpp"
#include "MaximalIterator.hpp"
#include "VSet.hpp"
#include "VerificationError.hpp"
#include "config.h"
#include <cstdint>
#include <vector>

class IMMDriver : public GenMCDriver {

private:
	enum class NodeStatus : unsigned char { unseen, entered, left };

	struct NodeCountStatus {
		NodeCountStatus() = default;
		NodeCountStatus(uint16_t c, NodeStatus s) : count(c), status(s) {}
		uint16_t count = 0;
		NodeStatus status = NodeStatus::unseen;
	};

public:
	IMMDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
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
	void visitCalc0_3(const EventLabel *lab, View &calcRes);
	void visitCalc0_4(const EventLabel *lab, View &calcRes);
	void visitCalc0_5(const EventLabel *lab, View &calcRes);
	void visitCalc0_6(const EventLabel *lab, View &calcRes);

	View calculate0(const EventLabel *lab);

	mutable std::vector<NodeStatus> visitedCalc0_0;
	mutable std::vector<NodeStatus> visitedCalc0_1;
	mutable std::vector<NodeStatus> visitedCalc0_2;
	mutable std::vector<NodeStatus> visitedCalc0_3;
	mutable std::vector<NodeStatus> visitedCalc0_4;
	mutable std::vector<NodeStatus> visitedCalc0_5;
	mutable std::vector<NodeStatus> visitedCalc0_6;

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

	bool visitAcyclic0_0(const EventLabel *lab) const;
	bool visitAcyclic0_1(const EventLabel *lab) const;
	bool visitAcyclic0_2(const EventLabel *lab) const;
	bool visitAcyclic0_3(const EventLabel *lab) const;
	bool visitAcyclic0_4(const EventLabel *lab) const;
	bool visitAcyclic0_5(const EventLabel *lab) const;
	bool visitAcyclic0_6(const EventLabel *lab) const;
	bool visitAcyclic0_7(const EventLabel *lab) const;
	bool visitAcyclic0_8(const EventLabel *lab) const;
	bool visitAcyclic0_9(const EventLabel *lab) const;
	bool visitAcyclic0_10(const EventLabel *lab) const;
	bool visitAcyclic0_11(const EventLabel *lab) const;
	bool visitAcyclic0_12(const EventLabel *lab) const;
	bool visitAcyclic0_13(const EventLabel *lab) const;
	bool visitAcyclic0_14(const EventLabel *lab) const;
	bool visitAcyclic0_15(const EventLabel *lab) const;

	bool isAcyclic0(const EventLabel *lab) const;

	mutable std::vector<NodeCountStatus> visitedAcyclic0_0;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_1;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_2;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_3;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_4;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_5;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_6;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_7;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_8;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_9;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_10;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_11;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_12;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_13;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_14;
	mutable std::vector<NodeCountStatus> visitedAcyclic0_15;

	mutable uint16_t visitedAccepting0 = 0;
	bool shouldVisitAcyclic0_0(const EventLabel *lab) const;
	bool shouldVisitAcyclic0_1(const EventLabel *lab) const;

	bool shouldVisitAcyclic0(void) const;

	mutable std::vector<NodeStatus> shouldVisitedAcyclic0_0;
	mutable std::vector<NodeStatus> shouldVisitedAcyclic0_1;

	bool visitAcyclic1_0(const EventLabel *lab) const;
	bool visitAcyclic1_1(const EventLabel *lab) const;
	bool visitAcyclic1_2(const EventLabel *lab) const;
	bool visitAcyclic1_3(const EventLabel *lab) const;
	bool visitAcyclic1_4(const EventLabel *lab) const;
	bool visitAcyclic1_5(const EventLabel *lab) const;
	bool visitAcyclic1_6(const EventLabel *lab) const;
	bool visitAcyclic1_7(const EventLabel *lab) const;
	bool visitAcyclic1_8(const EventLabel *lab) const;
	bool visitAcyclic1_9(const EventLabel *lab) const;
	bool visitAcyclic1_10(const EventLabel *lab) const;
	bool visitAcyclic1_11(const EventLabel *lab) const;
	bool visitAcyclic1_12(const EventLabel *lab) const;

	bool isAcyclic1(const EventLabel *lab) const;

	mutable std::vector<NodeCountStatus> visitedAcyclic1_0;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_1;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_2;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_3;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_4;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_5;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_6;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_7;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_8;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_9;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_10;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_11;
	mutable std::vector<NodeCountStatus> visitedAcyclic1_12;

	mutable uint16_t visitedAccepting1 = 0;
	bool shouldVisitAcyclic1(void) const { return true; };

	bool isRecAcyclic(const EventLabel *lab) const;

	mutable uint16_t visitedRecAccepting = 0;
	void visitPPoRf0(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf1(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf2(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf3(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf4(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf5(const EventLabel *lab, DepView &pporf) const;
	void visitPPoRf6(const EventLabel *lab, DepView &pporf) const;

	DepView calcPPoRfBefore(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedPPoRf0;
	mutable std::vector<NodeStatus> visitedPPoRf1;
	mutable std::vector<NodeStatus> visitedPPoRf2;
	mutable std::vector<NodeStatus> visitedPPoRf3;
	mutable std::vector<NodeStatus> visitedPPoRf4;
	mutable std::vector<NodeStatus> visitedPPoRf5;
	mutable std::vector<NodeStatus> visitedPPoRf6;

	mutable std::vector<VSet<Event>> saved;
	mutable std::vector<View> views;
};

#endif /* __IMM_DRIVER_HPP__ */
