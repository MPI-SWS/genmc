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

#include "IMMChecker.hpp"

void IMMChecker::visitCalc0_0(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::entered;
	calcRes.update(lab->view(0));
	calcRes.updateIdx(lab->getPos());
	visitedCalc0_0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitCalc0_1(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::entered;
	if (llvm::isa<ThreadStartLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_6(pLab, calcRes);
		}
	if (llvm::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_6(pLab, calcRes);
		}
	if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && !lab->isAtLeastRelease() ||
	    llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && lab->isAtLeastRelease() ||
	    llvm::isa<FenceLabel>(lab) && lab->isSC())
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
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc0_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_2(pLab, calcRes);
	}
	if (llvm::isa<ThreadStartLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_4(pLab, calcRes);
		}
	if (llvm::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_4(pLab, calcRes);
		}
	if (llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && !lab->isAtLeastRelease() ||
	    llvm::isa<FenceLabel>(lab) && lab->isAtLeastAcquire() && lab->isAtLeastRelease() ||
	    llvm::isa<FenceLabel>(lab) && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc0_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_4(pLab, calcRes);
		}
	visitedCalc0_1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitCalc0_2(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_2[lab->getStamp().get()] = NodeStatus::entered;
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto status = visitedCalc0_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_5(pLab, calcRes);
		}
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
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (pLab->isAtLeastRelease()) {
				auto status = visitedCalc0_0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitCalc0_0(pLab, calcRes);
			}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto status = visitedCalc0_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_3(pLab, calcRes);
		}
	visitedCalc0_2[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitCalc0_3(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_3[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto status = visitedCalc0_0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_0(pLab, calcRes);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto status = visitedCalc0_0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_0(pLab, calcRes);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
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

void IMMChecker::visitCalc0_4(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_5(pLab, calcRes);
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (pLab->isAtLeastRelease()) {
			auto status = visitedCalc0_0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitCalc0_0(pLab, calcRes);
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc0_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitCalc0_3(pLab, calcRes);
	}
	visitedCalc0_4[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitCalc0_5(const EventLabel *lab, View &calcRes)
{
	auto &g = getGraph();

	visitedCalc0_5[lab->getStamp().get()] = NodeStatus::entered;
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto status = visitedCalc0_4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitCalc0_4(pLab, calcRes);
			}
	visitedCalc0_5[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitCalc0_6(const EventLabel *lab, View &calcRes)
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

View IMMChecker::calculate0(const EventLabel *lab)
{
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
std::vector<VSet<Event>> IMMChecker::calculateSaved(const EventLabel *lab)
{
	return std::move(saved);
}

std::vector<View> IMMChecker::calculateViews(const EventLabel *lab)
{
	views.push_back(calculate0(lab));
	return std::move(views);
}

bool IMMChecker::isDepTracking() { return 1; }

VerificationError IMMChecker::checkErrors(const EventLabel *lab)
{
	return VerificationError::VE_OK;
}

bool IMMChecker::visitAcyclic0(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic0[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (pLab->isSC()) {
			auto &node = visitedAcyclic15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic0[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic1(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic1[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic1[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic2(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic2[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab)
		if (pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic2[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic3(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic3[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic1(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto &node = visitedAcyclic2[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic2(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic3[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic4(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic4[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (pLab->isAtLeastRelease()) {
				auto &node = visitedAcyclic0[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic3(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (pLab->isAtLeastRelease()) {
				auto &node = visitedAcyclic5[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
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
	visitedAcyclic4[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic5(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic5[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic0(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic5[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic6(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic6[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
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
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (pLab->isAtLeastRelease()) {
				auto &node = visitedAcyclic13[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic11(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic6[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic7(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic7[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic7(pLab))
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
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (pLab->isSC()) {
			auto &node = visitedAcyclic15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic7[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic8(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic8[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (pLab->isSC()) {
			auto &node = visitedAcyclic15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true)
			if (pLab->isSC()) {
				auto &node = visitedAcyclic15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	visitedAcyclic8[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic9(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic9[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic9[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic10(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic10[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab)
		if (pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic11(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic10[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic11(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic11[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic9(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto &node = visitedAcyclic10[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic10(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic11[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic12(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic12[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
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
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (pLab->isAtLeastRelease()) {
				auto &node = visitedAcyclic13[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic11(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic12[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic13(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic13[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0)) if (llvm::isa<FenceLabel>(pLab) && pLab->isSC())
	{
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic13[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic14(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic14[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic7(pLab))
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
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0)) if (llvm::isa<FenceLabel>(pLab) && pLab->isSC())
	{
		auto &node = visitedAcyclic15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (pLab->isSC()) {
			auto &node = visitedAcyclic15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic14(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic12(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic14[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic15(const EventLabel *lab)
{
	auto &g = getGraph();

	++visitedAccepting;
	visitedAcyclic15[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic7(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic7(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		for (auto &oLab : fr_imm_preds(g, lab))
			if (auto *pLab = &oLab; true) {
				auto &node = visitedAcyclic13[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic13(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		for (auto &oLab : fr_imm_preds(g, lab))
			if (auto *pLab = &oLab; true) {
				auto &node = visitedAcyclic6[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic6(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic8(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic5(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic4(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		if (llvm::isa<FenceLabel>(pLab) && pLab->isSC())
		{
			auto &node = visitedAcyclic15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (pLab->isSC()) {
				auto &node = visitedAcyclic15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab)
			if (pLab->isSC()) {
				auto &node = visitedAcyclic15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (lab->isSC())
		for (auto &oLab : fr_imm_preds(g, lab))
			if (auto *pLab = &oLab; true)
				if (pLab->isSC()) {
					auto &node = visitedAcyclic15[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic15(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting > node.count)
						return false;
				}
	if (lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (pLab->isSC()) {
				auto &node = visitedAcyclic15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic15(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic14(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	--visitedAccepting;
	visitedAcyclic15[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic16(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic16[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	for (auto *pLab : poloc_imm_preds(g, lab))
		if (llvm::isa<WriteLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isSC() ||
		    llvm::isa<WriteLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	for (auto *pLab : poloc_imm_preds(g, lab)) {
		auto &node = visitedAcyclic16[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic16(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic16[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic17(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic17[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic17(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic17[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic18(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic18[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab)
		if (pLab->isAtLeastRelease()) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic19(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic18[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic19(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic19[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto &node = visitedAcyclic18[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic18(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastRelease() &&
			    !pLab->isAtLeastAcquire() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isAtLeastAcquire() &&
			    pLab->isAtLeastRelease() ||
		    llvm::isa<FenceLabel>(pLab) && pLab->isSC()) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic17(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic19[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic20(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic20[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic20(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic20(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic20(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedAcyclic21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedAcyclic21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (pLab->isAtLeastRelease()) {
				auto &node = visitedAcyclic21[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedAcyclic19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic19(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic20[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic21(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic21[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	FOREACH_MAXIMAL(pLab, g, lab->view(0)) if (llvm::isa<FenceLabel>(pLab) && pLab->isSC())
	{
		auto &node = visitedAcyclic28[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic21[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic22(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic22[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic20(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic20(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic20(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedAcyclic21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic21[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &oLab : fr_imm_preds(g, lab))
		if (auto *pLab = &oLab; true) {
			auto &node = visitedAcyclic21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic21(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	FOREACH_MAXIMAL(pLab, g, lab->view(0))
	{
		auto &node = visitedAcyclic22[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic22(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic22[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic23(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic23[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
				auto &node = visitedAcyclic28[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic25[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedAcyclic25[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic23[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedAcyclic23[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = rfi_pred(g, lab); pLab)
		if (g.isRMWStore(pLab)) {
			auto &node = visitedAcyclic24[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic24(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic23[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic24(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic24[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (g.isRMWLoad(pLab)) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (g.isRMWLoad(pLab)) {
			auto &node = visitedAcyclic25[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (g.isRMWLoad(pLab)) {
			auto &node = visitedAcyclic23[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic24[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic25(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic25[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
				auto &node = visitedAcyclic28[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
				auto &node = visitedAcyclic28[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic25[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic25[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic25[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic23[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedAcyclic23[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	visitedAcyclic25[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic26(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic26[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic26[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic26(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic26[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic27(const EventLabel *lab)
{
	auto &g = getGraph();

	visitedAcyclic27[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic28[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic27[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic27(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	visitedAcyclic27[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::visitAcyclic28(const EventLabel *lab)
{
	auto &g = getGraph();

	++visitedAccepting;
	visitedAcyclic28[lab->getStamp().get()] = {visitedAccepting, NodeStatus::entered};
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto &node = visitedAcyclic28[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
					auto &node = visitedAcyclic28[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic28(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting > node.count)
						return false;
				}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
					auto &node = visitedAcyclic28[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic28(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting > node.count)
						return false;
				}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
					auto &node = visitedAcyclic28[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen &&
					    !visitAcyclic28(pLab))
						return false;
					else if (node.status == NodeStatus::entered &&
						 visitedAccepting > node.count)
						return false;
				}
	for (auto *pLab : detour_preds(g, lab)) {
		auto &node = visitedAcyclic28[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (llvm::isa<WriteLabel>(lab))
		for (auto *pLab : poloc_imm_preds(g, lab))
			if (llvm::isa<WriteLabel>(pLab) && pLab->isAtLeastRelease() &&
				    !pLab->isSC() ||
			    llvm::isa<WriteLabel>(pLab) && pLab->isSC()) {
				auto &node = visitedAcyclic28[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (pLab->isAtLeastAcquire()) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic28[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto &node = visitedAcyclic28[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic28(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedAcyclic26[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen && !visitAcyclic26(pLab))
			return false;
		else if (node.status == NodeStatus::entered && visitedAccepting > node.count)
			return false;
	}
	if (llvm::isa<WriteLabel>(lab))
		for (auto *pLab : poloc_imm_preds(g, lab)) {
			auto &node = visitedAcyclic16[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic16(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic25[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic25[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic25[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<WriteLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic25[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto &node = visitedAcyclic25[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic25(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic23[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic23[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedAcyclic23[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto &node = visitedAcyclic23[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen && !visitAcyclic23(pLab))
					return false;
				else if (node.status == NodeStatus::entered &&
					 visitedAccepting > node.count)
					return false;
			}
	if (llvm::isa<FenceLabel>(lab) && lab->isSC())
		FOREACH_MAXIMAL(pLab, g, lab->view(0))
		{
			auto &node = visitedAcyclic22[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic22(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic27[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic27(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	if (llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedAcyclic27[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen && !visitAcyclic27(pLab))
				return false;
			else if (node.status == NodeStatus::entered &&
				 visitedAccepting > node.count)
				return false;
		}
	--visitedAccepting;
	visitedAcyclic28[lab->getStamp().get()] = {visitedAccepting, NodeStatus::left};
	return true;
}

bool IMMChecker::isAcyclic(const EventLabel *lab)
{
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
	visitedAcyclic16.clear();
	visitedAcyclic16.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic17.clear();
	visitedAcyclic17.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic18.clear();
	visitedAcyclic18.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic19.clear();
	visitedAcyclic19.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic20.clear();
	visitedAcyclic20.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic21.clear();
	visitedAcyclic21.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic22.clear();
	visitedAcyclic22.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic23.clear();
	visitedAcyclic23.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic24.clear();
	visitedAcyclic24.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic25.clear();
	visitedAcyclic25.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic26.clear();
	visitedAcyclic26.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic27.clear();
	visitedAcyclic27.resize(g.getMaxStamp().get() + 1);
	visitedAcyclic28.clear();
	visitedAcyclic28.resize(g.getMaxStamp().get() + 1);
	return true && visitAcyclic0(lab) && visitAcyclic5(lab) && visitAcyclic6(lab) &&
	       visitAcyclic8(lab) && visitAcyclic12(lab) && visitAcyclic13(lab) &&
	       visitAcyclic15(lab) && visitAcyclic20(lab) && visitAcyclic21(lab);
}

bool IMMChecker::isConsistent(const EventLabel *lab) { return isAcyclic(lab); }

bool IMMChecker::isRecAcyclic(const EventLabel *lab)
{
	visitedRecAccepting = 0;
	return true;
}

bool IMMChecker::isRecoveryValid(const EventLabel *lab) { return isRecAcyclic(lab); }

void IMMChecker::visitPPoRf0(const EventLabel *lab, DepView &pporf)
{
	auto &g = getGraph();

	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf0[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf0(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf1(const EventLabel *lab, DepView &pporf)
{
	auto &g = getGraph();

	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	if (auto pLab = rfi_pred(g, lab); pLab)
		if (g.isRMWStore(pLab)) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
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
	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf2(const EventLabel *lab, DepView &pporf)
{
	auto &g = getGraph();

	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (g.isRMWLoad(pLab)) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (g.isRMWLoad(pLab)) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (g.isRMWLoad(pLab)) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf3(const EventLabel *lab, DepView &pporf)
{
	auto &g = getGraph();

	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf3(pLab, pporf);
	}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
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
	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf4(const EventLabel *lab, DepView &pporf)
{
	auto &g = getGraph();

	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf5(const EventLabel *lab, DepView &pporf)
{
	auto &g = getGraph();

	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::entered;
	for (auto *pLab : poloc_imm_preds(g, lab))
		if (llvm::isa<WriteLabel>(pLab) && pLab->isAtLeastRelease() && !pLab->isSC() ||
		    llvm::isa<WriteLabel>(pLab) && pLab->isSC()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	for (auto *pLab : poloc_imm_preds(g, lab)) {
		auto status = visitedPPoRf5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf5(pLab, pporf);
	}
	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf6(const EventLabel *lab, DepView &pporf)
{
	auto &g = getGraph();

	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
			}
	if (llvm::isa<WriteLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	if (llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
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
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (llvm::isa<ReadLabel>(pLab) || pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	for (auto *pLab : detour_preds(g, lab)) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (llvm::isa<WriteLabel>(lab))
		for (auto *pLab : poloc_imm_preds(g, lab))
			if (llvm::isa<WriteLabel>(pLab) && pLab->isAtLeastRelease() &&
				    !pLab->isSC() ||
			    llvm::isa<WriteLabel>(pLab) && pLab->isSC()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto *pLab : poloc_imm_preds(g, lab)) {
			auto status = visitedPPoRf5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf5(pLab, pporf);
		}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (g.isRMWStore(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (g.isRMWLoad(pLab)) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::left;
}

DepView IMMChecker::calcPPoRfBefore(const EventLabel *lab)
{
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
std::unique_ptr<VectorClock> IMMChecker::getPPoRfBefore(const EventLabel *lab)
{
	return LLVM_MAKE_UNIQUE<DepView>(calcPPoRfBefore(lab));
}

const View &IMMChecker::getHbView(const EventLabel *lab) { return lab->view(0); }

bool IMMChecker::isWriteRfBefore(Event a, Event b)
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

std::vector<Event> IMMChecker::getInitRfsAtLoc(SAddr addr)
{
	std::vector<Event> result;

	for (const auto *lab : labels(getGraph())) {
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
			if (rLab->getRf()->getPos().isInitializer() && rLab->getAddr() == addr)
				result.push_back(rLab->getPos());
	}
	return result;
}

bool IMMChecker::isHbOptRfBefore(const Event e, const Event write)
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

ExecutionGraph::co_iterator IMMChecker::splitLocMOBefore(SAddr addr, Event e)
{
	auto &g = getGraph();
	auto rit = std::find_if(g.co_rbegin(addr), g.co_rend(addr),
				[&](auto &lab) { return isWriteRfBefore(lab.getPos(), e); });
	/* Convert to forward iterator, but be _really_ careful */
	if (rit == g.co_rend(addr))
		return g.co_begin(addr);
	return ++ExecutionGraph::co_iterator(*rit);
}

ExecutionGraph::co_iterator IMMChecker::splitLocMOAfterHb(SAddr addr, const Event read)
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

ExecutionGraph::co_iterator IMMChecker::splitLocMOAfter(SAddr addr, const Event e)
{
	auto &g = getGraph();
	return std::find_if(g.co_begin(addr), g.co_end(addr),
			    [&](auto &lab) { return isHbOptRfBefore(e, lab.getPos()); });
}

std::vector<Event> IMMChecker::getCoherentStores(SAddr addr, Event read)
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
		stores.push_back(Event::getInitializer());
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

std::vector<Event> IMMChecker::getMOOptRfAfter(const WriteLabel *sLab)
{
	std::vector<Event> after;

	const auto &g = getGraph();
	std::for_each(g.co_succ_begin(sLab), g.co_succ_end(sLab), [&](auto &wLab) {
		after.push_back(wLab.getPos());
		std::transform(wLab.readers_begin(), wLab.readers_end(), std::back_inserter(after),
			       [&](auto &rLab) { return rLab.getPos(); });
	});

	return after;
}

std::vector<Event> IMMChecker::getMOInvOptRfAfter(const WriteLabel *sLab)
{
	auto &g = getGraph();
	std::vector<Event> after;
	std::vector<ReadLabel *> rfAfter;

	/* First, add (mo;rf?)-before */
	std::for_each(g.co_pred_begin(sLab), g.co_pred_end(sLab), [&](auto &wLab) {
		after.push_back(wLab.getPos());
		std::transform(wLab.readers_begin(), wLab.readers_end(), std::back_inserter(after),
			       [&](auto &rLab) { return rLab.getPos(); });
	});

	/* Then, we add the reader list for the initializer */
	auto initRfs = g.getInitRfsAtLoc(sLab->getAddr());
	after.insert(after.end(), initRfs.begin(), initRfs.end());
	return after;
}

std::vector<Event> IMMChecker::getCoherentRevisits(const WriteLabel *sLab, const VectorClock &pporf)
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
IMMChecker::getCoherentPlacings(SAddr addr, Event store, bool isRMW)
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
