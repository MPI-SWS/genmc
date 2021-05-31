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

#include "Error.hpp"

template<typename T, typename H>
void AdjList<T, H>::addNode(T a)
{
	auto id = elems.size();

	ids[a] = id;
	elems.push_back(a);
	nodeSucc.push_back({});
	inDegree.push_back(0);

	calculatedTransC = false;
	transC.push_back(llvm::BitVector(id));
	return;
}

template<typename T, typename H>
void AdjList<T, H>::addEdge(NodeId a, NodeId b)
{
	/* If edge already exists, nothing to do */
	if ((*this)(a, b))
		return;

	nodeSucc[a].push_back(b);
	transC[a].set(b);
	++inDegree[b];
	calculatedTransC = false;
	return;
}

template<typename T, typename H>
void AdjList<T, H>::addEdge(T a, T b)
{
	addEdge(getIndex(a), getIndex(b));
	return;
}

template<typename T, typename H>
void AdjList<T, H>::addEdgesFromTo(const std::vector<T> &froms, const std::vector<T> &tos)
{
	for (auto &f : froms)
		for (auto &t : tos)
			addEdge(f, t);
}

template<typename T, typename H>
const std::vector<int> &AdjList<T, H>::getInDegrees() const
{
	return inDegree;
}

template<typename T, typename H>
template<typename FVB, typename FET, typename FEB,
	 typename FEF, typename FVE>
void AdjList<T, H>::dfsUtil(NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
			    std::vector<NodeId> &p, std::vector<Timestamp> &d,
			    std::vector<Timestamp> &f, FVB&& atEntryV, FET&& atTreeE,
			    FEB&& atBackE, FEF&& atForwE, FVE&& atExitV) const
{
	m[i] = NodeStatus::entered;
	d[i] = ++t;
	atEntryV(i, t, m, p, d, f);
	for (auto &j : nodeSucc[i]) {
		if (m[j] == NodeStatus::unseen) {
			p[j] = i;
			dfsUtil(j, t, m, p, d, f, atEntryV, atTreeE,
				atBackE, atForwE, atExitV);
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
	return;
}

template<typename T, typename H>
template<typename FVB, typename FET, typename FEB,
	 typename FEF, typename FVE, typename FEND>
void AdjList<T, H>::dfs(FVB&& atEntryV, FET&& atTreeE, FEB&& atBackE,
			FEF&& atForwE, FVE&& atExitV, FEND&& atEnd) const
{
	Timestamp t = 0;
	std::vector<NodeStatus> m(nodeSucc.size(), NodeStatus::unseen); /* Node status */
	std::vector<NodeId> p(nodeSucc.size(), -42);                    /* Node parent */
	std::vector<Timestamp> d(nodeSucc.size(), 0);                   /* First visit */
	std::vector<Timestamp> f(nodeSucc.size(), 0);                   /* Last visit */

	for (auto i = 0u; i < nodeSucc.size(); i++) {
		if (m[i] == NodeStatus::unseen)
			dfsUtil(i, t, m, p, d, f, atEntryV, atTreeE,
				atBackE, atForwE, atExitV);
	}
	atEnd(t, m, p, d, f);
	return;
}

template<typename T, typename H>
template<typename FVB, typename FET, typename FEB,
	 typename FEF, typename FVE, typename FEND>
void AdjList<T, H>::visitReachable(T a, FVB&& atEntryV, FET&& atTreeE, FEB&& atBackE,
				   FEF&& atForwE, FVE&& atExitV, FEND&& atEnd) const
{
	Timestamp t = 0;
	std::vector<NodeStatus> m(nodeSucc.size(), NodeStatus::unseen); /* Node status */
	std::vector<NodeId> p(nodeSucc.size(), -42);                    /* Node parent */
	std::vector<Timestamp> d(nodeSucc.size(), 0);                   /* First visit */
	std::vector<Timestamp> f(nodeSucc.size(), 0);                   /* Last visit */

	dfsUtil(getIndex(a), t, m, p, d, f, atEntryV, atTreeE, atBackE, atForwE, atExitV);
	atEnd(t, m, p, d, f);
	return;
}

template<typename T, typename H>
std::vector<T> AdjList<T, H>::topoSort()
{
	std::vector<T> sort;

	dfs([&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }, /* atEntryV */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }, /* atTreeE */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ BUG(); }, /* atBackE */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }, /* atForwE*/
	    [&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ /* atExitV */
		    sort.push_back(elems[i]);
		    return;
	    },
	    [&](Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }); /* atEnd */

	std::reverse(sort.begin(), sort.end());
	return sort;
}

template<typename T, typename H>
template<typename F>
bool AdjList<T, H>::allTopoSortUtil(std::vector<T> &current,
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

template<typename T, typename H>
template<typename F>
bool AdjList<T, H>::allTopoSort(F&& prop) const
{
	std::vector<bool> visited(elems.size(), false);
	std::vector<T> current;
	auto inDegree(getInDegrees());
	auto found = false;

	return allTopoSortUtil(current, visited, inDegree, prop, found);
}

template<typename T, typename H>
template<typename F>
bool AdjList<T, H>::combineAllTopoSortUtil(unsigned int index, std::vector<std::vector<T>> &current,
					   bool &found, const std::vector<AdjList<T, H> *> &toCombine,
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
	return toCombine[index]->allTopoSort([&](std::vector<T> &sorting){
			current.push_back(sorting);
			if (combineAllTopoSortUtil(index + 1, current, found, toCombine, prop))
				return true;
			current.pop_back();
			return false;
		});
}

template<typename T, typename H>
template<typename F>
bool AdjList<T, H>::combineAllTopoSort(const std::vector<AdjList<T, H> *> &toCombine, F&& prop)
{
	std::vector<std::vector<T>> current; /* The current sorting for each matrix */
	bool found = false;

	return combineAllTopoSortUtil(0, current, found, toCombine, prop);
}

template<typename T, typename H>
void AdjList<T, H>::transClosure()
{
	if (calculatedTransC)
		return;

	dfs([&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }, /* atEntryV */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }, /* atTreeE */
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }, /* atBackE*/
	    [&](NodeId i, NodeId j, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }, /* atForwE*/
	    [&](NodeId i, Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){
		    for (auto &j : nodeSucc[i]) {
			    transC[i] |= transC[j];
			    transC[i].set(j);
		    }
	    }, /* atExitV*/
	    [&](Timestamp &t, std::vector<NodeStatus> &m,
		std::vector<NodeId> &p, std::vector<Timestamp> &d,
		std::vector<Timestamp> &f){ return; }); /* atEnd */

	calculatedTransC = true;
	return;
}

template<typename T, typename H>
bool AdjList<T, H>::isIrreflexive()
{
	for (auto i = 0u; i < getElems().size(); i++)
		if (transC[i][i])
			return false;
	return true;
}

template<typename T, typename H>
llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const AdjList<T, H> &l)
{
	auto &elems = l.getElems();

	s << "Elements: ";
	for (auto &e : elems)
		s << e << " ";
	s << "\n";

	for (auto i = 0u; i < elems.size(); i++) {
		s << elems[i] << " -> ";
		for (auto &j : l.nodeSucc[i])
			s << elems[j] << " ";
		s << "\n";
	}

	if (!l.calculatedTransC)
		return s;

	s << "Transitive closure:\n";
	for (auto i = 0u; i < elems.size(); i++) {
		s << elems[i] << " -> ";
		for (auto j = 0u; j < l.transC[i].size(); j++)
			if (l.transC[i][j])
				s << elems[j] << " ";
		s << "\n";
	}
	return s;
}
