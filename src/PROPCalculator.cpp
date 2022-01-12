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

#include "PROPCalculator.hpp"

void PROPCalculator::initCalc()
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);

	/* Collect all atomic accesses and fences */
	auto events = g.collectAllEvents([&](const EventLabel *lab)
					     { return (PROPCalculator::isNonTrivial(lab) &&
						       !lab->isNotAtomic()) ||
						  llvm::isa<SmpFenceLabelLKMM>(lab) ||
						  llvm::isa<RCUSyncLabelLKMM>(lab); });

	/* Keep the fences separately because we will need them later */
	cumulFences.clear();
	strongFences.clear();
	locks.clear();
	for (auto &e : events) {
		auto *lab = g.getEventLabel(e);
		if (llvm::isa<WriteLabel>(lab) && lab->isAtLeastRelease())
			cumulFences.push_back(e);
		if (llvm::isa<WriteLabel>(lab) && lab->isSC())
			strongFences.push_back(e);

		if (llvm::isa<RCUSyncLabelLKMM>(lab))
			strongFences.push_back(e);

		auto *fLab = llvm::dyn_cast<SmpFenceLabelLKMM>(lab);
		if (fLab && fLab->isCumul())
			cumulFences.push_back(e);
		if (fLab && fLab->isStrong())
			strongFences.push_back(e);

		if (llvm::isa<LockCasWriteLabel>(lab) || llvm::isa<TrylockCasWriteLabel>(lab))
			locks.push_back(lab->getPos().prev());
	}
	events.erase(std::remove_if(events.begin(), events.end(), [&](Event e)
				    { return llvm::isa<FenceLabel>(g.getEventLabel(e)); }),
		events.end());

	/* Populate an initial PROP relation */
	prop = Calculator::GlobalRelation(std::move(events));
	return;
}

bool PROPCalculator::isCumulFenceBefore(Event a, Event b) const
{
	auto &g = getGraph();
	auto *labB = llvm::dyn_cast<MemAccessLabel>(g.getEventLabel(b));

	BUG_ON(!labB || llvm::isa<LockLabelLAPOR>(labB) || llvm::isa<UnlockLabelLAPOR>(labB));
	return labB->getFenceView().contains(a);
}

bool PROPCalculator::isPoUnlRfLockPoBefore(Event a, Event b) const
{
	auto &g = getGraph();

	for (auto &l1 : locks) {
		auto *lab1 = llvm::dyn_cast<CasReadLabel>(g.getEventLabel(l1));
		BUG_ON(!lab1);
		auto ul1 = g.getMatchingUnlock(l1);
		if (!a.isBetween(l1, ul1) || ul1.isInitializer())
			continue;
		for (auto &l2 : locks) {
			auto *lab2 = llvm::dyn_cast<CasReadLabel>(g.getEventLabel(l2));
			BUG_ON(!lab2);
			if (lab2->getAddr() != lab1->getAddr())
				continue;

			if (lab2->getHbView().contains(ul1) && b.thread == l2.thread && b.index > l2.index)
				return true;
		}
	}
	return false;
}

std::vector<Event> PROPCalculator::getExtOverwrites(Event e) const
{
	auto &g = getGraph();
	auto *lab = llvm::dyn_cast<MemAccessLabel>(g.getEventLabel(e));
	BUG_ON(!lab);
	auto &coRel = g.getPerLocRelation(ExecutionGraph::RelationId::co);
	auto &co = coRel[lab->getAddr()];
	auto &stores = co.getElems();
	std::vector<Event> owrs;

	auto from = e;
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
		from = rLab->getRf();

	if (from.isInitializer()) {
		std::copy_if(stores.begin(), stores.end(), std::back_inserter(owrs),
			     [&](Event s){ return s.thread != e.thread; });
	} else {
		std::copy_if(stores.begin(), stores.end(), std::back_inserter(owrs),
			     [&](Event s){ return co(from, s) && s.thread != e.thread; });
	}
	return owrs;
}

bool PROPCalculator::addConstraint(Event a, Event b)
{
	/* Do not consider identity edges */
	if (a == b)
		return false;

	auto &prop = getGraph().getGlobalRelation(ExecutionGraph::RelationId::prop);
	bool changed = !prop(a, b);
	prop.addEdge(a, b);
	return changed;
}

bool PROPCalculator::addPropConstraints()
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);
	auto &elems = prop.getElems();

	bool changed = false;
	for (auto e1 : elems) {
		/* First, add overwrite_e edges */
		auto owrs = getExtOverwrites(e1);
		for (auto o : owrs) {
			if (!g.getEventLabel(o)->isNotAtomic())
				changed |= addConstraint(e1, o);
		}

		/* Then, add cumul-fence; rfe? edges */
		for (auto e2 : elems) {
			if (e1 == e2)
				continue;
			if (isCumulFenceBefore(e1, e2) ||
			    std::any_of(owrs.begin(), owrs.end(), [&](Event o){ return isCumulFenceBefore(o, e2); })) {
				changed |= addConstraint(e1, e2);

				if (auto *wLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(e2))) {
					for (auto &rf : wLab->getReadersList()) {
						if (e2.thread != rf.thread) {
							if (!g.getEventLabel(rf)->isNotAtomic()) {
								changed |= addConstraint(e1, rf);
								changed |= addConstraint(e2, rf);
							}
						}
					}
				}
			}
			/* Add po-unlock-rf-lock-po edges */
			if (isPoUnlRfLockPoBefore(e1, e2) ||
			    std::any_of(owrs.begin(), owrs.end(), [&](Event o){ return isPoUnlRfLockPoBefore(o, e2); })) {
				changed |= addConstraint(e1, e2);

				if (auto *wLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(e2))) {
					for (auto &rf : wLab->getReadersList()) {
						if (e2.thread != rf.thread) {
							if (!g.getEventLabel(rf)->isNotAtomic()) {
								changed |= addConstraint(e1, rf);
								changed |= addConstraint(e2, rf);
							}
						}
					}
				}
			}
		}
		/* And also overwrite_e?; rfe edges*/
		owrs.push_back(e1); /* account for rfe */
		for (auto o : owrs) {
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(g.getEventLabel(o))) {
				for (auto &rf : wLab->getReadersList()) {
					if (o.thread != rf.thread) {
						if (!g.getEventLabel(rf)->isNotAtomic()) {
							changed |= addConstraint(e1, rf);
							if (!g.getEventLabel(o)->isNotAtomic())
								changed |= addConstraint(o, rf);
						}
					}
				}
			}
		}
	}
	return changed;
}

Calculator::CalculationResult PROPCalculator::doCalc()
{
	auto &g = getGraph();
	auto &prop = g.getGlobalRelation(ExecutionGraph::RelationId::prop);

	auto changed = addPropConstraints();
	return Calculator::CalculationResult(changed, prop.isIrreflexive());
}

void PROPCalculator::removeAfter(const VectorClock &preds)
{
}
