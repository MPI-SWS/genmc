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

#ifndef GENMC_ADJ_LIST_HPP
#define GENMC_ADJ_LIST_HPP

#include <llvm/ADT/BitVector.h>
#include <llvm/Support/raw_ostream.h>

#include <functional>
#include <unordered_map>
#include <vector>

/** An adjacency-list representation of an ordered unlabeled graph */
template <class T, class Hash = std::hash<T>> class AdjList {

public:
	/** Simple aliases to easily defer what function arguments represent */
	using NodeId = unsigned int;
	using Timestamp = unsigned int;

	/** Node status during DFS exploration */
	enum class NodeStatus : std::uint8_t { unseen, entered, left };

	AdjList() = default;
	AdjList(const std::vector<T> &es) : elems(es)
	{
		auto size = elems.size();

		nodeSucc.resize(size);
		inDegree.resize(size);
		transC.resize(size);

		for (auto i = 0U; i < size; i++) {
			ids[elems[i]] = i;
			transC[i].resize(size);
		}
	}
	AdjList(std::vector<T> &&es) : elems(std::move(es))
	{
		auto size = elems.size();

		nodeSucc.resize(size);
		inDegree.resize(size);
		transC.resize(size);

		for (auto i = 0U; i < size; i++) {
			ids[elems[i]] = i;
			transC[i].resize(size);
		}
	}

	/** Iterator typedefs */
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;

	using adj_iterator = std::vector<NodeId>::iterator;
	using const_adj_iterator = std::vector<NodeId>::const_iterator;

	/** Iterators -- they iterate over the nodes of the graph */
	auto begin() -> iterator { return elems.begin(); };
	auto end() -> iterator { return elems.end(); };
	auto begin() const -> const_iterator { return elems.begin(); };
	auto end() const -> const_iterator { return elems.end(); };

	auto adj_begin(T el) -> adj_iterator { return nodeSucc[getIndex(el)].begin(); }
	auto adj_end(T el) -> adj_iterator { return nodeSucc[getIndex(el)].end(); }
	auto adj_begin(NodeId el) -> adj_iterator { return nodeSucc[el].begin(); }
	auto adj_end(NodeId el) -> adj_iterator { return nodeSucc[el].end(); }
	auto adj_begin(T el) const -> const_adj_iterator { return nodeSucc[getIndex(el)].begin(); }
	auto adj_end(T el) const -> const_adj_iterator { return nodeSucc[getIndex(el)].end(); }
	[[nodiscard]] auto adj_begin(NodeId el) const -> const_adj_iterator
	{
		return nodeSucc[el].begin();
	}
	[[nodiscard]] auto adj_end(NodeId el) const -> const_adj_iterator
	{
		return nodeSucc[el].end();
	}

	/** Returns the elements (nodes) of the graph */
	auto getElems() const -> const std::vector<T> & { return elems; }

	auto getIndex(T el) const -> unsigned int { return ids.at(el); }

	/** Returns the number of elements in the graph */
	[[nodiscard]] auto size() const -> unsigned int { return elems.size(); }

	/** Returns true when the graph has no elements */
	[[nodiscard]] auto empty() const -> bool { return size() == 0; }

	/** Adds a node to the graph */
	void addNode(T a);

	/** Adds a new edge to the graph */
	void addEdge(T a, T b);

	/** Helper for addEdge() that adds nodes with known IDs */
	void addEdge(NodeId a, NodeId b);

	/** For each "f" in "froms", adds edges to all the "tos"*/
	void addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos);

	/** Returns the in-degree of each element */
	[[nodiscard]] auto getInDegrees() const -> const std::vector<int> &;

	/** Returns true if the in-degree and out-degree of a node is 0 */
	auto hasNoEdges(T a) const -> bool
	{
		return inDegree[getIndex(a)] == 0 && nodeSucc[getIndex(a)].size() == 0;
	}

	/** Performs a DFS exploration */
	template <typename FVB, typename FET, typename FEB, typename FEF, typename FVE,
		  typename FEND>
	void dfs(FVB &&atEntryV, FET &&atTreeE, FEB &&atBackE, FEF &&atForwE, FVE &&atExitV,
		 FEND &&atEnd) const;

	/** Visits all reachable nodes starting from a in a DFS manner */
	template <typename FVB, typename FET, typename FEB, typename FEF, typename FVE,
		  typename FEND>
	void visitReachable(T a, FVB &&atEntryV, FET &&atTreeE, FEB &&atBackE, FEF &&atForwE,
			    FVE &&atExitV, FEND &&atEnd) const;

	/** Returns a topological sorting of the graph */
	auto topoSort() -> std::vector<T>;

	/** Runs prop on all topological sortings */
	template <typename F> auto allTopoSort(F &&prop) const -> bool;

	template <typename F>
	static auto combineAllTopoSort(const std::vector<AdjList<T, Hash> *> &toCombine, F &&prop)
		-> bool;

	void transClosure();

	auto isIrreflexive() -> bool;

	/** Returns true if the respective edge exists */
	auto operator()(const T a, const T b) const -> bool
	{
		return transC[getIndex(a)][getIndex(b)];
	}
	auto operator()(const T a, NodeId b) const -> bool { return transC[getIndex(a)][b]; }
	auto operator()(NodeId a, const T b) const -> bool { return transC[a][getIndex(b)]; }
	auto operator()(NodeId a, NodeId b) const -> bool { return transC[a][b]; }

	template <typename U, typename Z>
	friend auto operator<<(llvm::raw_ostream &s, const AdjList<U, Z> &l) -> llvm::raw_ostream &;

private:
	/** Helper for dfs() */
	template <typename FVB, typename FET, typename FEB, typename FEF, typename FVE>
	void dfsUtil(NodeId i, Timestamp &t, std::vector<NodeStatus> &m, std::vector<NodeId> &p,
		     std::vector<Timestamp> &d, std::vector<Timestamp> &f, FVB &&atEntryV,
		     FET &&atTreeE, FEB &&atBackE, FEF &&atForwE, FVE &&atExitV) const;

	template <typename F>
	auto allTopoSortUtil(std::vector<T> &current, std::vector<bool> visited,
			     std::vector<int> &inDegree, F &&prop, bool &found) const -> bool;

	template <typename F>
	static auto combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					   bool &found,
					   const std::vector<AdjList<T, Hash> *> &toCombine,
					   F &&prop) -> bool;

	/** The node elements.
	 * Must be in 1-1 correspondence with the successor list below */
	std::vector<T> elems;

	/** The successor list for each node */
	std::vector<std::vector<NodeId>> nodeSucc;

	std::vector<int> inDegree;

	/** Map that maintains the ID of each element */
	std::unordered_map<T, NodeId, Hash> ids;

	/** Maintain transitive closure info */
	bool calculatedTransC = false;
	std::vector<llvm::BitVector> transC;
};

#include "AdjList.tcc"

#endif /* GENMC_ADJ_LIST_HPP */
