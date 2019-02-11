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

#ifndef __EVENTLABEL_HPP__
#define __EVENTLABEL_HPP__

#include "Event.hpp"
#include "RevisitSet.hpp"
#include "View.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>

#include <list>

class EventLabel {

public:
	EventType type;
	unsigned int stamp = 0;
	EventAttr attr = ATTR_PLAIN;
	llvm::AtomicOrdering ord = llvm::AtomicOrdering::NotAtomic;
	Event pos;
	llvm::GenericValue *addr;
	llvm::GenericValue val; /* For Writes and CASs */
	llvm::GenericValue nextVal; /* For CASs and FAIs */
	llvm::AtomicRMWInst::BinOp op;
	llvm::Type *valTyp;
	Event rf; /* For Reads */
	std::vector<Event> rfm1; /* For Writes */
	View msgView;
	View hbView;
	View porfView;
	int cid; /* For TCreates */
	std::string functionName; /* For GReads/GWrites */
	bool initial; /* For GWrites */
	bool revisitable;
	RevisitSet revs;
	std::vector<Event> invalidRfs;

	EventLabel(EventType typ, llvm::AtomicOrdering ord, Event e, Event tc); /* Start */
	EventLabel(EventType typ, llvm::AtomicOrdering ord, Event e, int cid); /* Thread Create */
	EventLabel(EventType typ, llvm::AtomicOrdering ord, Event e); /* Fence */
	EventLabel(EventType typ, Event e, llvm::GenericValue *addr,
		   llvm::GenericValue val); /* Malloc/Free */
	EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		   llvm::GenericValue *addr, llvm::Type *valTyp, Event rf,
		   llvm::GenericValue expected, llvm::GenericValue nextVal,
		   llvm::AtomicRMWInst::BinOp op); /* Plain Read */
	EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		   llvm::GenericValue *addr, llvm::Type *valTyp, Event rf,
		   std::string &functionName); /* Lib Read */

	EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		   llvm::GenericValue *addr, llvm::Type *valTyp, llvm::GenericValue val); /* Writes */
	EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		   llvm::GenericValue *addr, llvm::Type *valTyp, llvm::GenericValue val,
		    std::string &functionName, bool isInit); /* Lib Writes */

	unsigned int getStamp() const;
	const Event &getPos() const;
	int getIndex() const;
	int getThread() const;
	EventAttr getAttr() const;
	llvm::GenericValue *getAddr() const;
	View& getHbView();
	View& getPorfView();
	View& getMsgView();

	bool isStart() const;
	bool isFinish() const;
	bool isCreate() const;
	bool isJoin() const;
	bool isRead() const;
	bool isWrite() const;
	bool isFence() const;
	bool isMalloc() const;
	bool isFree() const;
	bool isNotAtomic() const;
	bool isAtLeastAcquire() const;
	bool isAtLeastRelease() const;
	bool isSC() const;
	bool isRMW() const;
	bool isFAI() const;
	bool isCAS() const;
	bool isLock() const;
	bool isUnlock() const;
	bool isLibInit() const;
	bool isRevisitable() const;
	bool hasReadSem() const;
	bool hasWriteSem() const;

	void makeNotRevisitable() { revisitable = false; };
	void makeRevisitable()    { revisitable = true; };

	inline bool operator==(const EventLabel &lab) const {
		return type == lab.type && pos == lab.pos && (!hasReadSem() || rf == lab.rf);
	}
	inline bool operator!=(const EventLabel &lab) const { return !(*this == lab); };

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const EventLabel &lab);
};

#endif /* #define __EVENTLABEL_HPP__ */
