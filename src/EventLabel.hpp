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
#include "SExpr.hpp"
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
		EL_LibRead,
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

	virtual ~EventLabel() = default;

	/* Returns a clone object (virtual to allow deep copying from base) */
	virtual std::unique_ptr<EventLabel> clone() const = 0;

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
		: EventLabel(EL_Empty, st, llvm::AtomicOrdering::NotAtomic, pos) {}

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
 **                            LoopBeginLabel Class
 ******************************************************************************/

/* A label that marks the beginning of a spinloop. Used in DSA. */
class LoopBeginLabel : public EventLabel {

public:

	LoopBeginLabel(unsigned int st, Event pos)
		: EventLabel(EL_LoopBegin, st, llvm::AtomicOrdering::NotAtomic, pos) {}

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
		       Event pos, const llvm::GenericValue *loc, const llvm::Type *typ)
		: EventLabel(k, st, ord, pos), addr(loc), valueType(typ) {}
public:
	/* Returns the address of this access */
	const llvm::GenericValue *getAddr() const { return addr; }

	/* Returns the type of the access's value */
	const llvm::Type *getType() const { return valueType; }

	/* Getter/setter for a the fence view of this memory access */
	const DepView& getFenceView() const { return fenceView; }
	void setFenceView(DepView &&v) { fenceView = std::move(v); }

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_MemAccessBegin && k <= EL_MemAccessEnd;
	}

private:
	/* The address of the accessing */
	const llvm::GenericValue *addr;

	/* The type of the value accessed */
	const llvm::Type *valueType;

	/* A view of fences that could be used by some memory models (e.g., LKMM) */
	DepView fenceView;
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

protected:
	ReadLabel(EventLabelKind k, unsigned int st, llvm::AtomicOrdering ord,
		  Event pos, const llvm::GenericValue *loc,
		  const llvm::Type *typ, Event rf, std::unique_ptr<SExpr> annot = nullptr)
		: MemAccessLabel(k, st, ord, pos, loc, typ),
		  readsFrom(rf), revisitable(true), annotExpr(std::move(annot)) {}

public:
	ReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		  const llvm::GenericValue *loc, const llvm::Type *typ,
		  Event rf, std::unique_ptr<SExpr> annot = nullptr)
		: ReadLabel(EL_Read, st, ord, pos, loc, typ, rf, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<ReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<ReadLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the position of the write this read is readinf-from */
	Event getRf() const { return readsFrom; }

	/* Returns true if this read can be revisited */
	bool isRevisitable() const { return revisitable; }

	/* SAVer: Returns the expression with which this load is annotated */
	const SExpr *getAnnot() const { return annotExpr.get(); }

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

	/* Makes the relevant read revisitable/non-revisitable. The
	 * execution graph is responsible for making such changes */
	void setRevisitStatus(bool status) { revisitable = status; }

	/* Position of the write it is reading from in the graph */
	Event readsFrom;

	/* Revisitability status */
	bool revisitable;

	/* SAVer: Expression for annotatable loads. Shared between clones
	 * for easier copying, but clones will not be revisitable anyway */
	std::shared_ptr<SExpr> annotExpr;
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
		       const llvm::GenericValue *loc, const llvm::Type *typ,
		       Event rf, std::unique_ptr<SExpr> annot = nullptr)
		: ReadLabel(EL_BWaitRead, st, ord, pos, loc, typ, rf, std::move(annot)) {}

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
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     llvm::AtomicRMWInst::BinOp op, llvm::GenericValue val,
		     std::unique_ptr<SExpr> annot = nullptr)
		: ReadLabel(k, st, ord, pos, addr, typ, rf, std::move(annot)),
		  binOp(op), opValue(val) {}

public:
	FaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     llvm::AtomicRMWInst::BinOp op, llvm::GenericValue val,
		     std::unique_ptr<SExpr> annot = nullptr)
		: FaiReadLabel(EL_FaiRead, st, ord, pos, addr, typ, rf, op, val, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<FaiReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<FaiReadLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the type of this RMW operation (e.g., add, sub) */
	llvm::AtomicRMWInst::BinOp getOp() const { return binOp; }

	/* Returns the other operand's value */
	const llvm::GenericValue& getOpVal() const { return opValue; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<FaiReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_FaiRead && k <= EL_FaiReadLast; }

private:
	/* The binary operator for this RMW operation */
	const llvm::AtomicRMWInst::BinOp binOp;

	/* The other operand's value for the operation */
	const llvm::GenericValue opValue;
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
	NoRetFaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			  const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
			  llvm::AtomicRMWInst::BinOp op, llvm::GenericValue val,
			  std::unique_ptr<SExpr> annot = nullptr)
		: FaiReadLabel(EL_NoRetFaiRead, st, ord, pos, addr, typ, rf,
			       op, val, std::move(annot)) {}

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
	BIncFaiReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
			 const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
			 llvm::AtomicRMWInst::BinOp op, llvm::GenericValue val,
			 std::unique_ptr<SExpr> annot = nullptr)
		: FaiReadLabel(EL_BIncFaiRead, st, ord, pos, addr, typ, rf,
			       op, val, std::move(annot)) {}

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
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     const llvm::GenericValue &exp, const llvm::GenericValue &swap,
		     std::unique_ptr<SExpr> annot = nullptr)
		: ReadLabel(k, st, ord, pos, addr, typ, rf, std::move(annot)),
		  expected(exp), swapValue(swap) {}

public:
	CasReadLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		     const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
		     const llvm::GenericValue &exp, const llvm::GenericValue &swap,
		     std::unique_ptr<SExpr> annot = nullptr)
		: CasReadLabel(EL_CasRead, st, ord, pos, addr, typ, rf, exp, swap, std::move(annot)) {}

	template<typename... Ts>
	static std::unique_ptr<CasReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<CasReadLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the value that will make this CAS succeed */
	const llvm::GenericValue& getExpected() const { return expected; }

	/* Returns the value that will be written is the CAS succeeds */
	const llvm::GenericValue& getSwapVal() const { return swapValue; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<CasReadLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_CasRead && k <= EL_CasReadLast; }

private:
	/* The value that will make this CAS succeed */
	const llvm::GenericValue expected;

	/* The value that will be written if the CAS succeeds */
	const llvm::GenericValue swapValue;
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
			 const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
			 const llvm::GenericValue &exp, const llvm::GenericValue &swap,
			 std::unique_ptr<SExpr> annot = nullptr)
		: CasReadLabel(EL_LockCasRead, st, ord, pos, addr, typ, rf,
			       exp, swap, std::move(annot)) {}

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
			    const llvm::GenericValue *addr, const llvm::Type *typ, Event rf,
			    const llvm::GenericValue &exp, const llvm::GenericValue &swap,
			    std::unique_ptr<SExpr> annot = nullptr)
		: CasReadLabel(EL_TrylockCasRead, st, ord, pos, addr, typ, rf,
			       exp, swap, std::move(annot)) {}

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
		: ReadLabel(EL_LibRead, st, ord, pos, addr, typ, rf), functionName(name) {}

	template<typename... Ts>
	static std::unique_ptr<LibReadLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LibReadLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the name of this operation */
	const std::string& getFunctionName() const { return functionName; }

	/* Returns a vector with writes that this read could not read from
	 * when it was first added to the graph */
	const std::vector<Event>& getInvalidRfs() const { return invalidRfs; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LibReadLabel>(*this);
	}

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
		   const llvm::Type *valTyp, llvm::GenericValue val)
		: MemAccessLabel(k, st, ord, pos, addr, valTyp), value(val) {}

public:
	WriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		   const llvm::GenericValue *addr, const llvm::Type *valTyp,
		   llvm::GenericValue val)
		: WriteLabel(EL_Write, st, ord, pos, addr, valTyp, val) {}

	template<typename... Ts>
	static std::unique_ptr<WriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<WriteLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the value written by this write access */
	const llvm::GenericValue& getVal() const { return value; }

	/* Returns a list of the reads reading from this write */
	const std::vector<Event>& getReadersList() const { return readerList; }

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
	const llvm::GenericValue value;

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
			 const llvm::GenericValue *addr, const llvm::Type *valTyp,
			 llvm::GenericValue val)
		: WriteLabel(EL_UnlockWrite, st, ord, pos, addr, valTyp, val) {}

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
			const llvm::GenericValue *addr, const llvm::Type *valTyp,
			llvm::GenericValue val)
		: WriteLabel(EL_BInitWrite, st, ord, pos, addr, valTyp, val) {}

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
			const llvm::GenericValue *addr, const llvm::Type *valTyp,
			llvm::GenericValue val)
		: WriteLabel(EL_BDestroyWrite, st, ord, pos, addr, valTyp, val) {}

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
		      const llvm::GenericValue *addr, const llvm::Type *valTyp, llvm::GenericValue val)
		: WriteLabel(k, st, ord, pos, addr, valTyp, val) {}

public:
	FaiWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val)
		: FaiWriteLabel(EL_FaiWrite, st, ord, pos, addr, valTyp, val) {}

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
			   const llvm::GenericValue *addr, const llvm::Type *valTyp,
			   llvm::GenericValue val)
		: FaiWriteLabel(EL_NoRetFaiWrite, st, ord, pos, addr, valTyp, val) {}

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
			  const llvm::GenericValue *addr, const llvm::Type *valTyp,
			  llvm::GenericValue val)
		: FaiWriteLabel(EL_BIncFaiWrite, st, ord, pos, addr, valTyp, val) {}

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
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val)
		: WriteLabel(k, st, ord, pos, addr, valTyp, val) {}

public:
	CasWriteLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos,
		      const llvm::GenericValue *addr, const llvm::Type *valTyp,
		      llvm::GenericValue val)
		: CasWriteLabel(EL_CasWrite, st, ord, pos, addr, valTyp, val) {}

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
			  const llvm::GenericValue *addr, const llvm::Type *valTyp,
			  llvm::GenericValue val)
		: CasWriteLabel(EL_LockCasWrite, st, ord, pos, addr, valTyp, val) {}

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
			     const llvm::GenericValue *addr, const llvm::Type *valTyp,
			     llvm::GenericValue val)
		: CasWriteLabel(EL_TrylockCasWrite, st, ord, pos, addr, valTyp, val) {}

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

	template<typename... Ts>
	static std::unique_ptr<LibWriteLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LibWriteLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the name of the respective function of the library */
	const std::string& getFunctionName() const { return functionName; }

	/* Returns true if this is the initializing write for this memory
	 * location and this particular library */
	bool isLibInit() const { return initial; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LibWriteLabel>(*this);
	}

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
			const llvm::GenericValue *addr, const llvm::Type *valTyp,
			llvm::GenericValue val, void *mapping,
			std::pair<void *, void*> ordDataRange)
		: DskWriteLabel(EL_DskMdWrite, st, ord, pos, addr, valTyp, val, mapping),
		  ordDataRange(ordDataRange) {}

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
			 const llvm::GenericValue *addr, const llvm::Type *valTyp,
			 llvm::GenericValue val, void *mapping)
		: DskWriteLabel(EL_DskDirWrite, st, ord, pos, addr, valTyp, val, mapping) {}

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
public:
	FenceLabel(unsigned int st, llvm::AtomicOrdering ord, Event pos)
		: FenceLabel(EL_Fence, st, ord, pos) {}

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
	DskFsyncLabel(unsigned int st, Event pos, const void *inode, unsigned int size)
		: DskFsyncLabel(st, llvm::AtomicOrdering::Release, pos, inode, size) {}

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
	DskSyncLabel(unsigned int st, Event pos)
		: DskSyncLabel(st, llvm::AtomicOrdering::Release, pos) {}

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
	DskPbarrierLabel(unsigned int st, Event pos)
		: DskPbarrierLabel(st, llvm::AtomicOrdering::Release, pos) {}

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

private:
	/* Nothing necessary for the time being */
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
		: FenceLabel(EL_SmpFenceLKMM, st, ord, pos), type(t) {}

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
	ThreadCreateLabel(unsigned int st, Event pos, unsigned int cid)
		: ThreadCreateLabel(st, llvm::AtomicOrdering::Release, pos, cid) {}

	template<typename... Ts>
	static std::unique_ptr<ThreadCreateLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<ThreadCreateLabel>(std::forward<Ts>(params)...);
	}

	/* Returns an identifier for the thread created (child) */
	unsigned int getChildId() const { return childId; }

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
	ThreadJoinLabel(unsigned int st, Event pos, unsigned int childId)
		: ThreadJoinLabel(st, llvm::AtomicOrdering::Acquire, pos, childId) {}

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
	ThreadStartLabel(unsigned int st, Event pos, Event pc, int symm = -1)
		: ThreadStartLabel(st, llvm::AtomicOrdering::Acquire, pos, pc, symm) {}

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
	ThreadFinishLabel(unsigned int st, Event pos)
		: ThreadFinishLabel(st, llvm::AtomicOrdering::Release, pos) {}

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
		    const void *addr, unsigned int size, Storage s, AddressSpace spc)
		: EventLabel(EL_Malloc, st, ord, pos),
		  allocAddr(addr), allocSize(size), s(s), spc(spc) {}
	MallocLabel(unsigned int st, Event pos, const void *addr,
		    unsigned int size, Storage s, AddressSpace spc)
		: MallocLabel(st, llvm::AtomicOrdering::NotAtomic, pos, addr, size, s, spc) {}

	template<typename... Ts>
	static std::unique_ptr<MallocLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<MallocLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the (fresh) address returned by the allocation */
	const void *getAllocAddr() const { return allocAddr; }

	/* Returns the size of this allocation */
	unsigned int getAllocSize() const { return allocSize; }

	/* Returns the storage type of this chunk */
	Storage getStorage() const { return s; }

	/* Returns the address space of this chunk */
	AddressSpace getAddrSpace() const { return spc; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<MallocLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Malloc; }

private:
	/* The address returned by malloc() */
	const void *allocAddr;

	/* The size of the requested allocation */
	unsigned int allocSize;

	/* Storage typ */
	Storage s;

	/* Whether this chunk lives in the stack/heap/internal memory */
	AddressSpace spc;
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
	FreeLabel(unsigned int st, Event pos, const void *addr)
		: FreeLabel(st, llvm::AtomicOrdering::NotAtomic, pos, addr) {}


	template<typename... Ts>
	static std::unique_ptr<FreeLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<FreeLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the address being freed */
	const void *getFreedAddr() const { return freeAddr; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<FreeLabel>(*this);
	}

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
	LockLabelLAPOR(unsigned int st, Event pos, const llvm::GenericValue *addr)
		: LockLabelLAPOR(st, llvm::AtomicOrdering::Acquire, pos, addr) {}

	template<typename... Ts>
	static std::unique_ptr<LockLabelLAPOR> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<LockLabelLAPOR>(std::forward<Ts>(params)...);
	}

	/* Returns the address of the acquired lock */
	const llvm::GenericValue *getLockAddr() const { return lockAddr; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<LockLabelLAPOR>(*this);
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
	UnlockLabelLAPOR(unsigned int st, Event pos, const llvm::GenericValue *addr)
		: UnlockLabelLAPOR(st, llvm::AtomicOrdering::Release, pos, addr) {}

	template<typename... Ts>
	static std::unique_ptr<UnlockLabelLAPOR> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<UnlockLabelLAPOR>(std::forward<Ts>(params)...);
	}

	/* Returns the address of the released lock */
	const llvm::GenericValue *getLockAddr() const { return lockAddr; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<UnlockLabelLAPOR>(*this);
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
	DskOpenLabel(unsigned int st, Event pos, const char *fileName, llvm::GenericValue fd)
		: DskOpenLabel(st, llvm::AtomicOrdering::Release, pos, fileName, fd) {}

	template<typename... Ts>
	static std::unique_ptr<DskOpenLabel> create(Ts&&... params) {
		return LLVM_MAKE_UNIQUE<DskOpenLabel>(std::forward<Ts>(params)...);
	}

	/* Returns the name of the opened file */
	const char *getFileName() const { return fileName; }

	/* Returns the file descriptor returned by open() */
	const llvm::GenericValue &getFd() const { return fd; }

	std::unique_ptr<EventLabel> clone() const override {
		return LLVM_MAKE_UNIQUE<DskOpenLabel>(*this);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_DskOpen; }

private:
	/* The name of the opened file */
	const char *fileName;

	/* The file descriptor allocated for this call */
	llvm::GenericValue fd;
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
