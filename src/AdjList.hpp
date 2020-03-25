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

#ifndef __ADJ_LIST_HPP__
#define __ADJ_LIST_HPP__

#include <llvm/ADT/BitVector.h>
#include <llvm/Support/raw_ostream.h>

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <class T, class Hash = std::hash<T> >
class AdjList {

protected:
	/* Simple aliases to easily defer what function arguments represent */
	using NodeId = unsigned int;
	using Timestamp = unsigned int;

	/* Node status during DFS exploration */
	enum class NodeStatus { unseen, entered, left };

public:
	AdjList() {}
	AdjList(const std::vector<T> &es) : elems(es) {
		auto size = elems.size();

		nodeSucc.resize(size);
		inDegree.resize(size);
		transC.resize(size);
		calculatedTransC = false;

		for (auto i = 0u; i < size; i++) {
			ids[elems[i]] = i;
			transC[i].resize(size);
		}
	}
	AdjList(std::vector<T> &&es) : elems(std::move(es)) {
		auto size = elems.size();

		nodeSucc.resize(size);
		inDegree.resize(size);
		transC.resize(size);
		calculatedTransC = false;

		for (auto i = 0u; i < size; i++) {
			ids[elems[i]] = i;
			transC[i].resize(size);
		}
	}

	/* Iterator typedefs */
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;

	using adj_iterator = std::vector<NodeId>::iterator;
	using const_adj_iterator = std::vector<NodeId>::const_iterator;

	/* Iterators -- they iterate over the nodes of the graph */
	iterator begin() { return elems.begin(); };
	iterator end()   { return elems.end(); };
	const_iterator begin() const { return elems.begin(); };
	const_iterator end()   const { return elems.end(); };

	adj_iterator adj_begin(T a) { return nodeSucc[getIndex(a)].begin(); }
	adj_iterator adj_end(T a)   { return nodeSucc[getIndex(a)].end(); }
	adj_iterator adj_begin(NodeId a) { return nodeSucc[a].begin(); }
	adj_iterator adj_end(NodeId a)   { return nodeSucc[a].end(); }
	const_adj_iterator adj_begin(T a) const { return nodeSucc[getIndex(a)].begin(); }
	const_adj_iterator adj_end(T a)   const { return nodeSucc[getIndex(a)].end(); }
	const_adj_iterator adj_begin(NodeId a) const { return nodeSucc[a].begin(); }
	const_adj_iterator adj_end(NodeId a)   const { return nodeSucc[a].end(); }

	/* Returns the elements (nodes) of the graph */
	const std::vector<T> &getElems() const { return elems; }

	unsigned int getIndex(T a) const { return ids.at(a); }

	/* Adds a node to the graph */
	void addNode(T a);

	/* Adds a new edge to the graph */
	void addEdge(T a, T b);

	/* Helper for addEdge() that adds nodes with known IDs */
	void addEdge(NodeId a, NodeId b);

	/* For each "f" in "froms", adds edges to all the "tos"*/
	void addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos);

	/* Returns the in-degree of each element */
	const std::vector<int> &getInDegrees() const;

	/* Returns true if the in-degree and out-degree of a node is 0 */
	bool hasNoEdges(T a) const {
		return inDegree[getIndex(a)] == 0 &&
		       nodeSucc[getIndex(a)].size() == 0;
	}

	/* Performs a DFS exploration */
	template<typename FC, typename FN, typename FE, typename FEND>
	void dfs(FC&& onCycle, FN&& onNeighbor, FE&& atExplored, FEND&& atEnd);

	/* Returns a topological sorting of the graph */
	std::vector<T> topoSort();

	/* Runs prop on all topological sortings */
	template<typename F>
	bool allTopoSort(F&& prop) const;

	template<typename F>
	static bool combineAllTopoSort(const std::vector<AdjList<T, Hash> *> &toCombine, F&& prop);

	void transClosure();

	bool isIrreflexive();

	/* Returns true if the respective edge exists */
	inline bool operator()(const T a, const T b) const {
		return transC[getIndex(a)][getIndex(b)];
	}
	inline bool operator()(const T a, NodeId b) const {
		return transC[getIndex(a)][b];
	}
	inline bool operator()(NodeId a, const T b) const {
		return transC[a][getIndex(b)];
	}
	inline bool operator()(NodeId a, NodeId b) const {
		return transC[a][b];
	}

	template<typename U, typename Z>
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const AdjList<U, Z> &l);

private:
	/* Helper for dfs() */
	template<typename FC, typename FN, typename FE>
	void dfsUtil(NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
		     std::vector<NodeId> &p, std::vector<Timestamp> &d,
		     std::vector<Timestamp> &f, FC&& onCycle,
		     FN&& onNeighbor, FE&& atExplored);

	template<typename F>
	bool allTopoSortUtil(std::vector<T> &current, std::vector<bool> visited,
			     std::vector<int> &inDegree, F&& prop, bool &found) const;

	template<typename F>
	static bool combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					   bool &found, const std::vector<AdjList<T, Hash> *> &toCombine,
					   F&& prop);

	/* The node elements.
	 * Must be in 1-1 correspondence with the successor list below */
	std::vector<T> elems;

	/* The successor list for each node */
	std::vector<std::vector<NodeId> > nodeSucc;

	std::vector<int> inDegree;

	/* Map that maintains the ID of each element */
	std::unordered_map<T, NodeId, Hash> ids;

	/* Maintain transitive closure info */
	bool calculatedTransC = false;
	std::vector<llvm::BitVector> transC;
};

#include "AdjList.tcc"

#endif /* __ADJ_LIST_HPP__ */
