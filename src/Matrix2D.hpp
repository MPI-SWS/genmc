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

#ifndef __MATRIX_2D_HPP__
#define __MATRIX_2D_HPP__

#include <vector>

template <class T>
class Matrix2D {

public:
	/* Constructors */
	Matrix2D() {};
	Matrix2D(const std::vector<T> &es);
	Matrix2D(std::vector<T> &&es);

	/* Iterator typedefs */
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;
	using reverse_iterator = typename std::vector<T>::reverse_iterator;
	using const_revserse_iteratr = typename std::vector<T>::const_reverse_iterator;

	/* Iterators -- they iterate over the *elements* of a matrix */
	iterator begin() { return elems_.begin(); };
	iterator end() { return elems_.end(); };
	const_iterator begin() const { return elems_.begin(); };
	const_iterator end() const { return elems_.end(); };

	/* Returns the elements of this matrix */
	const std::vector<T>& getElems() const;

	/* Returns the index of a given element in the matrix */
	int getIndex(const T &e) const;

	/* Returns the number of incoming/outgoing edges */
	std::vector<T> getInEdges(const T &e) const;
	std::vector<T> getOutEdges(const T &e) const;

	/* Returns true if e has no incoming and outgoing edges */
	bool hasNoEdges(const T &e) const;

	/* Returns a vector that corresponds 1-to-1 to the in-degrees
	 * of the matrix's elements */
	std::vector<int> getInDegrees() const;

	/* Returns a topological sorting of the matrix */
	std::vector<T> topoSort() const;

	/* Calls "prop" on all topological sortings of the matrix,
	 * until one where "prop" returns true is found.
	 * Returns whether such a sorting is found */
	template<typename F>
	bool allTopoSort(F&& prop) const;

	/* Runs prop on each combination of topological sortings of matrices in
	 * "toCombine", until a combination that satisfies "prop" is found.
	 * Returns whether a valid combination was found */
	template<typename F>
	static bool combineAllTopoSort(const std::vector<Matrix2D<T> *> &toCombine, F&& prop);

	/* For each "f" in "froms", adds edges to all the "tos"*/
	void addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos);

	/* Empties the matrix */
	void clear() { matrix_.clear(); elems_.clear(); }

	/* Returns the number of elements in the matrix */
	unsigned int size() const;

	/* Returns true when the matrix has no elements */
	bool empty() const;

	/* Returns true if the matrix is irreflexive */
	bool isIrreflexive() const;

	/* Calculates the matrix's transitive closure */
	void transClosure();

	/* Operators */
	inline bool operator()(const T &a, const T &b) const {
		return !!matrix_[computeIndex(getIndex(a),getIndex(b))];
	};
	inline bool operator()(const T &a, unsigned int j) const {
		return !!matrix_[computeIndex(getIndex(a),j)];
	};
	inline bool operator()(unsigned int i, const T &b) const {
		return !!matrix_[computeIndex(i,getIndex(b))];
	};
	inline bool operator()(unsigned int i, unsigned int j) const {
		return !!matrix_[computeIndex(i,j)];
	};

	inline unsigned char& operator()(const T &a, const T &b) {
		return matrix_[computeIndex(getIndex(a),getIndex(b))];
	};
	inline unsigned char& operator()(const T &a, unsigned int j) {
		return matrix_[computeIndex(getIndex(a),j)];
	};
	inline unsigned char& operator()(unsigned int i, const T &b) {
		return matrix_[computeIndex(i, getIndex(b))];
	};
	inline unsigned char& operator()(unsigned int i, unsigned int j) {
		return matrix_[computeIndex(i,j)];
	};

	inline bool operator==(const Matrix2D<T> &m) const {
		return m.getElems() == getElems() && m.matrix_ == matrix_;
	};
	inline bool operator!=(const Matrix2D<T> &m) const { return !(*this == m); };

	template<typename U>
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const Matrix2D<U> &m);

private:
	/* Workhorse of allTopoSort() */
	template<typename F>
	bool allTopoSortUtil(std::vector<T> &current,
			     std::vector<bool> visited,
			     std::vector<int> &inDegree,
			     F&& prop, bool &found) const;

	/* Workhorse of combineAllTopoSort() */
	template<typename F>
	static bool combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					   bool &found, const std::vector<Matrix2D<T> *> &toCombine,
					   F&& prop);


#ifdef __MATRIX_2D_INCREMENTAL__
	static inline unsigned int computeIndex (unsigned int i, unsigned int j) {
		return (i < j) ? (j * j + i) : (i * i + i + j);
	}
#else
	inline unsigned int computeIndex (unsigned int i, unsigned int j) const {
		return i * elems_.size() + j;
	}
#endif

	using MatrixRep = std::vector<unsigned char>;

	MatrixRep matrix_;
	std::vector<T> elems_;
};

#include "Matrix2D.tcc"

#endif /* __MATRIX_2D_HPP__ */
