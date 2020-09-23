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

#include "PersistencyChecker.hpp"

/************************************************************
 ** Helper functions
 ***********************************************************/

bool isDskAccessInRange(const DskAccessLabel *dLab, char *inode, unsigned int iSize)
{
	/* Disk fences are included in the range unconditionally */
	if (llvm::isa<FenceLabel>(dLab))
		return true;

	auto *mAddr = (char *) llvm::dyn_cast<MemAccessLabel>(dLab)->getAddr();
	BUG_ON(!llvm::isa<MemAccessLabel>(dLab));
	return inode <= mAddr && mAddr < inode + iSize;
}

bool writeSameBlock(const DskWriteLabel *dw1, const DskWriteLabel *dw2,
		    unsigned int blockSize)
{
	if (dw1->getMapping() != dw2->getMapping())
		return false;
	if (dw1->getKind() != dw2->getKind())
		return false;

	ptrdiff_t off1 = (char *) dw1->getAddr() - (char *) dw1->getMapping();
	ptrdiff_t off2 = (char *) dw2->getAddr() - (char *) dw2->getMapping();
	return (off1 / blockSize) == (off2 / blockSize);
}

std::vector<Event> getDskWriteOperations(const ExecutionGraph &g)
{
	auto recId = g.getRecoveryRoutineId();
	return g.collectAllEvents([&](const EventLabel *lab) {
			BUG_ON(llvm::isa<DskWriteLabel>(lab) &&
			       lab->getThread() == recId);
			return llvm::isa<DskWriteLabel>(lab);
		});
}


/************************************************************
 ** View calculation
 ***********************************************************/

/* TODO: Optimize */
void PersistencyChecker::calcMemAccessPbView(MemAccessLabel *mLab)
{
	auto &g = getGraph();
	auto &porf = mLab->getPorfView();
	auto &prefix = porf.empty() ? (VectorClock&) mLab->getPPoRfView() :
		(VectorClock&) porf;
	DepView pb;

	/* Check whether we are ordered wrt other writes */
	auto ordRange = std::make_pair((void *) nullptr, (void *) nullptr);
	if (auto *wLab = llvm::dyn_cast<DskMdWriteLabel>(mLab))
		ordRange = wLab->getOrdDataRange();

	BUG_ON(prefix.empty()); /* Must run after plain views calc */
	for (auto i = 0u; i < prefix.size(); i++) {
		auto lim = (i == mLab->getThread()) ? prefix[i] - 1 : prefix[i];
		for (auto j = 1u; j <= lim; j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (llvm::isa<FenceLabel>(lab)) {
				if (auto *dLab = llvm::dyn_cast<DskAccessLabel>(lab))
					pb.update(dLab->getPbView());
			}
			if (auto *jLab = llvm::dyn_cast<DskJnlWriteLabel>(lab))
				if (auto *wLab = llvm::dyn_cast<DskWriteLabel>(mLab))
					if (jLab->getTransInode() == wLab->getMapping())
						pb.update(jLab->getPbView());
			if (auto *oLab = llvm::dyn_cast<DskWriteLabel>(lab)) {
				if (oLab->getAddr() >= ordRange.first &&
				    oLab->getAddr() <  ordRange.second)
					pb.update(oLab->getPbView());
				if (auto *jLab = llvm::dyn_cast<DskJnlWriteLabel>(mLab))
					if (jLab->getTransInode() == oLab->getMapping())
						pb.update(oLab->getPbView());
			}
			if (auto *mdLab = llvm::dyn_cast<DskMdWriteLabel>(mLab))
				if (auto *dLab = llvm::dyn_cast<DskDirWriteLabel>(lab))
					pb.update(dLab->getPbView());
		}
	}

	auto prevIdx = pb[mLab->getThread()];
	pb[mLab->getThread()] = mLab->getIndex();
	pb.addHolesInRange(Event(mLab->getThread(), prevIdx + 1), mLab->getIndex());
	llvm::dyn_cast<DskAccessLabel>(mLab)->setPbView(std::move(pb));
	return;
}

/* TODO: Optimize */
void PersistencyChecker::calcFsyncPbView(DskFsyncLabel *fLab)
{
	auto &g = getGraph();
	auto *fInode = (char *) fLab->getInode();
	auto iSize = fLab->getSize();
	auto &hb = fLab->getHbView();
	DepView pb;

	BUG_ON(hb.empty()); /* Must run after plain views calc */
	for (auto i = 0u; i < hb.size(); i++) {
		auto lim = (i == fLab->getThread()) ? hb[i] - 1 : hb[i];
		for (auto j = 1u; j <= lim; j++) {
			const EventLabel *lab = g.getEventLabel(Event(i, j));
			if (auto *dLab = llvm::dyn_cast<DskAccessLabel>(lab)) {
				if (isDskAccessInRange(dLab, fInode, iSize))
					pb.update(dLab->getPbView());
			}
		}
	}
	auto prevIdx = pb[fLab->getThread()];
	pb[fLab->getThread()] = fLab->getIndex();
	pb.addHolesInRange(Event(fLab->getThread(), prevIdx + 1), fLab->getIndex());
	fLab->setPbView(std::move(pb));
	return;
}

void PersistencyChecker::calcSyncPbView(DskSyncLabel *fLab)
{
	auto &hb = fLab->getHbView();
	DepView pb;

	BUG_ON(hb.empty()); /* Must run after plain views calc */
	for (auto i = 0u; i < hb.size(); i++)
		pb[i] = hb[i];
	fLab->setPbView(std::move(pb));
	return;
}

void PersistencyChecker::calcPbarrierPbView(DskPbarrierLabel *fLab)
{
	auto &hb = fLab->getHbView();
	DepView pb;

	BUG_ON(hb.empty()); /* Must run after plain views calc */
	for (auto i = 0u; i < hb.size(); i++)
		pb[i] = hb[i];
	fLab->setPbView(std::move(pb));
	return;
}

/************************************************************
 ** PB calculation and recovery validity
 ***********************************************************/

void PersistencyChecker::calcPb()
{
	auto &g = getGraph();
	auto &pbRelation = getPbRelation();

	pbRelation = Calculator::GlobalRelation(getDskWriteOperations(g));

	/* Add all co edges to pb */
	g.getCoherenceCalculator()->initCalc();
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);
	for (auto &coLoc : coRelation) {
		/* We must only consider file operations */
		if (coLoc.second.empty() ||
		    !llvm::isa<DskWriteLabel>(g.getEventLabel(*coLoc.second.begin())))
			continue;
		for (auto &s1 : coLoc.second) {
			for (auto &s2 : coLoc.second) {
				if (coLoc.second(s1, s2))
					pbRelation.addEdge(s1, s2);
			}
		}
	}

	/* Add all sync + same-block edges
	 * FIXME: optimize */
	const auto &pbs = pbRelation.getElems();
	for (auto &d1 : pbs) {
		auto *lab1 = llvm::dyn_cast<DskWriteLabel>(g.getEventLabel(d1));
		BUG_ON(!lab1);
		for (auto &d2 : pbs) {
			if (d1 == d2)
				continue;
			auto *lab2 = llvm::dyn_cast<DskWriteLabel>(g.getEventLabel(d2));
			BUG_ON(!lab2);
			if (lab1->getPbView().contains(d2))
				pbRelation.addEdge(d2, d1);
			if (g.getHbBefore(d1).contains(d2)) {
				if (writeSameBlock(lab1, lab2, getBlockSize()))
					pbRelation.addEdge(d2, d1);
			}
		}

	}
	pbRelation.transClosure();
	BUG_ON(!pbRelation.isIrreflexive());
	return;
}

bool PersistencyChecker::isStoreReadFromRecRoutine(Event s)
{
	auto &g = getGraph();
	auto recId = g.getRecoveryRoutineId();
	auto recLast = g.getLastThreadEvent(recId);

	BUG_ON(!llvm::isa<WriteLabel>(g.getEventLabel(s)));
	auto *wLab = static_cast<const WriteLabel *>(g.getEventLabel(s));
	auto &readers = wLab->getReadersList();
	return std::any_of(readers.begin(), readers.end(), [&](Event r)
			   { return r.thread == recId &&
				    r.index <= recLast.index; });
}

bool PersistencyChecker::isRecFromReadValid(const DskReadLabel *rLab)
{
	auto &g = getGraph();

	/* co should already be calculated due to calcPb() */
	auto &pbRelation = getPbRelation();
	auto &coRelation = g.getPerLocRelation(ExecutionGraph::RelationId::co);
	auto &coLoc = coRelation[rLab->getAddr()];
	auto &stores = coLoc.getElems();

	/* Check the existence of an rb;pb?;rf;rec cycle */
	for (auto &s : stores) {
		if (!rLab->getRf().isInitializer() && !coLoc(rLab->getRf(), s))
			continue;

		/* check for rb;rf;rec cycle */
		if (isStoreReadFromRecRoutine(s))
			return false;

		/* check for rb;pb;rf;rec cycle */
		for (auto &p : pbRelation.getElems()) {
			/* only consider pb-later elements */
			if (!pbRelation(s, p))
				continue;

			auto *pLab = g.getEventLabel(p);
			if (auto *wLab = llvm::dyn_cast<WriteLabel>(pLab)) {
				if (isStoreReadFromRecRoutine(p))
					return false;
			}
		}
	}
	return true;
}

bool PersistencyChecker::isRecAcyclic()
{
	auto &g = getGraph();
	auto recId = g.getRecoveryRoutineId();
	BUG_ON(recId == -1);

	/* Calculate pb (and propagate co relation) */
	calcPb();

	for (auto j = 1u; j < g.getThreadSize(recId); j++) {
		const EventLabel *lab = g.getEventLabel(Event(recId, j));
		if (auto *rLab = llvm::dyn_cast<DskReadLabel>(lab)) {
			if (!isRecFromReadValid(rLab))
				return false;
		}
	}
	return true;
}
