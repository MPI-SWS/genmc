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

#include "IMMDriver.hpp"
#include "ModuleInfo.hpp"

IMMDriver::IMMDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		     std::unique_ptr<ModuleInfo> MI,
		     GenMCDriver::Mode mode /* = GenMCDriver::VerificationMode{} */)
	: GenMCDriver(conf, std::move(mod), std::move(MI), mode)
{}

void IMMDriver::visitCalc0_0(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(0));
	calcRes.updateIdx(lab->getPos());
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCalc0_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_2(pLab, calcRes);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCalc0_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_2(pLab, calcRes);
	}
	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc0_1(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedCalc0_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_4(pLab, calcRes);
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_6(pLab, calcRes);
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_6(pLab, calcRes);
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadStartLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_6(pLab, calcRes);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_0(pLab, calcRes);
	}
	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc0_2(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_2[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(0));
	calcRes.updateIdx(lab->getPos());
	visitedCalc0_2[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc0_3(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_3[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCalc0_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_2(pLab, calcRes);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto status = visitedCalc0_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_2(pLab, calcRes);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto status = visitedCalc0_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_2(pLab, calcRes);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_3(pLab, calcRes);
	}
	visitedCalc0_3[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc0_4(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto status = visitedCalc0_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_5(pLab, calcRes);
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto status = visitedCalc0_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_2(pLab, calcRes);
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_3(pLab, calcRes);
	}
	visitedCalc0_4[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc0_5(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_5[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto status = visitedCalc0_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_4(pLab, calcRes);
		}
	visitedCalc0_5[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc0_6(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_6[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto status = visitedCalc0_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_5(pLab, calcRes);
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto status = visitedCalc0_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_2(pLab, calcRes);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_6(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_3(pLab, calcRes);
	}
	visitedCalc0_6[lab->getStamp().get()] = NodeStatus::left;
}

View IMMDriver::calculate0(const EventLabel *lab)
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
void IMMDriver::visitCalc1_0(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_0[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(1));
	calcRes.updateIdx(lab->getPos());
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCalc1_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_2(pLab, calcRes);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCalc1_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_2(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc1_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_2(pLab, calcRes);
	}
	visitedCalc1_0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc1_1(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_1[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc1_0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc1_0(pLab, calcRes);
	}
	visitedCalc1_1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitCalc1_2(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc1_2[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(1));
	calcRes.updateIdx(lab->getPos());
	visitedCalc1_2[lab->getStamp().get()] = NodeStatus::left;
}

View IMMDriver::calculate1(const EventLabel *lab)
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
std::vector<VSet<Event>> IMMDriver::calculateSaved(const EventLabel *lab)
{
	return std::move(saved);
}

std::vector<View> IMMDriver::calculateViews(const EventLabel *lab)
{
	views.push_back(calculate0(lab));
	views.push_back(calculate1(lab));
	return std::move(views);
}

void IMMDriver::updateMMViews(EventLabel *lab)
{
	lab->setCalculated(calculateSaved(lab));
	lab->setViews(calculateViews(lab));
}

bool IMMDriver::isDepTracking() const { return 1; }

bool IMMDriver::visitInclusionLHS0_0(const EventLabel *lab, const View &v) const
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

bool IMMDriver::visitInclusionLHS0_1(const EventLabel *lab, const View &v) const
{
	auto &g = getGraph();

	visitedInclusionLHS0_1[lab->getStamp().get()] = NodeStatus::entered;
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					auto status =
						visitedInclusionLHS0_0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen &&
					    !visitInclusionLHS0_0(pLab, v))
						return false;
				}
	visitedInclusionLHS0_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMDriver::checkInclusion0(const EventLabel *lab) const
{
	auto &g = getGraph();
	auto &v = lab->view(1);

	visitedInclusionLHS0_0.clear();
	visitedInclusionLHS0_0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedInclusionLHS0_1.clear();
	visitedInclusionLHS0_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	return true && visitInclusionLHS0_1(lab, v);
}

VerificationError IMMDriver::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	return VerificationError::VE_OK;
}

std::vector<VerificationError>
IMMDriver::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings,
			 std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkInclusion0(lab)) {
		racyLabs.push_back(racyLab0);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool IMMDriver::visitAcyclic0_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_0[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_0[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_1[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	visitedAcyclic0_1[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_2[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_2(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_2[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_3(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_3[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_2(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic0_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_4(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_3[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_4(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_4[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic0_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_3(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_4[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_5(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_5[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_5(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic0_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_3(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	visitedAcyclic0_5[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_6(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_6[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab))
	{
		auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	visitedAcyclic0_6[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_7(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_7[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_7(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_7[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_8(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_8[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic0_9[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_9(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_7(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_8[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_9(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_9[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_9[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_10(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_10[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	visitedAcyclic0_10[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_11(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_11[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_11(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	visitedAcyclic0_11[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_12(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_12[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_14(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_11(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab))
	{
		auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_12[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_13(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_13[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isSC()) {
				auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_13[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_14(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic0_14[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_14(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_14(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic0_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_14(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting0 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	visitedAcyclic0_14[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic0_15(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedAccepting0;
	visitedAcyclic0_15[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::entered};
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_10(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic0_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic0_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_3(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isAtLeastAcquire()) {
					auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic0_8(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting0 > node.count)
						return false;
				}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic0_8[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_8(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic0_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_14(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_1[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_1(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic0_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_11(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_11(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic0_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab))
		{
			auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isSC()) {
					auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic0_15(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting0 > node.count)
						return false;
				}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedAcyclic0_15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting0 > node.count)
					return false;
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic0_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting0 > node.count)
				return false;
		}
	--visitedAccepting0;
	visitedAcyclic0_15[lab->getStamp().get()] = {visitedAccepting0, NodeStatus::left};
	return true;
}

bool IMMDriver::isAcyclic0(const EventLabel *lab) const
{
	auto &g = getGraph();

	if (!shouldVisitAcyclic0())
		return true;

	visitedAccepting0 = 0;
	visitedAcyclic0_0.clear();
	visitedAcyclic0_0.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_1.clear();
	visitedAcyclic0_1.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_2.clear();
	visitedAcyclic0_2.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_3.clear();
	visitedAcyclic0_3.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_4.clear();
	visitedAcyclic0_4.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_5.clear();
	visitedAcyclic0_5.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_6.clear();
	visitedAcyclic0_6.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_7.clear();
	visitedAcyclic0_7.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_8.clear();
	visitedAcyclic0_8.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_9.clear();
	visitedAcyclic0_9.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_10.clear();
	visitedAcyclic0_10.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_11.clear();
	visitedAcyclic0_11.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_12.clear();
	visitedAcyclic0_12.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_13.clear();
	visitedAcyclic0_13.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_14.clear();
	visitedAcyclic0_14.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic0_15.clear();
	visitedAcyclic0_15.resize(g.getMaxStamp().get() + 1);
	return true && visitAcyclic0_0(lab) && visitAcyclic0_1(lab) && visitAcyclic0_6(lab) &&
	       visitAcyclic0_8(lab) && visitAcyclic0_10(lab) && visitAcyclic0_13(lab) &&
	       visitAcyclic0_14(lab) && visitAcyclic0_15(lab);
}

bool IMMDriver::shouldVisitAcyclic0_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	shouldVisitedAcyclic0_0[lab->getStamp().get()] = NodeStatus::entered;
	return false;
	shouldVisitedAcyclic0_0[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMDriver::shouldVisitAcyclic0_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	shouldVisitedAcyclic0_1[lab->getStamp().get()] = NodeStatus::entered;
	if (true && lab->isSC())
		if (auto pLab = lab; true) {
			auto &status = shouldVisitedAcyclic0_0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen && !shouldVisitAcyclic0_0(pLab))
				return false;
		}
	shouldVisitedAcyclic0_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMDriver::shouldVisitAcyclic0(void) const
{
	auto &g = getGraph();

	shouldVisitedAcyclic0_0.clear();
	shouldVisitedAcyclic0_0.resize(g.getMaxStamp().get() + 1);
	shouldVisitedAcyclic0_1.clear();
	shouldVisitedAcyclic0_1.resize(g.getMaxStamp().get() + 1);
	return false || std::any_of(label_begin(g), label_end(g),
				    [&](auto &lab) { return !shouldVisitAcyclic0_1(&lab); });
}

bool IMMDriver::visitAcyclic1_0(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_0[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab))
	{
		auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	visitedAcyclic1_0[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_1(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_1[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_1[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_2(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_2[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_3(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_2[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_3(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_3[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_2(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_3[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_4(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_4[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_2(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_2(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic1_2[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_2(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_4(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_4(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic1_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_4(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_4[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_5(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_5[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_2(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_2(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedAcyclic1_2[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_2(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic1_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic1_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_5(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_4(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_4(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic1_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_4(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_5[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_6(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_6[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (auto pLab = rfi_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	visitedAcyclic1_6[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_7(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_7[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	visitedAcyclic1_7[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_8(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_8[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) &&
		    pLab->isDependable()) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_8[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_9(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_9[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_9[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_10(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_10[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_10(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	visitedAcyclic1_10[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_11(const EventLabel *lab) const
{
	auto &g = getGraph();

	visitedAcyclic1_11[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	for (auto &tmp : poloc_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
				auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	for (auto &tmp : poloc_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_11(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	visitedAcyclic1_11[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::visitAcyclic1_12(const EventLabel *lab) const
{
	auto &g = getGraph();

	++visitedAccepting1;
	visitedAcyclic1_12[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::entered};
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic1_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_10(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic1_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_10(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) ||
	     (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
			     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
				auto &node = visitedAcyclic1_6[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_6(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic1_5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting1 > node.count)
			return false;
	}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic1_12(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting1 > node.count)
						return false;
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic1_12(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting1 > node.count)
						return false;
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic1_12(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting1 > node.count)
						return false;
				}
	for (auto &tmp : detour_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : poloc_imm_preds(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isAtLeastRelease() &&
				    llvm::isa<WriteLabel>(pLab)) {
					auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic1_12(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting1 > node.count)
						return false;
				}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) ||
	     (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
			     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) &&
			    pLab->isDependable()) {
				auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting1 > node.count)
				return false;
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) ||
	     (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
			     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
				auto &node = visitedAcyclic1_7[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_7(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : poloc_imm_preds(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedAcyclic1_11[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic1_11(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting1 > node.count)
					return false;
			}
	--visitedAccepting1;
	visitedAcyclic1_12[lab->getStamp().get()] = {visitedAccepting1, NodeStatus::left};
	return true;
}

bool IMMDriver::isAcyclic1(const EventLabel *lab) const
{
	auto &g = getGraph();

	if (!shouldVisitAcyclic1())
		return true;

	visitedAccepting1 = 0;
	visitedAcyclic1_0.clear();
	visitedAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_1.clear();
	visitedAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_2.clear();
	visitedAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_3.clear();
	visitedAcyclic1_3.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_4.clear();
	visitedAcyclic1_4.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_5.clear();
	visitedAcyclic1_5.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_6.clear();
	visitedAcyclic1_6.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_7.clear();
	visitedAcyclic1_7.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_8.clear();
	visitedAcyclic1_8.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_9.clear();
	visitedAcyclic1_9.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_10.clear();
	visitedAcyclic1_10.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_11.clear();
	visitedAcyclic1_11.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic1_12.clear();
	visitedAcyclic1_12.resize(g.getMaxStamp().get() + 1);
	return true && visitAcyclic1_0(lab) && visitAcyclic1_2(lab) && visitAcyclic1_4(lab);
}

bool IMMDriver::isConsistent(const EventLabel *lab) const
{

	return true && isAcyclic0(lab) && isAcyclic0(lab);
}

bool IMMDriver::isRecAcyclic(const EventLabel *lab) const
{
	visitedRecAccepting = 0;
	return true;
}

bool IMMDriver::isRecoveryValid(const EventLabel *lab) const { return isRecAcyclic(lab); }

void IMMDriver::visitPPoRf0(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	if (auto pLab = rfi_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf1(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf2(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) &&
		    pLab->isDependable()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
		     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf3(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf4(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf5(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &tmp : poloc_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	for (auto &tmp : poloc_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf5(pLab, pporf);
		}
	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::left;
}

void IMMDriver::visitPPoRf6(const EventLabel *lab, DepView &pporf) const
{
	auto &g = getGraph();

	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	for (auto &tmp : detour_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : poloc_imm_preds(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isAtLeastRelease() &&
				    llvm::isa<WriteLabel>(pLab)) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) ||
	     (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
			     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab))) &&
			    pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) ||
	     (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
			     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && g.isRMWLoad(lab)) ||
	     (llvm::isa<WriteLabel>(lab) && g.isRMWStore(lab))))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) && g.isRMWLoad(pLab)) ||
			     (llvm::isa<WriteLabel>(pLab) && g.isRMWStore(pLab)))) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : poloc_imm_preds(g, lab))
			if (auto *pLab = &tmp; true) {
				auto status = visitedPPoRf5[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf5(pLab, pporf);
			}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::left;
}

DepView IMMDriver::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = getGraph();
	DepView pporf;
	pporf.updateIdx(lab->getPos());
	visitedPPoRf0.clear();
	visitedPPoRf0.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf1.clear();
	visitedPPoRf1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf2.clear();
	visitedPPoRf2.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf3.clear();
	visitedPPoRf3.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf4.clear();
	visitedPPoRf4.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf5.clear();
	visitedPPoRf5.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedPPoRf6.clear();
	visitedPPoRf6.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);

	visitPPoRf6(lab, pporf);
	return pporf;
}
std::unique_ptr<VectorClock> IMMDriver::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<DepView>(calcPPoRfBefore(lab));
}

const View &IMMDriver::getHbView(const EventLabel *lab) const { return lab->view(0); }

bool IMMDriver::isWriteRfBefore(Event a, Event b)
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

std::vector<Event> IMMDriver::getInitRfsAtLoc(SAddr addr)
{
	std::vector<Event> result;

	for (const auto &lab : labels(getGraph())) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(&lab))
			if (rLab->getRf()->getPos().isInitializer() && rLab->getAddr() == addr)
				result.push_back(rLab->getPos());
	}
	return result;
}

bool IMMDriver::isHbOptRfBefore(const Event e, const Event write)
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

ExecutionGraph::co_iterator IMMDriver::splitLocMOBefore(SAddr addr, Event e)
{
	auto &g = getGraph();
	auto rit = std::find_if(g.co_rbegin(addr), g.co_rend(addr),
				[&](auto &lab) { return isWriteRfBefore(lab.getPos(), e); });
	/* Convert to forward iterator, but be _really_ careful */
	if (rit == g.co_rend(addr))
		return g.co_begin(addr);
	return ++ExecutionGraph::co_iterator(*rit);
}

ExecutionGraph::co_iterator IMMDriver::splitLocMOAfterHb(SAddr addr, const Event read)
{
	auto &g = getGraph();

	auto initRfs = g.getInitRfsAtLoc(addr);
	if (std::any_of(initRfs.begin(), initRfs.end(), [&read, &g](const Event &rf) {
		    return g.getEventLabel(rf)->view(0).contains(read);
	    }))
		return g.co_begin(addr);

	auto it = std::find_if(g.co_begin(addr), g.co_end(addr),
			       [&](auto &lab) { return isHbOptRfBefore(read, lab.getPos()); });
	if (it == g.co_end(addr) || it->view(0).contains(read))
		return it;
	return ++it;
}

ExecutionGraph::co_iterator IMMDriver::splitLocMOAfter(SAddr addr, const Event e)
{
	auto &g = getGraph();
	return std::find_if(g.co_begin(addr), g.co_end(addr),
			    [&](auto &lab) { return isHbOptRfBefore(e, lab.getPos()); });
}

std::vector<Event> IMMDriver::getCoherentStores(SAddr addr, Event read)
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
	std::transform(begIt, endIt, std::back_inserter(stores),
		       [&](auto &lab) { return lab.getPos(); });
	return stores;
}

std::vector<Event> IMMDriver::getMOOptRfAfter(const WriteLabel *sLab)
{
	std::vector<Event> after;
	std::vector<const ReadLabel *> rfAfter;

	const auto &g = getGraph();
	std::for_each(g.co_succ_begin(sLab), g.co_succ_end(sLab), [&](auto &wLab) {
		after.push_back(wLab.getPos());
		std::transform(wLab.readers_begin(), wLab.readers_end(),
			       std::back_inserter(rfAfter), [&](auto &rLab) { return &rLab; });
	});
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after),
		       [](auto *rLab) { return rLab->getPos(); });
	return after;
}

std::vector<Event> IMMDriver::getMOInvOptRfAfter(const WriteLabel *sLab)
{
	auto &g = getGraph();
	std::vector<Event> after;
	std::vector<const ReadLabel *> rfAfter;

	/* First, add (mo;rf?)-before */
	std::for_each(g.co_pred_begin(sLab), g.co_pred_end(sLab), [&](auto &wLab) {
		after.push_back(wLab.getPos());
		std::transform(wLab.readers_begin(), wLab.readers_end(),
			       std::back_inserter(rfAfter), [&](auto &rLab) { return &rLab; });
	});
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after),
		       [](auto *rLab) { return rLab->getPos(); });

	/* Then, we add the reader list for the initializer */
	auto initRfs = g.getInitRfsAtLoc(sLab->getAddr());
	after.insert(after.end(), initRfs.begin(), initRfs.end());
	return after;
}

std::vector<Event> IMMDriver::getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf)
{
	auto &g = getGraph();
	auto ls = g.getRevisitable(sLab, pporf);

	/* If this store is po- and mo-maximal then we are done */
	if (!isDepTracking() && g.isCoMaximal(sLab->getAddr(), sLab->getPos()))
		return ls;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(),
				[&](Event e) {
					const View &before = g.getEventLabel(e)->view(0);
					return std::any_of(
						optRfs.begin(), optRfs.end(),
						[&](Event ev) { return before.contains(ev); });
				}),
		 ls.end());

	/* If out-of-order event addition is not supported, then we are done
	 * due to po-maximality */
	if (!isDepTracking())
		return ls;

	/* Otherwise, we also have to exclude hb-before loads */
	ls.erase(std::remove_if(ls.begin(), ls.end(),
				[&](Event e) {
					return g.getEventLabel(sLab->getPos())->view(0).contains(e);
				}),
		 ls.end());

	/* ...and also exclude (mo^-1; rf?; (hb^-1)?; sb^-1)-after reads in
	 * the resulting graph */
	auto &before = pporf;
	auto moInvOptRfs = getMOInvOptRfAfter(sLab);
	ls.erase(std::remove_if(
			 ls.begin(), ls.end(),
			 [&](Event e) {
				 auto *eLab = g.getEventLabel(e);
				 auto v = g.getViewFromStamp(eLab->getStamp());
				 v->update(before);
				 return std::any_of(
					 moInvOptRfs.begin(), moInvOptRfs.end(), [&](Event ev) {
						 return v->contains(ev) &&
							g.getEventLabel(ev)->view(0).contains(e);
					 });
			 }),
		 ls.end());

	return ls;
}

llvm::iterator_range<ExecutionGraph::co_iterator>
IMMDriver::getCoherentPlacings(SAddr addr, Event store, bool isRMW)
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
