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

#ifndef GENMC_VSET_HPP
#define GENMC_VSET_HPP

#include <llvm/Support/raw_ostream.h>

#include <initializer_list>
#include <vector>

/** A set implemented as a sorted vector */
template <class T> class VSet {

public:
	/* Pre: Set needs to support random_access iterators */
	using Set = std::vector<T>;

	VSet() : vset_() {};

	template <typename ITER> VSet(ITER begin, ITER end);

	VSet(const std::vector<T> &v) : VSet(v.begin(), v.end()) {}

	VSet(std::vector<T> &&v) : vset_(std::move(v)) { std::sort(vset_.begin(), vset_.end()); }

	VSet(std::initializer_list<T> il);

	VSet(const VSet &) = default;

	VSet(VSet &&) = default;

	auto operator=(const VSet &) -> VSet & = default;

	auto operator=(VSet &&) -> VSet & = default;

	virtual ~VSet() = default;

	using const_iterator = typename Set::const_iterator;
	using const_reverse_iterator = typename Set::const_reverse_iterator;

	auto begin() const -> const_iterator { return vset_.begin(); };
	auto end() const -> const_iterator { return vset_.end(); };
	auto rbegin() const -> const_reverse_iterator { return vset_.rbegin(); };
	auto rend() const -> const_reverse_iterator { return vset_.rend(); };

	auto insert(const T &el) -> std::pair<const_iterator, bool>;
	auto insert(const VSet<T> &s) -> int;
	template <typename ITER> void insert(ITER begin, ITER end);

	auto erase(const T &el) -> int;
	auto erase(const VSet<T> &S) -> int;

	/** Return the number of elements in the set */
	auto count(const T &el) const -> int;

	/** Returns whether the set contains EL */
	auto contains(const T &el) const -> bool;

	auto find(const T &el) const -> const_iterator;

	[[nodiscard]] auto size() const -> size_t { return vset_.size(); };

	/** Returns whether the set empty */
	[[nodiscard]] auto empty() const -> bool { return vset_.empty(); };

	/** Empties the set */
	void clear() { vset_.clear(); };

	/** Returns whether THIS is a subset of S */
	auto subsetOf(const VSet<T> &s) const -> bool;

	/** Returns whether the intersection of THIS with S is non-empty */
	auto intersects(const VSet<T> &s) const -> bool;

	/** Returns the intersection of THIS and S */
	auto intersectWith(const VSet<T> &s) const -> VSet<T>;

	/** Returns the set THIS\S */
	auto diff(const VSet<T> &s) const -> VSet<T>;

	auto min() const -> const T & { return vset_[0]; };
	auto max() const -> const T & { return vset_.back(); };

	auto operator[](int i) const -> const T & { return vset_[i]; };

	auto operator<=>(const VSet<T> &other) const = default;

	template <typename U>
	friend auto operator<<(llvm::raw_ostream &s, const VSet<U> &set) -> llvm::raw_ostream &;

private:
	Set vset_;
};

#include "VSet.tcc"

#endif /* GENMC_VSET_HPP */
