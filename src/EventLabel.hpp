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
#include "DepView.hpp"
#include "InterpreterEnumAPI.hpp"
#include "View.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

class DskAccessLabel;
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
		EL_Empty,
		EL_ThreadStart,
		EL_ThreadFinish,
		EL_ThreadCreate,
		EL_ThreadJoin,
		EL_MemAccessBegin,
		EL_Read,
		EL_FaiRead,
		EL_CasRead,
		EL_LibRead,
		EL_DskRead,
		EL_LastRead,
		EL_Write,
		EL_FaiWrite,
		EL_CasWrite,
		EL_LibWrite,
		EL_DskWrite,
		EL_DskMdWrite,
		EL_DskJnlWrite,
		EL_DskDirWrite,
		EL_LastDskWrite,
		EL_LastWrite,
		EL_MemAccessEnd,
		EL_Fence,
		EL_DskFsync,
		EL_DskSync,
		EL_DskPbarrier,
		EL_LastFence,
		EL_Malloc,
		EL_Free,
		EL_LockLabelLAPOR,
		EL_UnlockLabelLAPOR,
		EL_DskOpen,
	};

protected:
	/* ExecutionGraph needs to be a friend to call the constructors */
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	EventLabel(EventLabelKind k, unsigned int s, llvm::AtomicOrdering o,
		   Event p) : kind(k), stamp(s), ordering(o), position(p) {}

public:

	/* Returns the discriminator of this object */
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

	/* Methods that get/set the vector clocks for this label. */
	const View& getHbView() const { return hbView; }
	const View& getPorfView() const { return porfView; }
	const DepView& getPPoView() const { return ppoView; }
	const DepView& getPPoRfView() const { return pporfView; }

	void updateHbView(const View &v) { hbView.update(v); };
	void updatePorfView(const View &v) { porfView.update(v); };
	void updatePPoView(const View &v) { ppoView.update(v); };
	void updatePPoRfView(const DepView &v) { pporfView.update(v); };

	void setHbView(View &&v) { hbView = std::move(v); }
	void setPorfView(View &&v) { porfView = std::move(v); }
	void setPPoView(DepView &&v) { ppoView = std::move(v); }
	void setPPoRfView(DepView &&v) { pporfView = std::move(v); }

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

	/* Necessary for multiple inheritance + LLVM-style RTTI to work */
	static bool classofKind(EventLabelKind K) { return true; }
	static DskAccessLabel *castToDskAccessLabel(const EventLabel *);
	static EventLabel *castFromDskAccessLabel(const DskAccessLabel *);

	virtual ~EventLabel() {}

	/* Returns a clone object (virtual to allow deep copying from base) */
	virtual EventLabel *clone() const = 0;

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const EventLabel &lab);

private:
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

	/* Events that are (ppo U rf)*;ppo-before this label */
	DepView ppoView;

	/* Events that are (ppo U rf)*-before this label */
	DepView pporfView;
};

llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
			      const llvm::AtomicOrdering o);
llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
			      const EventLabel::EventLabelKind k);


/*******************************************************************************
 **                       DskAccessLabel Class
 ******************************************************************************/

/* This is used only as base class of specific labels that
 * model disk accesses. These labels are (only the top classes that directly
 * derive from EventLabel are listed, not their subclasses):
 *
 *     DskReadLabel
 *     DskWriteLabel
 *     DskSyncLabel
 *     DskFsyncLabel
 *     DskPbarrierLabel
 *
 * Note: This is not a child of EventLabel to avoid virtual inheritance */
class DskAccessLabel {

protected:
	DskAccessLabel(EventLabel::EventLabelKind k)
		: eventLabelKind(k) {}

public:
	/* Getter/setter for a view representing (a subset of)
	 * events that have persisted before this disk access */
	const DepView& getPbView() const { return pbView; }
	void setPbView(DepView &&v) { pbView = std::move(v); }

	EventLabel::EventLabelKind getEventLabelKind() const {
		return eventLabelKind;
	}

	static bool classof(const EventLabel *lab) {
		return lab->getKind() == EventLabel::EL_DskRead ||
		       (lab->getKind() >= EventLabel::EL_DskWrite &&
			lab->getKind() <= EventLabel::EL_LastDskWrite) ||
		       lab->getKind() == EventLabel::EL_DskSync ||
		       lab->getKind() == EventLabel::EL_DskFsync ||
		       lab->getKind() == EventLabel::EL_DskPbarrier;
	}

private:
	/* The EventLabel class that this disk access represents */
	EventLabel::EventLabelKind eventLabelKind;

	/* View indicating a _subset_ of the events that must have
	 * persisted before the access */
	DepView pbView;
};


/*******************************************************************************
 **                            EmptyLabel Class
 ******************************************************************************/

/* A plain empty label. This label type provides a good alternative to nullptr
 * in graphs that track dependencies where the po-predecessors of a label might
 * not exist (or might have been removed). It also does not break LLVM-style
 * RTTI, in contrast to nullptr */
class EmptyLabel : public EventLabel {

public:

	EmptyLabel(unsigned int st, Event pos)
		: EventLabel(EL_Empty, st, llvm::AtomicOrdering::NotAtomic,
			     pos) {}

	EmptyLabel *clone() const override { return new EmptyLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Empty; }
};


/*******************************************************************************
 **                       MemAccessLabel Class (Abstract)
 ******************************************************************************/

/* This label abstracts the common functionality that loads and stores have
 * (e.g., asking for the address of such a label) */
class MemAccessLabel : public EventLabel {

protected:
	MemAccessLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		       Event pos, const llvm::GenericValue *loc, const llvm::Type *typ)
		: EventLabel(k, st, ord, pos), addr(loc), valueType(typ) {}
public:
	/* Returns the address of this access */
	const llvm::GenericValue *getAddr() const { return addr; }

	/* Returns the type of the access's value */
	const llvm::Type *getType() const { return valueType; }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_MemAccessBegin && k <= EL_MemAccessEnd;
	}

private:
	/* The address of the accessing */
	const llvm::GenericValue *addr;

	/* The type of the value accessed */
	const llvm::Type *valueType;
};


/*******************************************************************************
 **                         ReadLabel Class
 ******************************************************************************/

/* The label for reads. All special read types (e.g., FAI, CAS) should inherit
 * from this class */
class ReadLabel : public MemAccessLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	ReadLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		  Event pos, const llvm::GenericValue *loc,
		  const llvm::Type *typ, Event rf)
		: MemAccessLabel(k, st, ord, pos, loc, typ),
		  readsFrom(rf), revisitable(true) {}

public:
	ReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		  const llvm::GenericValue *loc, const llvm::Type *typ,
		  Event rf)
		: MemAccessLabel(EL_Read, st, ord, pos, loc, typ),
		  readsFrom(rf), revisitable(true) {}

	/* Returns the position of the write this read is readinf-from */
	Event getRf() const { return readsFrom; }

	/* Returns true if this read can be revisited */
	bool isRevisitable() const { return revisitable; }

	ReadLabel *clone() const override { return new ReadLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_Read && k <= EL_LastRead;
	}

private:
	/* Changes the reads-from edge for this label. This should only
	 * be called from the execution graph to update other relevant
	 * information as well */
	void setRf(Event rf) { readsFrom = rf; }

	/* Makes the relevant read revisitable/non-revisitable. The
	 * execution graph is responsible for making such changes */
	void setRevisitStatus(bool status) { revisitable = status; }

	/* Position of the write it is reading from in the graph */
	Event readsFrom;

	/* Revisitability status */
	bool revisitable;
};


/*******************************************************************************
 **                         FaiReadLabel Class
 ******************************************************************************/

/* Represents the read part of a read-modify-write (RMW) (e.g., fetch-and-add,
 * fetch-and-sub, etc) operation (compare-and-exhange is excluded) */
class FaiReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	FaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     llvm::AtomicRMWInst::BinOp op, llvm::GenericValue val)
		: ReadLabel(EL_FaiRead, st, ord, pos, addr, typ, rf),
		  binOp(op), opValue(val) {}

	/* Returns the type of this RMW operation (e.g., add, sub) */
	llvm::AtomicRMWInst::BinOp getOp() const { return binOp; }

	/* Returns the other operand's value */
	const llvm::GenericValue& getOpVal() const { return opValue; }

	FaiReadLabel *clone() const override { return new FaiReadLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_FaiRead; }

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
	friend class DepExecutionGraph;

public:
	CasReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     const llvm::GenericValue &exp, const llvm::GenericValue &swap,
		     bool lockCas = false)
		: ReadLabel(EL_CasRead, st, ord, pos, addr, typ, rf),
		  expected(exp), swapValue(swap), lockCas(lockCas) {}

	/* Returns the value that will make this CAS succeed */
	const llvm::GenericValue& getExpected() const { return expected; }

	/* Returns the value that will be written is the CAS succeeds */
	const llvm::GenericValue& getSwapVal() const { return swapValue; }

	/* Returns true if this CAS models a lock acquire operation */
	bool isLock() const { return lockCas; }

	CasReadLabel *clone() const override { return new CasReadLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_CasRead; }

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
	friend class DepExecutionGraph;

public:
	LibReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     std::string name)
		: ReadLabel(EL_LibRead, st, ord, pos, addr, typ, rf),
		  functionName(name) {}

	/* Returns the name of this operation */
	const std::string& getFunctionName() const { return functionName; }

	/* Returns a vector with writes that this read could not read from
	 * when it was first added to the graph */
	const std::vector<Event>& getInvalidRfs() const { return invalidRfs; }

	LibReadLabel *clone() const override { return new LibReadLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LibRead; }

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
 **                         DskReadLabel Class
 ******************************************************************************/

/* Models a read from the disk (e.g., via read()) */
class DskReadLabel : public ReadLabel, public DskAccessLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *loc, const llvm::Type *typ,
		     Event rf)
		: ReadLabel(EL_DskRead, st, ord, pos, loc, typ, rf),
		  DskAccessLabel(EL_DskRead) {}

	DskReadLabel *clone() const override { return new DskReadLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskRead; }
	static DskAccessLabel *castToDskAccessLabel(const DskReadLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskReadLabel*>(D));
	}
	static DskReadLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskReadLabel *>(const_cast<DskAccessLabel*>(DC));
	}

private:
	/* Nothing necessary for the time being */
};


/*******************************************************************************
 **                         WriteLabel Class
 ******************************************************************************/

/* Represents a write operation. All special types of writes (e.g., FAI, CAS)
 * should inherit from this class */
class WriteLabel : public MemAccessLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	WriteLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		   Event pos, const llvm::GenericValue *addr,
		   const llvm::Type *valTyp, llvm::GenericValue val,
		   bool isUnlock = false)
		: MemAccessLabel(k, st, ord, pos, addr, valTyp),
		  value(val), unlock(isUnlock) {}

public:
	WriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		   const llvm::GenericValue *addr, const llvm::Type *valTyp,
		   llvm::GenericValue val, bool isUnlock = false)
		: MemAccessLabel(EL_Write, st, ord, pos, addr, valTyp),
		  value(val), unlock(isUnlock) {}

	/* Returns the value written by this write access */
	const llvm::GenericValue& getVal() const { return value; }

	/* Returns a list of the reads reading from this write */
	const std::vector<Event>& getReadersList() const { return readerList; }

	/* Getter/setter for a view representing the
	 * release sequence of this write */
	const View& getMsgView() const { return msgView; }
	void setMsgView(View &&v) { msgView = std::move(v); }

	/* Returns true if this write models the release of a lock */
	bool isUnlock() const { return unlock; }

	WriteLabel *clone() const override { return new WriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_Write && k <= EL_LastWrite;
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

	/* The value written by this label */
	const llvm::GenericValue value;

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
	friend class DepExecutionGraph;

public:
	FaiWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val)
		: WriteLabel(EL_FaiWrite, st, ord, pos, addr, valTyp, val) {}

	FaiWriteLabel *clone() const override { return new FaiWriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_FaiWrite; }
};


/*******************************************************************************
 **                         CasWriteLabel Class
 ******************************************************************************/

/* Represents the write part of a compare-and-swap (CAS) operation */
class CasWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	CasWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val, bool lockCas)
		: WriteLabel(EL_CasWrite, st, ord, pos, addr, valTyp, val),
		  lockCas(lockCas) {}

	/* Returns true if this label is used for modeling a lock-acquire op */
	bool isLock() const { return lockCas; }

	CasWriteLabel *clone() const override { return new CasWriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_CasWrite; }

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
	friend class DepExecutionGraph;

public:
	LibWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val, std::string name, bool isInit)
		: WriteLabel(EL_LibWrite, st, ord, pos, addr, valTyp, val),
		  functionName(name), initial(isInit) {}

	/* Returns the name of the respective function of the library */
	const std::string& getFunctionName() const { return functionName; }

	/* Returns true if this is the initializing write for this memory
	 * location and this particular library */
	bool isLibInit() const { return initial; }

	LibWriteLabel *clone() const override { return new LibWriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LibWrite; }

private:
	/* The name of the corresponding library member */
	std::string functionName;

	/* Whether this is the initializing write */
	bool initial;
};


/*******************************************************************************
 **                         DskWriteLabel Class
 ******************************************************************************/

/* Models a write to disk (e.g., via write()) */
class DskWriteLabel : public WriteLabel, public DskAccessLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	DskWriteLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		      Event pos,  const llvm::GenericValue *addr,
		      const llvm::Type *valTyp, llvm::GenericValue val, void *mapping)
		: WriteLabel(k, st, ord, pos, addr, valTyp, val),
		  DskAccessLabel(k), mapping(mapping) {}

public:
	DskWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val, void *mapping)
		: DskWriteLabel(EL_DskWrite, st, ord, pos, addr, valTyp, val, mapping) {}

	/* Returns the starting offset for this write's disk mapping */
	const void *getMapping() const { return mapping; }

	DskWriteLabel *clone() const override { return new DskWriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_DskWrite && k <= EL_LastDskWrite;
	}
	static DskAccessLabel *castToDskAccessLabel(const DskWriteLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskWriteLabel*>(D));
	}
	static DskWriteLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskWriteLabel *>(const_cast<DskAccessLabel*>(DC));
	}

private:
	/* The starting offset for this write's disk mapping */
	void *mapping;
};


/*******************************************************************************
 **                         DskMdWriteLabel Class
 ******************************************************************************/

/* Models a disk write that writes metadata */
class DskMdWriteLabel : public DskWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskMdWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			const llvm::GenericValue *addr, const llvm::Type *valTyp,
			llvm::GenericValue val, void *mapping,
			std::pair<void *, void*> ordDataRange)
		: DskWriteLabel(EL_DskMdWrite, st, ord, pos, addr, valTyp, val, mapping),
		  ordDataRange(ordDataRange) {}

	/* Helpers that return data with which this write is ordered */
	const void *getOrdDataBegin() const { return ordDataRange.first; }
	const void *getOrdDataEnd() const { return ordDataRange.second; }
	const std::pair<void *, void *>
	getOrdDataRange() const { return ordDataRange; }

	DskMdWriteLabel *clone() const override { return new DskMdWriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskMdWrite; }
	static DskAccessLabel *castToDskAccessLabel(const DskMdWriteLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskMdWriteLabel*>(D));
	}
	static DskMdWriteLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskMdWriteLabel *>(const_cast<DskAccessLabel*>(DC));
	}

private:
	/* The data range [begin, end) that with which this write is ordered */
	std::pair<void *, void *> ordDataRange;
};


/*******************************************************************************
 **                         DskDirWriteLabel Class
 ******************************************************************************/

/* Models a write to a directory on disk */
class DskDirWriteLabel : public DskWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskDirWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			 const llvm::GenericValue *addr, const llvm::Type *valTyp,
			 llvm::GenericValue val, void *mapping)
		: DskWriteLabel(EL_DskDirWrite, st, ord, pos, addr, valTyp, val, mapping) {}

	DskDirWriteLabel *clone() const override { return new DskDirWriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskDirWrite; }
	static DskAccessLabel *castToDskAccessLabel(const DskDirWriteLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskDirWriteLabel*>(D));
	}
	static DskDirWriteLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskDirWriteLabel *>(const_cast<DskAccessLabel*>(DC));
	}

private:
	/* Nothing for the time being */
};


/*******************************************************************************
 **                         DskJnlWriteLabel Class
 ******************************************************************************/

/* Models a write to disk that marks the beginning or end of a disk transaction.
 * (We assume that each transaction affects only one inode.) */
class DskJnlWriteLabel : public DskWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskJnlWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			 const llvm::GenericValue *addr, const llvm::Type *valTyp,
			 llvm::GenericValue val, void *mapping, void *inode)
		: DskWriteLabel(EL_DskJnlWrite, st, ord, pos, addr, valTyp, val, mapping),
		  inode(inode) {}

	/* Returns the inode on which the transaction takes place */
	const void *getTransInode() const { return inode; }

	DskJnlWriteLabel *clone() const override { return new DskJnlWriteLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskJnlWrite; }
	static DskAccessLabel *castToDskAccessLabel(const DskJnlWriteLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskJnlWriteLabel*>(D));
	}
	static DskJnlWriteLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskJnlWriteLabel *>(const_cast<DskAccessLabel*>(DC));
	}

private:
	/* The inode on which the transaction takes place */
	void *inode;
};


/*******************************************************************************
 **                         FenceLabel Class
 ******************************************************************************/

/* Represents a fence */
class FenceLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	FenceLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		   Event pos)
		: EventLabel(k, st, ord, pos) {}
public:
	FenceLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos)
		: EventLabel(EL_Fence, st, ord, pos) {}

	FenceLabel *clone() const override { return new FenceLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_Fence && k <= EL_LastFence;
	}
};


/*******************************************************************************
 **                         DskFsyncLabel Class
 ******************************************************************************/

/* Represents an fsync() operation */
class DskFsyncLabel : public FenceLabel, public DskAccessLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskFsyncLabel(unsigned int st, llvm::AtomicOrdering ord,
		      Event pos, const void *inode, unsigned int size)
		: FenceLabel(EL_DskFsync, st, ord, pos), inode(inode), size(size),
		  DskAccessLabel(EL_DskFsync) {}

	/* Returns a pointer to the inode on which the fsync() took place */
	const void *getInode() const { return inode; }

	/* Returns the "size" of this fsync()'s range */
	const unsigned int getSize() const { return size; }

	DskFsyncLabel *clone() const override { return new DskFsyncLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskFsync; }
	static DskAccessLabel *castToDskAccessLabel(const DskFsyncLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskFsyncLabel*>(D));
	}
	static DskFsyncLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskFsyncLabel *>(const_cast<DskAccessLabel*>(DC));
	}

private:
	/* The inode on which the fsync() was issued */
	const void *inode;

	/* The range of this fsync() */
	const unsigned int size;
};


/*******************************************************************************
 **                         DskSyncLabel Class
 ******************************************************************************/

/* Represents an operation that synchronizes writes to persistent storage (e.g, sync()) */
class DskSyncLabel : public FenceLabel, public DskAccessLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskSyncLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos)
		: FenceLabel(EL_DskSync, st, ord, pos), DskAccessLabel(EL_DskSync) {}

	DskSyncLabel *clone() const override { return new DskSyncLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskSync; }
	static DskAccessLabel *castToDskAccessLabel(const DskSyncLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskSyncLabel*>(D));
	}
	static DskSyncLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskSyncLabel *>(const_cast<DskAccessLabel*>(DC));
	}
};


/******************************************************************************
 **                        DskPbarrierLabel Class
 ******************************************************************************/

/* Corresponds to a call to __VERIFIER_pbarrier(), i.e.,
 * all events before this label will have persisted when the
 * recovery routine runs */
class DskPbarrierLabel : public FenceLabel, public DskAccessLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskPbarrierLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos)
		: FenceLabel(EL_DskPbarrier, st, ord, pos), DskAccessLabel(EL_DskPbarrier) {}

	DskPbarrierLabel *clone() const override { return new DskPbarrierLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskPbarrier; }
	static DskAccessLabel *castToDskAccessLabel(const DskPbarrierLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskPbarrierLabel*>(D));
	}
	static DskPbarrierLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskPbarrierLabel *>(const_cast<DskAccessLabel*>(DC));
	}

private:
	/* Nothing necessary for the time being */
};


/*******************************************************************************
 **                     ThreadCreateLabel Class
 ******************************************************************************/

/* This label denotes the creation of a thread (via, e.g., pthread_create()) */
class ThreadCreateLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	ThreadCreateLabel(unsigned int st, llvm::AtomicOrdering ord,
			  Event pos, unsigned int cid)
		: EventLabel(EL_ThreadCreate, st, ord, pos), childId(cid) {}

	/* Returns an identifier for the thread created (child) */
	unsigned int getChildId() const { return childId; }

	ThreadCreateLabel *clone() const override {
		return new ThreadCreateLabel(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadCreate; }

private:
	/* The identifier of the child thread */
	unsigned int childId;
};


/*******************************************************************************
 **                     ThreadJoinLabel Class
 ******************************************************************************/

/* Represents a join() operation (e.g., pthread_join()) */
class ThreadJoinLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	ThreadJoinLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			unsigned int childId)
		: EventLabel(EL_ThreadJoin, st, ord, pos),
		  childId(childId), childLast(Event::getInitializer()) {}

	/* Returns the identifier of the thread this join() is waiting on */
	unsigned int getChildId() const { return childId; }

	/* Returns the last event of the thread the join() is waiting on */
	Event getChildLast() const { return childLast; }

	ThreadJoinLabel *clone() const override {
		return new ThreadJoinLabel(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadJoin; }

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
	friend class DepExecutionGraph;

public:
	ThreadStartLabel(unsigned int st, llvm::AtomicOrdering ord,
			 Event pos, Event pc, int symm = -1)
		: EventLabel(EL_ThreadStart, st, ord, pos), parentCreate(pc), symmetricTid(symm) {}

	/* Returns the position of the corresponding create operation */
	Event getParentCreate() const { return parentCreate; }

	/* SR: Returns the id of a symmetric thread, or -1 if no symmetric thread exists  */
	int getSymmetricTid() const { return symmetricTid; }

	ThreadStartLabel *clone() const override {
		return new ThreadStartLabel(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadStart; }

private:
	/* The position of the corresponding create opeartion */
	Event parentCreate;

	/* SR: The tid a symmetric thread (currently: minimum among all) */
	int symmetricTid = -1;
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
	friend class DepExecutionGraph;

public:
	ThreadFinishLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos)
		: EventLabel(EL_ThreadFinish, st, ord, pos),
		  parentJoin(Event::getInitializer()) {}

	/* Returns the join() operation waiting on this thread (or the
	 * initializer event, if no such operation exists) */
	Event getParentJoin() const { return parentJoin; }

	ThreadFinishLabel *clone() const override {
		return new ThreadFinishLabel(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadFinish; }

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

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	MallocLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		    const void *addr, unsigned int size, Storage s, AddressSpace spc)
		: EventLabel(EL_Malloc, st, ord, pos),
		  allocAddr(addr), allocSize(size), s(s), spc(spc) {}

	/* Returns the (fresh) address returned by the allocation */
	const void *getAllocAddr() const { return allocAddr; }

	/* Returns the size of this allocation */
	unsigned int getAllocSize() const { return allocSize; }

	/* Returns the address space of this chunk */
	AddressSpace getAddrSpace() const { return spc; }

	/* Returns the storage type of this chunk */
	Storage getStorage() const { return s; }

	MallocLabel *clone() const override { return new MallocLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Malloc; }

private:
	/* The address returned by malloc() */
	const void *allocAddr;

	/* The size of the requested allocation */
	unsigned int allocSize;

	/* Whether this chunk lives in the stack/heap/internal memory */
	AddressSpace spc;

	/* Storage typ */
	Storage s;
};


/*******************************************************************************
 **                         FreeLabel Class
 ******************************************************************************/

/* Corresponds to a memory-freeing operation (e.g., free()) */
class FreeLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	FreeLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		  const void *addr)
		: EventLabel(EL_Free, st, ord, pos),
		  freeAddr(addr) {}

	/* Returns the address being freed */
	const void *getFreedAddr() const { return freeAddr; }

	FreeLabel *clone() const override { return new FreeLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Free; }

private:
	/* The address of the memory freed */
	const void *freeAddr;
};


/*******************************************************************************
 **                         LockLabelLAPOR Class
 ******************************************************************************/

/* Corresponds to a label modeling a lock operation --under LAPOR only-- */
class LockLabelLAPOR : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	LockLabelLAPOR(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		       const llvm::GenericValue *addr)
		: EventLabel(EL_LockLabelLAPOR, st, ord, pos),
		  lockAddr(addr) {}

	/* Returns the address of the acquired lock */
	const llvm::GenericValue *getLockAddr() const { return lockAddr; }

	LockLabelLAPOR *clone() const override {
		return new LockLabelLAPOR(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LockLabelLAPOR; }

private:
	/* The address of the acquired lock */
	const llvm::GenericValue *lockAddr;
};


/*******************************************************************************
 **                         UnlockLabelLAPOR Class
 ******************************************************************************/

/* Corresponds to a label modeling an unlock operation --under LAPOR only-- */
class UnlockLabelLAPOR : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	UnlockLabelLAPOR(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			 const llvm::GenericValue *addr)
		: EventLabel(EL_UnlockLabelLAPOR, st, ord, pos),
		  lockAddr(addr) {}

	/* Returns the address of the released lock */
	const llvm::GenericValue *getLockAddr() const { return lockAddr; }

	UnlockLabelLAPOR *clone() const override {
		return new UnlockLabelLAPOR(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_UnlockLabelLAPOR; }

private:
	/* The address of the released lock */
	const llvm::GenericValue *lockAddr;
};


/******************************************************************************
 **                        DskOpenLabel Class
 ******************************************************************************/

/* Corresponds to the beginning of a file-opening operation (e.g., open()) */
class DskOpenLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	DskOpenLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const char *fileName, llvm::GenericValue fd)
		: EventLabel(EL_DskOpen, st, ord, pos),
		  fileName(fileName), fd(fd) {}

	/* Returns the name of the opened file */
	const char *getFileName() const { return fileName; }

	/* Returns the file descriptor returned by open() */
	const llvm::GenericValue &getFd() const { return fd; }

	DskOpenLabel *clone() const override { return new DskOpenLabel(*this); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskOpen; }

private:
	/* The name of the opened file */
	const char *fileName;

	/* The file descriptor allocated for this call */
	llvm::GenericValue fd;
};


/* Specialization selected when ToTy is not a known subclass of DskAccessLabel */
template <class ToTy,
	  bool IsKnownSubtype = ::std::is_base_of<DskAccessLabel, ToTy>::value>
struct cast_convert_decl_context {
	static const ToTy *doit(const DskAccessLabel *Val) {
		return static_cast<const ToTy*>(EventLabel::castFromDskAccessLabel(Val));
	}

	static ToTy *doit(DskAccessLabel *Val) {
		return static_cast<ToTy*>(EventLabel::castFromDskAccessLabel(Val));
	}
};

/* Specialization selected when ToTy is a known subclass of DskAccessLabel */
template <class ToTy>
struct cast_convert_decl_context<ToTy, true> {
	static const ToTy *doit(const DskAccessLabel *Val) {
		return static_cast<const ToTy*>(Val);
	}

	static ToTy *doit(DskAccessLabel *Val) {
		return static_cast<ToTy*>(Val);
	}
};

namespace llvm {

	/* isa<T>(DskAccessLabel *) */
	template <typename To>
	struct isa_impl<To, ::DskAccessLabel> {
		static bool doit(const ::DskAccessLabel &Val) {
			return To::classofKind(Val.getEventLabelKind());
		}
	};

	/* cast<T>(DskAccessLabel *) */
	template<class ToTy>
	struct cast_convert_val<ToTy,
				const ::DskAccessLabel,const ::DskAccessLabel> {
		static const ToTy &doit(const ::DskAccessLabel &Val) {
			return *::cast_convert_decl_context<ToTy>::doit(&Val);
		}
	};

	template<class ToTy>
	struct cast_convert_val<ToTy, ::DskAccessLabel, ::DskAccessLabel> {
		static ToTy &doit(::DskAccessLabel &Val) {
			return *::cast_convert_decl_context<ToTy>::doit(&Val);
		}
	};

	template<class ToTy>
	struct cast_convert_val<ToTy,
				const ::DskAccessLabel*, const ::DskAccessLabel*> {
		static const ToTy *doit(const ::DskAccessLabel *Val) {
			return ::cast_convert_decl_context<ToTy>::doit(Val);
		}
	};

	template<class ToTy>
	struct cast_convert_val<ToTy, ::DskAccessLabel*, ::DskAccessLabel*> {
		static ToTy *doit(::DskAccessLabel *Val) {
			return ::cast_convert_decl_context<ToTy>::doit(Val);
		}
	};

	/// Implement cast_convert_val for EventLabel -> DskAccessLabel conversions.
	template<class FromTy>
	struct cast_convert_val< ::DskAccessLabel, FromTy, FromTy> {
		static ::DskAccessLabel &doit(const FromTy &Val) {
			return *FromTy::castToDskAccessLabel(&Val);
		}
	};

	template<class FromTy>
	struct cast_convert_val< ::DskAccessLabel, FromTy*, FromTy*> {
		static ::DskAccessLabel *doit(const FromTy *Val) {
			return FromTy::castToDskAccessLabel(Val);
		}
	};

	template<class FromTy>
	struct cast_convert_val< const ::DskAccessLabel, FromTy, FromTy> {
		static const ::DskAccessLabel &doit(const FromTy &Val) {
			return *FromTy::castToDskAccessLabel(&Val);
		}
	};

	template<class FromTy>
	struct cast_convert_val< const ::DskAccessLabel, FromTy*, FromTy*> {
		static const ::DskAccessLabel *doit(const FromTy *Val) {
			return FromTy::castToDskAccessLabel(Val);
		}
	};

} /* namespace llvm */

#endif /* __EVENTLABEL_HPP__ */
