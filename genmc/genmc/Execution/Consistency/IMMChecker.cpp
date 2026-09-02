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
#include "IMMChecker.hpp"
#include "genmc/ADT/DepView.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/GraphUtils.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/VerificationError.hpp"

#include <algorithm>

bool IMMChecker::isDepTracking() const { return 1; }

bool IMMChecker::visitCalc75Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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
						visitedCalc75_3.getStatus(pLab->getStamp().get());
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
						auto status = visitedCalc75_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc75_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc75_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCalc75_3.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc75_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCalc75_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

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
				    genmc::isa<FenceLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc75_3.getStatus(pLab->getStamp().get());
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
				auto status = visitedCalc75_3.getStatus(pLab->getStamp().get());
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
						visitedCalc75_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedCalc75_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc75_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc75_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

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
				visitedCalc75_6.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc75_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCalc75_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

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
				auto status = visitedCalc75_3.getStatus(pLab->getStamp().get());
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
						visitedCalc75_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc75_6.getStatus(pLab->getStamp().get());
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

View IMMChecker::visitCalc75(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc75_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc75_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc75_6.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{2, lab},
	};

	visitCalc75Iterative(startStates, calcRes);
	return calcRes;
}

auto IMMChecker::checkCalc75(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc75(lab);
}

bool IMMChecker::visitCalc76Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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
						visitedCalc76_3.getStatus(pLab->getStamp().get());
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
						auto status = visitedCalc76_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc76_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc76_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadStartLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
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
				    genmc::isa<FenceLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
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

View IMMChecker::visitCalc76(const EventLabel *lab) const
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

auto IMMChecker::checkCalc76(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc76(lab);
}

bool IMMChecker::visitCalc87Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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

View IMMChecker::visitCalc87(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{2, lab},
	};

	visitCalc87Iterative(startStates, calcRes);
	return calcRes;
}

auto IMMChecker::checkCalc87(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc87(lab);
}

void IMMChecker::calculateSaved([[maybe_unused]] EventLabel *lab) {}

void IMMChecker::calculateViews(EventLabel *lab)
{
	lab->setViews({});

	lab->addView(checkCalc75(lab));
	if (!getConf()->collectLinSpec && !getConf()->checkLinSpec)
		lab->addView({});
	else
		lab->addView(checkCalc76(lab));

	lab->addView(checkCalc87(lab));
}

void IMMChecker::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
}

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
		    return rfLab.view(0).contains(rLab->getPos());
	    }))
		return std::ranges::begin(g.co(rLab->getAddr()));

	auto it = std::ranges::find_if(g.co(rLab->getAddr()),
				       [&](auto &wLab) { return isHbOptRfBefore(rLab, &wLab); });
	if (it == std::ranges::end(g.co(rLab->getAddr())) || it->view(0).contains(rLab->getPos()))
		return it;
	return ++it;
}

static auto splitLocMOAfter(WriteLabel *wLab) -> ExecutionGraph::co_iterator
{
	auto &g = *wLab->getParent();
	return std::ranges::find_if(g.co(wLab->getAddr()),
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

void IMMChecker::filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls)
{
	/* If this store is po- and mo-maximal then we are done */
	auto &g = *sLab->getParent();
	if (!isDepTracking() && sLab == g.co_max(sLab->getAddr()))
		return;

	/* First, we have to exclude (mo;rf?;hb?;sb)-after reads */
	auto optRfs = getMOOptRfAfter(sLab);
	ls.erase(std::remove_if(ls.begin(), ls.end(),
				[&](auto &eLab) {
					auto &before = g.po_imm_pred(eLab)->view(0); // hb;sb
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
							g.po_imm_pred(evLab)->view(0).contains(
								eLab->getPos()); // po-pred to check
										 // evLab != rLab
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
bool IMMChecker::visitCoherenceIterative(std::vector<DFSWorklistEntry> &worklist,
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

bool IMMChecker::visitCoherenceRelinche(const ExecutionGraph &g) const
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

bool IMMChecker::visitConsAcyclic1Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				visitedConsAcyclic1_0.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_0.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_0.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(0, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_0.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 1: {
			if (isFinishing) {
				visitedConsAcyclic1_1.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_1.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_1.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(1, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_1.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_0.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_0.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_0.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_1.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(1, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_1.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_1.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(1, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_1.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_1.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(1, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_1.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_3.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_3.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_5.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_5.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic1_5.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_5.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 2: {
			if (isFinishing) {
				visitedConsAcyclic1_2.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_2.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_2.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(2, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_2.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic1_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic1_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_2.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_3.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_5.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_5.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedConsAcyclic1_3.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_3.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_3.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(3, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_3.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_2.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_3.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(4, pLab);
				}

			break;
		}
		case 4: {
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
					auto status = visitedConsAcyclic1_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_3.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedConsAcyclic1_5.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_5.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_5.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(5, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_5.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_3.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_5.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(5, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_5.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 6: {
			if (isFinishing) {
				visitedConsAcyclic1_6.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_6.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_6.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(6, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_6.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic1_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 7: {
			if (isFinishing) {
				visitedConsAcyclic1_7.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_7.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_7.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(7, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_7.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_7.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 8: {
			if (isFinishing) {
				visitedConsAcyclic1_8.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_8.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_8.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(8, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_8.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_7.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(9, pLab);
				}

			break;
		}
		case 9: {
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
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 10: {
			if (isFinishing) {
				visitedConsAcyclic1_10.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_10.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_10.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(10, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_10.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_8.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_10.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(10, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_10.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 11: {
			if (isFinishing) {
				visitedConsAcyclic1_11.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_11.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_11.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(11, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_11.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_12.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(12, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_12.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_12.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(12, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_12.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_13.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(13, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_13.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_13.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(13, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_13.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_13.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(13, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_13.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_13.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(13, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_13.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_15.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(15, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_15.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_15.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(15, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_15.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_17.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic1_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_17.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_18.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(18, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_18.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_18.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(18, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_18.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_18.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(18, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_18.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 12: {
			if (isFinishing) {
				visitedConsAcyclic1_12.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_12.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_12.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(12, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_12.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic1_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_12.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(12, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_12.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 13: {
			if (isFinishing) {
				visitedConsAcyclic1_13.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_13.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_13.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(13, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_13.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_13.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(13, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_13.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_19.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(19, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_19.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 14: {
			if (isFinishing) {
				visitedConsAcyclic1_14.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_14.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_14.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(14, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_14.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic1_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_13.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(13, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_13.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic1_13.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(13, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_13.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_14.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(14, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_14.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_15.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(15, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_15.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_17.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_19.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(19, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_19.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 15: {
			if (isFinishing) {
				visitedConsAcyclic1_15.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_15.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_15.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(15, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_15.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_13.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(13, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_13.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_14.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(14, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_14.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_15.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(15, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_15.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(16, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 16: {
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
					auto status = visitedConsAcyclic1_15.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(15, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_15.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 17: {
			if (isFinishing) {
				visitedConsAcyclic1_17.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_17.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_17.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(17, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_17.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_15.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(15, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_15.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_17.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(17, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_17.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 18: {
			if (isFinishing) {
				visitedConsAcyclic1_18.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_18.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_18.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(18, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_18.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_18.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(18, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_18.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_18.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(18, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_18.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 19: {
			if (isFinishing) {
				--visitedConsAcyclic1Accepting;
				visitedConsAcyclic1_19.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_19.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				++visitedConsAcyclic1Accepting;
				visitedConsAcyclic1_19.setIncr(lab->getStamp().get(),
							       visitedConsAcyclic1Accepting,
							       NodeStatus::entered);
				worklist.emplace_back(19, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_19.getCount(
									   lab->getStamp().get()) ||
				    1)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_3.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_5.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<ThreadJoinLabel>(pLab)) {
						auto status = visitedConsAcyclic1_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_5.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				for (auto &tmp : g.fr_imm_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic1_6.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(6, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_6.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.co_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC())
				if (auto pLab = g.rf_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC())
				for (auto &tmp : g.fr_imm_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isAtLeastAcquire()) {
							auto status =
								visitedConsAcyclic1_8.getStatus(
									pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(8, pLab);
							} else if (
								status == NodeStatus::entered &&
								(visitedConsAcyclic1Accepting >
									 visitedConsAcyclic1_8.getCount(
										 pLab->getStamp()
											 .get()) ||
								 0)) {
								return false; /* cycle detected */
							}
						}
			if (true && lab->isSC())
				if (auto pLab = g.co_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_10.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(10, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_10.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<ThreadJoinLabel>(pLab)) {
						auto status = visitedConsAcyclic1_10.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(10, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_10.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_13.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(13, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_13.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC())
				if (auto pLab = g.co_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_13.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(13, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_13.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isAtLeastAcquire() && lab->isSC() &&
			    genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_15.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(15, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_15.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_15.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(15, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_15.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isAtLeastAcquire() && lab->isSC() &&
			    genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_17.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_17.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(17, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_17.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<ThreadJoinLabel>(pLab)) {
						auto status = visitedConsAcyclic1_17.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(17, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_17.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_19.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(19, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_19.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				for (auto &tmp : g.fr_imm_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isSC()) {
							auto status =
								visitedConsAcyclic1_19.getStatus(
									pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(19, pLab);
							} else if (
								status == NodeStatus::entered &&
								(visitedConsAcyclic1Accepting >
									 visitedConsAcyclic1_19.getCount(
										 pLab->getStamp()
											 .get()) ||
								 1)) {
								return false; /* cycle detected */
							}
						}
			if (true && lab->isSC())
				if (auto pLab = g.co_imm_pred(lab); pLab)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_19.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(19, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_19.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_19.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(19, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_19.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_19.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(19, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_19.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
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

bool IMMChecker::visitConsAcyclic1(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_17.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.maybeClearResize(g.getMaxStamp().get() + 1);

	/* States we need to explore from using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{19, lab}, {18, lab}, {15, lab}, {13, lab}, {12, lab}, {11, lab},
		{8, lab},  {6, lab},  {3, lab},	 {1, lab},  {0, lab},
	};

	return visitConsAcyclic1Iterative(startStates);
}

bool IMMChecker::visitConsAcyclic1Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_15.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_17.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_19.maybeClearResize(g.getMaxStamp().get() + 1);

	auto exploreLab = [&](auto &lab) {
		/* Explore from all accepting states using DFS */
		std::vector<DFSWorklistEntry> startStates = {
			{19, &lab},
		};

		return visitConsAcyclic1Iterative(startStates);
	};

	return std::ranges::all_of(g.labels(), exploreLab);
}

void IMMChecker::recomputeUnlessConsAcyclic1(const ExecutionGraph &g) const
{
	auto isMatchingLabel = [](const auto &l) {
		auto *lab = &l;
		if (true && lab->isSC())
			return true;
		return false;
	};

	cacheCounterUnlessConsAcyclic1 =
		std::count_if(g.label_begin(), g.label_end(), isMatchingLabel);
}

bool IMMChecker::visitUnlessConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	if (cacheCounterUnlessConsAcyclic1 == -1) /* Cache is empty */
		recomputeUnlessConsAcyclic1(g);
	return cacheCounterUnlessConsAcyclic1 < 2;
}

bool IMMChecker::checkConsAcyclic1(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessConsAcyclic1(lab))
		return true;

	return visitConsAcyclic1(lab);
}

bool IMMChecker::checkConsAcyclic1(const ExecutionGraph &g) const
{
	return visitConsAcyclic1Full(g);
}

bool IMMChecker::visitConsAcyclic2Iterative(std::vector<DFSWorklistEntry> &worklist) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				visitedConsAcyclic2_0.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_0.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_0.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(0, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_0.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.ctrl_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rfi_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_0.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.ctrl_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rfi_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_1.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(1, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_1.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rfi_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(2, pLab);
				}
			for (auto &tmp : g.ctrl_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable()) {
						auto status = visitedConsAcyclic2_17.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(17, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_17.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable()) {
						auto status = visitedConsAcyclic2_17.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(17, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_17.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}

			break;
		}
		case 1: {
			if (isFinishing) {
				visitedConsAcyclic2_1.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_1.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_1.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(1, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_1.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_1.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(1, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_1.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable()) {
						auto status = visitedConsAcyclic2_17.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(17, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_17.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}

			break;
		}
		case 2: {
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
					auto status = visitedConsAcyclic2_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_0.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status = visitedConsAcyclic2_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
				    pLab->isDependable()) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedConsAcyclic2_3.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_3.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_3.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(3, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_3.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_3.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_17.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(17, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_17.getCount(
							    pLab->getStamp().get()) ||
					    1)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 4: {
			if (isFinishing) {
				visitedConsAcyclic2_4.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_4.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_4.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(4, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_4.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_4.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(4, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_4.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedConsAcyclic2_5.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_5.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_5.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(5, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_5.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_5.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(5, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_5.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.poloc_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 6: {
			if (isFinishing) {
				visitedConsAcyclic2_6.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_6.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_6.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(6, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_6.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic2_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic2_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic2_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 7: {
			if (isFinishing) {
				visitedConsAcyclic2_7.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_7.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_7.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(7, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_7.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic2_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_7.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_10.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_10.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 8: {
			if (isFinishing) {
				visitedConsAcyclic2_8.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_8.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_8.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(8, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_8.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic2_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_7.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic2_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(9, pLab);
				}

			break;
		}
		case 9: {
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
					auto status = visitedConsAcyclic2_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 10: {
			if (isFinishing) {
				visitedConsAcyclic2_10.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_10.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_10.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(10, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > visitedConsAcyclic2_10.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_8.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_10.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(10, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_10.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 11: {
			if (isFinishing) {
				visitedConsAcyclic2_11.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_11.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_11.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(11, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > visitedConsAcyclic2_11.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_6.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_6.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic2_8.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(8, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_8.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic2_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic2_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_8.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 12: {
			if (isFinishing) {
				visitedConsAcyclic2_12.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_12.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_12.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(12, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > visitedConsAcyclic2_12.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_11.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_12.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(12, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_12.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_12.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(12, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_12.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_12.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(12, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_12.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic2_14.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(14, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_14.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic2_14.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(14, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_14.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic2_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 13: {
			if (isFinishing) {
				visitedConsAcyclic2_13.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_13.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_13.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(13, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > visitedConsAcyclic2_13.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic2_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
					auto status = visitedConsAcyclic2_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_13.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(13, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_13.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_14.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(14, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_14.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 14: {
			if (isFinishing) {
				visitedConsAcyclic2_14.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_14.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_14.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(14, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > visitedConsAcyclic2_14.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic2_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic2_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_13.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(13, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_13.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic2_14.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(14, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_14.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(15, pLab);
				}

			break;
		}
		case 15: {
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
					auto status = visitedConsAcyclic2_14.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(14, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_14.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 16: {
			if (isFinishing) {
				visitedConsAcyclic2_16.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_16.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_16.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(16, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > visitedConsAcyclic2_16.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_14.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(14, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_14.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_16.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(16, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_16.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 17: {
			if (isFinishing) {
				--visitedConsAcyclic2Accepting;
				visitedConsAcyclic2_17.set(lab->getStamp().get(),
							   visitedConsAcyclic2Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_17.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				++visitedConsAcyclic2Accepting;
				visitedConsAcyclic2_17.setIncr(lab->getStamp().get(),
							       visitedConsAcyclic2Accepting,
							       NodeStatus::entered);
				worklist.emplace_back(17, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting > visitedConsAcyclic2_17.getCount(
									   lab->getStamp().get()) ||
				    1)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && genmc::isa<WriteLabel>(lab) &&
			    ((genmc::isa<ReadLabel>(lab) &&
			      genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(lab) &&
			      genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    ((genmc::isa<ReadLabel>(pLab) &&
					      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
					     (genmc::isa<WriteLabel>(pLab) &&
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
						auto status = visitedConsAcyclic2_0.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(0, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_0.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.addr_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic2_0.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(0, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_0.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic2_0.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(0, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_0.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.data_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic2_0.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(0, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_0.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab) &&
			    ((genmc::isa<ReadLabel>(lab) &&
			      genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(lab) &&
			      genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    ((genmc::isa<ReadLabel>(pLab) &&
					      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
					     (genmc::isa<WriteLabel>(pLab) &&
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
						auto status = visitedConsAcyclic2_1.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(1, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_1.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.addr_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic2_1.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(1, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_1.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic2_1.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(1, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_1.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.data_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic2_1.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(1, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_1.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_1.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(1, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_1.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_3.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isAtLeastRelease())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_3.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_4.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(4, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_4.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_5.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_5.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_11.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_12.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(12, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_12.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isAtLeastAcquire() && lab->isSC() &&
			    genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_14.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(14, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_14.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic2_14.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(14, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_14.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isAtLeastAcquire() && lab->isSC() &&
			    genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic2_16.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(16, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_16.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<ThreadJoinLabel>(pLab)) {
						auto status = visitedConsAcyclic2_16.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(16, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_16.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			for (auto &tmp : g.detour_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rfe_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_17.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(17, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_17.getCount(
							    pLab->getStamp().get()) ||
					    1)) {
					return false; /* cycle detected */
				}
			}
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (true && genmc::isa<WriteLabel>(lab) &&
			    ((genmc::isa<ReadLabel>(lab) &&
			      genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(lab) &&
			      genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    ((genmc::isa<ReadLabel>(pLab) &&
					      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
					     (genmc::isa<WriteLabel>(pLab) &&
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
					    pLab->isDependable()) {
						auto status = visitedConsAcyclic2_17.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(17, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_17.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.addr_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable()) {
							auto status =
								visitedConsAcyclic2_17.getStatus(
									pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(17, pLab);
							} else if (
								status == NodeStatus::entered &&
								(visitedConsAcyclic2Accepting >
									 visitedConsAcyclic2_17.getCount(
										 pLab->getStamp()
											 .get()) ||
								 1)) {
								return false; /* cycle detected */
							}
						}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable()) {
							auto status =
								visitedConsAcyclic2_17.getStatus(
									pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(17, pLab);
							} else if (
								status == NodeStatus::entered &&
								(visitedConsAcyclic2Accepting >
									 visitedConsAcyclic2_17.getCount(
										 pLab->getStamp()
											 .get()) ||
								 1)) {
								return false; /* cycle detected */
							}
						}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.data_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable()) {
							auto status =
								visitedConsAcyclic2_17.getStatus(
									pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(17, pLab);
							} else if (
								status == NodeStatus::entered &&
								(visitedConsAcyclic2Accepting >
									 visitedConsAcyclic2_17.getCount(
										 pLab->getStamp()
											 .get()) ||
								 1)) {
								return false; /* cycle detected */
							}
						}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab)) {
						auto status = visitedConsAcyclic2_17.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(17, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic2Accepting >
								    visitedConsAcyclic2_17.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isAtLeastRelease())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_17.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(17, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_17.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
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

bool IMMChecker::visitConsAcyclic2(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedConsAcyclic2Accepting = 0;
	visitedConsAcyclic2_0.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_1.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_8.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_11.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_12.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_13.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_14.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_16.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_17.maybeClearResize(g.getMaxStamp().get() + 1);

	/* States we need to explore from using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{14, lab}, {12, lab}, {11, lab}, {8, lab}, {6, lab},
	};

	return visitConsAcyclic2Iterative(startStates);
}

bool IMMChecker::visitConsAcyclic2Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic2Accepting = 0;
	visitedConsAcyclic2_0.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_1.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_8.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_11.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_12.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_13.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_14.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_16.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic2_17.maybeClearResize(g.getMaxStamp().get() + 1);

	auto exploreLab = [&](auto &lab) {
		/* Explore from all accepting states using DFS */
		std::vector<DFSWorklistEntry> startStates = {
			{17, &lab},
		};

		return visitConsAcyclic2Iterative(startStates);
	};

	return std::ranges::all_of(g.labels(), exploreLab);
}

bool IMMChecker::checkConsAcyclic2(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitConsAcyclic2(lab);
}

bool IMMChecker::checkConsAcyclic2(const ExecutionGraph &g) const
{
	return visitConsAcyclic2Full(g);
}

bool IMMChecker::visitWarning3([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessWarning3Iterative(std::vector<DFSWorklistEntry> &worklist,
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
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.samelocs(lab))
					if (auto *pLab = &tmp; true)
						if (true && genmc::isa<WriteLabel>(pLab)) {
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

bool IMMChecker::visitUnlessWarning3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessWarning3Accepting.clear();
	visitedLHSUnlessWarning3Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(2);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessWarning3Iterative(startStatesLHS, v);
}

bool IMMChecker::checkWarning3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessWarning3(lab))
		return true;

	return visitWarning3(lab);
}

std::optional<VerificationError>
IMMChecker::checkErrors([[maybe_unused]] const EventLabel *lab,
			[[maybe_unused]] const EventLabel *&race) const
{
	return {};
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

bool IMMChecker::isConsistent([[maybe_unused]] const EventLabel *lab) const
{

	return true && checkConsAcyclic1(lab) && checkConsAcyclic2(lab);
}

bool IMMChecker::isConsistent([[maybe_unused]] const ExecutionGraph &g) const
{

	return true && checkConsAcyclic1(g) && checkConsAcyclic2(g);
}

bool IMMChecker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true && visitCoherenceRelinche(g);
}

void IMMChecker::visitPPoRf0(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	visitedPPoRf0[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf1(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &tmp : g.ctrl_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	for (auto &tmp : g.data_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	for (auto &tmp : g.ctrl_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	for (auto &tmp : g.data_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	if (auto pLab = g.rfi_pred(lab); pLab) {
		auto status = visitedPPoRf1[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf1(pLab, pporf);
	}
	for (auto &tmp : g.ctrl_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	for (auto &tmp : g.data_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	if (auto pLab = g.rfi_pred(lab); pLab) {
		auto status = visitedPPoRf2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf2(pLab, pporf);
	}
	if (auto pLab = g.rfi_pred(lab); pLab)
		if (true && genmc::isa<WriteLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedPPoRf3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf3(pLab, pporf);
		}
	for (auto &tmp : g.ctrl_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	for (auto &tmp : g.data_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	for (auto &tmp : g.ctrl_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	for (auto &tmp : g.data_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	visitedPPoRf1[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf2(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::entered;
	for (auto &tmp : g.addr_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	for (auto &tmp : g.addr_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	for (auto &tmp : g.addr_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedPPoRf2[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf2(pLab, pporf);
	}
	for (auto &tmp : g.addr_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	for (auto &tmp : g.addr_preds(lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	visitedPPoRf2[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf3(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
		    pLab->isDependable())
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedPPoRf1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf1(pLab, pporf);
		}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
		    pLab->isDependable()) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
		    pLab->isDependable()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	visitedPPoRf3[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf4(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = g.tc_pred(lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	if (auto pLab = g.tj_pred(lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	for (auto &tmp : g.lin_preds(lab))
		if (auto *pLab = &tmp; true)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (auto pLab = g.rfe_pred(lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf0[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf0(pLab, pporf);
		}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.ctrl_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable())
					if (pporf.updateIdx(pLab->getPos()); true) {
						auto status = visitedPPoRf0[pLab->getStamp().get()];
						if (status == NodeStatus::unseen)
							visitPPoRf0(pLab, pporf);
					}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.addr_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable())
					if (pporf.updateIdx(pLab->getPos()); true) {
						auto status = visitedPPoRf0[pLab->getStamp().get()];
						if (status == NodeStatus::unseen)
							visitPPoRf0(pLab, pporf);
					}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.data_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable())
					if (pporf.updateIdx(pLab->getPos()); true) {
						auto status = visitedPPoRf0[pLab->getStamp().get()];
						if (status == NodeStatus::unseen)
							visitPPoRf0(pLab, pporf);
					}
	for (auto &tmp : g.detour_preds(lab))
		if (auto *pLab = &tmp; true)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab))
		if (auto pLab = g.poloc_imm_pred(lab); pLab)
			if (true && pLab->isAtLeastRelease() && genmc::isa<WriteLabel>(pLab))
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = g.po_imm_pred(lab); pLab)
			if (true && genmc::isa<ReadLabel>(pLab) &&
			    ((genmc::isa<ReadLabel>(pLab) &&
			      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(pLab) &&
			      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
			    pLab->isDependable())
				if (pporf.updateIdx(pLab->getPos()); true) {
					auto status = visitedPPoRf0[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf0(pLab, pporf);
				}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.ctrl_preds(lab))
			if (auto *pLab = &tmp; true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.addr_preds(lab))
			if (auto *pLab = &tmp; true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.data_preds(lab))
			if (auto *pLab = &tmp; true) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = g.po_imm_pred(lab); pLab)
			if (true && genmc::isa<ReadLabel>(pLab) &&
			    ((genmc::isa<ReadLabel>(pLab) &&
			      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(pLab) &&
			      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto status = visitedPPoRf1[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf1(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.ctrl_preds(lab))
			if (auto *pLab = &tmp; true) {
				auto status = visitedPPoRf2[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf2(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.addr_preds(lab))
			if (auto *pLab = &tmp; true) {
				auto status = visitedPPoRf2[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf2(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.data_preds(lab))
			if (auto *pLab = &tmp; true) {
				auto status = visitedPPoRf2[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf2(pLab, pporf);
			}
	if (true && genmc::isa<WriteLabel>(lab))
		if (auto pLab = g.po_imm_pred(lab); pLab) {
			auto status = visitedPPoRf2[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf2(pLab, pporf);
		}
	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = g.po_imm_pred(lab); pLab)
			if (true && genmc::isa<ReadLabel>(pLab) &&
			    ((genmc::isa<ReadLabel>(pLab) &&
			      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(pLab) &&
			      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto status = visitedPPoRf2[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf2(pLab, pporf);
			}
	if (auto pLab = g.tc_pred(lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (auto pLab = g.tj_pred(lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	for (auto &tmp : g.lin_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = g.rfe_pred(lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.ctrl_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf4[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf4(pLab, pporf);
				}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.addr_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf4[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf4(pLab, pporf);
				}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.data_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf4[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf4(pLab, pporf);
				}
	for (auto &tmp : g.detour_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (true && genmc::isa<WriteLabel>(lab))
		if (auto pLab = g.poloc_imm_pred(lab); pLab)
			if (true && pLab->isAtLeastRelease() && genmc::isa<WriteLabel>(pLab)) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = g.po_imm_pred(lab); pLab) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = g.po_imm_pred(lab); pLab)
			if (true && genmc::isa<ReadLabel>(pLab) &&
			    ((genmc::isa<ReadLabel>(pLab) &&
			      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(pLab) &&
			      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
			    pLab->isDependable()) {
				auto status = visitedPPoRf4[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf4(pLab, pporf);
			}
	if (true && genmc::isa<FenceLabel>(lab))
		if (auto pLab = g.po_imm_pred(lab); pLab) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = g.po_imm_pred(lab); pLab)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf5[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf5(pLab, pporf);
			}
	if (true && genmc::isa<FenceLabel>(lab))
		if (auto pLab = g.po_imm_pred(lab); pLab)
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf5[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf5(pLab, pporf);
			}
	if (auto pLab = g.tc_pred(lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (auto pLab = g.tj_pred(lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	for (auto &tmp : g.lin_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = g.rfe_pred(lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.ctrl_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.addr_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : g.data_preds(lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isDependable()) {
					auto status = visitedPPoRf6[pLab->getStamp().get()];
					if (status == NodeStatus::unseen)
						visitPPoRf6(pLab, pporf);
				}
	for (auto &tmp : g.detour_preds(lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (true && genmc::isa<WriteLabel>(lab))
		if (auto pLab = g.poloc_imm_pred(lab); pLab)
			if (true && pLab->isAtLeastRelease() && genmc::isa<WriteLabel>(pLab)) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	if (true && lab->isAtLeastRelease())
		if (auto pLab = g.po_imm_pred(lab); pLab) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = g.po_imm_pred(lab); pLab)
			if (true && genmc::isa<ReadLabel>(pLab) &&
			    ((genmc::isa<ReadLabel>(pLab) &&
			      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(pLab) &&
			      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())) &&
			    pLab->isDependable()) {
				auto status = visitedPPoRf6[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf6(pLab, pporf);
			}
	if (true && genmc::isa<FenceLabel>(lab))
		if (auto pLab = g.po_imm_pred(lab); pLab) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (true && genmc::isa<WriteLabel>(lab))
		if (auto pLab = g.poloc_imm_pred(lab); pLab) {
			auto status = visitedPPoRf7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf7(pLab, pporf);
		}
	visitedPPoRf4[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf5(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::entered;
	pporf.updateIdx(lab->getPos());
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedPPoRf4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf4(pLab, pporf);
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (pporf.updateIdx(pLab->getPos()); true) {
			auto status = visitedPPoRf5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf5(pLab, pporf);
		}
	visitedPPoRf5[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf6(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && pLab->isAtLeastAcquire())
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<FenceLabel>(pLab))
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = g.po_imm_pred(lab); pLab) {
		auto status = visitedPPoRf6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf6(pLab, pporf);
	}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = g.po_imm_pred(lab); pLab)
		if (true && genmc::isa<FenceLabel>(pLab)) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	visitedPPoRf6[lab->getStamp().get()] = NodeStatus::left;
}

void IMMChecker::visitPPoRf7(const EventLabel *lab, DepView &pporf) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedPPoRf7[lab->getStamp().get()] = NodeStatus::entered;
	if (auto pLab = g.poloc_imm_pred(lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<WriteLabel>(pLab))
			if (pporf.updateIdx(pLab->getPos()); true) {
				auto status = visitedPPoRf0[pLab->getStamp().get()];
				if (status == NodeStatus::unseen)
					visitPPoRf0(pLab, pporf);
			}
	if (auto pLab = g.poloc_imm_pred(lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<WriteLabel>(pLab)) {
			auto status = visitedPPoRf4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf4(pLab, pporf);
		}
	if (auto pLab = g.poloc_imm_pred(lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<WriteLabel>(pLab)) {
			auto status = visitedPPoRf6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen)
				visitPPoRf6(pLab, pporf);
		}
	if (auto pLab = g.poloc_imm_pred(lab); pLab) {
		auto status = visitedPPoRf7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen)
			visitPPoRf7(pLab, pporf);
	}
	visitedPPoRf7[lab->getStamp().get()] = NodeStatus::left;
}

DepView IMMChecker::calcPPoRfBefore(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
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
	visitPPoRf6(lab, pporf);
	return pporf;
}

std::unique_ptr<VectorClock> IMMChecker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<DepView>(calcPPoRfBefore(lab));
}

void IMMChecker::recomputeCacheCounters([[maybe_unused]] const ExecutionGraph &g) const
{
	recomputeUnlessConsAcyclic1(g);
}

void IMMChecker::resetCacheCounters() const { cacheCounterUnlessConsAcyclic1 = -1; }

void IMMChecker::maybeDecreaseCacheCounters([[maybe_unused]] const EventLabel *lab) const
{
	if (true && lab->isSC()) {
		VERIFY(cacheCounterUnlessConsAcyclic1 > 0);
		cacheCounterUnlessConsAcyclic1 -= 1;
	}
}

void IMMChecker::maybeIncreaseCacheCounters([[maybe_unused]] const EventLabel *lab) const
{
	if (true && lab->isSC()) {
		VERIFY(cacheCounterUnlessConsAcyclic1 >= 0);
		cacheCounterUnlessConsAcyclic1 += 1;
	}
}

// NOLINTEND
