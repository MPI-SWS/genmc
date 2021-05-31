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

#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <llvm/ADT/Hashing.h>
#include <llvm/Support/raw_ostream.h>

struct Event {
	int thread;
	int index;

	Event() : thread(-17), index(-17) {};
	Event(int t, int e) : thread(t), index(e) {};

	static Event getInitializer() { return Event(0, 0); };
	static Event getBottom() { return Event(-42, -42); };

	bool isInitializer() const { return *this == getInitializer(); };
	bool isBottom() const { return *this == getBottom(); };

	Event prev() const { return Event(thread, index-1); };
	Event next() const { return Event(thread, index+1); };

	bool isBetween(Event a, Event b) const {
		if (a.thread != b.thread)
			return false;
		if (this->thread != a.thread)
			return false;
		return this->index >= a.index && this->index <= b.index;
	}

	inline bool operator==(const Event &e) const {
		return e.index == index && e.thread == thread;
	}
	inline bool operator!=(const Event &e) const {
		return !(*this == e);
	}
	inline bool operator<(const Event &e) const {
		return (index < e.index) || (index == e.index && thread < e.thread);
	}
	inline bool operator>(const Event &e) const {
		return (index > e.index) || (index == e.index && thread > e.thread);
	}

	friend llvm::hash_code hash_value(const Event &e) {
		return llvm::hash_combine(e.thread, e.index);
	};
};

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, Event e);

struct EventHasher {

	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v)	const {
		seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}

	std::size_t operator()(const Event& e) const {
		std::size_t hash = 0;
		hash_combine(hash, e.thread);
		hash_combine(hash, e.index);
		return hash;
	}
};

#endif /* __EVENT_HPP__ */
