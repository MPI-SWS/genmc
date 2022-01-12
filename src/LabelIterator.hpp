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

#ifndef __LABEL_ITERATOR_HPP__
#define __LABEL_ITERATOR_HPP__

#include "config.h"
#include "ExecutionGraph.hpp"
#include <iterator>
#include <llvm/ADT/iterator_range.h>

/*******************************************************************************
 **                         LabelIterator Class
 ******************************************************************************/

/*
 * This class implements some helper iterators for ExecutionGraph.
 * A bit ugly, but easily tunable, and deals with UP containers
 */
template<typename ThreadT, typename ThreadItT, typename LabelT, typename LabelItT>
class LabelIterator {

protected:
	ThreadT *threads;
	ThreadItT thread;
	LabelItT label;

public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = LabelT;
	using difference_type = signed;
	using pointer = LabelT *;
	using reference = LabelT &;


	/*** Constructors/destructor ***/
	LabelIterator() = default;

	/* begin() constructor */
	template<typename G>
	LabelIterator(G &g) : threads(&g.getThreadList()), thread(g.begin()) {
		if (thread != threads->end()) {
			label = thread->begin();
			advanceThread();
		}
	}

	/* end() constructor -- dummy parameter */
	template<typename G>
	LabelIterator(G &g, bool) : threads(&g.getThreadList()), thread(g.end()) {}


	/*** Operators ***/
	inline pointer operator*() const { return &**label; }
	inline pointer operator->() const { return operator*(); }

	inline bool operator==(const LabelIterator &other) const {
		return thread == other.thread &&
		       (thread == threads->end() || label == other.label);
	}
	inline bool operator!=(const LabelIterator& other) const {
		return !operator==(other);
	}

	LabelIterator& operator++() {
		++label;
		advanceThread();
		return *this;
	}
	inline LabelIterator operator++(int) {
		auto tmp = *this; ++*this; return tmp;
	}

	LabelIterator& operator--() {
		while (thread == threads->end() || label == thread->begin()) {
			--thread;
			label = thread->end();
		}
		--label;
		return *this;
	}
	inline LabelIterator operator--(int) {
		auto tmp = *this; --*this; return tmp;
	}

private:
	/* Checks whether we have reached the end of a thread, and appropriately
	 * advances the thread and label iterators. Does nothing if that is not the case. */
	inline void advanceThread() {
		while (label == thread->end()) {
			++thread;
			if (thread == threads->end())
				break;
			label = thread->begin();
		}
	}

};

using label_iterator = LabelIterator<ExecutionGraph::ThreadList,
				     ExecutionGraph::iterator,
				     EventLabel,
				     ExecutionGraph::Thread::iterator>;
using const_label_iterator = LabelIterator<const ExecutionGraph::ThreadList,
					   ExecutionGraph::const_iterator,
					   const EventLabel,
					   ExecutionGraph::Thread::const_iterator>;

using label_range = llvm::iterator_range<label_iterator>;
using const_label_range = llvm::iterator_range<const_label_iterator>;

inline label_iterator label_begin(ExecutionGraph *G) { return label_iterator(*G); }
inline label_iterator label_begin(ExecutionGraph &G) { return label_iterator(G); }
inline const_label_iterator label_begin(const ExecutionGraph *G)
{
	return const_label_iterator(*G);
}
inline const_label_iterator label_begin(const ExecutionGraph &G)
{
	return const_label_iterator(G);
}

inline label_iterator label_end(ExecutionGraph *G) { return label_iterator(*G, true); }
inline label_iterator label_end(ExecutionGraph &G)   { return label_iterator(G, true); }
inline const_label_iterator label_end(const ExecutionGraph *G)
{
	return const_label_iterator(*G, true);
}
inline const_label_iterator label_end(const ExecutionGraph &G)
{
	return const_label_iterator(G, true);
}

inline label_range labels(ExecutionGraph *G) { return label_range(label_begin(G), label_end(G)); }
inline label_range labels(ExecutionGraph &G) { return label_range(label_begin(G), label_end(G)); }
inline const_label_range labels(const ExecutionGraph *G) {
	return const_label_range(label_begin(G), label_end(G));
}
inline const_label_range labels(const ExecutionGraph &G) {
	return const_label_range(label_begin(G), label_end(G));
}

#endif /* __LABEL_ITERATOR_HPP__ */
