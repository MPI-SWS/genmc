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

#ifndef GENMC_DEP_VIEW_HPP
#define GENMC_DEP_VIEW_HPP

#include "ADT/VSet.hpp"
#include "ADT/VectorClock.hpp"
#include "ADT/View.hpp"
#include "Support/Error.hpp"
#include <llvm/ADT/IndexedMap.h>
#include <llvm/Support/raw_ostream.h>

/**
 * An instantiation of a vector clock where if an event is contained in the clock,
 * it is _not_ guaranteed that all of its po-predecessors are also contained in
 * the clock.
 */
class DepView : public VectorClock {
private:
	using Holes = VSet<int>;

	class HoleView {
		llvm::IndexedMap<Holes> hs_;

	public:
		HoleView() : hs_(Holes()) {}

		[[nodiscard]] auto size() const -> unsigned int { return hs_.size(); }

		void clear() { hs_.clear(); }

		auto operator[](int idx) const -> const Holes &
		{
			if (idx < hs_.size())
				return hs_[idx];
			BUG();
		}

		auto operator[](int idx) -> Holes &
		{
			hs_.grow(idx);
			return hs_[idx];
		}
	};

public:
	/** Constructors */
	DepView() : VectorClock(VectorClock::VectorClockKind::VC_DepView) {}

	/** Returns the size of the depview (i.e., number of threads seen) */
	[[nodiscard]] auto size() const -> unsigned int override { return view_.size(); }

	/** Returns true if the clock is empty */
	[[nodiscard]] auto empty() const -> bool { return size() == 0; }

	void clear() override
	{
		view_.clear();
		holes_.clear();
	}

	/** Returns true if the clock contains e */
	[[nodiscard]] auto contains(Event e) const -> bool override;

	auto updateIdx(Event e) -> DepView & override
	{
		auto old = view_.getMax(e.thread);
		if (e.index > old) {
			view_.setMax(e);
			holes_[e.thread];
			addHolesInRange(Event(e.thread, old + 1), e.index);
		} else
			removeHole(e);
		return *this;
	}

	[[nodiscard]] auto getMax(int thread) const -> int override { return view_.getMax(thread); }

	void setMax(Event e) override
	{
		if (e.thread >= (int)view_.size())
			holes_[e.thread]; // grow
		auto old = view_.getMax(e.thread);
		view_.setMax(e);
		if (old < e.index)
			addHolesInRange(Event(e.thread, old + 1), e.index);
		else
			removeHolesInRange(e, old + 1);
	}

	/** Returns true if there's a hole in E's position */
	[[nodiscard]] auto hasHole(const Event e) const -> bool
	{
		return e.thread < holes_.size() && !holes_[e.thread].count(e.index);
	}

	/** Records that the event in the index of e has not been
	 * seen in the respective thread */
	void addHole(Event e);

	/** Similar to addHole(), but records that a range has not been seen */
	void addHolesInRange(Event start, int endIdx);

	/** Marks event e as seen */
	void removeHole(Event e);

	/** Marks all events of "thread" as seen */
	void removeAllHoles(int thread);

	/** Marks all events in a range as seen */
	void removeHolesInRange(Event start, int endIdx);

	/** Updates the view based on another clock. The update is valid
	 * only if the other clock provided is also a DepView */
	auto update(const View &v) -> View & override;
	auto update(const DepView &v) -> DepView & override;
	auto update(const VectorClock &vc) -> VectorClock & override;

	void printData(llvm::raw_ostream &s) const override;

	static auto classof(const VectorClock *vc) -> bool { return vc->getKind() == VC_DepView; }

private:
	/** A view containing the highest index seen for each thread */
	View view_;

	/** A view of holes, showing which events are not seen for each thread */
	HoleView holes_;
};

#endif /* GENMC_DEP_VIEW_HPP */
