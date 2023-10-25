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

#include "config.h"
#include "ExecutionGraph.hpp"
#include "GenMCDriver.hpp"
#include "GraphIterators.hpp"
#include "MaximalIterator.hpp"
#include "VerificationError.hpp"
#include "VSet.hpp"
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
		std::unique_ptr<ModuleInfo> MI, GenMCDriver::Mode mode = GenMCDriver::VerificationMode{});

	std::vector<VSet<Event>> calculateSaved(const EventLabel *lab);
	std::vector<View> calculateViews(const EventLabel *lab);
	void updateMMViews(EventLabel *lab) override;
	bool isDepTracking() const override;
	bool isConsistent(const EventLabel *lab) const override;
	VerificationError checkErrors(const EventLabel *lab, const EventLabel *&race) const override;
	std::vector<VerificationError> checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings, std::vector<const EventLabel *> &racyLabs) const;
	bool isRecoveryValid(const EventLabel *lab) const override;
	std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const override;
	const View &getHbView(const EventLabel *lab) const override;
	std::vector<Event> getCoherentStores(SAddr addr, Event read) override;
	std::vector<Event> getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf) override;
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

	bool visitAcyclic0(const EventLabel *lab) const;
	bool visitAcyclic1(const EventLabel *lab) const;
	bool visitAcyclic2(const EventLabel *lab) const;
	bool visitAcyclic3(const EventLabel *lab) const;
	bool visitAcyclic4(const EventLabel *lab) const;
	bool visitAcyclic5(const EventLabel *lab) const;
	bool visitAcyclic6(const EventLabel *lab) const;
	bool visitAcyclic7(const EventLabel *lab) const;
	bool visitAcyclic8(const EventLabel *lab) const;
	bool visitAcyclic9(const EventLabel *lab) const;
	bool visitAcyclic10(const EventLabel *lab) const;
	bool visitAcyclic11(const EventLabel *lab) const;
	bool visitAcyclic12(const EventLabel *lab) const;
	bool visitAcyclic13(const EventLabel *lab) const;
	bool visitAcyclic14(const EventLabel *lab) const;
	bool visitAcyclic15(const EventLabel *lab) const;
	bool visitAcyclic16(const EventLabel *lab) const;
	bool visitAcyclic17(const EventLabel *lab) const;
	bool visitAcyclic18(const EventLabel *lab) const;
	bool visitAcyclic19(const EventLabel *lab) const;
	bool visitAcyclic20(const EventLabel *lab) const;
	bool visitAcyclic21(const EventLabel *lab) const;
	bool visitAcyclic22(const EventLabel *lab) const;
	bool visitAcyclic23(const EventLabel *lab) const;
	bool visitAcyclic24(const EventLabel *lab) const;
	bool visitAcyclic25(const EventLabel *lab) const;
	bool visitAcyclic26(const EventLabel *lab) const;
	bool visitAcyclic27(const EventLabel *lab) const;
	bool visitAcyclic28(const EventLabel *lab) const;

	bool isAcyclic(const EventLabel *lab) const ;

	mutable std::vector<NodeCountStatus> visitedAcyclic0;
	mutable std::vector<NodeCountStatus> visitedAcyclic1;
	mutable std::vector<NodeCountStatus> visitedAcyclic2;
	mutable std::vector<NodeCountStatus> visitedAcyclic3;
	mutable std::vector<NodeCountStatus> visitedAcyclic4;
	mutable std::vector<NodeCountStatus> visitedAcyclic5;
	mutable std::vector<NodeCountStatus> visitedAcyclic6;
	mutable std::vector<NodeCountStatus> visitedAcyclic7;
	mutable std::vector<NodeCountStatus> visitedAcyclic8;
	mutable std::vector<NodeCountStatus> visitedAcyclic9;
	mutable std::vector<NodeCountStatus> visitedAcyclic10;
	mutable std::vector<NodeCountStatus> visitedAcyclic11;
	mutable std::vector<NodeCountStatus> visitedAcyclic12;
	mutable std::vector<NodeCountStatus> visitedAcyclic13;
	mutable std::vector<NodeCountStatus> visitedAcyclic14;
	mutable std::vector<NodeCountStatus> visitedAcyclic15;
	mutable std::vector<NodeCountStatus> visitedAcyclic16;
	mutable std::vector<NodeCountStatus> visitedAcyclic17;
	mutable std::vector<NodeCountStatus> visitedAcyclic18;
	mutable std::vector<NodeCountStatus> visitedAcyclic19;
	mutable std::vector<NodeCountStatus> visitedAcyclic20;
	mutable std::vector<NodeCountStatus> visitedAcyclic21;
	mutable std::vector<NodeCountStatus> visitedAcyclic22;
	mutable std::vector<NodeCountStatus> visitedAcyclic23;
	mutable std::vector<NodeCountStatus> visitedAcyclic24;
	mutable std::vector<NodeCountStatus> visitedAcyclic25;
	mutable std::vector<NodeCountStatus> visitedAcyclic26;
	mutable std::vector<NodeCountStatus> visitedAcyclic27;
	mutable std::vector<NodeCountStatus> visitedAcyclic28;

	mutable uint16_t visitedAccepting = 0;

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
