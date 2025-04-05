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

#include <vector>

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
auto operator<<(llvm::raw_ostream &s, const typename Matrix2D<T>::Mapper &gi) -> llvm::raw_ostream &
{
	for (auto &e : gi.elems_)
		s << e << " ";
	return s;
}

template <unsigned>
auto operator<<(llvm::raw_ostream &s, const typename Matrix2D<unsigned>::Mapper &gi)
	-> llvm::raw_ostream &
{
	for (auto i = 0U; i < gi.size(); ++i)
		s << i << " ";
	return s;
}

template <typename T>
auto operator<<(llvm::raw_ostream &s, const Matrix2D<T> &matrix) -> llvm::raw_ostream &
{
	s << "Elements: " << matrix.getMapper() << "\n";

	for (auto i = 0U; i < matrix.size(); i++) {
		for (auto j = 0U; j < matrix.size(); j++)
			s << matrix(i, j) << " ";
		s << "\n";
	}
	return s;
}
