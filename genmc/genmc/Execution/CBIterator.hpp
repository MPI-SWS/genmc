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

#ifndef GENMC_PORF_ITERATOR_HPP
#define GENMC_PORF_ITERATOR_HPP

#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Support/Error.hpp"

#include <vector>

/**
 * A helper class for iterating over the causal predecessors of an
 * event.
 *
 *   cb \defeq (po U rf U tc U tj)+
 */
class CBIterator {
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = const EventLabel;
	using difference_type = std::ptrdiff_t;
	using pointer = const EventLabel *;
	using reference = const EventLabel &;

	/* end sentinel and begin iterator */
	CBIterator() = default;
	explicit CBIterator(const EventLabel *lab) : g_(lab->getParent())
	{
		ASSERT(lab);
		stack_.emplace_back(lab->getPos(), 0, false);
		a_.setMax(lab->getPos());
		advance();
	}

	auto operator*() const -> reference { return *current_; }
	auto operator++() -> CBIterator &
	{
		advance();
		return *this;
	}
	auto operator++(int) -> CBIterator
	{
		auto tmp = *this;
		advance();
		return tmp;
	}

	/* Sentinel comparison: end if stack is empty */
	friend auto operator==(const CBIterator &lhs, const CBIterator &rhs) -> bool
	{
		return lhs.stack_.empty() && rhs.stack_.empty();
	}

private:
	struct Frame {
		Event end;	  /* inclusive last index for this recursion */
		int i{};	  /* next index to process in this thread */
		bool descended{}; /* whether pred of i has been pushed */
	};

	/* computes next current_, pops empty frames */
	void advance();

	const ExecutionGraph *g_ = nullptr;
	View a_;
	std::vector<Frame> stack_;
	const EventLabel *current_ = nullptr;
};

#endif /* GENMC_PORF_ITERATOR_HPP */
