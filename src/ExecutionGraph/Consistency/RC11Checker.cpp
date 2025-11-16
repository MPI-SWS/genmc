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
#include "ADT/VSet.hpp"
#include "ADT/View.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/GraphUtils.hpp"
#include "Verification/Config.hpp"
#include "Verification/VerificationError.hpp"

bool RC11Checker::isDepTracking() const { return 0; }

bool RC11Checker::visitCalc64_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool RC11Checker::visitCalc64_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitCalc64_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc64_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc64_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc64_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		if (!visitCalc64_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc64_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc64_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

View RC11Checker::visitCalc64(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());

	visitCalc64_2(lab, calcRes);
	return calcRes;
}
auto RC11Checker::checkCalc64(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc64(lab);
}
bool RC11Checker::visitCalc68_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool RC11Checker::visitCalc68_1(const EventLabel *lab, View &calcRes) const
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

bool RC11Checker::visitCalc68_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease())
				if (calcRes.updateIdx(pLab->getPos()); true) {
					if (!visitCalc68_0(pLab, calcRes)) {
						return false;
					}
				}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc68_0(pLab, calcRes)) {
				return false;
			}
		}
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && genmc::isa<WriteLabel>(pLab) &&
			    ((genmc::isa<ReadLabel>(pLab) &&
			      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(pLab) &&
			      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto status = visitedCalc68_5[pLab->getStamp().get()];
				if (status == NodeStatus::unseen) {
					if (!visitCalc68_5(pLab, calcRes)) {
						return false;
					}

				} else if (status == NodeStatus::entered) {

				} else if (status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto status = visitedCalc68_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc68_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_6(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc68_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_6(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease()) {
				if (!visitCalc68_1(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc68_1(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

bool RC11Checker::visitCalc68_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc68_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc68_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc68_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_3(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc68_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc68_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && genmc::isa<WriteLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc68_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc68_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_3(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitCalc68_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc68_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc68_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCalc68_4(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc68_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc68_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc68_6[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc68_6[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc68_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && genmc::isa<WriteLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			auto status = visitedCalc68_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc68_5(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc68_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_3(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc68_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc68_6(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc68_1(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc68_6[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

View RC11Checker::visitCalc68(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());

	visitedCalc68_3.clear();
	visitedCalc68_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc68_5.clear();
	visitedCalc68_5.resize(g.getMaxStamp().get() + 1);
	visitedCalc68_6.clear();
	visitedCalc68_6.resize(g.getMaxStamp().get() + 1);

	visitCalc68_2(lab, calcRes);
	return calcRes;
}
auto RC11Checker::checkCalc68(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc68(lab);
}
bool RC11Checker::visitCalc69_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool RC11Checker::visitCalc69_1(const EventLabel *lab, View &calcRes) const
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

bool RC11Checker::visitCalc69_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab) &&
	    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc69_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_6(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab) &&
	    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc69_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_6(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadStartLabel>(lab) &&
	    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = po_imm_pred(g, lab); pLab) {
			auto status = visitedCalc69_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_6(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
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
	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease())
				if (calcRes.updateIdx(pLab->getPos()); true) {
					if (!visitCalc69_0(pLab, calcRes)) {
						return false;
					}
				}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc69_0(pLab, calcRes)) {
				return false;
			}
		}
	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = rf_pred(g, lab); pLab) {
			auto status = visitedCalc69_3[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCalc69_3(pLab, calcRes)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && genmc::isa<WriteLabel>(pLab) &&
			    ((genmc::isa<ReadLabel>(pLab) &&
			      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
			     (genmc::isa<WriteLabel>(pLab) &&
			      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
				auto status = visitedCalc69_5[pLab->getStamp().get()];
				if (status == NodeStatus::unseen) {
					if (!visitCalc69_5(pLab, calcRes)) {
						return false;
					}

				} else if (status == NodeStatus::entered) {

				} else if (status == NodeStatus::left) {
				}
			}
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
	if (true && lab->isAtLeastAcquire() && !(genmc::isa<AbstractLockCasReadLabel>(lab)))
		if (auto pLab = rf_pred(g, lab); pLab)
			if (true && pLab->isAtLeastRelease()) {
				if (!visitCalc69_1(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc69_1(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

bool RC11Checker::visitCalc69_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc69_3[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc69_3[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab))
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc69_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_3(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc69_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc69_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease())
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc69_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc69_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_3(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && genmc::isa<WriteLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
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
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitCalc69_5(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc69_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc69_5[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCalc69_4(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc69_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCalc69_6(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (visitedCalc69_6[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCalc69_6[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCalc69_6[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_6(pLab, calcRes)) {
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
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCalc69_3[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCalc69_3(pLab, calcRes)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && genmc::isa<WriteLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
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
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCalc69_1(pLab, calcRes)) {
				return false;
			}
		}

	visitedCalc69_6[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

View RC11Checker::visitCalc69(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());

	visitedCalc69_3.clear();
	visitedCalc69_3.resize(g.getMaxStamp().get() + 1);
	visitedCalc69_5.clear();
	visitedCalc69_5.resize(g.getMaxStamp().get() + 1);
	visitedCalc69_6.clear();
	visitedCalc69_6.resize(g.getMaxStamp().get() + 1);

	visitCalc69_2(lab, calcRes);
	return calcRes;
}
auto RC11Checker::checkCalc69(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc69(lab);
}
void RC11Checker::calculateSaved(EventLabel *lab) {}

void RC11Checker::calculateViews(EventLabel *lab)
{
	lab->setViews({});

	lab->addView(checkCalc64(lab));

	lab->addView(checkCalc68(lab));
	if (!getConf()->collectLinSpec && !getConf()->checkLinSpec)
		lab->addView({});
	else
		lab->addView(checkCalc69(lab));
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
	auto rit = std::find_if(g.co_rbegin(addr), g.co_rend(addr),
				[&](auto &oLab) { return isWriteRfBefore(&oLab, lab); });
	/* Convert to forward iterator, but be _really_ careful */
	return (rit == g.co_rend(addr)) ? g.co_begin(addr) : ++ExecutionGraph::co_iterator(*rit);
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
		std::ranges::transform(wLab.readers(), std::back_inserter(rfAfter),
				       [&](auto &rLab) { return &rLab; });
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
		std::ranges::transform(wLab.readers(), std::back_inserter(rfAfter),
				       [&](auto &rLab) { return &rLab; });
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
						 return v->contains(evLab->getPos()) && // stays in
											// graph?
							g.po_imm_pred(evLab)->view(1).contains(
								eLab->getPos()); // po-pred to check
										 // evLab != rLab
					 });
			 }),
		 ls.end());
}

auto RC11Checker::getCoherentPlacings(WriteLabel *wLab) -> std::vector<EventLabel *>
{
	auto &g = *wLab->getParent();
	std::vector<EventLabel *> result;

	/* If it is an RMW store, there is only one possible position in MO */
	if (wLab->isRMW()) {
		auto *rLab = genmc::dyn_cast<ReadLabel>(g.po_imm_pred(wLab));
		BUG_ON(!rLab);
		auto *rfLab = rLab->getRf();
		BUG_ON(!rfLab);
		result.push_back(rfLab);
		return result;
	}

	/* Otherwise, we calculate the full range and add the store */
	auto rangeBegin = splitLocMOBefore(wLab->getAddr(), wLab);
	auto rangeEnd = (isDepTracking()) ? splitLocMOAfter(wLab) : g.co_end(wLab->getAddr());
	auto cos = std::ranges::subrange(rangeBegin, rangeEnd) |
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
bool RC11Checker::visitCoherence_0(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (lab == initLab)
		return false;

	return true;
}

bool RC11Checker::visitCoherence_1(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
				auto status = visitedCoherence_10[pLab->getStamp().get()];
				if (status == NodeStatus::unseen) {
					if (!visitCoherence_10(pLab, initLab)) {
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

bool RC11Checker::visitCoherence_2(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && genmc::isa<FenceLabel>(pLab)) {
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
		if (true && pLab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(pLab)) {
			auto status = visitedCoherence_6[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_6(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
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
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCoherence_0(pLab, initLab)) {
			return false;
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

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    genmc::isa<FenceLabel>(pLab)) {
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
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    genmc::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_4(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
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
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
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
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
			if (!visitCoherence_0(pLab, initLab)) {
				return false;
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

	visitedCoherence_3[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_4(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
			auto status = visitedCoherence_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_4(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
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
		if (true && genmc::isa<WriteLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
			if (!visitCoherence_5(pLab, initLab)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease()) {
			if (!visitCoherence_0(pLab, initLab)) {
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

	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_5(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
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

bool RC11Checker::visitCoherence_7(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_7[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_7[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab)
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
	if (auto pLab = co_imm_pred(g, lab); pLab)
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
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true)
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
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : fr_imm_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_7(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
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

	visitedCoherence_7[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_8(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_8[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_8[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_7(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_7[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_7(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && genmc::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_12[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_12(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(pLab)) {
			auto status = visitedCoherence_12[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_12(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isAtLeastAcquire()) {
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
		if (true && pLab->isAtLeastAcquire()) {
			auto status = visitedCoherence_10[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_10(pLab, initLab)) {
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

	visitedCoherence_8[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherence_9(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_9[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_9[lab->getStamp().get()] = NodeStatus::entered;

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
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_7(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
			auto status = visitedCoherence_7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_7(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    genmc::isa<FenceLabel>(pLab)) {
			auto status = visitedCoherence_12[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_12(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
		    genmc::isa<FenceLabel>(pLab)) {
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
		if (true && pLab->isAtLeastRelease() && genmc::isa<FenceLabel>(pLab)) {
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
		if (true && pLab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(pLab)) {
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

bool RC11Checker::visitCoherence_10(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_10[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_10[lab->getStamp().get()] = NodeStatus::entered;

	if (auto pLab = rf_pred(g, lab); pLab) {
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
			auto status = visitedCoherence_7[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_7(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
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
			auto status = visitedCoherence_8[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_8(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (true && genmc::isa<WriteLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
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

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && genmc::isa<ReadLabel>(pLab) &&
		    ((genmc::isa<ReadLabel>(pLab) && genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
		     (genmc::isa<WriteLabel>(pLab) &&
		      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
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

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		auto status = visitedCoherence_12[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_12(pLab, initLab)) {
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

	visitedCoherence_12[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitCoherenceRelinche(const ExecutionGraph &g) const
{
	for (auto &lab : g.labels()) {
		if (!genmc::isa<MethodBeginLabel>(&lab))
			continue;
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

	if (auto pLab = tc_succ(g, lab); pLab) {
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
	if (auto pLab = tj_succ(g, lab); pLab) {
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
	for (auto &tmp : lin_succs(g, lab))
		if (auto *pLab = &tmp; true) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_3(pLab)) {
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

bool RC11Checker::visitConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_1[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = tc_succ(g, lab); pLab) {
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
	if (auto pLab = tj_succ(g, lab); pLab) {
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
	for (auto &tmp : lin_succs(g, lab))
		if (auto *pLab = &tmp; true) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_3(pLab)) {
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

bool RC11Checker::visitConsAcyclic1_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_2[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	for (auto &tmp : rf_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
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

bool RC11Checker::visitConsAcyclic1_3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_3[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (true && genmc::isa<ReadLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			if (!visitConsAcyclic1_4(pLab)) {
				return false;
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_5(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (true && lab->isAtLeastAcquire())
		for (auto &tmp : lin_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() &&
	    genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_3(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	visitedConsAcyclic1_3[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_3(pLab)) {
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

bool RC11Checker::visitConsAcyclic1_5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_5[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_5[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_5(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() &&
	    genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	visitedConsAcyclic1_5[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_6[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = tc_succ(g, lab); pLab) {
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
	if (auto pLab = tj_succ(g, lab); pLab) {
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
	for (auto &tmp : lin_succs(g, lab))
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
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
	if (auto pLab = co_imm_succ(g, lab); pLab) {
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
	if (auto pLab = fr_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	for (auto &tmp : rf_succs(g, lab))
		if (auto *pLab = &tmp; true) {
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
	if (auto pLab = co_imm_succ(g, lab); pLab) {
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
	if (auto pLab = fr_imm_succ(g, lab); pLab) {
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
	for (auto &tmp : rf_succs(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = co_imm_succ(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = fr_imm_succ(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
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

bool RC11Checker::visitConsAcyclic1_7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_7[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	for (auto &tmp : rf_succs(g, lab))
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	visitedConsAcyclic1_7[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_8[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (true && lab->isAtLeastAcquire())
		for (auto &tmp : lin_succs(g, lab))
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && genmc::isa<ReadLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			if (!visitConsAcyclic1_9(pLab)) {
				return false;
			}
		}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = co_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = fr_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() &&
	    genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = co_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = fr_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isSC()) {
					auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen) {
						if (!visitConsAcyclic1_19(pLab)) {
							return false;
						}

					} else if (node.status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting > node.count ||
						    1)) {

						return false;
					} else if (node.status == NodeStatus::left) {
					}
				}
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = co_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = fr_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
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

bool RC11Checker::visitConsAcyclic1_9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		for (auto &tmp : rf_succs(g, lab))
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

	return true;
}

bool RC11Checker::visitConsAcyclic1_10(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_10[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() &&
	    genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	visitedConsAcyclic1_10[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_11(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_11[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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
	if (auto pLab = tc_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_11[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_12(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_12[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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
	if (auto pLab = tc_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_12[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_13(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_13[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (auto pLab = co_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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
	if (auto pLab = tc_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_succ(g, lab); pLab)
		if (true && pLab->isSC()) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
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

bool RC11Checker::visitConsAcyclic1_14(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_14[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	for (auto &tmp : rf_succs(g, lab))
		if (auto *pLab = &tmp; true) {
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
	visitedConsAcyclic1_14[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_15(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_15[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() &&
	    genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && genmc::isa<ReadLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			if (!visitConsAcyclic1_16(pLab)) {
				return false;
			}
		}
	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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
	if (true && lab->isAtLeastAcquire())
		for (auto &tmp : lin_succs(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_11(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isSC() &&
		    genmc::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire())
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_17(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic1_15[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_16(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<WriteLabel>(lab) &&
	    ((genmc::isa<ReadLabel>(lab) && genmc::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (genmc::isa<WriteLabel>(lab) && genmc::dyn_cast<WriteLabel>(lab)->isRMW())))
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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

	return true;
}

bool RC11Checker::visitConsAcyclic1_17(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_17[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isAtLeastAcquire() && lab->isAtLeastRelease() &&
	    genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isAtLeastAcquire() && pLab->isSC() &&
		    genmc::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_17[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_17(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	visitedConsAcyclic1_17[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_18(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_18[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isAtLeastRelease() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && genmc::isa<ThreadCreateLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
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
	if (auto pLab = tc_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_11(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 0)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : rf_succs(g, lab))
		if (auto *pLab = &tmp; true) {
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
	if (auto pLab = co_imm_succ(g, lab); pLab) {
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
	if (auto pLab = fr_imm_succ(g, lab); pLab) {
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
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_19(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_18[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

	return true;
}

bool RC11Checker::visitConsAcyclic1_19(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	++visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_19[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::entered};

	if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && lab->isSC() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
		if (auto pLab = co_imm_succ(g, lab); pLab) {
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
	if (true && lab->isSC())
		if (auto pLab = fr_imm_succ(g, lab); pLab) {
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
	if (true && lab->isAtLeastRelease() && lab->isSC() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true) {
				auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_11(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 0)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_11[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_11(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 0)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC())
		for (auto &tmp : rf_succs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isSC()) {
					auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
					if (node.status == NodeStatus::unseen) {
						if (!visitConsAcyclic1_19(pLab)) {
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
		if (auto pLab = co_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = fr_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC()) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_19[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_19(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && lab->isSC())
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			if (!visitConsAcyclic1_0(pLab)) {
				return false;
			}
		}
	--visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_19[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							 NodeStatus::left};

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
	return true && (visitConsAcyclic1_0(lab)) &&
	       (visitedConsAcyclic1_1[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_1(lab)) &&
	       (visitedConsAcyclic1_3[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_3(lab)) &&
	       (visitedConsAcyclic1_6[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_6(lab)) &&
	       (visitedConsAcyclic1_8[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_8(lab)) &&
	       (visitedConsAcyclic1_11[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_11(lab)) &&
	       (visitedConsAcyclic1_12[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_12(lab)) &&
	       (visitedConsAcyclic1_13[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_13(lab)) &&
	       (visitedConsAcyclic1_15[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_15(lab)) &&
	       (visitedConsAcyclic1_18[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_18(lab)) &&
	       (visitedConsAcyclic1_19[lab->getStamp().get()].status != NodeStatus::unseen ||
		visitConsAcyclic1_19(lab));
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
	return true && std::ranges::all_of(g.labels(), [&](auto &lab) {
		       return visitedConsAcyclic1_19[lab.getStamp().get()].status !=
				      NodeStatus::unseen ||
			      visitConsAcyclic1_19(&lab);
	       });
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

	for (auto &tmp : other_labels(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && pLab->isSC()) {
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
		if (visitedLHSUnlessConsAcyclic1Accepting[i] &&
		    !visitedRHSUnlessConsAcyclic1Accepting[i]) {
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
bool RC11Checker::visitError2(const EventLabel *lab) const { return false; }

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

	if (auto pLab = alloc_pred(g, lab); pLab) {
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

	return true && visitLHSUnlessError2_1(lab, v);
}

bool RC11Checker::checkError2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError2(lab))
		return true;

	return visitError2(lab);
}
bool RC11Checker::visitError3(const EventLabel *lab) const { return false; }

bool RC11Checker::visitLHSUnlessError3_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool RC11Checker::visitLHSUnlessError3_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<FreeLabel>(pLab) &&
				    !genmc::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError3_0(pLab)) {
						return false;
					}
				}
	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError3_0(pLab)) {
						return false;
					}
				}
	if (true && genmc::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<FreeLabel>(pLab) &&
				    !genmc::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError3_0(pLab)) {
						return false;
					}
				}
	if (true && genmc::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<HpRetireLabel>(pLab)) {
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
			cexLab = &*std::find_if(g.label_begin(), g.label_end(),
						[&](auto &lab) { return lab.getStamp() == i; });
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
bool RC11Checker::visitError4(const EventLabel *lab) const { return false; }

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

	for (auto &tmp : alloc_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitLHSUnlessError4_0(pLab, v)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitLHSUnlessError4_2(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError4_1(pLab, v)) {
				return false;
			}
		}
	if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
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

	return true && visitLHSUnlessError4_2(lab, v);
}

bool RC11Checker::checkError4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}
bool RC11Checker::visitError5(const EventLabel *lab) const { return false; }

bool RC11Checker::visitLHSUnlessError5_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool RC11Checker::visitLHSUnlessError5_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = free_succ(g, lab); pLab)
		if (true && genmc::isa<FreeLabel>(pLab) && !genmc::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError5_0(pLab)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitLHSUnlessError5_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = alloc_pred(g, lab); pLab) {
		if (!visitLHSUnlessError5_1(pLab)) {
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

	if (!visitLHSUnlessError5_2(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError5Accepting.size(); i++) {
		if (visitedLHSUnlessError5Accepting[i] && !visitedRHSUnlessError5Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(),
						[&](auto &lab) { return lab.getStamp() == i; });
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
bool RC11Checker::visitError6(const EventLabel *lab) const { return false; }

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

	for (auto &tmp : alloc_succs(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && genmc::isa<MemAccessLabel>(pLab) &&
			    genmc::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() &&
			    !isHazptrProtected(genmc::dyn_cast<MemAccessLabel>(pLab))) {
				if (!visitLHSUnlessError6_0(pLab, v)) {
					return false;
				}
			}

	return true;
}

bool RC11Checker::visitLHSUnlessError6_2(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab)
			if (true && genmc::isa<MemAccessLabel>(pLab) &&
			    genmc::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() &&
			    !isHazptrProtected(genmc::dyn_cast<MemAccessLabel>(pLab))) {
				if (!visitLHSUnlessError6_0(pLab, v)) {
					return false;
				}
			}
	if (true && genmc::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError6_1(pLab, v)) {
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

	return true && visitLHSUnlessError6_2(lab, v);
}

bool RC11Checker::checkError6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError6(lab))
		return true;

	return visitError6(lab);
}
bool RC11Checker::visitError7(const EventLabel *lab) const { return false; }

bool RC11Checker::visitLHSUnlessError7_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool RC11Checker::visitLHSUnlessError7_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = free_succ(g, lab); pLab)
		if (true && genmc::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError7_0(pLab)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitLHSUnlessError7_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<MemAccessLabel>(lab) &&
	    genmc::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() &&
	    !isHazptrProtected(genmc::dyn_cast<MemAccessLabel>(lab)))
		if (auto pLab = alloc_pred(g, lab); pLab) {
			if (!visitLHSUnlessError7_1(pLab)) {
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

	if (!visitLHSUnlessError7_2(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError7Accepting.size(); i++) {
		if (visitedLHSUnlessError7Accepting[i] && !visitedRHSUnlessError7Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(),
						[&](auto &lab) { return lab.getStamp() == i; });
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
bool RC11Checker::visitError8(const EventLabel *lab) const { return false; }

bool RC11Checker::visitLHSUnlessError8_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError8Accepting[lab->getStamp().get()] = true;

	return true;
}

bool RC11Checker::visitLHSUnlessError8_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = free_succ(g, lab); pLab)
		if (true && genmc::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError8_0(pLab)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitLHSUnlessError8_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<MemAccessLabel>(lab) &&
	    genmc::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() &&
	    isHazptrProtected(genmc::dyn_cast<MemAccessLabel>(lab)))
		if (auto pLab = alloc_pred(g, lab); pLab) {
			if (!visitLHSUnlessError8_1(pLab)) {
				return false;
			}
		}

	return true;
}

bool RC11Checker::visitRHSUnlessError8_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedRHSUnlessError8Accepting[lab->getStamp().get()] = true;

	return true;
}

bool RC11Checker::visitRHSUnlessError8_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitedRHSUnlessError8_1[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedRHSUnlessError8_1[lab->getStamp().get()] = NodeStatus::entered;

	for (auto &tmp : rf_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedRHSUnlessError8_1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitRHSUnlessError8_1(pLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	for (auto &tmp : po_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedRHSUnlessError8_1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitRHSUnlessError8_1(pLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	for (auto &tmp : rf_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitRHSUnlessError8_0(pLab)) {
				return false;
			}
		}
	for (auto &tmp : po_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitRHSUnlessError8_0(pLab)) {
				return false;
			}
		}

	visitedRHSUnlessError8_1[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool RC11Checker::visitRHSUnlessError8_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : fr_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedRHSUnlessError8_1[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitRHSUnlessError8_1(pLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	return true;
}

bool RC11Checker::visitRHSUnlessError8_3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitRHSUnlessError8_2(pLab)) {
			return false;
		}
	}

	return true;
}

bool RC11Checker::visitRHSUnlessError8_4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && genmc::isa<HpProtectLabel>(pLab)) {
			if (!visitRHSUnlessError8_3(pLab)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitRHSUnlessError8_4(pLab)) {
			return false;
		}
	}

	return true;
}

bool RC11Checker::visitUnlessError8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedRHSUnlessError8_1.clear();
	visitedRHSUnlessError8_1.resize(g.getMaxStamp().get() + 1, NodeStatus::unseen);
	visitedLHSUnlessError8Accepting.clear();
	visitedLHSUnlessError8Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError8Accepting.clear();
	visitedRHSUnlessError8Accepting.resize(g.getMaxStamp().get() + 1, false);

	if (!visitLHSUnlessError8_2(lab))
		return false;
	if (!visitRHSUnlessError8_4(lab))
		return false;
	for (auto i = 0u; i < visitedLHSUnlessError8Accepting.size(); i++) {
		if (visitedLHSUnlessError8Accepting[i] && !visitedRHSUnlessError8Accepting[i]) {
			cexLab = &*std::find_if(g.label_begin(), g.label_end(),
						[&](auto &lab) { return lab.getStamp() == i; });
			return false;
		}
	}
	return true;
}

bool RC11Checker::checkError8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError8(lab))
		return true;

	return visitError8(lab);
}
bool RC11Checker::visitError9(const EventLabel *lab) const { return false; }

bool RC11Checker::visitLHSUnlessError9_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool RC11Checker::visitLHSUnlessError9_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError9_0(pLab, v)) {
						return false;
					}
				}
	if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<ReadLabel>(pLab)) {
					if (!visitLHSUnlessError9_0(pLab, v)) {
						return false;
					}
				}
	if (true && lab->isNotAtomic() && genmc::isa<ReadLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError9_0(pLab, v)) {
						return false;
					}
				}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && genmc::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError9_0(pLab, v)) {
						return false;
					}
				}
	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && genmc::isa<ReadLabel>(pLab)) {
					if (!visitLHSUnlessError9_0(pLab, v)) {
						return false;
					}
				}
	if (true && genmc::isa<ReadLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && genmc::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError9_0(pLab, v)) {
						return false;
					}
				}

	return true;
}

bool RC11Checker::visitUnlessError9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError9Accepting.clear();
	visitedLHSUnlessError9Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(1);

	return true && visitLHSUnlessError9_1(lab, v);
}

bool RC11Checker::checkError9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError9(lab))
		return true;

	return visitError9(lab);
}
bool RC11Checker::visitWarning10(const EventLabel *lab) const { return false; }

bool RC11Checker::visitLHSUnlessWarning10_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool RC11Checker::visitLHSUnlessWarning10_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && genmc::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && genmc::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessWarning10_0(pLab, v)) {
						return false;
					}
				}

	return true;
}

bool RC11Checker::visitUnlessWarning10(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessWarning10Accepting.clear();
	visitedLHSUnlessWarning10Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true && visitLHSUnlessWarning10_1(lab, v);
}

bool RC11Checker::checkWarning10(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessWarning10(lab))
		return true;

	return visitWarning10(lab);
}
std::optional<VerificationError> RC11Checker::checkErrors(const EventLabel *lab,
							  const EventLabel *&race) const
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
		return {VerificationError::VE_AccessFreed};
	}

	if (!checkError9(lab)) {
		race = cexLab;
		return {VerificationError::VE_RaceNotAtomic};
	}

	return {};
}

std::vector<VerificationError>
RC11Checker::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings,
			   std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning10(lab)) {
		racyLabs.push_back(cexLab);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool RC11Checker::isConsistent(const EventLabel *lab) const
{

	return true && checkConsAcyclic1(lab);
}

bool RC11Checker::isConsistent(const ExecutionGraph &g) const
{

	return true && checkConsAcyclic1(g);
}

bool RC11Checker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true && visitCoherenceRelinche(g);
}

View RC11Checker::calcPPoRfBefore(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View pporf;
	pporf.updateIdx(lab->getPos());

	auto *pLab = g.po_imm_pred(lab);
	if (!pLab)
		return pporf;
	pporf.update(pLab->getPrefixView());
	if (auto *rLab = genmc::dyn_cast<ReadLabel>(pLab))
		pporf.update(rLab->getRf()->getPrefixView());
	auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(pLab);
	if (tsLab && tsLab->getCreate())
		pporf.update(tsLab->getCreate()->getPrefixView());
	if (auto *tjLab = genmc::dyn_cast<ThreadJoinLabel>(pLab))
		pporf.update(g.getLastThreadLabel(tjLab->getChildId())->getPrefixView());
	return pporf;
}
std::unique_ptr<VectorClock> RC11Checker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}
