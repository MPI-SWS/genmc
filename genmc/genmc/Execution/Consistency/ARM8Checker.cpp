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
#include "ARM8Checker.hpp"
#include "genmc/ADT/DepView.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Execution/GraphUtils.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/VerificationError.hpp"

#include <algorithm>

/* The po-first surviving same-location read after RLAB whose store is surviving, or null */
template <typename F>
static auto poLocNextReader(const ReadLabel *rLab, F surviving) -> const ReadLabel *
{
	auto succs = rLab->getParent()->poloc_succs(rLab);
	auto it = std::ranges::find_if(succs, [&](const EventLabel &lab) {
		const auto *oLab = genmc::dyn_cast<ReadLabel>(&lab);
		return oLab && surviving(lab) && oLab->getRf() && surviving(*oLab->getRf());
	});
	return it != std::ranges::end(succs) ? genmc::dyn_cast<ReadLabel>(&*it) : nullptr;
}

auto ARM8Checker::getMaximalRf(const ReadLabel *rLab) const -> const EventLabel *
{
	auto surviving = [](const EventLabel &lab) { return !lab.isSkeleton(); };
	const auto *nLab = poLocNextReader(rLab, surviving);
	return nLab != nullptr ? nLab->getRf() : ConsistencyChecker::getMaximalRf(rLab);
}

/* Stores are placed co-maximally */
auto ARM8Checker::getMaximalCo(const WriteLabel *wLab) const -> const EventLabel *
{
	return ConsistencyChecker::getMaximalCo(wLab);
}

auto ARM8Checker::isMaximalRf(const ReadLabel *rLab, const VectorClock &keep,
			      const EventLabel *except) const -> bool
{
	auto surviving = [&](const EventLabel &lab) {
		return survives(lab, rLab->getStamp(), keep, except);
	};
	const auto *nLab = poLocNextReader(rLab, surviving);
	return nLab != nullptr ? rLab->getRf() == nLab->getRf()
			       : ConsistencyChecker::isMaximalRf(rLab, keep, except);
}

auto ARM8Checker::isMaximalCo(const WriteLabel *wLab, const VectorClock &keep,
			      const EventLabel *except) const -> bool
{
	return ConsistencyChecker::isMaximalCo(wLab, keep, except);
}

bool ARM8Checker::isDepTracking() const { return 1; }

bool ARM8Checker::visitCalc86Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							if (!genmc::isa<InitLabel>(pLab)) {
								worklist.emplace_back(0, pLab);
							}
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
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto *iLab =
					    genmc::dyn_cast_if_present<InitLabel>(g.rf_pred(lab));
				    iLab) {
					calcRes.update(iLab->locView(
						genmc::cast<ReadLabel>(lab)->getAddr(), 0));
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab)) {
						if (!genmc::isa<InitLabel>(pLab)) {
							worklist.emplace_back(1, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab)) {
						if (!genmc::isa<InitLabel>(pLab)) {
							auto status = visitedCalc86_3.getStatus(
								pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(3, pLab);
							}
						}
					}
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab)) {
						auto status = visitedCalc86_4.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(4, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    ((genmc::isa<ReadLabel>(pLab) &&
					      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
					     (genmc::isa<WriteLabel>(pLab) &&
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
						if (!genmc::isa<InitLabel>(pLab)) {
							auto status = visitedCalc86_5.getStatus(
								pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(5, pLab);
							}
						}
					}
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc86_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCalc86_3.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc86_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCalc86_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc86_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}

			break;
		}
		case 4: {
			if (isFinishing) {
				visitedCalc86_4.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc86_4.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(4, lab, true);
			visitedCalc86_4.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						if (!genmc::isa<InitLabel>(pLab)) {
							worklist.emplace_back(0, pLab);
						}
					}
			if (auto *iLab = genmc::dyn_cast_if_present<InitLabel>(g.rf_pred(lab));
			    iLab) {
				calcRes.update(
					iLab->locView(genmc::cast<ReadLabel>(lab)->getAddr(), 0));
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					if (!genmc::isa<InitLabel>(pLab)) {
						worklist.emplace_back(1, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab)) {
					if (!genmc::isa<InitLabel>(pLab)) {
						auto status = visitedCalc86_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					if (!genmc::isa<InitLabel>(pLab)) {
						auto status = visitedCalc86_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedCalc86_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc86_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc86_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					auto status =
						visitedCalc86_4.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}

			break;
		}
		case 6: {
			if (isFinishing) {
				visitedCalc86_6.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc86_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCalc86_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab)) {
					auto status =
						visitedCalc86_4.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc86_6.getStatus(pLab->getStamp().get());
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

View ARM8Checker::visitCalc86(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc86_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc86_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc86_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc86_6.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{0, lab},
		{2, lab},
	};

	visitCalc86Iterative(startStates, calcRes);
	return calcRes;
}

auto ARM8Checker::checkCalc86(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc86(lab);
}

bool ARM8Checker::visitCalc89Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					if (!genmc::isa<InitLabel>(pLab)) {
						worklist.emplace_back(0, pLab);
					}
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
			if (auto *iLab = genmc::dyn_cast_if_present<InitLabel>(g.rf_pred(lab));
			    iLab) {
				calcRes.update(
					iLab->locView(genmc::cast<ReadLabel>(lab)->getAddr(), 1));
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				if (!genmc::isa<InitLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
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

View ARM8Checker::visitCalc89(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{0, lab},
		{2, lab},
	};

	visitCalc89Iterative(startStates, calcRes);
	return calcRes;
}

auto ARM8Checker::checkCalc89(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc89(lab);
}

bool ARM8Checker::visitCalc90Iterative(std::vector<DFSWorklistEntry> &worklist,
				       DepView &calcRes) const
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
				if (calcRes.update(pLab->getPrefixView()); true) {
					worklist.emplace_back(0, pLab);
				}

			break;
		}
		case 2: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.detour_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.rfe_pred(lab); pLab)
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
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
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
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW())))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true)
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab)))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab)))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.rfi_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			for (auto &tmp : g.detour_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.rfe_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					worklist.emplace_back(1, pLab);
				}
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						worklist.emplace_back(1, pLab);
					}
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					worklist.emplace_back(1, pLab);
				}
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						worklist.emplace_back(1, pLab);
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
						worklist.emplace_back(1, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(1, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab))) {
						worklist.emplace_back(1, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab))) {
						worklist.emplace_back(1, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab) {
					worklist.emplace_back(1, pLab);
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab)) {
						worklist.emplace_back(1, pLab);
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.rfi_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						worklist.emplace_back(1, pLab);
					}
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					worklist.emplace_back(1, pLab);
				}
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						worklist.emplace_back(1, pLab);
					}
			if (auto pLab = g.rfi_pred(lab); pLab) {
				worklist.emplace_back(3, pLab);
			}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc90_4.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					}
				}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab))) {
						auto status = visitedCalc90_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc90_9.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
					}
				}
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc90_10.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					}
				}
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc90_10.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					}
				}
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc90_10.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(10, pLab);
					}
				}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab) {
					auto status =
						visitedCalc90_11.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					}
				}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(12, pLab);
				}
			for (auto &tmp : g.data_nrmw_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(12, pLab);
				}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
						worklist.emplace_back(12, pLab);
					}

			break;
		}
		case 3: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(12, pLab);
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(12, pLab);
				}

			break;
		}
		case 4: {
			if (isFinishing) {
				visitedCalc90_4.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc90_4.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(4, lab, true);
			visitedCalc90_4.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab)))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab)))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab))) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab))) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc90_4.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(4, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab))) {
					auto status =
						visitedCalc90_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(12, pLab);
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedCalc90_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc90_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc90_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(1, pLab);
				}
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true) {
					worklist.emplace_back(12, pLab);
				}

			break;
		}
		case 6: {
			if (isFinishing) {
				visitedCalc90_6.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc90_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCalc90_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc90_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}

			break;
		}
		case 7: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ThreadJoinLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ThreadStartLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && genmc::isa<ReadLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ThreadJoinLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ThreadStartLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ReadLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab)) {
					auto status =
						visitedCalc90_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(7, pLab);
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status =
						visitedCalc90_8.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}

			break;
		}
		case 8: {
			if (isFinishing) {
				visitedCalc90_8.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc90_8.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(8, lab, true);
			visitedCalc90_8.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc90_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				}
			}

			break;
		}
		case 9: {
			if (isFinishing) {
				visitedCalc90_9.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc90_9.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(9, lab, true);
			visitedCalc90_9.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc90_9.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(9, pLab);
				}
			}

			break;
		}
		case 10: {
			if (isFinishing) {
				visitedCalc90_10.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc90_10.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(10, lab, true);
			visitedCalc90_10.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc90_10.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(10, pLab);
				}
			}

			break;
		}
		case 11: {
			if (isFinishing) {
				visitedCalc90_11.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc90_11.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(11, lab, true);
			visitedCalc90_11.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.poloc_imm_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
				auto status = visitedCalc90_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				}
			}

			break;
		}
		case 12: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rmw_succ(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.rmw_succ(lab); pLab) {
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

DepView ARM8Checker::visitCalc90(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	DepView calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc90_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc90_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc90_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc90_8.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc90_9.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc90_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc90_11.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{0, lab},
		{7, lab},
		{2, lab},
	};

	visitCalc90Iterative(startStates, calcRes);
	return calcRes;
}

auto ARM8Checker::checkCalc90(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc90(lab);
}

void ARM8Checker::calculateSaved([[maybe_unused]] EventLabel *lab) {}

void ARM8Checker::calculateViews(EventLabel *lab)
{
	lab->setViews({});
	{
		lab->addView({});
		lab->setView(std::move(checkCalc86(lab)), 0);
	}
	{
		lab->addView({});
		lab->setView(std::move(checkCalc89(lab)), 1);
	}
}

void ARM8Checker::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
	lab->setPrefixView(calculatePrefixView(lab));
}

std::unique_ptr<VectorClock> ARM8Checker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<DepView>(checkCalc90(lab));
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

auto ARM8Checker::getCoherentStores(ReadLabel *rLab) -> std::vector<EventLabel *>
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

void ARM8Checker::filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls)
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

auto ARM8Checker::getCoherentPlacings(WriteLabel *wLab) -> std::vector<EventLabel *>
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
bool ARM8Checker::visitCoherenceIterative(std::vector<DFSWorklistEntry> &worklist,
					  const EventLabel *initLab) const
{
	while (!worklist.empty()) {
		auto [stateId, lab, isFinishing] = worklist.back();
		worklist.pop_back();
		switch (stateId) {
		case 0: {
			if (isFinishing) {
				visitedCoherence_0.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_0.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(0, lab, true);
			visitedCoherence_0.setStatus(lab->getStamp().get(), NodeStatus::entered);
			if (lab == initLab)
				return false;

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status = visitedCoherence_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status = visitedCoherence_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedCoherence_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_5.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_5.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedCoherence_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedCoherence_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
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
					auto status = visitedCoherence_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedCoherence_5.setStatus(lab->getStamp().get(),
							     NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCoherence_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedCoherence_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_5.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(5, pLab);
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
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status = visitedCoherence_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCoherence_0.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(0, pLab);
				}
			}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<ReadLabel>(pLab)) {
						auto status = visitedCoherence_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status = visitedCoherence_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab) {
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
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}
			if (auto pLab = g.tc_pred(lab); pLab) {
				auto status = visitedCoherence_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}
			if (auto pLab = g.tj_pred(lab); pLab) {
				auto status = visitedCoherence_6.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(6, pLab);
				}
			}
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedCoherence_9.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedCoherence_8.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(10, pLab);
				}

			break;
		}
		case 10: {
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
					auto status = visitedCoherence_9.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
					}
				}

			break;
		}
		case 11: {
			if (isFinishing) {
				visitedCoherence_11.setStatus(lab->getStamp().get(),
							      NodeStatus::left);
				break;
			}

			auto status = visitedCoherence_11.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(11, lab, true);
			visitedCoherence_11.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedCoherence_9.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
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

bool ARM8Checker::visitCoherenceRelinche(const ExecutionGraph &g) const
{
	for (auto &lab : g.labels()) {
		if (!genmc::isa<MethodBeginLabel>(&lab))
			continue;

		visitedCoherence_0.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_2.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_3.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_5.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_6.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_7.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_8.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_9.maybeClearResize(g.getMaxStamp().get() + 1);
		visitedCoherence_11.maybeClearResize(g.getMaxStamp().get() + 1);

		/* Explore from this accepting state using DFS */
		std::vector<DFSWorklistEntry> startState = {{1, &lab}};
		if (!visitCoherenceIterative(startState, &lab /* initLab */))
			return false;
	}
	return true;
}

bool ARM8Checker::visitConsAcyclic1Iterative(std::vector<DFSWorklistEntry> &worklist) const
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
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
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
			if (auto pLab = g.poloc_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 1: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
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
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab))) {
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
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab))) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab))) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
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
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 4: {
			if (isFinishing) {
				visitedConsAcyclic1_4.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_4.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_4.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(4, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_4.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_4.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(4, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_4.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_4.getCount(
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
				if (true && genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ThreadStartLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
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
				if (true && genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
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
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
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
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    1)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}

			break;
		}
		case 9: {
			if (isFinishing) {
				visitedConsAcyclic1_9.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_9.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_9.set(lab->getStamp().get(),
							  visitedConsAcyclic1Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(9, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting >
					    visitedConsAcyclic1_9.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_9.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(9, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_9.getCount(
							    pLab->getStamp().get()) ||
					    0)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    1)) {
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
			if (auto pLab = g.rmw_succ(lab); pLab) {
				auto status =
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    1)) {
					return false; /* cycle detected */
				}
			}

			break;
		}
		case 11: {
			if (isFinishing) {
				--visitedConsAcyclic1Accepting;
				visitedConsAcyclic1_11.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_11.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				++visitedConsAcyclic1Accepting;
				visitedConsAcyclic1_11.setIncr(lab->getStamp().get(),
							       visitedConsAcyclic1Accepting,
							       NodeStatus::entered);
				worklist.emplace_back(11, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_11.getCount(
									   lab->getStamp().get()) ||
				    1)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.fr_imm_preds(lab))
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
			if (auto pLab = g.rfi_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_2.getStatus(
						pLab->getStamp().get());
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
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab))) {
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
				if (true && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_4.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(4, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_4.getCount(
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
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_7.getStatus(
						pLab->getStamp().get());
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
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_9.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_9.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			for (auto &tmp : g.data_nrmw_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
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
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.detour_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
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
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.co_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    1)) {
					return false; /* cycle detected */
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ThreadStartLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ReadLabel>(pLab)) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.rfe_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_11.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(11, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_11.getCount(
							    pLab->getStamp().get()) ||
					    1)) {
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
					    1)) {
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
					    1)) {
					return false; /* cycle detected */
				}
			}
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (true && genmc::isa<ThreadCreateLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (true && genmc::isa<ThreadFinishLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
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
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab))) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab))) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab)) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ReadLabel>(lab))
				if (auto pLab = g.rfi_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_11.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isAtLeastRelease() && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						auto status = visitedConsAcyclic1_11.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(11, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_11.getCount(
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

bool ARM8Checker::visitConsAcyclic1(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.maybeClearResize(g.getMaxStamp().get() + 1);

	/* States we need to explore from using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{11, lab},
		{0, lab},
	};

	return visitConsAcyclic1Iterative(startStates);
}

bool ARM8Checker::visitConsAcyclic1Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_8.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.maybeClearResize(g.getMaxStamp().get() + 1);

	auto exploreLab = [&](auto &lab) {
		/* Explore from all accepting states using DFS */
		std::vector<DFSWorklistEntry> startStates = {
			{11, &lab},
		};

		return visitConsAcyclic1Iterative(startStates);
	};

	return std::ranges::all_of(g.labels(), exploreLab);
}

bool ARM8Checker::checkConsAcyclic1(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitConsAcyclic1(lab);
}

bool ARM8Checker::checkConsAcyclic1(const ExecutionGraph &g) const
{
	return visitConsAcyclic1Full(g);
}

bool ARM8Checker::visitError2([[maybe_unused]] const EventLabel *lab) const { return false; }

bool ARM8Checker::visitLHSUnlessError2Iterative(std::vector<DFSWorklistEntry> &worklist,
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
			if (auto tmpE = g.alloc_pos(lab); tmpE) {
				if (!v.contains(*tmpE)) {
					cexLab = g.getEventLabelIfPresent(*tmpE);
					return false;
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

bool ARM8Checker::visitUnlessError2(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError2Accepting.clear();
	visitedLHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(0);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError2Iterative(startStatesLHS, v);
}

bool ARM8Checker::checkError2(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError2(lab))
		return true;

	return visitError2(lab);
}

bool ARM8Checker::visitError3([[maybe_unused]] const EventLabel *lab) const { return false; }

bool ARM8Checker::visitLHSUnlessError3Iterative(std::vector<DFSWorklistEntry> &worklist) const
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
				if (auto tmpE = g.free_pos(lab); tmpE) {
					return false;
				}
			if (true && genmc::isa<HpRetireLabel>(lab))
				if (auto tmpE = g.free_pos(lab); tmpE) {
					return false;
				}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool ARM8Checker::visitRHSUnlessError3Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool ARM8Checker::visitUnlessError3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError3Accepting.clear();
	visitedLHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError3Accepting.clear();
	visitedRHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	if (!visitLHSUnlessError3Iterative(startStatesLHS))
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

bool ARM8Checker::checkError3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError3(lab))
		return true;

	return visitError3(lab);
}

bool ARM8Checker::visitError4([[maybe_unused]] const EventLabel *lab) const { return false; }

bool ARM8Checker::visitLHSUnlessError4Iterative(std::vector<DFSWorklistEntry> &worklist) const
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
				if (auto tmpE = g.free_pos(lab); tmpE) {
					return false;
				}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto tmpE = g.free_pos(lab); tmpE) {
					return false;
				}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool ARM8Checker::visitRHSUnlessError4Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool ARM8Checker::visitUnlessError4(const EventLabel *lab) const
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

bool ARM8Checker::checkError4(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}

bool ARM8Checker::visitError5([[maybe_unused]] const EventLabel *lab) const { return false; }

bool ARM8Checker::visitLHSUnlessError5Iterative(std::vector<DFSWorklistEntry> &worklist) const
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
				if (auto tmpE = g.retire_pos(lab); tmpE) {
					return false;
				}

			break;
		}
		default:
			UNREACHABLE();
		}
	}
	return true;
}

bool ARM8Checker::visitRHSUnlessError5Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool ARM8Checker::visitUnlessError5(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError5Accepting.clear();
	visitedLHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError5Accepting.clear();
	visitedRHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	if (!visitLHSUnlessError5Iterative(startStatesLHS))
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

bool ARM8Checker::checkError5(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError5(lab))
		return true;

	return visitError5(lab);
}

std::optional<VerificationError>
ARM8Checker::checkErrors([[maybe_unused]] const EventLabel *lab,
			 [[maybe_unused]] const EventLabel *&race) const
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

	return {};
}

std::vector<VerificationError>
ARM8Checker::checkWarnings([[maybe_unused]] const EventLabel *lab,
			   [[maybe_unused]] const VSet<VerificationError> &seenWarnings,
			   [[maybe_unused]] std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	return result;
}

bool ARM8Checker::isConsistent([[maybe_unused]] const EventLabel *lab) const
{

	return true && checkConsAcyclic1(lab);
}

bool ARM8Checker::isConsistent([[maybe_unused]] const ExecutionGraph &g) const
{

	return true && checkConsAcyclic1(g);
}

bool ARM8Checker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true && visitCoherenceRelinche(g);
}

void ARM8Checker::recomputeCacheCounters([[maybe_unused]] const ExecutionGraph &g) const {}

void ARM8Checker::resetCacheCounters() const {}

void ARM8Checker::maybeDecreaseCacheCounters([[maybe_unused]] const EventLabel *lab) const {}

void ARM8Checker::maybeIncreaseCacheCounters([[maybe_unused]] const EventLabel *lab) const {}

// NOLINTEND
