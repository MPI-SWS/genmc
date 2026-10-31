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

#include <ranges>

/* The surviving co-maximal predecessor of LAB (the initializer if none) */
template <typename F>
static auto survivingCoPred(const EventLabel *lab, F surviving) -> const EventLabel *
{
	const auto &g = *lab->getParent();
	auto preds = g.co_preds(lab);
	auto it = std::ranges::find_if(preds, surviving);
	return it != std::ranges::end(preds) ? static_cast<const EventLabel *>(&*it)
					     : g.getInitLabel();
}

/* The first surviving poloc-successor of RLAB not reading from a store po-after RLAB, or null */
template <typename F>
static auto poLocNext(const ReadLabel *rLab, F surviving) -> const EventLabel *
{
	auto readsPoLater = [rLab](const EventLabel &lab) {
		const auto *oLab = genmc::dyn_cast<ReadLabel>(&lab);
		return oLab && oLab->getRf() && oLab->getRf()->getThread() == rLab->getThread() &&
		       oLab->getRf()->getIndex() > rLab->getIndex();
	};
	auto succs = rLab->getParent()->poloc_succs(rLab);
	auto it = std::ranges::find_if(
		succs, [&](const EventLabel &lab) { return surviving(lab) && !readsPoLater(lab); });
	return it != std::ranges::end(succs) ? &*it : nullptr;
}

/* The po-first surviving same-location store after WLAB, or null */
template <typename F>
static auto poLocNextStore(const WriteLabel *wLab, F surviving) -> const EventLabel *
{
	auto succs = wLab->getParent()->poloc_succs(wLab);
	auto it = std::ranges::find_if(succs, [&](const EventLabel &lab) {
		return genmc::isa<WriteLabel>(&lab) && surviving(lab);
	});
	return it != std::ranges::end(succs) ? &*it : nullptr;
}

auto IMMChecker::getMaximalRf(const ReadLabel *rLab) const -> const EventLabel *
{
	auto surviving = [](const EventLabel &lab) { return !lab.isSkeleton(); };
	const auto *nLab = poLocNext(rLab, surviving);
	if (nLab == nullptr)
		return ConsistencyChecker::getMaximalRf(rLab);

	const auto *rnLab = genmc::dyn_cast<ReadLabel>(nLab);
	return rnLab != nullptr ? rnLab->getRf() : survivingCoPred(nLab, surviving);
}

auto IMMChecker::getMaximalCo(const WriteLabel *wLab) const -> const EventLabel *
{
	auto surviving = [wLab](const EventLabel &lab) {
		return !lab.isSkeleton() && &lab != wLab;
	};
	const auto *nLab = poLocNextStore(wLab, surviving);
	return nLab != nullptr ? survivingCoPred(nLab, surviving)
			       : ConsistencyChecker::getMaximalCo(wLab);
}

auto IMMChecker::isMaximalRf(const ReadLabel *rLab, const VectorClock &keep,
			     const EventLabel *except) const -> bool
{
	auto surviving = [&](const EventLabel &lab) {
		return survives(lab, rLab->getStamp(), keep, except);
	};
	const auto *nLab = poLocNext(rLab, surviving);
	if (nLab == nullptr)
		return ConsistencyChecker::isMaximalRf(rLab, keep, except);

	const auto *rnLab = genmc::dyn_cast<ReadLabel>(nLab);
	return rLab->getRf() ==
	       (rnLab != nullptr ? rnLab->getRf() : survivingCoPred(nLab, surviving));
}

auto IMMChecker::isMaximalCo(const WriteLabel *wLab, const VectorClock &keep,
			     const EventLabel *except) const -> bool
{
	auto surviving = [&](const EventLabel &lab) {
		return survives(lab, wLab->getStamp(), keep, except);
	};
	const auto *nLab = poLocNextStore(wLab, surviving);
	if (nLab == nullptr)
		return ConsistencyChecker::isMaximalCo(wLab, keep, except);

	/* WLAB must be the surviving co-predecessor of NLAB */
	auto survivingOrSelf = [&](const EventLabel &lab) {
		return &lab == wLab || surviving(lab);
	};
	return survivingCoPred(nLab, survivingOrSelf) == wLab;
}

bool IMMChecker::isDepTracking() const { return 1; }

bool IMMChecker::visitCalc80Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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
			if (true && lab->isAtLeastAcquire())
				if (auto *iLab =
					    genmc::dyn_cast_if_present<InitLabel>(g.rf_pred(lab));
				    iLab) {
					calcRes.update(iLab->locView(
						genmc::cast<ReadLabel>(lab)->getAddr(), 0));
				}
			if (true && lab->isAtLeastAcquire())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease()) {
						if (!genmc::isa<InitLabel>(pLab)) {
							worklist.emplace_back(1, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire())
				if (auto pLab = g.rf_pred(lab); pLab) {
					if (!genmc::isa<InitLabel>(pLab)) {
						auto status = visitedCalc80_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
				}
			if (true && lab->isAtLeastAcquire())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    ((genmc::isa<ReadLabel>(pLab) &&
					      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
					     (genmc::isa<WriteLabel>(pLab) &&
					      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
						if (!genmc::isa<InitLabel>(pLab)) {
							auto status = visitedCalc80_5.getStatus(
								pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(5, pLab);
							}
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc80_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc80_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCalc80_3.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc80_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCalc80_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

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
				auto status = visitedCalc80_3.getStatus(pLab->getStamp().get());
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
				if (true && pLab->isAtLeastRelease()) {
					if (!genmc::isa<InitLabel>(pLab)) {
						worklist.emplace_back(1, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				if (!genmc::isa<InitLabel>(pLab)) {
					auto status =
						visitedCalc80_3.getStatus(pLab->getStamp().get());
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
						auto status = visitedCalc80_5.getStatus(
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
				visitedCalc80_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc80_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc80_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

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
				visitedCalc80_6.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc80_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCalc80_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease())
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
				if (true && pLab->isAtLeastRelease()) {
					if (!genmc::isa<InitLabel>(pLab)) {
						worklist.emplace_back(1, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				if (!genmc::isa<InitLabel>(pLab)) {
					auto status =
						visitedCalc80_3.getStatus(pLab->getStamp().get());
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
						auto status = visitedCalc80_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc80_6.getStatus(pLab->getStamp().get());
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

View IMMChecker::visitCalc80(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc80_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc80_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc80_6.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{0, lab},
		{2, lab},
	};

	visitCalc80Iterative(startStates, calcRes);
	return calcRes;
}

auto IMMChecker::checkCalc80(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc80(lab);
}

bool IMMChecker::visitCalc81Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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
			if (true && lab->isAtLeastAcquire() &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto *iLab =
					    genmc::dyn_cast_if_present<InitLabel>(g.rf_pred(lab));
				    iLab) {
					calcRes.update(iLab->locView(
						genmc::cast<ReadLabel>(lab)->getAddr(), 1));
				}
			if (true && lab->isAtLeastAcquire() &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease()) {
						if (!genmc::isa<InitLabel>(pLab)) {
							worklist.emplace_back(1, pLab);
						}
					}
			if (true && lab->isAtLeastAcquire() &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.rf_pred(lab); pLab) {
					if (!genmc::isa<InitLabel>(pLab)) {
						auto status = visitedCalc81_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
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
						if (!genmc::isa<InitLabel>(pLab)) {
							auto status = visitedCalc81_5.getStatus(
								pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(5, pLab);
							}
						}
					}
			if (true && lab->isAtLeastAcquire() && genmc::isa<FenceLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc81_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadJoinLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc81_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (true && lab->isAtLeastAcquire() && genmc::isa<ThreadStartLabel>(lab) &&
			    !(genmc::isa<AbstractLockCasReadLabel>(lab)))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc81_6.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCalc81_3.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc81_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCalc81_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

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
				auto status = visitedCalc81_3.getStatus(pLab->getStamp().get());
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
						if (!genmc::isa<InitLabel>(pLab)) {
							worklist.emplace_back(0, pLab);
						}
					}
			if (auto *iLab = genmc::dyn_cast_if_present<InitLabel>(g.rf_pred(lab));
			    iLab) {
				calcRes.update(
					iLab->locView(genmc::cast<ReadLabel>(lab)->getAddr(), 1));
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					if (!genmc::isa<InitLabel>(pLab)) {
						worklist.emplace_back(1, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				if (!genmc::isa<InitLabel>(pLab)) {
					auto status =
						visitedCalc81_3.getStatus(pLab->getStamp().get());
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
						auto status = visitedCalc81_5.getStatus(
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
				visitedCalc81_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc81_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc81_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

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
				visitedCalc81_6.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc81_6.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(6, lab, true);
			visitedCalc81_6.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease())
					if (calcRes.updateIdx(pLab->getPos()); true) {
						if (!genmc::isa<InitLabel>(pLab)) {
							worklist.emplace_back(0, pLab);
						}
					}
			if (auto *iLab = genmc::dyn_cast_if_present<InitLabel>(g.rf_pred(lab));
			    iLab) {
				calcRes.update(
					iLab->locView(genmc::cast<ReadLabel>(lab)->getAddr(), 1));
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					if (!genmc::isa<InitLabel>(pLab)) {
						worklist.emplace_back(1, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				if (!genmc::isa<InitLabel>(pLab)) {
					auto status =
						visitedCalc81_3.getStatus(pLab->getStamp().get());
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
						auto status = visitedCalc81_5.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(5, pLab);
						}
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc81_6.getStatus(pLab->getStamp().get());
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

View IMMChecker::visitCalc81(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc81_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc81_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc81_6.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{0, lab},
		{2, lab},
	};

	visitCalc81Iterative(startStates, calcRes);
	return calcRes;
}

auto IMMChecker::checkCalc81(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc81(lab);
}

bool IMMChecker::visitCalc94Iterative(std::vector<DFSWorklistEntry> &worklist, View &calcRes) const
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
					iLab->locView(genmc::cast<ReadLabel>(lab)->getAddr(), 2));
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

View IMMChecker::visitCalc94(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	View calcRes;

	calcRes.updateIdx(lab->getPos());

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{0, lab},
		{2, lab},
	};

	visitCalc94Iterative(startStates, calcRes);
	return calcRes;
}

auto IMMChecker::checkCalc94(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc94(lab);
}

bool IMMChecker::visitCalc95Iterative(std::vector<DFSWorklistEntry> &worklist,
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
				visitedCalc95_2.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc95_2.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(2, lab, true);
			visitedCalc95_2.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable())
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			for (auto &tmp : g.ctrl_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable())
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable())
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable()) {
						worklist.emplace_back(1, pLab);
					}
			for (auto &tmp : g.ctrl_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable()) {
						worklist.emplace_back(1, pLab);
					}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable()) {
						worklist.emplace_back(1, pLab);
					}
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_2.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			for (auto &tmp : g.ctrl_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_2.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_2.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.rfi_pred(lab); pLab) {
				auto status = visitedCalc95_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			for (auto &tmp : g.cas_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_3.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			for (auto &tmp : g.ctrl_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_3.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			for (auto &tmp : g.data_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_3.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (auto pLab = g.rfi_pred(lab); pLab) {
				auto status = visitedCalc95_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}

			break;
		}
		case 3: {
			if (isFinishing) {
				visitedCalc95_3.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc95_3.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(3, lab, true);
			visitedCalc95_3.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable())
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)) &&
				    pLab->isDependable())
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isDependable()) {
						worklist.emplace_back(1, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)) &&
				    pLab->isDependable()) {
					worklist.emplace_back(1, pLab);
				}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_2.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					auto status =
						visitedCalc95_2.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					}
				}
			for (auto &tmp : g.addr_preds(lab))
				if (auto *pLab = &tmp; true) {
					auto status =
						visitedCalc95_3.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc95_3.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(3, pLab);
				}
			}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					auto status =
						visitedCalc95_3.getStatus(pLab->getStamp().get());
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
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.addr_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable())
							if (calcRes.updateIdx(pLab->getPos());
							    true) {
								worklist.emplace_back(0, pLab);
							}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.cas_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable())
							if (calcRes.updateIdx(pLab->getPos());
							    true) {
								worklist.emplace_back(0, pLab);
							}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable())
							if (calcRes.updateIdx(pLab->getPos());
							    true) {
								worklist.emplace_back(0, pLab);
							}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.data_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable())
							if (calcRes.updateIdx(pLab->getPos());
							    true) {
								worklist.emplace_back(0, pLab);
							}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)) &&
					    pLab->isDependable())
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab))
						if (calcRes.updateIdx(pLab->getPos()); true) {
							worklist.emplace_back(0, pLab);
						}
			if (true && lab->isAtLeastRelease())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
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
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					worklist.emplace_back(1, pLab);
				}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.addr_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable()) {
							worklist.emplace_back(1, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.cas_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable()) {
							worklist.emplace_back(1, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable()) {
							worklist.emplace_back(1, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.data_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isDependable()) {
							worklist.emplace_back(1, pLab);
						}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)) &&
					    pLab->isDependable()) {
						worklist.emplace_back(1, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						worklist.emplace_back(1, pLab);
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastRelease() &&
					    genmc::isa<WriteLabel>(pLab)) {
						worklist.emplace_back(1, pLab);
					}
			if (true && lab->isAtLeastRelease())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					worklist.emplace_back(1, pLab);
				}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.addr_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_2.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(2, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.cas_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_2.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(2, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_2.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(2, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.data_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_2.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(2, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						auto status = visitedCalc95_2.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(2, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.addr_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.cas_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.ctrl_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto &tmp : g.data_preds(lab))
					if (auto *pLab = &tmp; true) {
						auto status = visitedCalc95_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc95_3.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
						auto status = visitedCalc95_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc95_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}
			if (true && lab->isAtLeastRelease())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc95_5.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab) {
					auto status =
						visitedCalc95_7.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status =
						visitedCalc95_8.getStatus(pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(8, pLab);
					}
				}

			break;
		}
		case 5: {
			if (isFinishing) {
				visitedCalc95_5.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc95_5.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(5, lab, true);
			visitedCalc95_5.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (calcRes.updateIdx(pLab->getPos()); true) {
					worklist.emplace_back(0, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(1, pLab);
			}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc95_5.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(5, pLab);
				}
			}

			break;
		}
		case 6: {
			if (isFinishing) {
				break;
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire())
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<FenceLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				worklist.emplace_back(6, pLab);
			}

			break;
		}
		case 7: {
			if (isFinishing) {
				visitedCalc95_7.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc95_7.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(7, lab, true);
			visitedCalc95_7.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.poloc_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab))
					if (calcRes.updateIdx(pLab->getPos()); true) {
						worklist.emplace_back(0, pLab);
					}
			if (auto pLab = g.poloc_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<WriteLabel>(pLab)) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
				auto status = visitedCalc95_7.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(7, pLab);
				}
			}

			break;
		}
		case 8: {
			if (isFinishing) {
				visitedCalc95_8.setStatus(lab->getStamp().get(), NodeStatus::left);
				break;
			}

			auto status = visitedCalc95_8.getStatus(lab->getStamp().get());
			if (status != NodeStatus::unseen)
				break; /* already explored */

			worklist.emplace_back(8, lab, true);
			visitedCalc95_8.setStatus(lab->getStamp().get(), NodeStatus::entered);

			[[maybe_unused]] auto &g = *lab->getParent();
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
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
					worklist.emplace_back(1, pLab);
				}
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCalc95_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
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

DepView IMMChecker::visitCalc95(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();
	DepView calcRes;

	calcRes.updateIdx(lab->getPos());
	visitedCalc95_2.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc95_3.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc95_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc95_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedCalc95_8.maybeClearResize(g.getMaxStamp().get() + 1);

	/* Explore from all accepting states using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{0, lab},
		{6, lab},
		{4, lab},
	};

	visitCalc95Iterative(startStates, calcRes);
	return calcRes;
}

auto IMMChecker::checkCalc95(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	return visitCalc95(lab);
}

void IMMChecker::calculateSaved([[maybe_unused]] EventLabel *lab) {}

void IMMChecker::calculateViews(EventLabel *lab)
{
	lab->setViews({});
	{
		lab->addView({});
		lab->setView(std::move(checkCalc80(lab)), 0);
	}

	if (!getConf()->collectLinSpec && !getConf()->checkLinSpec)
		lab->addView({});
	else {
		lab->addView({});
		lab->setView(std::move(checkCalc81(lab)), 1);
	}
	{
		lab->addView({});
		lab->setView(std::move(checkCalc94(lab)), 2);
	}
}

void IMMChecker::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
	lab->setPrefixView(calculatePrefixView(lab));
}

std::unique_ptr<VectorClock> IMMChecker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<DepView>(checkCalc95(lab));
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
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedCoherence_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_5.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(5, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
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
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_9.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(9, pLab);
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
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
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
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
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_0.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(0, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCoherence_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_3.getStatus(
						pLab->getStamp().get());
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
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_3.getStatus(pLab->getStamp().get());
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
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_3.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(3, pLab);
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedCoherence_3.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(3, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
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
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedCoherence_9.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(9, pLab);
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedCoherence_9.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_11.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(11, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
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
				    genmc::isa<ThreadCreateLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
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
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedCoherence_9.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
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
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_6.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(6, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_7.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(7, pLab);
					}
				}
			if (auto pLab = g.rf_pred(lab); pLab) {
				auto status = visitedCoherence_8.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(8, pLab);
				}
			}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedCoherence_9.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(9, pLab);
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
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status = visitedCoherence_9.getStatus(pLab->getStamp().get());
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

bool IMMChecker::visitCoherenceRelinche(const ExecutionGraph &g) const
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
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_2.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(2, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_2.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
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
			if (auto pLab = g.rf_pred(lab); pLab) {
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(3, pLab);
				}

			break;
		}
		case 3: {
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
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			if (auto pLab = g.tc_pred(lab); pLab) {
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
			if (auto pLab = g.tj_pred(lab); pLab) {
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
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_7.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(7, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_7.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
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
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
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
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(8, pLab);
				}

			break;
		}
		case 8: {
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
			if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			for (auto &tmp : g.lin_preds(lab))
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
			if (auto pLab = g.tc_pred(lab); pLab) {
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
			if (auto pLab = g.tj_pred(lab); pLab) {
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
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_14.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(14, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_14.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
					auto status = visitedConsAcyclic1_14.getStatus(
						pLab->getStamp().get());
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
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
					auto status = visitedConsAcyclic1_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			for (auto &tmp : g.lin_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			if (auto pLab = g.tc_pred(lab); pLab) {
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
			if (auto pLab = g.tj_pred(lab); pLab) {
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
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
				if (true && pLab->isAtLeastAcquire()) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<FenceLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() &&
				    genmc::isa<ThreadJoinLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
								    pLab->getStamp().get()) ||
						    1)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
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
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			if (auto pLab = g.co_imm_pred(lab); pLab) {
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
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_7.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(7, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_7.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
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
			if (auto pLab = g.co_imm_pred(lab); pLab) {
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
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_18.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(18, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_18.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isSC()) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
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
				if (true && pLab->isAtLeastRelease() &&
				    genmc::isa<ThreadCreateLabel>(pLab)) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_14.getStatus(
						pLab->getStamp().get());
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
					auto status = visitedConsAcyclic1_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease() &&
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
				    genmc::isa<ThreadCreateLabel>(pLab)) {
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
				if (true && pLab->isAtLeastRelease() && pLab->isSC() &&
				    genmc::isa<FenceLabel>(pLab)) {
					auto status = visitedConsAcyclic1_18.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(18, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_18.getCount(
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
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
			if (auto pLab = g.rf_pred(lab); pLab) {
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire() && pLab->isAtLeastRelease()) {
					auto status = visitedConsAcyclic1_14.getStatus(
						pLab->getStamp().get());
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
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    ((genmc::isa<ReadLabel>(pLab) &&
				      genmc::dyn_cast<ReadLabel>(pLab)->isRMW()) ||
				     (genmc::isa<WriteLabel>(pLab) &&
				      genmc::dyn_cast<WriteLabel>(pLab)->isRMW()))) {
					worklist.emplace_back(15, pLab);
				}
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastRelease()) {
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
					auto status = visitedConsAcyclic1_14.getStatus(
						pLab->getStamp().get());
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

			break;
		}
		case 16: {
			if (isFinishing) {
				visitedConsAcyclic1_16.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_16.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic1_16.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::entered);
				worklist.emplace_back(16, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_16.getCount(
									   lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
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
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic1_16.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(16, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting >
						    visitedConsAcyclic1_16.getCount(
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
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			if (auto pLab = g.co_imm_pred(lab); pLab) {
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
			if (auto pLab = g.rf_pred(lab); pLab) {
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
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_7.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(7, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_7.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (auto pLab = g.co_imm_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
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
			if (auto pLab = g.rf_pred(lab); pLab)
				if (true && pLab->isAtLeastAcquire()) {
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
			for (auto &tmp : g.fr_imm_preds(lab))
				if (auto *pLab = &tmp; true) {
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
			if (auto pLab = g.co_imm_pred(lab); pLab) {
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
			if (auto pLab = g.rf_pred(lab); pLab) {
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
				--visitedConsAcyclic1Accepting;
				visitedConsAcyclic1_18.set(lab->getStamp().get(),
							   visitedConsAcyclic1Accepting,
							   NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic1_18.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				++visitedConsAcyclic1Accepting;
				visitedConsAcyclic1_18.setIncr(lab->getStamp().get(),
							       visitedConsAcyclic1Accepting,
							       NodeStatus::entered);
				worklist.emplace_back(18, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > visitedConsAcyclic1_18.getCount(
									   lab->getStamp().get()) ||
				    1)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_2.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(2, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_2.getCount(
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
						auto status = visitedConsAcyclic1_4.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(4, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_4.getCount(
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
						auto status = visitedConsAcyclic1_4.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(4, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_4.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (true && lab->isSC())
				for (auto &tmp : g.fr_imm_preds(lab))
					if (auto *pLab = &tmp; true) {
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
				if (auto pLab = g.co_imm_pred(lab); pLab) {
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
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (true && lab->isSC())
				if (auto pLab = g.rf_pred(lab); pLab) {
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
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_7.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(7, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_7.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				for (auto &tmp : g.fr_imm_preds(lab))
					if (auto *pLab = &tmp; true)
						if (true && pLab->isAtLeastAcquire()) {
							auto status =
								visitedConsAcyclic1_7.getStatus(
									pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(7, pLab);
							} else if (
								status == NodeStatus::entered &&
								(visitedConsAcyclic1Accepting >
									 visitedConsAcyclic1_7.getCount(
										 pLab->getStamp()
											 .get()) ||
								 0)) {
								return false; /* cycle detected */
							}
						}
			if (true && lab->isSC())
				if (auto pLab = g.co_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_7.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(7, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_7.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_7.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(7, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_7.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_7.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(7, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_7.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_9.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(9, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_9.getCount(
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
						auto status = visitedConsAcyclic1_9.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(9, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_9.getCount(
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
						auto status = visitedConsAcyclic1_9.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(9, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_9.getCount(
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
						auto status = visitedConsAcyclic1_9.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(9, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_9.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (true && lab->isSC())
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
				if (auto pLab = g.co_imm_pred(lab); pLab) {
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
			if (true && lab->isAtLeastAcquire() && lab->isSC() &&
			    genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_14.getStatus(
						pLab->getStamp().get());
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
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire()) {
						auto status = visitedConsAcyclic1_14.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(14, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_14.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isAtLeastAcquire() && lab->isSC() &&
			    genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic1_16.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(16, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic1Accepting >
							    visitedConsAcyclic1_16.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isAtLeastAcquire() &&
					    genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_16.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(16, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_16.getCount(
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
						auto status = visitedConsAcyclic1_16.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(16, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_16.getCount(
									    pLab->getStamp()
										    .get()) ||
							    0)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
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
					if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_18.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(18, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_18.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC() && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_18.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(18, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_18.getCount(
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
								visitedConsAcyclic1_18.getStatus(
									pLab->getStamp().get());
							if (status == NodeStatus::unseen) {
								worklist.emplace_back(18, pLab);
							} else if (
								status == NodeStatus::entered &&
								(visitedConsAcyclic1Accepting >
									 visitedConsAcyclic1_18.getCount(
										 pLab->getStamp()
											 .get()) ||
								 1)) {
								return false; /* cycle detected */
							}
						}
			if (true && lab->isSC())
				if (auto pLab = g.co_imm_pred(lab); pLab)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_18.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(18, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_18.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isSC() && genmc::isa<FenceLabel>(pLab)) {
						auto status = visitedConsAcyclic1_18.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(18, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_18.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_18.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(18, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_18.getCount(
									    pLab->getStamp()
										    .get()) ||
							    1)) {
							return false; /* cycle detected */
						}
					}
			if (true && lab->isSC())
				if (auto pLab = g.rf_pred(lab); pLab)
					if (true && pLab->isSC()) {
						auto status = visitedConsAcyclic1_18.getStatus(
							pLab->getStamp().get());
						if (status == NodeStatus::unseen) {
							worklist.emplace_back(18, pLab);
						} else if (status == NodeStatus::entered &&
							   (visitedConsAcyclic1Accepting >
								    visitedConsAcyclic1_18.getCount(
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
	visitedConsAcyclic1_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_16.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_17.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.maybeClearResize(g.getMaxStamp().get() + 1);

	/* States we need to explore from using DFS */
	std::vector<DFSWorklistEntry> startStates = {
		{18, lab}, {17, lab}, {14, lab}, {12, lab}, {11, lab},
		{10, lab}, {7, lab},  {5, lab},	 {2, lab},  {0, lab},
	};

	return visitConsAcyclic1Iterative(startStates);
}

bool IMMChecker::visitConsAcyclic1Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_4.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_5.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_6.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_7.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_9.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_10.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_11.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_12.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_13.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_14.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_16.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_17.maybeClearResize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_18.maybeClearResize(g.getMaxStamp().get() + 1);

	auto exploreLab = [&](auto &lab) {
		/* Explore from all accepting states using DFS */
		std::vector<DFSWorklistEntry> startStates = {
			{18, &lab},
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
			for (auto &tmp : g.cas_preds(lab))
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
			for (auto &tmp : g.cas_preds(lab))
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
			for (auto &tmp : g.cas_preds(lab))
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<ReadLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab)) &&
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
		case 2: {
			if (isFinishing) {
				visitedConsAcyclic2_2.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::left);
				break;
			}

			auto status = visitedConsAcyclic2_2.getStatus(lab->getStamp().get());
			if (status == NodeStatus::unseen) {
				visitedConsAcyclic2_2.set(lab->getStamp().get(),
							  visitedConsAcyclic2Accepting,
							  NodeStatus::entered);
				worklist.emplace_back(2, lab, true /* isFinishing */);
			} else if (status == NodeStatus::entered &&
				   (visitedConsAcyclic2Accepting >
					    visitedConsAcyclic2_2.getCount(lab->getStamp().get()) ||
				    0)) {
				return false; /* cycle detected */
			} else if (status == NodeStatus::left) {
				break; /* already explored*/
			}

			[[maybe_unused]] auto &g = *lab->getParent();
			if (auto pLab = g.po_imm_pred(lab); pLab) {
				auto status =
					visitedConsAcyclic2_2.getStatus(pLab->getStamp().get());
				if (status == NodeStatus::unseen) {
					worklist.emplace_back(2, pLab);
				} else if (status == NodeStatus::entered &&
					   (visitedConsAcyclic2Accepting >
						    visitedConsAcyclic2_2.getCount(
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
			if (auto pLab = g.poloc_imm_pred(lab); pLab) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab) {
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
			if (auto pLab = g.po_imm_pred(lab); pLab)
				if (true && genmc::isa<WriteLabel>(pLab) &&
				    (genmc::isa<CasReadLabel>(pLab) ||
				     genmc::isa<FaiReadLabel>(pLab) ||
				     genmc::isa<CasWriteLabel>(pLab) ||
				     genmc::isa<FaiWriteLabel>(pLab))) {
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
				for (auto &tmp : g.cas_preds(lab))
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
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
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
				for (auto &tmp : g.cas_preds(lab))
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
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
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
			if (true && genmc::isa<FenceLabel>(lab))
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_2.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
			if (true && lab->isAtLeastRelease())
				if (auto pLab = g.po_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_2.getStatus(
						pLab->getStamp().get());
					if (status == NodeStatus::unseen) {
						worklist.emplace_back(2, pLab);
					} else if (status == NodeStatus::entered &&
						   (visitedConsAcyclic2Accepting >
							    visitedConsAcyclic2_2.getCount(
								    pLab->getStamp().get()) ||
						    0)) {
						return false; /* cycle detected */
					}
				}
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
			if (true && genmc::isa<WriteLabel>(lab))
				if (auto pLab = g.poloc_imm_pred(lab); pLab) {
					auto status = visitedConsAcyclic2_4.getStatus(
						pLab->getStamp().get());
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
				if (auto pLab = g.po_imm_pred(lab); pLab) {
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
				for (auto &tmp : g.cas_preds(lab))
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
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<ReadLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab)) &&
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
				if (auto pLab = g.po_imm_pred(lab); pLab)
					if (true && genmc::isa<WriteLabel>(pLab) &&
					    (genmc::isa<CasReadLabel>(pLab) ||
					     genmc::isa<FaiReadLabel>(pLab) ||
					     genmc::isa<CasWriteLabel>(pLab) ||
					     genmc::isa<FaiWriteLabel>(pLab))) {
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
	visitedConsAcyclic2_2.maybeClearResize(g.getMaxStamp().get() + 1);
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
	visitedConsAcyclic2_2.maybeClearResize(g.getMaxStamp().get() + 1);
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

bool IMMChecker::visitError3([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessError3Iterative(std::vector<DFSWorklistEntry> &worklist,
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

bool IMMChecker::visitUnlessError3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError3Accepting.clear();
	visitedLHSUnlessError3Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(0);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError3Iterative(startStatesLHS, v);
}

bool IMMChecker::checkError3(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError3(lab))
		return true;

	return visitError3(lab);
}

bool IMMChecker::visitError4([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessError4Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool IMMChecker::visitRHSUnlessError4Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool IMMChecker::visitUnlessError4(const EventLabel *lab) const
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

bool IMMChecker::checkError4(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}

bool IMMChecker::visitError5([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessError5Iterative(std::vector<DFSWorklistEntry> &worklist,
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
				for (auto tmpE : g.pomax_at_reads_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				for (auto tmpE : g.pomax_at_writes_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				for (auto tmpE : g.pomax_na_reads_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && genmc::isa<FreeLabel>(lab) && !genmc::isa<HpRetireLabel>(lab))
				for (auto tmpE : g.pomax_na_writes_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
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

bool IMMChecker::visitUnlessError5(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError5Accepting.clear();
	visitedLHSUnlessError5Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(0);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError5Iterative(startStatesLHS, v);
}

bool IMMChecker::checkError5(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError5(lab))
		return true;

	return visitError5(lab);
}

bool IMMChecker::visitError6([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessError6Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool IMMChecker::visitRHSUnlessError6Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool IMMChecker::visitUnlessError6(const EventLabel *lab) const
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

bool IMMChecker::checkError6(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError6(lab))
		return true;

	return visitError6(lab);
}

bool IMMChecker::visitError7([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessError7Iterative(std::vector<DFSWorklistEntry> &worklist,
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
				for (auto tmpE : g.unprotected_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
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

bool IMMChecker::visitUnlessError7(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError7Accepting.clear();
	visitedLHSUnlessError7Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(0);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError7Iterative(startStatesLHS, v);
}

bool IMMChecker::checkError7(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError7(lab))
		return true;

	return visitError7(lab);
}

bool IMMChecker::visitError8([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessError8Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool IMMChecker::visitRHSUnlessError8Iterative(std::vector<DFSWorklistEntry> &worklist) const
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

bool IMMChecker::visitUnlessError8(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError8Accepting.clear();
	visitedLHSUnlessError8Accepting.resize(g.getMaxStamp().get() + 1, false);
	visitedRHSUnlessError8Accepting.clear();
	visitedRHSUnlessError8Accepting.resize(g.getMaxStamp().get() + 1, false);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	if (!visitLHSUnlessError8Iterative(startStatesLHS))
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

bool IMMChecker::checkError8(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError8(lab))
		return true;

	return visitError8(lab);
}

bool IMMChecker::visitError9([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessError9Iterative(std::vector<DFSWorklistEntry> &worklist,
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
				for (auto tmpE : g.pomax_na_writes_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto tmpE : g.pomax_na_reads_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && genmc::isa<WriteLabel>(lab))
				for (auto tmpE : g.pomax_na_writes_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && lab->isNotAtomic() && genmc::isa<ReadLabel>(lab))
				for (auto tmpE : g.pomax_at_writes_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))
				for (auto tmpE : g.pomax_at_reads_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
						return false;
					}
				}
			if (true && lab->isNotAtomic() && genmc::isa<WriteLabel>(lab))
				for (auto tmpE : g.pomax_at_writes_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
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

bool IMMChecker::visitUnlessError9(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessError9Accepting.clear();
	visitedLHSUnlessError9Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(0);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessError9Iterative(startStatesLHS, v);
}

bool IMMChecker::checkError9(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessError9(lab))
		return true;

	return visitError9(lab);
}

bool IMMChecker::visitWarning10([[maybe_unused]] const EventLabel *lab) const { return false; }

bool IMMChecker::visitLHSUnlessWarning10Iterative(std::vector<DFSWorklistEntry> &worklist,
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
				for (auto tmpE : g.pomax_at_writes_pos(lab)) {
					if (!v.contains(tmpE)) {
						cexLab = g.getEventLabelIfPresent(tmpE);
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

bool IMMChecker::visitUnlessWarning10(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	visitedLHSUnlessWarning10Accepting.clear();
	visitedLHSUnlessWarning10Accepting.resize(g.getMaxStamp().get() + 1, false);

	auto &v = lab->view(2);

	/* Explore from all accepting states in LHS using DFS */
	std::vector<DFSWorklistEntry> startStatesLHS = {
		{1, lab},
	};

	return visitLHSUnlessWarning10Iterative(startStatesLHS, v);
}

bool IMMChecker::checkWarning10(const EventLabel *lab) const
{
	[[maybe_unused]] auto &g = *lab->getParent();

	if (visitUnlessWarning10(lab))
		return true;

	return visitWarning10(lab);
}

std::optional<VerificationError>
IMMChecker::checkErrors([[maybe_unused]] const EventLabel *lab,
			[[maybe_unused]] const EventLabel *&race) const
{
	if (!checkError3(lab)) {
		race = cexLab;
		return {VerificationError::VE_AccessNonMalloc};
	}

	if (!checkError4(lab)) {
		race = cexLab;
		return {VerificationError::VE_DoubleFree};
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
IMMChecker::checkWarnings([[maybe_unused]] const EventLabel *lab,
			  [[maybe_unused]] const VSet<VerificationError> &seenWarnings,
			  [[maybe_unused]] std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning10(lab)) {
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
