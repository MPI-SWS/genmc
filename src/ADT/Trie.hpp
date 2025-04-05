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

#include "Support/Error.hpp"
#include <memory>
#include <vector>

/** A generic trie for random-access sequences of comparable elements.
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

		Node(Node &&) = delete;
		auto operator=(Node &&) -> Node & = delete;
		Node(const Node &) = delete;
		auto operator=(const Node &) -> Node & = delete;

		explicit Node(const Payload &data, const Seq &label = {}) : lab(label), dat(data) {}
		explicit Node(Payload &&data, Seq &&label = {})
			: lab(std::move(label)), dat(std::move(data))
		{}

		auto begin() -> iterator { return children.begin(); }
		auto begin() const -> const_iterator { return children.begin(); }
		auto end() -> iterator { return children.end(); }
		auto end() const -> const_iterator { return children.end(); }

		[[nodiscard]] auto size() const -> size_t { return children.size(); }
		[[nodiscard]] auto empty() const -> bool { return children.empty(); }
		auto front() const -> const Node *& { return children.front(); }
		auto front() -> Node *& { return children.front(); }
		auto back() const -> const Node *& { return children.back(); }
		auto back() -> Node *& { return children.back(); }

		auto data() const -> const Payload & { return dat; }
		void setData(Payload &&data) { dat = std::move(data); }

		auto label() const -> const Seq & { return lab; }

		auto getEdge(value_type id) -> Node *
		{
			Node *fNode = NULL;
			auto it = std::lower_bound(begin(), end(), id, NodeCmp());
			if (it != end() && (*it)->label()[0] == id)
				fNode = *it;
			return fNode;
		}

#if ENABLE_GENMC_DEBUG
		void dump()
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
			HaveCommonPart /* purposefully non-zero */
		};

		/** Node comparators */
		struct NodeCmp {
			auto operator()(const Node *n1, const Node *n2) -> bool
			{
				return ValCmp()(n1->label()[0], n2->label()[0]);
			}
			auto operator()(const Node *n, value_type id) -> bool
			{
				return ValCmp()(n->label()[0], id);
			}
		};

		void addEdge(Node *n)
		{
			if (this->empty())
				children.push_back(n);
			else {
				auto it = std::lower_bound(begin(), end(), n, NodeCmp());
				// FIXME: no dups are allowed
				children.insert(it, n);
			}
		}

		void setEdge(Node *n)
		{
			auto id = n->label()[0];
			auto it = std::lower_bound(begin(), end(), id, NodeCmp());
			BUG_ON(it == end() && "Node does not exists!");
			*it = n;
		}

		auto query(typename Seq::const_iterator sBeg,
			   typename Seq::const_iterator sEnd) const -> QueryResult
		{
			unsigned i, l;
			unsigned l1 = std::distance(sBeg, sEnd);
			unsigned l2 = label().size();

			/** Find the length of common part */
			l = std::min(l1, l2);
			i = 0;
			while ((i < l) && (*(sBeg + i) == label()[i]))
				++i;

			if (i == l) { // One is prefix of another, find who is who
				if (l1 == l2)
					return QueryResult::Same;
				if (i == l1)
					return QueryResult::StringIsPrefix;
				return QueryResult::LabelIsPrefix;
			}
			// s and Label have common (possible empty) part, return its length
			return static_cast<QueryResult>(i);
		}

		Seq lab;
		Payload dat;
		NodeVectorType children;
	};

	Trie()
	{
		addNode(Payload()); // FIXME
	}
	explicit Trie(const Payload &root) { addNode(root); }
	explicit Trie(Payload &&root) { addNode(root); }

	~Trie() = default;

	Trie(const Trie &) = delete;
	auto operator=(const Trie &) -> Trie & = delete;
	Trie(Trie &&) = default;
	auto operator=(Trie &&) -> Trie & = default;

	auto addSeq(const Seq &s, Payload &&data) -> bool
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

	auto findLongestCommonPrefix(const Seq &s) const -> unsigned
	{
		Node *cNode = getRoot();
		Node *tNode = nullptr;
		unsigned result = 0;

		if (s.empty())
			return result;

		auto sBeg = s.begin();
		auto sEnd = s.end();
		while (tNode == nullptr) {
			auto Id = *sBeg;
			if (auto *nNode = cNode->getEdge(Id)) {
				auto r = nNode->query(sBeg, sEnd);

				switch (r) {
				case Node::QueryResult::Same:
					tNode = nNode;
					++result;
					break;
				case Node::QueryResult::StringIsPrefix:
					return result + std::distance(sBeg, sEnd);
				case Node::QueryResult::DontMatch:
					BUG();
					return 0;
				case Node::QueryResult::LabelIsPrefix:
					sBeg = sBeg + nNode->label().size();
					cNode = nNode;
					result += nNode->label().size();
					break;
				default:
					return result + static_cast<unsigned>(r);
				}
			} else
				return result;
		}
		BUG_ON(result != s.size());
		return result;
	}

	auto lookup(const Seq &s) const -> const Payload *
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

	auto lookup(const Seq &s) -> Payload *
	{
		return const_cast<Payload *>(static_cast<const Trie &>(*this).lookup(s));
	}

	auto setData(const Seq &s, Payload &&data) -> bool
	{
		auto *load = lookup(s);
		if (!load)
			return false;
		*load = std::move(data);
		return true;
	}

#if ENABLE_GENMC_DEBUG
	void dump()
	{
		for (auto &n : nodes)
			n->dump();
	}
#endif

private:
	auto getRoot() const -> Node * { return &*nodes[0]; }

	auto addNode(Payload &&data, const Seq &label = {}) -> Node *
	{
		auto nUP = std::make_unique<Node>(std::move(data), Seq(label));
		nodes.push_back(std::move(nUP));
		return &*nodes.back();
	}

	auto splitEdge(Node *n, value_type id, size_t index) -> Node *
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
