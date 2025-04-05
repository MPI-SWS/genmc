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

#ifndef GENMC_GRAPH_ITERATORS_HPP
#define GENMC_GRAPH_ITERATORS_HPP

#include "ExecutionGraph.hpp"
#include "config.h"
#include <iterator>
#include <llvm/ADT/iterator_range.h>
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

using label_range = llvm::iterator_range<ExecutionGraph::label_iterator>;
using const_label_range = llvm::iterator_range<ExecutionGraph::const_label_iterator>;

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
using const_co_range = llvm::iterator_range<const_co_iterator>;
using const_reverse_co_range = llvm::iterator_range<const_reverse_co_iterator>;

namespace detail {
inline const_co_iterator coSentinel;
inline const_reverse_co_iterator coRevSentinel;
}; // namespace detail

inline auto co_succ_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_succ_begin(wLab) : ::detail::coSentinel;
}
inline auto co_succ_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_succ_end(wLab) : ::detail::coSentinel;
}
inline auto co_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_co_range(co_succ_begin(G, lab), co_succ_end(G, lab));
}
inline const WriteLabel *co_imm_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return !wLab ? nullptr : G.co_imm_succ(wLab);
}

inline const_reverse_co_iterator co_pred_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_pred_begin(wLab) : ::detail::coRevSentinel;
}
inline const_reverse_co_iterator co_pred_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? G.co_pred_end(wLab) : ::detail::coRevSentinel;
}
inline const_reverse_co_range co_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_reverse_co_range(co_pred_begin(G, lab), co_pred_end(G, lab));
}
inline const WriteLabel *co_imm_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
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

using const_ppo_iterator = DepInfo::const_iterator;
using const_ppo_range = llvm::iterator_range<const_ppo_iterator>;

#define PPO_ITERATOR(name)                                                                         \
	inline const_ppo_iterator name##_pred_begin(const ExecutionGraph &G, Event e)              \
	{                                                                                          \
		return G.getEventLabel(e)->name##_begin();                                         \
	}                                                                                          \
                                                                                                   \
	inline const_ppo_iterator name##_pred_end(const ExecutionGraph &G, Event e)                \
	{                                                                                          \
		return G.getEventLabel(e)->name##_end();                                           \
	}                                                                                          \
                                                                                                   \
	inline const_ppo_range name##_preds(const ExecutionGraph &G, Event e)                      \
	{                                                                                          \
		return G.getEventLabel(e)->name();                                                 \
	}                                                                                          \
	inline const_ppo_range name##_preds(const ExecutionGraph &G, const EventLabel *lab)        \
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
		auto *lab = llvm::dyn_cast<MemAccessLabel>(&sLab);
		return lab && lab->getAddr() == addr;
	}

private:
	const ExecutionGraph &graph;
	const SAddr addr;
};

template <typename IterT>
struct poloc_filter_iterator : public llvm::filter_iterator<IterT, LocationFilter> {
public:
	using BaseT = llvm::filter_iterator<IterT, LocationFilter>;

	poloc_filter_iterator(IterT it, IterT end, LocationFilter filter) : BaseT(it, end, filter)
	{}

	poloc_filter_iterator &operator++()
	{
		return static_cast<poloc_filter_iterator &>(BaseT::operator++());
	}
	poloc_filter_iterator operator++(int)
	{
		auto tmp = *this;
		BaseT::operator++();
		return tmp;
	}
};

static inline bool hasLocation(const EventLabel *lab) { return lab->hasLocation(); }

static inline SAddr getLocation(const EventLabel *lab)
{
	if (auto *mLab = llvm::dyn_cast<MemAccessLabel>(lab))
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
		auto *lab = llvm::dyn_cast<ReadLabel>(&rLab);
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
		auto *lab = llvm::dyn_cast<WriteLabel>(&sLab);
		return lab && lab->getPos() != write;
	}

private:
	const ExecutionGraph &graph;
	const Event write;
};
} /* namespace detail */

inline auto detour_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? poloc_succs(G, lab) |
			       std::views::filter(::detail::RfIntFilter(G, lab->getPos()))
		    : poloc_succs(G, G.getLastThreadLabel(lab->getThread())) |
			       std::views::filter(::detail::RfIntFilter(G, lab->getPos()));
}

inline auto detour_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
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
using const_rf_range = llvm::iterator_range<const_rf_iterator>;

namespace detail {
inline const_rf_iterator sentinel;
};

inline const_rf_iterator rf_succ_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? wLab->readers_begin() : ::detail::sentinel;
}

inline const_rf_iterator rf_succ_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? wLab->readers_end() : ::detail::sentinel;
}

inline const_rf_range rf_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_rf_range(rf_succ_begin(G, lab), rf_succ_end(G, lab));
}

inline const EventLabel *rf_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
	return (!rLab || !rLab->getRf()) ? nullptr : rLab->getRf();
}

/*******************************************************************************
 **                         rfe-iteration utilities
 ******************************************************************************/

namespace detail {
struct DiffThreadFilter {
	DiffThreadFilter() = delete;
	DiffThreadFilter(const ExecutionGraph &g, int t) : graph(g), thread(t) {}

	bool operator()(const ReadLabel &rLab) const { return rLab.getThread() != thread; }

private:
	const ExecutionGraph &graph;
	const int thread;
};

template <typename IterT>
struct rfe_filter_iterator : public llvm::filter_iterator<IterT, DiffThreadFilter> {
public:
	using BaseT = llvm::filter_iterator<IterT, DiffThreadFilter>;

	rfe_filter_iterator(IterT it, IterT end, DiffThreadFilter filter) : BaseT(it, end, filter)
	{}

	rfe_filter_iterator &operator++()
	{
		return static_cast<rfe_filter_iterator &>(BaseT::operator++());
	}
	rfe_filter_iterator operator++(int)
	{
		auto tmp = *this;
		BaseT::operator++();
		return tmp;
	}
};
} /* namespace detail */

using const_rfe_iterator = ::detail::rfe_filter_iterator<const_rf_iterator>;
using const_rfe_range = llvm::iterator_range<const_rfe_iterator>;

inline const_rfe_iterator rfe_succ_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_rfe_iterator(rf_succ_begin(G, lab), rf_succ_end(G, lab),
				  ::detail::DiffThreadFilter(G, lab->getThread()));
}
inline const_rfe_iterator rfe_succ_end(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_rfe_iterator(rf_succ_end(G, lab), rf_succ_end(G, lab),
				  ::detail::DiffThreadFilter(G, lab->getThread()));
}
inline const_rfe_range rfe_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_rfe_range(rfe_succ_begin(G, lab), rfe_succ_end(G, lab));
}

inline const EventLabel *rfe_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
	return (rLab && rLab->readsExt()) ? rLab->getRf() : nullptr;
}

/*******************************************************************************
 **                         rfi-iteration utilities
 ******************************************************************************/

namespace detail {
struct SameThreadFilter {
	SameThreadFilter() = delete;
	SameThreadFilter(const ExecutionGraph &g, int t) : graph(g), thread(t) {}

	bool operator()(const ReadLabel &rLab) const { return rLab.getThread() == thread; }

private:
	const ExecutionGraph &graph;
	const int thread;
};

template <typename IterT>
struct rfi_filter_iterator : public llvm::filter_iterator<IterT, SameThreadFilter> {
public:
	using BaseT = llvm::filter_iterator<IterT, SameThreadFilter>;

	rfi_filter_iterator(IterT it, IterT end, SameThreadFilter filter) : BaseT(it, end, filter)
	{}

	rfi_filter_iterator &operator++()
	{
		return static_cast<rfi_filter_iterator &>(BaseT::operator++());
	}
	rfi_filter_iterator operator++(int)
	{
		auto tmp = *this;
		BaseT::operator++();
		return tmp;
	}
};
} /* namespace detail */

using const_rfi_iterator = ::detail::rfi_filter_iterator<const_rf_iterator>;
using const_rfi_range = llvm::iterator_range<const_rfi_iterator>;

inline const_rfi_iterator rfi_succ_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_rfi_iterator(rf_succ_begin(G, lab), rf_succ_end(G, lab),
				  ::detail::SameThreadFilter(G, lab->getThread()));
}
inline const_rfi_iterator rfi_succ_end(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_rfi_iterator(rf_succ_end(G, lab), rf_succ_end(G, lab),
				  ::detail::SameThreadFilter(G, lab->getThread()));
}
inline const_rfi_range rfi_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_rfi_range(rfi_succ_begin(G, lab), rfi_succ_end(G, lab));
}

inline const EventLabel *rfi_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
	return (rLab && rLab->readsInt()) ? rLab->getRf() : nullptr;
}

/*******************************************************************************
 **                         tcreate-iteration utilities
 ******************************************************************************/

inline const ThreadStartLabel *tc_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *tcLab = llvm::dyn_cast<ThreadCreateLabel>(lab);
	return tcLab ? G.getFirstThreadLabel(tcLab->getChildId()) : nullptr;
}
inline const ThreadCreateLabel *tc_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *tsLab = llvm::dyn_cast<ThreadStartLabel>(lab);
	return tsLab ? tsLab->getCreate() : nullptr;
}

/*******************************************************************************
 **                         tjoin-iteration utilities
 ******************************************************************************/

inline const ThreadJoinLabel *tj_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(lab);
	return !eLab ? nullptr : eLab->getParentJoin();
}

inline const ThreadFinishLabel *tj_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *tjLab = llvm::dyn_cast<ThreadJoinLabel>(lab);
	return tjLab ? llvm::dyn_cast_or_null<ThreadFinishLabel>(
			       G.getLastThreadLabel(tjLab->getChildId()))
		     : nullptr;
}

/*******************************************************************************
 **                         fr-iteration utilities
 ******************************************************************************/

using const_fr_iterator = const_co_iterator;
using const_reverse_fr_iterator = const_reverse_co_iterator;

using const_fr_range = llvm::iterator_range<const_fr_iterator>;
using const_reverse_fr_range = llvm::iterator_range<const_reverse_fr_iterator>;

using const_fr_inv_iterator = WriteLabel::const_rf_iterator;
using const_fr_inv_range = llvm::iterator_range<const_fr_inv_iterator>;

inline const_fr_iterator fr_succ_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
	return rLab ? G.fr_succ_begin(rLab) : ::detail::coSentinel;
}
inline const_fr_iterator fr_succ_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
	return rLab ? G.fr_succ_end(rLab) : ::detail::coSentinel;
}
inline const_fr_range fr_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_fr_range(fr_succ_begin(G, lab), fr_succ_end(G, lab));
}
inline const WriteLabel *fr_imm_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *rLab = llvm::dyn_cast<ReadLabel>(lab);
	return !rLab ? nullptr : G.fr_imm_succ(rLab);
}

inline const_fr_inv_iterator fr_imm_pred_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? G.fr_imm_pred_begin(wLab) : ::detail::sentinel;
}
inline const_fr_inv_iterator fr_imm_pred_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *wLab = llvm::dyn_cast<WriteLabel>(lab);
	return wLab ? G.fr_imm_pred_end(wLab) : ::detail::sentinel;
}
inline const_fr_inv_range fr_imm_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_fr_inv_range(fr_imm_pred_begin(G, lab), fr_imm_pred_end(G, lab));
}

/*******************************************************************************
 **                         sameloc-iteration utilities
 ******************************************************************************/

namespace detail {
struct IDAndLocFilter {
	IDAndLocFilter() = delete;
	IDAndLocFilter(const ExecutionGraph &g, const SAddr &a, Event e) : graph(g), addr(a), pos(e)
	{}

	bool operator()(const EventLabel &sLab) const
	{
		if (auto *lab = llvm::dyn_cast<MemAccessLabel>(&sLab))
			return lab->getPos() != pos && lab->getAddr() == addr;
		if (auto *lab = llvm::dyn_cast<MallocLabel>(&sLab))
			return lab->getPos() != pos && lab->contains(addr);
		if (auto *lab = llvm::dyn_cast<FreeLabel>(&sLab))
			return lab->getPos() != pos && lab->contains(addr);
		return false;
	}

private:
	const ExecutionGraph &graph;
	const SAddr addr;
	const Event pos;
};

template <typename IterT>
struct sameloc_filter_iterator : public llvm::filter_iterator<IterT, IDAndLocFilter> {
public:
	using BaseT = llvm::filter_iterator<IterT, IDAndLocFilter>;

	sameloc_filter_iterator(IterT it, IterT end, IDAndLocFilter filter) : BaseT(it, end, filter)
	{}

	sameloc_filter_iterator &operator++()
	{
		return static_cast<sameloc_filter_iterator &>(BaseT::operator++());
	}
	sameloc_filter_iterator operator++(int)
	{
		auto tmp = *this;
		BaseT::operator++();
		return tmp;
	}
};
} /* namespace detail */

using const_sameloc_iterator = ::detail::sameloc_filter_iterator<const_label_iterator>;
using const_sameloc_range = llvm::iterator_range<const_sameloc_iterator>;

inline const_sameloc_iterator sameloc_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	using namespace ::detail;
	return hasLocation(lab)
		       ? const_sameloc_iterator(G.label_begin(), G.label_end(),
						IDAndLocFilter(G, getLocation(lab), lab->getPos()))
		       : const_sameloc_iterator(G.label_end(), G.label_end(),
						IDAndLocFilter(G, SAddr(), lab->getPos()));
}

inline const_sameloc_iterator sameloc_end(const ExecutionGraph &G, const EventLabel *lab)
{
	using namespace ::detail;
	auto addr = hasLocation(lab) ? getLocation(lab) : SAddr();
	return const_sameloc_iterator(G.label_end(), G.label_end(),
				      IDAndLocFilter(G, addr, lab->getPos()));
}

inline const_sameloc_range samelocs(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_sameloc_range(sameloc_begin(G, lab), sameloc_end(G, lab));
}

/*******************************************************************************
 **                         alloc-iteration utilities
 ******************************************************************************/

using const_alloc_iterator = MallocLabel::const_access_iterator;
using const_alloc_range = llvm::iterator_range<const_alloc_iterator>;

namespace detail {
inline const_alloc_iterator allocSentinel;
};

inline const_alloc_iterator alloc_succ_begin(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *aLab = llvm::dyn_cast<MallocLabel>(lab);
	return aLab ? aLab->accesses_begin() : ::detail::allocSentinel;
}

inline const_alloc_iterator alloc_succ_end(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *aLab = llvm::dyn_cast<MallocLabel>(lab);
	return aLab ? aLab->accesses_end() : ::detail::allocSentinel;
}

inline const_alloc_range alloc_succs(const ExecutionGraph &G, const EventLabel *lab)
{
	return const_alloc_range(alloc_succ_begin(G, lab), alloc_succ_end(G, lab));
}

inline const MallocLabel *alloc_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *aLab = llvm::dyn_cast<MemAccessLabel>(lab);
	return (!aLab || !aLab->getAlloc()) ? nullptr : aLab->getAlloc();
}

/*******************************************************************************
 **                         free-iteration utilities
 ******************************************************************************/

inline const FreeLabel *free_succ(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *aLab = llvm::dyn_cast<MallocLabel>(lab);
	return (!aLab || !aLab->getFree()) ? nullptr : aLab->getFree();
}

inline const MallocLabel *free_pred(const ExecutionGraph &G, const EventLabel *lab)
{
	auto *dLab = llvm::dyn_cast<FreeLabel>(lab);
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
	const auto *endLab = llvm::dyn_cast<MethodEndLabel>(lab);
	return (endLab ? endLab->lin_succs() : std::views::all(::detail::sentinelSuccs)) |
	       std::views::transform(::detail::indirectBegin);
}

inline auto lin_preds(const ExecutionGraph &G, const EventLabel *lab)
{
	const auto *begLab = llvm::dyn_cast<MethodBeginLabel>(lab);
	return (begLab ? begLab->lin_preds() : std::views::all(::detail::sentinelPreds)) |
	       std::views::transform(::detail::indirectEnd);
}

#endif /* GENMC_GRAPH_ITERATORS_HPP */
