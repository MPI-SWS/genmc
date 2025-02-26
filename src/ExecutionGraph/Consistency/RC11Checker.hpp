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

#ifndef GENMC_RC11_CHECKER_HPP
#define GENMC_RC11_CHECKER_HPP

#include "ExecutionGraph/Consistency/ConsistencyChecker.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include <cstdint>
#include <vector>

class RC11Checker : public ConsistencyChecker {

private:
	enum class NodeStatus : unsigned char { unseen, entered, left };

	struct NodeVisitStatus {
		NodeVisitStatus() = default;
		NodeVisitStatus(uint32_t c, NodeStatus s) : count(c), status(s) {}
		uint32_t count{};
		NodeStatus status{};
	};

public:
	RC11Checker() {};

private:
	bool isConsistent(const EventLabel *lab) const override;
	VerificationError checkErrors(const EventLabel *lab, const EventLabel *&race) const;
	std::vector<VerificationError> checkWarnings(const EventLabel *lab, const VSet<VerificationError> &reported, std::vector<const EventLabel *> &races) const override;
	std::vector<EventLabel *> getCoherentStores(ReadLabel *rLab) override;
	std::vector<ReadLabel *> getCoherentRevisits(WriteLabel *sLab, const VectorClock &pporf) override;
	std::vector<EventLabel *> getCoherentPlacings(WriteLabel *sLab) override;
	void updateMMViews(EventLabel *lab) override;
	std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const override;
	const View &getHbView(const EventLabel *lab) const override;
	bool isDepTracking() const;
	void calculateSaved(EventLabel *lab);
	void calculateViews(EventLabel *lab);
	mutable const EventLabel *cexLab{};

	mutable std::vector<NodeStatus> visitedCalc58_0;
	mutable std::vector<NodeStatus> visitedCalc58_1;
	mutable std::vector<NodeStatus> visitedCalc58_2;
	mutable std::vector<NodeStatus> visitedCalc58_3;

	bool visitCalc58_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc58_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc58_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc58_3(const EventLabel *lab, View &calcRes) const;

	View visitCalc58(const EventLabel *lab) const;
	const View&getPorfStableView(const EventLabel *lab) const { return lab->view(0); }

	auto checkCalc58(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCalc64_0;
	mutable std::vector<NodeStatus> visitedCalc64_1;
	mutable std::vector<NodeStatus> visitedCalc64_2;
	mutable std::vector<NodeStatus> visitedCalc64_3;
	mutable std::vector<NodeStatus> visitedCalc64_4;
	mutable std::vector<NodeStatus> visitedCalc64_5;
	mutable std::vector<NodeStatus> visitedCalc64_6;
	mutable std::vector<NodeStatus> visitedCalc64_7;

	bool visitCalc64_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc64_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc64_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc64_3(const EventLabel *lab, View &calcRes) const;
	bool visitCalc64_4(const EventLabel *lab, View &calcRes) const;
	bool visitCalc64_5(const EventLabel *lab, View &calcRes) const;
	bool visitCalc64_6(const EventLabel *lab, View &calcRes) const;
	bool visitCalc64_7(const EventLabel *lab, View &calcRes) const;

	View visitCalc64(const EventLabel *lab) const;
	const View&getHbStableView(const EventLabel *lab) const { return lab->view(1); }

	auto checkCalc64(const EventLabel *lab) const;
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

	bool visitCoherenceFull(const ExecutionGraph &g) const;

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

	bool visitConsAcyclic1Full(const ExecutionGraph &g) const;

	mutable std::vector<NodeStatus> visitedLHSUnlessConsAcyclic1_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessConsAcyclic1_1;

	bool visitLHSUnlessConsAcyclic1_0(const EventLabel *lab) const;
	bool visitLHSUnlessConsAcyclic1_1(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessConsAcyclic1Accepting;
	mutable std::vector<bool> visitedRHSUnlessConsAcyclic1Accepting;
	bool visitUnlessConsAcyclic1(const EventLabel *lab) const;
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

#endif /* GENMC_RC11_CHECKER_HPP */
