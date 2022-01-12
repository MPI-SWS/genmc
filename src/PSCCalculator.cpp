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

#include "PSCCalculator.hpp"
#include "Error.hpp"
#include "ExecutionGraph.hpp"
#include "WBCalculator.hpp"

std::vector<SAddr> PSCCalculator::getDoubleLocs() const
{
	auto &g = getGraph();
	std::vector<SAddr> singles, doubles;

	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 1u; j < g.getThreadSize(i); j++) { /* Do not consider thread inits */
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (!llvm::isa<MemAccessLabel>(lab))
				continue;

			auto *mLab = static_cast<const MemAccessLabel *>(lab);
			if (std::find(doubles.begin(), doubles.end(),
				      mLab->getAddr()) != doubles.end())
				continue;
			if (std::find(singles.begin(), singles.end(),
				      mLab->getAddr()) != singles.end()) {
				singles.erase(std::remove(singles.begin(),
							  singles.end(),
							  mLab->getAddr()),
					      singles.end());
				doubles.push_back(mLab->getAddr());
			} else {
				singles.push_back(mLab->getAddr());
			}
		}
	}
	return doubles;
}

std::vector<Event> PSCCalculator::calcSCFencesSuccs(const std::vector<Event> &fcs,
						     const Event e) const
{
	auto &g = getGraph();
	auto &hbRelation = g.getGlobalRelation(ExecutionGraph::RelationId::hb);
	std::vector<Event> succs;

	if (g.isRMWLoad(e))
		return succs;
	for (auto &f : fcs) {
		if (hbRelation(e, f))
			succs.push_back(f);
	}
	return succs;
}

std::vector<Event> PSCCalculator::calcSCFencesPreds(const std::vector<Event> &fcs,
						     const Event e) const
{
	auto &g = getGraph();
	auto &hbRelation = g.getGlobalRelation(ExecutionGraph::RelationId::hb);
	std::vector<Event> preds;

	if (g.isRMWLoad(e))
		return preds;
	for (auto &f : fcs) {
		if (hbRelation(f, e))
			preds.push_back(f);
	}
	return preds;
}

std::vector<Event> PSCCalculator::calcSCSuccs(const std::vector<Event> &fcs,
					       const Event e) const
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(e);

	if (g.isRMWLoad(lab))
		return {};
	if (lab->isSC())
		return {e};
	else
		return calcSCFencesSuccs(fcs, e);
}

std::vector<Event> PSCCalculator::calcSCPreds(const std::vector<Event> &fcs,
					       const Event e) const
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(e);

	if (g.isRMWLoad(lab))
		return {};
	if (lab->isSC())
		return {e};
	else
		return calcSCFencesPreds(fcs, e);
}

std::vector<Event> PSCCalculator::calcRfSCSuccs(const std::vector<Event> &fcs,
						 const Event ev) const
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(ev);
	std::vector<Event> rfs;

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto succs = calcSCSuccs(fcs, e);
		rfs.insert(rfs.end(), succs.begin(), succs.end());
	}
	return rfs;
}

std::vector<Event> PSCCalculator::calcRfSCFencesSuccs(const std::vector<Event> &fcs,
						       const Event ev) const
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(ev);
	std::vector<Event> fenceRfs;

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto fenceSuccs = calcSCFencesSuccs(fcs, e);
		fenceRfs.insert(fenceRfs.end(), fenceSuccs.begin(), fenceSuccs.end());
	}
	return fenceRfs;
}

void PSCCalculator::addRbEdges(const std::vector<Event> &fcs,
				const std::vector<Event> &moAfter,
				const std::vector<Event> &moRfAfter,
				Calculator::GlobalRelation &matrix,
				const Event &ev) const
{
	auto &g = getGraph();
	const EventLabel *lab = g.getEventLabel(ev);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (const auto &e : wLab->getReadersList()) {
		auto preds = calcSCPreds(fcs, e);
		auto fencePreds = calcSCFencesPreds(fcs, e);

		matrix.addEdgesFromTo(preds, moAfter);        /* Base/fence: Adds rb-edges */
		matrix.addEdgesFromTo(fencePreds, moRfAfter); /* Fence: Adds (rb;rf)-edges */
	}
	return;
}

void PSCCalculator::addMoRfEdges(const std::vector<Event> &fcs,
				  const std::vector<Event> &moAfter,
				  const std::vector<Event> &moRfAfter,
				  Calculator::GlobalRelation &matrix,
				  const Event &ev) const
{
	auto &g = getGraph();
	auto preds = calcSCPreds(fcs, ev);
	auto fencePreds = calcSCFencesPreds(fcs, ev);
	auto rfs = calcRfSCSuccs(fcs, ev);

	matrix.addEdgesFromTo(preds, moAfter);        /* Base/fence:  Adds mo-edges */
	matrix.addEdgesFromTo(preds, rfs);            /* Base/fence:  Adds rf-edges (hb_loc) */
	matrix.addEdgesFromTo(fencePreds, moRfAfter); /* Fence:       Adds (mo;rf)-edges */
	return;
}

/*
 * addSCEcosLoc - Helper function that calculates a part of PSC_base and PSC_fence
 *
 * For PSC_base and PSC_fence, it adds co, rb, and hb_loc edges. The
 * procedure for co and rb is straightforward: at each point, we only
 * need to keep a list of all the co-after writes that are either SC,
 * or can reach an SC fence. For hb_loc, however, we only consider
 * rf-edges because the other cases are implicitly covered (sb, co, etc).
 *
 * For PSC_fence only, it adds (co;rf)- and (rb;rf)-edges. Simple cases like
 * co, rf, and rb are covered by PSC_base, and all other combinations with
 * more than one step either do not compose, or lead to an already added
 * single-step relation (e.g, (rf;rb) => co, (rb;co) => rb)
 */
void PSCCalculator::addSCEcosLoc(const std::vector<Event> &fcs,
				 Calculator::GlobalRelation &coMatrix,
				 Calculator::GlobalRelation &pscMatrix) const
{
	auto &stores = coMatrix.getElems();
	for (auto i = 0u; i < stores.size(); i++) {

		/*
		 * Calculate which of the stores are co-after the current
		 * write, and then collect co-after and (co;rf)-after SC successors
		 */
		std::vector<Event> coAfter, coRfAfter;
		for (auto j = 0u; j < stores.size(); j++) {
			if (coMatrix(i, j)) {
				auto succs = calcSCSuccs(fcs, stores[j]);
				auto fenceRfs = calcRfSCFencesSuccs(fcs, stores[j]);
				coAfter.insert(coAfter.end(), succs.begin(), succs.end());
				coRfAfter.insert(coRfAfter.end(), fenceRfs.begin(), fenceRfs.end());
			}
		}

		/* Then, add the proper edges to PSC using co-after and (co;rf)-after successors */
		addRbEdges(fcs, coAfter, coRfAfter, pscMatrix, stores[i]);
		addMoRfEdges(fcs, coAfter, coRfAfter, pscMatrix, stores[i]);
	}
}

/*
 * Adds sb as well as [Esc];sb_(<>loc);hb;sb_(<>loc);[Esc] edges. The first
 * part of this function is common for PSC_base and PSC_fence, while the second
 * part of this function is not triggered for fences (these edges are covered in
 * addSCEcos()).
 */
void PSCCalculator::addSbHbEdges(Calculator::GlobalRelation &matrix) const
{
	auto &g = getGraph();
	auto &hbRelation = g.getGlobalRelation(ExecutionGraph::RelationId::hb);

	auto &scs = matrix.getElems();
	for (auto i = 0u; i < scs.size(); i++) {
		for (auto j = 0u; j < scs.size(); j++) {
			if (i == j)
				continue;
			const EventLabel *eiLab = g.getEventLabel(scs[i]);
			const EventLabel *ejLab = g.getEventLabel(scs[j]);

			/* PSC_base/PSC_fence: Adds sb-edges*/
			if (eiLab->getThread() == ejLab->getThread()) {
				if (eiLab->getIndex() < ejLab->getIndex())
					matrix.addEdge(i, j);
				continue;
			}

			/* PSC_base: Adds [Esc];sb_(<>loc);hb;sb_(<>loc);[Esc] edges.
			 * We do need to consider the [Fsc];hb? cases, since these
			 * will be covered by addSCEcos(). (More speficically, from
			 * the rf/hb_loc case in addMoRfEdges().)  */
			const EventLabel *ejPrevLab = g.getPreviousNonEmptyLabel(ejLab);
			if (!llvm::isa<MemAccessLabel>(ejPrevLab) ||
			    !llvm::isa<MemAccessLabel>(ejLab) ||
			    !llvm::isa<MemAccessLabel>(eiLab))
				continue;

			if (eiLab->getPos() == g.getLastThreadEvent(eiLab->getThread()))
				continue;

			auto *ejPrevMLab = static_cast<const MemAccessLabel *>(ejPrevLab);
			auto *ejMLab = static_cast<const MemAccessLabel *>(ejLab);
			auto *eiMLab = static_cast<const MemAccessLabel *>(eiLab);

			if (ejPrevMLab->getAddr() != ejMLab->getAddr()) {
				Event next = eiMLab->getPos().next();
				const EventLabel *eiNextLab = g.getEventLabel(next);
				if (auto *eiNextMLab =
				    llvm::dyn_cast<MemAccessLabel>(eiNextLab)) {
					if (eiMLab->getAddr() != eiNextMLab->getAddr() &&
					    hbRelation(eiNextMLab->getPos(), ejPrevMLab->getPos()))
						matrix.addEdge(i, j);
				}
			}
		}
	}
	return;
}

void PSCCalculator::addInitEdges(const std::vector<Event> &fcs,
				  Calculator::GlobalRelation &matrix) const
{
	auto &g = getGraph();
	for (auto i = 0u; i < g.getNumThreads(); i++) {
		for (auto j = 0u; j < g.getThreadSize(i); j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			/* Consider only reads that read from the initializer write */
			if (!llvm::isa<ReadLabel>(lab) || g.isRMWLoad(lab))
				continue;
			auto *rLab = static_cast<const ReadLabel *>(lab);
			if (!rLab->getRf().isInitializer())
				continue;

			auto preds = calcSCPreds(fcs, rLab->getPos());
			auto fencePreds = calcSCFencesPreds(fcs, rLab->getPos());
			for (auto &w : g.getStoresToLoc(rLab->getAddr())) {
				/* Can be casted to WriteLabel by construction */
				auto *wLab = static_cast<const WriteLabel *>(
					g.getEventLabel(w));
				auto wSuccs = calcSCSuccs(fcs, w);
				matrix.addEdgesFromTo(preds, wSuccs); /* Adds rb-edges */
				for (auto &r : wLab->getReadersList()) {
					auto fenceSuccs = calcSCFencesSuccs(fcs, r);
					matrix.addEdgesFromTo(fencePreds, fenceSuccs); /*Adds (rb;rf)-edges */
				}
			}
		}
	}
	return;
}

void PSCCalculator::addSCEcos(const std::vector<Event> &fcs,
			      const std::vector<SAddr> &scLocs,
			      Calculator::GlobalRelation &matrix) const
{
	auto &g = getGraph();
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);

	for (auto loc : scLocs)
		addSCEcosLoc(fcs, coRelation[loc], matrix);
	return;
}

void PSCCalculator::calcPscRelation()
{
	auto &g = getGraph();
	auto &pscRelation = g.getGlobalRelation(ExecutionGraph::RelationId::psc);

	/* Collect all SC events (except for RMW loads) */
	auto accesses = g.getSCs();
	auto &scs = accesses.first;
	auto &fcs = accesses.second;

	/* If there are no SC events, it is a valid execution */
	if (scs.empty())
		return;

	/* Add edges from the initializer write (special case) */
	addInitEdges(fcs, pscRelation);
	/* Add sb and sb_(<>loc);hb;sb_(<>loc) edges (+ Fsc;hb;Fsc) */
	addSbHbEdges(pscRelation);

	/*
	 * Collect memory locations with more than one SC accesses
	 * and add the rest of PSC_base and PSC_fence
	 */
	addSCEcos(fcs, getDoubleLocs(), pscRelation);
	pscRelation.transClosure();
	return;
}

Calculator::CalculationResult PSCCalculator::addPscConstraints()
{
	auto &g = getGraph();
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);
	auto &pscRelation = g.getGlobalRelation(ExecutionGraph::RelationId::psc);
	Calculator::CalculationResult result;

	if (auto *wbCoh = llvm::dyn_cast<WBCalculator>(
		    g.getCoherenceCalculator())) {
		for (auto &coLoc : coRelation)
			result |= wbCoh->calcWbRelation(coLoc.first, coLoc.second,
							pscRelation, [&](Event e)
							{ return g.getEventLabel(e)->isSC() &&
								 !g.isRMWLoad(e); });
	}
	return result;
}

void PSCCalculator::initCalc()
{
	auto &g = getGraph();
	auto &pscRelation = g.getGlobalRelation(ExecutionGraph::RelationId::psc);

	/* Collect all SC events (except for RMW loads) */
	auto accesses = getGraph().getSCs();

	pscRelation = Calculator::GlobalRelation(accesses.first);
	return;
}

Calculator::CalculationResult PSCCalculator::doCalc()
{
	auto &g = getGraph();
	auto &hbRelation = g.getGlobalRelation(ExecutionGraph::RelationId::hb);
	auto &pscRelation = g.getGlobalRelation(ExecutionGraph::RelationId::psc);
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);

	hbRelation.transClosure();
	if (!hbRelation.isIrreflexive())
		return Calculator::CalculationResult(false, false);
	calcPscRelation();
	if (!pscRelation.isIrreflexive())
		return Calculator::CalculationResult(false, false);

	auto result = addPscConstraints();
	if (!result.cons)
		return Calculator::CalculationResult(result.changed, false);
	for (auto &coLoc : coRelation)
		coLoc.second.transClosure();

	/* Check that co is acyclic */
	for (auto &coLoc : coRelation) {
		if (!coLoc.second.isIrreflexive())
		return Calculator::CalculationResult(result.changed, false);
	}
	return Calculator::CalculationResult(result.changed, true);
}

void PSCCalculator::removeAfter(const VectorClock &preds)
{
	/* We do not track anything specific for PSC */
	return;
}
