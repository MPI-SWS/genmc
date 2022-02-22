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

#include "config.h"
#include "AdjList.hpp"
#include "CoherenceCalculator.hpp"
#include "DriverGraphEnumAPI.hpp"
#include "DepInfo.hpp"
#include "Error.hpp"
#include "Event.hpp"
#include "EventLabel.hpp"
#include "Revisit.hpp"
#include "VectorClock.hpp"
#include <llvm/ADT/StringMap.h>

#include <memory>
#include <unordered_map>

class CoherenceCalculator;
class LBCalculatorLAPOR;
class PSCCalculator;
class PersistencyChecker;

/*******************************************************************************
 **                           ExecutionGraph Class
 ******************************************************************************/

/*
 * A class representing plain execution graphs. This class offers
 * the basic infrastructure all types of graphs should provide (e.g.,
 * calculation of hb-predecessors, psc, etc). More specialized types
 * of graphs can provide extra functionality (e.g., take dependencies
 * into account when restricting a graph).
 */
class ExecutionGraph {

public:
	using Thread = std::vector<std::unique_ptr<EventLabel> >;
	using ThreadList = std::vector<Thread>;

private:
	using FixpointResult = Calculator::CalculationResult;

	enum FixpointStatus { FS_Stale, FS_InProgress, FS_Done };

	/* Packs together structures useful for calculations on relations */
	struct Relations {

		Relations() = default;

		std::vector<Calculator::GlobalRelation> global;
		std::vector<Calculator::PerLocRelation> perLoc;

		FixpointStatus fixStatus;
		FixpointResult fixResult;
		CheckConsType fixType;
	};

public:
	/* Should be used for the contruction of execution graphs */
	class Builder;

	/* Different relations that might exist in the graph */
	enum class RelationId {
		hb, co, lb, psc, ar, prop, ar_lkmm, pb, rcu_link, rcu, rcu_fence, xb
	};

protected:
	/* Constructor should only be called from the builder */
	friend class GraphBuilder;
	ExecutionGraph(unsigned warnOnGraphSize = UINT_MAX);
	ExecutionGraph(const ExecutionGraph &og);

public:
	virtual ~ExecutionGraph();

	/* Iterators */
	using iterator = ThreadList::iterator;
	using const_iterator = ThreadList::const_iterator;
	using reverse_iterator = ThreadList::reverse_iterator;
	using const_reverse_iterator = ThreadList::const_reverse_iterator;

	iterator begin() { return events.begin(); };
	iterator end() { return events.end(); };
	const_iterator begin() const { return events.begin(); };
	const_iterator end() const { return events.end(); };

	reverse_iterator rbegin() { return events.rbegin(); };
	reverse_iterator rend()   { return events.rend(); };
	const_reverse_iterator rbegin() const { return events.rbegin(); };
	const_reverse_iterator rend()   const { return events.rend(); };


	/* Thread-related methods */

	/* Returns a list of the threads in the graph */
	inline const ThreadList &getThreadList() const {
		return events;
	}
	inline ThreadList &getThreadList() {
		return const_cast<ThreadList &>(static_cast<const ExecutionGraph &>(*this).getThreadList());
	}

	/* Creates a new thread in the execution graph */
	inline void addNewThread() { events.push_back({}); };

	/* Pers: Add/remove a thread for the recovery procedure */
	inline void addRecoveryThread() {
		recoveryTID = events.size();
		events.push_back({});
	};
	inline void delRecoveryThread() {
		events.pop_back();
		recoveryTID = -1;
	};

	/* Returns the tid of the recovery routine.
	 * If not in recovery mode, returns -1 */
	inline int getRecoveryRoutineId() const { return recoveryTID; };

	/* Returns the number of threads currently in the graph */
	inline unsigned int getNumThreads() const { return events.size(); };

	/* Returns the size of the thread tid */
	inline unsigned int getThreadSize(int tid) const { return events[tid].size(); };

	/* Returns true if the thread tid is empty */
	inline bool isThreadEmpty(int tid) const { return getThreadSize(tid) == 0; };


	/* Event addition/removal methods */

	/* Returns the next available stamp (and increases the counter) */
	unsigned int nextStamp() { return timestamp++; }

	/* Resets the next available stamp to the specified value */
	void resetStamp(unsigned int val) { timestamp = val; }

	/* Event addition methods should be called from the managing objects,
	 * so that the relation managing objects are also informed */
	const ReadLabel *addReadLabelToGraph(std::unique_ptr<ReadLabel> lab,
					     Event rf = Event::getBottom());
	const WriteLabel *addWriteLabelToGraph(std::unique_ptr<WriteLabel> lab,
					       int offsetMO = -1);
	const WriteLabel *addWriteLabelToGraph(std::unique_ptr<WriteLabel> lab,
					       Event pred);
	const LockLabelLAPOR *addLockLabelToGraphLAPOR(std::unique_ptr<LockLabelLAPOR> lab);
	const EventLabel *addOtherLabelToGraph(std::unique_ptr<EventLabel> lab);

	/* Removes an event from the execution graph. If the event is
	 * not the last of a thread, it replaces it with an empty label.
	 * (Updates reader lists appropriately.) */
	void remove(const Event &e) { return remove(getEventLabel(e)); }
	void remove(const EventLabel *lab);


	/* Event getter methods */

	/* Returns the label in the position denoted by event e */
	const EventLabel *getEventLabel(Event e) const {
		return events[e.thread][e.index].get();
	}
	EventLabel *getEventLabel(Event e) {
		return const_cast<EventLabel *>(static_cast<const ExecutionGraph&>(*this).getEventLabel(e));
	}

	/* Returns a label as a ReadLabel.
	 * If the passed event is not a read, returns nullptr  */
	const ReadLabel *getReadLabel(Event e) const {
		return llvm::dyn_cast<ReadLabel>(getEventLabel(e));
	}
	ReadLabel *getReadLabel(Event e) {
		return const_cast<ReadLabel *>(static_cast<const ExecutionGraph &>(*this).getReadLabel(e));
	}

	/* Returns a label as a WriteLabel.
	 * If the passed event is not a write, returns nullptr  */
	const WriteLabel *getWriteLabel(Event e) const {
		return llvm::dyn_cast<WriteLabel>(getEventLabel(e));
	}
	WriteLabel *getWriteLabel(Event e) {
		return const_cast<WriteLabel *>(static_cast<const ExecutionGraph &>(*this).getWriteLabel(e));
	}

	/* Returns the label in the previous position of E.
	 * Returns nullptr if E is the first event of a thread */
	const EventLabel *getPreviousLabel(Event e) const {
		return e.index == 0 ? nullptr : getEventLabel(e.prev());
	}
	EventLabel *getPreviousLabel(Event e) {
		return const_cast<EventLabel *>(static_cast<const ExecutionGraph &>(*this).getPreviousLabel(e));
	}
	const EventLabel *getPreviousLabel(const EventLabel *lab) const {
		return getPreviousLabel(lab->getPos());
	}
	EventLabel *getPreviousLabel(EventLabel *lab) {
		return getPreviousLabel(lab->getPos());
	}

	/* Returns the label in the next position of E.
	 * Returns nullptr if E is the last event of a thread */
	const EventLabel *getNextLabel(Event e) const {
		return e == getLastThreadEvent(e.thread) ? nullptr : getEventLabel(e.next());
	}
	EventLabel *getNextLabel(Event e) {
		return const_cast<EventLabel *>(static_cast<const ExecutionGraph &>(*this).getNextLabel(e));
	}
	const EventLabel *getNextLabel(const EventLabel *lab) const {
		return getNextLabel(lab->getPos());
	}
	EventLabel *getNextLabel(EventLabel *lab) {
		return getNextLabel(lab->getPos());
	}

	/* Returns the previous non-empty label of e. Since all threads
	 * have an initializing event, it returns that as a base case */
	const EventLabel *getPreviousNonEmptyLabel(Event e) const;
	const EventLabel *getPreviousNonEmptyLabel(const EventLabel *lab) const {
		return getPreviousNonEmptyLabel(lab->getPos());
	}

	/* Returns the previous non-trivial predecessor of e.
	 * Returns INIT in case no such event is found */
	Event getPreviousNonTrivial(const Event e) const;

	/* Returns the first event in the thread tid */
	Event getFirstThreadEvent(int tid) const {
		return Event(tid, 0);
	}

	/* Returns the first label in the thread tid */
	const ThreadStartLabel *getFirstThreadLabel(int tid) const {
		return llvm::dyn_cast<ThreadStartLabel>(getEventLabel(getFirstThreadEvent(tid)));
	}

	/* Returns the last event/label in the thread tid */
	Event getLastThreadEvent(int thread) const {
		return Event(thread, getThreadSize(thread) - 1);
	}
	const EventLabel *getLastThreadLabel(int thread) const {
		return getEventLabel(getLastThreadEvent(thread));
	}
	EventLabel *getLastThreadLabel(int thread) {
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph&>(*this).getLastThreadLabel(thread));
	}

	/* Returns the last store at ADDR that is before UPPERLIMIT in
	 * UPPERLIMIT's thread. If such a store does not exist, it
	 * returns INIT */
	Event getLastThreadStoreAtLoc(Event upperLimit, SAddr addr) const;

	/* Returns the last release before upperLimit in the latter's thread.
	 * If it's not a fence, then it has to be at location addr */
	Event getLastThreadReleaseAtLoc(Event upperLimit, SAddr addr) const;

	/* Returns the last release before upperLimit in the latter's thread */
	Event getLastThreadRelease(Event upperLimit) const;

	/* Returns a list of acquire (R or F) in upperLimit's thread (before it) */
	std::vector<Event> getThreadAcquiresAndFences(const Event upperLimit) const;

	/* Returns the unlock that matches UNLOCK.
	 * If no such event exists, returns INIT */
	Event getMatchingLock(const Event unlock) const;

	/* Returns the unlock that matches LOCK. LOCK needs to be the
	 * read part of a lock operation. If no such event exists,
	 * returns INIT */
	Event getMatchingUnlock(const Event lock) const;

	/* Returns the RCU unlock that matches LOCK. If such an event does not exist,
	 * it returns the (non-existing event) at thread size */
	Event getMatchingRCUUnlockLKMM(Event lock) const;

	/* Helper: Returns the last speculative read in CONF's location that
	 * is not matched. If no such event exists, returns INIT.
	 * (If SC is non-null and an SC event is in-between the confirmation,
	 * SC is set to that event) */
	Event getMatchingSpeculativeRead(Event conf, Event *sc = nullptr) const;

	/* LAPOR: Returns the last lock that is not matched before "upperLimit".
	 * If no such event exists, returns INIT */
	Event getLastThreadUnmatchedLockLAPOR(const Event upperLimit) const;

	/* LAPOR: Returns the unlock that matches "lock". If no such event
	 * exists, returns INIT */
	Event getMatchingUnlockLAPOR(const Event lock) const;

	/* LAPOR: Returns the last lock at location "loc" before "upperLimit".
	 * If no such event exists, returns INIT */
	Event getLastThreadLockAtLocLAPOR(const Event upperLimit, SAddr addr) const;

	/* LAPOR: Returns the last unlock at location "loc" before "upperLimit".
	 * If no such event exists, returns INIT */
	Event getLastThreadUnlockAtLocLAPOR(const Event upperLimit, SAddr addr) const;

	/* LAPOR: Returns a linear extension of LB */
	std::vector<Event> getLbOrderingLAPOR() const;

	/* Returns the allocating event for ADDR.
	 * Assumes that only one such event may exist */
	Event getMalloc(const SAddr &addr) const;

	/* Given a deallocating event FLAB, returns its allocating
	 * counterpart (their addresses need to match exactly).
	 * Assumes that only one such event may exist */
	Event getMallocCounterpart(const FreeLabel *fLab) const;

	/* Returns pair with all SC accesses and all SC fences */
	std::pair<std::vector<Event>, std::vector<Event> > getSCs() const;

	/* Given a write label sLab that is part of an RMW, returns
	 * another RMW that reads from the same write. If no such event
	 * exists, it returns INIT. If there are multiple such events,
	 * returns the one with the smallest stamp */
	Event getPendingRMW(const WriteLabel *sLab) const;

	/* Given a revisit RLAB <- WLAB, returns the view of the resulting graph.
	 * (This function can be abused and also be utilized for returning the view
	 * of "fictional" revisits, e.g., the view of an event in a maximal path.) */
	virtual std::unique_ptr<VectorClock>
	getRevisitView(const BackwardRevisit &r) const;

	/* Returns a list of loads that can be revisited */
	virtual std::vector<Event> getRevisitable(const WriteLabel *sLab) const;

	/* Returns the first po-predecessor satisfying F */
	template <typename F>
	const EventLabel *getPreviousLabelST(const EventLabel *lab, F&& cond) const {
		for (auto j = lab->getIndex() - 1; j >= 0; j--) {
			auto *eLab = getEventLabel(Event(lab->getThread(), j));
			if (cond(eLab))
				return eLab;
		}
		return nullptr;
	}

	/* Returns a list of all events satisfying property F */
	template <typename F>
	std::vector<Event> collectAllEvents(F cond) const {
		std::vector<Event> result;

		for (auto i = 0u; i < getNumThreads(); i++)
			for (auto j = 0u; j < getThreadSize(i); j++)
				if (cond(getEventLabel(Event(i, j))))
					result.push_back(Event(i, j));
		return result;
	}


	/* Calculation of relations in the graph */

	/* Adds the specified calculator to the list */
	void addCalculator(std::unique_ptr<Calculator> cc, RelationId r,
			   bool perLoc, bool partial = false);

	/* Returns the list of the calculators */
	const std::vector<Calculator *> getCalcs() const;

	/* Returns the list of the partial calculators */
	const std::vector<Calculator *> getPartialCalcs() const;

	/* Returns a reference to the specified relation matrix */
	Calculator::GlobalRelation& getGlobalRelation(RelationId id);
	Calculator::PerLocRelation& getPerLocRelation(RelationId id);

	/* Returns a reference to the cached version of the
	 * specified relation matrix */
	Calculator::GlobalRelation& getCachedGlobalRelation(RelationId id);
	Calculator::PerLocRelation& getCachedPerLocRelation(RelationId id);

	/* Caches all calculated relations. If "copy" is true then a
	 * copy of each relation is cached */
	void cacheRelations(bool copy = true);

	/* Restores all relations to their most recently cached versions.
	 * If "move" is true then the cache is cleared as well */
	void restoreCached(bool move = false);

	/* Returns a pointer if the graph has a calculator for the specified relation */
	bool hasCalculator(RelationId id) const;

	/* Returns a pointer to the specified relation's calculator */
	Calculator *getCalculator(RelationId id);

	/* Commonly queried calculator getters */
	CoherenceCalculator *getCoherenceCalculator();
	CoherenceCalculator *getCoherenceCalculator() const;
	LBCalculatorLAPOR *getLbCalculatorLAPOR();
	LBCalculatorLAPOR *getLbCalculatorLAPOR() const;

	/* Pers: Adds a persistency checker to the graph */
	void addPersistencyChecker(std::unique_ptr<PersistencyChecker> pc);

	/* Pers: Returns the persistency checker */
	PersistencyChecker *getPersChecker() const {
		return persChecker.get();
	}
	PersistencyChecker *getPersChecker() {
		return persChecker.get();
	}

	const DepView &getPPoRfBefore(Event e) const;
	const View &getPorfBefore(Event e) const;
	const View &getHbPoBefore(Event e) const;
	std::vector<Event> getInitRfsAtLoc(SAddr addr) const;

	/* Returns true if a is hb-before b */
	bool isHbBefore(Event a, Event b, CheckConsType t = CheckConsType::fast);

	/* Returns true if e is maximal in addr */
	bool isCoMaximal(SAddr addr, Event e, bool checkCache = false,
			 CheckConsType t = CheckConsType::fast);

	/* Returns true if the current graph is consistent */
	bool isConsistent(CheckConsType t = CheckConsType::fast);

	/* Matrix filling for external relation calculation */
	void populatePorfEntries(AdjList<Event, EventHasher> &relation) const;
	void populatePPoRfEntries(AdjList<Event, EventHasher> &relation) const;
	void populateHbEntries(AdjList<Event, EventHasher> &relation) const;


	/* Boolean helper functions */

	/* Returns true if the graph contains e */
	bool contains(const Event &e) const {
		return e.thread >= 0 && e.thread < getNumThreads() &&
		        e.index >= 0 && e.index < getThreadSize(e.thread);
	}
	bool contains(const EventLabel *lab) const {
		return contains(lab->getPos());
	}

	/* Returns true if the graph contains e, and the label is not EMPTY */
	bool containsNonEmpty(const Event &e) const {
		return contains(e) && !llvm::isa<EmptyLabel>(getEventLabel(e));
	}
	bool containsNonEmpty(const EventLabel *lab) const {
		return containsNonEmpty(lab->getPos());
	}

	/* Returns true if the event should be taken into account when
	 * calculating some relation (e.g., hb, ar, etc) */
	bool isNonTrivial(const Event e) const;
	bool isNonTrivial(const EventLabel *lab) const;

	/* LAPOR: Returns true if the critical section started by lLab is empty */
	bool isCSEmptyLAPOR(const LockLabelLAPOR *lLab) const;

	/* BAM: Returns true if BAM is enabled for this graph */
	bool hasBAM() const { return bam; }

	/* Return true if its argument is the load/store part of a successful RMW */
	bool isRMWLoad(const EventLabel *lab) const;
	bool isRMWLoad(const Event e) const { return isRMWLoad(getEventLabel(e)); }
	bool isRMWStore(const EventLabel *lab) const {
		return llvm::isa<FaiWriteLabel>(lab) || llvm::isa<CasWriteLabel>(lab);
	}
	bool isRMWStore(const Event e) const { return isRMWStore(getEventLabel(e)); }

	/* Returns true if the addition of SLAB violates atomicity in the graph */
	bool violatesAtomicity(const WriteLabel *sLab){
		return isRMWStore(sLab) && !getPendingRMW(sLab).isInitializer();
	}

	/* Helper: Returns true if RLAB is a confirming operation */
	bool isConfirming(const ReadLabel *rLab) const {
		return llvm::isa<ConfirmingReadLabel>(rLab) || llvm::isa<ConfirmingCasReadLabel>(rLab);
	}

	/* Opt: Returns true if LAB is causing the respective thread
	 * to block due to some optimization */
	bool isOptBlockedRead(const EventLabel *lab) const {
		if (llvm::isa<ReadLabel>(lab) &&
		    getLastThreadEvent(lab->getThread()) == lab->getPos().next()) {
			auto *bLab = llvm::dyn_cast<BlockLabel>(getNextLabel(lab));
			return bLab && bLab->getType() == BlockageType::ReadOptBlock;
		}
		return false;
	}

	/* Returns true if e is hb-before w, or any of the reads that read from w */
	bool isHbOptRfBefore(const Event e, const Event write) const;
	bool isHbOptRfBeforeInView(const Event e, const Event write, const VectorClock &v) const;

	/* Returns true if e is rel-before w, or any of the reads that read from w
	 * in the relation "rel".
	 * Pre: all examined events need to be a part of rel */
	template <typename F = bool (*)(Event)>
	bool isHbOptRfBeforeRel(const AdjList<Event, EventHasher> &rel, Event a, Event b,
				F prop = [](Event e){ return true; }) const;

	/* Returns true if a (or any of the reads reading from a) is hb-before b */
	bool isWriteRfBefore(Event a, Event b) const;

	/* Returns true if a (or any of the reads reading from a) is before b in
	 * the relation "rel".
	 * Pre: all examined events need to be a part of rel */
	template <typename F = bool (*)(Event)>
	bool isWriteRfBeforeRel(const AdjList<Event, EventHasher> &rel, Event a, Event b,
				F prop = [](Event e){ return true; }) const;

	/* Returns true if store is read a successful RMW in the location ptr */
	bool isStoreReadByExclusiveRead(Event store, SAddr addr) const;

	/* Returns true if store is read by a successful RMW that is either non
	 * revisitable, or in the view porfBefore */
	bool isStoreReadBySettledRMW(Event store, SAddr addr, const VectorClock &porfBefore) const;

	/* Pers: Returns true if the recovery routine is valid */
	bool isRecoveryValid() const;

	/* Returnes true if the revisit R will delete LAB from the graph */
	bool revisitDeletesEvent(const BackwardRevisit &r, const EventLabel *lab) const {
		auto v = getRevisitView(r);
		return !v->contains(lab->getPos()) && !prefixContainsSameLoc(r, lab);
	}

	/* Returns true if ELAB has been revisited by some event that
	 * will be deleted by the revisit R */
	bool hasBeenRevisitedByDeleted(const BackwardRevisit &r, const EventLabel *eLab) const;

	/* Returns whether the prefix of SLAB contains LAB's matching lock */
	bool prefixContainsMatchingLock(const BackwardRevisit &r, const EventLabel *lab) const {
		if (!llvm::isa<UnlockWriteLabel>(lab))
			return false;
		auto l = getMatchingLock(lab->getPos());
		if (l.isInitializer())
			return false;
		if (getPrefixView(r.getRev()).contains(l))
			return true;
		if (auto *br = llvm::dyn_cast<BackwardRevisitHELPER>(&r))
			return getPrefixView(br->getMid()).contains(l);
		return false;
	}

	/* Returns true if all events to be removed by the revisit
	 * RLAB <- SLAB form a maximal extension */
	bool isMaximalExtension(const BackwardRevisit &r) const;

	/* Returns true if the graph that will be created when sLab revisits rLab
	 * will be the same as the current one */
	virtual bool revisitModifiesGraph(const BackwardRevisit &r) const;

	virtual bool prefixContainsSameLoc(const BackwardRevisit &r,
					   const EventLabel *lab) const {
		return false;
	}


	/* Debugging methods */

	void validate(void);


	/* Modification order methods */

	void trackCoherenceAtLoc(SAddr addr);
	std::vector<Event> getCoherentStores(SAddr addr, Event pos);
	std::pair<int, int> getCoherentPlacings(SAddr addr, Event pos, bool isRMW);
	std::vector<Event> getCoherentRevisits(const WriteLabel *wLab);


	/* Graph modification methods */

	void changeRf(Event read, Event store);
	void changeStoreOffset(SAddr addr, Event s, int newOffset);
	void resetJoin(Event join);
	bool updateJoin(Event join, Event childLast);


	/* Prefix saving and restoring */

	/* Returns a vector clock representing the prefix of e.
	 * Depending on whether dependencies are tracked, the prefix can be
	 * either (po U rf) or (AR U rf) */
	virtual const VectorClock& getPrefixView(Event e) const {
		return getEventLabel(e)->getPorfView();
	}

	/* Returns a vector clock representing the events added before e */
	virtual std::unique_ptr<VectorClock> getPredsView(Event e) const;

#ifdef ENABLE_GENMC_DEBUG
	/* Saves the prefix of sLab that is not before rLab. */
	virtual std::vector<std::unique_ptr<EventLabel> >
	getPrefixLabelsNotBefore(const WriteLabel *sLab, const ReadLabel *rLab) const;

	/* Returns a list of the rfs of the reads in labs */
	std::vector<Event>
	extractRfs(const std::vector<std::unique_ptr<EventLabel> > &labs) const;

	/* Returns pairs of the form <store, pred> where store is a write from labs,
	 * and pred is an mo-before store that was added before rLab */
	std::vector<std::pair<Event, Event> >
	saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
			    const ReadLabel *rLab) const;
#endif

	/* Graph cutting */

	/* Returns a view of the graph representing events with stamp <= st */
	View getViewFromStamp(unsigned int st) const;

	/* Simmilar to getViewFromStamp() but returns a DepView */
	DepView getDepViewFromStamp(unsigned int st) const;

	/* Cuts a graph so that it only contains events with stamp <= st */
	virtual void cutToStamp(unsigned int st);

	/* FIXME: Use value ptrs? (less error-prone than using explicit copy fun) */
	/* Or maybe simply consolidate the copying procedure:
	 * 1) Copy graph structure (calculators, constant members, etc)
	 * 2) Copy events => these should notify calculators so that calcs populate their structures
	 */
	virtual std::unique_ptr<ExecutionGraph> getCopyUpTo(const VectorClock &v) const;

	/* Overloaded operators */
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ExecutionGraph &g);

protected:
	void enableBAM() { bam = true; }

	void resizeThread(unsigned int tid, unsigned int size) {
		events[tid].resize(size);
	};
	void resizeThread(Event pos) { resizeThread(pos.thread, pos.index); }

	void setEventLabel(Event e, std::unique_ptr<EventLabel> lab) {
		events[e.thread][e.index] = std::move(lab);
	};

	/* Returns the event with the minimum stamp in ES.
	 * If ES is empty, returns INIT */
	Event getMinimumStampEvent(const std::vector<Event> &es) const;

	void copyGraphUpTo(ExecutionGraph &other, const VectorClock &v) const;

	FixpointStatus getFPStatus() const { return relations.fixStatus; }
	void setFPStatus(FixpointStatus s) { relations.fixStatus = s; }

	CheckConsType getFPType() const { return relations.fixType; }
	void setFPType(CheckConsType t) { relations.fixType = t; }

	FixpointResult getFPResult() const { return relations.fixResult; }
	void setFPResult(FixpointResult r) { relations.fixResult = r; }

	void doInits(bool fullCalc = false);

	/* Performs a step of all the specified calculations. Takes as
	 * a parameter whether a full calculation needs to be performed */
	FixpointResult doCalcs(bool fullCalc = false);

	/* Does some final consistency checks after the fixpoint is over,
	 * and returns the final decision re. consistency */
	bool doFinalConsChecks(bool checkFull = false);

private:
	/* A collection of threads and the events for each threads */
	ThreadList events;

	/* The next available timestamp */
	unsigned int timestamp;

	/* Relations and calculation status/result */
	Relations relations;
	Relations relsCache;

	/* A list of all the calculations that need to be performed
	 * when checking for full consistency*/
	std::vector<std::unique_ptr<Calculator> > consistencyCalculators;

	/* The indices of all the calculations that need to be performed
	 * at each step of the algorithm (partial consistency check) */
	std::vector<int> partialConsCalculators;

	/* Keeps track of calculator indices */
	std::unordered_map<RelationId, unsigned int, ENUM_HASH(RelationId) > calculatorIndex;

	/* Keeps track of relation indices. Note that an index might
	 * refer to either relations.global or relations.perLoc */
	std::unordered_map<RelationId, unsigned int, ENUM_HASH(RelationId) > relationIndex;

	/* Pers: An object calculating persistency relations */
	std::unique_ptr<PersistencyChecker> persChecker; /* nullptr in ctor */

	/* Pers: The ID of the recovery routine.
	 * It should be -1 if not in recovery mode, or have the
	 * value of the recovery routine otherwise. */
	int recoveryTID = -1;

	/* BAM: Flag indicating how we should treat barrier operations */
	bool bam = false;

	/* Dbg: Size of graphs which triggers a warning */
	unsigned int warnOnGraphSize = UINT_MAX;
};

template <typename F>
bool ExecutionGraph::isHbOptRfBeforeRel(const AdjList<Event, EventHasher> &rel, Event a, Event b,
					F prop /* = [](Event e){ return true; } */) const
{
	if (rel(a, b))
		return true;

	const EventLabel *lab = getEventLabel(b);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (auto &r : wLab->getReadersList()) {
		if (prop(r) && rel(a, r))
			return true;
	}
	return false;
}

template <typename F>
bool ExecutionGraph::isWriteRfBeforeRel(const AdjList<Event, EventHasher> &rel, Event a, Event b,
					F prop /* = [&](Event e){ return true; } */) const
{
	if (rel(a, b))
		return true;

	const EventLabel *lab = getEventLabel(a);

	BUG_ON(!llvm::isa<WriteLabel>(lab));
	auto *wLab = static_cast<const WriteLabel *>(lab);
	for (auto &r : wLab->getReadersList())
		if (prop(r) && rel(r, b))
			return true;
	return false;
}

#endif /* __EXECUTION_GRAPH_HPP__ */
