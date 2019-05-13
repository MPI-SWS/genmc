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

protected:
	using MatrixRep = std::vector<unsigned char>;

	MatrixRep matrix_;
	std::vector<T> elems_;

	void allTopoSortUtil(std::vector<std::vector<T> > &sortings,
			     std::vector<T> &current,
			     std::vector<bool> visited,
			     std::vector<int> &inDegree) const;

#ifdef __MATRIX_2D_INCREMENTAL__
	static inline unsigned int computeIndex (unsigned int i, unsigned int j) {
		return (i < j) ? (j * j + i) : (i * i + i + j);
	}
#else
	inline unsigned int computeIndex (unsigned int i, unsigned int j) const {
		return i * elems_.size() + j;
	}
#endif

public:

	Matrix2D() {};
	Matrix2D(const std::vector<T> &es);
	Matrix2D(std::vector<T> &&es);

	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;
	using reverse_iterator = typename std::vector<T>::reverse_iterator;
	using const_revserse_iteratr = typename std::vector<T>::const_reverse_iterator;

	iterator begin() { return elems_.begin(); };
	iterator end() { return elems_.end(); };
	const_iterator begin() const { return elems_.begin(); };
	const_iterator end() const { return elems_.end(); };

	const std::vector<T>& getElems() const;
	int getIndex(const T &e) const;
	std::vector<T> getInEdges(const T &e) const;
	std::vector<T> getOutEdges(const T &e) const;

	std::vector<int> getInDegrees() const;
	std::vector<T> topoSort() const;
	std::vector<std::vector<T> > allTopoSort() const;

	void addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos);

	void clear() { matrix_.clear(); elems_.clear(); }

	unsigned int size() const;
	bool empty() const;

	bool isReflexive() const;
	void transClosure();

	inline bool operator()(const T &a, const T &b) const { return !!matrix_[computeIndex(getIndex(a),getIndex(b))]; };
	inline bool operator()(const T &a, unsigned int j) const { return !!matrix_[computeIndex(getIndex(a),j)]; };
	inline bool operator()(unsigned int i, const T &b) const { return !!matrix_[computeIndex(i,getIndex(b))]; };
	inline bool operator()(unsigned int i, unsigned int j) const { return !!matrix_[computeIndex(i,j)]; };

	inline unsigned char& operator()(const T &a, const T &b) { return matrix_[computeIndex(getIndex(a),getIndex(b))]; };
	inline unsigned char& operator()(const T &a, unsigned int j) { return matrix_[computeIndex(getIndex(a),j)]; };
	inline unsigned char& operator()(unsigned int i, const T &b) { return matrix_[computeIndex(i, getIndex(b))]; };
	inline unsigned char& operator()(unsigned int i, unsigned int j) { return matrix_[computeIndex(i,j)]; };

	inline bool operator==(const Matrix2D<T> &m) const {
		return m.getElems() == getElems() && m.matrix_ == matrix_;
	};
	inline bool operator!=(const Matrix2D<T> &m) const { return !(*this == m); };

	template<typename U>
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const Matrix2D<U> &m);
};

#include "Matrix2D.tcc"

#endif /* __MATRIX_2D_HPP__ */
