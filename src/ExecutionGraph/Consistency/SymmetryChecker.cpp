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

#include "ExecutionGraph/Consistency/SymmetryChecker.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"

static auto calcLargestSymmPrefixBeforeSR(int symm, const EventLabel *lab) -> int
{
	const auto &g = *lab->getParent();

	if (symm < 0 || symm >= g.getNumThreads())
		return -1;

	auto limit = std::min((long)lab->getIndex(), (long)g.getThreadSize(symm) - 1);
	for (auto j = 0; j < limit; j++) {
		const auto *labA = g.getEventLabel(Event(symm, j));
		const auto *labB = g.getEventLabel(Event(lab->getThread(), j));

		if (labA->getKind() != labB->getKind())
			return j - 1;
		if (const auto *rLabA = genmc::dyn_cast<ReadLabel>(labA)) {
			const auto *rLabB = genmc::dyn_cast<ReadLabel>(labB);
			if (rLabA->getRf()->getThread() == symm &&
			    rLabB->getRf()->getThread() == lab->getThread() &&
			    rLabA->getRf()->getIndex() == rLabB->getRf()->getIndex())
				continue;
			if (rLabA->getRf() != rLabB->getRf())
				return j - 1;
		}
		if (const auto *wLabA = genmc::dyn_cast<WriteLabel>(labA))
			if (!wLabA->isLocal())
				return j - 1;
	}
	return limit;
}

auto SymmetryChecker::sharePrefixSR(int symm, const EventLabel *lab) const -> bool
{
	return calcLargestSymmPrefixBeforeSR(symm, lab) == lab->getIndex();
}

auto SymmetryChecker::isEcoBefore(const EventLabel *lab, int tid) const -> bool
{
	const auto &g = *lab->getParent();
	if (!genmc::isa<MemAccessLabel>(lab))
		return false;

	auto symmPos = Event(tid, lab->getIndex());
	// if (auto *wLab = rf_pred(g, lab); wLab) {
	// 	return wLab.getPos() == symmPos;
	// }))
	// 	return true;
	if (std::ranges::any_of(g.co_succs(lab), [&](auto &sLab) {
		    return sLab.getPos() == symmPos ||
			   std::ranges::any_of(sLab.readers(), [&](auto &rLab) {
				   return rLab.getPos() == symmPos;
			   });
	    }))
		return true;
	if (std::ranges::any_of(g.fr_succs(lab), [&](auto &sLab) {
		    return sLab.getPos() == symmPos ||
			   std::ranges::any_of(sLab.readers(), [&](auto &rLab) {
				   return rLab.getPos() == symmPos;
			   });
	    }))
		return true;
	return false;
}

static auto isEcoSymmetric(const EventLabel *lab, int tid) -> bool
{
	const auto &g = *lab->getParent();

	const auto *symmLab = g.getEventLabel(Event(tid, lab->getIndex()));
	if (const auto *rLab = genmc::dyn_cast<ReadLabel>(lab)) {
		return rLab->getRf() == genmc::dyn_cast<ReadLabel>(symmLab)->getRf();
	}

	const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	BUG_ON(!wLab);
	return g.co_imm_succ(wLab) == genmc::dyn_cast<WriteLabel>(symmLab);
}

auto SymmetryChecker::isPredSymmetryOK(const EventLabel *lab, int symm) const -> bool
{
	const auto &g = *lab->getParent();

	BUG_ON(symm == -1);
	if (!sharePrefixSR(symm, lab) || !g.containsPos(Event(symm, lab->getIndex())))
		return true;

	const auto *symmLab = g.getEventLabel(Event(symm, lab->getIndex()));
	if (symmLab->getKind() != lab->getKind())
		return true;

	return !isEcoBefore(lab, symm);
}

auto SymmetryChecker::isPredSymmetryOK(const EventLabel *lab) const -> bool
{
	const auto &g = *lab->getParent();
	std::vector<int> preds;

	auto symm = g.getFirstThreadLabel(lab->getThread())->getSymmPredTid();
	while (symm != -1) {
		preds.push_back(symm);
		symm = g.getFirstThreadLabel(symm)->getSymmPredTid();
	}
	return std::ranges::all_of(preds, [&](auto &symm) { return isPredSymmetryOK(lab, symm); });
}

auto SymmetryChecker::isSuccSymmetryOK(const EventLabel *lab, int symm) const -> bool
{
	const auto &g = *lab->getParent();

	BUG_ON(symm == -1);
	if (!sharePrefixSR(symm, lab) || !g.containsPos(Event(symm, lab->getIndex())))
		return true;

	const auto *symmLab = g.getEventLabel(Event(symm, lab->getIndex()));
	if (symmLab->getKind() != lab->getKind())
		return true;

	return !isEcoBefore(symmLab, lab->getThread());
}

auto SymmetryChecker::isSuccSymmetryOK(const EventLabel *lab) const -> bool
{
	const auto &g = *lab->getParent();
	std::vector<int> succs;

	auto symm = g.getFirstThreadLabel(lab->getThread())->getSymmSuccTid();
	while (symm != -1) {
		succs.push_back(symm);
		symm = g.getFirstThreadLabel(symm)->getSymmSuccTid();
	}
	return std::ranges::all_of(succs, [&](auto &symm) { return isSuccSymmetryOK(lab, symm); });
}

auto SymmetryChecker::isSymmetryOK(const EventLabel *lab) const -> bool
{
	return isPredSymmetryOK(lab) && isSuccSymmetryOK(lab);
}

void SymmetryChecker::updatePrefixWithSymmetries(EventLabel *lab)
{
	auto &g = *lab->getParent();
	auto symm = g.getFirstThreadLabel(lab->getThread())->getSymmPredTid();
	if (symm == -1)
		return;

	auto &v = lab->getPrefixView();
	auto si = calcLargestSymmPrefixBeforeSR(symm, lab);
	auto *symmLab = g.getEventLabel({symm, si});

	/* It might be that symmlab doesn't have a prefix (ReadOptBlock optimization) */
	if (!genmc::isa<BlockLabel>(symmLab))
		v.update(symmLab->getPrefixView());
	if (auto *rLab = genmc::dyn_cast<ReadLabel>(symmLab)) {
		v.update(rLab->getRf()->getPrefixView());
	}
}
