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

#ifndef GENMC_IMM_DRIVER_HPP
#define GENMC_IMM_DRIVER_HPP

#include "config.h"
#include "ADT/VSet.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/MaximalIterator.hpp"
#include "Verification/GenMCDriver.hpp"
#include "Verification/VerificationError.hpp"
#include <cstdint>
#include <vector>

class IMMDriver : public GenMCDriver {

private:
	enum class NodeStatus : unsigned char { unseen, entered, left };

	struct NodeVisitStatus {
		NodeVisitStatus() = default;
		NodeVisitStatus(uint32_t c, NodeStatus s) : count(c), status(s) {}
		uint32_t count{};
		NodeStatus status{};
	};

public:
	IMMDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		std::unique_ptr<ModuleInfo> MI, GenMCDriver::Mode mode = GenMCDriver::VerificationMode{});

	void calculateSaved(EventLabel *lab);
	void calculateViews(EventLabel *lab);
	void updateMMViews(EventLabel *lab) override;
	bool isDepTracking() const override;
	bool isConsistent(const EventLabel *lab) const override;
	VerificationError checkErrors(const EventLabel *lab, const EventLabel *&race) const override;
	std::vector<VerificationError> checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings, std::vector<const EventLabel *> &racyLabs) const;
	std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const override;
	const View &getHbView(const EventLabel *lab) const override;
	std::vector<Event> getCoherentStores(SAddr addr, Event read) override;
	std::vector<Event> getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf) override;
	std::vector<Event> getCoherentPlacings(SAddr addr, Event store, bool isRMW) override;

private:
	bool isWriteRfBefore(Event a, Event b);
	std::vector<Event> getInitRfsAtLoc(SAddr addr);
	bool isHbOptRfBefore(const Event e, const Event write);
	ExecutionGraph::co_iterator splitLocMOBefore(SAddr addr, Event e);
	ExecutionGraph::co_iterator splitLocMOAfterHb(SAddr addr, const Event read);
	ExecutionGraph::co_iterator splitLocMOAfter(SAddr addr, const Event e);
	std::vector<Event> getMOOptRfAfter(const WriteLabel *sLab);
	std::vector<Event> getMOInvOptRfAfter(const WriteLabel *sLab);
	mutable const EventLabel *cexLab{};

	mutable std::vector<NodeStatus> visitedCalc61_0;
	mutable std::vector<NodeStatus> visitedCalc61_1;
	mutable std::vector<NodeStatus> visitedCalc61_2;
	mutable std::vector<NodeStatus> visitedCalc61_3;
	mutable std::vector<NodeStatus> visitedCalc61_4;
	mutable std::vector<NodeStatus> visitedCalc61_5;
	mutable std::vector<NodeStatus> visitedCalc61_6;
	mutable std::vector<NodeStatus> visitedCalc61_7;

	bool visitCalc61_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_3(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_4(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_5(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_6(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_7(const EventLabel *lab, View &calcRes) const;

	View visitCalc61(const EventLabel *lab) const;
	const View&getHbStableView(const EventLabel *lab) const { return lab->view(0); }

	auto checkCalc61(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCalc73_0;
	mutable std::vector<NodeStatus> visitedCalc73_1;
	mutable std::vector<NodeStatus> visitedCalc73_2;
	mutable std::vector<NodeStatus> visitedCalc73_3;

	bool visitCalc73_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc73_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc73_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc73_3(const EventLabel *lab, View &calcRes) const;

	View visitCalc73(const EventLabel *lab) const;
	const View&getPorfStableView(const EventLabel *lab) const { return lab->view(1); }

	auto checkCalc73(const EventLabel *lab) const;
	mutable std::vector<NodeVisitStatus> visitedCoherence_0;
	mutable std::vector<NodeVisitStatus> visitedCoherence_1;
	mutable std::vector<NodeVisitStatus> visitedCoherence_2;
	mutable std::vector<NodeVisitStatus> visitedCoherence_3;
	mutable std::vector<NodeVisitStatus> visitedCoherence_4;
	mutable std::vector<NodeVisitStatus> visitedCoherence_5;
	mutable std::vector<NodeVisitStatus> visitedCoherence_6;
	mutable uint32_t visitedCoherenceAccepting;

	bool visitCoherence_0(const EventLabel *lab) const;
	bool visitCoherence_1(const EventLabel *lab) const;
	bool visitCoherence_2(const EventLabel *lab) const;
	bool visitCoherence_3(const EventLabel *lab) const;
	bool visitCoherence_4(const EventLabel *lab) const;
	bool visitCoherence_5(const EventLabel *lab) const;
	bool visitCoherence_6(const EventLabel *lab) const;

	bool visitCoherenceFull() const;

	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_0;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_1;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_2;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_3;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_4;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_5;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_6;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_7;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_8;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_9;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_10;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_11;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_12;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_13;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_14;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_15;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_16;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_17;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_18;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_19;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_20;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_21;
	mutable uint32_t visitedConsAcyclic1Accepting;

	bool visitConsAcyclic1_0(const EventLabel *lab) const;
	bool visitConsAcyclic1_1(const EventLabel *lab) const;
	bool visitConsAcyclic1_2(const EventLabel *lab) const;
	bool visitConsAcyclic1_3(const EventLabel *lab) const;
	bool visitConsAcyclic1_4(const EventLabel *lab) const;
	bool visitConsAcyclic1_5(const EventLabel *lab) const;
	bool visitConsAcyclic1_6(const EventLabel *lab) const;
	bool visitConsAcyclic1_7(const EventLabel *lab) const;
	bool visitConsAcyclic1_8(const EventLabel *lab) const;
	bool visitConsAcyclic1_9(const EventLabel *lab) const;
	bool visitConsAcyclic1_10(const EventLabel *lab) const;
	bool visitConsAcyclic1_11(const EventLabel *lab) const;
	bool visitConsAcyclic1_12(const EventLabel *lab) const;
	bool visitConsAcyclic1_13(const EventLabel *lab) const;
	bool visitConsAcyclic1_14(const EventLabel *lab) const;
	bool visitConsAcyclic1_15(const EventLabel *lab) const;
	bool visitConsAcyclic1_16(const EventLabel *lab) const;
	bool visitConsAcyclic1_17(const EventLabel *lab) const;
	bool visitConsAcyclic1_18(const EventLabel *lab) const;
	bool visitConsAcyclic1_19(const EventLabel *lab) const;
	bool visitConsAcyclic1_20(const EventLabel *lab) const;
	bool visitConsAcyclic1_21(const EventLabel *lab) const;

	bool visitConsAcyclic1(const EventLabel *lab) const;

	bool visitConsAcyclic1Full() const;

	mutable std::vector<NodeStatus> visitedLHSUnlessConsAcyclic1_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessConsAcyclic1_1;

	bool visitLHSUnlessConsAcyclic1_0(const EventLabel *lab) const;
	bool visitLHSUnlessConsAcyclic1_1(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessConsAcyclic1Accepting;
	mutable std::vector<bool> visitedRHSUnlessConsAcyclic1Accepting;
	bool visitUnlessConsAcyclic1(const EventLabel *lab) const;
	bool checkConsAcyclic1(const EventLabel *lab) const;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_0;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_1;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_2;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_3;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_4;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_5;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_6;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_7;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_8;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_9;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_10;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_11;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_12;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_13;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_14;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_15;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_16;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_17;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_18;
	mutable std::vector<NodeVisitStatus> visitedConsAcyclic2_19;
	mutable uint32_t visitedConsAcyclic2Accepting;

	bool visitConsAcyclic2_0(const EventLabel *lab) const;
	bool visitConsAcyclic2_1(const EventLabel *lab) const;
	bool visitConsAcyclic2_2(const EventLabel *lab) const;
	bool visitConsAcyclic2_3(const EventLabel *lab) const;
	bool visitConsAcyclic2_4(const EventLabel *lab) const;
	bool visitConsAcyclic2_5(const EventLabel *lab) const;
	bool visitConsAcyclic2_6(const EventLabel *lab) const;
	bool visitConsAcyclic2_7(const EventLabel *lab) const;
	bool visitConsAcyclic2_8(const EventLabel *lab) const;
	bool visitConsAcyclic2_9(const EventLabel *lab) const;
	bool visitConsAcyclic2_10(const EventLabel *lab) const;
	bool visitConsAcyclic2_11(const EventLabel *lab) const;
	bool visitConsAcyclic2_12(const EventLabel *lab) const;
	bool visitConsAcyclic2_13(const EventLabel *lab) const;
	bool visitConsAcyclic2_14(const EventLabel *lab) const;
	bool visitConsAcyclic2_15(const EventLabel *lab) const;
	bool visitConsAcyclic2_16(const EventLabel *lab) const;
	bool visitConsAcyclic2_17(const EventLabel *lab) const;
	bool visitConsAcyclic2_18(const EventLabel *lab) const;
	bool visitConsAcyclic2_19(const EventLabel *lab) const;

	bool visitConsAcyclic2(const EventLabel *lab) const;

	bool visitConsAcyclic2Full() const;

	bool checkUnlessConsAcyclic2(const EventLabel *lab) { return false; }
	bool checkConsAcyclic2(const EventLabel *lab) const;
	bool visitWarning3(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessWarning3_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessWarning3_1;

	bool visitLHSUnlessWarning3_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessWarning3_1(const EventLabel *lab, const View &v) const;

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

#endif /* GENMC_IMM_DRIVER_HPP */
