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

#include "RC11Driver.hpp"
#include "ModuleInfo.hpp"

RC11Driver::RC11Driver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		std::unique_ptr<ModuleInfo> MI, GenMCDriver::Mode mode /* = GenMCDriver::VerificationMode{} */)
	: GenMCDriver(conf, std::move(mod), std::move(MI), mode) {}

void RC11Driver::visitCalc0_0(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(0));
	calcRes.updateIdx(lab->getPos());
	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc0_1(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_2(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc0_2(const EventLabel *lab, View &calcRes)
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
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	visitedCalc0_2[lab->getStamp().get()] = NodeStatus::left;
}

View RC11Driver::calculate0(const EventLabel *lab)
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

	visitCalc0_1(lab, calcRes);
	return calcRes;
}
void RC11Driver::visitCalc1_0(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_0[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(1));
	calcRes.updateIdx(lab->getPos());
	visitedCalc1_0[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc1_1(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_2(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (llvm::isa<ThreadStartLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_4(pLab, calcRes);
	}
	if (llvm::isa<ThreadJoinLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_4(pLab, calcRes);
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && !lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_4(pLab, calcRes);
	}
	if (llvm::isa<ThreadStartLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_6(pLab, calcRes);
	}
	if (llvm::isa<ThreadJoinLabel>(lab))if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_6(pLab, calcRes);
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && !lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && lab->isAtLeastRelease() || llvm::isa<FenceLabel>(lab) && lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_6(pLab, calcRes);
	}
	visitedCalc1_1[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc1_2(const EventLabel *lab, View &calcRes)
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
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc1_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_5(pLab, calcRes);
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc1_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_3(pLab, calcRes);
	}
	visitedCalc1_2[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc1_3(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_3[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_3(pLab, calcRes);
	}
	visitedCalc1_3[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc1_4(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc1_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_5(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc1_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_3(pLab, calcRes);
	}
	visitedCalc1_4[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc1_5(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_5[lab->getStamp().get()] = NodeStatus::entered;
	if (g.isRMWStore(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (g.isRMWLoad(pLab)) {
		auto status = visitedCalc1_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_4(pLab, calcRes);
	}
	visitedCalc1_5[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitCalc1_6(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_6[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_4(pLab, calcRes);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_6(pLab, calcRes);
	}
	visitedCalc1_6[lab->getStamp().get()] = NodeStatus::left;
}

View RC11Driver::calculate1(const EventLabel *lab)
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
	visitedCalc1_3.clear();
	visitedCalc1_3.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc1_4.clear();
	visitedCalc1_4.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc1_5.clear();
	visitedCalc1_5.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedCalc1_6.clear();
	visitedCalc1_6.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);

	visitCalc1_1(lab, calcRes);
	return calcRes;
}
std::vector<VSet<Event>> RC11Driver::calculateSaved(const EventLabel *lab)
{
	return std::move(saved);
}

std::vector<View> RC11Driver::calculateViews(const EventLabel *lab)
{
	views.push_back(calculate0(lab));
	views.push_back(calculate1(lab));
	return std::move(views);
}

void RC11Driver::updateMMViews(EventLabel *lab)
{
	lab->setCalculated(calculateSaved(lab));
	lab->setViews(calculateViews(lab));
	lab->setPrefixView(calculatePrefixView(lab));
}

bool RC11Driver::isDepTracking() const
{
	return 0;
}

bool RC11Driver::visitInclusionLHS0_0(const EventLabel *lab, const View &v) const
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

bool RC11Driver::visitInclusionLHS0_1(const EventLabel *lab, const View &v) const
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

bool RC11Driver::checkInclusion0(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(1);

	visitedInclusionLHS0_0.clear();
	visitedInclusionLHS0_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS0_1.clear();
	visitedInclusionLHS0_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS0_1(lab, v);
}

void RC11Driver::visitInclusionLHS1_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS1_0[lab->getStamp().get()] = NodeStatus::entered;
	lhsAccept1[lab->getStamp().get()] = true;
	visitedInclusionLHS1_0[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitInclusionLHS1_1(const EventLabel *lab) const
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

bool RC11Driver::checkInclusion1(const EventLabel *lab) const
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

bool RC11Driver::visitInclusionLHS2_0(const EventLabel *lab, const View &v) const
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

bool RC11Driver::visitInclusionLHS2_1(const EventLabel *lab, const View &v) const
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

bool RC11Driver::visitInclusionLHS2_2(const EventLabel *lab, const View &v) const
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

bool RC11Driver::checkInclusion2(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(1);

	visitedInclusionLHS2_0.clear();
	visitedInclusionLHS2_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS2_1.clear();
	visitedInclusionLHS2_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS2_2.clear();
	visitedInclusionLHS2_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS2_2(lab, v);
}

void RC11Driver::visitInclusionLHS3_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS3_0[lab->getStamp().get()] = NodeStatus::entered;
	lhsAccept3[lab->getStamp().get()] = true;
	visitedInclusionLHS3_0[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitInclusionLHS3_1(const EventLabel *lab) const
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

void RC11Driver::visitInclusionLHS3_2(const EventLabel *lab) const
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

bool RC11Driver::checkInclusion3(const EventLabel *lab) const
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

bool RC11Driver::visitInclusionLHS4_0(const EventLabel *lab, const View &v) const
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

bool RC11Driver::visitInclusionLHS4_1(const EventLabel *lab, const View &v) const
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

bool RC11Driver::visitInclusionLHS4_2(const EventLabel *lab, const View &v) const
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

bool RC11Driver::checkInclusion4(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(1);

	visitedInclusionLHS4_0.clear();
	visitedInclusionLHS4_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS4_1.clear();
	visitedInclusionLHS4_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS4_2.clear();
	visitedInclusionLHS4_2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS4_2(lab, v);
}

void RC11Driver::visitInclusionLHS5_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedInclusionLHS5_0[lab->getStamp().get()] = NodeStatus::entered;
	lhsAccept5[lab->getStamp().get()] = true;
	visitedInclusionLHS5_0[lab->getStamp().get()] = NodeStatus::left;
}

void RC11Driver::visitInclusionLHS5_1(const EventLabel *lab) const
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

void RC11Driver::visitInclusionLHS5_2(const EventLabel *lab) const
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

bool RC11Driver::checkInclusion5(const EventLabel *lab) const
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

bool RC11Driver::visitInclusionLHS6_0(const EventLabel *lab, const View &v) const
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

bool RC11Driver::visitInclusionLHS6_1(const EventLabel *lab, const View &v) const
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

bool RC11Driver::checkInclusion6(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(1);

	visitedInclusionLHS6_0.clear();
	visitedInclusionLHS6_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS6_1.clear();
	visitedInclusionLHS6_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS6_1(lab, v);
}

bool RC11Driver::visitInclusionLHS7_0(const EventLabel *lab, const View &v) const
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

bool RC11Driver::visitInclusionLHS7_1(const EventLabel *lab, const View &v) const
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

bool RC11Driver::checkInclusion7(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(0);

	visitedInclusionLHS7_0.clear();
	visitedInclusionLHS7_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS7_1.clear();
	visitedInclusionLHS7_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true
		&& visitInclusionLHS7_1(lab, v);
}

VerificationError RC11Driver::checkErrors(const EventLabel *lab, const EventLabel *&race) const
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

std::vector<VerificationError> RC11Driver::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings, std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkInclusion7(lab)) {
		racyLabs.push_back(racyLab7);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool RC11Driver::visitAcyclic0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic0[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic1[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic2[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic2(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic2[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic3(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic3[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = po_imm_pred(g, lab); pLab)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic3[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic4(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic4[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic7(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic4(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic4[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic5(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic5[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic5[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic6(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic6[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic7(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic6[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic7(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic7[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (g.isRMWStore(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (g.isRMWLoad(pLab)) {
		auto &node = visitedAcyclic6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic7[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic8(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic8[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic8[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic9(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic9[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isAtLeastAcquire())if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic9[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic10(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic10[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic10(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic10[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic11(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic11[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isAtLeastRelease()) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic11[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic12(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic12[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic10(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadFinishLabel>(pLab)) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<ThreadCreateLabel>(pLab)) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isAtLeastAcquire() || llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() || llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (g.isRMWStore(lab))if (auto pLab = po_imm_pred(g, lab); pLab)if (g.isRMWLoad(pLab)) {
		auto &node = visitedAcyclic11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic11(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic12[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic13(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic13[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(1))if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic13[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic14(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic14[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic2(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(1))if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic14(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic14[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::visitAcyclic15(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedAccepting;
	visitedAcyclic15[lab->getStamp().get()] = { visitedAccepting, NodeStatus::entered };
	if (lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic4(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic2(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic2(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())FOREACH_MAXIMAL(pLab, g, lab->view(1))if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = rf_pred(g, lab); pLab)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = co_imm_pred(g, lab); pLab)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())for (auto &tmp : fr_imm_preds(g, lab)) if (auto *pLab = &tmp; true)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isSC())if (auto pLab = po_imm_pred(g, lab); pLab)if (pLab->isSC()) {
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())FOREACH_MAXIMAL(pLab, g, lab->view(1)) {
		auto &node = visitedAcyclic14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic14(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	--visitedAccepting;
	visitedAcyclic15[lab->getStamp().get()] = { visitedAccepting, NodeStatus::left };
	return true;
}

bool RC11Driver::isAcyclic(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAccepting = 0;
	visitedAcyclic0.clear();
	visitedAcyclic0.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1.clear();
	visitedAcyclic1.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic2.clear();
	visitedAcyclic2.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic3.clear();
	visitedAcyclic3.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic4.clear();
	visitedAcyclic4.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic5.clear();
	visitedAcyclic5.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic6.clear();
	visitedAcyclic6.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic7.clear();
	visitedAcyclic7.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic8.clear();
	visitedAcyclic8.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic9.clear();
	visitedAcyclic9.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic10.clear();
	visitedAcyclic10.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic11.clear();
	visitedAcyclic11.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic12.clear();
	visitedAcyclic12.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic13.clear();
	visitedAcyclic13.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic14.clear();
	visitedAcyclic14.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic15.clear();
	visitedAcyclic15.resize(g.getMaxStamp().get() + 1);
	return true
		&& visitAcyclic0(lab)
		&& visitAcyclic2(lab)
		&& visitAcyclic3(lab)
		&& visitAcyclic8(lab)
		&& visitAcyclic9(lab)
		&& visitAcyclic13(lab)
		&& visitAcyclic15(lab);
}

bool RC11Driver::isConsistent(const EventLabel *lab) const
{
	return isAcyclic(lab);
}

bool RC11Driver::isRecAcyclic(const EventLabel *lab) const
{
	visitedRecAccepting = 0;
	return true;
}

bool RC11Driver::isRecoveryValid(const EventLabel *lab) const
{
	return isRecAcyclic(lab);
}

View RC11Driver::calcPPoRfBefore(const EventLabel *lab) const
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
std::unique_ptr<VectorClock> RC11Driver::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}

const View &RC11Driver::getHbView(const EventLabel *lab) const
{
	return lab->view(1);
}


bool RC11Driver::isWriteRfBefore(Event a, Event b)
{
	auto &g = getGraph();
	auto &before = g.getEventLabel(b)->view(1);
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
RC11Driver::getInitRfsAtLoc(SAddr addr)
{
	std::vector<Event> result;

	for (const auto &lab : labels(getGraph())) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab))
			if (rLab->getRf()->getPos().isInitializer() && rLab->getAddr() == addr)
				result.push_back(rLab->getPos());
	}
	return result;
}

bool RC11Driver::isHbOptRfBefore(const Event e, const Event write)
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(write);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *sLab = static_cast<const WriteLabel *>(lab);
	if (sLab->view(1).contains(e))
		return true;

	for (auto &rLab : sLab->readers()) {
		if (rLab.view(1).contains(e))
			return true;
	}
	return false;
}

ExecutionGraph::co_iterator
RC11Driver::splitLocMOBefore(SAddr addr, Event e)
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
RC11Driver::splitLocMOAfterHb(SAddr addr, const Event read)
{
	auto &g = getGraph();

	auto initRfs = g.getInitRfsAtLoc(addr);
	if (std::any_of(initRfs.begin(), initRfs.end(), [&read,&g](const Event &rf){
		return g.getEventLabel(rf)->view(1).contains(read);
	}))
		return g.co_begin(addr);

	auto it = std::find_if(g.co_begin(addr), g.co_end(addr), [&](auto &lab){
		return isHbOptRfBefore(read, lab.getPos());
	});
	if (it == g.co_end(addr) || it->view(1).contains(read))
		return it;
	return ++it;
}

ExecutionGraph::co_iterator
RC11Driver::splitLocMOAfter(SAddr addr, const Event e)
{
	auto &g = getGraph();
	return std::find_if(g.co_begin(addr), g.co_end(addr), [&](auto &lab){
		return isHbOptRfBefore(e, lab.getPos());
	});
}

std::vector<Event>
RC11Driver::getCoherentStores(SAddr addr, Event read)
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
RC11Driver::getMOOptRfAfter(const WriteLabel *sLab)
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
RC11Driver::getMOInvOptRfAfter(const WriteLabel *sLab)
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
RC11Driver::getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf)
{
	auto &g = getGraph();
	auto ls = g.getRevisitable(sLab, pporf);

	/* If this store is po- and mo-maximal then we are done */
	if (!isDepTracking() && g.isCoMaximal(sLab->getAddr(), sLab->getPos()))
		return ls;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(), [&](Event e)
				{ const View &before = g.getEventLabel(e)->view(1);
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
		{ return g.getEventLabel(sLab->getPos())->view(1).contains(e); }),
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
						       g.getEventLabel(ev)->view(1).contains(e); });
				}),
		 ls.end());

	return ls;
}

llvm::iterator_range<ExecutionGraph::co_iterator>
RC11Driver::getCoherentPlacings(SAddr addr, Event store, bool isRMW)
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
