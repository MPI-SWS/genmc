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

#include "RADriver.hpp"
#include "ModuleInfo.hpp"

RADriver::RADriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		std::unique_ptr<ModuleInfo> MI, GenMCDriver::Mode mode /* = GenMCDriver::VerificationMode{} */)
	: GenMCDriver(conf, std::move(mod), std::move(MI), mode) {}

void RADriver::visitCalc0_0(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(0));
	calcRes.updateIdx(lab->getPos());
	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc0_1(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (llvm::isa<ThreadStartLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_6(pLab, calcRes);
	}
	if (llvm::isa<ThreadJoinLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_6(pLab, calcRes);
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && !lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_6(pLab, calcRes);
	}
	if (llvm::isa<ThreadStartLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_4(pLab, calcRes);
	}
	if (llvm::isa<ThreadJoinLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_4(pLab, calcRes);
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && !lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_4(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_2(pLab, calcRes);
	}
	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc0_2(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_2[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_5(pLab, calcRes);
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_3(pLab, calcRes);
	}
	visitedCalc0_2[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc0_3(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_3[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_3(pLab, calcRes);
	}
	visitedCalc0_3[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc0_4(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_5(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_3(pLab, calcRes);
	}
	visitedCalc0_4[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc0_5(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_5[lab->getStamp().get()] = NodeStatus::entered;
	if (g.isRMWStore(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (g.isRMWLoad(pLab)) {
		auto status = visitedCalc0_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_4(pLab, calcRes);
	}
	visitedCalc0_5[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc0_6(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_6[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_6(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_4(pLab, calcRes);
	}
	visitedCalc0_6[lab->getStamp().get()] = NodeStatus::left;
}

View RADriver::calculate0(const EventLabel *lab)
{
	auto &g = getGraph();
	View calcRes;

	calcRes.updateIdx(lab->getPos().prev());
	visitedCalc0_0.clear();
	visitedCalc0_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc0_1.clear();
	visitedCalc0_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc0_2.clear();
	visitedCalc0_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc0_3.clear();
	visitedCalc0_3.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc0_4.clear();
	visitedCalc0_4.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc0_5.clear();
	visitedCalc0_5.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc0_6.clear();
	visitedCalc0_6.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);

	visitCalc0_1(lab, calcRes);
	return calcRes;
}
void RADriver::visitCalc1_0(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_0[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(1));
	calcRes.updateIdx(lab->getPos());
	visitedCalc1_0[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc1_1(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_2(pLab, calcRes);
	}
	visitedCalc1_1[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitCalc1_2(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_2[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	visitedCalc1_2[lab->getStamp().get()] = NodeStatus::left;
}

View RADriver::calculate1(const EventLabel *lab)
{
	auto &g = getGraph();
	View calcRes;

	calcRes.updateIdx(lab->getPos().prev());
	visitedCalc1_0.clear();
	visitedCalc1_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc1_1.clear();
	visitedCalc1_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc1_2.clear();
	visitedCalc1_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);

	visitCalc1_1(lab, calcRes);
	return calcRes;
}
std::vector<VSet<Event>> RADriver::calculateSaved(const EventLabel *lab)
{
	return std::move(saved);
}

std::vector<View> RADriver::calculateViews(const EventLabel *lab)
{
	views.push_back(calculate0(lab));
	views.push_back(calculate1(lab));
	return std::move(views);
}

void RADriver::updateMMViews(EventLabel *lab)
{
	lab->setCalculated(calculateSaved(lab));
	lab->setViews(calculateViews(lab));
	lab->setPrefixView(calculatePrefixView(lab));
}

bool RADriver::isDepTracking() const
{
	return 0;
}

bool RADriver::visitInclusionLHS0_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS0_0[lab->getStamp().get()] = NodeStatus::entered;
	if (!v.contains(lab->getPos())) {
		racyLab0 = lab;
		return false;
	}
	visitedInclusionLHS0_0[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::visitInclusionLHS0_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS0_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = alloc_pred(g, lab); pLab) {
		auto status = visitedInclusionLHS0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS0_0(pLab, v))
			return false;
	}
	visitedInclusionLHS0_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::checkInclusion0(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(0);

	visitedInclusionLHS0_0.clear();
	visitedInclusionLHS0_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS0_1.clear();
	visitedInclusionLHS0_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS0_1(lab, v);
}

void RADriver::visitInclusionLHS1_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS1_0[lab->getStamp().get()] = NodeStatus::entered;
	lhsAccept1[lab->getStamp().get()] = true;
	visitedInclusionLHS1_0[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitInclusionLHS1_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS1_1[lab->getStamp().get()] = NodeStatus::entered;
	if (llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<HpRetireLabel>(pLab)) {
		auto status = visitedInclusionLHS1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS1_0(pLab);
	}
	if (llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
		auto status = visitedInclusionLHS1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS1_0(pLab);
	}
	if (llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<HpRetireLabel>(pLab)) {
		auto status = visitedInclusionLHS1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS1_0(pLab);
	}
	if (llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
		auto status = visitedInclusionLHS1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS1_0(pLab);
	}
	visitedInclusionLHS1_1[lab->getStamp().get()] = NodeStatus::left;
}

bool RADriver::checkInclusion1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS1_0.clear();
	visitedInclusionLHS1_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS1_1.clear();
	visitedInclusionLHS1_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	lhsAccept1.clear();
	lhsAccept1.resize(g.getMaxStamp().get() + 1, false);
	rhsAccept1.clear();
	rhsAccept1.resize(g.getMaxStamp().get() + 1, false);

	visitInclusionLHS1_1(lab);
	for (auto i = 0u; i < lhsAccept1.size(); i++) {
		if (lhsAccept1[i] && !rhsAccept1[i]) {
			racyLab1 = &*std::find_if(label_begin(g), label_end(g), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RADriver::visitInclusionLHS2_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS2_0[lab->getStamp().get()] = NodeStatus::entered;
	if (!v.contains(lab->getPos())) {
		racyLab2 = lab;
		return false;
	}
	visitedInclusionLHS2_0[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::visitInclusionLHS2_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS2_1[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &tmp : alloc_succs(g, lab)) if (auto *pLab = &tmp; true) {
		auto status = visitedInclusionLHS2_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS2_0(pLab, v))
			return false;
	}
	visitedInclusionLHS2_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::visitInclusionLHS2_2(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS2_2[lab->getStamp().get()] = NodeStatus::entered;
	if (llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))if (auto pLab = free_pred(g, lab); pLab) {
		auto status = visitedInclusionLHS2_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS2_0(pLab, v))
			return false;
	}
	if (llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))if (auto pLab = free_pred(g, lab); pLab) {
		auto status = visitedInclusionLHS2_1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS2_1(pLab, v))
			return false;
	}
	visitedInclusionLHS2_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::checkInclusion2(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(0);

	visitedInclusionLHS2_0.clear();
	visitedInclusionLHS2_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS2_1.clear();
	visitedInclusionLHS2_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS2_2.clear();
	visitedInclusionLHS2_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS2_2(lab, v);
}

void RADriver::visitInclusionLHS3_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS3_0[lab->getStamp().get()] = NodeStatus::entered;
	lhsAccept3[lab->getStamp().get()] = true;
	visitedInclusionLHS3_0[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitInclusionLHS3_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS3_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = free_succ(g, lab); pLab)if (llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
		auto status = visitedInclusionLHS3_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS3_0(pLab);
	}
	visitedInclusionLHS3_1[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitInclusionLHS3_2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS3_2[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = alloc_pred(g, lab); pLab) {
		auto status = visitedInclusionLHS3_1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS3_1(pLab);
	}
	visitedInclusionLHS3_2[lab->getStamp().get()] = NodeStatus::left;
}

bool RADriver::checkInclusion3(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS3_0.clear();
	visitedInclusionLHS3_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS3_1.clear();
	visitedInclusionLHS3_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS3_2.clear();
	visitedInclusionLHS3_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	lhsAccept3.clear();
	lhsAccept3.resize(g.getMaxStamp().get() + 1, false);
	rhsAccept3.clear();
	rhsAccept3.resize(g.getMaxStamp().get() + 1, false);

	visitInclusionLHS3_2(lab);
	for (auto i = 0u; i < lhsAccept3.size(); i++) {
		if (lhsAccept3[i] && !rhsAccept3[i]) {
			racyLab3 = &*std::find_if(label_begin(g), label_end(g), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RADriver::visitInclusionLHS4_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS4_0[lab->getStamp().get()] = NodeStatus::entered;
	if (!v.contains(lab->getPos())) {
		racyLab4 = lab;
		return false;
	}
	visitedInclusionLHS4_0[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::visitInclusionLHS4_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS4_1[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &tmp : alloc_succs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<MemAccessLabel>(pLab) && llvm::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() && !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(pLab))) {
		auto status = visitedInclusionLHS4_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS4_0(pLab, v))
			return false;
	}
	visitedInclusionLHS4_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::visitInclusionLHS4_2(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS4_2[lab->getStamp().get()] = NodeStatus::entered;
	if (llvm::isa<HpRetireLabel>(lab))if (auto pLab = free_pred(g, lab); pLab) {
		auto status = visitedInclusionLHS4_1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS4_1(pLab, v))
			return false;
	}
	visitedInclusionLHS4_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::checkInclusion4(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(0);

	visitedInclusionLHS4_0.clear();
	visitedInclusionLHS4_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS4_1.clear();
	visitedInclusionLHS4_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS4_2.clear();
	visitedInclusionLHS4_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS4_2(lab, v);
}

void RADriver::visitInclusionLHS5_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS5_0[lab->getStamp().get()] = NodeStatus::entered;
	lhsAccept5[lab->getStamp().get()] = true;
	visitedInclusionLHS5_0[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitInclusionLHS5_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS5_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = free_succ(g, lab); pLab)if (llvm::isa<HpRetireLabel>(pLab)) {
		auto status = visitedInclusionLHS5_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS5_0(pLab);
	}
	visitedInclusionLHS5_1[lab->getStamp().get()] = NodeStatus::left;
}

void RADriver::visitInclusionLHS5_2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS5_2[lab->getStamp().get()] = NodeStatus::entered;
	if (llvm::isa<MemAccessLabel>(lab) && llvm::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() && !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(lab)))if (auto pLab = alloc_pred(g, lab); pLab) {
		auto status = visitedInclusionLHS5_1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitInclusionLHS5_1(pLab);
	}
	visitedInclusionLHS5_2[lab->getStamp().get()] = NodeStatus::left;
}

bool RADriver::checkInclusion5(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS5_0.clear();
	visitedInclusionLHS5_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS5_1.clear();
	visitedInclusionLHS5_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS5_2.clear();
	visitedInclusionLHS5_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	lhsAccept5.clear();
	lhsAccept5.resize(g.getMaxStamp().get() + 1, false);
	rhsAccept5.clear();
	rhsAccept5.resize(g.getMaxStamp().get() + 1, false);

	visitInclusionLHS5_2(lab);
	for (auto i = 0u; i < lhsAccept5.size(); i++) {
		if (lhsAccept5[i] && !rhsAccept5[i]) {
			racyLab5 = &*std::find_if(label_begin(g), label_end(g), [&](auto &lab){ return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RADriver::visitInclusionLHS6_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS6_0[lab->getStamp().get()] = NodeStatus::entered;
	if (!v.contains(lab->getPos())) {
		racyLab6 = lab;
		return false;
	}
	visitedInclusionLHS6_0[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::visitInclusionLHS6_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS6_1[lab->getStamp().get()] = NodeStatus::entered;
	if (llvm::isa<ReadLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<WriteLabel>(pLab) && pLab->isNotAtomic()) {
		auto status = visitedInclusionLHS6_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS6_0(pLab, v))
			return false;
	}
	if (llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<WriteLabel>(pLab) && pLab->isNotAtomic()) {
		auto status = visitedInclusionLHS6_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS6_0(pLab, v))
			return false;
	}
	if (llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<ReadLabel>(pLab) && pLab->isNotAtomic()) {
		auto status = visitedInclusionLHS6_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS6_0(pLab, v))
			return false;
	}
	if (llvm::isa<WriteLabel>(lab) && lab->isNotAtomic())for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<ReadLabel>(pLab)) {
		auto status = visitedInclusionLHS6_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS6_0(pLab, v))
			return false;
	}
	if (llvm::isa<WriteLabel>(lab) && lab->isNotAtomic())for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<WriteLabel>(pLab)) {
		auto status = visitedInclusionLHS6_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS6_0(pLab, v))
			return false;
	}
	if (llvm::isa<ReadLabel>(lab) && lab->isNotAtomic())for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<WriteLabel>(pLab)) {
		auto status = visitedInclusionLHS6_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS6_0(pLab, v))
			return false;
	}
	visitedInclusionLHS6_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::checkInclusion6(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(0);

	visitedInclusionLHS6_0.clear();
	visitedInclusionLHS6_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS6_1.clear();
	visitedInclusionLHS6_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS6_1(lab, v);
}

bool RADriver::visitInclusionLHS7_0(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS7_0[lab->getStamp().get()] = NodeStatus::entered;
	if (!v.contains(lab->getPos())) {
		racyLab7 = lab;
		return false;
	}
	visitedInclusionLHS7_0[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::visitInclusionLHS7_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS7_1[lab->getStamp().get()] = NodeStatus::entered;
	if (llvm::isa<WriteLabel>(lab))for (auto &tmp : samelocs(g, lab)) if (auto *pLab = &tmp; true)if (llvm::isa<WriteLabel>(pLab)) {
		auto status = visitedInclusionLHS7_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen && !visitInclusionLHS7_0(pLab, v))
			return false;
	}
	visitedInclusionLHS7_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RADriver::checkInclusion7(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(1);

	visitedInclusionLHS7_0.clear();
	visitedInclusionLHS7_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS7_1.clear();
	visitedInclusionLHS7_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS7_1(lab, v);
}

VerificationError RADriver::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	if (!checkInclusion0(lab)) {
		race = racyLab0;
		return VerificationError::VE_AccessNonMalloc;
	}

	if (!checkInclusion1(lab)) {
		race = racyLab1;
		return VerificationError::VE_DoubleFree;
	}

	if (!checkInclusion2(lab)) {
		race = racyLab2;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkInclusion3(lab)) {
		race = racyLab3;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkInclusion4(lab)) {
		race = racyLab4;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkInclusion5(lab)) {
		race = racyLab5;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkInclusion6(lab)) {
		race = racyLab6;
		return VerificationError::VE_RaceNotAtomic;
	}

	return VerificationError::VE_OK;
}

std::vector<VerificationError> RADriver::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings, std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkInclusion7(lab)) {
		racyLabs.push_back(racyLab7);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool RADriver::isAcyclic(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAccepting = 0;
	return true;
}

bool RADriver::isConsistent(const EventLabel *lab) const
{
	return isAcyclic(lab);
}

bool RADriver::isRecAcyclic(const EventLabel *lab) const
{
	visitedRecAccepting = 0;
	return true;
}

bool RADriver::isRecoveryValid(const EventLabel *lab) const
{
	return isRecAcyclic(lab);
}

View RADriver::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = getGraph();
	View pporf;
	pporf.updateIdx(lab->getPos());

	auto *pLab = g.getPreviousLabel(lab);
	if (!pLab)
		return pporf;
	pporf.update(pLab->getPrefixView());
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(pLab))
		pporf.update(rLab->getRf()->getPrefixView());
	if (auto *tsLab = llvm::dyn_cast<ThreadStartLabel>(pLab))
		pporf.update(g.getEventLabel(tsLab->getParentCreate())->getPrefixView());
	if (auto *tjLab = llvm::dyn_cast<ThreadJoinLabel>(pLab))
		pporf.update(g.getLastThreadLabel(tjLab->getChildId())->getPrefixView());
	return pporf;
}
std::unique_ptr<VectorClock> RADriver::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}

const View &RADriver::getHbView(const EventLabel *lab) const
{
	return lab->view(0);
}


bool RADriver::isWriteRfBefore(Event a, Event b)
{
	auto &g = getGraph();
	auto &before = g.getEventLabel(b)->view(0);
	if (before.contains(a))
		return true;

	const EventLabel *lab = g.getEventLabel(a);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (auto &rLab : wLab->readers())
		if (before.contains(rLab.getPos()))
			return true;
	return false;
}

std::vector<Event>
RADriver::getInitRfsAtLoc(SAddr addr)
{
	std::vector<Event> result;

	for (const auto &lab : labels(getGraph())) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab))
			if (rLab->getRf()->getPos().isInitializer() && rLab->getAddr() == addr)
				result.push_back(rLab->getPos());
	}
	return result;
}

bool RADriver::isHbOptRfBefore(const Event e, const Event write)
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(write);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *sLab = static_cast<const WriteLabel *>(lab);
	if (sLab->view(0).contains(e))
		return true;

	for (auto &rLab : sLab->readers()) {
		if (rLab.view(0).contains(e))
			return true;
	}
	return false;
}

ExecutionGraph::co_iterator
RADriver::splitLocMOBefore(SAddr addr, Event e)
{
	auto &g = getGraph();
	auto rit = std::find_if(g.co_rbegin(addr), g.co_rend(addr), [&](auto &lab){
		return isWriteRfBefore(lab.getPos(), e);
	});
	/* Convert to forward iterator, but be _really_ careful */
	if (rit == g.co_rend(addr))
		return g.co_begin(addr);
	return ++ExecutionGraph::co_iterator(*rit);
}

ExecutionGraph::co_iterator
RADriver::splitLocMOAfterHb(SAddr addr, const Event read)
{
	auto &g = getGraph();

	auto initRfs = g.getInitRfsAtLoc(addr);
	if (std::any_of(initRfs.begin(), initRfs.end(), [&read,&g](const Event &rf){
		return g.getEventLabel(rf)->view(0).contains(read);
	}))
		return g.co_begin(addr);

	auto it = std::find_if(g.co_begin(addr), g.co_end(addr), [&](auto &lab){
		return isHbOptRfBefore(read, lab.getPos());
	});
	if (it == g.co_end(addr) || it->view(0).contains(read))
		return it;
	return ++it;
}

ExecutionGraph::co_iterator
RADriver::splitLocMOAfter(SAddr addr, const Event e)
{
	auto &g = getGraph();
	return std::find_if(g.co_begin(addr), g.co_end(addr), [&](auto &lab){
		return isHbOptRfBefore(e, lab.getPos());
	});
}

std::vector<Event>
RADriver::getCoherentStores(SAddr addr, Event read)
{
	auto &g = getGraph();
	std::vector<Event> stores;

	/*
	 * If there are no stores (rf?;hb)-before the current event
	 * then we can read read from all concurrent stores and the
	 * initializer store. Otherwise, we can read from all concurrent
	 * stores and the mo-latest of the (rf?;hb)-before stores.
	 */
	auto begIt = splitLocMOBefore(addr, read);
	if (begIt == g.co_begin(addr))
		stores.push_back(Event::getInit());
	else {
		stores.push_back((--begIt)->getPos());
		++begIt;
	}

	/*
	 * If the model supports out-of-order execution we have to also
	 * account for the possibility the read is hb-before some other
	 * store, or some read that reads from a store.
	 */
	auto endIt = (isDepTracking()) ? splitLocMOAfterHb(addr, read) : g.co_end(addr);
	std::transform(begIt, endIt, std::back_inserter(stores), [&](auto &lab){
		return lab.getPos();
	});
	return stores;
}

std::vector<Event>
RADriver::getMOOptRfAfter(const WriteLabel *sLab)
{
	std::vector<Event> after;
	std::vector<const ReadLabel *> rfAfter;

	const auto &g = getGraph();
	std::for_each(g.co_succ_begin(sLab), g.co_succ_end(sLab),
		      [&](auto &wLab){
			      after.push_back(wLab.getPos());
			      std::transform(wLab.readers_begin(), wLab.readers_end(), std::back_inserter(rfAfter),
			      [&](auto &rLab){ return &rLab; });
	});
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after), [](auto *rLab){
		return rLab->getPos();
	});
	return after;
}

std::vector<Event>
RADriver::getMOInvOptRfAfter(const WriteLabel *sLab)
{
	auto &g = getGraph();
	std::vector<Event> after;
	std::vector<const ReadLabel *> rfAfter;

	/* First, add (mo;rf?)-before */
	std::for_each(g.co_pred_begin(sLab),
		      g.co_pred_end(sLab), [&](auto &wLab){
			      after.push_back(wLab.getPos());
			      std::transform(wLab.readers_begin(), wLab.readers_end(), std::back_inserter(rfAfter),
			      [&](auto &rLab){ return &rLab; });
	});
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after), [](auto *rLab){
		return rLab->getPos();
	});

	/* Then, we add the reader list for the initializer */
	auto initRfs = g.getInitRfsAtLoc(sLab->getAddr());
	after.insert(after.end(), initRfs.begin(), initRfs.end());
	return after;
}

std::vector<Event>
RADriver::getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf)
{
	auto &g = getGraph();
	auto ls = g.getRevisitable(sLab, pporf);

	/* If this store is po- and mo-maximal then we are done */
	if (!isDepTracking() && g.isCoMaximal(sLab->getAddr(), sLab->getPos()))
		return ls;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
				{ const View &before = g.getEventLabel(e)->view(0);
				  return std::any_of(optRfs.begin(), optRfs.end(),
					 [&](Event ev)
					 { return before.contains(ev); });
				}), ls.end());

	/* If out-of-order event addition is not supported, then we are done
	 * due to po-maximality */
	if (!isDepTracking())
		return ls;

	/* Otherwise, we also have to exclude hb-before loads */
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
		{ return g.getEventLabel(sLab->getPos())->view(0).contains(e); }),
		ls.end());

	/* ...and also exclude (mo^-1; rf?; (hb^-1)?; sb^-1)-after reads in
	 * the resulting graph */
	auto &before = pporf;
	auto moInvOptRfs = getMOInvOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
				{ auto *eLab = g.getEventLabel(e);
				  auto v = g.getViewFromStamp(eLab->getStamp());
				  v->update(before);
				  return std::any_of(moInvOptRfs.begin(),
						     moInvOptRfs.end(),
						     [&](Event ev)
						     { return v->contains(ev) &&
						       g.getEventLabel(ev)->view(0).contains(e); });
				}),
		 ls.end());

	return ls;
}

llvm::iterator_range<ExecutionGraph::co_iterator>
RADriver::getCoherentPlacings(SAddr addr, Event store, bool isRMW)
{
	auto &g = getGraph();

	/* If it is an RMW store, there is only one possible position in MO */
	if (isRMW) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(g.getEventLabel(store.prev()))) {
			auto *rfLab = rLab->getRf();
			BUG_ON(!rfLab);
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab)) {
				auto wIt = g.co_succ_begin(wLab);
				return llvm::iterator_range<ExecutionGraph::co_iterator>(wIt, wIt);
			}
			return llvm::iterator_range<ExecutionGraph::co_iterator>(g.co_begin(addr),
										 g.co_begin(addr));
		}
		BUG();
	}

	/* Otherwise, we calculate the full range and add the store */
	auto rangeBegin = splitLocMOBefore(addr, store);
	auto rangeEnd = (isDepTracking()) ? splitLocMOAfter(addr, store) : g.co_end(addr);
	return llvm::iterator_range(rangeBegin, rangeEnd);

}
