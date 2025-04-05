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

#include "RAChecker.hpp"
#include "ADT/VSet.hpp"
#include "ADT/View.hpp"
#include "Config/Config.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/GraphUtils.hpp"
#include "Verification/VerificationError.hpp"

bool RAChecker::isDepTracking() const { return 0; }

bool RAChecker::visitCalc61_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool RAChecker::visitCalc61_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc61_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitCalc61_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc61_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc61_3(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

bool RAChecker::visitCalc61_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc61_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc61_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc61_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		if (!visitCalc61_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc61_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc61_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

View RAChecker::visitCalc61(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitCalc61_2(lab, calcRes);
	return calcRes;
}
auto RAChecker::checkCalc61(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc61(lab);
}
bool RAChecker::visitCalc67_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool RAChecker::visitCalc67_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(1)); true) {
			if (!visitCalc67_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitCalc67_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc67_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc67_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc67_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc67_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			if (!visitCalc67_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc67_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			if (!visitCalc67_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc67_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc67_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc67_0(pLab, calcRes)) {
					return false;
				}
			}

	visitedCalc67_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc67_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc67_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc67_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc67_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc67_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc67_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc67_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc67_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc67_0(pLab, calcRes)) {
					return false;
				}
			}

	visitedCalc67_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc67_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc67_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc67_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc67_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc67_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCalc67_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc67_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc67_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc67_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc67_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc67_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc67_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc67_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc67_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc67_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc67_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc67_5(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}

	visitedCalc67_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc67_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc67_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc67_7(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc67_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedCalc67_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc67_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc67_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc67_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc67_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc67_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadStartLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc67_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc67_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	return true;
}

bool RAChecker::visitCalc67_7(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc67_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc67_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc67_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc67_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc67_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc67_0(pLab, calcRes)) {
					return false;
				}
			}

	return true;
}

View RAChecker::visitCalc67(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitedCalc67_2.clear();
	visitedCalc67_2.resize(g.getMaxStamp().get() + 1);
	visitedCalc67_3.clear();
	visitedCalc67_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc67_4.clear();
	visitedCalc67_4.resize(g.getMaxStamp().get() + 1);
	visitedCalc67_5.clear();
	visitedCalc67_5.resize(g.getMaxStamp().get() + 1);

	visitCalc67_6(lab, calcRes);
	return calcRes;
}
auto RAChecker::checkCalc67(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc67(lab);
}
bool RAChecker::visitCalc69_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool RAChecker::visitCalc69_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(2)); true) {
			if (!visitCalc69_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitCalc69_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc69_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc69_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc69_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}

	visitedCalc69_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc69_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc69_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc69_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc69_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc69_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}

	visitedCalc69_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc69_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc69_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc69_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc69_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCalc69_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc69_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc69_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc69_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc69_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc69_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc69_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_5(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}

	visitedCalc69_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCalc69_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc69_7(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc69_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() &&
		    !(llvm::isa<AbstractLockCasReadLabel>(pLab))) {
			auto status = visitedCalc69_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<FenceLabel>(lab) &&
	    !(llvm::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc69_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(lab) &&
	    !(llvm::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc69_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadStartLabel>(lab) &&
	    !(llvm::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc69_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc69_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitCalc69_7(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc69_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc69_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc69_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc69_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}

	return true;
}

View RAChecker::visitCalc69(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitedCalc69_2.clear();
	visitedCalc69_2.resize(g.getMaxStamp().get() + 1);
	visitedCalc69_3.clear();
	visitedCalc69_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc69_4.clear();
	visitedCalc69_4.resize(g.getMaxStamp().get() + 1);
	visitedCalc69_5.clear();
	visitedCalc69_5.resize(g.getMaxStamp().get() + 1);

	visitCalc69_6(lab, calcRes);
	return calcRes;
}
auto RAChecker::checkCalc69(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc69(lab);
}
void RAChecker::calculateSaved(EventLabel *lab) {}

void RAChecker::calculateViews(EventLabel *lab)
{

	lab->addView(checkCalc61(lab));

	lab->addView(checkCalc67(lab));
	if (!getConf()->collectLinSpec && !getConf()->checkLinSpec)
		lab->addView({});
	else
		lab->addView(checkCalc69(lab));
}

void RAChecker::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
	lab->setPrefixView(calculatePrefixView(lab));
}

const View &RAChecker::getHbView(const EventLabel *lab) const { return lab->view(1); }

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

static auto splitLocMOBefore(MemAccessLabel *lab) -> ExecutionGraph::co_iterator
{
	auto &g = *lab->getParent();
	auto rit = std::find_if(g.co_rbegin(lab->getAddr()), g.co_rend(lab->getAddr()),
				[&](auto &oLab) { return isWriteRfBefore(&oLab, lab); });
	/* Convert to forward iterator, but be _really_ careful */
	return (rit == g.co_rend(lab->getAddr())) ? g.co_begin(lab->getAddr())
						  : ++ExecutionGraph::co_iterator(*rit);
}

static auto splitLocMOAfterHb(ReadLabel *rLab) -> ExecutionGraph::co_iterator
{
	auto &g = *rLab->getParent();
	if (std::any_of(g.init_rf_begin(rLab->getAddr()), g.init_rf_end(rLab->getAddr()),
			[rLab](auto &rfLab) { return rfLab.view(1).contains(rLab->getPos()); }))
		return g.co_begin(rLab->getAddr());

	auto it = std::find_if(g.co_begin(rLab->getAddr()), g.co_end(rLab->getAddr()),
			       [&](auto &wLab) { return isHbOptRfBefore(rLab, &wLab); });
	if (it == g.co_end(rLab->getAddr()) || it->view(1).contains(rLab->getPos()))
		return it;
	return ++it;
}

static auto splitLocMOAfter(WriteLabel *wLab) -> ExecutionGraph::co_iterator
{
	auto &g = *wLab->getParent();
	return std::find_if(g.co_begin(wLab->getAddr()), g.co_end(wLab->getAddr()),
			    [&](auto &sLab) { return isHbOptRfBefore(wLab, &sLab); });
}

auto RAChecker::getCoherentStores(ReadLabel *rLab) -> std::vector<EventLabel *>
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
	auto begIt = splitLocMOBefore(rLab);
	if (begIt == g.co_begin(rLab->getAddr()))
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
	auto endIt = (isDepTracking()) ? splitLocMOAfterHb(rLab) : g.co_end(rLab->getAddr());
	std::transform(begIt, endIt, std::back_inserter(stores), [&](auto &lab) { return &lab; });
	return stores;
}

static auto getMOOptRfAfter(WriteLabel *sLab) -> std::vector<EventLabel *>
{
	auto &g = *sLab->getParent();
	std::vector<EventLabel *> after;
	std::vector<ReadLabel *> rfAfter;

	std::for_each(g.co_succ_begin(sLab), g.co_succ_end(sLab), [&](auto &wLab) {
		after.push_back(&wLab);
		std::transform(wLab.readers_begin(), wLab.readers_end(),
			       std::back_inserter(rfAfter), [&](auto &rLab) { return &rLab; });
	});
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
	std::for_each(g.co_pred_begin(sLab), g.co_pred_end(sLab), [&](auto &wLab) {
		after.push_back(&wLab);
		std::transform(wLab.readers_begin(), wLab.readers_end(),
			       std::back_inserter(rfAfter), [&](auto &rLab) { return &rLab; });
	});
	std::transform(rfAfter.begin(), rfAfter.end(), std::back_inserter(after),
		       [](auto *rLab) { return rLab; });

	/* Then, we add the reader list for the initializer */
	std::for_each(g.init_rf_begin(sLab->getAddr()), g.init_rf_end(sLab->getAddr()),
		      [&](auto &rLab) { after.insert(after.end(), &rLab); });
	return after;
}

static auto getRevisitableFrom(WriteLabel *sLab, const VectorClock &pporf, WriteLabel *coPred)
	-> std::vector<ReadLabel *>
{
	auto &g = *sLab->getParent();
	auto confLab = findPendingRMW(sLab);
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

void RAChecker::filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls)
{
	auto &g = *sLab->getParent();

	/* If this store is po- and mo-maximal then we are done */
	if (!isDepTracking() && sLab == g.co_max(sLab->getAddr()))
		return;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(),
				[&](auto &eLab) {
					auto &before = eLab->view(1);
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

	/* ...and also exclude (mo^-1; rf?; (hb^-1)?; sb^-1)-after reads in
	 * the resulting graph */
	auto &before = sLab->getPrefixView();
	auto moInvOptRfs = getMOInvOptRfAfter(sLab);
	ls.erase(std::remove_if(
			 ls.begin(), ls.end(),
			 [&](auto &eLab) {
				 auto v = g.getViewFromStamp(eLab->getStamp());
				 v->update(before);
				 return std::any_of(
					 moInvOptRfs.begin(), moInvOptRfs.end(), [&](auto &evLab) {
						 return v->contains(evLab->getPos()) &&
							evLab->view(1).contains(eLab->getPos());
					 });
			 }),
		 ls.end());
}

auto RAChecker::getCoherentPlacings(WriteLabel *wLab) -> std::vector<EventLabel *>
{
	auto &g = *wLab->getParent();
	std::vector<EventLabel *> result;

	/* If it is an RMW store, there is only one possible position in MO */
	if (wLab->isRMW()) {
		auto *rLab = llvm::dyn_cast<ReadLabel>(g.po_imm_pred(wLab));
		BUG_ON(!rLab);
		auto *rfLab = rLab->getRf();
		BUG_ON(!rfLab);
		result.push_back(rfLab);
		return result;
	}

	/* Otherwise, we calculate the full range and add the store */
	auto rangeBegin = splitLocMOBefore(wLab);
	auto rangeEnd = (isDepTracking()) ? splitLocMOAfter(wLab) : g.co_end(wLab->getAddr());
	auto cos = llvm::iterator_range(rangeBegin, rangeEnd) |
		   std::views::filter([&](auto &sLab) { return !sLab.isRMW(); }) |
		   std::views::transform([&](auto &sLab) {
			   auto *pLab = g.co_imm_pred(&sLab);
			   return pLab ? (EventLabel *)pLab : (EventLabel *)g.getInitLabel();
		   });
	std::ranges::copy(cos, std::back_inserter(result));
	result.push_back(rangeEnd == g.co_end(wLab->getAddr())
				 ? g.co_max(wLab->getAddr())
				 : (!g.co_imm_pred(&*rangeEnd)
					    ? (EventLabel *)g.getInitLabel()
					    : (EventLabel *)g.co_imm_pred(&*rangeEnd)));
	return result;
}
bool RAChecker::visitCoherence_0(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (lab == initLab)
		return false;

	return true;
}

bool RAChecker::visitCoherence_1(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_9[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_9(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	return true;
}

bool RAChecker::visitCoherence_2(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedCoherence_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_4(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCoherence_0(pLab, initLab)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_6(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto status = visitedCoherence_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_6(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadStartLabel>(pLab)) {
			auto status = visitedCoherence_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_6(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCoherence_7(pLab, initLab)) {
			return false;
		}
	}

	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_3(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_3(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_2(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto status = visitedCoherence_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_2(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto status = visitedCoherence_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_2(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_6(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCoherence_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_4(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCoherence_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_3(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCoherence_5(pLab, initLab)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto status = visitedCoherence_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_2(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
			}
		}

	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_5(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
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

bool RAChecker::visitCoherence_6(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_6[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_6[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_6(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}

	visitedCoherence_6[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_7(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_2(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCoherence_0(pLab, initLab)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCoherence_0(pLab, initLab)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitCoherence_8(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_8[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_8[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_2(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_2(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_8[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_8(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCoherence_8[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_9(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_9[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_9[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_13[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_13(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto status = visitedCoherence_13[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_13(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadStartLabel>(pLab)) {
			auto status = visitedCoherence_13[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_13(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedCoherence_11[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_11(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCoherence_14(pLab, initLab)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_9[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_9(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}

	visitedCoherence_9[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_10(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_10[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_10[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_13[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_13(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_9[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_9(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto status = visitedCoherence_9[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_9(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto status = visitedCoherence_9[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_9(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_8[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_8(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto status = visitedCoherence_8[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_8(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto status = visitedCoherence_8[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_8(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCoherence_10[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_11(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_11[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_11[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCoherence_10[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_10(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCoherence_12(pLab, initLab)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto status = visitedCoherence_9[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_9(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto status = visitedCoherence_8[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_8(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCoherence_11[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_12(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCoherence_11[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_11(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	return true;
}

bool RAChecker::visitCoherence_13(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_13[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_13[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_13[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_13(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_11[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_11(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}

	visitedCoherence_13[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RAChecker::visitCoherence_14(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCoherence_9[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_9(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCoherence_9[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_9(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_9[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_9(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCoherence_8[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_8(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
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

bool RAChecker::visitCoherenceRelinche(const ExecutionGraph &g) const
{
	for (auto &lab : g.labels()) {
		if (!llvm::isa<MethodBeginLabel>(&lab))
			continue;
		visitedCoherence_2.clear();
		visitedCoherence_2.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_3.clear();
		visitedCoherence_3.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_4.clear();
		visitedCoherence_4.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_6.clear();
		visitedCoherence_6.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_8.clear();
		visitedCoherence_8.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_9.clear();
		visitedCoherence_9.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_10.clear();
		visitedCoherence_10.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_11.clear();
		visitedCoherence_11.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_13.clear();
		visitedCoherence_13.resize(g.getMaxStamp().get() + 1);
		if (true && !visitCoherence_1(&lab, &lab))
			return false;
	}
	return true;
}

bool RAChecker::visitError1(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError1_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool RAChecker::visitLHSUnlessError1_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (auto pLab = alloc_pred(g, lab); pLab) {
		if (!visitLHSUnlessError1_0(pLab, v)) {
			return false;
		}
	}

	return true;
}

bool RAChecker::visitUnlessError1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError1Accepting.clear();
	visitedLHSUnlessError1Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true && visitLHSUnlessError1_1(lab, v);
}

bool RAChecker::checkError1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError1(lab))
		return true;

	return visitError1(lab);
}
bool RAChecker::visitError2(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError2_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool RAChecker::visitLHSUnlessError2_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<FreeLabel>(pLab) &&
				    !llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError2_0(pLab)) {
						return false;
					}
				}
	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError2_0(pLab)) {
						return false;
					}
				}
	if (true && llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<FreeLabel>(pLab) &&
				    !llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError2_0(pLab)) {
						return false;
					}
				}
	if (true && llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError2_0(pLab)) {
						return false;
					}
				}

	return true;
}

bool RAChecker::visitUnlessError2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError2Accepting.clear();
	visitedLHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError2Accepting.clear();
	visitedRHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError2_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError2Accepting.size(); i++) {
		if (visitedLHSUnlessError2Accepting[i] && !visitedRHSUnlessError2Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(),
						[&](auto &lab) { return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RAChecker::checkError2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError2(lab))
		return true;

	return visitError2(lab);
}
bool RAChecker::visitError3(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError3_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool RAChecker::visitLHSUnlessError3_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : alloc_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError3_0(pLab, v)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitLHSUnlessError3_2(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError3_1(pLab, v)) {
				return false;
			}
		}
	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError3_0(pLab, v)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitUnlessError3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError3Accepting.clear();
	visitedLHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true && visitLHSUnlessError3_2(lab, v);
}

bool RAChecker::checkError3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError3(lab))
		return true;

	return visitError3(lab);
}
bool RAChecker::visitError4(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError4_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool RAChecker::visitLHSUnlessError4_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = free_succ(g, lab); pLab)
		if (true && llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError4_0(pLab)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitLHSUnlessError4_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = alloc_pred(g, lab); pLab) {
		if (!visitLHSUnlessError4_1(pLab)) {
			return false;
		}
	}

	return true;
}

bool RAChecker::visitUnlessError4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError4Accepting.clear();
	visitedLHSUnlessError4Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError4Accepting.clear();
	visitedRHSUnlessError4Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError4_2(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError4Accepting.size(); i++) {
		if (visitedLHSUnlessError4Accepting[i] && !visitedRHSUnlessError4Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(),
						[&](auto &lab) { return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RAChecker::checkError4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}
bool RAChecker::visitError5(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError5_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool RAChecker::visitLHSUnlessError5_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : alloc_succs(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && llvm::isa<MemAccessLabel>(pLab) &&
			    llvm::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() &&
			    !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(pLab))) {
				if (!visitLHSUnlessError5_0(pLab, v)) {
					return false;
				}
			}

	return true;
}

bool RAChecker::visitLHSUnlessError5_2(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError5_1(pLab, v)) {
				return false;
			}
		}
	if (true && llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab)
			if (true && llvm::isa<MemAccessLabel>(pLab) &&
			    llvm::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() &&
			    !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(pLab))) {
				if (!visitLHSUnlessError5_0(pLab, v)) {
					return false;
				}
			}

	return true;
}

bool RAChecker::visitUnlessError5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError5Accepting.clear();
	visitedLHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true && visitLHSUnlessError5_2(lab, v);
}

bool RAChecker::checkError5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError5(lab))
		return true;

	return visitError5(lab);
}
bool RAChecker::visitError6(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError6_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool RAChecker::visitLHSUnlessError6_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = free_succ(g, lab); pLab)
		if (true && llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError6_0(pLab)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitLHSUnlessError6_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<MemAccessLabel>(lab) &&
	    llvm::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() &&
	    !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(lab)))
		if (auto pLab = alloc_pred(g, lab); pLab) {
			if (!visitLHSUnlessError6_1(pLab)) {
				return false;
			}
		}

	return true;
}

bool RAChecker::visitUnlessError6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError6Accepting.clear();
	visitedLHSUnlessError6Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError6Accepting.clear();
	visitedRHSUnlessError6Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError6_2(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError6Accepting.size(); i++) {
		if (visitedLHSUnlessError6Accepting[i] && !visitedRHSUnlessError6Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(),
						[&](auto &lab) { return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RAChecker::checkError6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError6(lab))
		return true;

	return visitError6(lab);
}
bool RAChecker::visitError7(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError7_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool RAChecker::visitLHSUnlessError7_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && lab->isNotAtomic() && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError7_0(pLab, v)) {
						return false;
					}
				}
	if (true && lab->isNotAtomic() && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<ReadLabel>(pLab)) {
					if (!visitLHSUnlessError7_0(pLab, v)) {
						return false;
					}
				}
	if (true && lab->isNotAtomic() && llvm::isa<ReadLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError7_0(pLab, v)) {
						return false;
					}
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError7_0(pLab, v)) {
						return false;
					}
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && llvm::isa<ReadLabel>(pLab)) {
					if (!visitLHSUnlessError7_0(pLab, v)) {
						return false;
					}
				}
	if (true && llvm::isa<ReadLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError7_0(pLab, v)) {
						return false;
					}
				}

	return true;
}

bool RAChecker::visitUnlessError7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError7Accepting.clear();
	visitedLHSUnlessError7Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true && visitLHSUnlessError7_1(lab, v);
}

bool RAChecker::checkError7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError7(lab))
		return true;

	return visitError7(lab);
}
bool RAChecker::visitWarning8(const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessWarning8_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool RAChecker::visitLHSUnlessWarning8_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessWarning8_0(pLab, v)) {
						return false;
					}
				}

	return true;
}

bool RAChecker::visitUnlessWarning8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessWarning8Accepting.clear();
	visitedLHSUnlessWarning8Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true && visitLHSUnlessWarning8_1(lab, v);
}

bool RAChecker::checkWarning8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessWarning8(lab))
		return true;

	return visitWarning8(lab);
}
VerificationError RAChecker::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	if (!checkError1(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessNonMalloc;
	}

	if (!checkError2(lab)) {
		race = cexLab;
		return VerificationError::VE_DoubleFree;
	}

	if (!checkError3(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError4(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError5(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError6(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError7(lab)) {
		race = cexLab;
		return VerificationError::VE_RaceNotAtomic;
	}

	return VerificationError::VE_OK;
}

std::vector<VerificationError>
RAChecker::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings,
			 std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning8(lab)) {
		racyLabs.push_back(cexLab);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool RAChecker::isConsistent(const EventLabel *lab) const { return true; }

bool RAChecker::isConsistent(const ExecutionGraph &g) const { return true; }

bool RAChecker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true && visitCoherenceRelinche(g);
}

View RAChecker::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View pporf;
	pporf.updateIdx(lab->getPos());

	auto *pLab = g.po_imm_pred(lab);
	if (!pLab)
		return pporf;
	pporf.update(pLab->getPrefixView());
	auto *rLab = llvm::dyn_cast<ReadLabel>(pLab);
	if (rLab && rLab->getRf())
		pporf.update(rLab->getRf()->getPrefixView());
	auto *tsLab = llvm::dyn_cast<ThreadStartLabel>(pLab);
	if (tsLab && tsLab->getCreate())
		pporf.update(tsLab->getCreate()->getPrefixView());
	auto *tjLab = llvm::dyn_cast<ThreadJoinLabel>(pLab);
	if (tjLab && g.getLastThreadLabel(tjLab->getChildId()))
		pporf.update(g.getLastThreadLabel(tjLab->getChildId())->getPrefixView());
	return pporf;
}
std::unique_ptr<VectorClock> RAChecker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}
