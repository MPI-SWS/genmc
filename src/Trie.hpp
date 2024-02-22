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

#ifndef GENMC_TRIE_HPP
#define GENMC_TRIE_HPP

#include "Error.hpp"
#include <memory>
#include <vector>

/*
 * This class defines a generic trie structure with the only
 * restriction being that trie keys need to be sequences that
 * support random access iterators, and that have comparable
 * elements.
 *
 * Inspired by the Trie class in LLVM 2.9.
 */

template <typename Seq, typename Payload, typename ValCmp = std::less<typename Seq::value_type>>
class Trie {

public:
	using value_type = typename Seq::value_type;

	class Node {
		friend class Trie;

	public:
		using NodeVectorType = std::vector<Node *>;
		using iterator = typename NodeVectorType::iterator;
		using const_iterator = typename NodeVectorType::const_iterator;

		Node(const Node &) = delete;
		Node &operator=(const Node &) = delete;

		inline explicit Node(const Payload &data, const Seq &label = {})
			: lab(label), dat(data)
		{}
		inline explicit Node(Payload &&data, Seq &&label = {})
			: lab(std::move(label)), dat(std::move(data))
		{}

		inline iterator begin() { return children.begin(); }
		inline const_iterator begin() const { return children.begin(); }
		inline iterator end() { return children.end(); }
		inline const_iterator end() const { return children.end(); }

		inline size_t size() const { return children.size(); }
		inline bool empty() const { return children.empty(); }
		inline const Node *&front() const { return children.front(); }
		inline Node *&front() { return children.front(); }
		inline const Node *&back() const { return children.back(); }
		inline Node *&back() { return children.back(); }

		inline const Payload &data() const { return dat; }
		inline void setData(Payload &&data) { dat = std::move(data); }

		inline const Seq &label() const { return lab; }

		inline Node *getEdge(value_type id)
		{
			Node *fNode = NULL;
			auto it = std::lower_bound(begin(), end(), id, NodeCmp());
			if (it != end() && (*it)->label()[0] == id)
				fNode = *it;
			return fNode;
		}

#if ENABLE_GENMC_DEBUG
		inline void dump()
		{
			llvm::dbgs() << "Node: " << this << "\n"
				     << "Label: " << format(label()) << "\n"
				     << "Children:\n";

			for (auto i = begin(), e = end(); i != e; ++i)
				llvm::dbgs() << *i << " -> " << format((*i)->label()) << "\n";
		}
#endif

	private:
		enum class QueryResult : std::int8_t {
			Same = -3,
			StringIsPrefix = -2,
			LabelIsPrefix = -1,
			DontMatch = 0,
			HaveCommonPart
		};

		/* Node comparators */
		struct NodeCmp {
			bool operator()(const Node *n1, const Node *n2)
			{
				return ValCmp()(n1->label()[0], n2->label()[0]);
			}
			bool operator()(const Node *n, value_type id)
			{
				return ValCmp()(n->label()[0], id);
			}
		};

		inline void addEdge(Node *n)
		{
			if (this->empty())
				children.push_back(n);
			else {
				auto it = std::lower_bound(begin(), end(), n, NodeCmp());
				// FIXME: no dups are allowed
				children.insert(it, n);
			}
		}

		inline void setEdge(Node *n)
		{
			auto id = n->label()[0];
			auto it = std::lower_bound(begin(), end(), id, NodeCmp());
			BUG_ON(it == end() && "Node does not exists!");
			*it = n;
		}

		QueryResult query(typename Seq::const_iterator sBeg,
				  typename Seq::const_iterator sEnd) const
		{
			unsigned i, l;
			unsigned l1 = std::distance(sBeg, sEnd);
			unsigned l2 = label().size();

			/* Find the length of common part */
			l = std::min(l1, l2);
			i = 0;
			while ((i < l) && (*(sBeg + i) == label()[i]))
				++i;

			if (i == l) { // One is prefix of another, find who is who
				if (l1 == l2)
					return QueryResult::Same;
				else if (i == l1)
					return QueryResult::StringIsPrefix;
				else
					return QueryResult::LabelIsPrefix;
			} else // s and Label have common (possible empty) part, return its length
				return static_cast<QueryResult>(i);
		}

		Seq lab;
		Payload dat;
		NodeVectorType children;
	};

	inline Trie()
	{
		addNode(Payload()); // FIXME
	}
	inline explicit Trie(const Payload &root) { addNode(root); }
	inline explicit Trie(Payload &&root) { addNode(root); }

	inline ~Trie() = default;

	Trie(const Trie &) = delete;
	Trie &operator=(const Trie &) = delete;
	Trie(Trie &&) = default;
	Trie &operator=(Trie &&) = default;

	bool addSeq(const Seq &s, Payload &&data)
	{
		Node *cNode = getRoot();
		Node *tNode = NULL;

		if (s.empty())
			return false;

		auto sBeg = s.begin();
		auto sEnd = s.end();
		while (tNode == NULL) {
			auto id = *sBeg;
			if (auto *nNode = cNode->getEdge(id)) {
				auto r = nNode->query(sBeg, sEnd);

				switch (r) {
				case Node::QueryResult::Same:
					return false;
				case Node::QueryResult::StringIsPrefix: {
					auto index = std::distance(sBeg, sEnd);
					nNode = splitEdge(cNode, id, index);
					nNode->setData(std::move(data));
					return true;
				}
				case Node::QueryResult::DontMatch:
					BUG();
					return false;
				case Node::QueryResult::LabelIsPrefix:
					sBeg = sBeg + nNode->label().size();
					cNode = nNode;
					break;
				default: {
					auto index = static_cast<
						std::underlying_type_t<typename Node::QueryResult>>(
						r);
					nNode = splitEdge(cNode, id, index);
					tNode = addNode(std::move(data), Seq(sBeg + index, sEnd));
					nNode->addEdge(tNode);
				}
				}
			} else {
				tNode = addNode(std::move(data), Seq(sBeg, sEnd));
				cNode->addEdge(tNode);
			}
		}
		return true;
	}

	const Payload *lookup(const Seq &s) const
	{
		Node *cNode = getRoot();
		Node *tNode = nullptr;

		if (s.empty())
			return &cNode->data();

		auto sBeg = s.begin();
		auto sEnd = s.end();
		while (tNode == nullptr) {
			auto Id = *sBeg;
			if (auto *nNode = cNode->getEdge(Id)) {
				auto r = nNode->query(sBeg, sEnd);

				switch (r) {
				case Node::QueryResult::Same:
					tNode = nNode;
					break;
				case Node::QueryResult::StringIsPrefix:
					return nullptr;
				case Node::QueryResult::DontMatch:
					BUG();
					return nullptr;
				case Node::QueryResult::LabelIsPrefix:
					sBeg = sBeg + nNode->label().size();
					cNode = nNode;
					break;
				default:
					return nullptr;
				}
			} else
				return nullptr;
		}
		return &tNode->data();
	}

	Payload *lookup(const Seq &s)
	{
		return const_cast<Payload *>(static_cast<const Trie &>(*this).lookup(s));
	}

	bool setData(const Seq &s, Payload &&data)
	{
		auto *load = lookup(s);
		if (!load)
			return false;
		*load = std::move(data);
		return true;
	}

#if ENABLE_GENMC_DEBUG
	inline void dump()
	{
		for (auto &n : nodes)
			n->dump();
	}
#endif

private:
	inline Node *getRoot() const { return &*nodes[0]; }

	inline Node *addNode(Payload &&data, const Seq &label = {})
	{
		auto nUP = std::make_unique<Node>(std::move(data), Seq(label));
		nodes.push_back(std::move(nUP));
		return &*nodes.back();
	}

	inline Node *splitEdge(Node *n, value_type id, size_t index)
	{
		auto *eNode = n->getEdge(id);
		BUG_ON(!eNode && "Node doesn't exist");

		auto &l = eNode->label();
		BUG_ON((index <= 0 || index >= l.size()) && "Trying to split too far!");
		auto l1 = Seq(l.begin(), l.begin() + index);
		auto l2 = Seq(l.begin() + index, l.end());

		auto *nNode = addNode(Payload(), l1);
		n->setEdge(nNode);

		eNode->lab = l2;
		nNode->addEdge(eNode);

		return nNode;
	}

	std::vector<std::unique_ptr<Node>> nodes;
};

#endif /* GENMC_TRIE_HPP */
