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
#include "value_ptr.hpp"
#include "DepView.hpp"
#include "InterpreterEnumAPI.hpp"
#include "ModuleID.hpp"
#include "NameInfo.hpp"
#include "MemAccess.hpp"
#include "SAddr.hpp"
#include "SExpr.hpp"
#include "SVal.hpp"
#include "View.hpp"
#include <llvm/IR/Instructions.h> /* For AtomicOrdering in older LLVMs */
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
		EL_Block,
		EL_ThreadStart,
		EL_ThreadFinish,
		EL_ThreadCreate,
		EL_ThreadJoin,
		EL_LoopBegin,
		EL_SpinStart,
		EL_FaiZNESpinEnd,
		EL_LockZNESpinEnd,
		EL_MemAccessBegin,
		EL_Read,
		EL_BWaitRead,
		EL_FaiRead,
		EL_NoRetFaiRead,
		EL_BIncFaiRead,
		EL_FaiReadLast,
		EL_CasRead,
		EL_LockCasRead,
		EL_TrylockCasRead,
		EL_CasReadLast,
		EL_DskRead,
		EL_LastRead,
		EL_Write,
		EL_UnlockWrite,
		EL_BInitWrite,
		EL_BDestroyWrite,
		EL_FaiWrite,
		EL_NoRetFaiWrite,
		EL_BIncFaiWrite,
		EL_FaiWriteLast,
		EL_CasWrite,
		EL_LockCasWrite,
		EL_TrylockCasWrite,
		EL_CasWriteLast,
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
		EL_SmpFenceLKMM,
		EL_RCUSyncLKMM,
		EL_LastFence,
		EL_Malloc,
		EL_Free,
		EL_LockLabelLAPOR,
		EL_UnlockLabelLAPOR,
		EL_DskOpen,
		EL_RCULockLKMM,
		EL_RCUUnlockLKMM,
	};

protected:
	/* ExecutionGraph needs to be a friend to call the constructors */
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	EventLabel(EventLabelKind k, unsigned int s, llvm::AtomicOrdering o,
		   Event p) : kind(k), stamp(s), ordering(o), position(p) {}
	EventLabel(EventLabelKind k, llvm::AtomicOrdering o, Event p)
		: kind(k), stamp(0), ordering(o), position(p) {}

public:

	/* Returns the discriminator of this object */
	EventLabelKind getKind() const { return kind; }

	/* Getter/setter for the label stamp in an execution graph*/
	unsigned int getStamp() const { return stamp; }
	void setStamp(unsigned int s) { stamp = s; }

	/* Getter/setter for the label ordering */
	llvm::AtomicOrdering getOrdering() const { return ordering; }
	void setOrdering(llvm::AtomicOrdering ord) { ordering = ord; }

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

	virtual ~EventLabel() = default;

	/* Returns a clone object (virtual to allow deep copying from base) */
	virtual std::unique_ptr<EventLabel> clone() const = 0;

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const EventLabel &lab);

private:
	/* Discriminator enum for LLVM-style RTTI */
	const EventLabelKind kind;

	/* The stamp of this label in the execution graph */
	unsigned int stamp;

	/* Ordering of this access mode */
	llvm::AtomicOrdering ordering;

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
		: EventLabel(EL_Empty, st, llvm::AtomicOrdering::NotAtomic, pos) {}
	EmptyLabel(Event pos)
		: EventLabel(EL_Empty, llvm::AtomicOrdering::NotAtomic, pos) {}

	template<typename... Ts>
	static std::unique_ptr<EmptyLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<EmptyLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<EmptyLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Empty; }
};


/*******************************************************************************
 **                            BlockLabel Class
 ******************************************************************************/

/* A label that represents a blockage event */
class BlockLabel : public EventLabel {

public:
	BlockLabel(unsigned int st, Event pos, BlockageType t)
		: EventLabel(EL_Block, st, llvm::AtomicOrdering::NotAtomic, pos), type(t) {}
	BlockLabel(Event pos, BlockageType t)
		: EventLabel(EL_Block, llvm::AtomicOrdering::NotAtomic, pos), type(t) {}

	BlockageType getType() const { return type; }

	template<typename... Ts>
	static std::unique_ptr<BlockLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<BlockLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<BlockLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Block; }

private:
	BlockageType type;
};


/*******************************************************************************
 **                            LoopBeginLabel Class
 ******************************************************************************/

/* A label that marks the beginning of a spinloop. Used in DSA. */
class LoopBeginLabel : public EventLabel {

public:

	LoopBeginLabel(unsigned int st, Event pos)
		: EventLabel(EL_LoopBegin, st, llvm::AtomicOrdering::NotAtomic, pos) {}
	LoopBeginLabel(Event pos)
		: EventLabel(EL_LoopBegin, llvm::AtomicOrdering::NotAtomic, pos) {}

	template<typename... Ts>
	static std::unique_ptr<LoopBeginLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LoopBeginLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LoopBeginLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LoopBegin; }
};


/*******************************************************************************
 **                            SpinStartLabel Class
 ******************************************************************************/

/* A label that marks the beginning of a spinloop iteration. It is meant
 * to be used by the liveness (await-termination) checks. */
class SpinStartLabel : public EventLabel {

public:

	SpinStartLabel(unsigned int st, Event pos)
		: EventLabel(EL_SpinStart, st, llvm::AtomicOrdering::NotAtomic, pos) {}
	SpinStartLabel(Event pos)
		: EventLabel(EL_SpinStart, llvm::AtomicOrdering::NotAtomic, pos) {}

	template<typename... Ts>
	static std::unique_ptr<SpinStartLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<SpinStartLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<SpinStartLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_SpinStart; }
};


/*******************************************************************************
 **                            FaiZNESpinEndLabel Class
 ******************************************************************************/

/* A label that marks the end of a potential FaiZNE spinloop. If the loop turns out to be not
 * a spinloop, this is meaningless; otherwise, it indicates that the thread should block */
class FaiZNESpinEndLabel : public EventLabel {

public:

	FaiZNESpinEndLabel(unsigned int st, Event pos)
		: EventLabel(EL_FaiZNESpinEnd, st, llvm::AtomicOrdering::NotAtomic, pos) {}
	FaiZNESpinEndLabel(Event pos)
		: EventLabel(EL_FaiZNESpinEnd, llvm::AtomicOrdering::NotAtomic, pos) {}

	template<typename... Ts>
	static std::unique_ptr<FaiZNESpinEndLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<FaiZNESpinEndLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<FaiZNESpinEndLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_FaiZNESpinEnd; }
};


/*******************************************************************************
 **                            LockZNESpinEndLabel Class
 ******************************************************************************/

/* A label that marks the end of a potential LockZNE spinloop */
class LockZNESpinEndLabel : public EventLabel {

public:

	LockZNESpinEndLabel(unsigned int st, Event pos)
		: EventLabel(EL_LockZNESpinEnd, st, llvm::AtomicOrdering::NotAtomic, pos) {}
	LockZNESpinEndLabel(Event pos)
		: EventLabel(EL_LockZNESpinEnd, llvm::AtomicOrdering::NotAtomic, pos) {}

	template<typename... Ts>
	static std::unique_ptr<LockZNESpinEndLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LockZNESpinEndLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LockZNESpinEndLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LockZNESpinEnd; }
};


/*******************************************************************************
 **                       MemAccessLabel Class (Abstract)
 ******************************************************************************/

/* This label abstracts the common functionality that loads and stores have
 * (e.g., asking for the address of such a label) */
class MemAccessLabel : public EventLabel {

protected:
	MemAccessLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		       Event pos, SAddr loc, ASize size, AType type)
		: EventLabel(k, st, ord, pos), addr(loc), access(size, type) {}
	MemAccessLabel(EventLabelKind k, llvm::AtomicOrdering ord,
		       Event pos, SAddr loc, ASize size, AType type)
		: EventLabel(k, ord, pos), addr(loc), access(size, type) {}

	MemAccessLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		       Event pos, SAddr loc, AAccess a)
		: EventLabel(k, st, ord, pos), addr(loc), access(a) {}
	MemAccessLabel(EventLabelKind k, llvm::AtomicOrdering ord,
		       Event pos, SAddr loc, AAccess a)
		: EventLabel(k, ord, pos), addr(loc), access(a) {}
public:
	/* Returns the address of this access */
	SAddr getAddr() const { return addr; }

	/* Returns the size (in bytes) of the access */
	ASize getSize() const { return access.getSize(); }

	/* Returns the type of the access */
	AType getType() const { return access.getType(); }

	/* Returns the packed access */
	AAccess getAccess() const { return access; }

	bool wasAddedMax() const { return maximal; }
	void setAddedMax(bool status) { maximal = status; }

	/* Getter/setter for a the fence view of this memory access */
	const DepView& getFenceView() const { return fenceView; }
	void setFenceView(DepView &&v) { fenceView = std::move(v); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_MemAccessBegin && k <= EL_MemAccessEnd;
	}

private:
	/* The address of the accessing */
	SAddr addr;

	/* The size of the access performed (in bytes) */
	AAccess access;

	/* A view of fences that could be used by some memory models (e.g., LKMM) */
	DepView fenceView;

	/* Whether was mo-maximal when added */
	bool maximal = true;
};


/*******************************************************************************
 **                         ReadLabel Class
 ******************************************************************************/

/* The label for reads. All special read types (e.g., FAI, CAS) should inherit
 * from this class */
class ReadLabel : public MemAccessLabel {

public:
	using AnnotT = SExpr<ModuleID::ID>;
	using AnnotVP = value_ptr<AnnotT, SExprCloner<ModuleID::ID>>;

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

protected:
	ReadLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		  Event pos, SAddr loc, ASize size, AType type, Event rf,
		  AnnotVP annot = nullptr)
		: MemAccessLabel(k, st, ord, pos, loc, size, type),
		  readsFrom(rf), revisitable(true), annotExpr(std::move(annot)) {}
	ReadLabel(EventLabelKind k, llvm::AtomicOrdering ord,
		  Event pos, SAddr loc, ASize size, AType type, Event rf,
		  AnnotVP annot = nullptr)
		: MemAccessLabel(k, ord, pos, loc, size, type),
		  readsFrom(rf), revisitable(true), annotExpr(std::move(annot)) {}

public:
	ReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos, SAddr loc,
		  ASize size, AType type, Event rf = Event::getBottom(),
		  AnnotVP annot = nullptr)
		: ReadLabel(EL_Read, st, ord, pos, loc, size, type, rf, std::move(annot)) {}
	ReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr loc, ASize size, AType type,
		  Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: ReadLabel(EL_Read, ord, pos, loc, size, type, rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<ReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<ReadLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the position of the write this read is readinf-from */
	Event getRf() const { return readsFrom; }

	/* Returns true if this read can be revisited */
	bool isRevisitable() const { return revisitable; }

	/* Makes the relevant read revisitable/non-revisitable. The
	 * execution graph is responsible for making such changes */
	void setRevisitStatus(bool status) { revisitable = status; }

	/* SAVer: Getter/setter for the annotation expression */
	const AnnotT *getAnnot() const { return annotExpr.get(); }
	void setAnnot(std::unique_ptr<AnnotT> annot) { annotExpr = std::move(annot); }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<ReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_Read && k <= EL_LastRead;
	}

private:
	/* Changes the reads-from edge for this label. This should only
	 * be called from the execution graph to update other relevant
	 * information as well */
	void setRf(Event rf) { readsFrom = rf; }

	/* Position of the write it is reading from in the graph */
	Event readsFrom;

	/* Revisitability status */
	bool revisitable;

	/* SAVer: Expression for annotatable loads. This needs to have
	 * heap-value semantics so that it does not create concurrency issues */
	AnnotVP annotExpr;
};


/*******************************************************************************
 **                         BWaitReadLabel Class
 ******************************************************************************/

/* Specialization of ReadLabel for the read part of a barrier_wait() op */
class BWaitReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	BWaitReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		       SAddr loc, ASize size, AType type, Event rf = Event::getBottom(),
		       AnnotVP annot = nullptr)
		: ReadLabel(EL_BWaitRead, st, ord, pos, loc, size, type, rf, std::move(annot)) {}
	BWaitReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr loc, ASize size, AType type,
		       Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: ReadLabel(EL_BWaitRead, ord, pos, loc, size, type, rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<BWaitReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<BWaitReadLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<BWaitReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_BWaitRead; }
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

	FaiReadLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     SAddr addr, ASize size, AType type, llvm::AtomicRMWInst::BinOp op,
		     SVal val, Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: ReadLabel(k, st, ord, pos, addr, size, type, rf, std::move(annot)),
		  binOp(op), opValue(val) {}
	FaiReadLabel(EventLabelKind k, llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size,
		     AType type, llvm::AtomicRMWInst::BinOp op, SVal val, Event rf = Event::getBottom(),
		     AnnotVP annot = nullptr)
		: ReadLabel(k, ord, pos, addr, size, type, rf, std::move(annot)),
		  binOp(op), opValue(val) {}

public:
	FaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos, SAddr addr,
		     ASize size, AType type, llvm::AtomicRMWInst::BinOp op, SVal val,
		     Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: FaiReadLabel(EL_FaiRead, st, ord, pos, addr, size, type,
			       op, val, rf, std::move(annot)) {}
	FaiReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, Event rf = Event::getBottom(),
		     AnnotVP annot = nullptr)
		: FaiReadLabel(EL_FaiRead, ord, pos, addr, size, type,
			       op, val, rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<FaiReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<FaiReadLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the type of this RMW operation (e.g., add, sub) */
	llvm::AtomicRMWInst::BinOp getOp() const { return binOp; }

	/* Returns the other operand's value */
	SVal getOpVal() const { return opValue; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<FaiReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_FaiRead && k <= EL_FaiReadLast; }

private:
	/* The binary operator for this RMW operation */
	const llvm::AtomicRMWInst::BinOp binOp;

	/* The other operand's value for the operation */
	SVal opValue;
};


/*******************************************************************************
 **                         BIncFaiReadLabel Class
 ******************************************************************************/

/* Specialization of FaiReadLabel for FAI reads that do not return a value */
class NoRetFaiReadLabel : public FaiReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	NoRetFaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos, SAddr addr,
			  ASize size, AType type, llvm::AtomicRMWInst::BinOp op, SVal val,
			  Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: FaiReadLabel(EL_NoRetFaiRead, st, ord, pos, addr, size, type,
			       op, val, rf, std::move(annot)) {}
	NoRetFaiReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size,
			  AType type, llvm::AtomicRMWInst::BinOp op, SVal val,
			  Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: FaiReadLabel(EL_NoRetFaiRead, ord, pos, addr, size, type, op, val,
			       rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<NoRetFaiReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<NoRetFaiReadLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<NoRetFaiReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_NoRetFaiRead; }
};


/*******************************************************************************
 **                         BIncFaiReadLabel Class
 ******************************************************************************/

/* Specialization of FaiReadLabel for barrier FAIs */
class BIncFaiReadLabel : public FaiReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	BIncFaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos, SAddr addr,
			 ASize size, AType type, llvm::AtomicRMWInst::BinOp op, SVal val,
			 Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: FaiReadLabel(EL_BIncFaiRead, st, ord, pos, addr, size,
			       type, op, val, rf, std::move(annot)) {}
	BIncFaiReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size, AType type,
			 llvm::AtomicRMWInst::BinOp op, SVal val, Event rf = Event::getBottom(),
			 AnnotVP annot = nullptr)
		: FaiReadLabel(EL_BIncFaiRead, ord, pos, addr, size,
			       type, op, val, rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<BIncFaiReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<BIncFaiReadLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<BIncFaiReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_BIncFaiRead; }
};


/*******************************************************************************
 **                         CasReadLabel Class
 ******************************************************************************/

/* Represents the read part of a compare-and-swap (CAS) operation */
class CasReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	CasReadLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     SAddr addr, ASize size, AType type, SVal exp, SVal swap,
		     Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: ReadLabel(k, st, ord, pos, addr, size, type, rf, std::move(annot)),
		  expected(exp), swapValue(swap) {}
	CasReadLabel(EventLabelKind k, llvm::AtomicOrdering ord, Event pos,
		     SAddr addr, ASize size, AType type, SVal exp, SVal swap,
		     Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: ReadLabel(k, ord, pos, addr, size, type, rf, std::move(annot)),
		  expected(exp), swapValue(swap) {}

public:
	CasReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     SAddr addr, ASize size, AType type, SVal exp, SVal swap,
		     Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: CasReadLabel(EL_CasRead, st, ord, pos, addr, size, type,
			       exp, swap, rf, std::move(annot)) {}
	CasReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size,
		     AType type, SVal exp, SVal swap, Event rf = Event::getBottom(),
		     AnnotVP annot = nullptr)
		: CasReadLabel(EL_CasRead, ord, pos, addr, size, type,
			       exp, swap, rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<CasReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<CasReadLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the value that will make this CAS succeed */
	SVal getExpected() const { return expected; }

	/* Returns the value that will be written is the CAS succeeds */
	SVal getSwapVal() const { return swapValue; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<CasReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_CasRead && k <= EL_CasReadLast; }

private:
	/* The value that will make this CAS succeed */
	const SVal expected;

	/* The value that will be written if the CAS succeeds */
	const SVal swapValue;
};


/*******************************************************************************
 **                         LockCasReadLabel Class
 ******************************************************************************/

/* Specialization of CasReadLabel for lock CASes */
class LockCasReadLabel : public CasReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	LockCasReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			 SAddr addr, ASize size, AType type, SVal exp, SVal swap,
			 Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: CasReadLabel(EL_LockCasRead, st, ord, pos, addr, size,
			       type, exp, swap, rf, std::move(annot)) {}
	LockCasReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size,
			 AType type, SVal exp, SVal swap, Event rf = Event::getBottom(),
			 AnnotVP annot = nullptr)
		: CasReadLabel(EL_LockCasRead, ord, pos, addr, size,
			       type, exp, swap, rf, std::move(annot)) {}

	LockCasReadLabel(unsigned int st, Event pos, SAddr addr, ASize size,
			 Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: LockCasReadLabel(st, llvm::AtomicOrdering::Acquire, pos, addr, size,
				   AType::Signed, SVal(0), SVal(1), rf, std::move(annot)) {}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, Event rf = Event::getBottom(),
			 AnnotVP annot = nullptr)
		: LockCasReadLabel(llvm::AtomicOrdering::Acquire, pos, addr, size,
				   AType::Signed, SVal(0), SVal(1), rf, std::move(annot)) {}


	template<typename... Ts>
	static std::unique_ptr<LockCasReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LockCasReadLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LockCasReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LockCasRead; }
};


/*******************************************************************************
 **                         TrylockCasReadLabel Class
 ******************************************************************************/

/* Specialization of CasReadLabel for trylock CASes */
class TrylockCasReadLabel : public CasReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	TrylockCasReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			    SAddr addr, ASize size, AType type, SVal exp, SVal swap,
			    Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: CasReadLabel(EL_TrylockCasRead, st, ord, pos, addr, size, type,
			       exp, swap, rf, std::move(annot)) {}
	TrylockCasReadLabel(llvm::AtomicOrdering ord, Event pos,
			    SAddr addr, ASize size, AType type, SVal exp, SVal swap,
			    Event rf = Event::getBottom(),
			    AnnotVP annot = nullptr)
		: CasReadLabel(EL_TrylockCasRead, ord, pos, addr, size, type,
			       exp, swap, rf, std::move(annot)) {}

	TrylockCasReadLabel(unsigned int st, Event pos, SAddr addr, ASize size,
			    Event rf = Event::getBottom(), AnnotVP annot = nullptr)
		: TrylockCasReadLabel(st, llvm::AtomicOrdering::Acquire, pos, addr, size,
				      AType::Signed, SVal(0), SVal(1), rf, std::move(annot)) {}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, Event rf = Event::getBottom(),
			    AnnotVP annot = nullptr)
		: TrylockCasReadLabel(llvm::AtomicOrdering::Acquire, pos, addr, size,
				      AType::Signed, SVal(0), SVal(1), rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<TrylockCasReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<TrylockCasReadLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<TrylockCasReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_TrylockCasRead; }
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
		     SAddr loc, ASize size, AType type, Event rf = Event::getBottom())
		: ReadLabel(EL_DskRead, st, ord, pos, loc, size, type, rf),
		  DskAccessLabel(EL_DskRead) {}
	DskReadLabel(llvm::AtomicOrdering ord, Event pos, SAddr loc,
		     ASize size, AType type, Event rf = Event::getBottom())
		: ReadLabel(EL_DskRead, ord, pos, loc, size, type, rf),
		  DskAccessLabel(EL_DskRead) {}

	DskReadLabel(unsigned int st, Event pos, SAddr loc, ASize size, AType type)
		: DskReadLabel(st, llvm::AtomicOrdering::Acquire, pos, loc, size, type) {}
	DskReadLabel(Event pos, SAddr loc, ASize size, AType type)
		: DskReadLabel(llvm::AtomicOrdering::Acquire, pos, loc, size, type) {}

	template<typename... Ts>
	static std::unique_ptr<DskReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskReadLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskRead; }
	static DskAccessLabel *castToDskAccessLabel(const DskReadLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskReadLabel*>(D));
	}
	static DskReadLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskReadLabel *>(const_cast<DskAccessLabel*>(DC));
	}
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
		   Event pos, SAddr addr, ASize size, AType type, SVal val)
		: MemAccessLabel(k, st, ord, pos, addr, size, type), value(val) {}
	WriteLabel(EventLabelKind k, llvm::AtomicOrdering ord,
		   Event pos, SAddr addr, ASize size, AType type, SVal val)
		: MemAccessLabel(k, ord, pos, addr, size, type), value(val) {}

public:
	WriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		   SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(EL_Write, st, ord, pos, addr, size, type, val) {}
	WriteLabel(llvm::AtomicOrdering ord, Event pos,
		   SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(EL_Write, ord, pos, addr, size, type, val) {}

	template<typename... Ts>
	static std::unique_ptr<WriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<WriteLabel>(std::forward<Ts>(params)...);
	}

	/* Getter/setter for the write value */
	SVal getVal() const { return value; }
	void setVal(SVal v) { value = v; }

	/* Returns a list of the reads reading from this write */
	const std::vector<Event>& getReadersList() const { return readerList; }

	/* Iterators for readers */
	using const_iterator = std::vector<Event>::const_iterator;
	const_iterator readers_begin() const { return readerList.begin(); }
	const_iterator readers_end() const { return readerList.end(); }

	/* Getter/setter for a view representing the
	 * release sequence of this write */
	const View& getMsgView() const { return msgView; }
	void setMsgView(View &&v) { msgView = std::move(v); }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<WriteLabel>(*this);
	}

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
	SVal value;

	/* View for the release sequence of the write */
	View msgView;

	/* List of reads reading from the write */
	std::vector<Event> readerList;
};


/*******************************************************************************
 **                         UnlockWriteLabel Class
 ******************************************************************************/

/* Specialization of writes for unlock events */
class UnlockWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	UnlockWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			 SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(EL_UnlockWrite, st, ord, pos, addr, size, type, val) {}
	UnlockWriteLabel(llvm::AtomicOrdering ord, Event pos,
			 SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(EL_UnlockWrite, ord, pos, addr, size, type, val) {}

	UnlockWriteLabel(Event pos, SAddr addr, ASize size)
		: UnlockWriteLabel(llvm::AtomicOrdering::Release, pos, addr, size,
				   AType::Signed, SVal(0)) {}

	template<typename... Ts>
	static std::unique_ptr<UnlockWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<UnlockWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<UnlockWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_UnlockWrite; }
};


/*******************************************************************************
 **                         BInitWriteLabel Class
 ******************************************************************************/

/* Specialization of writes for barrier initializations */
class BInitWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	BInitWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(EL_BInitWrite, st, ord, pos, addr, size, type, val) {}
	BInitWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
			ASize size, AType type, SVal val)
		: WriteLabel(EL_BInitWrite, ord, pos, addr, size, type, val) {}

	template<typename... Ts>
	static std::unique_ptr<BInitWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<BInitWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<BInitWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_BInitWrite; }
};


/*******************************************************************************
 **                         BDestroyWriteLabel Class
 ******************************************************************************/

/* Specialization of writes for barrier destruction */
class BDestroyWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	BDestroyWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			   SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(EL_BDestroyWrite, st, ord, pos, addr, size, type, val) {}
	BDestroyWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
			   ASize size, AType type, SVal val)
		: WriteLabel(EL_BDestroyWrite, ord, pos, addr, size, type, val) {}

	template<typename... Ts>
	static std::unique_ptr<BDestroyWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<BDestroyWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<BDestroyWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_BDestroyWrite; }
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

	FaiWriteLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(k, st, ord, pos, addr, size, type, val) {}
	FaiWriteLabel(EventLabelKind k, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(k, ord, pos, addr, size, type, val) {}

public:
	FaiWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val)
		: FaiWriteLabel(EL_FaiWrite, st, ord, pos, addr, size, type, val) {}
	FaiWriteLabel(llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val)
		: FaiWriteLabel(EL_FaiWrite, ord, pos, addr, size, type, val) {}

	template<typename... Ts>
	static std::unique_ptr<FaiWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<FaiWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<FaiWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_FaiWrite && k <= EL_FaiWriteLast; }
};


/*******************************************************************************
 **                         NoRetFaiWriteLabel Class
 ******************************************************************************/

/* Specialization of FaiWriteLabel for non-value-returning FAIs */
class NoRetFaiWriteLabel : public FaiWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	NoRetFaiWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			   SAddr addr, ASize size, AType type, SVal val)
		: FaiWriteLabel(EL_NoRetFaiWrite, st, ord, pos, addr, size, type, val) {}
	NoRetFaiWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
			   ASize size, AType type, SVal val)
		: FaiWriteLabel(EL_NoRetFaiWrite, ord, pos, addr, size, type, val) {}

	template<typename... Ts>
	static std::unique_ptr<NoRetFaiWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<NoRetFaiWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<NoRetFaiWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_NoRetFaiWrite; }
};


/*******************************************************************************
 **                         BIncFaiWriteLabel Class
 ******************************************************************************/

/* Specialization of FaiWriteLabel for barrier FAIs */
class BIncFaiWriteLabel : public FaiWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	BIncFaiWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			  SAddr addr, ASize size, AType type, SVal val)
		: FaiWriteLabel(EL_BIncFaiWrite, st, ord, pos, addr, size, type, val) {}
	BIncFaiWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
			  ASize size, AType type, SVal val)
		: FaiWriteLabel(EL_BIncFaiWrite, ord, pos, addr, size, type, val) {}

	template<typename... Ts>
	static std::unique_ptr<BIncFaiWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<BIncFaiWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<BIncFaiWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_BIncFaiWrite; }
};


/*******************************************************************************
 **                         CasWriteLabel Class
 ******************************************************************************/

/* Represents the write part of a compare-and-swap (CAS) operation */
class CasWriteLabel : public WriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	CasWriteLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(k, st, ord, pos, addr, size, type, val) {}
	CasWriteLabel(EventLabelKind k, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val)
		: WriteLabel(k, ord, pos, addr, size, type, val) {}

public:
	CasWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val)
		: CasWriteLabel(EL_CasWrite, st, ord, pos, addr, size, type, val) {}
	CasWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
		      ASize size, AType type, SVal val)
		: CasWriteLabel(EL_CasWrite, ord, pos, addr, size, type, val) {}

	template<typename... Ts>
	static std::unique_ptr<CasWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<CasWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<CasWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_CasWrite && k <= EL_CasWriteLast; }
};


/*******************************************************************************
 **                         LockCasWriteLabel Class
 ******************************************************************************/

/* Specialization of CasWriteLabel for lock CASes */
class LockCasWriteLabel : public CasWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	LockCasWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			  SAddr addr, ASize size, AType type, SVal val)
		: CasWriteLabel(EL_LockCasWrite, st, ord, pos, addr, size, type, val) {}
	LockCasWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
			  ASize size, AType type, SVal val)
		: CasWriteLabel(EL_LockCasWrite, ord, pos, addr, size, type, val) {}

	LockCasWriteLabel(unsigned int st, Event pos, SAddr addr, ASize size)
		: LockCasWriteLabel(st, llvm::AtomicOrdering::Acquire,
				    pos, addr, size, AType::Signed, SVal(1)) {}
	LockCasWriteLabel(Event pos, SAddr addr, ASize size)
		: LockCasWriteLabel(llvm::AtomicOrdering::Acquire, pos,
				    addr, size, AType::Signed, SVal(1)) {}

	template<typename... Ts>
	static std::unique_ptr<LockCasWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LockCasWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LockCasWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LockCasWrite; }
};


/*******************************************************************************
 **                         TrylockCasWriteLabel Class
 ******************************************************************************/

/* Specialization of CasWriteLabel for trylock CASes */
class TrylockCasWriteLabel : public CasWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	TrylockCasWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			     SAddr addr, ASize size, AType type, SVal val)
		: CasWriteLabel(EL_TrylockCasWrite, st, ord, pos, addr, size, type, val) {}
	TrylockCasWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
			     ASize size, AType type, SVal val)
		: CasWriteLabel(EL_TrylockCasWrite, ord, pos, addr, size, type, val) {}

	TrylockCasWriteLabel(unsigned int st, Event pos, SAddr addr, ASize size)
		: TrylockCasWriteLabel(st, llvm::AtomicOrdering::Acquire,
				       pos, addr, size, AType::Signed, SVal(1)) {}
	TrylockCasWriteLabel(Event pos, SAddr addr, ASize size)
		: TrylockCasWriteLabel(llvm::AtomicOrdering::Acquire, pos,
				       addr, size, AType::Signed, SVal(1)) {}


	template<typename... Ts>
	static std::unique_ptr<TrylockCasWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<TrylockCasWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<TrylockCasWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_TrylockCasWrite; }
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
		      Event pos,  SAddr addr, ASize size, AType type, SVal val, void *mapping)
		: WriteLabel(k, st, ord, pos, addr, size, type, val),
		  DskAccessLabel(k), mapping(mapping) {}
	DskWriteLabel(EventLabelKind k, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val, void *mapping)
		: WriteLabel(k, ord, pos, addr, size, type, val),
		  DskAccessLabel(k), mapping(mapping) {}

public:
	DskWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      SAddr addr, ASize size, AType type, SVal val, void *mapping)
		: DskWriteLabel(EL_DskWrite, st, ord, pos, addr, size, type, val, mapping) {}
	DskWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr,
		      ASize size, AType type, SVal val, void *mapping)
		: DskWriteLabel(EL_DskWrite, ord, pos, addr, size, type, val, mapping) {}

	DskWriteLabel(unsigned int st, Event pos, SAddr addr, ASize size, AType type,
		      SVal val, void *mapping)
		: DskWriteLabel(st, llvm::AtomicOrdering::Release, pos, addr, size, type, val, mapping) {}
	DskWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val, void *mapping)
		: DskWriteLabel(llvm::AtomicOrdering::Release, pos, addr, size, type, val, mapping) {}

	template<typename... Ts>
	static std::unique_ptr<DskWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskWriteLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the starting offset for this write's disk mapping */
	const void *getMapping() const { return mapping; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskWriteLabel>(*this);
	}

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
			SAddr addr, ASize size, AType type, SVal val, void *mapping,
			std::pair<void *, void*> ordDataRange)
		: DskWriteLabel(EL_DskMdWrite, st, ord, pos, addr, size, type, val, mapping),
		  ordDataRange(ordDataRange) {}
	DskMdWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size,
			AType type, SVal val, void *mapping,
			std::pair<void *, void*> ordDataRange)
		: DskWriteLabel(EL_DskMdWrite, ord, pos, addr, size, type, val, mapping),
		  ordDataRange(ordDataRange) {}

	DskMdWriteLabel(unsigned int st, Event pos, SAddr addr, ASize size, AType type,
			SVal val, void *mapping, std::pair<void *, void*> ordDataRange)
		: DskWriteLabel(EL_DskMdWrite, st, llvm::AtomicOrdering::Release, pos,
				addr, size, type, val, mapping), ordDataRange(ordDataRange) {}
	DskMdWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val,
			void *mapping, std::pair<void *, void*> ordDataRange)
		: DskWriteLabel(EL_DskMdWrite, llvm::AtomicOrdering::Release, pos, addr,
				size, type, val, mapping), ordDataRange(ordDataRange) {}

	template<typename... Ts>
	static std::unique_ptr<DskMdWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskMdWriteLabel>(std::forward<Ts>(params)...);
	}

	/* Helpers that return data with which this write is ordered */
	const void *getOrdDataBegin() const { return ordDataRange.first; }
	const void *getOrdDataEnd() const { return ordDataRange.second; }
	const std::pair<void *, void *>
	getOrdDataRange() const { return ordDataRange; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskMdWriteLabel>(*this);
	}

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
			 SAddr addr, ASize size, AType type, SVal val, void *mapping)
		: DskWriteLabel(EL_DskDirWrite, st, ord, pos, addr, size, type, val, mapping) {}
	DskDirWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size,
			 AType type, SVal val, void *mapping)
		: DskWriteLabel(EL_DskDirWrite, ord, pos, addr, size, type, val, mapping) {}

	DskDirWriteLabel(unsigned int st, Event pos, SAddr addr, ASize size,
			 AType type, SVal val, void *mapping)
		: DskWriteLabel(EL_DskDirWrite, st, llvm::AtomicOrdering::Release, pos,
				addr, size, type, val, mapping) {}
	DskDirWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val, void *mapping)
		: DskWriteLabel(EL_DskDirWrite, llvm::AtomicOrdering::Release, pos, addr,
				size, type, val, mapping) {}

	template<typename... Ts>
	static std::unique_ptr<DskDirWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskDirWriteLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskDirWriteLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskDirWrite; }
	static DskAccessLabel *castToDskAccessLabel(const DskDirWriteLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskDirWriteLabel*>(D));
	}
	static DskDirWriteLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskDirWriteLabel *>(const_cast<DskAccessLabel*>(DC));
	}
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
			 SAddr addr, ASize size, AType type, SVal val, void *mapping, void *inode)
		: DskWriteLabel(EL_DskJnlWrite, st, ord, pos, addr, size, type, val, mapping),
		  inode(inode) {}
	DskJnlWriteLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, ASize size,
			 AType type, SVal val, void *mapping, void *inode)
		: DskWriteLabel(EL_DskJnlWrite, ord, pos, addr, size, type, val, mapping),
		  inode(inode) {}

	DskJnlWriteLabel(unsigned int st, Event pos, SAddr addr, ASize size, AType type,
			 SVal val, void *mapping, void *inode)
		: DskWriteLabel(EL_DskJnlWrite, st, llvm::AtomicOrdering::Release,
				pos, addr, size, type, val, mapping), inode(inode) {}
	DskJnlWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val,
			 void *mapping, void *inode)
		: DskWriteLabel(EL_DskJnlWrite, llvm::AtomicOrdering::Release, pos, addr,
				size, type, val, mapping), inode(inode) {}

	template<typename... Ts>
	static std::unique_ptr<DskJnlWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskJnlWriteLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the inode on which the transaction takes place */
	const void *getTransInode() const { return inode; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskJnlWriteLabel>(*this);
	}

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
	FenceLabel(EventLabelKind k, llvm::AtomicOrdering ord, Event pos)
		: EventLabel(k, ord, pos) {}
public:
	FenceLabel(llvm::AtomicOrdering ord, Event pos)
		: FenceLabel(EL_Fence, ord, pos) {}

	template<typename... Ts>
	static std::unique_ptr<FenceLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<FenceLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<FenceLabel>(*this);
	}

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
		: FenceLabel(EL_DskFsync, st, ord, pos),
		  DskAccessLabel(EL_DskFsync), inode(inode), size(size) {}
	DskFsyncLabel(llvm::AtomicOrdering ord, Event pos, const void *inode, unsigned int size)
		: FenceLabel(EL_DskFsync, ord, pos),
		  DskAccessLabel(EL_DskFsync), inode(inode), size(size) {}

	DskFsyncLabel(unsigned int st, Event pos, const void *inode, unsigned int size)
		: DskFsyncLabel(st, llvm::AtomicOrdering::Release, pos, inode, size) {}
	DskFsyncLabel(Event pos, const void *inode, unsigned int size)
		: DskFsyncLabel(llvm::AtomicOrdering::Release, pos, inode, size) {}

	template<typename... Ts>
	static std::unique_ptr<DskFsyncLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskFsyncLabel>(std::forward<Ts>(params)...);
	}

	/* Returns a pointer to the inode on which the fsync() took place */
	const void *getInode() const { return inode; }

	/* Returns the "size" of this fsync()'s range */
	unsigned int getSize() const { return size; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskFsyncLabel>(*this);
	}

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
	DskSyncLabel(llvm::AtomicOrdering ord, Event pos)
		: FenceLabel(EL_DskSync, ord, pos), DskAccessLabel(EL_DskSync) {}

	DskSyncLabel(unsigned int st, Event pos)
		: DskSyncLabel(st, llvm::AtomicOrdering::Release, pos) {}
	DskSyncLabel(Event pos)
		: DskSyncLabel(llvm::AtomicOrdering::Release, pos) {}

	template<typename... Ts>
	static std::unique_ptr<DskSyncLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskSyncLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskSyncLabel>(*this);
	}

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
	DskPbarrierLabel(llvm::AtomicOrdering ord, Event pos)
		: FenceLabel(EL_DskPbarrier, ord, pos), DskAccessLabel(EL_DskPbarrier) {}

	DskPbarrierLabel(unsigned int st, Event pos)
		: DskPbarrierLabel(st, llvm::AtomicOrdering::Release, pos) {}
	DskPbarrierLabel(Event pos)
		: DskPbarrierLabel(llvm::AtomicOrdering::Release, pos) {}

	template<typename... Ts>
	static std::unique_ptr<DskPbarrierLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskPbarrierLabel>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskPbarrierLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskPbarrier; }
	static DskAccessLabel *castToDskAccessLabel(const DskPbarrierLabel *D) {
		return static_cast<DskAccessLabel *>(const_cast<DskPbarrierLabel*>(D));
	}
	static DskPbarrierLabel *castFromDskAccessLabel(const DskAccessLabel *DC) {
		return static_cast<DskPbarrierLabel *>(const_cast<DskAccessLabel*>(DC));
	}
};


/*******************************************************************************
 **                         SmpFenceKLMMLabel Class
 ******************************************************************************/

enum class SmpFenceType {
	MB = 0, WMB, RMB, MBBA, MBAA, MBAS, MBAUL
};
inline bool isCumul(SmpFenceType t) { return t <= SmpFenceType::WMB || t >= SmpFenceType::MBBA; }
inline bool isStrong(SmpFenceType t) { return t == SmpFenceType::MB || t >= SmpFenceType::MBBA; }

/* Represents a non-C11-type fence (LKMM only) */
class SmpFenceLabelLKMM : public FenceLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	SmpFenceLabelLKMM(unsigned int st, llvm::AtomicOrdering ord, SmpFenceType t, Event pos)
		: FenceLabel(EL_SmpFenceLKMM, st, ::isStrong(t) ?
			     llvm::AtomicOrdering::SequentiallyConsistent :
			     llvm::AtomicOrdering::Monotonic, pos), type(t) {}
	SmpFenceLabelLKMM(llvm::AtomicOrdering ord, SmpFenceType t, Event pos)
		: FenceLabel(EL_SmpFenceLKMM, ::isStrong(t) ?
			     llvm::AtomicOrdering::SequentiallyConsistent :
			     llvm::AtomicOrdering::Monotonic, pos), type(t) {}

	template<typename... Ts>
	static std::unique_ptr<SmpFenceLabelLKMM> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<SmpFenceLabelLKMM>(std::forward<Ts>(params)...);
	}

	/* Returns the type of this fence */
	SmpFenceType getType() const { return type; }

	/* Returns true if this fence is cumulative */
	bool isCumul() const { return ::isCumul(getType()); }

	/* Returns true if this fence is a strong fence */
	bool isStrong() const { return ::isStrong(getType()); }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<SmpFenceLabelLKMM>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_SmpFenceLKMM; }

private:
	/* The type of this LKMM fence */
	SmpFenceType type;
};


/******************************************************************************
 **                        RCUSyncLabelLKMM Class
 ******************************************************************************/

/* Corresponds to a the beginning of a grace period */
class RCUSyncLabelLKMM : public FenceLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	RCUSyncLabelLKMM(unsigned int st, Event pos)
		: FenceLabel(EL_RCUSyncLKMM, st, llvm::AtomicOrdering::SequentiallyConsistent, pos) {}
	RCUSyncLabelLKMM(Event pos)
		: FenceLabel(EL_RCUSyncLKMM, llvm::AtomicOrdering::SequentiallyConsistent, pos) {}

	template<typename... Ts>
	static std::unique_ptr<RCUSyncLabelLKMM> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<RCUSyncLabelLKMM>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<RCUSyncLabelLKMM>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_RCUSyncLKMM; }

private:
	/* The discriminator suffices */
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
	ThreadCreateLabel(llvm::AtomicOrdering ord, Event pos, unsigned int cid)
		: EventLabel(EL_ThreadCreate, ord, pos), childId(cid) {}

	ThreadCreateLabel(unsigned int st, Event pos, unsigned int cid)
		: ThreadCreateLabel(st, llvm::AtomicOrdering::Release, pos, cid) {}
	ThreadCreateLabel(Event pos, unsigned int cid)
		: ThreadCreateLabel(llvm::AtomicOrdering::Release, pos, cid) {}

	ThreadCreateLabel(unsigned int st, Event pos)
		: ThreadCreateLabel(st, pos, -1) {}
	ThreadCreateLabel(Event pos)
		: ThreadCreateLabel(pos, -1) {}


	template<typename... Ts>
	static std::unique_ptr<ThreadCreateLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<ThreadCreateLabel>(std::forward<Ts>(params)...);
	}

	/* Getter/setter for the created thread's identifier */
	unsigned int getChildId() const { return childId; }
	void setChildId(unsigned int cid)  { childId = cid; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<ThreadCreateLabel>(*this);
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
	ThreadJoinLabel(llvm::AtomicOrdering ord, Event pos, unsigned int childId)
		: EventLabel(EL_ThreadJoin, ord, pos),
		  childId(childId), childLast(Event::getInitializer()) {}

	ThreadJoinLabel(unsigned int st, Event pos, unsigned int childId)
		: ThreadJoinLabel(st, llvm::AtomicOrdering::Acquire, pos, childId) {}
	ThreadJoinLabel(Event pos, unsigned int childId)
		: ThreadJoinLabel(llvm::AtomicOrdering::Acquire, pos, childId) {}

	template<typename... Ts>
	static std::unique_ptr<ThreadJoinLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<ThreadJoinLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the identifier of the thread this join() is waiting on */
	unsigned int getChildId() const { return childId; }

	/* Returns the last event of the thread the join() is waiting on */
	Event getChildLast() const { return childLast; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<ThreadJoinLabel>(*this);
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
	ThreadStartLabel(llvm::AtomicOrdering ord, Event pos, Event pc, int symm = -1)
		: EventLabel(EL_ThreadStart, ord, pos), parentCreate(pc), symmetricTid(symm) {}

	ThreadStartLabel(unsigned int st, Event pos, Event pc, int symm = -1)
		: ThreadStartLabel(st, llvm::AtomicOrdering::Acquire, pos, pc, symm) {}
	ThreadStartLabel(Event pos, Event pc, int symm = -1)
		: ThreadStartLabel(llvm::AtomicOrdering::Acquire, pos, pc, symm) {}

	template<typename... Ts>
	static std::unique_ptr<ThreadStartLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<ThreadStartLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the position of the corresponding create operation */
	Event getParentCreate() const { return parentCreate; }

	/* SR: Returns the id of a symmetric thread, or -1 if no symmetric thread exists  */
	int getSymmetricTid() const { return symmetricTid; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<ThreadStartLabel>(*this);
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
	ThreadFinishLabel(llvm::AtomicOrdering ord, Event pos)
		: EventLabel(EL_ThreadFinish, ord, pos), parentJoin(Event::getInitializer()) {}

	ThreadFinishLabel(unsigned int st, Event pos)
		: ThreadFinishLabel(st, llvm::AtomicOrdering::Release, pos) {}
	ThreadFinishLabel(Event pos)
		: ThreadFinishLabel(llvm::AtomicOrdering::Release, pos) {}

	template<typename... Ts>
	static std::unique_ptr<ThreadFinishLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<ThreadFinishLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the join() operation waiting on this thread (or the
	 * initializer event, if no such operation exists) */
	Event getParentJoin() const { return parentJoin; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<ThreadFinishLabel>(*this);
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
		    SAddr addr, unsigned int size, NameInfo *info, const std::string &name)
		: EventLabel(EL_Malloc, st, ord, pos),
		  allocAddr(addr), allocSize(size), nameInfo(info), name(name) {}
	MallocLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, unsigned int size,
		    NameInfo *info, const std::string &name)
		: EventLabel(EL_Malloc, ord, pos),
		  allocAddr(addr), allocSize(size), nameInfo(info), name(name) {}

	MallocLabel(unsigned int st, Event pos, SAddr addr, unsigned int size,
		    NameInfo *info = nullptr, const std::string &name = {})
		: MallocLabel(st, llvm::AtomicOrdering::NotAtomic, pos,
			      addr, size, info, name) {}
	MallocLabel(Event pos, SAddr addr, unsigned int size,
		    NameInfo *info = nullptr, const std::string &name = {})
		: MallocLabel(llvm::AtomicOrdering::NotAtomic, pos,
			      addr, size, info, name) {}

	MallocLabel(unsigned int st, Event pos, unsigned int size,
		    NameInfo *info = nullptr, const std::string &name = {})
		: MallocLabel(st, pos, SAddr(), size, info, name) {}
	MallocLabel(Event pos, unsigned int size,
		    NameInfo *info = nullptr, const std::string &name = {})
		: MallocLabel(pos, SAddr(), size, info, name) {}

	template<typename... Ts>
	static std::unique_ptr<MallocLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<MallocLabel>(std::forward<Ts>(params)...);
	}

	/* Getter/setter for the (fresh) address returned by the allocation */
	SAddr getAllocAddr() const { return allocAddr; }
	void setAllocAddr(SAddr addr) { allocAddr = addr; }

	/* Returns the size of this allocation */
	unsigned int getAllocSize() const { return allocSize; }

	/* Returns true if ADDR is contained within the allocated block */
	bool contains(SAddr addr) const {
		return getAllocAddr() <= addr && addr < getAllocAddr() + getAllocSize();
	}

	/* Returns the name of the variable allocated */
	const std::string &getName() const { return name; }

	/* Returns the naming info associated with this allocation.
	 * Returns null if no such info is found. */
	const NameInfo *getNameInfo() const { return nameInfo; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<MallocLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Malloc; }

private:
	/* The address returned by malloc() */
	SAddr allocAddr;

	/* The size of the requested allocation */
	unsigned int allocSize;

	/* Name of the variable allocated */
	std::string name;

	/* Naming information for this allocation */
	NameInfo *nameInfo;
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
		  SAddr addr, unsigned int size = 0)
		: EventLabel(EL_Free, st, ord, pos), freeAddr(addr), freedSize(size) {}
	FreeLabel(llvm::AtomicOrdering ord, Event pos, SAddr addr, unsigned int size = 0)
		: EventLabel(EL_Free, ord, pos), freeAddr(addr), freedSize(size) {}

	FreeLabel(unsigned int st, Event pos, SAddr addr, unsigned int size = 0)
		: FreeLabel(st, llvm::AtomicOrdering::NotAtomic, pos, addr, size) {}
	FreeLabel(Event pos, SAddr addr, unsigned int size = 0)
		: FreeLabel(llvm::AtomicOrdering::NotAtomic, pos, addr, size) {}


	template<typename... Ts>
	static std::unique_ptr<FreeLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<FreeLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the address being freed */
	SAddr getFreedAddr() const { return freeAddr; }

	/* Getter/setter for the size of the memory freed */
	unsigned int getFreedSize() const { return freedSize; }
	void setFreedSize(unsigned int size) { freedSize = size; }

	/* Returns true if ADDR is contained within the deallocated block */
	bool contains(SAddr addr) const {
		return getFreedAddr() <= addr && addr < getFreedAddr() + getFreedSize();
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<FreeLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Free; }

private:
	/* The address of the memory freed */
	SAddr freeAddr;

	/* The size of the memory freed */
	unsigned int freedSize;
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
	LockLabelLAPOR(unsigned int st, llvm::AtomicOrdering ord, Event pos, SAddr addr)
		: EventLabel(EL_LockLabelLAPOR, st, ord, pos), lockAddr(addr) {}
	LockLabelLAPOR(llvm::AtomicOrdering ord, Event pos, SAddr addr)
		: EventLabel(EL_LockLabelLAPOR, ord, pos), lockAddr(addr) {}

	LockLabelLAPOR(unsigned int st, Event pos, SAddr addr)
		: LockLabelLAPOR(st, llvm::AtomicOrdering::Acquire, pos, addr) {}
	LockLabelLAPOR(Event pos, SAddr addr)
		: LockLabelLAPOR(llvm::AtomicOrdering::Acquire, pos, addr) {}

	template<typename... Ts>
	static std::unique_ptr<LockLabelLAPOR> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LockLabelLAPOR>(std::forward<Ts>(params)...);
	}

	/* Returns the address of the acquired lock */
	SAddr getLockAddr() const { return lockAddr; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LockLabelLAPOR>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LockLabelLAPOR; }

private:
	/* The address of the acquired lock */
	SAddr lockAddr;
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
	UnlockLabelLAPOR(unsigned int st, llvm::AtomicOrdering ord, Event pos, SAddr addr)
		: EventLabel(EL_UnlockLabelLAPOR, st, ord, pos), lockAddr(addr) {}
	UnlockLabelLAPOR(llvm::AtomicOrdering ord, Event pos, SAddr addr)
		: EventLabel(EL_UnlockLabelLAPOR, ord, pos), lockAddr(addr) {}

	UnlockLabelLAPOR(unsigned int st, Event pos, SAddr addr)
		: UnlockLabelLAPOR(st, llvm::AtomicOrdering::Release, pos, addr) {}
	UnlockLabelLAPOR(Event pos, SAddr addr)
		: UnlockLabelLAPOR(llvm::AtomicOrdering::Release, pos, addr) {}

	template<typename... Ts>
	static std::unique_ptr<UnlockLabelLAPOR> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<UnlockLabelLAPOR>(std::forward<Ts>(params)...);
	}

	/* Returns the address of the released lock */
	SAddr getLockAddr() const { return lockAddr; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<UnlockLabelLAPOR>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_UnlockLabelLAPOR; }

private:
	/* The address of the released lock */
	SAddr lockAddr;
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
		     const std::string &fileName, SVal fd)
		: EventLabel(EL_DskOpen, st, ord, pos),
		  fileName(fileName), fd(fd) {}
	DskOpenLabel(llvm::AtomicOrdering ord, Event pos, const std::string &fileName, SVal fd)
		: EventLabel(EL_DskOpen, ord, pos), fileName(fileName), fd(fd) {}

	DskOpenLabel(unsigned int st, Event pos, const std::string &fileName, SVal fd = SVal(0))
		: DskOpenLabel(st, llvm::AtomicOrdering::Release, pos, fileName, fd) {}
	DskOpenLabel(Event pos, const std::string &fileName, SVal fd = SVal(0))
		: DskOpenLabel(llvm::AtomicOrdering::Release, pos, fileName, fd) {}

	template<typename... Ts>
	static std::unique_ptr<DskOpenLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskOpenLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the name of the opened file */
	const std::string &getFileName() const { return fileName; }

	/* Setter/getter for the file descriptor returned by open() */
	SVal getFd() const { return fd; }
	void setFd(SVal d) { fd = d; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskOpenLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskOpen; }

private:
	/* The name of the opened file */
	std::string fileName;

	/* The file descriptor allocated for this call */
	SVal fd;
};


/******************************************************************************
 **                        RCULockLabelLKMM Class
 ******************************************************************************/

/* Corresponds to the beginning of an RCU read-side critical section */
class RCULockLabelLKMM : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	RCULockLabelLKMM(unsigned int st, Event pos)
		: EventLabel(EL_RCULockLKMM, st, llvm::AtomicOrdering::Acquire, pos) {}
	RCULockLabelLKMM(Event pos)
		: EventLabel(EL_RCULockLKMM, llvm::AtomicOrdering::Acquire, pos) {}

	template<typename... Ts>
	static std::unique_ptr<RCULockLabelLKMM> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<RCULockLabelLKMM>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<RCULockLabelLKMM>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_RCULockLKMM; }

private:
	/* The discriminator suffices */
};


/******************************************************************************
 **                        RCUUnlockLabelLKMM Class
 ******************************************************************************/

/* Corresponds to the ending of an RCU read-side critical section */
class RCUUnlockLabelLKMM : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	RCUUnlockLabelLKMM(unsigned int st, Event pos)
		: EventLabel(EL_RCUUnlockLKMM, st, llvm::AtomicOrdering::Release, pos) {}
	RCUUnlockLabelLKMM(Event pos)
		: EventLabel(EL_RCUUnlockLKMM, llvm::AtomicOrdering::Release, pos) {}

	template<typename... Ts>
	static std::unique_ptr<RCUUnlockLabelLKMM> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<RCUUnlockLabelLKMM>(std::forward<Ts>(params)...);
	}

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<RCUUnlockLabelLKMM>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_RCUUnlockLKMM; }

private:
	/* The discriminator suffices */
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
