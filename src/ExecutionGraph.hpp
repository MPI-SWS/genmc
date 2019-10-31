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

#include "DriverGraphEnumAPI.hpp"
#include "DepInfo.hpp"
#include "Error.hpp"
#include "Event.hpp"
#include "EventLabel.hpp"
#include "Library.hpp"
#include "Matrix2D.hpp"
#include "VectorClock.hpp"
#include <llvm/ADT/StringMap.h>

#include <memory>

class CoherenceCalculator;

/*******************************************************************************
 **                           ExecutionGraph Class
 ******************************************************************************/

/*
 * An class representing plain execution graphs. This class offers
 * the basic infrastructure all types of graphs should provide (e.g.,
 * calculation of hb-predecessors, psc, etc). More specialized types
 * of graphs can provide extra functionality (e.g., take dependencies
 * into account when restricting a graph).
 */
class ExecutionGraph {

private:
	using Thread = std::vector<std::unique_ptr<EventLabel> >;
	using Graph = std::vector<Thread>;

public:
	/* Should be used for the contruction of execution graphs */
	class Builder;

protected:
	/* Constructor should only be called from the builder */
	friend class GraphBuilder;
	ExecutionGraph();

public:
	/* Iterators */
	using iterator = Graph::iterator;
	using const_iterator = Graph::const_iterator;
	using reverse_iterator = Graph::reverse_iterator;
	using const_reverse_iterator = Graph::const_reverse_iterator;

	iterator begin() { return events.begin(); };
	iterator end() { return events.end(); };
	const_iterator begin() const { return events.begin(); };
	const_iterator end() const { return events.end(); };

	reverse_iterator rbegin() { return events.rbegin(); };
	reverse_iterator rend()   { return events.rend(); };
	const_reverse_iterator rbegin() const { return events.rbegin(); };
	const_reverse_iterator rend()   const { return events.rend(); };


	/* Modification order methods */
	void trackCoherenceAtLoc(const llvm::GenericValue *addr);
	const std::vector<Event>& getStoresToLoc(const llvm::GenericValue *addr) const;
	const std::vector<Event>& getStoresToLoc(const llvm::GenericValue *addr);
	std::vector<Event> getCoherentStores(const llvm::GenericValue *addr,
					     Event pos);
	std::pair<int, int> getCoherentPlacings(const llvm::GenericValue *addr,
						Event pos, bool isRMW);
	std::vector<Event> getCoherentRevisits(const WriteLabel *wLab);


	/* Thread-related methods */

	/* Creates a new thread in the execution graph */
	inline void addNewThread() { events.push_back({}); };

	/* Returns the number of threads currently in the graph */
	inline unsigned int getNumThreads() const { return events.size(); };

	/* Returns the size of the thread tid */
	inline unsigned int getThreadSize(int tid) const { return events[tid].size(); };

	/* Returns true if the thread tid is empty */
	inline bool isThreadEmpty(int tid) const { return getThreadSize(tid) == 0; };


	/* Event addition methods */

	/* Returns the next available stamp (and increases the counter) */
	unsigned int nextStamp();

	const ReadLabel *addReadLabelToGraph(std::unique_ptr<ReadLabel> lab,
					     Event rf);
	const WriteLabel *addWriteLabelToGraph(std::unique_ptr<WriteLabel> lab,
					       unsigned int offsetMO);
	const WriteLabel *addWriteLabelToGraph(std::unique_ptr<WriteLabel> lab,
					       Event pred);
	const EventLabel *addOtherLabelToGraph(std::unique_ptr<EventLabel> lab);


	/* Event getter methods */

	/* Returns the label in the position denoted by event e */
	const EventLabel *getEventLabel(Event e) const;
	EventLabel *getEventLabel(Event e) {
		return events[e.thread][e.index].get();
	};

	/* Returns the label in the previous position of e.
	 * Does _not_ perform any out-of-bounds checks */
	const EventLabel *getPreviousLabel(Event e) const;
	const EventLabel *getPreviousLabel(const EventLabel *lab) const;

	/* Returns the previous non-empty label of e. Since all threads
	 * have an initializing event, it returns that as a base case */
	const EventLabel *getPreviousNonEmptyLabel(Event e) const;
	const EventLabel *getPreviousNonEmptyLabel(const EventLabel *lab) const;

	/* Returns the last label in the thread tid */
	const EventLabel *getLastThreadLabel(int tid) const;

	/* Returns the last event in the thread tid */
	Event getLastThreadEvent(int tid) const;

	/* Returns the last release before upperLimit in the latter's thread.
	 * If it's not a fence, then it has to be at location addr */
	Event getLastThreadReleaseAtLoc(Event upperLimit,
					const llvm::GenericValue *addr) const;

	/* Returns the last release before upperLimit in the latter's thread */
	Event getLastThreadRelease(Event upperLimit) const;

	/* Returns a list of acquire (R or F) in upperLimit's thread (before it) */
	std::vector<Event> getThreadAcquiresAndFences(const Event upperLimit) const;

	/* Returns pair with all SC accesses and all SC fences */
	std::pair<std::vector<Event>, std::vector<Event> > getSCs() const;

	/* Returns a list with all accesses that are accessed at least twice */
	std::vector<const llvm::GenericValue *> getDoubleLocs() const;

	/* Given an write label sLab that is part of an RMW, return all
	 * other RMWs that read from the same write. Of course, there must
	 * be _at most_ one other RMW reading from the same write (see [Rex] set) */
	std::vector<Event> getPendingRMWs(const WriteLabel *sLab) const;

	/* Similar to getPendingRMWs() but for libraries (w/ functional RF) */
	Event getPendingLibRead(const LibReadLabel *lab) const;

	/* Returns a list of stores that access location loc, not part chain,
	 * that are hb-after some store in chain */
	std::vector<Event> getStoresHbAfterStores(const llvm::GenericValue *loc,
						  const std::vector<Event> &chain) const;

	virtual std::unique_ptr<VectorClock> getRevisitView(const ReadLabel *rLab,
							    const WriteLabel *wLab) const;


	/* Calculation of [(po U rf)*] predecessors and successors */
	const DepView &getPPoRfBefore(Event e) const;
	const View &getPorfBefore(Event e) const;
	const View &getHbBefore(Event e) const;
	const View &getHbPoBefore(Event e) const;
	View getHbBefore(const std::vector<Event> &es) const;
	View getHbRfBefore(const std::vector<Event> &es) const;
	View getPorfBeforeNoRfs(const std::vector<Event> &es) const;
	std::vector<Event> getInitRfsAtLoc(const llvm::GenericValue *addr) const;


	/* Boolean helper functions */

	/* Returns true if e is hb-before w, or any of the reads that read from w */
	bool isHbOptRfBefore(const Event e, const Event write) const;
	bool isHbOptRfBeforeInView(const Event e, const Event write,
				   const VectorClock &v) const;

	/* Returns true if a (or any of the reads reading from a) is hb-before b */
	bool isWriteRfBefore(Event a, Event b) const;

	/* Returns true if store is read a successful RMW in the location ptr */
	bool isStoreReadByExclusiveRead(Event store, const llvm::GenericValue *ptr) const;

	/* Returns true if store is read by a successful RMW that is either non
	 * revisitable, or in the view porfBefore */
	bool isStoreReadBySettledRMW(Event store, const llvm::GenericValue *ptr,
				     const VectorClock &porfBefore) const;

	/* Returns true if the graph that will be created when sLab revisits rLab
	 * will be the same as the current one */
	virtual bool revisitModifiesGraph(const ReadLabel *rLab,
					  const EventLabel *sLab) const;


	/* Consistency checks */

	/* Returns true if the current graph is consistent */
	bool isConsistent() const;

	/* Checks whether the provided condition "cond" holds for PSC.
	 * The calculation type (e.g., weak, full, etc) is determined by "t" */
	template <typename F>
	bool checkPscCondition(CheckPSCType t, F cond) const;

	/* Returns true if PSC is acyclic */
	bool isPscAcyclic(CheckPSCType t) const;


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


	/* Graph modification methods */

	void changeRf(Event read, Event store);
	void changeStoreOffset(const llvm::GenericValue *addr, Event s,
			       int newOffset);
	void resetJoin(Event join);
	bool updateJoin(Event join, Event childLast);


	/* Revisit set methods */

	/* Returns true if the revisit set for rLab contains the pair
	 * <writePrefix, moPlacings>*/
	bool revisitSetContains(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
				const std::vector<std::pair<Event, Event> > &moPlacings) const;

	/* Adds to the revisit set of rLab the pair <writePrefix, moPlacings> */
	void addToRevisitSet(const ReadLabel *rLab, const std::vector<Event> &writePrefix,
			     const std::vector<std::pair<Event, Event> > &moPlacings);

	/* Returns a list of loads that can be revisited */
	virtual std::vector<Event> getRevisitable(const WriteLabel *sLab) const;


	/* Prefix saving and restoring */

	/* Returns a vector clock representing the prefix of e.
	 * Depending on whether dependencies are tracked, the prefix can be
	 * either (po U rf) or (AR U rf) */
	virtual const VectorClock& getPrefixView(Event e) const;

	/* Returns a vector clock representing the events added before e */
	virtual std::unique_ptr<VectorClock> getPredsView(Event e) const;

	/* Saves the prefix of sLab that is not before rLab.
	 * (Because of functional libraries, sLab can be a ReadLabel as well.) */
	virtual std::vector<std::unique_ptr<EventLabel> >
	getPrefixLabelsNotBefore(const EventLabel *sLab, const ReadLabel *rLab) const;

	/* Returns a list of the rfs of the reads in labs */
	std::vector<Event>
	extractRfs(const std::vector<std::unique_ptr<EventLabel> > &labs) const;

	/* Returns pairs of the form <store, pred> where store is a write from labs,
	 * and pred is an mo-before store that was added before rLab */
	std::vector<std::pair<Event, Event> >
	saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
			    const ReadLabel *rLab) const;

	/* Restores the prefix stored in storePrefix (for revisiting rLab) and
	 * also the moPlacings of the above prefix */
	void restoreStorePrefix(const ReadLabel *rLab,
				std::vector<std::unique_ptr<EventLabel> > &storePrefix,
				std::vector<std::pair<Event, Event> > &moPlacings);

	/* Graph cutting */

	/* Returns a view of the graph representing events with stamp <= st */
	View getViewFromStamp(unsigned int st) const;

	/* Simmilar to getViewFromStamp() but returns a DepView */
	DepView getDepViewFromStamp(unsigned int st) const;

	/* Cuts a graph so that it only contains events with stamp <= st */
	virtual void cutToStamp(unsigned int st);

	/* Overloaded operators */
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ExecutionGraph &g);

protected:
	/* Returns a reference to the graph's coherence calculator */
	CoherenceCalculator *getCoherenceCalculator() { return cohTracker.get(); };
	const CoherenceCalculator *getCoherenceCalculator() const { return cohTracker.get(); };

	void resizeThread(unsigned int tid, unsigned int size) {
		events[tid].resize(size);
	};

	void setEventLabel(Event e, std::unique_ptr<EventLabel> lab) {
		events[e.thread][e.index] = std::move(lab);
	};

	void calcPorfAfter(const Event e, View &a);
	void calcHbRfBefore(Event e, const llvm::GenericValue *addr, View &a) const;
	void calcRelRfPoBefore(const Event last, View &v) const;
	std::vector<Event> calcSCFencesSuccs(const std::vector<Event> &fcs,
					     const Event e) const;
	std::vector<Event> calcSCFencesPreds(const std::vector<Event> &fcs,
					     const Event e) const;
	std::vector<Event> calcSCSuccs(const std::vector<Event> &fcs,
				       const Event e) const;
	std::vector<Event> calcSCPreds(const std::vector<Event> &fcs,
				       const Event e) const;
	std::vector<Event> calcRfSCSuccs(const std::vector<Event> &fcs,
					 const Event e) const;
	std::vector<Event> calcRfSCFencesSuccs(const std::vector<Event> &fcs,
					       const Event e) const;
	bool isRMWLoad(const Event e) const;
	bool isRMWLoad(const EventLabel *lab) const;

	void spawnAllChildren(int thread);

	void addRbEdges(const std::vector<Event> &fcs,
			const std::vector<Event> &moAfter,
			const std::vector<Event> &moRfAfter,
			Matrix2D<Event> &matrix, const Event &e) const;
	void addMoRfEdges(const std::vector<Event> &fcs,
			  const std::vector<Event> &moAfter,
			  const std::vector<Event> &moRfAfter,
			  Matrix2D<Event> &matrix, const Event &e) const;
	void addSCEcos(const std::vector<Event> &fcs,
		       const std::vector<Event> &mo,
		       Matrix2D<Event> &matrix) const;
	void addSCEcos(const std::vector<Event> &fcs,
		       Matrix2D<Event> &wbMatrix,
		       Matrix2D<Event> &pscMatrix) const;

	template <typename F>
	bool addSCEcosMO(const std::vector<Event> &fcs,
			 const std::vector<const llvm::GenericValue *> &scLocs,
			 Matrix2D<Event> &psc, F cond) const;
	template <typename F>
	bool addSCEcosWBWeak(const std::vector<Event> &fcs,
			     const std::vector<const llvm::GenericValue *> &scLocs,
			     Matrix2D<Event> &psc, F cond) const;
	template <typename F>
	bool addSCEcosWB(const std::vector<Event> &fcs,
			 const std::vector<const llvm::GenericValue *> &scLocs,
			 Matrix2D<Event> &matrix, F cond) const;
	template <typename F>
	bool addSCEcosWBFull(const std::vector<Event> &fcs,
			     const std::vector<const llvm::GenericValue *> &scLocs,
			     Matrix2D<Event> &matrix, F cond) const;

	void addInitEdges(const std::vector<Event> &fcs,
			  Matrix2D<Event> &matrix) const;
	void addSbHbEdges(Matrix2D<Event> &matrix) const;
	template <typename F>
	bool addEcoEdgesAndCheckCond(CheckPSCType t,
				     const std::vector<Event> &fcs,
				     Matrix2D<Event> &psc, F cond) const;

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

private:

	/* Sets the coherence calculator to the specified one */
	void setCoherenceCalculator(std::unique_ptr<CoherenceCalculator> cc) {
		cohTracker = std::move(cc);
	};

	/* A collection of threads and the events for each threads */
	Graph events;

	/* A coherence calculator for the graph */
	std::unique_ptr<CoherenceCalculator> cohTracker = nullptr;

	/* The next available timestamp */
	unsigned int timestamp;
};

#include "CoherenceCalculator.hpp"

#endif /* __EXECUTION_GRAPH_HPP__ */
