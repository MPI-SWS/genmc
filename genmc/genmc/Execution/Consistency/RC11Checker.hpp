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

#ifndef GENMC_RC11_CHECKER_HPP
#define GENMC_RC11_CHECKER_HPP

#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/EventLabel.hpp"
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
	RC11Checker(const Config *conf) : ConsistencyChecker(conf) {};

private:
	bool isConsistent(const EventLabel *lab) const override;
	bool isConsistent(const ExecutionGraph &g) const override;
	bool isCoherentRelinche(const ExecutionGraph &g) const override;
	std::optional<VerificationError> checkErrors(const EventLabel *lab, const EventLabel *&race) const override;
	std::vector<VerificationError> checkWarnings(const EventLabel *lab, const VSet<VerificationError> &reported, std::vector<const EventLabel *> &races) const override;
	std::vector<EventLabel *> getCoherentStores(ReadLabel *rLab) override;
	void filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls) override;
	std::vector<EventLabel *> getCoherentPlacings(WriteLabel *sLab) override;
	void updateMMViews(EventLabel *lab) override;
	std::unique_ptr<VectorClock> calculatePrefixView(const EventLabel *lab) const override;
	bool isDepTracking() const override;
	void calculateSaved(EventLabel *lab);
	void calculateViews(EventLabel *lab);
	mutable const EventLabel *cexLab{};

	mutable std::vector<NodeStatus> visitedCalc72_0;
	mutable std::vector<NodeStatus> visitedCalc72_1;
	mutable std::vector<NodeStatus> visitedCalc72_2;

	bool visitCalc72_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc72_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc72_2(const EventLabel *lab, View &calcRes) const;

	View visitCalc72(const EventLabel *lab) const;
	auto getPorfView(const EventLabel *lab) const -> const View & { return lab->view(0); }

	auto checkCalc72(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCalc78_0;
	mutable std::vector<NodeStatus> visitedCalc78_1;
	mutable std::vector<NodeStatus> visitedCalc78_2;
	mutable std::vector<NodeStatus> visitedCalc78_3;
	mutable std::vector<NodeStatus> visitedCalc78_4;
	mutable std::vector<NodeStatus> visitedCalc78_5;
	mutable std::vector<NodeStatus> visitedCalc78_6;

	bool visitCalc78_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc78_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc78_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc78_3(const EventLabel *lab, View &calcRes) const;
	bool visitCalc78_4(const EventLabel *lab, View &calcRes) const;
	bool visitCalc78_5(const EventLabel *lab, View &calcRes) const;
	bool visitCalc78_6(const EventLabel *lab, View &calcRes) const;

	View visitCalc78(const EventLabel *lab) const;
	auto getHbView(const EventLabel *lab) const -> const View & { return lab->view(1); }

	auto checkCalc78(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedCalc79_0;
	mutable std::vector<NodeStatus> visitedCalc79_1;
	mutable std::vector<NodeStatus> visitedCalc79_2;
	mutable std::vector<NodeStatus> visitedCalc79_3;
	mutable std::vector<NodeStatus> visitedCalc79_4;
	mutable std::vector<NodeStatus> visitedCalc79_5;
	mutable std::vector<NodeStatus> visitedCalc79_6;

	bool visitCalc79_0(const EventLabel *lab, View &calcRes) const;
	bool visitCalc79_1(const EventLabel *lab, View &calcRes) const;
	bool visitCalc79_2(const EventLabel *lab, View &calcRes) const;
	bool visitCalc79_3(const EventLabel *lab, View &calcRes) const;
	bool visitCalc79_4(const EventLabel *lab, View &calcRes) const;
	bool visitCalc79_5(const EventLabel *lab, View &calcRes) const;
	bool visitCalc79_6(const EventLabel *lab, View &calcRes) const;

	View visitCalc79(const EventLabel *lab) const;
	auto getHbRelincheView(const EventLabel *lab) const -> const View & { return lab->view(2); }

	auto checkCalc79(const EventLabel *lab) const;
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

	bool visitCoherenceRelinche(const ExecutionGraph &g) const;

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
	bool checkConsAcyclic1(const ExecutionGraph &g) const;
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

	bool visitLHSUnlessError4_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError4_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError4Accepting;
	bool visitUnlessError4(const EventLabel *lab) const;
	bool checkError4(const EventLabel *lab) const;
	bool visitError5(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError5_1;

	bool visitLHSUnlessError5_0(const EventLabel *lab) const;
	bool visitLHSUnlessError5_1(const EventLabel *lab) const;



	mutable std::vector<bool> visitedLHSUnlessError5Accepting;
	mutable std::vector<bool> visitedRHSUnlessError5Accepting;
	bool visitUnlessError5(const EventLabel *lab) const;
	bool checkError5(const EventLabel *lab) const;
	bool visitError6(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError6_1;

	bool visitLHSUnlessError6_0(const EventLabel *lab, const View &v) const;
	bool visitLHSUnlessError6_1(const EventLabel *lab, const View &v) const;

	mutable std::vector<bool> visitedLHSUnlessError6Accepting;
	bool visitUnlessError6(const EventLabel *lab) const;
	bool checkError6(const EventLabel *lab) const;
	bool visitError7(const EventLabel *lab) const;
	mutable std::vector<NodeStatus> visitedLHSUnlessError7_0;
	mutable std::vector<NodeStatus> visitedLHSUnlessError7_1;

	bool visitLHSUnlessError7_0(const EventLabel *lab) const;
	bool visitLHSUnlessError7_1(const EventLabel *lab) const;



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
