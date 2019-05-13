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
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

class ExecutionGraph;

/*******************************************************************************
 **                        EventLabel Class (Abstract)
 ******************************************************************************/

/*
 * An abstract class for modeling event labels. Contains the bare minimum
 * that all different labels (e.g., Reads, Writes, etc) should have. Although
 * hb and (po U rf) are stored as vector clocks in EventLabels, the respective
 * getter methods are private. One can obtain information about such relations
 * by querying the execution graph.
 */
class EventLabel {

public:
	/* Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * It is public to allow clients perform a switch() on it */
	enum EventLabelKind {
		EL_ThreadStart,
		EL_ThreadFinish,
		EL_ThreadCreate,
		EL_ThreadJoin,
		EL_MemAccessBegin,
		EL_Read,
		EL_FaiRead,
		EL_CasRead,
		EL_LibRead,
		EL_LastRead,
		EL_Write,
		EL_FaiWrite,
		EL_CasWrite,
		EL_LibWrite,
		EL_LastWrite,
		EL_MemAccessEnd,
		EL_Fence,
		EL_Malloc,
		EL_Free
	};

protected:
	/* ExecutionGraph needs to be a friend to call the constructors */
	friend class ExecutionGraph;

	EventLabel(EventLabelKind k, unsigned int s, llvm::AtomicOrdering o,
		   Event p) : kind(k), stamp(s), ordering(o), position(p) {}

public:

	/* Returns the descriminator of this object */
	EventLabelKind getKind() const { return kind; }

	/* The stamp this label has in the execution graph*/
	unsigned int getStamp() const { return stamp; }

	/* Returns the ordering of this label */
	llvm::AtomicOrdering getOrdering() const { return ordering; }

	/* Returns the position in the execution graph (thread, index) */
	Event getPos() const { return position; }

	/* Returns the index of this label within a thread */
	int getIndex() const { return position.index; }

	/* Returns the thread of this label in the execution graph */
	int getThread() const { return position.thread; }

	/* Returns true if this label corresponds to a non-atomic access */
	bool isNotAtomic() const {
		return ordering == llvm::AtomicOrdering::NotAtomic;
	}

	/* Returns true if the ordering of this access is acquire or stronger */
	bool isAtLeastAcquire() const {
		return ordering == llvm::AtomicOrdering::Acquire ||
		       ordering == llvm::AtomicOrdering::AcquireRelease ||
		       ordering == llvm::AtomicOrdering::SequentiallyConsistent;
	}

	/* Returns true if the ordering of this access is release or stronger */
	bool isAtLeastRelease() const {
		return ordering == llvm::AtomicOrdering::Release ||
		       ordering == llvm::AtomicOrdering::AcquireRelease ||
		       ordering == llvm::AtomicOrdering::SequentiallyConsistent;
	}

	/* Returns true if this is a sequentially consistent access */
	bool isSC() const {
		return ordering == llvm::AtomicOrdering::SequentiallyConsistent;
	}

	virtual ~EventLabel() {}

	/* Returns a clone object (virtual to allow deep copying from base) */
	virtual EventLabel *clone() const = 0;

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const EventLabel &lab);

private:
	/* Methods that get/set the vector clocks for this label.
	 * These are to be used only from the execution graph */
	const View& getHbView() const { return hbView; }
	const View& getPorfView() const { return porfView; }

	void setHbView(View &&v) { hbView = std::move(v); }
	void setPorfView(View &&v) { porfView = std::move(v); }

	/* Discriminator enum for LLVM-style RTTI */
	const EventLabelKind kind;

	/* The stamp of this label in the execution graph */
	const unsigned int stamp;

	/* Ordering of this access mode */
	const llvm::AtomicOrdering ordering;

	/* Position of this label within the execution graph (thread, index) */
	const Event position;

	/* Events that are hb-before this label */
	View hbView;

	/* Events that are (po U rf)-before this label */
	View porfView;
};


/*******************************************************************************
 **                       MemAccessLabel Class (Abstract)
 ******************************************************************************/

/* This label abstracts the common functionality that loads and stores have
 * (e.g., asking for the address of such a label) */
class MemAccessLabel : public EventLabel {

protected:
	using EventLabel::EventLabel;

public:

	static bool classof(const EventLabel *lab) {
		return lab->getKind() >= EL_MemAccessBegin &&
		       lab->getKind() <= EL_MemAccessEnd;
	}

	virtual const llvm::GenericValue *getAddr() const = 0;
	virtual const llvm::Type *getType() const = 0;
};


/*******************************************************************************
 **                         ReadLabel Class
 ******************************************************************************/

/* The label for reads. All special read types (e.g., FAI, CAS) should inherit
 * from this class */
class ReadLabel : public MemAccessLabel {

protected:
	friend class ExecutionGraph;

	ReadLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		  Event pos, const llvm::GenericValue *loc,
		  const llvm::Type *typ, Event rf)
		: MemAccessLabel(k, st, ord, pos), addr(loc),
		  valueType(typ), readsFrom(rf), revisitable(true) {}

	ReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		  const llvm::GenericValue *loc, const llvm::Type *typ,
		  Event rf)
		: MemAccessLabel(EL_Read, st, ord, pos), addr(loc),
		  valueType(typ), readsFrom(rf), revisitable(true) {}

public:
	/* Returns the position of the write this read is readinf-from */
	Event getRf() const { return readsFrom; }

	/* Returns true if this read can be revisited */
	bool isRevisitable() const { return revisitable; }

	/* Returns the address this read is accessing */
	const llvm::GenericValue *getAddr() const override { return addr; }

	/* Returns the type of the value this read is reading */
	const llvm::Type *getType() const override { return valueType; }

	ReadLabel *clone() const override { return new ReadLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() >= EL_Read &&
		       lab->getKind() <= EL_LastRead;
	}

private:
	/* Changes the reads-from edge for this label. This should only
	 * be called from the execution graph to update other relevant
	 * information as well */
	void setRf(Event rf) { readsFrom = rf; }

	/* Makes the relevant read revisitable/non-revisitable. The
	 * execution graph is responsible for making such changes */
	void setRevisitStatus(bool status) { revisitable = status; }

	/* The address the read is accessing */
	const llvm::GenericValue *addr;

	/* The type of the value the read is accessing */
	const llvm::Type *valueType;

	/* Position of the write it is reading from in the graph */
	Event readsFrom;

	/* Revisitability status */
	bool revisitable;

	/* Information about writes that have revisited (or will revisit)
	 * this read */
	RevisitSet revs;
};


/*******************************************************************************
 **                         FaiReadLabel Class
 ******************************************************************************/

/* Represents the read part of a read-modify-write (RMW) (e.g., fetch-and-add,
 * fetch-and-sub, etc) operation (compare-and-exhange is excluded) */
class FaiReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;

	FaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     llvm::AtomicRMWInst::BinOp op, llvm::GenericValue val)
		: ReadLabel(EL_FaiRead, st, ord, pos, addr, typ, rf),
		  binOp(op), opValue(val) {}

public:
	/* Returns the type of this RMW operation (e.g., add, sub) */
	llvm::AtomicRMWInst::BinOp getOp() const { return binOp; }

	/* Returns the other operand's value */
	const llvm::GenericValue& getOpVal() const { return opValue; }

	FaiReadLabel *clone() const override { return new FaiReadLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_FaiRead;
	}

private:
	/* The binary operator for this RMW operation */
	const llvm::AtomicRMWInst::BinOp binOp;

	/* The other operand's value for the operation */
	const llvm::GenericValue opValue;
};


/*******************************************************************************
 **                         CasReadLabel Class
 ******************************************************************************/

/* Represents the read part of a compare-and-swap (CAS) operation */
class CasReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;

	CasReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     const llvm::GenericValue &exp, const llvm::GenericValue &swap,
		     bool lockCas = false)
		: ReadLabel(EL_CasRead, st, ord, pos, addr, typ, rf),
		  expected(exp), swapValue(swap), lockCas(lockCas) {}

public:
	/* Returns the value that will make this CAS succeed */
	const llvm::GenericValue& getExpected() const { return expected; }

	/* Returns the value that will be written is the CAS succeeds */
	const llvm::GenericValue& getSwapVal() const { return swapValue; }

	/* Returns true if this CAS models a lock acquire operation */
	bool isLock() const { return lockCas; }

	CasReadLabel *clone() const override { return new CasReadLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_CasRead;
	}

private:
	/* The value that will make this CAS succeed */
	const llvm::GenericValue expected;

	/* The value that will be written if the CAS succeeds */
	const llvm::GenericValue swapValue;

	/* Whether this CAS models a lock-acquire operation */
	const bool lockCas;
};


/*******************************************************************************
 **                         LibReadLabel Class
 ******************************************************************************/

/* Represents an operation with read semantics of a higher-level library
 * (e.g., lock, dequeue, etc) */
class LibReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;

	LibReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     std::string name)
		: ReadLabel(EL_LibRead, st, ord, pos, addr, typ, rf),
		  functionName(name) {}

public:
	/* Returns the name of this operation */
	const std::string& getFunctionName() const { return functionName; }

	/* Returns a vector with writes that this read could not read from
	 * when it was first added to the graph */
	const std::vector<Event>& getInvalidRfs() const { return invalidRfs; }

	LibReadLabel *clone() const override { return new LibReadLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_LibRead;
	}

private:
	/* Records a write that this read cannot read from */
	void addInvalidRf(Event rf) { invalidRfs.push_back(rf); }

	/* Name of the corresponding library member with read semantics */
	std::string functionName;

	/* The list with writes this read could not read from when it was
	 * first added to the execution graph */
	std::vector<Event> invalidRfs;
};


/*******************************************************************************
 **                         WriteLabel Class
 ******************************************************************************/

/* Represents a write operation. All special types of writes (e.g., FAI, CAS)
 * should inherit from this class */
class WriteLabel : public MemAccessLabel {

protected:
	friend class ExecutionGraph;

	WriteLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		   Event pos, const llvm::GenericValue *addr,
		   const llvm::Type *valTyp, llvm::GenericValue val,
		   bool isUnlock = false)
		: MemAccessLabel(k, st, ord, pos), value(val), addr(addr),
		  valueType(valTyp), unlock(isUnlock) {}

	WriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		   const llvm::GenericValue *addr, const llvm::Type *valTyp,
		   llvm::GenericValue val, bool isUnlock = false)
		: MemAccessLabel(EL_Write, st, ord, pos), value(val),
		  addr(addr), valueType(valTyp), unlock(isUnlock) {}

public:
	/* Returns the value written by this write access */
	const llvm::GenericValue& getVal() const { return value; }

	/* Returns a list of the reads reading from this write */
	const std::vector<Event>& getReadersList() const { return readerList; }

	/* Returns the address this write is accessing */
	const llvm::GenericValue *getAddr() const override { return addr; }

	/* Returns the type of the value this write is writing */
	const llvm::Type *getType() const override { return valueType; }

	/* Returns true if this write models the release of a lock */
	bool isUnlock() const { return unlock; }

	WriteLabel *clone() const override { return new WriteLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() >= EL_Write &&
		       lab->getKind() <= EL_LastWrite;
	}

private:
	/* Adds a read to the list of reads reading from the write */
	void addReader(Event r) {
		if (std::find(readerList.begin(), readerList.end(), r) ==
		    readerList.end())
			readerList.push_back(r);
	}

	/* Removes all readers that satisfy predicate F */
	template <typename F>
	void removeReader(F cond) {
		readerList.erase(std::remove_if(readerList.begin(),
						readerList.end(), [&](Event r)
						{ return cond(r); }),
				 readerList.end());
	}

	/* Getter/setter for a view representing the release sequence
	 * of this write */
	const View& getMsgView() const { return msgView; }
	void setMsgView(View &&v) { msgView = std::move(v); }

	/* The value written by this label */
	const llvm::GenericValue value;

	/* The address this write is accessing */
	const llvm::GenericValue *addr;

	/* The type of the value being written */
	const llvm::Type *valueType;

	/* Whether this write models an unlock operation */
	const bool unlock;

	/* View for the release sequence of the write */
	View msgView;

	/* List of reads reading from the write */
	std::vector<Event> readerList;
};


/*******************************************************************************
 **                         FaiWriteLabel Class
 ******************************************************************************/

/* Represents the write part of a read-modify-write (RMW) operation (e.g,
fetch-and-add, fetch-and-sub, etc) */
class FaiWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;

	FaiWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val)
		: WriteLabel(EL_FaiWrite, st, ord, pos, addr, valTyp, val) {}

public:
	FaiWriteLabel *clone() const override { return new FaiWriteLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_FaiWrite;
	}
};


/*******************************************************************************
 **                         CasWriteLabel Class
 ******************************************************************************/

/* Represents the write part of a compare-and-swap (CAS) operation */
class CasWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;

	CasWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val, bool lockCas)
		: WriteLabel(EL_CasWrite, st, ord, pos, addr, valTyp, val),
		  lockCas(lockCas) {}

public:
	/* Returns true if this label is used for modeling a lock-acquire op */
	bool isLock() const { return lockCas; }

	CasWriteLabel *clone() const override { return new CasWriteLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_CasWrite;
	}

private:
	/* Whether this label is used for modeling a lock-acquire op */
	const bool lockCas;
};


/*******************************************************************************
 **                         LibWriteLabel Class
 ******************************************************************************/

/* Write counterpart of LibReadLabel clas */
class LibWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;

	LibWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val, std::string name, bool isInit)
		: WriteLabel(EL_LibWrite, st, ord, pos, addr, valTyp, val),
		  functionName(name), initial(isInit) {}

public:
	/* Returns the name of the respective function of the library */
	const std::string& getFunctionName() const { return functionName; }

	/* Returns true if this is the initializing write for this memory
	 * location and this particular library */
	bool isLibInit() const { return initial; }

	LibWriteLabel *clone() const override { return new LibWriteLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_LibWrite;
	}

private:
	/* The name of the corresponding library member */
	std::string functionName;

	/* Whether this is the initializing write */
	bool initial;
};


/*******************************************************************************
 **                         FenceLabel Class
 ******************************************************************************/

/* Represents a fence */
class FenceLabel : public EventLabel {

protected:
	friend class ExecutionGraph;

	FenceLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos)
		: EventLabel(EL_Fence, st, ord, pos) {}

public:
	FenceLabel *clone() const override { return new FenceLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_Fence;
	}
};


/*******************************************************************************
 **                     ThreadCreateLabel Class
 ******************************************************************************/

/* This label denotes the creation of a thread (via, e.g., pthread_create()) */
class ThreadCreateLabel : public EventLabel {

friend class ExecutionGraph;

protected:
	ThreadCreateLabel(unsigned int st, llvm::AtomicOrdering ord,
			  Event pos, unsigned int cid)
		: EventLabel(EL_ThreadCreate, st, ord, pos), childId(cid) {}

public:
	/* Returns an identifier for the thread created (child) */
	unsigned int getChildId() const { return childId; }

	ThreadCreateLabel *clone() const override {
		return new ThreadCreateLabel(*this);
	}

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_ThreadCreate;
	}

private:
	/* The identifier of the child thread */
	unsigned int childId;
};


/*******************************************************************************
 **                     ThreadJoinLabel Class
 ******************************************************************************/

/* Represents a join() operation (e.g., pthread_join()) */
class ThreadJoinLabel : public EventLabel {

friend class ExecutionGraph;

protected:
	ThreadJoinLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			unsigned int childId)
		: EventLabel(EL_ThreadJoin, st, ord, pos),
		  childId(childId), childLast(Event::getInitializer()) {}

public:
	/* Returns the identifier of the thread this join() is waiting on */
	unsigned int getChildId() const { return childId; }

	/* Returns the last event of the thread the join() is waiting on */
	Event getChildLast() const { return childLast; }

	ThreadJoinLabel *clone() const override {
		return new ThreadJoinLabel(*this);
	}

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_ThreadJoin;
	}

private:
	/* Sets the last event of the thread the join() is waiting on */
	void setChildLast(Event e) { childLast = e; }

	/* The identifier of the child */
	const unsigned int childId;

	/* Position of the last event of the thread the join() is waiting on */
	Event childLast;
};


/*******************************************************************************
 **                     ThreadStartLabel Class
 ******************************************************************************/

/* Represents the beginning of a thread. This label synchronizes with the
 * ThreadCreateLabel that led to the creation of this thread */
class ThreadStartLabel : public EventLabel {

protected:
	friend class ExecutionGraph;

	ThreadStartLabel(unsigned int st, llvm::AtomicOrdering ord,
			 Event pos, Event pc)
		: EventLabel(EL_ThreadStart, st, ord, pos), parentCreate(pc) {}

public:
	/* Returns the position of the corresponding create operation */
	Event getParentCreate() const { return parentCreate; }

	ThreadStartLabel *clone() const override {
		return new ThreadStartLabel(*this);
	}

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_ThreadStart;
	}

private:
	/* The position of the corresponding create opeartion */
	Event parentCreate;
};


/*******************************************************************************
 **                     ThreadFinishLabel Class
 ******************************************************************************/

/* Represents the ending of a thread. This label synchronizes with the
 * ThreadJoinLabel that awaits for this particular thread (if any)
 *
 * FIXME: no error is reported if multiple threads are waiting on the
 * same thread */
class ThreadFinishLabel : public EventLabel {

	friend class ExecutionGraph;

protected:
	ThreadFinishLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos)
		: EventLabel(EL_ThreadFinish, st, ord, pos),
		  parentJoin(Event::getInitializer()) {}

public:
	/* Returns the join() operation waiting on this thread (or the
	 * initializer event, if no such operation exists) */
	Event getParentJoin() const { return parentJoin; }

	ThreadFinishLabel *clone() const override {
		return new ThreadFinishLabel(*this);
	}

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_ThreadFinish;
	}

private:
	/* Sets the corresponding join() event */
	void setParentJoin(Event j) { parentJoin = j; }

	/* Position of corresponding join() event in the graph
	 * (INIT if such event does not exist) */
	Event parentJoin;
};


/*******************************************************************************
 **                        MallocLabel Class
 ******************************************************************************/

/* Corresponds to a memory-allocating operation (e.g., malloc()) */
class MallocLabel : public EventLabel {

friend class ExecutionGraph;

protected:
	MallocLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		    const void *addr, unsigned int size)
		: EventLabel(EL_Malloc, st, ord, pos),
		  allocAddr(addr), allocSize(size) {}

public:
	/* Returns the (fresh) address returned by the allocation */
	const void *getAllocAddr() const { return allocAddr; }

	/* Returns the size of this allocation */
	unsigned int getAllocSize() const { return allocSize; }

	MallocLabel *clone() const override { return new MallocLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_Malloc;
	}

private:
	/* The address returned by malloc() */
	const void *allocAddr;

	/* The size of the requested allocation */
	unsigned int allocSize;
};


/*******************************************************************************
 **                         FreeLabel Class
 ******************************************************************************/

/* Corresponds to a memory-freeing operation (e.g., free()) */
class FreeLabel : public EventLabel {

friend class ExecutionGraph;

protected:
	FreeLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		  const void *addr, unsigned int size)
		: EventLabel(EL_Free, st, ord, pos),
		  freeAddr(addr), allocSize(size) {}

public:
	/* Returns the address being freed */
	const void *getFreedAddr() const { return freeAddr; }

	/* Returns the size of the freed allocation */
	unsigned int getAllocSize() const { return allocSize; }

	FreeLabel *clone() const override { return new FreeLabel(*this); }

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EL_Free;
	}

private:
	/* The address of the memory freed */
	const void *freeAddr;

	/* The size of the freed allocation */
	unsigned int allocSize;
};

#endif /* #define __EVENTLABEL_HPP__ */
