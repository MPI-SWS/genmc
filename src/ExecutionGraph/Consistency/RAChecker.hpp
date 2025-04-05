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

#ifndef GENMC_RA_CHECKER_HPP
#define GENMC_RA_CHECKER_HPP

#include "ExecutionGraph/Consistency/ConsistencyChecker.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include <cstdint>
#include <vector>

class RAChecker : public ConsistencyChecker {

private:
	enum class NodeStatus : unsigned char { unseen, entered, left };

	struct NodeVisitStatus {
		NodeVisitStatus() = default;
		NodeVisitStatus(uint32_t c, NodeStatus s) : count(c), status(s) {}
		uint32_t count{};
		NodeStatus status{};
	};

public:
	RAChecker(const Config *conf) : ConsistencyChecker(conf) {};

private:
	bool isConsistent(const EventLabel *lab) const override;
	bool isConsistent(const ExecutionGraph &g) const override;
	bool isCoherentRelinche(const ExecutionGraph &g) const override;
	VerificationError checkErrors(const EventLabel *lab, const EventLabel *&race) const;
	std::vector<VerificationError> checkWarnings(const EventLabel *lab, const VSet<VerificationError> &reported, std::vector<const EventLabel *> &races) const override;
	std::vector<EventLabel *> getCoherentStores(ReadLabel *rLab) override;
	void filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls) override;
	std::vector<EventLabel *> getCoherentPlacings(WriteLabel *sLab) override;
	void updateMMViews(EventLabel *lab) override;
	std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const override;
	const View &getHbView(const EventLabel *lab) const override;
	bool isDepTracking() const;
	void calculateSaved(EventLabel *lab);
	void calculateViews(EventLabel *lab);
	mutable const EventLabel *cexLab{};

	mutable std::vector<NodeStatus> visitedCalc61_0;
	mutable std::vector<NodeStatus> visitedCalc61_1;
	mutable std::vector<NodeStatus> visitedCalc61_2;
	mutable std::vector<NodeStatus> visitedCalc61_3;

	bool visitCalc61_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc61_3(const EventLabel *lab, View &calcRes) const;

	View visitCalc61(const EventLabel *lab) const;
	const View&getPorfStableView(const EventLabel *lab) const { return lab->view(0); }

	auto checkCalc61(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCalc67_0;
	mutable std::vector<NodeStatus> visitedCalc67_1;
	mutable std::vector<NodeStatus> visitedCalc67_2;
	mutable std::vector<NodeStatus> visitedCalc67_3;
	mutable std::vector<NodeStatus> visitedCalc67_4;
	mutable std::vector<NodeStatus> visitedCalc67_5;
	mutable std::vector<NodeStatus> visitedCalc67_6;
	mutable std::vector<NodeStatus> visitedCalc67_7;

	bool visitCalc67_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc67_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc67_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc67_3(const EventLabel *lab, View &calcRes) const;
	bool visitCalc67_4(const EventLabel *lab, View &calcRes) const;
	bool visitCalc67_5(const EventLabel *lab, View &calcRes) const;
	bool visitCalc67_6(const EventLabel *lab, View &calcRes) const;
	bool visitCalc67_7(const EventLabel *lab, View &calcRes) const;

	View visitCalc67(const EventLabel *lab) const;
	const View&getHbStableView(const EventLabel *lab) const { return lab->view(1); }

	auto checkCalc67(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCalc69_0;
	mutable std::vector<NodeStatus> visitedCalc69_1;
	mutable std::vector<NodeStatus> visitedCalc69_2;
	mutable std::vector<NodeStatus> visitedCalc69_3;
	mutable std::vector<NodeStatus> visitedCalc69_4;
	mutable std::vector<NodeStatus> visitedCalc69_5;
	mutable std::vector<NodeStatus> visitedCalc69_6;
	mutable std::vector<NodeStatus> visitedCalc69_7;

	bool visitCalc69_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc69_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc69_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc69_3(const EventLabel *lab, View &calcRes) const;
	bool visitCalc69_4(const EventLabel *lab, View &calcRes) const;
	bool visitCalc69_5(const EventLabel *lab, View &calcRes) const;
	bool visitCalc69_6(const EventLabel *lab, View &calcRes) const;
	bool visitCalc69_7(const EventLabel *lab, View &calcRes) const;

	View visitCalc69(const EventLabel *lab) const;
	const View&getHbRelincheView(const EventLabel *lab) const { return lab->view(2); }

	auto checkCalc69(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCoherence_0;
	mutable std::vector<NodeStatus> visitedCoherence_1;
	mutable std::vector<NodeStatus> visitedCoherence_2;
	mutable std::vector<NodeStatus> visitedCoherence_3;
	mutable std::vector<NodeStatus> visitedCoherence_4;
	mutable std::vector<NodeStatus> visitedCoherence_5;
	mutable std::vector<NodeStatus> visitedCoherence_6;
	mutable std::vector<NodeStatus> visitedCoherence_7;
	mutable std::vector<NodeStatus> visitedCoherence_8;
	mutable std::vector<NodeStatus> visitedCoherence_9;
	mutable std::vector<NodeStatus> visitedCoherence_10;
	mutable std::vector<NodeStatus> visitedCoherence_11;
	mutable std::vector<NodeStatus> visitedCoherence_12;
	mutable std::vector<NodeStatus> visitedCoherence_13;
	mutable std::vector<NodeStatus> visitedCoherence_14;

	bool visitCoherence_0(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_1(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_2(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_3(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_4(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_5(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_6(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_7(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_8(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_9(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_10(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_11(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_12(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_13(const EventLabel *lab, const EventLabel *initLab) const;
	bool visitCoherence_14(const EventLabel *lab, const EventLabel *initLab) const;

	bool visitCoherenceRelinche(const ExecutionGraph &g) const;

	bool visitError1(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError1_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError1_1;

	bool visitLHSUnlessError1_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError1_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError1Accepting;
	bool visitUnlessError1(const EventLabel *lab) const;
	bool checkError1(const EventLabel *lab) const;
	bool visitError2(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError2_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError2_1;

	bool visitLHSUnlessError2_0(const EventLabel *lab) const;
	bool visitLHSUnlessError2_1(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessError2Accepting;
	mutable std::vector<bool> visitedRHSUnlessError2Accepting;
	bool visitUnlessError2(const EventLabel *lab) const;
	bool checkError2(const EventLabel *lab) const;
	bool visitError3(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError3_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError3_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError3_2;

	bool visitLHSUnlessError3_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError3_1(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError3_2(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError3Accepting;
	bool visitUnlessError3(const EventLabel *lab) const;
	bool checkError3(const EventLabel *lab) const;
	bool visitError4(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError4_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError4_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError4_2;

	bool visitLHSUnlessError4_0(const EventLabel *lab) const;
	bool visitLHSUnlessError4_1(const EventLabel *lab) const;
	bool visitLHSUnlessError4_2(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessError4Accepting;
	mutable std::vector<bool> visitedRHSUnlessError4Accepting;
	bool visitUnlessError4(const EventLabel *lab) const;
	bool checkError4(const EventLabel *lab) const;
	bool visitError5(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_2;

	bool visitLHSUnlessError5_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError5_1(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError5_2(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError5Accepting;
	bool visitUnlessError5(const EventLabel *lab) const;
	bool checkError5(const EventLabel *lab) const;
	bool visitError6(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_1;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_2;

	bool visitLHSUnlessError6_0(const EventLabel *lab) const;
	bool visitLHSUnlessError6_1(const EventLabel *lab) const;
	bool visitLHSUnlessError6_2(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessError6Accepting;
	mutable std::vector<bool> visitedRHSUnlessError6Accepting;
	bool visitUnlessError6(const EventLabel *lab) const;
	bool checkError6(const EventLabel *lab) const;
	bool visitError7(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError7_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError7_1;

	bool visitLHSUnlessError7_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError7_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError7Accepting;
	bool visitUnlessError7(const EventLabel *lab) const;
	bool checkError7(const EventLabel *lab) const;
	bool visitWarning8(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessWarning8_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessWarning8_1;

	bool visitLHSUnlessWarning8_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessWarning8_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessWarning8Accepting;
	bool visitUnlessWarning8(const EventLabel *lab) const;
	bool checkWarning8(const EventLabel *lab) const;

	void visitPPoRf0(const EventLabel *lab, View &pporf) const;
	void visitPPoRf1(const EventLabel *lab, View &pporf) const;

	View calcPPoRfBefore(const EventLabel *lab) const;

	mutable std::vector<NodeStatus> visitedPPoRf0;
	mutable std::vector<NodeStatus> visitedPPoRf1;


};

#endif /* GENMC_RA_CHECKER_HPP */
