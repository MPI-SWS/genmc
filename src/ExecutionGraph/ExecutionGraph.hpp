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

#include "ADT/VectorClock.hpp"
#include "ExecutionGraph/Event.hpp"
#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/Stamp.hpp"
#include "Support/Hash.hpp"
#include "config.h"
#include <llvm/ADT/StringMap.h>

#include <functional>
#include <memory>
#include <ranges>
#include <unordered_map>

// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)

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
	using PoList = llvm::simple_ilist<EventLabel, llvm::ilist_tag<po_tag>>;
	using PoLists = std::vector<PoList>;

	ExecutionGraph(InitValGetter f) : initValGetter_(std::move(f))
	{
		/* Create an entry for main() and push the "initializer" label */
		events.emplace_back();
		poLists.emplace_back();
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
	using reverse_label_iterator = llvm::simple_ilist<EventLabel>::reverse_iterator;
	using const_reverse_label_iterator = llvm::simple_ilist<EventLabel>::const_reverse_iterator;

	using co_iterator = StoreList::iterator;
	using const_co_iterator = StoreList::const_iterator;
	using reverse_co_iterator = StoreList::reverse_iterator;
	using const_reverse_co_iterator = StoreList::const_reverse_iterator;

	using initrf_iterator = InitLabel::rf_iterator;
	using const_initrf_iterator = InitLabel::const_rf_iterator;

	using po_iterator = PoList::iterator;
	using const_po_iterator = PoList::const_iterator;
	using reverse_po_iterator = PoList::reverse_iterator;
	using const_reverse_po_iterator = PoList::const_reverse_iterator;

	auto begin() -> iterator { return events.begin(); };
	auto end() -> iterator { return events.end(); };
	auto begin() const -> const_iterator { return events.begin(); };
	auto end() const -> const_iterator { return events.end(); };

	auto rbegin() -> reverse_iterator { return events.rbegin(); };
	auto rend() -> reverse_iterator { return events.rend(); };
	auto rbegin() const -> const_reverse_iterator { return events.rbegin(); };
	auto rend() const -> const_reverse_iterator { return events.rend(); };

	auto label_begin() const { return insertionOrder.begin(); }
	auto label_end() const { return insertionOrder.end(); }
	auto labels() const { return std::views::all(insertionOrder); }

	auto label_begin() { return insertionOrder.begin(); }
	auto label_end() { return insertionOrder.end(); }
	auto labels() { return std::views::all(insertionOrder); }

	auto thr_ids() const { return std::views::iota(0, (int)getNumThreads()); }

	auto po(int tid) const { return std::views::all(poLists[tid]); }
	auto po(int tid) { return std::views::all(poLists[tid]); }

	auto po_succs(const EventLabel *lab) const
	{
		auto begIt = std::next(const_po_iterator(lab));
		auto endIt = poLists[lab->getThread()].end();
		return std::ranges::subrange(begIt, endIt);
	}
	auto po_succs(EventLabel *lab)
	{
		auto begIt = std::next(po_iterator(lab));
		auto endIt = poLists[lab->getThread()].end();
		return std::ranges::subrange(begIt, endIt);
	}

	auto po_preds(const EventLabel *lab) const
	{
		auto begIt = std::next(const_reverse_po_iterator(lab));
		auto endIt = poLists[lab->getThread()].rend();
		return std::ranges::subrange(begIt, endIt);
	}
	auto po_preds(EventLabel *lab)
	{
		auto begIt = std::next(reverse_po_iterator(lab));
		auto endIt = poLists[lab->getThread()].rend();
		return std::ranges::subrange(begIt, endIt);
	}

	/* Returns the label in the previous position of E.
	 * Returns nullptr if E is the first event of a thread */
	auto po_imm_pred(const EventLabel *lab) const -> const EventLabel *
	{
		auto labIt = const_po_iterator(lab);
		auto begIt = poLists[lab->getThread()].begin();
		return labIt == begIt ? nullptr : &*std::prev(labIt);
	}
	auto po_imm_pred(EventLabel *lab) -> EventLabel *
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).po_imm_pred(lab));
	}

	/* Returns the label in the next position of E.
	 * Returns nullptr if E is the last event of a thread */
	auto po_imm_succ(const EventLabel *lab) const -> const EventLabel *
	{
		auto rLabIt = const_reverse_po_iterator(lab);
		auto rBegIt = poLists[lab->getThread()].rbegin();
		return rLabIt == rBegIt ? nullptr : &*std::prev(rLabIt);
	}
	auto po_imm_succ(EventLabel *lab) -> EventLabel *
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).po_imm_succ(lab));
	}

	auto loc_begin() -> loc_iterator { return coherence.begin(); }
	auto loc_begin() const -> const_loc_iterator { return coherence.begin(); };
	auto loc_end() -> loc_iterator { return coherence.end(); }
	auto loc_end() const -> const_loc_iterator { return coherence.end(); }

	auto co_begin(SAddr addr) -> co_iterator { return coherence[addr].begin(); }
	auto co_begin(SAddr addr) const -> const_co_iterator { return coherence.at(addr).begin(); };
	auto co_end(SAddr addr) -> co_iterator { return coherence[addr].end(); }
	auto co_end(SAddr addr) const -> const_co_iterator { return coherence.at(addr).end(); }
	auto co(SAddr addr) { return std::views::all(coherence[addr]); }
	auto co(SAddr addr) const { return std::views::all(coherence.at(addr)); }

	auto co_rbegin(SAddr addr) -> reverse_co_iterator { return coherence[addr].rbegin(); }
	auto co_rbegin(SAddr addr) const -> const_reverse_co_iterator
	{
		return coherence.at(addr).rbegin();
	};
	auto co_rend(SAddr addr) -> reverse_co_iterator { return coherence[addr].rend(); }
	auto co_rend(SAddr addr) const -> const_reverse_co_iterator
	{
		return coherence.at(addr).rend();
	}
	auto rco(SAddr addr) { return std::views::all(coherence[addr]) | std::views::reverse; }
	auto rco(SAddr addr) const
	{
		return std::views::all(coherence.at(addr)) | std::views::reverse;
	}

	auto init_rf_begin(SAddr addr) -> initrf_iterator { return getInitLabel()->rf_begin(addr); }
	auto init_rf_begin(SAddr addr) const -> const_initrf_iterator
	{
		return getInitLabel()->rf_begin(addr);
	};
	auto init_rf_end(SAddr addr) -> initrf_iterator { return getInitLabel()->rf_end(addr); }
	auto init_rf_end(SAddr addr) const -> const_initrf_iterator
	{
		return getInitLabel()->rf_end(addr);
	}

	auto co_succ_begin(WriteLabel *lab) -> co_iterator { return ++co_iterator(lab); }
	auto co_succ_begin(const WriteLabel *lab) const -> const_co_iterator
	{
		return ++const_co_iterator(lab);
	}
	auto co_succ_end(WriteLabel *lab) -> co_iterator { return co_end(lab->getAddr()); }
	auto co_succ_end(const WriteLabel *lab) const -> const_co_iterator
	{
		return co_end(lab->getAddr());
	}
	auto co_imm_succ(const WriteLabel *lab) const -> const WriteLabel *
	{
		auto it = co_succ_begin(lab);
		return it == co_succ_end(lab) ? nullptr : &*it;
	}

	auto co_pred_begin(WriteLabel *lab) -> reverse_co_iterator
	{
		return ++reverse_co_iterator(lab);
	}
	auto co_pred_begin(const WriteLabel *lab) const -> const_reverse_co_iterator
	{
		return ++const_reverse_co_iterator(lab);
	}
	auto co_pred_end(WriteLabel *lab) -> reverse_co_iterator { return co_rend(lab->getAddr()); }
	auto co_pred_end(const WriteLabel *lab) const -> const_reverse_co_iterator
	{
		return co_rend(lab->getAddr());
	}
	auto co_imm_pred(const WriteLabel *lab) const -> const WriteLabel *
	{
		auto it = co_pred_begin(lab);
		return it == co_pred_end(lab) ? nullptr : &*(it);
	}
	auto co_imm_pred(WriteLabel *lab) -> WriteLabel *
	{
		return const_cast<WriteLabel *>(
			static_cast<const ExecutionGraph &>(*this).co_imm_pred(lab));
	}

	auto co_max(SAddr addr) const -> const EventLabel *
	{
		return co_begin(addr) == co_end(addr) ? (EventLabel *)getInitLabel()
						      : (EventLabel *)&*co_rbegin(addr);
	}
	auto co_max(SAddr addr) -> EventLabel *
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).co_max(addr));
	}

	auto fr_succ_begin(ReadLabel *rLab) -> co_iterator
	{
		auto *wLab = llvm::dyn_cast<WriteLabel>(rLab->getRf());
		return wLab ? co_succ_begin(wLab) : co_begin(rLab->getAddr());
	}
	auto fr_succ_begin(const ReadLabel *rLab) const -> const_co_iterator
	{
		auto *wLab = llvm::dyn_cast<WriteLabel>(rLab->getRf());
		return wLab ? co_succ_begin(wLab) : co_begin(rLab->getAddr());
	}
	auto fr_succ_end(ReadLabel *rLab) -> co_iterator { return co_end(rLab->getAddr()); }
	auto fr_succ_end(const ReadLabel *rLab) const -> const_co_iterator
	{
		return co_end(rLab->getAddr());
	}
	auto fr_imm_succ(const ReadLabel *rLab) const -> const WriteLabel *
	{
		auto it = fr_succ_begin(rLab);
		return it == fr_succ_end(rLab) ? nullptr : &*it;
	}

	auto fr_imm_pred_begin(WriteLabel *wLab) -> WriteLabel::rf_iterator
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_begin(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_begin();
	}
	auto fr_imm_pred_begin(const WriteLabel *wLab) const -> WriteLabel::const_rf_iterator
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_begin(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_begin();
	}
	auto fr_imm_pred_end(WriteLabel *wLab) -> WriteLabel::rf_iterator
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_end(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_end();
	}
	auto fr_imm_pred_end(const WriteLabel *wLab) const -> WriteLabel::const_rf_iterator
	{
		return co_pred_begin(wLab) == co_pred_end(wLab)
			       ? init_rf_end(wLab->getAddr())
			       : (*co_pred_begin(wLab)).readers_end();
	}

	/* Thread-related methods */

	/* Creates a new thread in the execution graph */
	void addNewThread()
	{
		events.emplace_back();
		poLists.emplace_back();
	};

	/* Pers: Add/remove a thread for the recovery procedure */
	void addRecoveryThread()
	{
		recoveryTID = events.size();
		events.emplace_back();
		poLists.emplace_back();
	};
	void delRecoveryThread()
	{
		events.pop_back();
		poLists.pop_back();
		recoveryTID = -1;
	};

	/* Returns the tid of the recovery routine.
	 * If not in recovery mode, returns -1 */
	auto getRecoveryRoutineId() const -> int { return recoveryTID; };

	/* Returns the number of threads currently in the graph */
	auto getNumThreads() const -> unsigned int { return events.size(); };

	/* Returns the size of the thread tid */
	auto getThreadSize(int tid) const -> unsigned int { return events[tid].size(); };

	/* Returns true if the thread tid is empty */
	auto isThreadEmpty(int tid) const -> bool { return getThreadSize(tid) == 0; };

	/* Event addition/removal methods */

	auto getInitLabel() const -> const InitLabel *
	{
		return static_cast<const InitLabel *>(getEventLabel(Event(0, 0)));
	}
	auto getInitLabel() -> InitLabel *
	{
		return const_cast<InitLabel *>(
			static_cast<const ExecutionGraph &>(*this).getInitLabel());
	}

	/* Returns the maximum stamp used */
	auto getMaxStamp() const -> Stamp { return timestamp; }

	/* Adds LAB to the graph. If a label exists in the respective
	 * position, it is replaced.
	 * (Maintains well-formedness for read removals.) */
	auto addLabelToGraph(std::unique_ptr<EventLabel> lab) -> EventLabel *;

	/* Removes the last event from THREAD.
	 * If it is a read, updates the rf-lists.
	 * If it is a write, makes all readers read BOT. */
	void removeLast(unsigned int thread);

	/* Event getter methods */

	/* Returns the label in the position denoted by event e */
	auto getEventLabel(Event e) const -> const EventLabel *
	{
		return events[e.thread][e.index].get();
	}
	auto getEventLabel(Event e) -> EventLabel *
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).getEventLabel(e));
	}

	/* Returns a label as a ReadLabel.
	 * If the passed event is not a read, returns nullptr  */
	auto getReadLabel(Event e) const -> const ReadLabel *
	{
		return llvm::dyn_cast<ReadLabel>(getEventLabel(e));
	}
	auto getReadLabel(Event e) -> ReadLabel *
	{
		return const_cast<ReadLabel *>(
			static_cast<const ExecutionGraph &>(*this).getReadLabel(e));
	}

	/* Returns a label as a WriteLabel.
	 * If the passed event is not a write, returns nullptr  */
	auto getWriteLabel(Event e) const -> const WriteLabel *
	{
		return llvm::dyn_cast<WriteLabel>(getEventLabel(e));
	}
	auto getWriteLabel(Event e) -> WriteLabel *
	{
		return const_cast<WriteLabel *>(
			static_cast<const ExecutionGraph &>(*this).getWriteLabel(e));
	}

	/* Returns the first label in the thread tid */
	auto getFirstThreadLabel(int tid) const -> const ThreadStartLabel *
	{
		return llvm::dyn_cast<ThreadStartLabel>(getEventLabel(Event(tid, 0)));
	}
	auto getFirstThreadLabel(int tid) -> ThreadStartLabel *
	{
		return const_cast<ThreadStartLabel *>(
			static_cast<const ExecutionGraph &>(*this).getFirstThreadLabel(tid));
	}

	/* Returns the last label in the thread tid */
	auto getLastThreadLabel(int thread) const -> const EventLabel *
	{
		return poLists[thread].empty() ? nullptr : &poLists[thread].back();
	}
	auto getLastThreadLabel(int thread) -> EventLabel *
	{
		return const_cast<EventLabel *>(
			static_cast<const ExecutionGraph &>(*this).getLastThreadLabel(thread));
	}

	/* Boolean helper functions */

	auto getInitVal(const AAccess &access) const -> SVal { return initValGetter_(access); }

	void setInitValGetter(InitValGetter f) { initValGetter_ = std::move(f); }

	auto isLocEmpty(SAddr addr) const -> bool { return co_begin(addr) == co_end(addr); }

	/* Whether a location has more than one store */
	auto hasLocMoreThanOneStore(SAddr addr) const -> bool
	{
		return !isLocEmpty(addr) && ++co_begin(addr) != co_end(addr);
	}

	/* Returns true if the graph contains e */
	auto containsPos(const Event &e) const -> bool
	{
		return e.thread >= 0 && e.thread < getNumThreads() && e.index >= 0 &&
		       e.index < getThreadSize(e.thread);
	}
	auto containsLab(const EventLabel *lab) const -> bool
	{
		return containsPos(lab->getPos()) && getEventLabel(lab->getPos()) == lab;
	}

	/* Returns true if the graph contains e, and the label is not EMPTY */
	auto containsPosNonEmpty(const Event &e) const -> bool
	{
		return containsPos(e) && !llvm::isa<EmptyLabel>(getEventLabel(e));
	}

	/* Debugging methods */

	void validate();

	/* Graph modification methods */

	void addAlloc(MallocLabel *aLab, MemAccessLabel *mLab);

	/* Prefix saving and restoring */

	/* Returns a vector clock representing the events added before e */
	auto getPredsView(Event e) const -> std::unique_ptr<VectorClock>
	{
		auto stamp = getEventLabel(e)->getStamp();
		return getViewFromStamp(stamp);
	}

	/* Graph cutting */

	/* Returns a view of the graph representing events with stamp <= st */
	virtual auto getViewFromStamp(Stamp st) const -> std::unique_ptr<VectorClock>;

	/* Cuts a graph so that it only contains events with stamp <= st */
	virtual void cutToStamp(Stamp st);

	/* FIXME: Use value ptrs? (less error-prone than using explicit copy fun) */
	/* Or maybe simply consolidate the copying procedure:
	 * 1) Copy graph structure (calculators, constant members, etc)
	 * 2) Copy events => these should notify calculators so that calcs populate their structures
	 */
	virtual auto getCopyUpTo(const VectorClock &v) const -> std::unique_ptr<ExecutionGraph>;

	auto clone() const -> std::unique_ptr<ExecutionGraph>
	{
		return getCopyUpTo(*getViewFromStamp(getMaxStamp()));
	}

	/* Overloaded operators */
	friend auto operator<<(llvm::raw_ostream &s, const ExecutionGraph &g)
		-> llvm::raw_ostream &;

protected:
	friend class WriteLabel;

	static auto indirect(const std::unique_ptr<EventLabel> &ptr) -> EventLabel &
	{
		return *ptr;
	}

	void setEventLabel(Event e, std::unique_ptr<EventLabel> lab)
	{
		events[e.thread][e.index] = std::move(lab);
	};

	/* Returns the next available stamp (and increases the counter) */
	auto nextStamp() -> Stamp { return timestamp++; }

	/* Resets the next available stamp to the specified value */
	void resetStamp(Stamp val) { timestamp = val; }

	/* Returns the event with the minimum stamp in ES.
	 * If ES is empty, returns INIT */
	[[nodiscard]] auto getMinimumStampEvent(const std::vector<const EventLabel *> &es) const
		-> const EventLabel *;

	void trackCoherenceAtLoc(SAddr addr);

	void copyGraphUpTo(ExecutionGraph &other, const VectorClock &v) const;

	void addInitRfToLoc(ReadLabel *rLab) { getInitLabel()->addReader(rLab); }

	void removeInitRfToLoc(ReadLabel *rLab)
	{
		getInitLabel()->removeReader(rLab->getAddr(),
					     [&](auto &lab) { return &lab == rLab; });
	}

	void removeAfter(const VectorClock &preds);

	static auto createHoleLabel(Event pos) -> std::unique_ptr<EmptyLabel>
	{
		auto lab = EmptyLabel::create(pos);
		lab->setViews({{}});
		lab->setCalculated({{}});
		return lab;
	}

	/* A collection of threads and the events for each threads */
	ThreadList events;

	/* The next available timestamp */
	Stamp timestamp = 0;

	LocMap coherence;

	llvm::simple_ilist<EventLabel> insertionOrder;

	PoLists poLists{};

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
			for (const auto &lab : g.po(i)) {
				if (const auto *rLab = llvm::dyn_cast<ReadLabel>(&lab)) {
					hash_combine(hash, rLab->getRf() ? rLab->getRf()->getPos()
									 : Event::getBottom());
				}
				if (const auto *wLab = llvm::dyn_cast<WriteLabel>(&lab)) {
					const auto *pLab = g.co_imm_pred(wLab);
					hash_combine(hash,
						     pLab ? pLab->getPos() : Event::getInit());
				}
			}
		}
		return hash;
	}
};
} // namespace std

// NOLINTEND(cppcoreguidelines-pro-type-const-cast)

#endif /* GENMC_EXECUTION_GRAPH_HPP */
