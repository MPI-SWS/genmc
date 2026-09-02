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

#include "genmc/ADT/AdaptiveView.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Support/Error.hpp"

#include <algorithm>
#include <utility>
#include <variant>

/* Private helpers */
namespace {

using StorageT = AdaptiveView::StorageT;
using Events = AdaptiveView::Events;

template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

} // namespace

static void promoteToView(StorageT &storage)
{
	View view;

	std::visit(overloads{[&](Event &e) { view.updateIdx(e); },
			     [&](Events &es) { view.updateIdxs(es); },
			     [&](auto &) { UNREACHABLE(); }},
		   storage);
	storage = std::move(view);
}

/* addEvent() needs to handle storage being both a View and Events, since combine/operator+=()
 * call this method in a loop. In such a case, storage_ is promoted into a view */
static void addEvent(StorageT &storage, const Event &e)
{
	/* Is storage already a view? */
	if (auto *v = std::get_if<View>(&storage)) {
		v->updateIdx(e);
		return;
	}

	/* Otherwise, it has to be events */
	ASSERT(std::holds_alternative<Events>(storage));
	auto &es = std::get<Events>(storage);

	/* Can we update existing thread? */
	auto *it = std::ranges::find_if(es, [&](const auto &oe) { return oe.thread == e.thread; });
	if (it != es.end()) {
		*it = std::max(*it, e);
		return;
	}

	/* Nope, append or promote */
	if (es.size() < AdaptiveView::SmallVectorCapacity) {
		es.push_back(e);
		return;
	}

	promoteToView(storage);
	std::get<View>(storage).updateIdx(e);
}

static void unify(StorageT &storage, const Event &rhs)
{
	ASSERT(std::holds_alternative<Event>(storage));
	auto &lhs = std::get<Event>(storage);

	/* If the threads differ, switch to Events */
	if (lhs.thread != rhs.thread) {
		storage = Events{lhs, rhs};
		return;
	}

	/* Otherwise, keep max index */
	storage = std::max(lhs, rhs);
}

auto AdaptiveView::containedIn(const View &v) const -> bool
{
	return std::visit(overloads{[](const Empty &) { return true; },
				    [&](const Event &e) { return v.contains(e); },
				    [&](const Events &es) { return v.contains(es); },
				    [&](const View &view) { return v.contains(view); },
				    [&](const Update &update) { return v.contains(update.first); }},
			  storage_);
}

auto AdaptiveView::join(const AdaptiveView &rhs) -> AdaptiveView &
{
	if (this == &rhs)
		return *this;

	/* If current state is Update, convert to Event */
	if (auto *upd = std::get_if<Update>(&storage_)) {
		const auto ev = upd->first; /* needed for aliasing protection */
		storage_ = ev;
	}

	/* Fastpath: If RHS is empty, exit */
	if (std::holds_alternative<Empty>(rhs.storage_))
		return *this;

	/* Fastpath: If LHS is empty, return RHS */
	if (std::holds_alternative<Empty>(storage_)) {
		if (std::holds_alternative<Update>(rhs.storage_))
			storage_ = std::get<Update>(rhs.storage_).first;
		else
			storage_ = rhs.storage_;
		return *this;
	}

	std::visit(overloads{/* LHS is Event */
			     [&](Event &, const Event &rhs) { unify(storage_, rhs); },
			     [&](Event &, const Update &rhs) { unify(storage_, rhs.first); },
			     [&](Event &lhs, const Events &rhs) {
				     auto oldLhs = lhs;
				     storage_ = rhs;		 /* switch to Events */
				     addEvent(storage_, oldLhs); /* re-merge original */
			     },
			     [&](Event &lhs, const View &rhs) {
				     auto oldLhs = lhs;
				     storage_ = rhs; /* switch to View */
				     std::get<View>(storage_).updateIdx(oldLhs);
			     },
			     /* LHS is Events */
			     [&](Events &, const Event &rhs) { addEvent(storage_, rhs); },
			     [&](Events &, const Update &rhs) { addEvent(storage_, rhs.first); },
			     [&](Events &, const Events &rhs) {
				     for (const auto &e : rhs)
					     addEvent(storage_, e);
			     },
			     [&](Events &lhs, const View &rhs) {
				     /* move out before storage_ = rhs destroys lhs */
				     auto oldLhs = std::move(lhs);
				     storage_ = rhs;
				     std::get<View>(storage_).updateIdxs(oldLhs);
			     },
			     /* LHS is View */
			     [&](View &lhs, const Event &rhs) { lhs.updateIdx(rhs); },
			     [&](View &lhs, const Update &rhs) { lhs.updateIdx(rhs.first); },
			     [&](View &lhs, const Events &rhs) { lhs.updateIdxs(rhs); },
			     [&](View &lhs, const View &rhs) { lhs.update(rhs); },
			     /* Unreachable */
			     [](auto &, const auto &) { UNREACHABLE(); }},
		   storage_, rhs.storage_);
	return *this;
}

auto AdaptiveView::operator+=(const AdaptiveView &rhs) -> AdaptiveView &
{
	if (std::holds_alternative<Empty>(rhs.storage_))
		return *this;

	ASSERT(std::holds_alternative<Update>(rhs.storage_));
	const auto &[uPos, uView] = std::get<Update>(rhs.storage_);

	/* Case A: LHS is Empty */
	if (std::holds_alternative<Empty>(storage_)) {
		storage_ = uPos;
		return *this;
	}

	/* Case B: LHS is Event or Update */
	if (std::holds_alternative<Event>(storage_) || std::holds_alternative<Update>(storage_)) {
		auto e = std::holds_alternative<Event>(storage_) ? std::get<Event>(storage_)
								 : std::get<Update>(storage_).first;
		storage_ = uView.contains(e) ? StorageT{uPos} : StorageT{Events{e, uPos}};
		return *this;
	}

	/* Case C: LHS is Events */
	if (auto *es = std::get_if<Events>(&storage_)) {
		es->erase(std::ranges::remove_if(*es,
						 [&](const auto &e) { return uView.contains(e); })
				  .begin(),
			  es->end());

		if (es->empty()) {
			storage_ = uPos;
			return *this;
		}
		addEvent(storage_, uPos);
		return *this;
	}

	/* Case D: LHS is View */
	auto *v = std::get_if<View>(&storage_);
	ASSERT(v);
	if (uView.contains(*v))
		storage_ = uPos;
	else
		v->updateIdx(uPos);
	return *this;
}
