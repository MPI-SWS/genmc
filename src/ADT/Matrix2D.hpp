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

#ifndef GENMC_MATRIX_2D_HPP
#define GENMC_MATRIX_2D_HPP

#include <algorithm>
#include <ostream>
#include <ranges>
#include <utility>
#include <vector>

/**
 * An efficient 2D-matrix representation.
 * The elements are mapped to integers, and the matrix is represented as a vector of integers.
 */
template <typename T> class Matrix2D {

private:
	/**
	 * Helper class to get the element mapping, that also provides a
	 * specialization for the case where T = unsigned.
	 *
	 * We cannot have nested class specialization without first
	 * specializing the outer class, so we leverage partial specialization
	 * and introduce a dummy parameter.
	 */
	template <typename U, typename Dummy = void> class IndexMapper {

	public:
		explicit IndexMapper(const std::vector<U> &es) : elems_(es) {}
		explicit IndexMapper(std::vector<U> &&es) : elems_(std::move(es)) {};

		auto operator()(const U &e) const -> unsigned int
		{
			auto it = std::ranges::find(elems_.begin(), elems_.end(), e);
			BUG_ON(it == elems_.end());
			return it - elems_.begin();
		}

		auto getElem(unsigned int i) const -> const U & { return elems_[i]; }

		[[nodiscard]] auto size() const -> unsigned int { return elems_.size(); }

		void clear() { elems_.clear(); }

		template <typename F>
		friend auto operator<<(std::ostream &s, const IndexMapper<F> &getter)
			-> std::ostream &;

	private:
		std::vector<U> elems_;
	};

	template <typename Dummy> class IndexMapper<unsigned int, Dummy> {

	public:
		explicit IndexMapper(unsigned int s) : size_(s) {}

		auto operator()(unsigned int e) const -> unsigned int { return e; }

		[[nodiscard]] auto getElem(unsigned int e) const -> unsigned int { return e; }

		[[nodiscard]] auto size() const -> unsigned int { return size_; }

		void clear() { size_ = 0; }

	private:
		unsigned int size_;
	};

	using Mapper = IndexMapper<T>;

public:
	/** Constructor */
	template <typename... Args>
	Matrix2D(Args &&...args)
		: indexMapper_(std::forward<Args>(args)...),
		  matrix_(indexMapper_.size() * indexMapper_.size(), false)
	{}

	/** Returns the number of incoming/outgoing edges */
	[[nodiscard]] auto getInEdges(const T &e) const -> std::vector<T>;
	[[nodiscard]] auto getOutEdges(const T &e) const -> std::vector<T>;

	/** Returns true if e has no incoming and outgoing edges */
	[[nodiscard]] auto hasNoEdges(const T &e) const -> bool;

	/** Return true if the node is in the Matrix */
	[[nodiscard]] auto hasElement(const T &e) const -> bool;

	/** Returns a vector that corresponds 1-to-1 to the in-degrees
	 * of the matrix's elements */
	[[nodiscard]] auto getInDegrees() const -> std::vector<int>;

	/** Returns a topological sorting of the matrix */
	[[nodiscard]] auto topoSort() const -> std::vector<T>;

	/** Calls "prop" on all topological sortings of the matrix,
	 * until one where "prop" returns true is found.
	 * Returns whether such a sorting is found */
	template <typename F> auto allTopoSort(F &&prop) const -> bool;

	/** Runs prop on each combination of topological sortings of matrices in
	 * "toCombine", until a combination that satisfies "prop" is found.
	 * Returns whether a valid combination was found */
	template <typename F>
	static auto combineAllTopoSort(const std::vector<Matrix2D<T> *> &toCombine, F &&prop)
		-> bool;

	/** For each "f" in "froms", adds edges to all the "tos"*/
	void addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos);

	/** Adds the edge a->b */
	void addEdge(const T &a, const T &b);

	/** Adds the edge a-> and transitively closes */
	void addEdgeAndTransitive(const T &a, const T &b);

	/** Adds the edges in the range of pairs RANGE */
	void addEdges(std::ranges::input_range auto &&range)
	{
		for (const auto &v : range)
			addEdge(v.first, v.second);
	}

	/** Empties the matrix */
	void clear()
	{
		matrix_.clear();
		indexMapper_.clear();
	}

	/** Returns the number of elements in the matrix */
	[[nodiscard]] auto size() const -> unsigned int { return getMapper().size(); }

	/** Returns true when the matrix has no elements */
	[[nodiscard]] auto empty() const -> bool;

	/** Returns true if the matrix is irreflexive */
	[[nodiscard]] auto isIrreflexive() const -> bool;

	/** Transitively closes the matrix */
	void transClosure();

	/** Operators */
	auto operator()(const T &a, const T &b) const -> bool
	{
		return at(getMapper()(a), getMapper()(b));
	}
	auto operator()(const T &a, const T &b) -> unsigned char &
	{
		return at(getMapper()(a), getMapper()(b));
	}

	auto operator==(const Matrix2D<T> &m) const -> bool
	{
		return m.getMapper() == getMapper() && m.matrix_ == matrix_;
	}
	auto operator!=(const Matrix2D<T> &m) const -> bool { return !(*this == m); }

	template <typename U>
	friend auto operator<<(std::ostream &s, const Matrix2D<U> &m) -> std::ostream &;

private:
	/** Workhorse of allTopoSort() */
	template <typename F>
	auto allTopoSortUtil(std::vector<T> &current, std::vector<bool> visited,
			     std::vector<int> &inDegree, F &&prop, bool &found) const -> bool;

	/** Workhorse of combineAllTopoSort() */
	template <typename F>
	static auto combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					   bool &found, const std::vector<Matrix2D<T> *> &toCombine,
					   F &&prop) -> bool;

	/** Indexing */
	[[nodiscard]] auto computeIndex(unsigned int i, unsigned int j) const -> unsigned int
	{
		return (i * size()) + j;
	}

	[[nodiscard]] auto at(unsigned int a, unsigned int b) const -> bool
	{
		return !!matrix_[computeIndex(a, b)];
	};

	auto at(unsigned int a, unsigned int b) -> unsigned char &
	{
		return matrix_[computeIndex(a, b)];
	};

	[[nodiscard]] auto getMapper() const -> const Mapper & { return indexMapper_; }
	auto getMapper() -> Mapper & { return indexMapper_; }

	using MatrixRep = std::vector<unsigned char>;

	Mapper indexMapper_;
	MatrixRep matrix_;
};

/**** Matrix2D templates ****/

template <class T> auto binSearch(const std::vector<T> &arr, int len, T what) -> int
{
	int low = 0;
	int high = len - 1;
	while (low <= high) {
		int mid = (low + high) / 2;
		if (arr[mid] > what)
			high = mid - 1;
		else if (arr[mid] < what)
			low = mid + 1;
		else
			return mid;
	}
	return -1; /* not found */
}

template <typename T> auto Matrix2D<T>::getInEdges(const T &e) const -> std::vector<T>
{
	std::vector<T> result;

	auto eI = getMapper()(e);
	for (auto i = 0U; i < size(); i++) {
		if (at(i, eI))
			result.push_back(getMapper().getElem(i));
	}
	return result;
}

template <typename T> auto Matrix2D<T>::getOutEdges(const T &e) const -> std::vector<T>
{
	std::vector<T> result;

	auto eI = getMapper()(e);
	for (auto i = 0U; i < size(); i++) {
		if (at(eI, i))
			result.push_back(getMapper().getElem(i));
	}
	return result;
}

template <typename T> auto Matrix2D<T>::hasNoEdges(const T &e) const -> bool
{
	auto eI = getMapper()(e);
	for (auto i = 0U; i < size(); i++) {
		if (at(i, eI) || at(eI, i))
			return false;
	}
	return true;
}

template <typename T> auto Matrix2D<T>::hasElement(const T &e) const -> bool
{
	return std::find(getMapper().begin(), getMapper().end(), e) != getMapper().end();
}

/*
 * Get in-degrees for event es, according to adjacency matrix
 */
template <typename T> auto Matrix2D<T>::getInDegrees() const -> std::vector<int>
{
	std::vector<int> inDegree(size(), 0);

	for (auto i = 0U; i < size(); i++)
		for (auto j = 0U; j < size(); j++)
			inDegree[i] += (int)at(j, i);
	return inDegree;
}

template <typename T> auto Matrix2D<T>::topoSort() const -> std::vector<T>
{
	std::vector<T> sorted;
	std::vector<unsigned int> stack;

	/* Get in-degrees for es, according to matrix */
	auto inDegree = getInDegrees();

	/* Propagate events with no incoming edges to stack */
	for (auto i = 0U; i < inDegree.size(); i++)
		if (inDegree[i] == 0)
			stack.push_back(i);

	/* Perform topological sorting, filling up sorted */
	while (stack.size() > 0) {
		/* Pop next node-ID, and push node into sorted */
		auto nextI = stack.back();
		sorted.push_back(getMapper().getElem(nextI));
		stack.pop_back();

		for (auto i = 0U; i < size(); i++) {
			/* Finds all nodes with incoming edges from nextI */
			if (!at(nextI, i))
				continue;
			if (--inDegree[i] == 0)
				stack.push_back(i);
		}
	}

	/* Make sure that there is no cycle */
	BUG_ON(std::any_of(inDegree.begin(), inDegree.end(), [](int degI) { return degI > 0; }));
	return sorted;
}

template <typename T>
template <typename F>
auto Matrix2D<T>::allTopoSortUtil(std::vector<T> &current, std::vector<bool> visited,
				  std::vector<int> &inDegree, F &&prop, bool &found) const -> bool
{
	/* If we have already found a sorting satisfying "prop", return */
	if (found)
		return true;
	/*
	 * The boolean variable 'scheduled' indicates whether this recursive call
	 * has added (scheduled) one event (at least) to the current topological sorting.
	 * If no event was added, a full topological sort has been produced.
	 */
	auto scheduled = false;

	for (auto i = 0U; i < size(); i++) {
		/* If ith-event can be added */
		if (inDegree[i] == 0 && !visited[i]) {
			/* Reduce in-degrees of its neighbors */
			for (auto j = 0U; j < size(); j++)
				if (at(i, j))
					--inDegree[j];
			/* Add event in current sorting, mark as visited, and recurse */
			current.push_back(getMapper().getElem(i));
			visited[i] = true;

			allTopoSortUtil(current, visited, inDegree, prop, found);

			/* If the recursion yielded a sorting satisfying prop, stop */
			if (found)
				return true;

			/* Reset visited, current sorting, and inDegree */
			visited[i] = false;
			current.pop_back();
			for (auto j = 0U; j < size(); j++)
				if (at(i, j))
					++inDegree[j];
			/* Mark that at least one event has been added to the current sorting */
			scheduled = true;
		}
	}

	/*
	 * We reach this point if no events were added in the current sorting, meaning
	 * that this is a complete sorting
	 */
	if (!scheduled) {
		if (prop(current))
			found = true;
	}
	return found;
}

template <typename T> template <typename F> auto Matrix2D<T>::allTopoSort(F &&prop) const -> bool
{
	std::vector<bool> visited(size(), false);
	std::vector<T> current;
	auto inDegree = getInDegrees();
	auto found = false;

	return allTopoSortUtil(current, visited, inDegree, prop, found);
}

template <typename T>
template <typename F>
auto Matrix2D<T>::combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					 bool &found, const std::vector<Matrix2D<T> *> &toCombine,
					 F &&prop) -> bool
{
	/* If we have found a valid combination already, return */
	if (found)
		return true;

	/* Base case: a combination of sortings has been reached */
	BUG_ON(index > toCombine.size());
	if (index == toCombine.size()) {
		if (prop(current))
			found = true;
		return found;
	}

	/* Otherwise, we have more matrices to extend */
	toCombine[index]->allTopoSort([&](std::vector<T> &sorting) {
		current.push_back(sorting);
		auto res = combineAllTopoSortUtil(index + 1, current, found, toCombine, prop);
		current.pop_back();
		return res;
	});
	return found;
}

template <typename T>
template <typename F>
auto Matrix2D<T>::combineAllTopoSort(const std::vector<Matrix2D<T> *> &toCombine, F &&prop) -> bool
{
	std::vector<std::vector<T>> current; /* The current sorting for each matrix */
	bool found = false;

	return combineAllTopoSortUtil(0, current, found, toCombine, prop);
}

template <typename T>
void Matrix2D<T>::addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos)
{
	for (auto &f : froms)
		for (auto &t : tos)
			(*this)(f, t) = true;
}

template <typename T> void Matrix2D<T>::addEdge(const T &a, const T &b) { (*this)(a, b) = true; }

template <typename T> auto Matrix2D<T>::empty() const -> bool { return size() == 0; }

template <typename T> auto Matrix2D<T>::isIrreflexive() const -> bool
{
	for (auto i = 0U; i < size(); i++)
		if (at(i, i))
			return false;
	return true;
}

template <typename T> void Matrix2D<T>::transClosure()
{
	auto len = (int)size();
	for (auto i = 1; i < len; i++)
		for (auto k = 0; k < i; k++)
			if (matrix_[i * len + k])
				for (auto j = 0; j < len; j++)
					matrix_[i * len + j] |= matrix_[k * len + j];
	for (auto i = 0; i < len - 1; i++)
		for (auto k = i + 1; k < len; k++)
			if (matrix_[i * len + k])
				for (auto j = 0; j < len; j++)
					matrix_[i * len + j] |= matrix_[k * len + j];
}

template <typename T> void Matrix2D<T>::addEdgeAndTransitive(const T &a, const T &b)
{
	auto aI = getMapper()(a);
	auto bI = getMapper()(b);
	auto len = size();
	for (auto i = 0U; i < len; i++)
		if (at(i, aI))
			for (auto j = 0U; j < len; j++)
				at(i, j) |= at(bI, j);
	for (auto j = 0; j < len; j++)
		at(aI, j) |= at(bI, j);
	for (auto i = 0; i < len; i++)
		at(i, bI) |= at(i, aI);
	at(aI, bI) = true;
}

template <typename T>
auto operator<<(std::ostream &s, const typename Matrix2D<T>::Mapper &gi) -> std::ostream &
{
	for (auto &e : gi.elems_)
		s << e << " ";
	return s;
}

template <unsigned>
auto operator<<(std::ostream &s, const typename Matrix2D<unsigned>::Mapper &gi) -> std::ostream &
{
	for (auto i = 0U; i < gi.size(); ++i)
		s << i << " ";
	return s;
}

template <typename T> auto operator<<(std::ostream &s, const Matrix2D<T> &matrix) -> std::ostream &
{
	s << "Elements: " << matrix.getMapper() << "\n";

	for (auto i = 0U; i < matrix.size(); i++) {
		for (auto j = 0U; j < matrix.size(); j++)
			s << matrix(i, j) << " ";
		s << "\n";
	}
	return s;
}

#endif /* GENMC_MATRIX_2D_HPP */
