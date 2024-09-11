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

#ifndef GENMC_SC_DRIVER_HPP
#define GENMC_SC_DRIVER_HPP

#include "config.h"
#include "ADT/VSet.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/MaximalIterator.hpp"
#include "Verification/GenMCDriver.hpp"
#include "Verification/VerificationError.hpp"
#include <cstdint>
#include <vector>

class SCDriver : public GenMCDriver {

private:
	enum class NodeStatus : unsigned char { unseen, entered, left };

	struct NodeVisitStatus {
		NodeVisitStatus() = default;
		NodeVisitStatus(uint32_t c, NodeStatus s) : count(c), status(s) {}
		uint32_t count{};
		NodeStatus status{};
	};

public:
	SCDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
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

	mutable std::vector<NodeStatus> visitedCalc57_0;
	mutable std::vector<NodeStatus> visitedCalc57_1;
	mutable std::vector<NodeStatus> visitedCalc57_2;
	mutable std::vector<NodeStatus> visitedCalc57_3;

	bool visitCalc57_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc57_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc57_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc57_3(const EventLabel *lab, View &calcRes) const;

	View visitCalc57(const EventLabel *lab) const;
	const View&getHbStableView(const EventLabel *lab) const { return lab->view(0); }

	auto checkCalc57(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCalc62_0;
	mutable std::vector<NodeStatus> visitedCalc62_1;
	mutable std::vector<NodeStatus> visitedCalc62_2;
	mutable std::vector<NodeStatus> visitedCalc62_3;

	bool visitCalc62_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc62_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc62_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc62_3(const EventLabel *lab, View &calcRes) const;

	View visitCalc62(const EventLabel *lab) const;
	const View&getPorfStableView(const EventLabel *lab) const { return lab->view(1); }

	auto checkCalc62(const EventLabel *lab) const;
	mutable std::vector<NodeVisitStatus> visitedCoherence_0;
	mutable std::vector<NodeVisitStatus> visitedCoherence_1;
	mutable std::vector<NodeVisitStatus> visitedCoherence_2;
	mutable uint32_t visitedCoherenceAccepting;

	bool visitCoherence_0(const EventLabel *lab) const;
	bool visitCoherence_1(const EventLabel *lab) const;
	bool visitCoherence_2(const EventLabel *lab) const;

	bool visitCoherenceFull() const;

	mutable std::vector<NodeVisitStatus> visitedConsAcyclic1_0;
	mutable uint32_t visitedConsAcyclic1Accepting;

	bool visitConsAcyclic1_0(const EventLabel *lab) const;

	bool visitConsAcyclic1(const EventLabel *lab) const;

	bool visitConsAcyclic1Full() const;

	bool checkUnlessConsAcyclic1(const EventLabel *lab) { return false; }
	bool checkConsAcyclic1(const EventLabel *lab) const;
	bool visitError2(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError2_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError2_1;

	bool visitLHSUnlessError2_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError2_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError2Accepting;
	bool visitUnlessError2(const EventLabel *lab) const;
	bool checkError2(const EventLabel *lab) const;
	bool visitError3(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError3_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError3_1;

	bool visitLHSUnlessError3_0(const EventLabel *lab) const;
	bool visitLHSUnlessError3_1(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessError3Accepting;
	mutable std::vector<bool> visitedRHSUnlessError3Accepting;
	bool visitUnlessError3(const EventLabel *lab) const;
	bool checkError3(const EventLabel *lab) const;
	bool visitError4(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError4_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError4_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError4_2;

	bool visitLHSUnlessError4_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError4_1(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError4_2(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError4Accepting;
	bool visitUnlessError4(const EventLabel *lab) const;
	bool checkError4(const EventLabel *lab) const;
	bool visitError5(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_2;

	bool visitLHSUnlessError5_0(const EventLabel *lab) const;
	bool visitLHSUnlessError5_1(const EventLabel *lab) const;
	bool visitLHSUnlessError5_2(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessError5Accepting;
	mutable std::vector<bool> visitedRHSUnlessError5Accepting;
	bool visitUnlessError5(const EventLabel *lab) const;
	bool checkError5(const EventLabel *lab) const;
	bool visitError6(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_2;

	bool visitLHSUnlessError6_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError6_1(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError6_2(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError6Accepting;
	bool visitUnlessError6(const EventLabel *lab) const;
	bool checkError6(const EventLabel *lab) const;
	bool visitError7(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError7_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError7_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError7_2;

	bool visitLHSUnlessError7_0(const EventLabel *lab) const;
	bool visitLHSUnlessError7_1(const EventLabel *lab) const;
	bool visitLHSUnlessError7_2(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessError7Accepting;
	mutable std::vector<bool> visitedRHSUnlessError7Accepting;
	bool visitUnlessError7(const EventLabel *lab) const;
	bool checkError7(const EventLabel *lab) const;
	bool visitError8(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError8_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError8_1;

	bool visitLHSUnlessError8_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError8_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError8Accepting;
	bool visitUnlessError8(const EventLabel *lab) const;
	bool checkError8(const EventLabel *lab) const;
	bool visitWarning9(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessWarning9_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessWarning9_1;

	bool visitLHSUnlessWarning9_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessWarning9_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessWarning9Accepting;
	bool visitUnlessWarning9(const EventLabel *lab) const;
	bool checkWarning9(const EventLabel *lab) const;

	void visitPPoRf0(const EventLabel *lab, View &pporf) const;
	void visitPPoRf1(const EventLabel *lab, View &pporf) const;

	View calcPPoRfBefore(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedPPoRf0;
	mutable std::vector<NodeStatus> visitedPPoRf1;


};

#endif /* GENMC_SC_DRIVER_HPP */
