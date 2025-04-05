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
#include "ADT/DepView.hpp"
#include "ADT/VSet.hpp"
#include "Config/Config.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/GraphUtils.hpp"
#include "Verification/VerificationError.hpp"

bool IMMChecker::isDepTracking() const { return 1; }

bool IMMChecker::visitCalc66_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool IMMChecker::visitCalc66_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc66_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool IMMChecker::visitCalc66_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc66_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc66_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc66_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc66_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc66_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc66_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			if (!visitCalc66_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc66_1(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc66_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc66_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc66_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc66_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc66_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc66_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc66_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc66_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc66_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc66_1(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc66_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc66_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc66_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc66_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc66_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc66_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCalc66_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc66_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc66_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc66_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc66_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc66_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc66_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc66_5(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc66_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc66_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc66_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc66_1(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc66_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc66_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc66_7(pLab, calcRes)) {
			return false;
		}
	}
	if (true && lab->isAtLeastAcquire() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc66_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc66_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc66_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc66_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedCalc66_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc66_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc66_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc66_1(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

bool IMMChecker::visitCalc66_7(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc66_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc66_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc66_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc66_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc66_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc66_1(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

View IMMChecker::visitCalc66(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitedCalc66_2.clear();
	visitedCalc66_2.resize(g.getMaxStamp().get() + 1);
	visitedCalc66_3.clear();
	visitedCalc66_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc66_4.clear();
	visitedCalc66_4.resize(g.getMaxStamp().get() + 1);
	visitedCalc66_5.clear();
	visitedCalc66_5.resize(g.getMaxStamp().get() + 1);

	visitCalc66_6(lab, calcRes);
	return calcRes;
}
auto IMMChecker::checkCalc66(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc66(lab);
}
bool IMMChecker::visitCalc68_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool IMMChecker::visitCalc68_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(1)); true) {
			if (!visitCalc68_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool IMMChecker::visitCalc68_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc68_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc68_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc68_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}

	visitedCalc68_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc68_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc68_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc68_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc68_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_2(pLab, calcRes)) {
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
			auto status = visitedCalc68_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCalc68_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc68_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc68_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc68_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc68_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCalc68_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc68_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc68_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc68_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc68_2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_2(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc68_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_5(pLab, calcRes)) {
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
			auto status = visitedCalc68_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_4(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCalc68_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCalc68_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc68_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc68_0(pLab, calcRes)) {
				return false;
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<FenceLabel>(lab) &&
	    !(llvm::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc68_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(lab) &&
	    !(llvm::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc68_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && llvm::isa<ThreadStartLabel>(lab) &&
	    !(llvm::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc68_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc68_7(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() &&
		    !(llvm::isa<AbstractLockCasReadLabel>(pLab))) {
			auto status = visitedCalc68_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitCalc68_7(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc68_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc68_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc68_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc68_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}

	return true;
}

View IMMChecker::visitCalc68(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitedCalc68_2.clear();
	visitedCalc68_2.resize(g.getMaxStamp().get() + 1);
	visitedCalc68_3.clear();
	visitedCalc68_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc68_4.clear();
	visitedCalc68_4.resize(g.getMaxStamp().get() + 1);
	visitedCalc68_5.clear();
	visitedCalc68_5.resize(g.getMaxStamp().get() + 1);

	visitCalc68_6(lab, calcRes);
	return calcRes;
}
auto IMMChecker::checkCalc68(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc68(lab);
}
bool IMMChecker::visitCalc79_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool IMMChecker::visitCalc79_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(2)); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool IMMChecker::visitCalc79_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc79_3(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc79_1(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

bool IMMChecker::visitCalc79_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc79_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc79_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc79_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc79_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc79_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		if (!visitCalc79_1(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

View IMMChecker::visitCalc79(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitCalc79_2(lab, calcRes);
	return calcRes;
}
auto IMMChecker::checkCalc79(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc79(lab);
}
void IMMChecker::calculateSaved(EventLabel *lab) {}

void IMMChecker::calculateViews(EventLabel *lab)
{

	lab->addView(checkCalc66(lab));
	if (!getConf()->collectLinSpec && !getConf()->checkLinSpec)
		lab->addView({});
	else
		lab->addView(checkCalc68(lab));

	lab->addView(checkCalc79(lab));
}

void IMMChecker::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
}

const View &IMMChecker::getHbView(const EventLabel *lab) const { return lab->view(0); }

static auto isWriteRfBefore(const WriteLabel *wLab, const EventLabel *lab) -> bool
{
	auto &before = lab->view(0);
	return before.contains(wLab->getPos()) ||
	       std::ranges::any_of(wLab->readers(),
				   [&](auto &rLab) { return before.contains(rLab.getPos()); });
}

static auto isHbOptRfBefore(const EventLabel *lab, const WriteLabel *wLab) -> bool
{
	return wLab->view(0).contains(lab->getPos()) ||
	       std::ranges::any_of(wLab->readers(), [&](auto &rLab) {
		       return rLab.view(0).contains(lab->getPos());
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
			[rLab](auto &rfLab) { return rfLab.view(0).contains(rLab->getPos()); }))
		return g.co_begin(rLab->getAddr());

	auto it = std::find_if(g.co_begin(rLab->getAddr()), g.co_end(rLab->getAddr()),
			       [&](auto &wLab) { return isHbOptRfBefore(rLab, &wLab); });
	if (it == g.co_end(rLab->getAddr()) || it->view(0).contains(rLab->getPos()))
		return it;
	return ++it;
}

static auto splitLocMOAfter(WriteLabel *wLab) -> ExecutionGraph::co_iterator
{
	auto &g = *wLab->getParent();
	return std::find_if(g.co_begin(wLab->getAddr()), g.co_end(wLab->getAddr()),
			    [&](auto &sLab) { return isHbOptRfBefore(wLab, &sLab); });
}

auto IMMChecker::getCoherentStores(ReadLabel *rLab) -> std::vector<EventLabel *>
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

void IMMChecker::filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls)
{
	auto &g = *sLab->getParent();

	/* If this store is po- and mo-maximal then we are done */
	if (!isDepTracking() && sLab == g.co_max(sLab->getAddr()))
		return;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(),
				[&](auto &eLab) {
					auto &before = eLab->view(0);
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
				[&](auto &eLab) { return sLab->view(0).contains(eLab->getPos()); }),
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
							evLab->view(0).contains(eLab->getPos());
					 });
			 }),
		 ls.end());
}

auto IMMChecker::getCoherentPlacings(WriteLabel *wLab) -> std::vector<EventLabel *>
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
bool IMMChecker::visitCoherence_0(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (lab == initLab)
		return false;

	return true;
}

bool IMMChecker::visitCoherence_1(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherence_2(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCoherence_7(pLab, initLab)) {
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

	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCoherence_3(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_3[lab->getStamp().get()] = NodeStatus::entered;

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

	visitedCoherence_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCoherence_4(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::entered;

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
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCoherence_5(pLab, initLab)) {
				return false;
			}
		}
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
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
			}
		}

	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCoherence_5(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherence_6(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_6[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_6[lab->getStamp().get()] = NodeStatus::entered;

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

	visitedCoherence_6[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCoherence_7(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherence_8(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherence_9(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherence_10(const EventLabel *lab, const EventLabel *initLab) const
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

	visitedCoherence_10[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool IMMChecker::visitCoherence_11(const EventLabel *lab, const EventLabel *initLab) const
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
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCoherence_12(pLab, initLab)) {
				return false;
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

bool IMMChecker::visitCoherence_12(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherence_13(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherence_14(const EventLabel *lab, const EventLabel *initLab) const
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

bool IMMChecker::visitCoherenceRelinche(const ExecutionGraph &g) const
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

bool IMMChecker::visitConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_0[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_2(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_6(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_4(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_4(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_0(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_0[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_1[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_6(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_6(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_4(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_1[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_2[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitConsAcyclic1_3(pLab)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_1(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_6(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_2[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_2(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitConsAcyclic1_4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_4[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_2(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_4(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic1_4[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_6(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitConsAcyclic1_6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_6[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_2(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_4(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_4(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitConsAcyclic1_5(pLab)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_6[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_7[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_7[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_8[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_10(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_8[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_9[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_9(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_9[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_10(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_10[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_9(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic1_10[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_11(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_10(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitConsAcyclic1_12(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_12[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_12(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_10(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic1_12[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_13(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_13[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_19(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_16(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_18(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_18(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_13[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_14(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_14[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_10(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_10(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_21(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	visitedConsAcyclic1_14[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_15(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_15[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_20(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_20(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_18(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_14(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_14(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_21(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_15[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_16(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_16[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_20(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitConsAcyclic1_17(pLab)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_15(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_14(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_16[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_17(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_16(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitConsAcyclic1_18(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_18[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_16(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_18(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic1_18[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_19(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_19[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_13(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_20(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_14(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_14(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_19[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_20(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_20[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_10(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_10(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_10(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_20(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_20(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_20[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1_21(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	++visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_21[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_7(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_8(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_8[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_8(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_16(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic1_16[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_16(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_12(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_12[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_12(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic1_2[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_2(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_4(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_4[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_4(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_10(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_10(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isAtLeastAcquire()) {
					auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen) {
						if (!visitConsAcyclic1_10(pLab)) {
							return false;
						}

					} else if (node.status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting > node.count ||
						    0)) {

						return false;
					} else if (node.status == NodeStatus::left) {
					}
				}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic1_10[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_10(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_20[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_20(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_18(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_18(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_18[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_18(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_0(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_14(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_14[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_14(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_21(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = co_imm_pred(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_21(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		for (auto &tmp : fr_imm_preds(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isSC()) {
					auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen) {
						if (!visitConsAcyclic1_21(pLab)) {
							return false;
						}

					} else if (node.status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting > node.count ||
						    1)) {

						return false;
					} else if (node.status == NodeStatus::left) {
					}
				}
	if (true && lab->isSC())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_21(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_21[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_21(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	--visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_21[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_4.clear();
	visitedConsAcyclic1_4.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.clear();
	visitedConsAcyclic1_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.clear();
	visitedConsAcyclic1_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.clear();
	visitedConsAcyclic1_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.clear();
	visitedConsAcyclic1_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.clear();
	visitedConsAcyclic1_10.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.clear();
	visitedConsAcyclic1_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.clear();
	visitedConsAcyclic1_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.clear();
	visitedConsAcyclic1_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.clear();
	visitedConsAcyclic1_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_16.clear();
	visitedConsAcyclic1_16.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.clear();
	visitedConsAcyclic1_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.clear();
	visitedConsAcyclic1_19.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_20.clear();
	visitedConsAcyclic1_20.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_21.clear();
	visitedConsAcyclic1_21.resize(g.getMaxStamp().get() + 1);
	return true &&
	       (visitedConsAcyclic1_6[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_6(lab)) &&
	       (visitedConsAcyclic1_7[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_7(lab)) &&
	       (visitedConsAcyclic1_8[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_8(lab)) &&
	       (visitedConsAcyclic1_10[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_10(lab)) &&
	       (visitedConsAcyclic1_13[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_13(lab)) &&
	       (visitedConsAcyclic1_14[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_14(lab)) &&
	       (visitedConsAcyclic1_20[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_20(lab)) &&
	       (visitedConsAcyclic1_21[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_21(lab));
}

bool IMMChecker::visitConsAcyclic1Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_4.clear();
	visitedConsAcyclic1_4.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.clear();
	visitedConsAcyclic1_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.clear();
	visitedConsAcyclic1_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.clear();
	visitedConsAcyclic1_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.clear();
	visitedConsAcyclic1_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.clear();
	visitedConsAcyclic1_10.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.clear();
	visitedConsAcyclic1_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.clear();
	visitedConsAcyclic1_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.clear();
	visitedConsAcyclic1_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.clear();
	visitedConsAcyclic1_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_16.clear();
	visitedConsAcyclic1_16.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.clear();
	visitedConsAcyclic1_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.clear();
	visitedConsAcyclic1_19.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_20.clear();
	visitedConsAcyclic1_20.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_21.clear();
	visitedConsAcyclic1_21.resize(g.getMaxStamp().get() + 1);
	return true && std::ranges::all_of(g.labels(), [&](auto &lab) {
		       return visitedConsAcyclic1_21[lab.getStamp().get()].status !=
				      NodeStatus::unseen ||
			      visitConsAcyclic1_21(&lab);
	       });
}

bool IMMChecker::visitLHSUnlessConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool IMMChecker::visitLHSUnlessConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : other_labels(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isSC()) {
				if (!visitLHSUnlessConsAcyclic1_0(pLab)) {
					return false;
				}
			}

	return true;
}

bool IMMChecker::visitUnlessConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessConsAcyclic1Accepting.clear();
	visitedLHSUnlessConsAcyclic1Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessConsAcyclic1Accepting.clear();
	visitedRHSUnlessConsAcyclic1Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessConsAcyclic1_1(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessConsAcyclic1Accepting.size(); i++) {
		if (visitedLHSUnlessConsAcyclic1Accepting[i] &&
		    !visitedRHSUnlessConsAcyclic1Accepting[i]) {
			return false;
		}
	}
	return true;
}

bool IMMChecker::checkConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessConsAcyclic1(lab))
		return true;

	return visitConsAcyclic1(lab);
}
bool IMMChecker::checkConsAcyclic1(const ExecutionGraph &g) const
{
	return visitConsAcyclic1Full(g);
}
bool IMMChecker::visitConsAcyclic2_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_0[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	if (auto pLab = poloc_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_0[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_0(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = poloc_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_0[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_1[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_1(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_1[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_2[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_2(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic2_2[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_3[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_5(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_5(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_5(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rfi_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_3(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = rfi_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitConsAcyclic2_4(pLab)) {
				return false;
			}
		}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	visitedConsAcyclic2_3[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_5(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
		    pLab->isDependable()) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitConsAcyclic2_5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_5[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_5(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_5(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	visitedConsAcyclic2_5[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_6[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_6[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_7[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_9(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isSC() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic2_7[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_8[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_8[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_9[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::entered};

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_8[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_8(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitConsAcyclic2_10(pLab)) {
				return false;
			}
		}
	visitedConsAcyclic2_9[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_10(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_9(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitConsAcyclic2_11(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_11[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_9(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic2_11[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_12(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_12[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_7(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic2_7[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_7(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_9(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = co_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_9(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic2_9[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_9(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_6(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic2_6[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_6(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_12[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_13(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_13[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_15(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_17(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_17(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_18[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_18(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic2_13[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_14(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_14[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_14(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_17(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_14[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_15(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_15[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitConsAcyclic2_16(pLab)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_14[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_14(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_15[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_16(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_15(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}

	return true;
}

bool IMMChecker::visitConsAcyclic2_17(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_17[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_15(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_17(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic2_17[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_18(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2_18[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_12(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_13(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic2_18[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2_19(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	++visitedConsAcyclic2Accepting;
	visitedConsAcyclic2_19[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::entered};

	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = poloc_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_0[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_0(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_12[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_12(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_15(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire()) {
				auto &node = visitedConsAcyclic2_15[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_15(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_5(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_5(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_5(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_5(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto &node = visitedConsAcyclic2_5[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_5(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_3(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_3(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_3(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto &node = visitedConsAcyclic2_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_3(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_1[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_1(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic2_19(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic2Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen) {
						if (!visitConsAcyclic2_19(pLab)) {
							return false;
						}

					} else if (node.status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting > node.count ||
						    1)) {

						return false;
					} else if (node.status == NodeStatus::left) {
					}
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen) {
						if (!visitConsAcyclic2_19(pLab)) {
							return false;
						}

					} else if (node.status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting > node.count ||
						    1)) {

						return false;
					} else if (node.status == NodeStatus::left) {
					}
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen) {
						if (!visitConsAcyclic2_19(pLab)) {
							return false;
						}

					} else if (node.status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting > node.count ||
						    1)) {

						return false;
					} else if (node.status == NodeStatus::left) {
					}
				}
	for (auto &tmp : detour_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = poloc_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
				auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
			    pLab->isDependable()) {
				auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_13[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_13(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_2(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_2[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_2(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_17(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_17(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastAcquire() && llvm::isa<ThreadJoinLabel>(pLab)) {
				auto &node = visitedConsAcyclic2_17[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic2_17(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC() && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto &node = visitedConsAcyclic2_18[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic2_18(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	--visitedConsAcyclic2Accepting;
	visitedConsAcyclic2_19[lab->getStamp().get()] = {visitedConsAcyclic2Accepting,
							 NodeStatus::left};

	return true;
}

bool IMMChecker::visitConsAcyclic2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic2Accepting = 0;
	visitedConsAcyclic2_0.clear();
	visitedConsAcyclic2_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_1.clear();
	visitedConsAcyclic2_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_2.clear();
	visitedConsAcyclic2_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_3.clear();
	visitedConsAcyclic2_3.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_5.clear();
	visitedConsAcyclic2_5.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_6.clear();
	visitedConsAcyclic2_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_7.clear();
	visitedConsAcyclic2_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_8.clear();
	visitedConsAcyclic2_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_9.clear();
	visitedConsAcyclic2_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_11.clear();
	visitedConsAcyclic2_11.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_12.clear();
	visitedConsAcyclic2_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_13.clear();
	visitedConsAcyclic2_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_14.clear();
	visitedConsAcyclic2_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_15.clear();
	visitedConsAcyclic2_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_17.clear();
	visitedConsAcyclic2_17.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_18.clear();
	visitedConsAcyclic2_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_19.clear();
	visitedConsAcyclic2_19.resize(g.getMaxStamp().get() + 1);
	return true &&
	       (visitedConsAcyclic2_6[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic2_6(lab)) &&
	       (visitedConsAcyclic2_7[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic2_7(lab)) &&
	       (visitedConsAcyclic2_9[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic2_9(lab)) &&
	       (visitedConsAcyclic2_12[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic2_12(lab)) &&
	       (visitedConsAcyclic2_13[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic2_13(lab));
}

bool IMMChecker::visitConsAcyclic2Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic2Accepting = 0;
	visitedConsAcyclic2_0.clear();
	visitedConsAcyclic2_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_1.clear();
	visitedConsAcyclic2_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_2.clear();
	visitedConsAcyclic2_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_3.clear();
	visitedConsAcyclic2_3.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_5.clear();
	visitedConsAcyclic2_5.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_6.clear();
	visitedConsAcyclic2_6.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_7.clear();
	visitedConsAcyclic2_7.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_8.clear();
	visitedConsAcyclic2_8.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_9.clear();
	visitedConsAcyclic2_9.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_11.clear();
	visitedConsAcyclic2_11.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_12.clear();
	visitedConsAcyclic2_12.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_13.clear();
	visitedConsAcyclic2_13.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_14.clear();
	visitedConsAcyclic2_14.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_15.clear();
	visitedConsAcyclic2_15.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_17.clear();
	visitedConsAcyclic2_17.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_18.clear();
	visitedConsAcyclic2_18.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_19.clear();
	visitedConsAcyclic2_19.resize(g.getMaxStamp().get() + 1);
	return true && std::ranges::all_of(g.labels(), [&](auto &lab) {
		       return visitedConsAcyclic2_19[lab.getStamp().get()].status !=
				      NodeStatus::unseen ||
			      visitConsAcyclic2_19(&lab);
	       });
}

bool IMMChecker::checkConsAcyclic2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitConsAcyclic2(lab);
}
bool IMMChecker::checkConsAcyclic2(const ExecutionGraph &g) const
{
	return visitConsAcyclic2Full(g);
}
bool IMMChecker::visitWarning3(const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessWarning3_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool IMMChecker::visitLHSUnlessWarning3_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessWarning3_0(pLab, v)) {
						return false;
					}
				}

	return true;
}

bool IMMChecker::visitUnlessWarning3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessWarning3Accepting.clear();
	visitedLHSUnlessWarning3Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(2);

	return true && visitLHSUnlessWarning3_1(lab, v);
}

bool IMMChecker::checkWarning3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessWarning3(lab))
		return true;

	return visitWarning3(lab);
}
VerificationError IMMChecker::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	return VerificationError::VE_OK;
}

std::vector<VerificationError>
IMMChecker::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings,
			  std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning3(lab)) {
		racyLabs.push_back(cexLab);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool IMMChecker::isConsistent(const EventLabel *lab) const
{

	return true && checkConsAcyclic1(lab) && checkConsAcyclic2(lab);
}

bool IMMChecker::isConsistent(const ExecutionGraph &g) const
{

	return true && checkConsAcyclic1(g) && checkConsAcyclic2(g);
}

bool IMMChecker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true && visitCoherenceRelinche(g);
}

void IMMChecker::visitPPoRf0(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf1(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf7[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf7(pLab, pporf);
			}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf7[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf7(pLab, pporf);
			}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
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
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
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
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	for (auto &p : ctrl_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	for (auto &p : data_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf2(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
		    pLab->isDependable()) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
		    pLab->isDependable()) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<ReadLabel>(pLab) &&
		    ((llvm::isa<ReadLabel>(pLab) && llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (llvm::isa<WriteLabel>(pLab) && llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
		    pLab->isDependable())
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf3(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf7[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf7(pLab, pporf);
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
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	for (auto &p : addr_preds(g, lab))
		if (auto *pLab = g.getEventLabel(p); true)
			if (true && pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf4(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::entered;
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf7[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf7(pLab, pporf);
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf7[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf7(pLab, pporf);
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf7[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf7(pLab, pporf);
				}
	for (auto &tmp : detour_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = poloc_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
				auto status = visitedPPoRf7[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf7(pLab, pporf);
			}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
			    pLab->isDependable()) {
				auto status = visitedPPoRf7[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf7(pLab, pporf);
			}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto status = visitedPPoRf3[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf3(pLab, pporf);
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
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = poloc_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf5(pLab, pporf);
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = rfe_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf4[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf4(pLab, pporf);
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf4[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf4(pLab, pporf);
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf4[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf4(pLab, pporf);
				}
	for (auto &tmp : detour_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = poloc_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
			    pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = tc_pred(g, lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (auto pLab = rfe_pred(g, lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : ctrl_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable())
					if (pporf.updateIdx(pLab->getPos()); true) {
						auto status = visitedPPoRf0[pLab->getStamp().get()];
						if (status == NodeStatus::unseen)
							visitPPoRf0(pLab, pporf);
					}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : addr_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable())
					if (pporf.updateIdx(pLab->getPos()); true) {
						auto status = visitedPPoRf0[pLab->getStamp().get()];
						if (status == NodeStatus::unseen)
							visitPPoRf0(pLab, pporf);
					}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &p : data_preds(g, lab))
			if (auto *pLab = g.getEventLabel(p); true)
				if (true && pLab->isDependable())
					if (pporf.updateIdx(pLab->getPos()); true) {
						auto status = visitedPPoRf0[pLab->getStamp().get()];
						if (status == NodeStatus::unseen)
							visitPPoRf0(pLab, pporf);
					}
	for (auto &tmp : detour_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (true && llvm::isa<WriteLabel>(lab))
		if (auto pLab = poloc_imm_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab))
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_pred(g, lab); pLab)
			if (true && llvm::isa<ReadLabel>(pLab) &&
			    ((llvm::isa<ReadLabel>(pLab) &&
			      llvm::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (llvm::isa<WriteLabel>(pLab) &&
			      llvm::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
			    pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf5(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = poloc_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (auto pLab = poloc_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf5(pLab, pporf);
	}
	if (auto pLab = poloc_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab)) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = poloc_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && llvm::isa<WriteLabel>(pLab))
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf6(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf7(const EventLabel *lab, DepView &pporf) const
{
	auto &g = *lab->getParent();

	visitedPPoRf7[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire())
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab))
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	visitedPPoRf7[lab->getStamp().get()] = NodeStatus::left;
}

DepView IMMChecker::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
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
	visitedPPoRf7.clear();
	visitedPPoRf7.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);

	visitPPoRf4(lab, pporf);
	visitPPoRf7(lab, pporf);
	return pporf;
}
std::unique_ptr<VectorClock> IMMChecker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<DepView>(calcPPoRfBefore(lab));
}
