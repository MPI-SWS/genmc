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

#include "RC11Checker.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/GraphUtils.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/VerificationError.hpp"

bool RC11Checker::isDepTracking() const
{
	return 0;
}

bool RC11Checker::visitCalc72_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();




	return true;
}

bool RC11Checker::visitCalc72_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (auto pLab = lab; true)if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc72_0(pLab, calcRes)) {
				return false;
		}

	}

	return true;
}

bool RC11Checker::visitCalc72_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (auto pLab = g.tc_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc72_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tj_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc72_0(pLab, calcRes)) {
				return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc72_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc72_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc72_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tc_pred(lab); pLab) {
			if (!visitCalc72_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tj_pred(lab); pLab) {
			if (!visitCalc72_1(pLab, calcRes)) {
				return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
			if (!visitCalc72_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab) {
			if (!visitCalc72_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
			if (!visitCalc72_1(pLab, calcRes)) {
				return false;
		}

	}

	return true;
}

View RC11Checker::visitCalc72(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

calcRes.updateIdx(lab->getPos());


	visitCalc72_2(lab, calcRes);
	return calcRes;
}
auto RC11Checker::checkCalc72(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc72(lab);
}
bool RC11Checker::visitCalc78_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();




	return true;
}

bool RC11Checker::visitCalc78_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (auto pLab = lab; true)if (calcRes.update(pLab->view(1)); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}

	return true;
}

bool RC11Checker::visitCalc78_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (true && lab->isAtLeastAcquire())if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCalc78_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCalc78_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_5(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tc_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tj_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tc_pred(lab); pLab) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tj_pred(lab); pLab) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc78_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_6(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc78_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_6(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitCalc78_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc78_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc78_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc78_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab))if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab))if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}

	visitedCalc78_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc78_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCalc78_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCalc78_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_5(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}

	return true;
}

bool RC11Checker::visitCalc78_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc78_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc78_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && genmc::isa<ReadLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCalc78_4(pLab, calcRes)) {
				return false;
		}

	}

	visitedCalc78_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc78_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc78_6[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc78_6[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCalc78_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCalc78_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_5(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc78_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc78_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc78_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc78_6(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCalc78_6[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

View RC11Checker::visitCalc78(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

calcRes.updateIdx(lab->getPos());

	visitedCalc78_3.clear();
	visitedCalc78_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc78_5.clear();
	visitedCalc78_5.resize(g.getMaxStamp().get() + 1);
	visitedCalc78_6.clear();
	visitedCalc78_6.resize(g.getMaxStamp().get() + 1);

	visitCalc78_2(lab, calcRes);
	return calcRes;
}
auto RC11Checker::checkCalc78(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc78(lab);
}
bool RC11Checker::visitCalc79_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();




	return true;
}

bool RC11Checker::visitCalc79_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (auto pLab = lab; true)if (calcRes.update(pLab->view(2)); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}

	return true;
}

bool RC11Checker::visitCalc79_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCalc79_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_5(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCalc79_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tc_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tj_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tc_pred(lab); pLab) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.tj_pred(lab); pLab) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab) && !(genmc::isa<AbstractLockCasReadLabel>(lab)))if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc79_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_6(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab) && !(genmc::isa<AbstractLockCasReadLabel>(lab)))if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc79_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_6(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadStartLabel>(lab) && !(genmc::isa<AbstractLockCasReadLabel>(lab)))if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc79_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_6(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitCalc79_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc79_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc79_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc79_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab))if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab))if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}

	visitedCalc79_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc79_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();


	if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCalc79_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_5(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCalc79_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}

	return true;
}

bool RC11Checker::visitCalc79_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc79_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc79_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && genmc::isa<ReadLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCalc79_4(pLab, calcRes)) {
				return false;
		}

	}

	visitedCalc79_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc79_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc79_6[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc79_6[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCalc79_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_5(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCalc79_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_3(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease())if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCalc79_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc79_6(pLab, calcRes)) {
				return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCalc79_6[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

View RC11Checker::visitCalc79(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

calcRes.updateIdx(lab->getPos());

	visitedCalc79_3.clear();
	visitedCalc79_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc79_5.clear();
	visitedCalc79_5.resize(g.getMaxStamp().get() + 1);
	visitedCalc79_6.clear();
	visitedCalc79_6.resize(g.getMaxStamp().get() + 1);

	visitCalc79_2(lab, calcRes);
	return calcRes;
}
auto RC11Checker::checkCalc79(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc79(lab);
}
void RC11Checker::calculateSaved(EventLabel *lab)
{
}

void RC11Checker::calculateViews(EventLabel *lab)
{
	lab->setViews({});

	lab->addView(checkCalc72(lab));

	lab->addView(checkCalc78(lab));
if (!getConf()->collectLinSpec && !getConf()->checkLinSpec) lab->addView({}); else
	lab->addView(checkCalc79(lab));
}

void RC11Checker::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
	lab->setPrefixView(calculatePrefixView(lab));
}


static auto isWriteRfBefore(const WriteLabel *wLab, const EventLabel *lab) -> bool
{
	auto &before = lab->view(1);
	return before.contains(wLab->getPos()) ||
	       std::ranges::any_of(wLab->readers(),
				   [&](auto &rLab) { return before.contains(rLab.getPos()); });
}

static auto isHbOptRfBefore(const EventLabel *lab, const WriteLabel *wLab) -> bool
{
	return wLab->view(1).contains(lab->getPos()) ||
	       std::ranges::any_of(wLab->readers(), [&](auto &rLab) {
		       return rLab.view(1).contains(lab->getPos());
	       });
}

static auto splitLocMOBefore(SAddr addr, EventLabel *lab) -> ExecutionGraph::co_iterator
{
	auto &g = *lab->getParent();
	auto rit = std::ranges::find_if(g.rco(addr), [&](auto &oLab)
					{ return isWriteRfBefore(&oLab, lab); });
	/* Convert to forward iterator, but be _really_ careful */
	return (rit == std::ranges::end(g.rco(addr))) ? std::ranges::begin(g.co(addr)) : ++ExecutionGraph::co_iterator(*rit);
}

static auto splitLocMOAfterHb(ReadLabel *rLab) -> ExecutionGraph::co_iterator
{
	auto &g = *rLab->getParent();
	if (std::ranges::any_of(g.getInitLabel()->rfs(rLab->getAddr()),
			[rLab](auto &rfLab) { return rfLab.view(1).contains(rLab->getPos()); }))
		return std::ranges::begin(g.co(rLab->getAddr()));

	auto it = std::ranges::find_if(g.co(rLab->getAddr()),
			       [&](auto &wLab) { return isHbOptRfBefore(rLab, &wLab); });
	if (it == std::ranges::end(g.co(rLab->getAddr())) || it->view(1).contains(rLab->getPos()))
		return it;
	return ++it;
}

static auto splitLocMOAfter(WriteLabel *wLab) -> ExecutionGraph::co_iterator
{
	auto &g = *wLab->getParent();
	return std::ranges::find_if(g.co(wLab->getAddr()),
			    [&](auto &sLab) { return isHbOptRfBefore(wLab, &sLab); });
}

auto RC11Checker::getCoherentStores(ReadLabel *rLab) -> std::vector<EventLabel *>
{
	auto &g = *rLab->getParent();
	std::vector<EventLabel *> stores;

	/* Fastpath: co_max(G) is po-before R */
	auto *comaxLab = g.co_max(rLab->getAddr());
	if (comaxLab->getThread() == rLab->getThread() && comaxLab->getIndex() < rLab->getIndex())
		return {comaxLab};

	/*
	 * If there are no stores (rf?;hb)-before the current event
	 * then we can read read from all concurrent stores and the
	 * initializer store. Otherwise, we can read from all concurrent
	 * stores and the mo-latest of the (rf?;hb)-before stores.
	 */
	auto begIt = splitLocMOBefore(rLab->getAddr(), rLab);
	if (begIt == std::ranges::begin(g.co(rLab->getAddr())))
		stores.push_back(g.getInitLabel());
	else {
		stores.push_back(&*(--begIt));
		++begIt;
	}

	/*
	 * If the model supports out-of-order execution we have to also
	 * account for the possibility the read is hb-before some other
	 * store, or some read that reads from a store.
	 */
	auto endIt = (isDepTracking()) ? splitLocMOAfterHb(rLab) : std::ranges::end(g.co(rLab->getAddr()));
	std::transform(begIt, endIt, std::back_inserter(stores), [&](auto &lab){
		return &lab;
	});
	return stores;
}

static auto getMOOptRfAfter(WriteLabel *sLab) -> std::vector<EventLabel *>
{
	auto &g = *sLab->getParent();
	std::vector<EventLabel *> after;
	std::vector<ReadLabel *> rfAfter;

	for (auto &wLab : g.co_succs(sLab)) {
		after.push_back(&wLab);
		std::ranges::transform(wLab.readers(), std::back_inserter(rfAfter),
			[&](auto &rLab) { return &rLab; });
	}
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after),
		       [](auto *rLab) { return rLab; });
	return after;
}

static auto getMOInvOptRfAfter(WriteLabel *sLab) -> std::vector<EventLabel *>
{
	auto &g = *sLab->getParent();
	std::vector<EventLabel *> after;
	std::vector<ReadLabel *> rfAfter;

	/* First, add (mo;rf?)-before */
	for (auto &wLab : g.co_preds(sLab)) {
		after.push_back(&wLab);
		std::ranges::transform(wLab.readers(), std::back_inserter(rfAfter),
			[&](auto &rLab) { return &rLab; });
	}
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after),
		       [](auto *rLab) { return rLab; });

	/* Then, we add the reader list for the initializer */
	for (auto &rLab : g.getInitLabel()->rfs(sLab->getAddr()))
		after.insert(after.end(), &rLab);
	return after;
}

static auto getRevisitableFrom(WriteLabel *sLab, const VectorClock &pporf, WriteLabel *coPred)
	-> std::vector<ReadLabel *>
{
	auto &g = *sLab->getParent();
	const auto *confLab = findPendingRMW(sLab);
	std::vector<ReadLabel *> loads;

	for (auto &rLab : coPred->readers()) {
		if (!pporf.contains(rLab.getPos()) && rLab.getAddr() == sLab->getAddr() &&
		    rLab.isRevisitable() && rLab.wasAddedMax())
			loads.push_back(&rLab);
	}
	if (confLab)
		loads.erase(std::remove_if(loads.begin(), loads.end(),
					   [&](auto &eLab) {
						   return eLab->getStamp() > confLab->getStamp();
					   }),
			    loads.end());
	return loads;
}

void RC11Checker::filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls)
{
	/* If this store is po- and mo-maximal then we are done */
	auto &g = *sLab->getParent();
	if (!isDepTracking() && sLab == g.co_max(sLab->getAddr()))
		return;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(),
				[&](auto &eLab) {
					auto &before = g.po_imm_pred(eLab)->view(1); // hb;sb
					return std::any_of(
						optRfs.begin(), optRfs.end(), [&](auto &evLab) {
							return before.contains(evLab->getPos());
						});
				}),
		 ls.end());

	/* If out-of-order event addition is not supported, then we are done
	 * due to po-maximality */
	if (!isDepTracking())
		return;

	/* Otherwise, we also have to exclude hb-before loads */
	ls.erase(std::remove_if(ls.begin(), ls.end(),
				[&](auto &eLab) { return sLab->view(1).contains(eLab->getPos()); }),
		 ls.end());

	/* ...and also exclude (mo^-1; rf?; (hb^-1)?; sb^-1)-after reads in the *resulting* graph */
	auto &before = sLab->getPrefixView();
	auto moInvOptRfs = getMOInvOptRfAfter(sLab);
	ls.erase(std::remove_if(
			 ls.begin(), ls.end(),
			 [&](auto &eLab) {
				 auto v = g.getViewFromStamp(eLab->getStamp());
				 v->update(before);
				 return std::any_of(
					 moInvOptRfs.begin(), moInvOptRfs.end(), [&](auto &evLab) {
						 return v->contains(evLab->getPos()) && // stays in graph?
							g.po_imm_pred(evLab)->view(1).contains(eLab->getPos()); // po-pred to check evLab != rLab
					 });
			 }),
		 ls.end());
}

auto RC11Checker::getCoherentPlacings(WriteLabel *wLab)
	-> std::vector<EventLabel *>
{
	auto &g = *wLab->getParent();
	std::vector<EventLabel *> result;

	/* If it is an RMW store, there is only one possible position in MO */
	if (wLab->isRMW()) {
		auto *rLab = genmc::dyn_cast<ReadLabel>(g.po_imm_pred(wLab));
		VERIFY(rLab);
		auto *rfLab = rLab->getRf();
		VERIFY(rfLab);
		result.push_back(rfLab);
		return result;
	}

	/* Otherwise, we calculate the full range and add the store */
	auto rangeBegin = splitLocMOBefore(wLab->getAddr(), wLab);
	auto rangeEnd = (isDepTracking()) ? splitLocMOAfter(wLab) : std::ranges::end(g.co(wLab->getAddr()));
	auto cos = std::ranges::subrange(rangeBegin, rangeEnd) |
		   std::views::filter([&](auto &sLab) { return !sLab.isRMW(); }) |
		   std::views::transform([&](auto &sLab) {
			   auto *pLab = g.co_imm_pred(&sLab);
			   return pLab ? (EventLabel *)pLab : (EventLabel *)g.getInitLabel();
		   });
	std::ranges::copy(cos, std::back_inserter(result));
	result.push_back(rangeEnd == std::ranges::end(g.co(wLab->getAddr()))
				 ? g.co_max(wLab->getAddr())
				 : (!g.co_imm_pred(&*rangeEnd)
					    ? (EventLabel *)g.getInitLabel()
					    : (EventLabel *)g.co_imm_pred(&*rangeEnd)));
	return result;
}
bool RC11Checker::visitCoherence_0(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();


if (lab == initLab) return false;

	return true;
}

bool RC11Checker::visitCoherence_1(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();


	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitCoherence_2(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_6(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(pLab)) {
		auto status = visitedCoherence_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_6(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tc_pred(lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_pred(lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tc_pred(lab); pLab) {
			if (!visitCoherence_0(pLab, initLab)) {
return false;
		}

	}
	if (auto pLab = g.tj_pred(lab); pLab) {
			if (!visitCoherence_0(pLab, initLab)) {
return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
			if (!visitCoherence_0(pLab, initLab)) {
return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
			if (!visitCoherence_0(pLab, initLab)) {
return false;
		}

	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_3(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_6(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
			if (!visitCoherence_0(pLab, initLab)) {
return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCoherence_0(pLab, initLab)) {
return false;
		}

	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_3(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_4(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
			if (!visitCoherence_0(pLab, initLab)) {
return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCoherence_5(pLab, initLab)) {
return false;
		}

	}
	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCoherence_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_3(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_5(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();


	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && genmc::isa<ReadLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitCoherence_6(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_6[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_6[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_6(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_6[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_7(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_7[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_7[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_pred(lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.fr_imm_preds(lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_pred(lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.fr_imm_preds(lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.fr_imm_preds(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_7[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_8(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_8[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_8[lab->getStamp().get()] = NodeStatus::entered;

	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire()) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_12[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_12(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(pLab)) {
		auto status = visitedCoherence_12[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_12(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tc_pred(lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_pred(lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tc_pred(lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_pred(lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_preds(lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_8[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_9(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_9[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_9[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_9[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_9(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_12[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_12(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_9[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_10(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_10[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_10[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.rf_pred(lab); pLab) {
		auto status = visitedCoherence_9[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_9(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && pLab->isAtLeastRelease()) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.rf_pred(lab); pLab)if (true && genmc::isa<WriteLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCoherence_11(pLab, initLab)) {
return false;
		}

	}

	visitedCoherence_10[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_11(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();


	if (auto pLab = g.po_imm_pred(lab); pLab)if (true && genmc::isa<ReadLabel>(pLab) && ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) || (genmc::isa<WriteLabel>(pLab) && genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitCoherence_12(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_12[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_12[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedCoherence_12[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_12(pLab, initLab)) {
return false;
		}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {

		}
	}

	visitedCoherence_12[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherenceRelinche(const ExecutionGraph &g) const
{
	for (auto &lab : g.labels()) {
		if (!genmc::isa<MethodBeginLabel>(&lab)) continue;
		visitedCoherence_2.clear();
		visitedCoherence_2.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_3.clear();
		visitedCoherence_3.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_4.clear();
		visitedCoherence_4.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_6.clear();
		visitedCoherence_6.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_7.clear();
		visitedCoherence_7.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_8.clear();
		visitedCoherence_8.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_9.clear();
		visitedCoherence_9.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_10.clear();
		visitedCoherence_10.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_12.clear();
		visitedCoherence_12.resize(g.getMaxStamp().get() + 1);
		if (true && !visitCoherence_1(&lab, &lab))
			return false;
	}
	return true;
}

bool RC11Checker::visitConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (auto pLab = g.tc_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_1[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.tc_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_1[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_2[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_2[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_3[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (true && genmc::isa<ReadLabel>(lab) && ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) || (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))if (auto pLab = g.po_imm_succ(lab); pLab) {
			if (!visitConsAcyclic1_4(pLab)){
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire())for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_5(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_3[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<WriteLabel>(lab) && ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) || (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitConsAcyclic1_5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_5[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_5(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_5[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_6[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.fr_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tc_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.fr_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.fr_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_6[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_7[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_7[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_8[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.co_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.fr_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && genmc::isa<ReadLabel>(lab) && ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) || (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))if (auto pLab = g.po_imm_succ(lab); pLab) {
			if (!visitConsAcyclic1_9(pLab)){
				return false;
		}

	}
	if (true && lab->isAtLeastAcquire())for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.co_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.fr_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.co_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.fr_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_8[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<WriteLabel>(lab) && ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) || (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitConsAcyclic1_10(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_10[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_10[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_11(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_11[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.tc_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_11[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_12(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_12[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.tc_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_12[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_13(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_13[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.tc_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_13[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_14(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_14[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_14[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_15(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_15[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_17(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && genmc::isa<ReadLabel>(lab) && ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) || (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))if (auto pLab = g.po_imm_succ(lab); pLab) {
			if (!visitConsAcyclic1_16(pLab)){
				return false;
		}

	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire())if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_15[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_16(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<WriteLabel>(lab) && ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) || (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}

	return true;
}

bool RC11Checker::visitConsAcyclic1_17(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_17[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_17(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isAtLeastAcquire() && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_17[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_18(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_18[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };

	if (auto pLab = g.tc_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.tj_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.lin_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.co_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (auto pLab = g.fr_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	visitedConsAcyclic1_18[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1_19(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	++visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_19[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::entered };


	if (true && lab->isAtLeastRelease() && lab->isSC() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())for (auto &tmp : g.rf_succs(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = g.co_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = g.fr_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC()) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab)if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = g.po_imm_succ(lab); pLab) {
			if (!visitConsAcyclic1_0(pLab)){
				return false;
		}

	}
	if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = g.co_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isSC())if (auto pLab = g.fr_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	if (true && lab->isAtLeastRelease() && lab->isSC() && genmc::isa<FenceLabel>(lab))if (auto pLab = g.po_imm_succ(lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)){
				return false;
		}

		} else if (node.status == NodeStatus::entered && (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {

		}
	}
	--visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_19[lab->getStamp().get()] = { visitedConsAcyclic1Accepting, NodeStatus::left };

	return true;
}

bool RC11Checker::visitConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.clear();
	visitedConsAcyclic1_3.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.clear();
	visitedConsAcyclic1_5.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.clear();
	visitedConsAcyclic1_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.clear();
	visitedConsAcyclic1_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.clear();
	visitedConsAcyclic1_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.clear();
	visitedConsAcyclic1_10.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.clear();
	visitedConsAcyclic1_11.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.clear();
	visitedConsAcyclic1_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.clear();
	visitedConsAcyclic1_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.clear();
	visitedConsAcyclic1_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.clear();
	visitedConsAcyclic1_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_17.clear();
	visitedConsAcyclic1_17.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.clear();
	visitedConsAcyclic1_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.clear();
	visitedConsAcyclic1_19.resize(g.getMaxStamp().get() + 1);
	return true
		&& (visitConsAcyclic1_0(lab))
		&& (visitedConsAcyclic1_1[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_1(lab))
		&& (visitedConsAcyclic1_3[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_3(lab))
		&& (visitedConsAcyclic1_6[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_6(lab))
		&& (visitedConsAcyclic1_8[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_8(lab))
		&& (visitedConsAcyclic1_11[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_11(lab))
		&& (visitedConsAcyclic1_12[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_12(lab))
		&& (visitedConsAcyclic1_13[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_13(lab))
		&& (visitedConsAcyclic1_15[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_15(lab))
		&& (visitedConsAcyclic1_18[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_18(lab))
		&& (visitedConsAcyclic1_19[lab->getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_19(lab));
}

bool RC11Checker::visitConsAcyclic1Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.clear();
	visitedConsAcyclic1_3.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.clear();
	visitedConsAcyclic1_5.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.clear();
	visitedConsAcyclic1_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.clear();
	visitedConsAcyclic1_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.clear();
	visitedConsAcyclic1_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.clear();
	visitedConsAcyclic1_10.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.clear();
	visitedConsAcyclic1_11.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.clear();
	visitedConsAcyclic1_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.clear();
	visitedConsAcyclic1_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.clear();
	visitedConsAcyclic1_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.clear();
	visitedConsAcyclic1_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_17.clear();
	visitedConsAcyclic1_17.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.clear();
	visitedConsAcyclic1_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.clear();
	visitedConsAcyclic1_19.resize(g.getMaxStamp().get() + 1);
	return true
		&& std::ranges::all_of(g.labels(), [&](auto &lab){ return visitedConsAcyclic1_19[lab.getStamp().get()].status != NodeStatus::unseen || visitConsAcyclic1_19(&lab); });
}

bool RC11Checker::visitLHSUnlessConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	return false;


	return true;
}

bool RC11Checker::visitLHSUnlessConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	for (auto &tmp : g.other_labels(lab)) if (auto *pLab = &tmp; true)if (true && pLab->isSC()) {
			if (!visitLHSUnlessConsAcyclic1_0(pLab)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessConsAcyclic1Accepting.clear();
	visitedLHSUnlessConsAcyclic1Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessConsAcyclic1Accepting.clear();
	visitedRHSUnlessConsAcyclic1Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessConsAcyclic1_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessConsAcyclic1Accepting.size(); i++) {
		if (visitedLHSUnlessConsAcyclic1Accepting[i] && !visitedRHSUnlessConsAcyclic1Accepting[i]) {
			return false;
		}
	}
	return true;
}

bool RC11Checker::checkConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessConsAcyclic1(lab))
		return true;

	return visitConsAcyclic1(lab);
}
bool RC11Checker::checkConsAcyclic1(const ExecutionGraph &g) const
{
	return visitConsAcyclic1Full(g);
}
bool RC11Checker::visitError2(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessError2_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool RC11Checker::visitLHSUnlessError2_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (auto pLab = g.alloc(lab); pLab) {
			if (!visitLHSUnlessError2_0(pLab, v)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessError2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError2Accepting.clear();
	visitedLHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true
		&& visitLHSUnlessError2_1(lab, v);
}

bool RC11Checker::checkError2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessError2(lab))
		return true;

	return visitError2(lab);
}
bool RC11Checker::visitError3(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessError3_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	return false;


	return true;
}

bool RC11Checker::visitLHSUnlessError3_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))if (auto pLab = g.free(lab); pLab) {
			if (!visitLHSUnlessError3_0(pLab)) {
			return false;
		}

	}
	if (true && genmc::isa<HpRetireLabel>(lab))if (auto pLab = g.free(lab); pLab) {
			if (!visitLHSUnlessError3_0(pLab)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessError3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError3Accepting.clear();
	visitedLHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError3Accepting.clear();
	visitedRHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError3_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError3Accepting.size(); i++) {
		if (visitedLHSUnlessError3Accepting[i] && !visitedRHSUnlessError3Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RC11Checker::checkError3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessError3(lab))
		return true;

	return visitError3(lab);
}
bool RC11Checker::visitError4(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessError4_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool RC11Checker::visitLHSUnlessError4_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))for (auto &tmp : g.pomax_na_reads(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError4_0(pLab, v)) {
			return false;
		}

	}
	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))for (auto &tmp : g.pomax_na_writes(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError4_0(pLab, v)) {
			return false;
		}

	}
	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))for (auto &tmp : g.pomax_at_reads(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError4_0(pLab, v)) {
			return false;
		}

	}
	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))for (auto &tmp : g.pomax_at_writes(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError4_0(pLab, v)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessError4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError4Accepting.clear();
	visitedLHSUnlessError4Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true
		&& visitLHSUnlessError4_1(lab, v);
}

bool RC11Checker::checkError4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}
bool RC11Checker::visitError5(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessError5_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	return false;


	return true;
}

bool RC11Checker::visitLHSUnlessError5_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<WriteLabel>(lab))if (auto pLab = g.free(lab); pLab) {
			if (!visitLHSUnlessError5_0(pLab)) {
			return false;
		}

	}
	if (true && genmc::isa<ReadLabel>(lab))if (auto pLab = g.free(lab); pLab) {
			if (!visitLHSUnlessError5_0(pLab)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessError5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError5Accepting.clear();
	visitedLHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError5Accepting.clear();
	visitedRHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError5_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError5Accepting.size(); i++) {
		if (visitedLHSUnlessError5Accepting[i] && !visitedRHSUnlessError5Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RC11Checker::checkError5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessError5(lab))
		return true;

	return visitError5(lab);
}
bool RC11Checker::visitError6(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessError6_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool RC11Checker::visitLHSUnlessError6_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<HpRetireLabel>(lab))for (auto &tmp : g.unprotected(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError6_0(pLab, v)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessError6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError6Accepting.clear();
	visitedLHSUnlessError6Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true
		&& visitLHSUnlessError6_1(lab, v);
}

bool RC11Checker::checkError6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessError6(lab))
		return true;

	return visitError6(lab);
}
bool RC11Checker::visitError7(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessError7_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	return false;


	return true;
}

bool RC11Checker::visitLHSUnlessError7_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<MemAccessLabel>(lab) && genmc::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() && !isHazptrProtected(genmc::dyn_cast<MemAccessLabel>(lab)))if (auto pLab = g.retire(lab); pLab) {
			if (!visitLHSUnlessError7_0(pLab)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessError7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError7Accepting.clear();
	visitedLHSUnlessError7Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError7Accepting.clear();
	visitedRHSUnlessError7Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError7_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError7Accepting.size(); i++) {
		if (visitedLHSUnlessError7Accepting[i] && !visitedRHSUnlessError7Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RC11Checker::checkError7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessError7(lab))
		return true;

	return visitError7(lab);
}
bool RC11Checker::visitError8(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessError8_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool RC11Checker::visitLHSUnlessError8_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (true && genmc::isa<WriteLabel>(lab))for (auto &tmp : g.pomax_na_reads(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError8_0(pLab, v)) {
			return false;
		}

	}
	if (true && genmc::isa<WriteLabel>(lab))for (auto &tmp : g.pomax_na_writes(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError8_0(pLab, v)) {
			return false;
		}

	}
	if (true && genmc::isa<ReadLabel>(lab))for (auto &tmp : g.pomax_na_writes(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError8_0(pLab, v)) {
			return false;
		}

	}
	if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))for (auto &tmp : g.pomax_at_reads(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError8_0(pLab, v)) {
			return false;
		}

	}
	if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))for (auto &tmp : g.pomax_at_writes(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError8_0(pLab, v)) {
			return false;
		}

	}
	if (true && lab->isNotAtomic() && genmc::isa<ReadLabel>(lab))for (auto &tmp : g.pomax_at_writes(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError8_0(pLab, v)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessError8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError8Accepting.clear();
	visitedLHSUnlessError8Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true
		&& visitLHSUnlessError8_1(lab, v);
}

bool RC11Checker::checkError8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessError8(lab))
		return true;

	return visitError8(lab);
}
bool RC11Checker::visitWarning9(const EventLabel *lab) const
{
	return false;
}

bool RC11Checker::visitLHSUnlessWarning9_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (!v.contains(lab->getPos())) {
cexLab = lab;
		return false;
	}


	return true;
}

bool RC11Checker::visitLHSUnlessWarning9_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();


	if (true && !(lab->isNotAtomic()) && genmc::isa<WriteLabel>(lab))for (auto &tmp : g.pomax_at_writes(lab)) if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessWarning9_0(pLab, v)) {
			return false;
		}

	}

	return true;
}

bool RC11Checker::visitUnlessWarning9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessWarning9Accepting.clear();
	visitedLHSUnlessWarning9Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true
		&& visitLHSUnlessWarning9_1(lab, v);
}

bool RC11Checker::checkWarning9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();


	if (visitUnlessWarning9(lab))
		return true;

	return visitWarning9(lab);
}
std::optional<VerificationError> RC11Checker::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	if (!checkError2(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessNonMalloc};
	}

	if (!checkError3(lab)) {
		race = cexLab;
		return {VerificationError::VE_DoubleFree};
	}

	if (!checkError4(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessFreed};
	}

	if (!checkError5(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessFreed};
	}

	if (!checkError6(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessFreed};
	}

	if (!checkError7(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessFreed};
	}

	if (!checkError8(lab)) {
		race = cexLab;
		return {VerificationError::VE_RaceNotAtomic};
	}

	return {};
}

std::vector<VerificationError> RC11Checker::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings, std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning9(lab)) {
		racyLabs.push_back(cexLab);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool RC11Checker::isConsistent(const EventLabel *lab) const
{

	return true
		&& checkConsAcyclic1(lab);
}

bool RC11Checker::isConsistent(const ExecutionGraph &g) const
{

	return true
		&& checkConsAcyclic1(g);
}

bool RC11Checker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true
		&& visitCoherenceRelinche(g);
}

View RC11Checker::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View pporf;
	pporf.updateIdx(lab->getPos());

	if (auto *pLab = g.po_imm_pred(lab); pLab)
		pporf.update(pLab->getPrefixView());
	if (auto *rLab = genmc::dyn_cast<ReadLabel>(lab); rLab && rLab->getRf())
		pporf.update(rLab->getRf()->getPrefixView());
	auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(lab);
	if (tsLab && tsLab->getCreate())
		pporf.update(tsLab->getCreate()->getPrefixView());
	if (auto *tjLab = genmc::dyn_cast<ThreadJoinLabel>(lab))
		pporf.update(g.getLastThreadLabel(tjLab->getChildId())->getPrefixView());
	return pporf;
}
std::unique_ptr<VectorClock> RC11Checker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}
