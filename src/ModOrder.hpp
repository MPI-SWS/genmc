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

#ifndef __MOD_ORDER_HPP__
#define __MOD_ORDER_HPP__

#include "Event.hpp"
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>

/*
 * ModOrder class - This class represents the modification order
 */
class ModOrder {
protected:
	typedef std::unordered_map<const llvm::GenericValue *, std::vector<Event> > ModifOrder;
	ModifOrder mo_;

public:
	/* Constructors */
	ModOrder();

	/* Iterators */
	typedef ModifOrder::iterator iterator;
	typedef ModifOrder::const_iterator const_iterator;
	iterator begin();
	iterator end();
	const_iterator cbegin();
	const_iterator cend();

	/* Basic getter/setter methods  */
	const llvm::GenericValue *getAddrAtPos(ModOrder::iterator it);
	std::vector<Event> getMoAfter(const llvm::GenericValue *addr, Event e);
	void addAtLocEnd(const llvm::GenericValue *addr, Event e);
	void addAtLocAfter(const llvm::GenericValue *addr, Event pred, Event e);
	bool locContains(const llvm::GenericValue *addr, Event e);
	bool areOrdered(const llvm::GenericValue *addr, Event a, Event b);
	int getStoreOffset(const llvm::GenericValue *addr, Event e);
	void changeStoreOffset(const llvm::GenericValue *addr, Event e, int newOffset);

	unsigned int size() const { return mo_.size(); };

	inline std::vector<Event> &operator[](const llvm::GenericValue *addr) { return mo_[addr]; };

	/* Overloaded operators */
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ModOrder &rev);
};

#endif /* __REVISIT_SET_HPP__ */
