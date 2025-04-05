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

#include "ExecutionGraph/GraphUtils.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/GraphIterators.hpp"
#include <algorithm>
#include <llvm/Support/Casting.h>
#include <ranges>

auto isHazptrProtected(const MemAccessLabel *mLab) -> bool
{
	auto &g = *mLab->getParent();
	BUG_ON(!mLab->getAddr().isDynamic());

	auto *aLab = mLab->getAlloc();
	BUG_ON(!aLab);

	auto mpreds = po_preds(g, mLab);
	auto pLabIt = std::ranges::find_if(mpreds, [&](auto &lab) {
		auto *pLab = llvm::dyn_cast<HpProtectLabel>(&lab);
		return pLab && aLab->contains(pLab->getProtectedAddr());
	});
	if (pLabIt == mpreds.end() || !llvm::isa<HpProtectLabel>(&*pLabIt))
		return false;

	auto *pLab = llvm::dyn_cast<HpProtectLabel>(&*pLabIt);
	for (auto &lab : std::ranges::subrange(std::ranges::begin(mpreds), pLabIt)) {
		if (auto *oLab = llvm::dyn_cast<HpProtectLabel>(&lab))
			if (oLab->getHpAddr() == pLab->getHpAddr())
				return false;
	}
	return true;
}

auto findMatchingLock(const UnlockWriteLabel *uLab) -> const CasWriteLabel *
{
	const auto &g = *uLab->getParent();
	std::vector<const UnlockWriteLabel *> locUnlocks;

	for (auto &lab : g.po_preds(uLab)) {

		/* In case support for reentrant locks is added... */
		if (auto *suLab = llvm::dyn_cast<UnlockWriteLabel>(&lab)) {
			if (suLab->getAddr() == uLab->getAddr())
				locUnlocks.push_back(suLab);
		}
		if (auto *lLab = llvm::dyn_cast<CasWriteLabel>(&lab)) {
			if ((llvm::isa<LockCasWriteLabel>(lLab) ||
			     llvm::isa<TrylockCasWriteLabel>(lLab)) &&
			    lLab->getAddr() == uLab->getAddr()) {
				if (locUnlocks.empty())
					return lLab;
				locUnlocks.pop_back();
			}
		}
	}
	return nullptr;
}

auto findMatchingUnlock(const CasWriteLabel *lLab) -> const UnlockWriteLabel *
{
	const auto &g = *lLab->getParent();
	std::vector<const CasWriteLabel *> locLocks;

	BUG_ON(!llvm::isa<LockCasReadLabel>(lLab) && !llvm::isa<TrylockCasReadLabel>(lLab));
	for (auto &lab : g.po_succs(lLab)) {
		/* skip next event */

		/* In case support for reentrant locks is added... */
		if (auto *slLab = llvm::dyn_cast<CasWriteLabel>(&lab)) {
			if ((llvm::isa<LockCasWriteLabel>(slLab) ||
			     llvm::isa<TrylockCasWriteLabel>(slLab)) &&
			    slLab->getAddr() == lLab->getAddr())
				locLocks.push_back(slLab);
		}
		if (auto *uLab = llvm::dyn_cast<UnlockWriteLabel>(&lab)) {
			if (uLab->getAddr() == lLab->getAddr()) {
				if (locLocks.empty())
					return uLab;
				locLocks.pop_back();
			}
		}
	}
	return nullptr;
}

auto findMatchingSpeculativeRead(const ReadLabel *cLab, const EventLabel *&scLab)
	-> const SpeculativeReadLabel *
{
	const auto &g = *cLab->getParent();
	for (auto &lab : g.po_preds(cLab)) {

		if (lab.isSC())
			scLab = &lab;

		/* We don't care whether all previous confirmations are matched;
		 * the same speculation maybe confirmed multiple times (e.g., baskets) */
		if (auto *rLab = llvm::dyn_cast<SpeculativeReadLabel>(&lab)) {
			if (rLab->getAddr() == cLab->getAddr())
				return rLab;
		}
	}
	return nullptr;
}

auto findAllocatingLabel(const ExecutionGraph &g, const SAddr &addr) -> const MallocLabel *
{
	/* Don't iterate over the graph if you don't have to */
	if (!addr.isDynamic())
		return nullptr;

	auto labels = g.labels();
	auto labIt = std::ranges::find_if(g.labels(), [addr](auto &lab) {
		auto *mLab = llvm::dyn_cast<MallocLabel>(&lab);
		return mLab && mLab->contains(addr);
	});
	return (labIt != std::ranges::end(labels)) ? llvm::dyn_cast<MallocLabel>(&*labIt) : nullptr;
}

auto findAllocatingLabel(ExecutionGraph &g, const SAddr &addr) -> MallocLabel *
{
	return const_cast<MallocLabel *>(
		findAllocatingLabel(static_cast<const ExecutionGraph &>(g), addr));
}

static auto getMinimumStampLabel(const std::vector<const WriteLabel *> &labs) -> const WriteLabel *
{
	auto minIt = std::ranges::min_element(
		labs, [&](auto &lab1, auto &lab2) { return lab1->getStamp() < lab2->getStamp(); });
	return minIt == std::ranges::end(labs) ? nullptr : *minIt;
}

auto findPendingRMW(const WriteLabel *sLab) -> const WriteLabel *
{
	if (!sLab->isRMW())
		return nullptr;

	const auto &g = *sLab->getParent();
	const auto *pLab = llvm::dyn_cast<ReadLabel>(g.po_imm_pred(sLab));
	BUG_ON(!pLab->getRf());
	std::vector<const WriteLabel *> pending;

	/* Fastpath: non-init rf */
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(pLab->getRf())) {
		for (auto &rLab : wLab->readers()) {
			if (rLab.isRMW() && &rLab != pLab)
				pending.push_back(llvm::dyn_cast<WriteLabel>(g.po_imm_succ(&rLab)));
		}
		return getMinimumStampLabel(pending);
	}

	/* Slowpath: scan init rfs */
	std::for_each(
		g.init_rf_begin(pLab->getAddr()), g.init_rf_end(pLab->getAddr()), [&](auto &rLab) {
			if (rLab.getRf() == pLab->getRf() && &rLab != pLab && rLab.isRMW())
				pending.push_back(llvm::dyn_cast<WriteLabel>(g.po_imm_succ(&rLab)));
		});
	return getMinimumStampLabel(pending);
}

auto findPendingRMW(WriteLabel *sLab) -> WriteLabel *
{
	return const_cast<WriteLabel *>(findPendingRMW(static_cast<const WriteLabel *>(sLab)));
}

auto findBarrierInitValue(const ExecutionGraph &g, const AAccess &access) -> SVal
{
	auto sIt = std::find_if(g.co_begin(access.getAddr()), g.co_end(access.getAddr()),
				[&access, &g](auto &bLab) {
					BUG_ON(!llvm::isa<WriteLabel>(bLab));
					return bLab.getAddr() == access.getAddr() &&
					       bLab.isNotAtomic();
				});

	/* All errors pertinent to initialization should be captured elsewhere */
	BUG_ON(sIt == g.co_end(access.getAddr()));
	return sIt->getAccessValue(access);
}

auto violatesAtomicity(const WriteLabel *sLab) -> bool
{
	return sLab->isRMW() && findPendingRMW(sLab);
}
