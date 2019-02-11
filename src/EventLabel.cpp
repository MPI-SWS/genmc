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

#include "Event.hpp"
#include "EventLabel.hpp"
#include "Error.hpp"
#include <llvm/IR/Instructions.h>

#include <cassert>

/* Start */
EventLabel::EventLabel(EventType typ, llvm::AtomicOrdering ord, Event e, Event tc)
	: type(typ), ord(ord), pos(e), rf(tc) {}

/* Thread Create/Join */
EventLabel::EventLabel(EventType typ, llvm::AtomicOrdering ord, Event e, int cid)
	: type(typ), ord(ord), pos(e), cid(cid) {}

/* Fence */
EventLabel::EventLabel(EventType typ, llvm::AtomicOrdering ord, Event e)
	: type(typ), ord(ord), pos(e) {}

/* Malloc/Free */
EventLabel::EventLabel(EventType typ, Event e, llvm::GenericValue *addr,
		       llvm::GenericValue val)
	: type(typ), pos(e), addr(addr), val(val) {}

/* Plain Read */
EventLabel::EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		       llvm::GenericValue *addr, llvm::Type *valTyp, Event rf,
		       llvm::GenericValue expected, llvm::GenericValue nextVal,
		       llvm::AtomicRMWInst::BinOp op)
	: type(typ), attr(attr), ord(ord), pos(e), addr(addr), val(expected),
	  nextVal(nextVal), op(op), valTyp(valTyp), rf(rf) {}

/* Lib Read */
EventLabel::EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		       llvm::GenericValue *addr, llvm::Type *valTyp, Event rf,
		       std::string &functionName)
	: type(typ), attr(attr), ord(ord), pos(e), addr(addr), valTyp(valTyp), rf(rf),
	  functionName(functionName), initial(false) {}

/* Store */
EventLabel::EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		       llvm::GenericValue *addr, llvm::Type *valTyp, llvm::GenericValue val)
	: type(typ), attr(attr), ord(ord), pos(e), addr(addr), val(val), valTyp(valTyp) {}

/* GStore */
EventLabel::EventLabel(EventType typ, EventAttr attr, llvm::AtomicOrdering ord, Event e,
		       llvm::GenericValue *addr, llvm::Type *valTyp, llvm::GenericValue val,
		       std::string &functionName, bool isInit)
	: type(typ), attr(attr), ord(ord), pos(e), addr(addr), val(val), valTyp(valTyp),
	  functionName(functionName), initial(isInit) {}


unsigned int EventLabel::getStamp() const
{
	return stamp;
}

const Event& EventLabel::getPos() const
{
	return pos;
}

int EventLabel::getIndex() const
{
	return pos.index;
}

int EventLabel::getThread() const
{
	return pos.thread;
}

EventAttr EventLabel::getAttr() const
{
	return attr;
}

llvm::GenericValue *EventLabel::getAddr() const
{
	return addr;
}

View& EventLabel::getHbView()
{
	return hbView;
}

View& EventLabel::getPorfView()
{
	return porfView;
}

View& EventLabel::getMsgView()
{
	return msgView;
}

bool EventLabel::isStart() const
{
	return type == EStart;
}

bool EventLabel::isFinish() const
{
	return type == EFinish;
}

bool EventLabel::isCreate() const
{
	return type == ETCreate;
}

bool EventLabel::isJoin() const
{
	return type == ETJoin;
}

bool EventLabel::isRead() const
{
	return type == ERead;
}

bool EventLabel::isWrite() const
{
	return type == EWrite;
}

bool EventLabel::isFence() const
{
	return type == EFence;
}

bool EventLabel::isMalloc() const
{
	return type == EMalloc;
}

bool EventLabel::isFree() const
{
	return type == EFree;
}

bool EventLabel::isNotAtomic() const
{
	return ord == llvm::AtomicOrdering::NotAtomic;
}

bool EventLabel::isAtLeastAcquire() const
{
	return (ord == llvm::AtomicOrdering::Acquire ||
		ord == llvm::AtomicOrdering::AcquireRelease ||
		ord == llvm::AtomicOrdering::SequentiallyConsistent);
	// return llvm::isAtLeastAcquire(ord);
}

bool EventLabel::isAtLeastRelease() const
{
	return (ord == llvm::AtomicOrdering::Release ||
		ord == llvm::AtomicOrdering::AcquireRelease ||
		ord == llvm::AtomicOrdering::SequentiallyConsistent);
	// return llvm::isAtLeastRelease(ord);
}

bool EventLabel::hasReadSem() const
{
	return type == ERead || type == EStart || type == ETJoin;
}

bool EventLabel::hasWriteSem() const
{
	return type == EWrite || type == ETCreate || type == EFinish;
}

bool EventLabel::isSC() const
{
	return (ord == llvm::AtomicOrdering::SequentiallyConsistent);
}

bool EventLabel::isRMW() const
{
	return attr != ATTR_PLAIN && attr != ATTR_UNLOCK ;
}

bool EventLabel::isFAI() const
{
	return attr == ATTR_FAI;
}

bool EventLabel::isCAS() const
{
	return attr == ATTR_CAS || attr == ATTR_LOCK ;
}

bool EventLabel::isLock() const
{
	return attr == ATTR_LOCK;
}

bool EventLabel::isUnlock() const
{
	return attr == ATTR_UNLOCK;
}

bool EventLabel::isLibInit() const
{
	return initial;
}

bool EventLabel::isRevisitable() const
{
	return revisitable;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const llvm::AtomicOrdering &o)
{
	switch (o) {
	case llvm::AtomicOrdering::NotAtomic : return s << "NA";
	case llvm::AtomicOrdering::Unordered : return s << "Un";
	case llvm::AtomicOrdering::Monotonic : return s << "Rlx";
	case llvm::AtomicOrdering::Acquire   : return s << "Acq";
	case llvm::AtomicOrdering::Release   : return s << "Rel";
	case llvm::AtomicOrdering::AcquireRelease : return s << "AcqRel";
	case llvm::AtomicOrdering::SequentiallyConsistent : return s << "SeqCst";
	default : return s;
	}
}

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const EventLabel &lab)
{
	return s << "EventLabel (" << lab.pos << ", " << lab.type
		 << "/" << lab.ord << (lab.isRMW() ? ", " : "")
		 << lab.attr << ", HB: " << lab.hbView
		 << ", SBRF: " << lab.porfView << ")";
}
