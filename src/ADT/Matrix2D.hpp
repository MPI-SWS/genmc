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

#ifndef GENMC_MATRIX_2D_HPP
#define GENMC_MATRIX_2D_HPP

#include <llvm/Support/raw_ostream.h>

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
		friend auto operator<<(llvm::raw_ostream &s, const IndexMapper<F> &getter)
			-> llvm::raw_ostream &;

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
	friend auto operator<<(llvm::raw_ostream &s, const Matrix2D<U> &m) -> llvm::raw_ostream &;

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

#include "Matrix2D.tcc"

#endif /* GENMC_MATRIX_2D_HPP */
