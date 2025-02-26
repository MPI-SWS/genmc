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

#ifndef GENMC_EXECUTION_GRAPH_HPP
#define GENMC_EXECUTION_GRAPH_HPP

#include "ADT/AdjList.hpp"
#include "ADT/VectorClock.hpp"
#include "ExecutionGraph/DepInfo.hpp"
#include "ExecutionGraph/Event.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/Stamp.hpp"
#include "Support/Error.hpp"
#include "Support/Hash.hpp"
#include "Verification/Revisit.hpp"
#include "config.h"
#include <llvm/ADT/StringMap.h>

#include <functional>
#include <memory>
#include <ranges>
#include <unordered_map>

class PSCCalculator;

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
	using Thread = std::vector<std::unique_ptr<EventLabel>>;
	using ThreadList = std::vector<Thread>;
	using StoreList = llvm::simple_ilist<WriteLabel>;
	using LocMap = std::unordered_map<SAddr, StoreList>;
	using InitValGetter = std::function<SVal(const AAccess &)>;

	ExecutionGraph(InitValGetter f) : initValGetter_(std::move(f))
	{
		/* Create an entry for main() and push the "initializer" label */
		events.push_back({});
		auto *iLab = addLabelToGraph(InitLabel::create());
		iLab->setCalculated({{}});
		iLab->setViews({{}});
		iLab->setPrefixView(std::make_unique<View>());
	}

	ExecutionGraph(const ExecutionGraph &) = delete;
	ExecutionGraph(ExecutionGraph &&) noexcept = default;

	auto operator=(const ExecutionGraph &) -> ExecutionGraph & = delete;
	auto operator=(ExecutionGraph &&) noexcept -> ExecutionGraph & = default;

	virtual ~ExecutionGraph() = default;

	/* Iterators */
	using iterator = ThreadList::iterator;
	using const_iterator = ThreadList::const_iterator;
	using reverse_iterator = ThreadList::reverse_iterator;
	using const_reverse_iterator = ThreadList::const_reverse_iterator;

	using loc_iterator = LocMap::iterator;
	using const_loc_iterator = LocMap::const_iterator;

	using label_iterator = llvm::simple_ilist<EventLabel>::iterator;
	using const_label_iterator = llvm::simple_ilist<EventLabel>::const_iterator;

	using co_iterator = StoreList::iterator;
	using const_co_iterator = StoreList::const_iterator;
	using reverse_co_iterator = StoreList::reverse_iterator;
	using const_reverse_co_iterator = StoreList::const_reverse_iterator;

	using initrf_iterator = InitLabel::rf_iterator;
	using const_initrf_iterator = InitLabel::const_rf_iterator;

	iterator begin() { return events.begin(); };
	iterator end() { return events.end(); };
	const_iterator begin() const { return events.begin(); };
	const_iterator end() const { return events.end(); };

	reverse_iterator rbegin() { return events.rbegin(); };
	reverse_iterator rend() { return events.rend(); };
	const_reverse_iterator rbegin() const { return events.rbegin(); };
	const_reverse_iterator rend() const { return events.rend(); };

	auto label_begin() const { return insertionOrder.begin(); }
	auto label_end() const { return insertionOrder.end(); }
	auto labels() const { return std::views::all(insertionOrder); }

	auto label_begin() { return insertionOrder.begin(); }
	auto label_end() { return insertionOrder.end(); }
	auto labels() { return std::views::all(insertionOrder); }

	auto thr_ids() const { return std::views::iota(0, (int)getNumThreads()); }
	auto thr_ids() { return std::views::iota(0, (int)getNumThreads()); }

	auto po(int tid) const
	{
		return std::views::all(events[tid]) | std::ranges::views::transform(indirect);
	}
	auto po(int tid)
	{
		return std::views::all(events[tid]) | std::ranges::views::transform(indirect);
	}

	auto po_succs(const EventLabel *lab) const
	{
		const auto &thr = events[lab->getThread()];
		return std::ranges::subrange(thr.begin() + lab->getIndex() + 1, thr.end()) |
		       std::ranges::views::transform(indirect);
	}
	auto po_succs(EventLabel *lab)
	{
		auto &thr = events[lab->getThread()];
		return std::ranges::subrange(thr.begin() + lab->getIndex() + 1, thr.end()) |
		       std::ranges::views::transform(indirect);
	}

	auto po_preds(const EventLabel *lab) const
	{
		const auto &thr = events[lab->getThread()];
		return std::ranges::subrange(thr.begin(), thr.begin() + lab->getIndex()) |
		       std::views::reverse | std::ranges::views::transform(indirect);
	}
	auto po_preds(EventLabel *lab)
	{
		const auto &thr = events[lab->getThread()];
		return std::ranges::subrange(thr.begin(), thr.begin() + lab->getIndex()) |
		       std::views::reverse | std::ranges::views::transform(indirect);
	}

	/* Returns the label in the previous position of E.
	 * Returns nullptr if E is the first event of a thread */
	auto po_imm_pred(const EventLabel *lab) const -> const EventLabel *
	{
		return lab->getIndex() == 0 ? nullptr
					    : events[lab->getThread()][lab->getIndex() - 1].get();
	}
	auto po_imm_pred(EventLabel *lab) -> EventLabel *
	{
		// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).po_imm_pred(lab));
		// NOLINTEND(cppcoreguidelines-pro-type-const-cast)
	}

	/* Returns the label in the next position of E.
	 * Returns nullptr if E is the last event of a thread */
	auto po_imm_succ(const EventLabel *lab) const -> const EventLabel *
	{
		return lab->getIndex() == getThreadSize(lab->getThread()) - 1
			       ? nullptr
			       : events[lab->getThread()][lab->getIndex() + 1].get();
	}
	auto po_imm_succ(EventLabel *lab) -> EventLabel *
	{
		// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).po_imm_succ(lab));
		// NOLINTEND(cppcoreguidelines-pro-type-const-cast)
	}

	loc_iterator loc_begin() { return coherence.begin(); }
	const_loc_iterator loc_begin() const { return coherence.begin(); };
	loc_iterator loc_end() { return coherence.end(); }
	const_loc_iterator loc_end() const { return coherence.end(); }

	co_iterator co_begin(SAddr addr) { return coherence[addr].begin(); }
	const_co_iterator co_begin(SAddr addr) const { return coherence.at(addr).begin(); };
	co_iterator co_end(SAddr addr) { return coherence[addr].end(); }
	const_co_iterator co_end(SAddr addr) const { return coherence.at(addr).end(); }
	auto co(SAddr addr) { return std::views::all(coherence[addr]); }
	auto co(SAddr addr) const { return std::views::all(coherence.at(addr)); }

	reverse_co_iterator co_rbegin(SAddr addr) { return coherence[addr].rbegin(); }
	const_reverse_co_iterator co_rbegin(SAddr addr) const
	{
		return coherence.at(addr).rbegin();
	};
	reverse_co_iterator co_rend(SAddr addr) { return coherence[addr].rend(); }
	const_reverse_co_iterator co_rend(SAddr addr) const { return coherence.at(addr).rend(); }
	auto rco(SAddr addr) { return std::views::all(coherence[addr]) | std::views::reverse; }
	auto rco(SAddr addr) const
	{
		return std::views::all(coherence.at(addr)) | std::views::reverse;
	}

	initrf_iterator init_rf_begin(SAddr addr) { return getInitLabel()->rf_begin(addr); }
	const_initrf_iterator init_rf_begin(SAddr addr) const
	{
		return getInitLabel()->rf_begin(addr);
	};
	initrf_iterator init_rf_end(SAddr addr) { return getInitLabel()->rf_end(addr); }
	const_initrf_iterator init_rf_end(SAddr addr) const { return getInitLabel()->rf_end(addr); }

	co_iterator co_succ_begin(WriteLabel *lab) { return ++co_iterator(lab); }
	const_co_iterator co_succ_begin(const WriteLabel *lab) const
	{
		return ++const_co_iterator(lab);
	}
	co_iterator co_succ_end(WriteLabel *lab) { return co_end(lab->getAddr()); }
	const_co_iterator co_succ_end(const WriteLabel *lab) const
	{
		return co_end(lab->getAddr());
	}
	const WriteLabel *co_imm_succ(const WriteLabel *lab) const
	{
		auto it = co_succ_begin(lab);
		return it == co_succ_end(lab) ? nullptr : &*it;
	}

	reverse_co_iterator co_pred_begin(WriteLabel *lab) { return ++reverse_co_iterator(lab); }
	const_reverse_co_iterator co_pred_begin(const WriteLabel *lab) const
	{
		return ++const_reverse_co_iterator(lab);
	}
	reverse_co_iterator co_pred_end(WriteLabel *lab) { return co_rend(lab->getAddr()); }
	const_reverse_co_iterator co_pred_end(const WriteLabel *lab) const
	{
		return co_rend(lab->getAddr());
	}
	const WriteLabel *co_imm_pred(const WriteLabel *lab) const
	{
		auto it = co_pred_begin(lab);
		return it == co_pred_end(lab) ? nullptr : &*(it);
	}
	WriteLabel *co_imm_pred(WriteLabel *lab)
	{
		return const_cast<WriteLabel *>(
			static_cast<const ExecutionGraph &>(*this).co_imm_pred(lab));
	}

	const EventLabel *co_max(SAddr addr) const
	{
		return co_begin(addr) == co_end(addr) ? (EventLabel *)getInitLabel()
						      : (EventLabel *)&*co_rbegin(addr);
	}
	EventLabel *co_max(SAddr addr)
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).co_max(addr));
	}

	co_iterator fr_succ_begin(ReadLabel *rLab)
	{
		auto *wLab = llvm::dyn_cast<WriteLabel>(rLab->getRf());
		return wLab ? co_succ_begin(wLab) : co_begin(rLab->getAddr());
	}
	const_co_iterator fr_succ_begin(const ReadLabel *rLab) const
	{
		auto *wLab = llvm::dyn_cast<WriteLabel>(rLab->getRf());
		return wLab ? co_succ_begin(wLab) : co_begin(rLab->getAddr());
	}
	co_iterator fr_succ_end(ReadLabel *rLab) { return co_end(rLab->getAddr()); }
	const_co_iterator fr_succ_end(const ReadLabel *rLab) const
	{
		return co_end(rLab->getAddr());
	}
	const WriteLabel *fr_imm_succ(const ReadLabel *rLab) const
	{
		auto it = fr_succ_begin(rLab);
		return it == fr_succ_end(rLab) ? nullptr : &*it;
	}

	WriteLabel::rf_iterator fr_imm_pred_begin(WriteLabel *wLab)
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_begin(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_begin();
	}
	WriteLabel::const_rf_iterator fr_imm_pred_begin(const WriteLabel *wLab) const
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_begin(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_begin();
	}
	WriteLabel::rf_iterator fr_imm_pred_end(WriteLabel *wLab)
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_end(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_end();
	}
	WriteLabel::const_rf_iterator fr_imm_pred_end(const WriteLabel *wLab) const
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_end(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_end();
	}

	/* Thread-related methods */

	/* Creates a new thread in the execution graph */
	void addNewThread() { events.push_back({}); };

	/* Pers: Add/remove a thread for the recovery procedure */
	void addRecoveryThread()
	{
		recoveryTID = events.size();
		events.push_back({});
	};
	void delRecoveryThread()
	{
		events.pop_back();
		recoveryTID = -1;
	};

	/* Returns the tid of the recovery routine.
	 * If not in recovery mode, returns -1 */
	int getRecoveryRoutineId() const { return recoveryTID; };

	/* Returns the number of threads currently in the graph */
	unsigned int getNumThreads() const { return events.size(); };

	/* Returns the size of the thread tid */
	unsigned int getThreadSize(int tid) const { return events[tid].size(); };

	/* Returns true if the thread tid is empty */
	bool isThreadEmpty(int tid) const { return getThreadSize(tid) == 0; };

	/* Event addition/removal methods */

	const InitLabel *getInitLabel() const
	{
		return static_cast<const InitLabel *>(getEventLabel(Event(0, 0)));
	}
	InitLabel *getInitLabel()
	{
		return const_cast<InitLabel *>(
			static_cast<const ExecutionGraph &>(*this).getInitLabel());
	}

	/* Returns the maximum stamp used */
	Stamp getMaxStamp() const { return timestamp; }

	/* Adds LAB to the graph. If a label exists in the respective
	 * position, it is replaced.
	 * (Maintains well-formedness for read removals.) */
	EventLabel *addLabelToGraph(std::unique_ptr<EventLabel> lab);

	void addStoreToCOBefore(WriteLabel *wLab, WriteLabel *succLab)
	{
		coherence[wLab->getAddr()].insert(co_iterator(succLab), *wLab);
	}
	void addStoreToCOAfter(WriteLabel *wLab, EventLabel *predLab)
	{
		auto *predLabW = llvm::dyn_cast<WriteLabel>(predLab);
		coherence[wLab->getAddr()].insert(
			predLabW ? ++co_iterator(*predLabW) : co_begin(wLab->getAddr()), *wLab);
	}

	void removeStoreFromCO(WriteLabel *wLab) { coherence[wLab->getAddr()].remove(*wLab); }

	void moveStoreCOBefore(WriteLabel *wLab, WriteLabel *succLab)
	{
		removeStoreFromCO(wLab);
		addStoreToCOBefore(wLab, succLab);
	}

	void moveStoreCOAfter(WriteLabel *wLab, EventLabel *predLab)
	{
		removeStoreFromCO(wLab);
		addStoreToCOAfter(wLab, predLab);
	}

	/* Removes the last event from THREAD.
	 * If it is a read, updates the rf-lists.
	 * If it is a write, makes all readers read BOT. */
	void removeLast(unsigned int thread);

	/* Event getter methods */

	/* Returns the label in the position denoted by event e */
	const EventLabel *getEventLabel(Event e) const { return events[e.thread][e.index].get(); }
	EventLabel *getEventLabel(Event e)
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).getEventLabel(e));
	}

	/* Returns a label as a ReadLabel.
	 * If the passed event is not a read, returns nullptr  */
	const ReadLabel *getReadLabel(Event e) const
	{
		return llvm::dyn_cast<ReadLabel>(getEventLabel(e));
	}
	ReadLabel *getReadLabel(Event e)
	{
		return const_cast<ReadLabel *>(
			static_cast<const ExecutionGraph &>(*this).getReadLabel(e));
	}

	/* Returns a label as a WriteLabel.
	 * If the passed event is not a write, returns nullptr  */
	const WriteLabel *getWriteLabel(Event e) const
	{
		return llvm::dyn_cast<WriteLabel>(getEventLabel(e));
	}
	WriteLabel *getWriteLabel(Event e)
	{
		return const_cast<WriteLabel *>(
			static_cast<const ExecutionGraph &>(*this).getWriteLabel(e));
	}

	/* Returns the first label in the thread tid */
	const ThreadStartLabel *getFirstThreadLabel(int tid) const
	{
		return llvm::dyn_cast<ThreadStartLabel>(getEventLabel(Event(tid, 0)));
	}
	ThreadStartLabel *getFirstThreadLabel(int tid)
	{
		return const_cast<ThreadStartLabel *>(
			static_cast<const ExecutionGraph &>(*this).getFirstThreadLabel(tid));
	}

	/* Returns the last label in the thread tid */
	const EventLabel *getLastThreadLabel(int thread) const
	{
		return getEventLabel(Event(thread, getThreadSize(thread) - 1));
	}
	EventLabel *getLastThreadLabel(int thread)
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).getLastThreadLabel(thread));
	}

	/* Given a write label sLab that is part of an RMW, returns
	 * another RMW that reads from the same write. If no such event
	 * exists, it returns INIT. If there are multiple such events,
	 * returns the one with the smallest stamp */
	Event getPendingRMW(const WriteLabel *sLab) const;

	/* Returns a list of loads that can be revisited */
	virtual std::vector<ReadLabel *> getRevisitable(WriteLabel *sLab, const VectorClock &pporf);

	/* Boolean helper functions */

	SVal getInitVal(const AAccess &access) const { return initValGetter_(access); }

	void setInitValGetter(InitValGetter f) { initValGetter_ = std::move(f); }

	bool isLocEmpty(SAddr addr) const { return co_begin(addr) == co_end(addr); }

	/* Whether a location has more than one store */
	bool hasLocMoreThanOneStore(SAddr addr) const
	{
		return !isLocEmpty(addr) && ++co_begin(addr) != co_end(addr);
	}

	/* Returns true if the graph contains e */
	bool containsPos(const Event &e) const
	{
		return e.thread >= 0 && e.thread < getNumThreads() && e.index >= 0 &&
		       e.index < getThreadSize(e.thread);
	}
	bool containsLab(const EventLabel *lab) const
	{
		return containsPos(lab->getPos()) && getEventLabel(lab->getPos()) == lab;
	}

	/* Returns true if the graph contains e, and the label is not EMPTY */
	bool containsPosNonEmpty(const Event &e) const
	{
		return containsPos(e) && !llvm::isa<EmptyLabel>(getEventLabel(e));
	}

	/* Returns true if the addition of SLAB violates atomicity in the graph */
	bool violatesAtomicity(const WriteLabel *sLab)
	{
		return sLab->isRMW() && !getPendingRMW(sLab).isInitializer();
	}

	/* Debugging methods */

	void validate(void);

	/* Graph modification methods */

	void addAlloc(MallocLabel *aLab, MemAccessLabel *mLab);

	/* Prefix saving and restoring */

	/* Returns a vector clock representing the events added before e */
	std::unique_ptr<VectorClock> getPredsView(Event e) const
	{
		auto stamp = getEventLabel(e)->getStamp();
		return getViewFromStamp(stamp);
	}

	/* Graph cutting */

	/* Returns a view of the graph representing events with stamp <= st */
	virtual std::unique_ptr<VectorClock> getViewFromStamp(Stamp st) const;

	/* Cuts a graph so that it only contains events with stamp <= st */
	virtual void cutToStamp(Stamp st);

	/* FIXME: Use value ptrs? (less error-prone than using explicit copy fun) */
	/* Or maybe simply consolidate the copying procedure:
	 * 1) Copy graph structure (calculators, constant members, etc)
	 * 2) Copy events => these should notify calculators so that calcs populate their structures
	 */
	virtual std::unique_ptr<ExecutionGraph> getCopyUpTo(const VectorClock &v) const;

	std::unique_ptr<ExecutionGraph> clone() const
	{
		return getCopyUpTo(*getViewFromStamp(getMaxStamp()));
	}

	/* Overloaded operators */
	friend llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const ExecutionGraph &g);

protected:
	static auto indirect(const std::unique_ptr<EventLabel> &ptr) -> EventLabel &
	{
		return *ptr;
	}

	void resizeThread(unsigned int tid, unsigned int size) { events[tid].resize(size); };
	void resizeThread(Event pos) { resizeThread(pos.thread, pos.index); }

	void setEventLabel(Event e, std::unique_ptr<EventLabel> lab)
	{
		events[e.thread][e.index] = std::move(lab);
	};

	/* Returns the next available stamp (and increases the counter) */
	Stamp nextStamp() { return timestamp++; }

	/* Resets the next available stamp to the specified value */
	void resetStamp(Stamp val) { timestamp = val; }

	/* Returns the event with the minimum stamp in ES.
	 * If ES is empty, returns INIT */
	Event getMinimumStampEvent(const std::vector<Event> &es) const;

	void trackCoherenceAtLoc(SAddr addr);

	void copyGraphUpTo(ExecutionGraph &other, const VectorClock &v) const;

	void addInitRfToLoc(ReadLabel *rLab) { getInitLabel()->addReader(rLab); }

	void removeInitRfToLoc(ReadLabel *rLab)
	{
		getInitLabel()->removeReader(rLab->getAddr(),
					     [&](auto &lab) { return &lab == rLab; });
	}

	void removeAfter(const VectorClock &preds);

	static std::unique_ptr<EmptyLabel> createHoleLabel(Event pos)
	{
		auto lab = EmptyLabel::create(pos);
		lab->setViews({{}});
		lab->setCalculated({{}});
		return lab;
	}

protected:
	/* A collection of threads and the events for each threads */
	ThreadList events{};

	/* The next available timestamp */
	Stamp timestamp = 0;

	LocMap coherence{};

	llvm::simple_ilist<EventLabel> insertionOrder{};

	/* Pers: The ID of the recovery routine.
	 * It should be -1 if not in recovery mode, or have the
	 * value of the recovery routine otherwise. */
	int recoveryTID = -1;

	InitValGetter initValGetter_;
};

namespace std {
template <> struct hash<ExecutionGraph> {
	auto operator()(const ExecutionGraph &g) const -> size_t
	{
		std::size_t hash = 0;

		/* Use a fixed (non-insertion-order-dependent) iteration order */
		hash_combine(hash, g.getNumThreads());
		for (auto i = 0U; i < g.getNumThreads(); i++) {
			hash_combine(hash, g.getThreadSize(i));
			for (auto j = 0U; j < g.getThreadSize(i); j++) {
				auto *lab = g.getEventLabel(Event(i, j));
				if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab)) {
					hash_combine(hash, rLab->getRf() ? rLab->getRf()->getPos()
									 : Event::getBottom());
				}
				if (auto *wLab = llvm::dyn_cast<WriteLabel>(lab)) {
					auto *pLab = g.co_imm_pred(wLab);
					hash_combine(hash,
						     pLab ? pLab->getPos() : Event::getInit());
				}
			}
		}
		return hash;
	}
};
} // namespace std

#endif /* GENMC_EXECUTION_GRAPH_HPP */
