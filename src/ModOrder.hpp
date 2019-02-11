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
	typedef std::unordered_map<llvm::GenericValue *, std::vector<Event> > ModifOrder;
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
	llvm::GenericValue *getAddrAtPos(ModOrder::iterator it);
	std::vector<Event> getMoAfter(llvm::GenericValue *addr, const Event &e);
	void addAtLocEnd(llvm::GenericValue *addr, const Event &e);
	void addAtLocAfter(llvm::GenericValue *addr, const Event &pred, const Event &e);
	bool locContains(llvm::GenericValue *addr, const Event &e);
	bool areOrdered(llvm::GenericValue *addr, const Event &a, const Event &b);
	int getStoreOffset(llvm::GenericValue *addr, const Event &e);
	void changeStoreOffset(llvm::GenericValue *addr, const Event &e, int newOffset);

	inline std::vector<Event> &operator[](llvm::GenericValue *addr) { return mo_[addr]; };

	/* Overloaded operators */
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const ModOrder &rev);
};

#endif /* __REVISIT_SET_HPP__ */
