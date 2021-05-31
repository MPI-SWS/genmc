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

#ifndef __VSET_HPP__
#define __VSET_HPP__

#include <llvm/Support/raw_ostream.h>
#include <functional>
#include <initializer_list>
#include <vector>

template<class T>
class VSet {

protected:
	/* Pre: Set needs to support random_access iterators */
	using Set = std::vector<T>;
	Set vset_;

public:

	VSet() : vset_() { };

	/* Pre: v is sorted and distinct */
	VSet(const std::vector<T> &v) : vset_(v) { };

	/* Pre: v is sorted and distinct */
	VSet(std::vector<T> &&v) : vset_(std::move(v)) { };

	template<typename ITER>
	VSet(ITER begin, ITER end);

	VSet(std::initializer_list<T> il);

	VSet(const VSet &) = default;

	VSet(VSet &&);

	VSet &operator=(const VSet&) = default;

	VSet &operator=(VSet&&);

	virtual ~VSet() {};

	using const_iterator = typename Set::const_iterator;
	using const_reverse_iterator = typename Set::const_reverse_iterator;

	const_iterator begin() const { return vset_.cbegin(); };
	const_iterator end() const { return vset_.cend(); };
	const_reverse_iterator rbegin() const { return vset_.crbegin(); };
	const_reverse_iterator rend() const { return vset_.crend(); };

	std::pair<const_iterator, bool> insert(const T &t);
	int insert(const VSet<T> &s);
	template<typename ITER>
	void insert(ITER begin, ITER end);

	int erase(const T &t);

	int erase(const VSet<T> &S);

	int count(const T &t) const;

	const_iterator find(const T &t) const;

	size_t size() const { return vset_.size(); };

	bool empty() const { return vset_.empty();};

	void clear() { vset_.clear(); };

	bool subsetOf(const VSet<T> &s) const;

	bool intersects(const VSet<T> &s) const;
	VSet<T> intersectWith(const VSet<T> &s) const;

	const T& min() const { return vset_[0]; };
	const T& max() const { return vset_.back(); };

	inline const T& operator[](int i) const { return vset_[i]; };

	template<typename U>
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const VSet<U>& set);
};

#include "VSet.tcc"

#endif /* __VSET_HPP__ */
