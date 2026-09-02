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

// NOLINTBEGIN
#include "RAChecker.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/GraphUtils.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/VerificationError.hpp"

#include <algorithm>

bool RAChecker::isDepTracking() const { return 0; }

bool RAChecker::visitCalc70Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = lab; true)
				if (calcRes.update(pLab->view(0)); true) {
					worklist.emplace_back(0, pLab);
				}

			break;
		}
		case 2: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.tc_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.tj_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

View RAChecker::visitCalc70(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{2, lab},
	};

	visitCalc70Iterative(startStates, calcRes);
	return calcRes;
}

auto RAChecker::checkCalc70(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc70(lab);
}

bool RAChecker::visitCalc76Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = lab; true)
				if (calcRes.update(pLab->view(1)); true) {
					worklist.emplace_back(0, pLab);
				}

			break;
		}
		case 2: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.tc_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.tj_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (true && lab->isAtLeastAcquire())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease())
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (true && lab->isAtLeastAcquire())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease()) {
						worklist.emplace_back(1, pLab);
					}
			if (true && lab->isAtLeastAcquire())
				if (auto pLab = g.rf_pred(lab); pLab) {
					auto status =
						visitedCalc76_3.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    ((genmc::isa<ReadLabel>(pLab) &&
					      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
					     (genmc::isa<WriteLabel>(pLab) &&
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
						auto status = visitedCalc76_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc76_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc76_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadStartLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc76_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCalc76_3.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc76_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCalc76_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc76_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}

			break;
		}
		case 4: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease())
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCalc76_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status =
						visitedCalc76_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedCalc76_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc76_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc76_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(4, pLab);
				}

			break;
		}
		case 6: {
			if (isFinishing) {
				visitedCalc76_6.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc76_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCalc76_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease())
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCalc76_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status =
						visitedCalc76_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc76_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

View RAChecker::visitCalc76(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc76_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc76_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc76_6.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{2, lab},
	};

	visitCalc76Iterative(startStates, calcRes);
	return calcRes;
}

auto RAChecker::checkCalc76(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc76(lab);
}

bool RAChecker::visitCalc77Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = lab; true)
				if (calcRes.update(pLab->view(2)); true) {
					worklist.emplace_back(0, pLab);
				}

			break;
		}
		case 2: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.tc_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.tj_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (true && lab->isAtLeastAcquire() &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease())
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (true && lab->isAtLeastAcquire() &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease()) {
						worklist.emplace_back(1, pLab);
					}
			if (true && lab->isAtLeastAcquire() &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.rf_pred(lab); pLab) {
					auto status =
						visitedCalc77_3.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    ((genmc::isa<ReadLabel>(pLab) &&
					      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
					     (genmc::isa<WriteLabel>(pLab) &&
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
						auto status = visitedCalc77_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc77_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc77_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadStartLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc77_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCalc77_3.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc77_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCalc77_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc77_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}

			break;
		}
		case 4: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease())
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCalc77_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status =
						visitedCalc77_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedCalc77_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc77_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc77_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(4, pLab);
				}

			break;
		}
		case 6: {
			if (isFinishing) {
				visitedCalc77_6.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc77_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCalc77_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease())
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCalc77_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status =
						visitedCalc77_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc77_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

View RAChecker::visitCalc77(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc77_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc77_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc77_6.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{2, lab},
	};

	visitCalc77Iterative(startStates, calcRes);
	return calcRes;
}

auto RAChecker::checkCalc77(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc77(lab);
}

void RAChecker::calculateSaved([[maybe_unused]] EventLabel *lab) {}

void RAChecker::calculateViews(EventLabel *lab)
{
	lab->setViews({});

	lab->addView(checkCalc70(lab));

	lab->addView(checkCalc76(lab));
	if (!getConf()->collectLinSpec && !getConf()->checkLinSpec)
		lab->addView({});
	else
		lab->addView(checkCalc77(lab));
}

void RAChecker::updateMMViews(EventLabel *lab)
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
	auto rit = std::ranges::find_if(g.rco(addr),
					[&](auto &oLab) { return isWriteRfBefore(&oLab, lab); });
	/* Convert to forward iterator, but be _really_ careful */
	return (rit == std::ranges::end(g.rco(addr))) ? std::ranges::begin(g.co(addr))
						      : ++ExecutionGraph::co_iterator(*rit);
}

static auto splitLocMOAfterHb(ReadLabel *rLab) -> ExecutionGraph::co_iterator
{
	auto &g = *rLab->getParent();
	if (std::ranges::any_of(g.getInitLabel()->rfs(rLab->getAddr()), [rLab](auto &rfLab) {
		    return rfLab.view(1).contains(rLab->getPos());
	    }))
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
	auto endIt = (isDepTracking()) ? splitLocMOAfterHb(rLab)
				       : std::ranges::end(g.co(rLab->getAddr()));
	std::transform(begIt, endIt, std::back_inserter(stores), [&](auto &lab) { return &lab; });
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

[[maybe_unused]] static auto getRevisitableFrom(WriteLabel *sLab, const VectorClock &pporf,
						WriteLabel *coPred) -> std::vector<ReadLabel *>
{
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

void RAChecker::filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls)
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
						 return v->contains(evLab->getPos()) && // stays in
											// graph?
							g.po_imm_pred(evLab)->view(1).contains(
								eLab->getPos()); // po-pred to check
										 // evLab != rLab
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
		auto *rLab = genmc::dyn_cast<ReadLabel>(g.po_imm_pred(wLab));
		VERIFY(rLab);
		auto *rfLab = rLab->getRf();
		VERIFY(rfLab);
		result.push_back(rfLab);
		return result;
	}

	/* Otherwise, we calculate the full range and add the store */
	auto rangeBegin = splitLocMOBefore(wLab->getAddr(), wLab);
	auto rangeEnd = (isDepTracking()) ? splitLocMOAfter(wLab)
					  : std::ranges::end(g.co(wLab->getAddr()));
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
bool RAChecker::visitCoherenceIterative(std::vector<DFSWorklistEntry> &worklist,
					const EventLabel *initLab) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			if (lab == initLab)
				return false;
			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_10.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(10, pLab);
						}
					}

			break;
		}
		case 2: {
			if (isFinishing) {
				visitedCoherence_2.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_2.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(2, lab, true);
			visitedCoherence_2.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(0, pLab);
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				worklist.emplace_back(0, pLab);
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				worklist.emplace_back(0, pLab);
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status = visitedCoherence_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status = visitedCoherence_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_4.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(4, pLab);
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedCoherence_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadStartLabel>(pLab)) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCoherence_3.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCoherence_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab)) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedCoherence_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab)) {
					auto status = visitedCoherence_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 4: {
			if (isFinishing) {
				visitedCoherence_4.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_4.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(4, lab, true);
			visitedCoherence_4.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCoherence_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(5, pLab);
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status = visitedCoherence_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}

			break;
		}
		case 6: {
			if (isFinishing) {
				visitedCoherence_6.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCoherence_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_4.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(4, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}

			break;
		}
		case 7: {
			if (isFinishing) {
				visitedCoherence_7.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_7.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(7, lab, true);
			visitedCoherence_7.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status = visitedCoherence_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCoherence_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_4.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(4, pLab);
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedCoherence_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedCoherence_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status = visitedCoherence_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCoherence_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				}
			}

			break;
		}
		case 8: {
			if (isFinishing) {
				visitedCoherence_8.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_8.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(8, lab, true);
			visitedCoherence_8.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status = visitedCoherence_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status = visitedCoherence_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status = visitedCoherence_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status = visitedCoherence_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_10.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(10, pLab);
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedCoherence_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedCoherence_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadStartLabel>(pLab)) {
					auto status = visitedCoherence_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					}
				}

			break;
		}
		case 9: {
			if (isFinishing) {
				visitedCoherence_9.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_9.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(9, lab, true);
			visitedCoherence_9.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab)) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedCoherence_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadFinishLabel>(pLab)) {
					auto status = visitedCoherence_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_9.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(9, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					}
				}

			break;
		}
		case 10: {
			if (isFinishing) {
				visitedCoherence_10.setStatus(lab->getStamp().get(),
							      NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_10.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(10, lab, true);
			visitedCoherence_10.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCoherence_9.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(9, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(11, pLab);
				}

			break;
		}
		case 11: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status = visitedCoherence_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					}
				}

			break;
		}
		case 12: {
			if (isFinishing) {
				visitedCoherence_12.setStatus(lab->getStamp().get(),
							      NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_12.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(12, lab, true);
			visitedCoherence_12.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_10.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(10, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_12.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(12, pLab);
				}
			}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitCoherenceRelinche(const ExecutionGraph &g) const
{
	for (auto &lab : g.labels()) {
		if (!genmc::isa<MethodBeginLabel>(&lab))
			continue;

		visitedCoherence_2.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_3.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_4.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_6.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_7.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_8.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_9.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_10.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_12.maybeClearResize(g.getMaxStamp().get() + 1);

		/* Explore from this accepting state using DFS */
		std::vector<DFSWorklistEntry> startState = {{1, &lab}};
		if (!visitCoherenceIterative(startState, &lab /* initLab */))
			return false;
	}
	return true;
}

bool RAChecker::visitError1([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError1Iterative(std::vector<DFSWorklistEntry> &worklist,
					      const View &v) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			if (!v.contains(lab->getPos())) {
				cexLab = lab;
				return false;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.alloc(lab); pLab) {
				worklist.emplace_back(0, pLab);
			}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessError1(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError1Accepting.clear();
	visitedLHSUnlessError1Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(1);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError1Iterative(startStatesLHS, v);
}

bool RAChecker::checkError1(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError1(lab))
		return true;

	return visitError1(lab);
}

bool RAChecker::visitError2([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError2Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			return false;
			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				if (auto pLab = g.free(lab); pLab) {
					worklist.emplace_back(0, pLab);
				}
			if (true && genmc::isa<HpRetireLabel>(lab))
				if (auto pLab = g.free(lab); pLab) {
					worklist.emplace_back(0, pLab);
				}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitRHSUnlessError2Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessError2(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError2Accepting.clear();
	visitedLHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError2Accepting.clear();
	visitedRHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	if (!visitLHSUnlessError2Iterative(startStatesLHS))
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
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError2(lab))
		return true;

	return visitError2(lab);
}

bool RAChecker::visitError3([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError3Iterative(std::vector<DFSWorklistEntry> &worklist,
					      const View &v) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			if (!v.contains(lab->getPos())) {
				cexLab = lab;
				return false;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				for (auto &tmp : g.pomax_at_reads(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				for (auto &tmp : g.pomax_at_writes(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				for (auto &tmp : g.pomax_na_reads(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				for (auto &tmp : g.pomax_na_writes(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessError3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError3Accepting.clear();
	visitedLHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(1);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError3Iterative(startStatesLHS, v);
}

bool RAChecker::checkError3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError3(lab))
		return true;

	return visitError3(lab);
}

bool RAChecker::visitError4([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError4Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			return false;
			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.free(lab); pLab) {
					worklist.emplace_back(0, pLab);
				}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.free(lab); pLab) {
					worklist.emplace_back(0, pLab);
				}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitRHSUnlessError4Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessError4(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError4Accepting.clear();
	visitedLHSUnlessError4Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError4Accepting.clear();
	visitedRHSUnlessError4Accepting.resize(g.getMaxStamp().get() + 1, false);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	if (!visitLHSUnlessError4Iterative(startStatesLHS))
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
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}

bool RAChecker::visitError5([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError5Iterative(std::vector<DFSWorklistEntry> &worklist,
					      const View &v) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			if (!v.contains(lab->getPos())) {
				cexLab = lab;
				return false;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && genmc::isa<HpRetireLabel>(lab))
				for (auto &tmp : g.unprotected(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessError5(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError5Accepting.clear();
	visitedLHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(1);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError5Iterative(startStatesLHS, v);
}

bool RAChecker::checkError5(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError5(lab))
		return true;

	return visitError5(lab);
}

bool RAChecker::visitError6([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError6Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			return false;
			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && genmc::isa<MemAccessLabel>(lab) &&
			    genmc::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() &&
			    !isHazptrProtected(genmc::dyn_cast<MemAccessLabel>(lab)))
				if (auto pLab = g.retire(lab); pLab) {
					worklist.emplace_back(0, pLab);
				}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitRHSUnlessError6Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessError6(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError6Accepting.clear();
	visitedLHSUnlessError6Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError6Accepting.clear();
	visitedRHSUnlessError6Accepting.resize(g.getMaxStamp().get() + 1, false);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	if (!visitLHSUnlessError6Iterative(startStatesLHS))
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
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError6(lab))
		return true;

	return visitError6(lab);
}

bool RAChecker::visitError7([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessError7Iterative(std::vector<DFSWorklistEntry> &worklist,
					      const View &v) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			if (!v.contains(lab->getPos())) {
				cexLab = lab;
				return false;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && genmc::isa<ReadLabel>(lab))
				for (auto &tmp : g.pomax_na_writes(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.pomax_na_reads(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.pomax_na_writes(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && lab->isNotAtomic() && genmc::isa<ReadLabel>(lab))
				for (auto &tmp : g.pomax_at_writes(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.pomax_at_reads(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.pomax_at_writes(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessError7(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError7Accepting.clear();
	visitedLHSUnlessError7Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(1);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError7Iterative(startStatesLHS, v);
}

bool RAChecker::checkError7(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError7(lab))
		return true;

	return visitError7(lab);
}

bool RAChecker::visitWarning8([[maybe_unused]] const EventLabel *lab) const { return false; }

bool RAChecker::visitLHSUnlessWarning8Iterative(std::vector<DFSWorklistEntry> &worklist,
						const View &v) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				break;
			}

			if (!v.contains(lab->getPos())) {
				cexLab = lab;
				return false;
			}

			[[maybe_unused]] auto &g = *lab->getParent();

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && !(lab->isNotAtomic()) && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.pomax_at_writes(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(0, pLab);
					}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool RAChecker::visitUnlessWarning8(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessWarning8Accepting.clear();
	visitedLHSUnlessWarning8Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(0);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessWarning8Iterative(startStatesLHS, v);
}

bool RAChecker::checkWarning8(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessWarning8(lab))
		return true;

	return visitWarning8(lab);
}

std::optional<VerificationError>
RAChecker::checkErrors([[maybe_unused]] const EventLabel *lab,
		       [[maybe_unused]] const EventLabel *&race) const
{
	if (!checkError1(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessNonMalloc};
	}

	if (!checkError2(lab)) {
		race = cexLab;
		return {VerificationError::VE_DoubleFree};
	}

	if (!checkError3(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessFreed};
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
		return {VerificationError::VE_RaceNotAtomic};
	}

	return {};
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

bool RAChecker::isConsistent([[maybe_unused]] const EventLabel *lab) const { return true; }

bool RAChecker::isConsistent([[maybe_unused]] const ExecutionGraph &g) const { return true; }

bool RAChecker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true && visitCoherenceRelinche(g);
}

View RAChecker::calcPPoRfBefore(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
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

std::unique_ptr<VectorClock> RAChecker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}

void RAChecker::recomputeCacheCounters([[maybe_unused]] const ExecutionGraph &g) const {}

void RAChecker::resetCacheCounters() const {}

void RAChecker::maybeDecreaseCacheCounters([[maybe_unused]] const EventLabel *lab) const {}

void RAChecker::maybeIncreaseCacheCounters([[maybe_unused]] const EventLabel *lab) const {}

// NOLINTEND
