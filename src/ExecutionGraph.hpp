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

#ifndef __EXECUTION_GRAPH_HPP__
#define __EXECUTION_GRAPH_HPP__

#include "Error.hpp"
#include "Event.hpp"
#include "EventLabel.hpp"
#include "Library.hpp"
#include "Matrix2D.hpp"
#include "ModOrder.hpp"
#include <llvm/ADT/StringMap.h>

#include <memory>

/*
 * ExecutionGraph class - This class represents an execution graph
 */
class ExecutionGraph {
public:
	std::vector<std::vector<std::unique_ptr<EventLabel> > > events;
	ModOrder modOrder;
	unsigned int timestamp;

	/* Constructors */
	ExecutionGraph();

	/* Basic getter methods */
	unsigned int nextStamp();
	const EventLabel *getEventLabel(Event e) const;
	const EventLabel *getPreviousLabel(Event e) const;
	const EventLabel *getPreviousLabel(const EventLabel *lab) const;
	const EventLabel *getLastThreadLabel(int thread) const;
	Event getLastThreadEvent(int thread) const;
	Event getLastThreadRelease(int thread, const llvm::GenericValue *addr) const;
	std::pair<std::vector<Event>, std::vector<Event> > getSCs();
	std::vector<const llvm::GenericValue *> getDoubleLocs();
	std::vector<Event> getPendingRMWs(const WriteLabel *sLab);
	Event getPendingLibRead(const LibReadLabel *lab);
	bool isWriteRfBefore(const View &before, Event e);
	bool isStoreReadByExclusiveRead(Event store, const llvm::GenericValue *ptr);
	bool isStoreReadBySettledRMW(Event store, const llvm::GenericValue *ptr, const View &porfBefore);

	/* Basic setter methods */
	const ReadLabel *addReadToGraph(int tid, llvm::AtomicOrdering ord,
					const llvm::GenericValue *ptr,
					const llvm::Type *typ, Event rf);

	const FaiReadLabel *addFaiReadToGraph(int tid, llvm::AtomicOrdering ord,
					      const llvm::GenericValue *ptr,
					      const llvm::Type *typ, Event rf,
					      llvm::AtomicRMWInst::BinOp op,
					      llvm::GenericValue &&opValue);

	const CasReadLabel *addCasReadToGraph(int tid, llvm::AtomicOrdering ord,
					      const llvm::GenericValue *ptr,
					      const llvm::Type *typ, Event rf,
					      const llvm::GenericValue &expected,
					      const llvm::GenericValue &swap,
					      bool isLock = false);

	const LibReadLabel *addLibReadToGraph(int tid, llvm::AtomicOrdering ord,
					      const llvm::GenericValue *ptr,
					      const llvm::Type *typ, Event rf,
					      std::string functionName);

	const WriteLabel *addStoreToGraph(int tid, llvm::AtomicOrdering ord,
					  const llvm::GenericValue *ptr,
					  const llvm::Type *typ,
					  const llvm::GenericValue &val,
					  int offsetMO, bool isUnlock = false);

	const FaiWriteLabel *addFaiStoreToGraph(int tid, llvm::AtomicOrdering ord,
						const llvm::GenericValue *ptr,
						const llvm::Type *typ,
						const llvm::GenericValue &val,
						int offsetMO);

	const CasWriteLabel *addCasStoreToGraph(int tid, llvm::AtomicOrdering ord,
						const llvm::GenericValue *ptr,
						const llvm::Type *typ,
						const llvm::GenericValue &val,
						int offsetMO, bool isLock = false);

	const LibWriteLabel *addLibStoreToGraph(int tid, llvm::AtomicOrdering ord,
						const llvm::GenericValue *ptr,
						const llvm::Type *typ,
						llvm::GenericValue &val,
						int offsetMO,
						std::string functionName,
						bool isInit);

	const FenceLabel *addFenceToGraph(int tid, llvm::AtomicOrdering ord);

	const MallocLabel *addMallocToGraph(int tid, const void *addr,
					    unsigned int size);

	const FreeLabel *addFreeToGraph(int tid, const void *addr,
					unsigned int size);

	const ThreadCreateLabel *addTCreateToGraph(int tid, int cid);

	const ThreadJoinLabel *addTJoinToGraph(int tid, int cid);

	const ThreadStartLabel *addStartToGraph(int tid, Event tc);

	const ThreadFinishLabel *addFinishToGraph(int tid);

	/* Calculation of [(po U rf)*] predecessors and successors */
	const View &getPorfBefore(Event e);
	const View &getHbBefore(Event e);
	const View &getHbPoBefore(Event e);
	View getHbBefore(const std::vector<Event> &es);
	View getHbRfBefore(const std::vector<Event> &es);
	View getPorfBeforeNoRfs(const std::vector<Event> &es);
	std::vector<Event> getMoOptRfAfter(const WriteLabel *sLab);

	Matrix2D<Event> calcWb(const llvm::GenericValue *addr);
	Matrix2D<Event> calcWbRestricted(const llvm::GenericValue *addr, const View &v);

	/* Calculation of particular sets of events/event labels */
	std::vector<Event> getStoresHbAfterStores(const llvm::GenericValue *loc,
						  const std::vector<Event> &chain);
	std::vector<std::unique_ptr<EventLabel> >
	getPrefixLabelsNotBefore(const View &prefix, const View &before);
	std::vector<Event> getRfsNotBefore(const std::vector<std::unique_ptr<EventLabel> > &labs,
					   const View &before);
	std::vector<std::pair<Event, Event> >
	getMOPredsInBefore(const std::vector<std::unique_ptr<EventLabel> > &labs,
			   const View &before);

	/* Calculation of loads that can be revisited */
	std::vector<Event> findOverwrittenBoundary(const llvm::GenericValue *addr, int thread);
	std::vector<Event> getRevisitable(const WriteLabel *sLab);

	/* Graph modification methods */
	void changeRf(Event read, Event store);
	bool updateJoin(Event join, Event childLast);

	View getViewFromStamp(unsigned int stamp);
	void cutToStamp(unsigned int stamp);
	void cutToView(const View &view);
	void restoreStorePrefix(const ReadLabel *rLab,
				std::vector<std::unique_ptr<EventLabel> > &storePrefix,
				std::vector<std::pair<Event, Event> > &moPlacings);

	bool revisitSetContains(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
				const std::vector<std::pair<Event, Event> > &moPlacings) const;
	void addToRevisitSet(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
			     const std::vector<std::pair<Event, Event> > &moPlacings);

	/* Consistency checks */
	bool isConsistent();
	bool isPscWeakAcyclicWB();
	bool isPscWbAcyclicWB();
	bool isPscAcyclicWB();
	bool isPscAcyclicMO();
	bool isWbAcyclic();

	/* Race detection methods */
	Event findRaceForNewLoad(const ReadLabel *rLab);
	Event findRaceForNewStore(const WriteLabel *wLab);

	/* Library consistency checks */
	std::vector<Event> getLibEventsInView(const Library &lib, const View &v);
	std::vector<Event> getLibConsRfsInView(const Library &lib, Event read,
					       const std::vector<Event> &stores,
					       const View &v);
	bool isLibConsistentInView(const Library &lib, const View &v);
	void addInvalidRfs(Event read, const std::vector<Event> &es);
	void addBottomToInvalidRfs(Event read);

	/* Debugging methods */
	void validate(void);

	/* Overloaded operators */
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ExecutionGraph &g);

protected:
	std::vector<unsigned int> calcRMWLimits(const Matrix2D<Event> &wb);
	View calcBasicHbView(const EventLabel *lab) const;
	View calcBasicPorfView(const EventLabel *lab) const;
	const EventLabel *addEventToGraph(std::unique_ptr<EventLabel> lab);
	const ReadLabel *addReadToGraphCommon(std::unique_ptr<ReadLabel> lab);
	const WriteLabel *addStoreToGraphCommon(std::unique_ptr<WriteLabel> lab);
	void calcPorfAfter(const Event e, View &a);
	void calcHbRfBefore(Event e, const llvm::GenericValue *addr, View &a);
	void calcRelRfPoBefore(const Event last, View &v);
	std::vector<Event> calcSCFencesSuccs(const std::vector<Event> &fcs, const Event e);
	std::vector<Event> calcSCFencesPreds(const std::vector<Event> &fcs, const Event e);
	std::vector<Event> calcSCSuccs(const std::vector<Event> &fcs, const Event e);
	std::vector<Event> calcSCPreds(const std::vector<Event> &fcs, const Event e);
	std::vector<Event> getSCRfSuccs(const std::vector<Event> &fcs, const Event e);
	std::vector<Event> getSCFenceRfSuccs(const std::vector<Event> &fcs, const Event e);
	bool isRMWLoad(const Event e);
	bool isRMWLoad(const EventLabel *lab);
	void spawnAllChildren(int thread);
	void addRbEdges(std::vector<Event> &fcs, std::vector<Event> &moAfter,
			std::vector<Event> &moRfAfter, Matrix2D<Event> &matrix, const Event &e);
	void addMoRfEdges(std::vector<Event> &fcs, std::vector<Event> &moAfter,
			  std::vector<Event> &moRfAfter, Matrix2D<Event> &matrix, const Event &e);
	void addInitEdges(std::vector<Event> &fcs, Matrix2D<Event> &matrix);
	void addSbHbEdges(Matrix2D<Event> &matrix);
	void addSCEcos(std::vector<Event> &fcs, std::vector<Event> &mo, Matrix2D<Event> &matrix);
	void addSCWbEcos(std::vector<Event> &fcs, Matrix2D<Event> &wbMatrix, Matrix2D<Event> &pscMatrix);

	void getPoEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
			    std::vector<Event> &tos);
	void getRfEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
			    std::vector<Event> &tos);
	void getHbEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
			    std::vector<Event> &tos);
	void getRfm1EdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
			      std::vector<Event> &tos);
	void getWbEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
			    std::vector<Event> &tos);
	void getMoEdgePairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
			    std::vector<Event> &tos);
	void calcSingleStepPairs(std::vector<std::pair<Event, std::vector<Event> > > &froms,
				 const std::string &step, std::vector<Event> &tos);
	void addStepEdgeToMatrix(std::vector<Event> &es,
				 Matrix2D<Event> &relMatrix,
				 const std::vector<std::string> &substeps);
	llvm::StringMap<Matrix2D<Event> >
	calculateAllRelations(const Library &lib, std::vector<Event> &es);
};

#endif /* __EXECUTION_GRAPH_HPP__ */
