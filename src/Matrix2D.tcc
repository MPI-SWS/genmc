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

#include <algorithm>

template <class T>
int binSearch(const std::vector<T> &arr, int len, T what)
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

template<typename T>
Matrix2D<T>::Matrix2D(const std::vector<T> &es) : matrix_(es.size() * es.size(), false), elems_(es) { }

template<typename T>
Matrix2D<T>::Matrix2D(std::vector<T> &&es) : matrix_(es.size() * es.size(), false), elems_(std::move(es))  { }

template<typename T>
const std::vector<T>& Matrix2D<T>::getElems() const
{
	return elems_;
}

template<typename T>
int Matrix2D<T>::getIndex(const T &e) const
{
	auto it = std::find(begin(), end(), e);
	BUG_ON(it == end());
	return it - begin();
}

template<typename T>
std::vector<T> Matrix2D<T>::getInEdges(const T &e) const
{
	std::vector<T> result;

	auto eI = getIndex(e);
	for (auto i = 0u; i < size(); i++) {
		if ((*this)(i, eI))
			result.push_back(elems_[i]);
	}
	return result;
}

template<typename T>
std::vector<T> Matrix2D<T>::getOutEdges(const T &e) const
{
	std::vector<T> result;

	auto eI = getIndex(e);
	for (auto i = 0u; i < size(); i++) {
		if ((*this)(eI, i))
			result.push_back(elems_[i]);
	}
	return result;
}

template<typename T>
bool Matrix2D<T>::hasNoEdges(const T &e) const
{
	auto eI = getIndex(e);
	for (auto i = 0u; i < size(); i++) {
		if ((*this)(i, eI) || (*this)(eI, i))
			return false;
	}
	return true;
}

/*
 * Get in-degrees for event es, according to adjacency matrix
 */
template<typename T>
std::vector<int> Matrix2D<T>::getInDegrees() const
{
	std::vector<int> inDegree(size(), 0);

	for (auto i = 0u; i < elems_.size(); i++)
		for (auto j = 0u; j < elems_.size(); j++)
			inDegree[i] += (int) (*this)(j,i);
	return inDegree;
}

template<typename T>
std::vector<T> Matrix2D<T>::topoSort() const
{
	std::vector<T> sorted;
	std::vector<unsigned int> stack;

	/* Get in-degrees for es, according to matrix */
	auto inDegree = getInDegrees();

	/* Propagate events with no incoming edges to stack */
	for (auto i = 0u; i < inDegree.size(); i++)
		if (inDegree[i] == 0)
			stack.push_back(i);

	/* Perform topological sorting, filling up sorted */
	while (stack.size() > 0) {
		/* Pop next node-ID, and push node into sorted */
		auto nextI = stack.back();
		sorted.push_back(elems_[nextI]);
		stack.pop_back();

		for (auto i = 0u; i < size(); i++) {
			/* Finds all nodes with incoming edges from nextI */
			if (!(*this)(nextI, i))
				continue;
			if (--inDegree[i] == 0)
				stack.push_back(i);
		}
	}

	/* Make sure that there is no cycle */
	BUG_ON(std::any_of(inDegree.begin(), inDegree.end(), [](int degI){ return degI > 0; }));
	return sorted;
}

template<typename T>
template<typename F>
bool Matrix2D<T>::allTopoSortUtil(std::vector<T> &current,
				  std::vector<bool> visited,
				  std::vector<int> &inDegree,
				  F&& prop, bool &found) const
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
	auto &es = getElems();

	for (auto i = 0u; i < es.size(); i++) {
		/* If ith-event can be added */
		if (inDegree[i] == 0 && !visited[i]) {
			/* Reduce in-degrees of its neighbors */
			for (auto j = 0u; j < es.size(); j++)
				if ((*this)(i, j))
					--inDegree[j];
			/* Add event in current sorting, mark as visited, and recurse */
			current.push_back(es[i]);
			visited[i] = true;

			allTopoSortUtil(current, visited, inDegree, prop, found);

			/* If the recursion yielded a sorting satisfying prop, stop */
			if (found)
				return true;

			/* Reset visited, current sorting, and inDegree */
			visited[i] = false;
			current.pop_back();
			for (auto j = 0u; j < es.size(); j++)
				if ((*this)(i, j))
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

template<typename T>
template<typename F>
bool Matrix2D<T>::allTopoSort(F&& prop) const
{
	std::vector<bool> visited(size(), false);
	std::vector<T> current;
	auto inDegree = getInDegrees();
	auto found = false;

	return allTopoSortUtil(current, visited, inDegree, prop, found);
}

template<typename T>
template<typename F>
bool Matrix2D<T>::combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					 bool &found, const std::vector<Matrix2D<T> *> &toCombine,
					 F&& prop)
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
	toCombine[index]->allTopoSort([&](std::vector<T> &sorting){
			current.push_back(sorting);
			auto res = combineAllTopoSortUtil(index + 1, current, found, toCombine, prop);
			current.pop_back();
			return res;
		});
	return found;
}

template<typename T>
template<typename F>
bool Matrix2D<T>::combineAllTopoSort(const std::vector<Matrix2D<T> *> &toCombine, F&& prop)
{
	std::vector<std::vector<T>> current; /* The current sorting for each matrix */
	bool found = false;

	return combineAllTopoSortUtil(0, current, found, toCombine, prop);
}

template<typename T>
void Matrix2D<T>::addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos)
{
	for (auto &f : froms)
		for (auto &t : tos)
			(*this)(f, t) = true;
}

template<typename T>
unsigned int Matrix2D<T>::size() const
{
	return elems_.size();
}

template<typename T>
bool Matrix2D<T>::empty() const
{
	return size() == 0;
}

template<typename T>
bool Matrix2D<T>::isIrreflexive() const
{
	for (auto i = 0u; i < size(); i++)
		if ((*this)(i, i))
			return false;
	return true;
}

template<typename T>
void Matrix2D<T>::transClosure()
{
	auto len = (int) size();
#ifdef __MATRIX_2D_INCREMENTAL__
	for (auto i = 1; i < len; i++)
		for (auto k = 0; k < i; k++)
			if (matrix_[i * i + i + k]) {
				auto j = 0;
				auto ij_index = i * (i + 1);
				auto kj_index = k * (k + 1);
				for (; j <= k; j++, ij_index++, kj_index++)
					matrix_[ij_index] = matrix_[ij_index] | matrix_[kj_index];
				kj_index += k;
				for (; j <= i; j++, ij_index++, kj_index += j + j - 1)
					matrix_[ij_index] = matrix_[ij_index] | matrix_[kj_index];
				ij_index += i;
				for (; j < len; j++, ij_index += j + j - 1, kj_index += j + j - 1)
					matrix_[ij_index] = matrix_[ij_index] | matrix_[kj_index];
			}
	for (auto i = 0; i < len - 1; i++)
		for (auto k = i + 1; k < len; k++)
			if (matrix_[k * k + i]) {
				auto j = 0;
				auto ij_index = i * (i + 1);
				auto kj_index = k * (k + 1);
				for (; j <= i; j++, ij_index++, kj_index++)
					matrix_[ij_index] = matrix_[ij_index] | matrix_[kj_index];
				ij_index += i;
				for (; j <= k; j++, ij_index += j + j - 1, kj_index++)
					matrix_[ij_index] = matrix_[ij_index] | matrix_[kj_index];
				kj_index += k;
				for (; j < len; j++, ij_index += j + j - 1, kj_index += j + j - 1)
					matrix_[ij_index] = matrix_[ij_index] | matrix_[kj_index];
			}
#else
	for (auto i = 1; i < len; i++)
		for (auto k = 0; k < i; k++)
			if (matrix_[i * len + k])
				for (auto j = 0; j < len; j++)
					matrix_[i * len + j] = matrix_[i * len + j] | matrix_[k * len + j];
	for (auto i = 0; i < len - 1; i++)
		for (auto k = i + 1; k < len; k++)
			if (matrix_[i * len + k])
				for (auto j = 0; j < len; j++)
					matrix_[i * len + j] = matrix_[i * len + j] | matrix_[k * len + j];
#endif
}

template<typename T>
llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const Matrix2D<T> &m)
{
	s << "Elements: ";
	for (auto &e : m.getElems())
		s << e << " ";
	s << "\n";

	for (auto i = 0u; i < m.size(); i++) {
		for (auto j = 0u; j < m.size(); j++)
			s << m(i, j) << " ";
		s << "\n";
	}
	return s;
}
