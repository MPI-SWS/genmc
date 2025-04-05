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

#include "TSOChecker.hpp"
#include "ADT/VSet.hpp"
#include "ADT/View.hpp"
#include "Config/Config.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include "ExecutionGraph/GraphUtils.hpp"
#include "Verification/VerificationError.hpp"

bool TSOChecker::isDepTracking() const { return 0; }

bool TSOChecker::visitCalc62_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool TSOChecker::visitCalc62_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc62_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitCalc62_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc62_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc62_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc62_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc62_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		if (!visitCalc62_1(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

bool TSOChecker::visitCalc62_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc62_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc62_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc62_2(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

View TSOChecker::visitCalc62(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitCalc62_3(lab, calcRes);
	return calcRes;
}
auto TSOChecker::checkCalc62(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc62(lab);
}
bool TSOChecker::visitCalc63_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool TSOChecker::visitCalc63_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(0)); true) {
			if (!visitCalc63_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitCalc63_2(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		if (!visitCalc63_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		if (!visitCalc63_1(pLab, calcRes)) {
			return false;
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			if (!visitCalc63_1(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		if (!visitCalc63_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = tc_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc63_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = tj_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc63_0(pLab, calcRes)) {
				return false;
			}
		}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true)
			if (calcRes.updateIdx(pLab->getPos()); true) {
				if (!visitCalc63_0(pLab, calcRes)) {
					return false;
				}
			}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc63_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitCalc63_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc63_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc63_2(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc63_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

View TSOChecker::visitCalc63(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitCalc63_3(lab, calcRes);
	return calcRes;
}
auto TSOChecker::checkCalc63(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc63(lab);
}
bool TSOChecker::visitCalc64_0(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	return true;
}

bool TSOChecker::visitCalc64_1(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = lab; true)
		if (calcRes.update(pLab->view(2)); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitCalc64_2(const EventLabel *lab, View &calcRes) const
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

	return true;
}

bool TSOChecker::visitCalc64_3(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = rf_pred(g, lab); pLab) {
		if (!visitCalc64_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = rf_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitCalc64_4(const EventLabel *lab, View &calcRes) const
{
	auto &g = *lab->getParent();

	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc64_1(pLab, calcRes)) {
			return false;
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (calcRes.updateIdx(pLab->getPos()); true) {
			if (!visitCalc64_0(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab)
		if (true && !(llvm::isa<AbstractLockCasReadLabel>(pLab))) {
			if (!visitCalc64_3(pLab, calcRes)) {
				return false;
			}
		}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCalc64_2(pLab, calcRes)) {
			return false;
		}
	}

	return true;
}

View TSOChecker::visitCalc64(const EventLabel *lab) const
{
	auto &g = *lab->getParent();
	View calcRes;

	visitCalc64_4(lab, calcRes);
	return calcRes;
}
auto TSOChecker::checkCalc64(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitCalc64(lab);
}
void TSOChecker::calculateSaved(EventLabel *lab) {}

void TSOChecker::calculateViews(EventLabel *lab)
{

	lab->addView(checkCalc62(lab));

	lab->addView(checkCalc63(lab));
	if (!getConf()->collectLinSpec && !getConf()->checkLinSpec)
		lab->addView({});
	else
		lab->addView(checkCalc64(lab));
}

void TSOChecker::updateMMViews(EventLabel *lab)
{
	calculateViews(lab);
	calculateSaved(lab);
	lab->setPrefixView(calculatePrefixView(lab));
}

const View &TSOChecker::getHbView(const EventLabel *lab) const { return lab->view(1); }

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

auto TSOChecker::getCoherentStores(ReadLabel *rLab) -> std::vector<EventLabel *>
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
	auto *confLab = findPendingRMW(sLab);
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

void TSOChecker::filterCoherentRevisits(WriteLabel *sLab, std::vector<ReadLabel *> &ls)
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

auto TSOChecker::getCoherentPlacings(WriteLabel *wLab) -> std::vector<EventLabel *>
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
bool TSOChecker::visitCoherence_0(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (lab == initLab)
		return false;

	return true;
}

bool TSOChecker::visitCoherence_1(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_5(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	return true;
}

bool TSOChecker::visitCoherence_2(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_2[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_2[lab->getStamp().get()] = NodeStatus::entered;

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
		if (!visitCoherence_3(pLab, initLab)) {
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

bool TSOChecker::visitCoherence_3(const EventLabel *lab, const EventLabel *initLab) const
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
	if (auto pLab = rf_pred(g, lab); pLab) {
		if (!visitCoherence_0(pLab, initLab)) {
			return false;
		}
	}

	return true;
}

bool TSOChecker::visitCoherence_4(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_4[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::entered;

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
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = co_imm_pred(g, lab); pLab) {
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
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_4[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_4(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}

	visitedCoherence_4[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool TSOChecker::visitCoherence_5(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (visitedCoherence_5[lab->getStamp().get()] != NodeStatus::unseen)
		return true;
	visitedCoherence_5[lab->getStamp().get()] = NodeStatus::entered;

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
		auto status = visitedCoherence_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_5(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_pred(g, lab); pLab) {
		if (!visitCoherence_6(pLab, initLab)) {
			return false;
		}
	}

	visitedCoherence_5[lab->getStamp().get()] = NodeStatus::left;
	return true;
}

bool TSOChecker::visitCoherence_6(const EventLabel *lab, const EventLabel *initLab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = tc_pred(g, lab); pLab) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCoherence_4[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_4(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
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
		auto status = visitedCoherence_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_5(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_pred(g, lab); pLab) {
		auto status = visitedCoherence_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_5(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_preds(g, lab))
		if (auto *pLab = &tmp; true) {
			auto status = visitedCoherence_5[pLab->getStamp().get()];
			if (status == NodeStatus::unseen) {
				if (!visitCoherence_5(pLab, initLab)) {
					return false;
				}

			} else if (status == NodeStatus::entered) {

			} else if (status == NodeStatus::left) {
			}
		}
	if (auto pLab = rf_pred(g, lab); pLab) {
		auto status = visitedCoherence_5[pLab->getStamp().get()];
		if (status == NodeStatus::unseen) {
			if (!visitCoherence_5(pLab, initLab)) {
				return false;
			}

		} else if (status == NodeStatus::entered) {

		} else if (status == NodeStatus::left) {
		}
	}

	return true;
}

bool TSOChecker::visitCoherenceRelinche(const ExecutionGraph &g) const
{
	for (auto &lab : g.labels()) {
		if (!llvm::isa<MethodBeginLabel>(&lab))
			continue;
		visitedCoherence_2.clear();
		visitedCoherence_2.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_4.clear();
		visitedCoherence_4.resize(g.getMaxStamp().get() + 1);
		visitedCoherence_5.clear();
		visitedCoherence_5.resize(g.getMaxStamp().get() + 1);
		if (true && !visitCoherence_1(&lab, &lab))
			return false;
	}
	return true;
}

bool TSOChecker::visitConsAcyclic1_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_0[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	visitedConsAcyclic1_0[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool TSOChecker::visitConsAcyclic1_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_1[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

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
		if (true && llvm::isa<WriteLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	visitedConsAcyclic1_1[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool TSOChecker::visitConsAcyclic1_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1_2[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && pLab->isSC() && llvm::isa<ReadLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
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
	visitedConsAcyclic1_2[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool TSOChecker::visitConsAcyclic1_3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	++visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_3[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::entered};

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
	if (auto pLab = tc_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = tj_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : lin_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = co_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	if (auto pLab = fr_imm_succ(g, lab); pLab) {
		auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
		if (node.status == NodeStatus::unseen) {
			if (!visitConsAcyclic1_3(pLab)) {
				return false;
			}

		} else if (node.status == NodeStatus::entered &&
			   (visitedConsAcyclic1Accepting > node.count || 1)) {

			return false;
		} else if (node.status == NodeStatus::left) {
		}
	}
	for (auto &tmp : rfe_succs(g, lab))
		if (auto *pLab = &tmp; true) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && llvm::isa<WriteLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && llvm::isa<FenceLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && llvm::isa<ThreadCreateLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (auto pLab = po_imm_succ(g, lab); pLab)
		if (true && llvm::isa<ThreadFinishLabel>(pLab)) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<WriteLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab)
			if (true && pLab->isSC() && llvm::isa<ReadLabel>(pLab)) {
				auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
				if (node.status == NodeStatus::unseen) {
					if (!visitConsAcyclic1_3(pLab)) {
						return false;
					}

				} else if (node.status == NodeStatus::entered &&
					   (visitedConsAcyclic1Accepting > node.count || 1)) {

					return false;
				} else if (node.status == NodeStatus::left) {
				}
			}
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<ReadLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && llvm::isa<ThreadStartLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
			auto &node = visitedConsAcyclic1_3[pLab->getStamp().get()];
			if (node.status == NodeStatus::unseen) {
				if (!visitConsAcyclic1_3(pLab)) {
					return false;
				}

			} else if (node.status == NodeStatus::entered &&
				   (visitedConsAcyclic1Accepting > node.count || 1)) {

				return false;
			} else if (node.status == NodeStatus::left) {
			}
		}
	if (true && lab->isSC() && llvm::isa<WriteLabel>(lab))
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
	if (true && llvm::isa<WriteLabel>(lab) &&
	    ((llvm::isa<ReadLabel>(lab) && llvm::dyn_cast<ReadLabel>(lab)->isRMW()) ||
	     (llvm::isa<WriteLabel>(lab) && llvm::dyn_cast<WriteLabel>(lab)->isRMW())))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && llvm::isa<ReadLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && llvm::isa<FenceLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && llvm::isa<ThreadJoinLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	if (true && llvm::isa<ThreadStartLabel>(lab))
		if (auto pLab = po_imm_succ(g, lab); pLab) {
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
	--visitedConsAcyclic1Accepting;
	visitedConsAcyclic1_3[lab->getStamp().get()] = {visitedConsAcyclic1Accepting,
							NodeStatus::left};

	return true;
}

bool TSOChecker::visitConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.clear();
	visitedConsAcyclic1_3.resize(g.getMaxStamp().get() + 1);
	return true && (visitedConsAcyclic1_3[lab->getStamp().get()].status != NodeStatus::unseen ||
			visitConsAcyclic1_3(lab));
}

bool TSOChecker::visitConsAcyclic1Full(const ExecutionGraph &g) const
{
	visitedConsAcyclic1Accepting = 0;
	visitedConsAcyclic1_0.clear();
	visitedConsAcyclic1_0.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_1.clear();
	visitedConsAcyclic1_1.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_2.clear();
	visitedConsAcyclic1_2.resize(g.getMaxStamp().get() + 1);
	visitedConsAcyclic1_3.clear();
	visitedConsAcyclic1_3.resize(g.getMaxStamp().get() + 1);
	return true && std::ranges::all_of(g.labels(), [&](auto &lab) {
		       return visitedConsAcyclic1_3[lab.getStamp().get()].status !=
				      NodeStatus::unseen ||
			      visitConsAcyclic1_3(&lab);
	       });
}

bool TSOChecker::checkConsAcyclic1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return visitConsAcyclic1(lab);
}
bool TSOChecker::checkConsAcyclic1(const ExecutionGraph &g) const
{
	return visitConsAcyclic1Full(g);
}
bool TSOChecker::visitError2(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessError2_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool TSOChecker::visitLHSUnlessError2_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (auto pLab = alloc_pred(g, lab); pLab) {
		if (!visitLHSUnlessError2_0(pLab, v)) {
			return false;
		}
	}

	return true;
}

bool TSOChecker::visitUnlessError2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError2Accepting.clear();
	visitedLHSUnlessError2Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true && visitLHSUnlessError2_1(lab, v);
}

bool TSOChecker::checkError2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError2(lab))
		return true;

	return visitError2(lab);
}
bool TSOChecker::visitError3(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessError3_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool TSOChecker::visitLHSUnlessError3_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<FreeLabel>(pLab) &&
				    !llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError3_0(pLab)) {
						return false;
					}
				}
	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError3_0(pLab)) {
						return false;
					}
				}
	if (true && llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<FreeLabel>(pLab) &&
				    !llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError3_0(pLab)) {
						return false;
					}
				}
	if (true && llvm::isa<HpRetireLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<HpRetireLabel>(pLab)) {
					if (!visitLHSUnlessError3_0(pLab)) {
						return false;
					}
				}

	return true;
}

bool TSOChecker::visitUnlessError3(const EventLabel *lab) const
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

bool TSOChecker::checkError3(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError3(lab))
		return true;

	return visitError3(lab);
}
bool TSOChecker::visitError4(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessError4_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool TSOChecker::visitLHSUnlessError4_1(const EventLabel *lab, const View &v) const
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

bool TSOChecker::visitLHSUnlessError4_2(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError4_1(pLab, v)) {
				return false;
			}
		}
	if (true && llvm::isa<FreeLabel>(lab) && !llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError4_0(pLab, v)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitUnlessError4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError4Accepting.clear();
	visitedLHSUnlessError4Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true && visitLHSUnlessError4_2(lab, v);
}

bool TSOChecker::checkError4(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError4(lab))
		return true;

	return visitError4(lab);
}
bool TSOChecker::visitError5(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessError5_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool TSOChecker::visitLHSUnlessError5_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = free_succ(g, lab); pLab)
		if (true && llvm::isa<FreeLabel>(pLab) && !llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError5_0(pLab)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitLHSUnlessError5_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = alloc_pred(g, lab); pLab) {
		if (!visitLHSUnlessError5_1(pLab)) {
			return false;
		}
	}

	return true;
}

bool TSOChecker::visitUnlessError5(const EventLabel *lab) const
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

bool TSOChecker::checkError5(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError5(lab))
		return true;

	return visitError5(lab);
}
bool TSOChecker::visitError6(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessError6_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool TSOChecker::visitLHSUnlessError6_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	for (auto &tmp : alloc_succs(g, lab))
		if (auto *pLab = &tmp; true)
			if (true && llvm::isa<MemAccessLabel>(pLab) &&
			    llvm::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() &&
			    !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(pLab))) {
				if (!visitLHSUnlessError6_0(pLab, v)) {
					return false;
				}
			}

	return true;
}

bool TSOChecker::visitLHSUnlessError6_2(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab) {
			if (!visitLHSUnlessError6_1(pLab, v)) {
				return false;
			}
		}
	if (true && llvm::isa<HpRetireLabel>(lab))
		if (auto pLab = free_pred(g, lab); pLab)
			if (true && llvm::isa<MemAccessLabel>(pLab) &&
			    llvm::dyn_cast<MemAccessLabel>(pLab)->getAddr().isDynamic() &&
			    !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(pLab))) {
				if (!visitLHSUnlessError6_0(pLab, v)) {
					return false;
				}
			}

	return true;
}

bool TSOChecker::visitUnlessError6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError6Accepting.clear();
	visitedLHSUnlessError6Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true && visitLHSUnlessError6_2(lab, v);
}

bool TSOChecker::checkError6(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError6(lab))
		return true;

	return visitError6(lab);
}
bool TSOChecker::visitError7(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessError7_0(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	return false;

	return true;
}

bool TSOChecker::visitLHSUnlessError7_1(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (auto pLab = free_succ(g, lab); pLab)
		if (true && llvm::isa<HpRetireLabel>(pLab)) {
			if (!visitLHSUnlessError7_0(pLab)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitLHSUnlessError7_2(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<MemAccessLabel>(lab) &&
	    llvm::dyn_cast<MemAccessLabel>(lab)->getAddr().isDynamic() &&
	    !isHazptrProtected(llvm::dyn_cast<MemAccessLabel>(lab)))
		if (auto pLab = alloc_pred(g, lab); pLab) {
			if (!visitLHSUnlessError7_1(pLab)) {
				return false;
			}
		}

	return true;
}

bool TSOChecker::visitUnlessError7(const EventLabel *lab) const
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

bool TSOChecker::checkError7(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError7(lab))
		return true;

	return visitError7(lab);
}
bool TSOChecker::visitError8(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessError8_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool TSOChecker::visitLHSUnlessError8_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && lab->isNotAtomic() && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError8_0(pLab, v)) {
						return false;
					}
				}
	if (true && lab->isNotAtomic() && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<ReadLabel>(pLab)) {
					if (!visitLHSUnlessError8_0(pLab, v)) {
						return false;
					}
				}
	if (true && lab->isNotAtomic() && llvm::isa<ReadLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError8_0(pLab, v)) {
						return false;
					}
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError8_0(pLab, v)) {
						return false;
					}
				}
	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && llvm::isa<ReadLabel>(pLab)) {
					if (!visitLHSUnlessError8_0(pLab, v)) {
						return false;
					}
				}
	if (true && llvm::isa<ReadLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && pLab->isNotAtomic() && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessError8_0(pLab, v)) {
						return false;
					}
				}

	return true;
}

bool TSOChecker::visitUnlessError8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessError8Accepting.clear();
	visitedLHSUnlessError8Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true && visitLHSUnlessError8_1(lab, v);
}

bool TSOChecker::checkError8(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessError8(lab))
		return true;

	return visitError8(lab);
}
bool TSOChecker::visitWarning9(const EventLabel *lab) const { return false; }

bool TSOChecker::visitLHSUnlessWarning9_0(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (!v.contains(lab->getPos())) {
		cexLab = lab;
		return false;
	}

	return true;
}

bool TSOChecker::visitLHSUnlessWarning9_1(const EventLabel *lab, const View &v) const
{
	auto &g = *lab->getParent();

	if (true && llvm::isa<WriteLabel>(lab))
		for (auto &tmp : samelocs(g, lab))
			if (auto *pLab = &tmp; true)
				if (true && llvm::isa<WriteLabel>(pLab)) {
					if (!visitLHSUnlessWarning9_0(pLab, v)) {
						return false;
					}
				}

	return true;
}

bool TSOChecker::visitUnlessWarning9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	visitedLHSUnlessWarning9Accepting.clear();
	visitedLHSUnlessWarning9Accepting.resize(g.getMaxStamp().get() + 1, false);
	auto &v = lab->view(0);

	return true && visitLHSUnlessWarning9_1(lab, v);
}

bool TSOChecker::checkWarning9(const EventLabel *lab) const
{
	auto &g = *lab->getParent();

	if (visitUnlessWarning9(lab))
		return true;

	return visitWarning9(lab);
}
VerificationError TSOChecker::checkErrors(const EventLabel *lab, const EventLabel *&race) const
{
	if (!checkError2(lab)) {
		race = cexLab;
		return VerificationError::VE_AccessNonMalloc;
	}

	if (!checkError3(lab)) {
		race = cexLab;
		return VerificationError::VE_DoubleFree;
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
		return VerificationError::VE_AccessFreed;
	}

	if (!checkError8(lab)) {
		race = cexLab;
		return VerificationError::VE_RaceNotAtomic;
	}

	return VerificationError::VE_OK;
}

std::vector<VerificationError>
TSOChecker::checkWarnings(const EventLabel *lab, const VSet<VerificationError> &seenWarnings,
			  std::vector<const EventLabel *> &racyLabs) const
{
	std::vector<VerificationError> result;

	if (seenWarnings.count(VerificationError::VE_WWRace) == 0 && !checkWarning9(lab)) {
		racyLabs.push_back(cexLab);
		result.push_back(VerificationError::VE_WWRace);
	}

	return result;
}

bool TSOChecker::isConsistent(const EventLabel *lab) const
{

	return true && checkConsAcyclic1(lab);
}

bool TSOChecker::isConsistent(const ExecutionGraph &g) const
{

	return true && checkConsAcyclic1(g);
}

bool TSOChecker::isCoherentRelinche(const ExecutionGraph &g) const
{

	return true && visitCoherenceRelinche(g);
}

View TSOChecker::calcPPoRfBefore(const EventLabel *lab) const
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
std::unique_ptr<VectorClock> TSOChecker::calculatePrefixView(const EventLabel *lab) const
{
	return std::make_unique<View>(calcPPoRfBefore(lab));
}
