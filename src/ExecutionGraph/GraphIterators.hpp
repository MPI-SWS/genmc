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

#ifndef GENMC_GRAPH_ITERATORS_HPP
#define GENMC_GRAPH_ITERATORS_HPP

#include "ExecutionGraph.hpp"
#include <iterator>
#include <type_traits>
#include <utility>

/*
 * Helper iterators for ExecutionGraphs
 */

using label_iterator = ExecutionGraph::label_iterator;
using const_label_iterator = ExecutionGraph::const_label_iterator;

/*******************************************************************************
 **                         label-iteration utilities
 ******************************************************************************/

inline auto other_labels(ExecutionGraph &G, const EventLabel *lab)
{
	return G.labels() |
	       std::views::filter([lab](auto &olab) { return olab.getPos() != lab->getPos(); });
}
inline auto other_labels(const ExecutionGraph &G, const EventLabel *lab)
{
	return G.labels() |
	       std::views::filter([lab](auto &olab) { return olab.getPos() != lab->getPos(); });
}

/*******************************************************************************
 **                         co-iteration utilities
 ******************************************************************************/

using const_co_iterator = ExecutionGraph::const_co_iterator;
using const_reverse_co_iterator = ExecutionGraph::const_reverse_co_iterator;

namespace detail {
inline const_co_iterator coSentinel;
inline const_reverse_co_iterator coRevSentinel;
}; // namespace detail

inline auto co_succ_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_succ_begin(wLab) : ::detail::coSentinel;
}
inline auto co_succ_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_succ_end(wLab) : ::detail::coSentinel;
}
inline auto co_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return std::ranges::subrange(co_succ_begin(G, lab), co_succ_end(G, lab));
}
inline const WriteLabel *co_imm_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return !wLab ? nullptr : G.co_imm_succ(wLab);
}

inline const_reverse_co_iterator co_pred_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_pred_begin(wLab) : ::detail::coRevSentinel;
}
inline const_reverse_co_iterator co_pred_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_pred_end(wLab) : ::detail::coRevSentinel;
}
inline auto co_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	return std::ranges::subrange(co_pred_begin(G, lab), co_pred_end(G, lab));
}
inline const WriteLabel *co_imm_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return !wLab ? nullptr : G.co_imm_pred(wLab);
}

/*******************************************************************************
 **                         po-iteration utilities
 ******************************************************************************/

using const_po_iterator = ExecutionGraph::const_po_iterator;
using const_reverse_po_iterator = ExecutionGraph::const_reverse_po_iterator;

inline auto po_succs(const ExecutionGraph &G, const EventLabel *lab) { return G.po_succs(lab); }

inline const EventLabel *po_imm_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	return G.po_imm_succ(lab);
}

inline auto po_preds(const ExecutionGraph &G, const EventLabel *lab) { return G.po_preds(lab); }

inline const EventLabel *po_imm_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	return G.po_imm_pred(lab);
}

/*******************************************************************************
 **                         ppo-iteration utilities
 ******************************************************************************/

#define PPO_ITERATOR(name)                                                                         \
	inline auto name##_preds(const ExecutionGraph &G, Event e)                                 \
	{                                                                                          \
		return G.getEventLabel(e)->name();                                                 \
	}                                                                                          \
	inline auto name##_preds(const ExecutionGraph &G, const EventLabel *lab)                   \
	{                                                                                          \
		return lab->name();                                                                \
	}

PPO_ITERATOR(data);
PPO_ITERATOR(addr);
PPO_ITERATOR(ctrl);

/*******************************************************************************
 **                         poloc-iteration utilities
 ******************************************************************************/

namespace detail {
struct LocationFilter {
	LocationFilter() = delete;
	LocationFilter(const ExecutionGraph &g, const SAddr &a) : graph(g), addr(a) {}

	bool operator()(const EventLabel &sLab) const
	{
		auto *lab = genmc::dyn_cast<MemAccessLabel>(&sLab);
		return lab && lab->getAddr() == addr;
	}

private:
	const ExecutionGraph &graph;
	const SAddr addr;
};

static inline bool hasLocation(const EventLabel *lab) { return lab->hasLocation(); }

static inline SAddr getLocation(const EventLabel *lab)
{
	if (auto *mLab = genmc::dyn_cast<MemAccessLabel>(lab))
		return mLab->getAddr();
	return SAddr();
}
} /* namespace detail */

inline auto poloc_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	/* Capture LAB explicitly as by reference it leads to weird segfaults */
	auto locFilter = [lab](auto &oLab) {
		return ::detail::hasLocation(&oLab) &&
		       ::detail::getLocation(&oLab) == ::detail::getLocation(lab);
	};
	if (::detail::hasLocation(lab))
		return po_succs(G, lab) | std::views::filter(locFilter);
	return po_succs(G, G.getLastThreadLabel(lab->getThread())) | std::views::filter(locFilter);
}

inline const EventLabel *poloc_imm_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto succs = poloc_succs(G, lab);
	return succs.begin() == succs.end() ? nullptr : &*succs.begin();
}

inline auto poloc_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	auto locFilter = [lab](auto &oLab) {
		return ::detail::hasLocation(&oLab) &&
		       ::detail::getLocation(&oLab) == ::detail::getLocation(lab);
	};
	return ::detail::hasLocation(lab) ? po_preds(G, lab) | std::views::filter(locFilter)
					  : po_preds(G, G.getFirstThreadLabel(lab->getThread())) |
						    std::views::filter(locFilter);
}

inline const EventLabel *poloc_imm_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto preds = poloc_preds(G, lab);
	return preds.begin() == preds.end() ? nullptr : &*preds.begin();
}

/*******************************************************************************
 **                         detour-iteration utilities
 ******************************************************************************/

namespace detail {
/* Filters out an event only --- assumes poloc iteration */
struct RfIntFilter {
	RfIntFilter() = delete;
	RfIntFilter(const ExecutionGraph &g, const Event &w) : graph(g), write(w) {}

	bool operator()(const EventLabel &rLab) const
	{
		auto *lab = genmc::dyn_cast<ReadLabel>(&rLab);
		return lab && lab->getRf()->getPos() != write;
	}

private:
	const ExecutionGraph &graph;
	const Event write;
};

struct RfInvIntFilter {
	RfInvIntFilter() = delete;
	RfInvIntFilter(const ExecutionGraph &g, const Event &w) : graph(g), write(w) {}

	bool operator()(const EventLabel &sLab) const
	{
		auto *lab = genmc::dyn_cast<WriteLabel>(&sLab);
		return lab && lab->getPos() != write;
	}

private:
	const ExecutionGraph &graph;
	const Event write;
};
} /* namespace detail */

inline auto detour_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return wLab ? poloc_succs(G, lab) |
			       std::views::filter(::detail::RfIntFilter(G, lab->getPos()))
		    : poloc_succs(G, G.getLastThreadLabel(lab->getThread())) |
			       std::views::filter(::detail::RfIntFilter(G, lab->getPos()));
}

inline auto detour_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
	return rLab && rLab->getRf()
		       ? poloc_preds(G, lab) | std::views::filter(::detail::RfInvIntFilter(
						       G, rLab->getRf()->getPos()))
		       : poloc_preds(G, G.getFirstThreadLabel(lab->getThread())) |
				 std::views::filter(::detail::RfInvIntFilter(G, Event::getInit()));
}

/*******************************************************************************
 **                         rf-iteration utilities
 ******************************************************************************/

using const_rf_iterator = WriteLabel::const_rf_iterator;

namespace detail {
inline const_rf_iterator sentinel;
};

inline auto rf_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return wLab ? wLab->readers()
		    : std::ranges::subrange(::detail::sentinel, ::detail::sentinel);
}

inline const EventLabel *rf_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
	return (!rLab || !rLab->getRf()) ? nullptr : rLab->getRf();
}

/*******************************************************************************
 **                         rfe-iteration utilities
 ******************************************************************************/

inline auto rfe_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return rf_succs(G, lab) | std::views::filter([lab](auto &oLab) {
		       return oLab.getThread() != lab->getThread();
	       });
}

inline const EventLabel *rfe_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
	return (rLab && rLab->readsExt()) ? rLab->getRf() : nullptr;
}

/*******************************************************************************
 **                         rfi-iteration utilities
 ******************************************************************************/

inline auto rfi_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return rf_succs(G, lab) | std::views::filter([lab](auto &oLab) {
		       return oLab.getThread() == lab->getThread();
	       });
}

inline const EventLabel *rfi_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
	return (rLab && rLab->readsInt()) ? rLab->getRf() : nullptr;
}

/*******************************************************************************
 **                         tcreate-iteration utilities
 ******************************************************************************/

inline const ThreadStartLabel *tc_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *tcLab = genmc::dyn_cast<ThreadCreateLabel>(lab);
	return tcLab ? G.getFirstThreadLabel(tcLab->getChildId()) : nullptr;
}
inline const ThreadCreateLabel *tc_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(lab);
	return tsLab ? tsLab->getCreate() : nullptr;
}

/*******************************************************************************
 **                         tjoin-iteration utilities
 ******************************************************************************/

inline const ThreadJoinLabel *tj_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *eLab = genmc::dyn_cast<ThreadFinishLabel>(lab);
	return !eLab ? nullptr : eLab->getParentJoin();
}

inline const ThreadFinishLabel *tj_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *tjLab = genmc::dyn_cast<ThreadJoinLabel>(lab);
	return tjLab ? genmc::dyn_cast_if_present<ThreadFinishLabel>(
			       G.getLastThreadLabel(tjLab->getChildId()))
		     : nullptr;
}

/*******************************************************************************
 **                         fr-iteration utilities
 ******************************************************************************/

inline auto fr_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	const auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
	return rLab ? G.fr_succs(rLab)
		    : std::ranges::subrange(::detail::coSentinel, ::detail::coSentinel);
}
inline const WriteLabel *fr_imm_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	const auto *rLab = genmc::dyn_cast<ReadLabel>(lab);
	return !rLab ? nullptr : G.fr_imm_succ(rLab);
}

inline auto fr_imm_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	const auto *wLab = genmc::dyn_cast<WriteLabel>(lab);
	return wLab ? G.fr_imm_preds(wLab)
		    : std::ranges::subrange(::detail::sentinel, ::detail::sentinel);
}

/*******************************************************************************
 **                         sameloc-iteration utilities
 ******************************************************************************/

inline auto samelocs(const ExecutionGraph &G, const EventLabel *lab) { return G.samelocs(lab); }

/*******************************************************************************
 **                         alloc-iteration utilities
 ******************************************************************************/

using const_alloc_iterator = MallocLabel::const_access_iterator;

namespace detail {
inline const_alloc_iterator allocSentinel;
};

inline auto alloc_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	const auto *aLab = genmc::dyn_cast<MallocLabel>(lab);
	return aLab ? aLab->accesses()
		    : std::ranges::subrange(::detail::allocSentinel, ::detail::allocSentinel);
}

inline const MallocLabel *alloc_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *aLab = genmc::dyn_cast<MemAccessLabel>(lab);
	return (!aLab || !aLab->getAlloc()) ? nullptr : aLab->getAlloc();
}

/*******************************************************************************
 **                         free-iteration utilities
 ******************************************************************************/

inline const FreeLabel *free_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *aLab = genmc::dyn_cast<MallocLabel>(lab);
	return (!aLab || !aLab->getFree()) ? nullptr : aLab->getFree();
}

inline const MallocLabel *free_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *dLab = genmc::dyn_cast<FreeLabel>(lab);
	return (!dLab || !dLab->getAlloc()) ? nullptr : dLab->getAlloc();
}

/*******************************************************************************
 **                         lin-iteration utilities
 ******************************************************************************/

namespace detail {
inline const std::vector<MethodBeginLabel *> sentinelSuccs;
inline const std::vector<MethodEndLabel *> sentinelPreds;

inline auto indirectBegin(MethodBeginLabel *lab) -> MethodBeginLabel & { return *lab; }
inline auto indirectEnd(MethodEndLabel *lab) -> MethodEndLabel & { return *lab; }

}; // namespace detail

inline auto lin_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	const auto *endLab = genmc::dyn_cast<MethodEndLabel>(lab);
	return (endLab ? endLab->lin_succs() : std::views::all(::detail::sentinelSuccs)) |
	       std::views::transform(::detail::indirectBegin);
}

inline auto lin_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	const auto *begLab = genmc::dyn_cast<MethodBeginLabel>(lab);
	return (begLab ? begLab->lin_preds() : std::views::all(::detail::sentinelPreds)) |
	       std::views::transform(::detail::indirectEnd);
}

#endif /* GENMC_GRAPH_ITERATORS_HPP */
