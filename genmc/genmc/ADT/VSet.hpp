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

#ifndef GENMC_VSET_HPP
#define GENMC_VSET_HPP

#include <algorithm>
#include <format>
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

	[[nodiscard]] auto begin() const -> const_iterator { return vset_.begin(); };
	[[nodiscard]] auto end() const -> const_iterator { return vset_.end(); };
	[[nodiscard]] auto rbegin() const -> const_reverse_iterator { return vset_.rbegin(); };
	[[nodiscard]] auto rend() const -> const_reverse_iterator { return vset_.rend(); };

	auto insert(const T &el) -> std::pair<const_iterator, bool>;
	auto insert(const VSet<T> &s) -> int;
	template <typename ITER> void insert(ITER begin, ITER end);

	auto erase(const T &el) -> int;
	auto erase(const VSet<T> &other) -> int;

	/** Return the number of elements in the set */
	[[nodiscard]] auto count(const T &el) const -> int;

	/** Returns whether the set contains EL */
	[[nodiscard]] auto contains(const T &el) const -> bool;

	[[nodiscard]] auto find(const T &el) const -> const_iterator;

	[[nodiscard]] auto size() const -> size_t { return vset_.size(); };

	/** Returns whether the set empty */
	[[nodiscard]] auto empty() const -> bool { return vset_.empty(); };

	/** Empties the set */
	void clear() { vset_.clear(); };

	/** Returns whether `THIS` is a subset of `S` */
	[[nodiscard]] auto subsetOf(const VSet<T> &s) const -> bool;

	/** Returns whether the intersection of `THIS` with `S` is non-empty */
	[[nodiscard]] auto intersects(const VSet<T> &s) const -> bool;

	/** Returns the intersection of `THIS` and `S` */
	[[nodiscard]] auto intersectWith(const VSet<T> &s) const -> VSet<T>;

	/** Returns the set `THIS\S` */
	[[nodiscard]] auto diff(const VSet<T> &s) const -> VSet<T>;

	[[nodiscard]] auto min() const -> const T & { return vset_[0]; };
	[[nodiscard]] auto max() const -> const T & { return vset_.back(); };

	auto operator[](int i) const -> const T & { return vset_[i]; };

	auto operator<=>(const VSet<T> &other) const = default;

	friend struct std::formatter<VSet<T>>;

private:
	Set vset_;
};

/** Make `VSet` formattable with `std::format`. */
template <typename T> struct std::formatter<VSet<T>> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const VSet<T> &set, std::format_context &ctx) const
	{
		auto out = std::format_to(ctx.out(), "[ ");
		for (const auto &elem : set.vset_) {
			out = std::format_to(out, "{} ", elem);
		}
		return std::format_to(out, "]");
	}
};

/**** VSet templates ****/

template <typename T> template <typename ITER> VSet<T>::VSet(ITER begin, ITER end)
{
	for (; begin != end; ++begin) {
		if (size() && max() < *begin) {
			vset_.push_back(*begin);
		} else {
			insert(*begin);
		}
	}
}

template <typename T> VSet<T>::VSet(std::initializer_list<T> il)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	for (auto it = il.begin(); it != il.end(); ++it) {
		if (size() && max() < *it) {
			vset_.push_back(*it);
		} else {
			insert(*it);
		}
	}
}

template <typename T> auto VSet<T>::find(const T &el) const -> typename VSet<T>::const_iterator
{
	auto it = std::lower_bound(begin(), end(), el);
	return (it == end() || *it != el) ? end() : it;
}

template <typename T> auto VSet<T>::count(const T &el) const -> int
{
	return (find(el) != end()) ? 1 : 0;
}

template <typename T> auto VSet<T>::contains(const T &el) const -> bool
{
	return find(el) != end();
}

template <typename T>
auto VSet<T>::insert(const T &el) -> std::pair<typename VSet<T>::const_iterator, bool>
{
	auto it = std::lower_bound(vset_.begin(), vset_.end(), el);
	if (it == end() || *it != el)
		return std::make_pair(vset_.insert(it, el), true);
	return std::make_pair(it, false);
}

/*
 * A slightly optimized function for bulk insertions, that takes
 * into account the structure of a VSet
 */
template <typename T> auto VSet<T>::insert(const VSet<T> &s) -> int
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
	auto aIt = begin();
	auto bIt = s.begin();
	while (aIt != end() && bIt != s.end()) {
		/* If aIt[i] < bIt[i], maybe bIt[i] exists later in aIt */
		if (*aIt < *bIt) {
			++aIt;
		} else if (*aIt == *bIt) { /* bIt[i] exists in aIt, skip */
			++aIt;
			++bIt;
		} else { /* bIt[i] does not exist in aIt, increase count */
			++bIt;
			++count;
		}
	}

	/*
	 * If there are still elements in bIt that have not been processed,
	 * these should all should be inserted in aIt
	 */
	if (bIt != s.end())
		count += s.end() - bIt;

	if (count == 0)
		return 0;

	/*
	 * We will make the insertion in-place, in O(size(a) + size(b)) time.
	 * The new size of a will be size(a) + count. We do not use iterators
	 * because we need to resize a, and this would invalidate them.
	 */

	/* Keep the index of the last elements of aIt and bIt before resizing */
	int idxA = size() - 1;
	int idxB = s.size() - 1;
	vset_.resize(vset_.size() + count, vset_[0]); /* aIt is not empty */

	/* Iterate over the new aIt, and move fill each position appropriately */
	for (int i = size() - 1; i >= 0; i--) {
		if (idxA < 0 || (idxB >= 0 && (*this)[idxA] < s[idxB])) {
			/* No more elements in aIt, or aIt[idxA] < bIt[idxB] */
			vset_[i] = s[idxB];
			--idxB;
		} else if (idxA >= 0 && idxB >= 0 && (*this)[idxA] == s[idxB]) {
			/* since equal, it does not matter from where we copy */
			vset_[i] = (*this)[idxA];
			--idxA;
			--idxB;
		} else {
			/* No more elements in bIt, or aIt[i] > bIt[i] */
			vset_[i] = (*this)[idxA];
			--idxA;
		}
	}
	return count;
}

template <typename T> template <typename ITER> void VSet<T>::insert(ITER begin, ITER end)
{
	for (; begin != end; ++begin) {
		if (size() && max() < *begin) {
			vset_.push_back(*begin);
		} else {
			insert(*begin);
		}
	}
}

template <typename T> auto VSet<T>::erase(const T &el) -> int
{
	auto it = std::lower_bound(begin(), end(), el);

	if (it == end() || *it != el)
		return 0;

	vset_.erase(it);
	return 1;
}

/* A slightly optimized function for bulk deletion */
template <typename T> auto VSet<T>::erase(const VSet<T> &other) -> int
{
	if (empty() || other.empty())
		return 0;

	auto erased = 0;
	auto aIdx = 0U; /* index in this */
	auto bIdx = 0U; /* index in other */
	auto aMov = 0U; /* Next position of this set to be filled */

	/* While iterating over the two sets, fill aIdx appropriately */
	while (aIdx < size() && bIdx < other.size()) {
		/*
		 * This element of aIdx should be erased. aMov is left unchanged; *
		 * it is pointing to the next position that needs to be filled.
		 * We will iterate over aIdx and bIdx to find the appropriate element
		 * to fill *aMov (this should be an element of aIdx)
		 */
		if (vset_[aIdx] == other.vset_[bIdx]) {
			++aIdx;
			++bIdx;
			++erased;
		} else if (vset_[aIdx] < other.vset_[bIdx]) {
			/* This element of aIdx should remain in the set */
			if (aMov != aIdx)
				vset_[aMov] = vset_[aIdx];
			++aIdx;
			++aMov;
		} else {
			/* *aIdx > *bIdx, we need to check if *aIdx appears later in bIdx */
			++bIdx;
		}
	}

	/* If we stopped whilst trying to find the next element for aMov... */
	if (aMov != aIdx) {
		/* If aIdx is not over copy the remaining elements */
		while (aIdx < size()) {
			vset_[aMov] = vset_[aIdx];
			++aIdx;
			++aMov;
		}
		/* Resize the vector appropriately */
		vset_.resize(aMov, vset_[0]);
	}
	return erased;
}

template <typename T> auto VSet<T>::subsetOf(const VSet<T> &s) const -> bool
{
	if (size() > s.size())
		return false;

	auto aIt = begin();
	auto bIt = s.begin();
	while (aIt != end()) {
		/* If the remaining elements of aIt are more than those of bIt */
		if ((end() - aIt) > (s.end() - bIt))
			return false;
		/* If aIt contains an element not in bIt */
		if (*aIt < *bIt)
			return false;

		if (*aIt == *bIt) {
			++aIt;
			++bIt;
		} else {
			++bIt; /* Need to check further in bIt */
		}
	}
	return true;
}

template <typename T> auto VSet<T>::intersects(const VSet<T> &s) const -> bool
{
	auto aIt = begin();
	auto bIt = s.begin();
	while (aIt != end() && bIt != s.end()) {
		if (*aIt == *bIt)
			return true;

		if (*aIt < *bIt)
			++aIt;
		else
			++bIt;
	}
	return false;
}

template <typename T> auto VSet<T>::intersectWith(const VSet<T> &s) const -> VSet<T>
{
	VSet<T> result;

	auto aIt = begin();
	auto bIt = s.begin();
	while (aIt != end() && bIt != s.end()) {
		if (*aIt == *bIt) {
			result.insert(*aIt);
			++aIt;
			++bIt;
		} else if (*aIt < *bIt) {
			++aIt;
		} else {
			++bIt;
		}
	}
	return result;
}

template <class T> auto VSet<T>::diff(const VSet<T> &s) const -> VSet<T>
{
	// TODO: optimize via a two-pointer algorithm
	VSet<T> result;
	for (const auto &e : vset_) {
		if (auto it = s.find(e); it == s.end())
			result.insert(e);
	}
	return result;
}

#endif /* GENMC_VSET_HPP */
