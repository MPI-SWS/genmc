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

#include <algorithm>

template<typename T>
template<typename ITER>
VSet<T>::VSet(ITER begin, ITER end)
{
	for (; begin != end; ++begin) {
		if (size() && max() < *begin) {
			vset_.push_back(*begin);
		} else {
			insert(*begin);
		}
	}
}

template<typename T>
VSet<T>::VSet(std::initializer_list<T> il)
{
	for (auto it = il.begin(); it != il.end(); ++it) {
		if (size() && max() < *it) {
			vset_.push_back(*it);
		} else {
			insert(*it);
		}
	}
}

template<typename T>
VSet<T>::VSet(VSet<T> &&s) : vset_(std::move(s.vset_)) { }

template<typename T>
VSet<T> &VSet<T>::operator=(VSet<T> &&s)
{
	if (this != &s){
		vset_ = std::move(s.vset_);
	}
	return *this;
}

template<typename T>
typename VSet<T>::const_iterator VSet<T>::find(const T &el) const
{
	auto it = std::lower_bound(begin(), end(), el);
	return (it == end() || *it != el) ? end() : it;
}

template<typename T>
int VSet<T>::count(const T &el) const
{
	return (find(el) != end()) ? 1 : 0;
}

template<typename T>
std::pair<typename VSet<T>::iterator, bool> VSet<T>::insert(const T &el)
{
	auto it = std::lower_bound(begin(), end(), el);
	if (it == end() || *it != el)
		return std::make_pair(vset_.insert(it, el), true);
	return std::make_pair(it, false);
}

/*
 * A slightly optimized function for bulk insertions, that takes
 * into account the structure of a VSet
 */
template<typename T>
int VSet<T>::insert(const VSet<T> &s)
{
	/* Simply copy the contents of s if the current set is empty */
	if (empty()) {
		*this = s;
		return s.size();
	}

	/* Check s for trivial cases */
	if (s.empty())
		return 0;
	if (s.size() == 1)
		return insert(s[0]).second ? 1 : 0;

	/*
	 * First, count the elements of s not in this set, by iterating
	 * over the two sets in parallel
	 */
	auto count = 0;
	auto a = begin();
	auto b = s.begin();
	while (a != end() && b != s.end()) {
		/* If a[i] < b[i], maybe b[i] exists later in a */
		if (*a < *b) {
			++a;
		} else if (*a == *b) { /* b[i] exists in a, skip */
			++a;
			++b;
		} else { /* b[i] does not exist in a, increase count */
			++b;
			++count;
		}
	}

	/*
	 * If there are still elements in b that have not been processed,
	 * these should all should be inserted in a
	 */
	if (b != s.end())
		count += s.end() - b;

	if (count == 0)
		return 0;

	/*
	 * We will make the insertion in-place, in O(size(a) + size(b)) time.
	 * The new size of a will be size(a) + count. We do not use iterators
	 * because we need to resize a, and this would invalidate them.
	 */

	/* Keep the index of the last elements of a and b before resizing */
	int idxA = size() - 1;
	int idxB = s.size() - 1;
	vset_.resize(vset_.size() + count, vset_[0]); /* a is not empty */

	/* Iterate over the new a, and move fill each position appropriately */
	for (int i = size() - 1; i >= 0; i--) {
		if (idxA < 0 || (idxB >= 0 && (*this)[idxA] < s[idxB])) {
			/* No more elements in a, or a[idxA] < b[idxB] */
			vset_[i] = s[idxB];
			--idxB;
		} else if (idxA >= 0 && idxB >= 0 && (*this)[idxA] == s[idxB]) {
			/* since equal, it does not matter from where we copy */
			vset_[i] = (*this)[idxA];
			--idxA;
			--idxB;
		} else {
			/* No more elements in b, or a[i] > b[i] */
			vset_[i] = (*this)[idxA];
			--idxA;
		}
	}
	return count;
}

template<typename T>
template<typename ITER>
void VSet<T>::insert(ITER begin, ITER end)
{
	for (; begin != end; ++begin) {
		if (size() && max() < *begin) {
			vset_.push_back(*begin);
		} else {
			insert(*begin);
		}
	}
}


template<typename T>
int VSet<T>::erase(const T &el)
{
	auto it = std::lower_bound(begin(), end(), el);

	if (it == end() || *it != el)
		return 0;

	vset_.erase(it);
	return 1;
}

/* A slightly optimized function for bulk deletion */
template<typename T>
int VSet<T>::erase(const VSet<T> &s)
{
	if (empty() || s.empty())
		return 0;

	auto erased = 0;
	auto a = begin();
	auto b = s.begin();
	auto aMov = a.begin(); /* Next position of this set to be filled */

	/* While iterating over the two sets, fill a appropriately */
	while (a != end() && b != s.end()) {
		/*
		 * This element of a should be erased. aMov is left unchanged; *
		 * it is pointing to the next position that needs to be filled.
		 * We will iterate over a and b to find the appropriate element
		 * to fill *aMov (this should be an element of a)
		 */
		if (*a == *b) {
			++a;
			++b;
			++erased;
		} else if (*a < *b) {
			/* This element of a should remain in the set */
			if (aMov != a)
				*aMov = *a;
			++a;
			++aMov;
		} else {
			/* *a > *b, we need to check if *a appears later in b */
			++b;
		}
	}

	/* If we stopped whilst trying to find the next element for aMov... */
	if (aMov != a) {
		/* If a is not over copy the remaining elements */
		while (a != end()) {
			*aMov = *a;
			++a;
			++aMov;
		}
		/* Resize the vector appropriately */
		vset_.resize(aMov, vset_[0]);
	}
	return erased;
}

template<typename T>
bool VSet<T>::subsetOf(const VSet<T> &s) const
{
	if (size() > s.size())
		return false;

	auto a = begin();
	auto b = s.begin();
	while (a != end()) {
		/* If the remaining elements of a are more than those of b */
		if ((end() - a) > (s.end() - b))
			return false;
		/* If a contains an element not in b */
		if (*a < *b)
			return false;

		if (*a == *b) {
			++a;
			++b;
		} else {
			++b; /* Need to check further in b */
		}
	}
	return true;
}

template<typename T>
bool VSet<T>::intersects(const VSet<T> &s) const
{
	auto a = begin();
	auto b = s.begin();
	while(a != end() && b != s.end()) {
		if (*a == *b)
			return true;

		if (*a < *b)
			++a;
		else
			++b;
	}
	return false;
}

template<typename T>
VSet<T> VSet<T>::intersectWith(const VSet<T> &s) const
{
	VSet<T> result;

	auto a = begin();
	auto b = s.begin();
	while(a != end() && b != s.end()) {
		if (*a == *b) {
			result.insert(*a);
			++a;
			++b;
		} else if (*a < *b) {
			++a;
		} else {
			++b;
		}
	}
	return result;
}

template <typename T>
llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const VSet<T>& set)
{
	s << "[ ";
	for (auto i = 0u; i < set.size(); i++)
		s << set[i] << " ";
	s << "]";
	return s;
}
