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

#ifndef GENMC_EXECUTION_GRAPH_HPP
#define GENMC_EXECUTION_GRAPH_HPP

#include "genmc/ADT/AdaptiveView.hpp"
#include "genmc/ADT/VectorClock.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/ADT/ilist.hpp"
#include "genmc/Execution/CBIterator.hpp"
#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionState.hpp"
#include "genmc/Execution/Stamp.hpp"
#include "genmc/Support/Hash.hpp"
#include "genmc/Support/MemAccess.hpp"
#include "genmc/Verification/VerificationError.hpp"

#include <algorithm>
#include <format>
#include <map>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <utility>

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
 *
 * NOTE: graphs cannot be passed across threads safely by default.
 * ctor arguments (initValGetter, consChecker) need to be adjusted.
 */
class ExecutionGraph {

public:
	/* Type definitions */
	using Thread = std::vector<std::unique_ptr<EventLabel>>;
	using ThreadList = std::vector<Thread>;
	using StoreList = genmc::ilist<WriteLabel>;
	using LocMap = std::unordered_map<SAddr, StoreList>;
	using AccessVector = std::vector<EventLabel *>;
	using AccessMap = std::unordered_map<SAddr, AccessVector>;
	using PoList = genmc::ilist<EventLabel, po_tag>;
	using PoLists = std::vector<PoList>;
	using IoList = genmc::ilist<EventLabel, io_tag>;

	/* Iterators */
	using iterator = ThreadList::iterator;
	using const_iterator = ThreadList::const_iterator;
	using label_iterator = IoList::iterator;
	using const_label_iterator = IoList::const_iterator;
	using reverse_label_iterator = IoList::reverse_iterator;
	using const_reverse_label_iterator = IoList::const_reverse_iterator;
	using loc_iterator = LocMap::iterator;
	using const_loc_iterator = LocMap::const_iterator;
	using po_iterator = PoList::iterator;
	using const_po_iterator = PoList::const_iterator;
	using reverse_po_iterator = PoList::reverse_iterator;
	using const_reverse_po_iterator = PoList::const_reverse_iterator;
	using co_iterator = StoreList::iterator;
	using const_co_iterator = StoreList::const_iterator;
	using reverse_co_iterator = StoreList::reverse_iterator;
	using const_reverse_co_iterator = StoreList::const_reverse_iterator;

	/**
	 * A configuration class for the graph coupling together
	 * construction information.
	 */
	struct Config {
		ExecutionState *execState{};
		ConsistencyChecker *consChecker{};
		bool emitNALabels{};
	};

	/**
	 * A class to do scoped insertions in an execution graph:
	 * labels are added during construction and removed during destruction.
	 * Ownership is temporarily transfered to the graph, and returned
	 * back upon destruction.
	 *
	 * The class is parameterized over LabelT so that we can insert
	 * unique pointers of the EventLabel subclasses.
	 */
	template <typename LabelT> class ScopedLabel {
	public:
		ScopedLabel(ExecutionGraph &g, std::unique_ptr<LabelT> &labRef)
			: graph_(g), labelRef_(labRef)
		{
			VERIFY(labelRef_);

			/* Cache raw pointer for removal and add to the graph */
			label_ = graph_.add(std::move(labelRef_));
		}

		ScopedLabel(const ScopedLabel & /*other*/) = delete;
		ScopedLabel(ScopedLabel && /*other*/) = delete;
		~ScopedLabel()
		{
			if (commited_)
				return;

			auto poppedUP = graph_.removeLast(label_->getThread());
			labelRef_.reset(genmc::cast<LabelT>(poppedUP.release()));
		}

		[[nodiscard]] auto get() const -> LabelT * { return label_; }
		auto operator->() const -> LabelT * { return label_; }

		void commit() { commited_ = true; }

		auto operator=(const ScopedLabel & /*other*/) -> ScopedLabel & = delete;
		auto operator=(ScopedLabel && /*other*/) -> ScopedLabel & = delete;

	private:
		ExecutionGraph &graph_;
		std::unique_ptr<LabelT> &labelRef_;
		LabelT *label_{};
		bool commited_{};
	};

	/* Constructors */
	ExecutionGraph(Config cfg)
		: consChecker_(cfg.consChecker), haveNAs_(cfg.emitNALabels), state_(cfg.execState)
	{
		/* Create an entry for main() and push the "initializer" label */
		events.emplace_back();
		poLists.emplace_back();
		auto *iLab = addLabelToGraph(InitLabel::create());
		iLab->setCalculated({{}});
		iLab->setViews({{View(), View(), View()}}); // FIXME
		iLab->setPrefixView(std::make_unique<View>());
	}

	ExecutionGraph(const ExecutionGraph &) = delete;
	ExecutionGraph(ExecutionGraph &&) noexcept = default;
	auto operator=(const ExecutionGraph &) -> ExecutionGraph & = delete;
	auto operator=(ExecutionGraph &&) noexcept -> ExecutionGraph & = default;
	virtual ~ExecutionGraph() = default;

	/***************************************************************************
	 * Thread & label traversal
	 **************************************************************************/

	auto begin() -> iterator { return events.begin(); };
	auto end() -> iterator { return events.end(); };
	auto begin() const -> const_iterator { return events.begin(); };
	auto end() const -> const_iterator { return events.end(); };

	auto label_begin() const { return insertionOrder.begin(); }
	auto label_end() const { return insertionOrder.end(); }
	auto labels() const { return std::views::all(insertionOrder); }
	auto labels() { return std::views::all(insertionOrder); }

	auto rlabels() const { return std::views::reverse(labels()); }
	auto rlabels() { return std::views::reverse(labels()); }

	auto other_labels(const EventLabel *lab) const
	{
		return labels() | std::views::filter([lab](auto &olab) {
			       return olab.getPos() != lab->getPos();
		       });
	}

	auto thr_ids() const { return std::views::iota(0, getNumThreads()); }

	auto loc_begin() -> loc_iterator { return coherence.begin(); }
	auto loc_begin() const -> const_loc_iterator { return coherence.begin(); };
	auto loc_end() -> loc_iterator { return coherence.end(); }
	auto loc_end() const -> const_loc_iterator { return coherence.end(); }

	/***************************************************************************
	 * Program Order (PO)
	 **************************************************************************/

	auto po(int tid) const { return std::views::all(poLists[tid]); }
	auto po(int tid) { return std::views::all(poLists[tid]); }

	auto po_succs(const EventLabel *lab) const
	{
		auto begIt = ++const_po_iterator(lab);
		auto endIt = poLists[lab->getThread()].end();
		return std::ranges::subrange(begIt, endIt);
	}
	auto po_succs(EventLabel *lab)
	{
		auto begIt = ++po_iterator(lab);
		auto endIt = poLists[lab->getThread()].end();
		return std::ranges::subrange(begIt, endIt);
	}

	auto po_preds(const EventLabel *lab) const
	{
		auto begIt = ++const_reverse_po_iterator(lab);
		auto endIt = poLists[lab->getThread()].rend();
		return std::ranges::subrange(begIt, endIt);
	}
	auto po_preds(EventLabel *lab)
	{
		auto begIt = ++reverse_po_iterator(lab);
		auto endIt = poLists[lab->getThread()].rend();
		return std::ranges::subrange(begIt, endIt);
	}

	/* Returns the label in the previous position of E.
	 * Returns nullptr if E is the first event of a thread */
	auto po_imm_pred(const EventLabel *lab) const -> const EventLabel *
	{
		auto labIt = const_po_iterator(lab);
		auto begIt = poLists[lab->getThread()].begin();
		return labIt == begIt ? nullptr : &*--labIt;
	}
	auto po_imm_pred(EventLabel *lab) -> EventLabel *
	{
		return const_cast<EventLabel *>(std::as_const(*this).po_imm_pred(lab));
	}

	/* Returns the label in the next position of E.
	 * Returns nullptr if E is the last event of a thread */
	auto po_imm_succ(const EventLabel *lab) const -> const EventLabel *
	{
		auto rLabIt = const_reverse_po_iterator(lab);
		auto rBegIt = poLists[lab->getThread()].rbegin();
		return rLabIt == rBegIt ? nullptr : &*--rLabIt;
	}
	auto po_imm_succ(EventLabel *lab) -> EventLabel *
	{
		return const_cast<EventLabel *>(std::as_const(*this).po_imm_succ(lab));
	}

	/***************************************************************************
	 * CB - Causal predecessors
	 **************************************************************************/

	auto cb_preds(const EventLabel *lab) const
	{
		return std::ranges::subrange(CBIterator(lab), CBIterator());
	}

	/***************************************************************************
	 * Program Order - Location (POLOC)
	 **************************************************************************/

	auto poloc_succs(const MemAccessLabel *lab) const
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };
		return po_succs(lab) | std::views::filter(locFilter);
	}
	auto poloc_succs(MemAccessLabel *lab)
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };
		return po_succs(lab) | std::views::filter(locFilter);
	}

	auto poloc_succs(const EventLabel *lab) const
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };

		if (const auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab))
			return po_succs(mLab) | std::views::filter(locFilter);
		return std::ranges::subrange<const_po_iterator>{} | std::views::filter(locFilter);
	}
	auto poloc_succs(EventLabel *lab)
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };

		if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab))
			return po_succs(mLab) | std::views::filter(locFilter);
		return std::ranges::subrange<po_iterator>{} | std::views::filter(locFilter);
	}

	auto poloc_imm_succ(const EventLabel *lab) const -> const EventLabel *
	{
		auto succs = poloc_succs(lab);
		return succs.empty() ? nullptr : &*succs.begin();
	}

	auto poloc_preds(const MemAccessLabel *lab) const
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };
		return po_preds(lab) | std::views::filter(locFilter);
	}
	auto poloc_preds(MemAccessLabel *lab)
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };
		return po_preds(lab) | std::views::filter(locFilter);
	}

	auto poloc_preds(const EventLabel *lab) const
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };

		if (const auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab))
			return po_preds(mLab) | std::views::filter(locFilter);
		return std::ranges::subrange<const_reverse_po_iterator>{} |
		       std::views::filter(locFilter);
	}
	auto poloc_preds(EventLabel *lab)
	{
		auto locFilter = [lab](const auto &oLab) { return isSameLoc(lab, &oLab); };

		if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab))
			return po_preds(mLab) | std::views::filter(locFilter);
		return std::ranges::subrange<reverse_po_iterator>{} | std::views::filter(locFilter);
	}

	auto poloc_imm_pred(const EventLabel *lab) const -> const EventLabel *
	{
		auto preds = poloc_preds(lab);
		return preds.empty() ? nullptr : &*preds.begin();
	}

	/***************************************************************************
	 * Detour
	 **************************************************************************/

	auto detour_succs(const WriteLabel *lab) const
	{
		return po_succs(lab) | std::views::filter(DetourSuccFilter(lab, lab->getPos()));
	}
	auto detour_succs(WriteLabel *lab)
	{
		return po_succs(lab) | std::views::filter(DetourSuccFilter(lab, lab->getPos()));
	}

	auto detour_succs(const EventLabel *lab) const
	{
		/* decltype works here because DetourSuccFilter is default constructible */
		using RangeType = decltype(detour_succs(static_cast<const WriteLabel *>(nullptr)));
		const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? detour_succs(wLab) : RangeType{};
	}
	auto detour_succs(EventLabel *lab)
	{
		using RangeType = decltype(detour_succs(static_cast<WriteLabel *>(nullptr)));
		auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? detour_succs(wLab) : RangeType{};
	}

	auto detour_preds(const ReadLabel *lab) const
	{
		const Event wPos = lab->getRf() ? lab->getRf()->getPos() : Event::getInit();
		return po_preds(lab) | std::views::filter(DetourPredFilter(lab, wPos));
	}
	auto detour_preds(ReadLabel *lab)
	{
		const Event wPos = lab->getRf() ? lab->getRf()->getPos() : Event::getInit();
		return po_preds(lab) | std::views::filter(DetourPredFilter(lab, wPos));
	}

	auto detour_preds(const EventLabel *lab) const
	{
		using RangeType = decltype(detour_preds(static_cast<const ReadLabel *>(nullptr)));
		const auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
		return rLab ? detour_preds(rLab) : RangeType{};
	}
	auto detour_preds(EventLabel *lab)
	{
		using RangeType = decltype(detour_preds(static_cast<ReadLabel *>(nullptr)));
		auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
		return rLab ? detour_preds(rLab) : RangeType{};
	}

	/***************************************************************************
	 * Preserved Program Order (PPO)
	 **************************************************************************/

#define DEFINE_PPO_ACC(NAME)                                                                       \
	auto NAME##_preds(const EventLabel *lab) const                                             \
	{                                                                                          \
		return lab->NAME() | std::views::transform([this](Event e) -> const EventLabel & { \
			       return *getEventLabel(e);                                           \
		       });                                                                         \
	}                                                                                          \
	auto NAME##_preds(EventLabel *lab)                                                         \
	{                                                                                          \
		return lab->NAME() | std::views::transform([this](Event e) -> EventLabel & {       \
			       return *getEventLabel(e);                                           \
		       });                                                                         \
	}                                                                                          \
	auto NAME##_preds(Event e) const { return NAME##_preds(getEventLabel(e)); }

	DEFINE_PPO_ACC(data);
	DEFINE_PPO_ACC(addr);
	DEFINE_PPO_ACC(ctrl);
#undef DEFINE_PPO_ACC

	/***************************************************************************
	 * Thread Creation / Join
	 **************************************************************************/

	auto tc_succ(const EventLabel *lab) const -> const ThreadStartLabel *
	{
		const auto *tcLab = genmc::dyn_cast<ThreadCreateLabel>(lab);
		return tcLab ? getFirstThreadLabel(tcLab->getChildId()) : nullptr;
	}

	auto tc_pred(const EventLabel *lab) const -> const ThreadCreateLabel *
	{
		const auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(lab);
		return tsLab ? tsLab->getCreate() : nullptr;
	}

	auto tj_succ(const EventLabel *lab) const -> const ThreadJoinLabel *
	{
		const auto *eLab = genmc::dyn_cast<ThreadFinishLabel>(lab);
		return eLab ? eLab->getParentJoin() : nullptr;
	}

	auto tj_pred(const EventLabel *lab) const -> const ThreadFinishLabel *
	{
		const auto *tjLab = genmc::dyn_cast<ThreadJoinLabel>(lab);
		return tjLab ? genmc::dyn_cast_if_present<ThreadFinishLabel>(
				       getLastThreadLabel(tjLab->getChildId()))
			     : nullptr;
	}

	/***************************************************************************
	 * Read-From (RF)
	 **************************************************************************/

	auto rf_succs(const WriteLabel *lab) const { return std::ranges::subrange(lab->readers()); }
	auto rf_succs(WriteLabel *lab) { return std::ranges::subrange(lab->readers()); }

	auto rf_succs(const EventLabel *lab) const
	{
		using RangeType = decltype(rf_succs(static_cast<const WriteLabel *>(nullptr)));
		const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? rf_succs(wLab) : RangeType{};
	}
	auto rf_succs(EventLabel *lab)
	{
		using RangeType = decltype(rf_succs(static_cast<WriteLabel *>(nullptr)));
		auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? rf_succs(wLab) : RangeType{};
	}

	auto rf_pred(const ReadLabel *lab) const -> const EventLabel *
	{
		return (!lab || !lab->getRf()) ? nullptr : lab->getRf();
	}

	auto rf_pred(const EventLabel *lab) const -> const EventLabel *
	{
		const auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
		return rLab ? rf_pred(rLab) : nullptr;
	}

	auto rfe_succs(const EventLabel *lab) const
	{
		return rf_succs(lab) | std::views::filter([lab](const auto &oLab) {
			       return oLab.getThread() != lab->getThread();
		       });
	}
	auto rfe_succs(EventLabel *lab)
	{
		return rf_succs(lab) | std::views::filter([lab](const auto &oLab) {
			       return oLab.getThread() != lab->getThread();
		       });
	}

	auto rfe_pred(const EventLabel *lab) const -> const EventLabel *
	{
		const auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
		return (rLab && rLab->readsExt()) ? rLab->getRf() : nullptr;
	}

	auto rfi_succs(const EventLabel *lab) const
	{
		return rf_succs(lab) | std::views::filter([lab](const auto &oLab) {
			       return oLab.getThread() == lab->getThread();
		       });
	}
	auto rfi_succs(EventLabel *lab)
	{
		return rf_succs(lab) | std::views::filter([lab](const auto &oLab) {
			       return oLab.getThread() == lab->getThread();
		       });
	}

	auto rfi_pred(const EventLabel *lab) const -> const EventLabel *
	{
		const auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
		return (rLab && rLab->readsInt()) ? rLab->getRf() : nullptr;
	}

	/***************************************************************************
	 * Coherence Order (CO)
	 **************************************************************************/

	auto co(SAddr addr) { return std::views::all(coherence[addr]); }
	auto co(SAddr addr) const { return std::views::all(coherence.at(addr)); }
	auto rco(SAddr addr) { return std::views::all(coherence[addr]) | std::views::reverse; }
	auto rco(SAddr addr) const
	{
		return std::views::all(coherence.at(addr)) | std::views::reverse;
	}

	auto co_succs(const WriteLabel *lab) const
	{
		return std::ranges::subrange(++const_co_iterator(lab),
					     coherence.at(lab->getAddr()).end());
	}
	auto co_succs(WriteLabel *lab)
	{
		return std::ranges::subrange(++co_iterator(lab), coherence[lab->getAddr()].end());
	}

	auto co_succs(const EventLabel *lab) const
	{
		using RangeType = decltype(co_succs(static_cast<const WriteLabel *>(nullptr)));
		const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? co_succs(wLab) : RangeType{};
	}
	auto co_succs(EventLabel *lab)
	{
		using RangeType = decltype(co_succs(static_cast<WriteLabel *>(nullptr)));
		auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? co_succs(wLab) : RangeType{};
	}

	auto co_imm_succ(const EventLabel *lab) const -> const WriteLabel *
	{
		auto succs = co_succs(lab);
		return succs.begin() == succs.end() ? nullptr : &*succs.begin();
	}

	auto co_preds(const WriteLabel *lab) const
	{
		return std::ranges::subrange(++const_reverse_co_iterator(lab),
					     coherence.at(lab->getAddr()).rend());
	}
	auto co_preds(WriteLabel *lab)
	{
		return std::ranges::subrange(++reverse_co_iterator(lab),
					     coherence[lab->getAddr()].rend());
	}

	auto co_preds(const EventLabel *lab) const
	{
		using RangeType = decltype(co_preds(static_cast<const WriteLabel *>(nullptr)));
		const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		VERIFY(!wLab || wLab->isInCo());
		return wLab ? co_preds(wLab) : RangeType{};
	}
	auto co_preds(EventLabel *lab)
	{
		using RangeType = decltype(co_preds(static_cast<WriteLabel *>(nullptr)));
		auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		VERIFY(!wLab || wLab->isInCo());
		return wLab ? co_preds(wLab) : RangeType{};
	}

	auto co_imm_pred(const EventLabel *lab) const -> const WriteLabel *
	{
		auto preds = co_preds(lab);
		return preds.begin() == preds.end() ? nullptr : &*preds.begin();
	}
	auto co_imm_pred(WriteLabel *lab) -> WriteLabel *
	{
		return const_cast<WriteLabel *>(std::as_const(*this).co_imm_pred(lab));
	}

	auto co_max(SAddr addr) const -> const EventLabel *
	{
		const auto &list = coherence.at(addr);
		return list.empty() ? (EventLabel *)getInitLabel() : (EventLabel *)&list.back();
	}
	auto co_max(SAddr addr) -> EventLabel *
	{
		return const_cast<EventLabel *>(std::as_const(*this).co_max(addr));
	}

	/***************************************************************************
	 * From-read (FR)
	 **************************************************************************/

	/* Fast path */
	auto fr_succs(const ReadLabel *lab) const
	{
		const auto *wLab = genmc::dyn_cast<WriteLabel>(lab->getRf());
		return wLab ? std::ranges::subrange(++const_co_iterator(wLab),
						    coherence.at(lab->getAddr()).end())
			    : std::ranges::subrange(coherence.at(lab->getAddr()).begin(),
						    coherence.at(lab->getAddr()).end());
	}
	auto fr_succs(ReadLabel *lab)
	{
		const auto *wLab = genmc::dyn_cast<WriteLabel>(lab->getRf());
		return wLab ? std::ranges::subrange(++co_iterator(const_cast<WriteLabel *>(wLab)),
						    coherence[lab->getAddr()].end())
			    : std::ranges::subrange(coherence[lab->getAddr()].begin(),
						    coherence[lab->getAddr()].end());
	}

	/* Slow path */
	auto fr_succs(const EventLabel *lab) const
	{
		using RangeType = decltype(fr_succs(static_cast<const ReadLabel *>(nullptr)));
		const auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
		return rLab ? fr_succs(rLab) : RangeType{};
	}
	auto fr_succs(EventLabel *lab)
	{
		using RangeType = decltype(fr_succs(static_cast<ReadLabel *>(nullptr)));
		auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
		return rLab ? fr_succs(rLab) : RangeType{};
	}

	/* Fast path */
	auto fr_imm_succ(const ReadLabel *lab) const -> const WriteLabel *
	{
		auto succs = fr_succs(lab);
		return succs.empty() ? nullptr : &*succs.begin();
	}
	auto fr_imm_succ(ReadLabel *lab) -> WriteLabel *
	{
		auto succs = fr_succs(lab);
		return succs.empty() ? nullptr : &*succs.begin();
	}

	/* Slow path */
	auto fr_imm_succ(const EventLabel *lab) const -> const WriteLabel *
	{
		auto succs = fr_succs(lab);
		return succs.empty() ? nullptr : &*succs.begin();
	}
	auto fr_imm_succ(EventLabel *lab) -> WriteLabel *
	{
		auto succs = fr_succs(lab);
		return succs.empty() ? nullptr : &*succs.begin();
	}

	/* Fast path */
	auto fr_imm_preds(const WriteLabel *lab) const
	{
		auto pIt = ++const_reverse_co_iterator(lab);
		const bool isInit = (pIt == coherence.at(lab->getAddr()).rend());

		return isInit ? std::ranges::subrange(getInitLabel()->rfs(lab->getAddr()))
			      : std::ranges::subrange(pIt->readers());
	}
	auto fr_imm_preds(WriteLabel *lab)
	{
		auto pIt = ++reverse_co_iterator(lab);
		const bool isInit = (pIt == coherence[lab->getAddr()].rend());

		return isInit ? std::ranges::subrange(getInitLabel()->rfs(lab->getAddr()))
			      : std::ranges::subrange(pIt->readers());
	}

	/* Slow path */
	auto fr_imm_preds(const EventLabel *lab) const
	{
		using RangeType = decltype(fr_imm_preds(static_cast<const WriteLabel *>(nullptr)));
		const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? fr_imm_preds(wLab) : RangeType{};
	}
	auto fr_imm_preds(EventLabel *lab)
	{
		using RangeType = decltype(fr_imm_preds(static_cast<WriteLabel *>(nullptr)));
		auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
		return wLab ? fr_imm_preds(wLab) : RangeType{};
	}

	/***************************************************************************
	 * Allocation / Free
	 **************************************************************************/

	auto alloc(const EventLabel *lab) const -> const EventLabel *
	{
		const auto *mLab = genmc::dyn_cast<MemLabel>(lab);
		if (!mLab || mLab->getAddr().isStatic())
			return nullptr;

		auto pos = getState().getAllocPos(mLab->getAddr());
		return getEventLabel(pos);
	}

	auto free(const EventLabel *lab) const -> const EventLabel *
	{
		const auto *mLab = genmc::dyn_cast<MemLabel>(lab);
		if (!mLab || mLab->getAddr().isStatic())
			return nullptr;

		auto freePos = getState().getFreePos(mLab->getAddr());
		auto isRetired = getState().isRetired(mLab->getAddr());
		return (!freePos || isRetired) ? nullptr : getEventLabel(*freePos);
	}

	auto retire(const EventLabel *lab) const -> const EventLabel *
	{
		const auto *mLab = genmc::dyn_cast<MemLabel>(lab);
		if (!mLab || mLab->getAddr().isStatic())
			return nullptr;

		auto freePos = getState().getFreePos(mLab->getAddr());
		auto isRetired = getState().isRetired(mLab->getAddr());
		return (!freePos || !isRetired) ? nullptr : getEventLabel(*freePos);
	}

	auto unprotected(const EventLabel *lab) const
	{
		auto getIndirect = [this](const auto &e) -> const EventLabel & {
			return *getEventLabel(e);
		};

		const auto *dLab = genmc::dyn_cast<MemLifecycleLabel>(lab);
		if (!dLab)
			return AdaptiveView() | std::views::transform(getIndirect);
		return getState().getMaxUnprotectedView(dLab->getAddr()) |
		       std::views::transform(getIndirect);
	}

	/* Piping below is safe (despite getMaxNAReadView returning a tmp) due
	 * to the tmp being wrapped into an owning_view */
#define DEFINE_POMAX_ITER(NAME, GETTER)                                                            \
	auto NAME(const EventLabel *lab) const                                                     \
	{                                                                                          \
		auto getIndirect = [this](const auto &e) -> const EventLabel & {                   \
			return *getEventLabel(e);                                                  \
		};                                                                                 \
		const auto *mLab = genmc::dyn_cast<MemLabel>(lab);                                 \
		if (!mLab)                                                                         \
			return AdaptiveView() | std::views::transform(getIndirect);                \
                                                                                                   \
		return getState().GETTER(mLab->getAccess()) | std::views::transform(getIndirect);  \
	}
	DEFINE_POMAX_ITER(pomax_na_reads, getMaxNAReadView)
	DEFINE_POMAX_ITER(pomax_na_writes, getMaxNAWriteEvent)
	DEFINE_POMAX_ITER(pomax_at_reads, getMaxAReadView)
	DEFINE_POMAX_ITER(pomax_at_writes, getMaxAWriteView)

	/***************************************************************************
	 * Linearization / Methods (lin)
	 **************************************************************************/

	auto lin_succs(const MethodEndLabel *lab) const
	{
		auto indirectBegin = [](auto *lab) -> MethodBeginLabel & { return *lab; };
		/* Normalize input to subrange first */
		return std::ranges::subrange(lab->lin_succs()) |
		       std::views::transform(indirectBegin);
	}
	auto lin_succs(const EventLabel *lab) const
	{
		using RangeType = decltype(lin_succs(static_cast<const MethodEndLabel *>(nullptr)));
		const auto *endLab = genmc::dyn_cast<MethodEndLabel>(lab);
		return endLab ? lin_succs(endLab) : RangeType{};
	}

	auto lin_preds(const MethodBeginLabel *lab) const
	{
		auto indirectEnd = [](auto *lab) -> MethodEndLabel & { return *lab; };
		return std::ranges::subrange(lab->lin_preds()) | std::views::transform(indirectEnd);
	}
	auto lin_preds(const EventLabel *lab) const
	{
		using RangeType =
			decltype(lin_preds(static_cast<const MethodBeginLabel *>(nullptr)));
		const auto *begLab = genmc::dyn_cast<MethodBeginLabel>(lab);
		return begLab ? lin_preds(begLab) : RangeType{};
	}

	/***************************************************************************
	 * Utilities & Helpers
	 **************************************************************************/

	auto samelocs(const EventLabel *lab) const
	{
		auto isSameLabel = [lab](const EventLabel *oLab) { return lab != oLab; };
		auto cIndirect = [](auto *lab) -> EventLabel & { return *lab; };
		static const std::vector<EventLabel *> accessSentinelVector;

		if (const auto *mLab = genmc::dyn_cast<MemLabel>(lab))
			return accessMap_.at(mLab->getAddr()) | std::views::filter(isSameLabel) |
			       std::views::transform(cIndirect);
		return accessSentinelVector | std::views::filter(isSameLabel) |
		       std::views::transform(cIndirect);
	}

	/* Basic getters */

	/* Returns the maximum stamp used */
	auto getMaxStamp() const -> Stamp { return timestamp - 1; }

	/* Returns the consistency checke for this graph */
	auto getConsChecker() const -> const ConsistencyChecker * { return consChecker_; }
	auto getConsChecker() -> ConsistencyChecker * { return consChecker_; }

	void setConsChecker(ConsistencyChecker *consChecker) { consChecker_ = consChecker; }

	auto getState() const -> const ExecutionState & { return *state_; }
	auto getState() -> ExecutionState & { return *state_; }

	void setState(ExecutionState *s) { state_ = s; }

	/* Thread-related methods */

	/* Creates a new thread in the execution graph */
	void addNewThread()
	{
		events.emplace_back();
		poLists.emplace_back();
	};

	/* Returns the number of threads currently in the graph */
	auto getNumThreads() const -> int
	{
		VERIFY(events.size() <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return static_cast<int>(events.size());
	};

	/* Returns the size of the thread tid */
	auto getThreadSize(int tid) const -> int
	{
		VERIFY(events[tid].size() <= static_cast<size_t>(std::numeric_limits<int>::max()));
		return static_cast<int>(events[tid].size());
	};

	/* Returns true if the thread tid is empty */
	auto isThreadEmpty(int tid) const -> bool { return events[tid].empty(); };

	/** Returns true if TID is blocked */
	auto isThreadBlocked(int tid) const -> bool
	{
		return !isThreadEmpty(tid) && genmc::isa<BlockLabel>(getLastThreadLabel(tid));
	}

	/* Event addition/removal methods */

	auto getInitLabel() const -> const InitLabel *
	{
		return genmc::cast<InitLabel>(getEventLabel(Event(0, 0)));
	}
	auto getInitLabel() -> InitLabel *
	{
		return const_cast<InitLabel *>(std::as_const(*this).getInitLabel());
	}

	/* Adds LAB to the graph. If a label exists in the respective
	 * position, it is replaced.
	 * (Maintains well-formedness for read removals.) */
	template <typename LabelT> auto add(std::unique_ptr<LabelT> lab) -> LabelT *
	{
		auto *raw = lab.get();
		ASSERT(getConsChecker());
		getConsChecker()->maybeIncreaseCacheCounters(raw);
		addLabelToGraph(std::move(lab));
		return raw;
	}

	/* Temporarily adds LAB to the graph (see add()).
	 * Returns the scope of the addition; if commit()
	 * is called, ownership is stolen and the label is permanently added */
	template <typename LabelT>
	auto addScoped(std::unique_ptr<LabelT> &labRef) -> ScopedLabel<LabelT>
	{
		return ScopedLabel(*this, labRef);
	}

	/* Removes the last label from THREAD, and returns it.
	 * If it is a read, updates the rf-lists.
	 * If it is a write, makes all readers read BOT. */
	auto removeLast(int thread) -> std::unique_ptr<EventLabel>;

	/* Event getter methods */

	/* Returns the label in the position denoted by event e */
	auto getEventLabel(Event e) const -> const EventLabel *
	{
		return events[e.thread][e.index].get();
	}
	auto getEventLabel(Event e) -> EventLabel *
	{
		return const_cast<EventLabel *>(std::as_const(*this).getEventLabel(e));
	}

	/* Returns a label as a ReadLabel.
	 * If the passed event is not a read, returns nullptr  */
	auto getReadLabel(Event e) const -> const ReadLabel *
	{
		return genmc::dyn_cast<ReadLabel>(getEventLabel(e));
	}
	auto getReadLabel(Event e) -> ReadLabel *
	{
		return const_cast<ReadLabel *>(std::as_const(*this).getReadLabel(e));
	}

	/* Returns a label as a WriteLabel.
	 * If the passed event is not a write, returns nullptr  */
	auto getWriteLabel(Event e) const -> const WriteLabel *
	{
		return genmc::dyn_cast<WriteLabel>(getEventLabel(e));
	}
	auto getWriteLabel(Event e) -> WriteLabel *
	{
		return const_cast<WriteLabel *>(std::as_const(*this).getWriteLabel(e));
	}

	/* Returns the first label in the thread tid */
	auto getFirstThreadLabel(int tid) const -> const ThreadStartLabel *
	{
		return genmc::dyn_cast<ThreadStartLabel>(getEventLabel(Event(tid, 0)));
	}
	auto getFirstThreadLabel(int tid) -> ThreadStartLabel *
	{
		return const_cast<ThreadStartLabel *>(
			std::as_const(*this).getFirstThreadLabel(tid));
	}

	auto getLastThreadLabel(int thread) const -> const EventLabel *
	{
		return poLists[thread].empty() ? nullptr : &poLists[thread].back();
	}
	auto getLastThreadLabel(int thread) -> EventLabel *
	{
		return const_cast<EventLabel *>(std::as_const(*this).getLastThreadLabel(thread));
	}

	/* Boolean helper functions */

	/** A graph is blocked if any of its threads is blocked */
	auto isBlocked() const -> bool
	{
		return std::ranges::any_of(thr_ids(),
					   [this](auto tid) { return isThreadBlocked(tid); });
	}

	auto getInitVal(const AAccess &access) const -> SVal
	{
#if EMIT_NA_LABELS
		return getState().readStaticInitValue(access);
#else
		if (initVals_.contains(access.addr))
			return initVals_.at(access.addr);

		/* If the access is not in initVals_, it is either fully initialized (if dynamic)
		 * or overwritten by NAs (if static). This is guaranteed by a prior call
		 * to checkinitializedMem or the check below, respectively.
		 * Either way, we return a dummy value. */
		// ASSERT(getState().isNAInitialized(access));
		return SVal(0);
#endif
	}

	void setInitVal(const SAddr &addr, SVal val)
	{
		auto result = initVals_.insert({addr, val});
		VERIFY(!(result.second &&
			 (((*result.first).second.get() != val.get() &&
			   (*result.first).second.getProvenance() !=
				   val.getProvenance())))); /* Attempt to replace initial value */
	}
	void updateDeferredValue(const AAccess &access, SVal val)
	{
		auto &state = getState();
		if (state.getMaxAWriteView(access).empty() &&
		    state.getMaxNAWriteEvent(access).empty()) {
			setInitVal(access.addr, val);
			return;
		}
		state.onDeferredValueUpdate(access, val);
	}
	auto containsLoc(SAddr addr) const -> bool { return coherence.contains(addr); }

	[[nodiscard]] auto resolveAccessValue(const EventLabel *lab, const AAccess &access) const
		-> SVal;

	auto isLocEmpty(SAddr addr) const -> bool
	{
		auto it = coherence.find(addr);
		return it == coherence.end() || it->second.empty();
	}

	/* Whether a location has more than one store */
	auto hasLocMoreThanOneStore(SAddr addr) const -> bool
	{
		auto it = coherence.find(addr);
		if (it == coherence.end() || it->second.empty())
			return false;

		/* Safe to increment because we checked empty() */
		auto listIt = it->second.begin();
		return ++listIt != it->second.end();
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
		return containsPos(e) && !genmc::isa<EmptyLabel>(getEventLabel(e));
	}

	/* Debugging methods */

	void validate();

	/* Graph modification methods */

	/* Use of this assumes NA labels are not in the graph */
	auto getCurrentMemValue(SAddr addr, ASize size) -> SVal;

	/* Prefix saving and restoring */

	/* Returns a vector clock representing the events added up to and including e */
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

	/* For formatting: */
	friend struct std::formatter<ExecutionGraph>;

protected:
	friend class WriteLabel;

	static auto isSameLoc(const EventLabel *labA, const EventLabel *labB) -> bool
	{
		const auto *mLabA = genmc::dyn_cast<MemAccessLabel>(labA);
		const auto *mLabB = genmc::dyn_cast<MemAccessLabel>(labB);
		return mLabA && mLabB && mLabA->getAddr() == mLabB->getAddr();
	}

	/* Functor for Detour Successors (Default Constructible for filter_view) */
	struct DetourSuccFilter {
		const EventLabel *lab = nullptr;
		Event pos;
		DetourSuccFilter() = default;
		DetourSuccFilter(const EventLabel *l, Event pos) : lab(l), pos(pos) {}

		auto operator()(const EventLabel &oLab) const -> bool
		{
			if (!isSameLoc(lab, &oLab))
				return false;
			const auto *rLab = genmc::dyn_cast<ReadLabel>(&oLab);
			return rLab && rLab->getRf() && rLab->getRf()->getPos() != pos;
		}
	};

	/* Functor for Detour Predecessors */
	struct DetourPredFilter {
		const EventLabel *lab = nullptr;
		Event wPos;
		DetourPredFilter() = default;
		DetourPredFilter(const EventLabel *l, Event wPos) : lab(l), wPos(wPos) {}

		auto operator()(const EventLabel &sLab) const -> bool
		{
			if (!isSameLoc(lab, &sLab))
				return false;
			const auto *wLab = genmc::dyn_cast<WriteLabel>(&sLab);
			return wLab && wLab->getPos() != wPos;
		}
	};

	static auto indirect(const std::unique_ptr<EventLabel> &ptr) -> EventLabel &
	{
		return *ptr;
	}

	/* Returns the next available stamp (and increases the counter) */
	auto nextStamp() -> Stamp { return timestamp++; }

	/* Resets the next available stamp to the specified value */
	void resetStamp(Stamp val) { timestamp = val; }

	void trackCoherenceAtLoc(SAddr addr);
	void copyGraphUpTo(ExecutionGraph &other, const VectorClock &v) const;
	void addInitRfToLoc(ReadLabel *rLab) { getInitLabel()->addReader(rLab); }

	void removeInitRfToLoc(ReadLabel *rLab)
	{
		getInitLabel()->removeReader(rLab->getAddr(),
					     [&](auto &lab) { return &lab == rLab; });
	}

	void removeAfter(const VectorClock &preds);

	auto addLabelToGraph(std::unique_ptr<EventLabel> lab) -> EventLabel *;

	static auto createHoleLabel(Event pos) -> std::unique_ptr<EmptyLabel>
	{
		auto lab = EmptyLabel::create(pos);
		lab->setViews({{}});
		lab->setCalculated({{}});
		return lab;
	}

	/* A collection of threads and the events for each threads */
	// NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
	ThreadList events;
	// NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
	IoList insertionOrder;
	// NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
	PoLists poLists;

	// NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
	ConsistencyChecker *consChecker_{};
	// NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
	bool haveNAs_{};

private:
	/* The next available timestamp */
	Stamp timestamp = 0;

	LocMap coherence;

	/* XXX: Temporary map; eventually remove */
	AccessMap accessMap_;

	/* XXX: Temporary map; eventually remove */
	std::unordered_map<SAddr, SVal> initVals_;

	ExecutionState *state_{};
};

/** Make `ExecutionGraph` formattable with `std::format`. */
template <> struct std::formatter<ExecutionGraph> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const ExecutionGraph &g, std::format_context &ctx) const
	{
		auto out = ctx.out();

		// Format threads
		for (auto i = 0; i < g.getNumThreads(); i++) {
			out = std::format_to(out, "Thread {}:\n", i);
			for (const auto &lab : g.po(i)) {
				out = std::format_to(out, "\t{} @ {}\n", lab.getStamp(), lab);
			}
		}

		// Format thread sizes
		out = std::format_to(out, "Thread sizes:\n\t");
		for (auto i = 0; i < g.getNumThreads(); i++) {
			out = std::format_to(out, "{} ", g.getThreadSize(i));
		}
		out = std::format_to(out, "\n");

		// Format coherence order for each location
		for (auto lIt = g.loc_begin(), lE = g.loc_end(); lIt != lE; ++lIt) {
			out = std::format_to(out, "{}: ", lIt->first);
			for (const auto &lab : g.co(lIt->first)) {
				out = std::format_to(out, "{} ", lab.getPos());
			}
			out = std::format_to(out, "\n");
		}

		return out;
	}
};

namespace std {
template <> struct hash<ExecutionGraph> {
	auto operator()(const ExecutionGraph &g) const -> size_t
	{
		std::size_t hash = 0;

		/* Use a fixed (non-insertion-order-dependent) iteration order */
		hash_combine(hash, g.getNumThreads());
		for (auto i = 0; i < g.getNumThreads(); i++) {
			hash_combine(hash, g.getThreadSize(i));
			for (const auto &lab : g.po(i)) {
				if (const auto *rLab = genmc::dyn_cast<ReadLabel>(&lab)) {
					hash_combine(hash, rLab->getRf() ? rLab->getRf()->getPos()
									 : Event{});
				}
				if (const auto *wLab = genmc::dyn_cast<WriteLabel>(&lab)) {
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
