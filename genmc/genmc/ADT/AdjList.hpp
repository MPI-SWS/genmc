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

#ifndef GENMC_ADJ_LIST_HPP
#define GENMC_ADJ_LIST_HPP

#include "genmc/ADT/BitVector.hpp"
#include "genmc/Support/Error.hpp"

#include <functional>
#include <ostream>
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
	friend auto operator<<(std::ostream &s, const AdjList<U, Z> &l) -> std::ostream &;

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
	std::vector<genmc::BitVector> transC;
};

/**** AdjList templates ****/

template <typename T, typename H> void AdjList<T, H>::addNode(T a)
{
	auto id = elems.size();

	ids[a] = id;
	elems.push_back(a);
	nodeSucc.push_back({});
	inDegree.push_back(0);

	calculatedTransC = false;
	transC.push_back(genmc::BitVector(id));
}

template <typename T, typename H> void AdjList<T, H>::addEdge(NodeId a, NodeId b)
{
	/* If edge already exists, nothing to do */
	if ((*this)(a, b))
		return;

	nodeSucc[a].push_back(b);
	transC[a].set(b);
	++inDegree[b];
	calculatedTransC = false;
}

template <typename T, typename H> void AdjList<T, H>::addEdge(T a, T b)
{
	addEdge(getIndex(a), getIndex(b));
}

template <typename T, typename H>
void AdjList<T, H>::addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos)
{
	for (auto &f : froms)
		for (auto &t : tos)
			addEdge(f, t);
}

template <typename T, typename H>
auto AdjList<T, H>::getInDegrees() const -> const std::vector<int> &
{
	return inDegree;
}

template <typename T, typename H>
template <typename FVB, typename FET, typename FEB, typename FEF, typename FVE>
void AdjList<T, H>::dfsUtil(NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f, FVB &&atEntryV, FET &&atTreeE, FEB &&atBackE,
			    FEF &&atForwE, FVE &&atExitV) const
{
	m[i] = NodeStatus::entered;
	d[i] = ++t;
	atEntryV(i, t, m, p, d, f);
	for (const auto &j : nodeSucc[i]) {
		if (m[j] == NodeStatus::unseen) {
			p[j] = i;
			dfsUtil(j, t, m, p, d, f, atEntryV, atTreeE, atBackE, atForwE, atExitV);
			atTreeE(i, j, t, m, p, d, f);
		} else if (m[j] == NodeStatus::entered) {
			atBackE(i, j, t, m, p, d, f);
		} else {
			atForwE(i, j, t, m, p, d, f);
		}
	}
	m[i] = NodeStatus::left;
	f[i] = ++t;
	atExitV(i, t, m, p, d, f);
}

template <typename T, typename H>
template <typename FVB, typename FET, typename FEB, typename FEF, typename FVE, typename FEND>
void AdjList<T, H>::dfs(FVB &&atEntryV, FET &&atTreeE, FEB &&atBackE, FEF &&atForwE, FVE &&atExitV,
			FEND &&atEnd) const
{
	Timestamp t = 0;
	std::vector<NodeStatus> m(nodeSucc.size(), NodeStatus::unseen); /* Node status */
	std::vector<NodeId> p(nodeSucc.size(), -42);			/* Node parent */
	std::vector<Timestamp> d(nodeSucc.size(), 0);			/* First visit */
	std::vector<Timestamp> f(nodeSucc.size(), 0);			/* Last visit */

	for (auto i = 0u; i < nodeSucc.size(); i++) {
		if (m[i] == NodeStatus::unseen)
			dfsUtil(i, t, m, p, d, f, atEntryV, atTreeE, atBackE, atForwE, atExitV);
	}
	atEnd(t, m, p, d, f);
}

template <typename T, typename H>
template <typename FVB, typename FET, typename FEB, typename FEF, typename FVE, typename FEND>
void AdjList<T, H>::visitReachable(T a, FVB &&atEntryV, FET &&atTreeE, FEB &&atBackE, FEF &&atForwE,
				   FVE &&atExitV, FEND &&atEnd) const
{
	Timestamp t = 0;
	std::vector<NodeStatus> m(nodeSucc.size(), NodeStatus::unseen); /* Node status */
	std::vector<NodeId> p(nodeSucc.size(), -42);			/* Node parent */
	std::vector<Timestamp> d(nodeSucc.size(), 0);			/* First visit */
	std::vector<Timestamp> f(nodeSucc.size(), 0);			/* Last visit */

	dfsUtil(getIndex(a), t, m, p, d, f, atEntryV, atTreeE, atBackE, atForwE, atExitV);
	atEnd(t, m, p, d, f);
}

template <typename T, typename H> auto AdjList<T, H>::topoSort() -> std::vector<T>
{
	std::vector<T> sort;

	dfs([&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m, std::vector<NodeId> &p,
		std::vector<Timestamp> &d, std::vector<Timestamp> &f) { return; }, /* atEntryV */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f) { return; }, /* atTreeE */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f) { UNREACHABLE(); }, /* atBackE */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f) { return; }, /* atForwE*/
	    [&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m, std::vector<NodeId> &p,
		std::vector<Timestamp> &d, std::vector<Timestamp> &f) { /* atExitV */
									sort.push_back(elems[i]);
									return;
	    },
	    [&](Timestamp &t, std::vector<NodeStatus> &m, std::vector<NodeId> &p,
		std::vector<Timestamp> &d, std::vector<Timestamp> &f) { return; }); /* atEnd */

	std::reverse(sort.begin(), sort.end());
	return sort;
}

template <typename T, typename H>
template <typename F>
auto AdjList<T, H>::allTopoSortUtil(std::vector<T> &current, std::vector<bool> visited,
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
	auto &es = getElems();

	for (auto i = 0u; i < es.size(); i++) {
		/* If ith-event can be added */
		if (inDegree[i] == 0 && !visited[i]) {
			/* Reduce in-degrees of its neighbors */
			for (auto it = adj_begin(i), ei = adj_end(i); it != ei; ++it)
				--inDegree[*it];

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
			for (auto it = adj_begin(i), ei = adj_end(i); it != ei; ++it)
				++inDegree[*it];
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

template <typename T, typename H>
template <typename F>
auto AdjList<T, H>::allTopoSort(F &&prop) const -> bool
{
	std::vector<bool> visited(elems.size(), false);
	std::vector<T> current;
	auto inDegree(getInDegrees());
	auto found = false;

	return allTopoSortUtil(current, visited, inDegree, prop, found);
}

template <typename T, typename H>
template <typename F>
auto AdjList<T, H>::combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					   bool &found,
					   const std::vector<AdjList<T, H> *> &toCombine, F &&prop)
	-> bool
{
	/* If we have found a valid combination already, return */
	if (found)
		return true;

	/* Base case: a combination of sortings has been reached */
	VERIFY(index <= toCombine.size());
	if (index == toCombine.size()) {
		if (prop(current))
			found = true;
		return found;
	}

	/* Otherwise, we have more matrices to extend */
	return toCombine[index]->allTopoSort([&](std::vector<T> &sorting) {
		current.push_back(sorting);
		if (combineAllTopoSortUtil(index + 1, current, found, toCombine, prop))
			return true;
		current.pop_back();
		return false;
	});
}

template <typename T, typename H>
template <typename F>
auto AdjList<T, H>::combineAllTopoSort(const std::vector<AdjList<T, H> *> &toCombine, F &&prop)
	-> bool
{
	std::vector<std::vector<T>> current; /* The current sorting for each matrix */
	bool found = false;

	return combineAllTopoSortUtil(0, current, found, toCombine, prop);
}

template <typename T, typename H> void AdjList<T, H>::transClosure()
{
	if (calculatedTransC)
		return;

	dfs([&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m, std::vector<NodeId> &p,
		std::vector<Timestamp> &d, std::vector<Timestamp> &f) { return; }, /* atEntryV */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f) { return; }, /* atTreeE */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f) { return; }, /* atBackE*/
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f) { return; }, /* atForwE*/
	    [&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m, std::vector<NodeId> &p,
		std::vector<Timestamp> &d, std::vector<Timestamp> &f) {
		    for (auto &j : nodeSucc[i]) {
			    transC[i] |= transC[j];
			    transC[i].set(j);
		    }
	    }, /* atExitV*/
	    [&](Timestamp &t, std::vector<NodeStatus> &m, std::vector<NodeId> &p,
		std::vector<Timestamp> &d, std::vector<Timestamp> &f) { return; }); /* atEnd */

	calculatedTransC = true;
}

template <typename T, typename H> auto AdjList<T, H>::isIrreflexive() -> bool
{
	for (auto i = 0u; i < getElems().size(); i++)
		if (transC[i][i])
			return false;
	return true;
}

template <typename T, typename H>
auto operator<<(std::ostream &s, const AdjList<T, H> &l) -> std::ostream &
{
	auto &elems = l.getElems();

	s << "Elements: ";
	for (auto &e : elems)
		s << e << " ";
	s << "\n";

	for (auto i = 0U; i < elems.size(); i++) {
		s << elems[i] << " -> ";
		for (auto &j : l.nodeSucc[i])
			s << elems[j] << " ";
		s << "\n";
	}

	if (!l.calculatedTransC)
		return s;

	s << "Transitive closure:\n";
	for (auto i = 0U; i < elems.size(); i++) {
		s << elems[i] << " -> ";
		for (auto j = 0U; j < l.transC[i].size(); j++)
			if (l.transC[i][j])
				s << elems[j] << " ";
		s << "\n";
	}
	return s;
}

#endif /* GENMC_ADJ_LIST_HPP */
