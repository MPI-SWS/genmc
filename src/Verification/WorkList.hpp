/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <mixaskok@gmail.com>
 */

#ifndef GENMC_WORK_LIST_HPP
#define GENMC_WORK_LIST_HPP

#include "Verification/Revisit.hpp"
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

/*
 * WorkList class - Represents a list of TODOs for the driver.
 */
class WorkList {

public:
	using ItemT = std::unique_ptr<Revisit>;

	WorkList() = default;

	/** Returns whether this worklist is empty */
	[[nodiscard]] auto empty() const -> bool { return wlist_.empty(); }

	/** Adds an item to the worklist */
	void add(auto &&item) { wlist_.emplace_back(std::move(item)); }

	/** Returns the next item to examine (NULL if none) */
	auto getNext() -> ItemT
	{
		if (wlist_.empty())
			return {};

		auto item = std::move(wlist_.back());
		wlist_.pop_back();
		return std::move(item);
	}

	/* Overloaded operators */
	friend auto operator<<(llvm::raw_ostream &s, const WorkList &wlist) -> llvm::raw_ostream &;

private:
	/* Each stamp was associated with a bucket of TODOs before.
	 * This is now unnecessary, as even a simple stack suffices */
	std::vector<ItemT> wlist_;
};

#endif /* GENMC_WORK_LIST_HPP */
