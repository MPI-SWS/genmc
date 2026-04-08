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

#ifndef GENMC_ILIST_HPP
#define GENMC_ILIST_HPP

#include "genmc/Support/Error.hpp"

#include <cstddef>
#include <iterator>

/** An intrusive list implementation inspired by LLVM's ilist design.
 *
 * Notes:
 *
 * - The API closely follows the STL list one, but some functions remain
 * unimplemented.
 * - clear() is constant time but does not unlink nodes from the list
 * - size() is O(N)
 * - The list does not use std::reverse_iterator, so no conversion
 * through forward_iterator is necesary.
 *
 * Usage:
 *
 * The list type T has to inherit from ilist_node<T>. If objects
 * of the same type need to participate in multiple lists, then
 * they also need to include a tag for each list they participate in
 * and the ilist needs to be defined with the same template parameters.
 * For instance:
 *
 * struct tagA {};
 * struct tagB {};
 * struct node : public ilist_node<node, tagA>, public ilist_node<node, tagB> {};
 * ilist<node, tagA> listA;
 * ilist<node, tabB> listB;
 */

namespace genmc {

template <typename T, typename Tag> class ilist;

template <typename T, typename Tag = void> class ilist_node {
private:
	template <typename U, typename UTag, bool IsConst, bool IsReverse>
	friend class ilist_iterator_base;
	template <typename U, typename UTag> friend class ilist;

	using node_base_type = T;
	using list_base_type = ilist<T, Tag>;

public:
	ilist_node() = default;

	[[nodiscard]] auto is_linked() const noexcept -> bool { return next_ != nullptr; }

private:
	ilist_node *next_{};
	ilist_node *prev_{};
};

template <typename T, typename Tag> struct ilist_traits {
	static auto node(T &obj) -> ilist_node<T, Tag> &
	{
		return static_cast<ilist_node<T, Tag> &>(obj);
	}
	static auto node(const T &obj) -> const ilist_node<T, Tag> &
	{
		return static_cast<const ilist_node<T, Tag> &>(obj);
	}

	static auto value(ilist_node<T, Tag> *node) -> T * { return static_cast<T *>(node); }
	static auto value(const ilist_node<T, Tag> *node) -> const T *
	{
		return static_cast<const T *>(node);
	}
};

template <typename T, typename Tag, bool IsConst, bool IsReverse> class ilist_iterator_base {
public:
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = T;
	using reference = typename std::conditional_t<IsConst, const T &, T &>;
	using pointer = typename std::conditional_t<IsConst, const T *, T *>;

private:
	using traits_type = ilist_traits<T, Tag>;
	using node_type = ilist_node<T, Tag>;
	using node_pointer = typename std::conditional_t<IsConst, const node_type *, node_type *>;
	using node_reference = typename std::conditional_t<IsConst, const node_type &, node_type &>;

public:
	ilist_iterator_base() = default;
	ilist_iterator_base(node_reference node) : nodePtr_(&node) {}
	ilist_iterator_base(node_pointer node) : nodePtr_(node) {}

	auto operator->() const -> pointer
	{
		ASSERT(nodePtr_->node_type::next_ != nodePtr_ ||
		       nodePtr_->node_type::prev_ != nodePtr_); /* sentinel */
		return traits_type::value(nodePtr_);
	}

	auto operator*() const -> reference
	{
		ASSERT(nodePtr_);
		return *operator->();
	}

	auto operator++() -> ilist_iterator_base &
	{
		ASSERT(nodePtr_);
		nodePtr_ = IsReverse ? nodePtr_->node_type::prev_ : nodePtr_->node_type::next_;
		return *this;
	}

	auto operator++(int) -> ilist_iterator_base
	{
		auto tmp = *this;
		operator++();
		return tmp;
	}

	auto operator--() -> ilist_iterator_base &
	{
		ASSERT(nodePtr_);
		nodePtr_ = IsReverse ? nodePtr_->node_type::next_ : nodePtr_->node_type::prev_;
		return *this;
	}

	auto operator--(int) -> ilist_iterator_base
	{
		auto tmp = *this;
		operator--();
		return tmp;
	}

	auto node() const -> node_pointer { return nodePtr_; }

	friend auto operator==(const ilist_iterator_base &lhs, const ilist_iterator_base &rhs)
		-> bool
	{
		return lhs.nodePtr_ == rhs.nodePtr_;
	}

	friend auto operator!=(const ilist_iterator_base &lhs, const ilist_iterator_base &rhs)
		-> bool
	{
		return !operator==(lhs, rhs);
	}

private:
	node_pointer nodePtr_{};
};

template <typename T, typename Tag = void> class ilist {
private:
	using node_type = ilist_node<T, Tag>;
	using traits_type = ilist_traits<T, Tag>;

public:
	using value_type = T;
	using reference = value_type &;
	using const_reference = const value_type &;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using iterator = ilist_iterator_base<T, Tag, false, false>;
	using const_iterator = ilist_iterator_base<T, Tag, true, false>;
	using reverse_iterator = ilist_iterator_base<T, Tag, false, true>;
	using const_reverse_iterator = ilist_iterator_base<T, Tag, true, true>;

	/** Ctors/dtors */
	ilist() { sentinel_.prev_ = sentinel_.next_ = &sentinel_; }
	ilist(const ilist &other) = delete;
	ilist(ilist &&other) noexcept : ilist() { splice(std::move(other)); }
	auto operator=(const ilist &) -> ilist & = delete;
	auto operator=(ilist &&other) noexcept -> ilist &
	{
		clear();
		splice(std::move(other));
		return *this;
	}
	~ilist() = default;

	/** Iterators */
	auto begin() noexcept -> iterator { return ++iterator(sentinel_); }
	auto begin() const noexcept -> const_iterator { return ++const_iterator(sentinel_); }
	auto end() noexcept -> iterator { return iterator(sentinel_); }
	auto end() const noexcept -> const_iterator { return const_iterator(sentinel_); }

	auto rbegin() noexcept -> reverse_iterator { return ++reverse_iterator(sentinel_); }
	auto rbegin() const noexcept -> const_reverse_iterator
	{
		return ++const_reverse_iterator(sentinel_);
	}
	auto rend() noexcept -> reverse_iterator { return reverse_iterator(sentinel_); }
	auto rend() const noexcept -> const_reverse_iterator
	{
		return const_reverse_iterator(sentinel_);
	}

	/** Capacity */
	[[nodiscard]] auto empty() const noexcept -> bool { return sentinel_.prev_ == &sentinel_; }
	[[nodiscard]] auto size() const noexcept -> size_type
	{
		return std::distance(begin(), end());
	}

	auto front() noexcept -> reference { return *begin(); }
	auto front() const noexcept -> const_reference { return *begin(); }
	auto back() noexcept -> reference { return *rbegin(); }
	auto back() const noexcept -> const_reference { return *rbegin(); }

	/** Modifiers */
	auto insert(iterator pos, reference node) -> iterator;
	void remove(reference node);
	auto erase(iterator pos) -> iterator;
	void clear() noexcept;

	void push_front(reference node) { insert(begin(), node); }
	void push_back(reference node) { insert(end(), node); }
	void pop_front() { erase(begin()); }
	void pop_back() { erase(--end()); }

private:
	void splice(ilist &&other)
	{
		/* Pre: empty lists have properly initialized sentinels (e.g., move ops) */
		if (other.empty())
			return;

		auto *first = other.sentinel_.next_;
		auto *last = other.sentinel_.prev_;

		first->prev_ = &sentinel_;
		last->next_ = &sentinel_;

		sentinel_.next_ = first;
		sentinel_.prev_ = last;

		other.sentinel_.next_ = other.sentinel_.prev_ = &other.sentinel_;
	}

	/* Acts as end() for both forward and reverse case */
	ilist_node<T, Tag> sentinel_;
};

template <typename T, typename Tag>
inline auto ilist<T, Tag>::insert(iterator pos, reference node) -> iterator
{
	auto &new_node = traits_type::node(node);
	auto &next = *pos.node();
	auto &prev = *next.prev_;
	// ASSERT(!new_node.next_ && !new_node.prev_); /* should be unlinked */

	new_node.next_ = &next;
	new_node.prev_ = &prev;
	next.prev_ = &new_node;
	prev.next_ = &new_node;
	return {new_node};
}

template <typename T, typename Tag> inline void ilist<T, Tag>::remove(reference node)
{
	auto *old_node = &traits_type::node(node);
	old_node->prev_->next_ = old_node->next_;
	old_node->next_->prev_ = old_node->prev_;
	old_node->prev_ = old_node->next_ = nullptr;
}

template <typename T, typename Tag> inline auto ilist<T, Tag>::erase(iterator pos) -> iterator
{
	remove(*pos++);
	return pos;
}

template <typename T, typename Tag> inline void ilist<T, Tag>::clear() noexcept
{
	sentinel_.next_ = sentinel_.prev_ = &sentinel_;
}

}; /* namespace genmc */

#endif /* GENMC_ILIST_HPP */
