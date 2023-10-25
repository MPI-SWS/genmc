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
#include "EventAttr.hpp"
#include "DepInfo.hpp"
#include "value_ptr.hpp"
#include "DepView.hpp"
#include "InterpreterEnumAPI.hpp"
#include "ModuleID.hpp"
#include "NameInfo.hpp"
#include "MemAccess.hpp"
#include "ThreadInfo.hpp"
#include "SAddr.hpp"
#include "SExpr.hpp"
#include "Stamp.hpp"
#include "SVal.hpp"
#include "View.hpp"
#include "VSet.hpp"
#include <llvm/IR/Instructions.h> /* For AtomicOrdering in older LLVMs */
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <optional>

class DskAccessLabel;
class ExecutionGraph;
class ReadLabel;

template<typename T, class ...Options>
class CopyableIList : public llvm::simple_ilist<T, Options...> {

	using BaseT = llvm::simple_ilist<T, Options...>;

public:
	CopyableIList() = default;
	CopyableIList(const CopyableIList &other) : BaseT(BaseT()) {}
	CopyableIList(CopyableIList &&other) = default;

	CopyableIList &operator=(const CopyableIList &other) {
		*this = std::move(BaseT());
	}
	CopyableIList &operator=(CopyableIList &&other) = default;
};

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
		EL_Optional,
		EL_ThreadStart,
		EL_Init,
		EL_ThreadStartEnd,
		EL_ThreadFinish,
		EL_ThreadCreate,
		EL_ThreadJoin,
		EL_ThreadKill,
		EL_LoopBegin,
		EL_SpinStart,
		EL_FaiZNESpinEnd,
		EL_LockZNESpinEnd,
		EL_MemAccessBegin,
		EL_Read,
		EL_BWaitRead,
		EL_SpeculativeRead,
		EL_ConfirmingRead,
		EL_FaiRead,
		EL_NoRetFaiRead,
		EL_BIncFaiRead,
		EL_FaiReadLast,
		EL_CasRead,
		EL_LockCasRead,
		EL_TrylockCasRead,
		EL_HelpedCasRead,
		EL_ConfirmingCasRead,
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
		EL_HelpedCasWrite,
		EL_ConfirmingCasWrite,
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
		EL_HpRetire,
		EL_FreeLast,
		EL_HpProtect,
		EL_LockLAPOR,
		EL_UnlockLAPOR,
		EL_HelpingCas,
		EL_DskOpen,
		EL_RCULockLKMM,
		EL_RCUUnlockLKMM,
		EL_CLFlush,
	};

protected:
	/* ExecutionGraph needs to be a friend to call the constructors */
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	EventLabel(EventLabelKind k, Event p, llvm::AtomicOrdering o,
		   const EventDeps &deps = EventDeps())
		: kind(k), position(p), ordering(o), deps(deps) {}

	using const_dep_iterator = DepInfo::const_iterator;
	using const_dep_range = llvm::iterator_range<const_dep_iterator>;

	using calc_const_iterator = VSet<Event>::const_iterator;
	using calc_const_range = llvm::iterator_range<calc_const_iterator>;

public:
	virtual ~EventLabel() = default;

	/* Iterators for dependencies */
	const_dep_iterator data_begin() const { return deps.data.begin(); }
	const_dep_iterator data_end() const { return deps.data.end(); }
	const_dep_range data() const {
		return const_dep_range(deps.data.begin(), deps.data.end());
	}

	const_dep_iterator addr_begin() const { return deps.addr.begin(); }
	const_dep_iterator addr_end() const { return deps.addr.end(); }
	const_dep_range addr() const {
		return const_dep_range(deps.addr.begin(), deps.addr.end());
	}

	const_dep_iterator ctrl_begin() const { return deps.ctrl.begin(); }
	const_dep_iterator ctrl_end() const { return deps.ctrl.end(); }
	const_dep_range ctrl() const {
		return const_dep_range(deps.ctrl.begin(), deps.ctrl.end());
	}

	/* Returns the discriminator of this object */
	EventLabelKind getKind() const { return kind; }

	/* Returns the position in the execution graph (thread, index) */
	Event getPos() const { return position; }
	Event &getPos() { return position; }

	/* Returns the index of this label within a thread */
	int getIndex() const { return position.index; }

	/* Returns the thread of this label in the execution graph */
	int getThread() const { return position.thread; }

	/* Getter/setter for the label ordering */
	llvm::AtomicOrdering getOrdering() const { return ordering; }
	void setOrdering(llvm::AtomicOrdering ord) { ordering = ord; }

	/* Returns this label's dependencies */
	const EventDeps &getDeps() const { return deps; }

	/* Sets this label's dependencies */
	void setDeps(const EventDeps &ds) { deps = ds; }
	void setDeps(const EventDeps *ds) { deps = *ds; }
	void setDeps(EventDeps &&ds) { deps = std::move(ds); }

	/* Returns whether a stamp has been assigned for this label */
	bool hasStamp() const { return stamp.has_value(); }

	/* Returns the stamp of the label in a graph */
	Stamp getStamp() const {
#ifdef ENABLE_GENMC_DEBUG
		return stamp.value();
#else
		return *stamp;
#endif
	}

	bool hasPrefixView() const { return prefixView.get() != nullptr; }
	const VectorClock &getPrefixView() const { return *prefixView; }
	VectorClock &getPrefixView() { return *prefixView; }
	void setPrefixView(std::unique_ptr<VectorClock> v) const {
		prefixView = std::move(v);
	}

	void setCalculated(std::vector<VSet<Event>> &&calc) {
		calculatedRels = std::move(calc);
	}

	void setViews(std::vector<View> &&views) {
		calculatedViews = std::move(views);
	}

	/* Iterators for calculated relations */
	calc_const_range calculated(size_t i) const {
		return (getPos().isInitializer() || getKind() == EL_Empty) ?
			calculatedRels[0] : calculatedRels[i];
	}

	/* Getters for calculated views */
	const View &view(size_t i) const {
		return (getPos().isInitializer() || getKind() == EL_Empty) ?
			calculatedViews[0] : calculatedViews[i];
	}

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

	/* Returns true if the ordering of this access is acquire or weaker */
	bool isAtMostAcquire() const {
		return ordering == llvm::AtomicOrdering::NotAtomic ||
		       ordering == llvm::AtomicOrdering::Monotonic ||
		       ordering == llvm::AtomicOrdering::Acquire;
	}

	/* Returns true if the ordering of this access is release or stronger */
	bool isAtLeastRelease() const {
		return ordering == llvm::AtomicOrdering::Release ||
		       ordering == llvm::AtomicOrdering::AcquireRelease ||
		       ordering == llvm::AtomicOrdering::SequentiallyConsistent;
	}

	/* Returns true if the ordering of this access is release or weaker */
	bool isAtMostRelease() const {
		return ordering == llvm::AtomicOrdering::NotAtomic ||
		       ordering == llvm::AtomicOrdering::Monotonic ||
		       ordering == llvm::AtomicOrdering::Release;
	}

	/* Returns true if this is a sequentially consistent access */
	bool isSC() const {
		return ordering == llvm::AtomicOrdering::SequentiallyConsistent;
	}

	/* Whether this label denotes the end of a thread */
	bool isTerminator() const {
		return isTerminator(getKind());
	}

	/* Whether this label can have outgoing dep edges */
	bool isDependable() const {
		return isDependable(getKind());
	}

	/* Whether this label carries a value */
	bool hasValue() const {
		return hasValue(getKind());
	}

	/* Returns true if this event can be revisited */
	bool isRevisitable() const { return revisitable; }

	/* Makes the relevant event revisitable/non-revisitable. The
	 * execution graph is responsible for making such changes */
	void setRevisitStatus(bool status) { revisitable = status; }

	/* Returns true if this event cannot be revisited or deleted */
	bool isStable() const;

	/* Necessary for multiple inheritance + LLVM-style RTTI to work */
	static bool classofKind(EventLabelKind K) { return true; }
	static DskAccessLabel *castToDskAccessLabel(const EventLabel *);
	static EventLabel *castFromDskAccessLabel(const DskAccessLabel *);

	/* Returns a clone object (virtual to allow deep copying from base) */
	virtual std::unique_ptr<EventLabel> clone() const = 0;

	/* Resets all graph-related info on a label to their default values */
	virtual void reset() {
		stamp = std::nullopt;
		calculatedRels.clear();
		calculatedViews.clear();
		prefixView = nullptr;
		revisitable = true;
	}

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const EventLabel &lab);

private:
	static inline bool isTerminator(EventLabelKind k) {
		return k == EL_Block || k == EL_ThreadKill || k == EL_ThreadFinish;
	}

	static inline bool isDependable(EventLabelKind k) {
		return (k >= EL_Read && k <= EL_LastRead) || k == EL_Malloc;
	}

	static inline bool hasValue(EventLabelKind k) {
		return (k >= EL_Read && k <= EL_LastRead) ||
			(k >= EL_ThreadStart && k <= EL_ThreadStartEnd) ||
			k == EL_ThreadJoin ||
			k == EL_Optional;
	}

	void setStamp(Stamp s) { stamp = s; }

	/* Discriminator enum for LLVM-style RTTI */
	const EventLabelKind kind;

	/* Position of this label within the execution graph (thread, index) */
	Event position;

	/* Ordering of this access mode */
	llvm::AtomicOrdering ordering;

	/* Events on which this label depends */
	EventDeps deps;

	/* The stamp of this label in the execution graph */
	std::optional<Stamp> stamp = std::nullopt;

	mutable value_ptr<VectorClock, VectorClockCloner> prefixView = nullptr;

	/* Saved calculations */
	std::vector<VSet<Event>> calculatedRels;

	/* Saved views */
	std::vector<View> calculatedViews;

	/* Revisitability status */
	bool revisitable = true;
};

#define DEFINE_CREATE_CLONE(name)					\
	template<typename... Ts>					\
	static std::unique_ptr<name> create(Ts&&... params) {		\
		return std::make_unique<name>(std::forward<Ts>(params)...); \
	}								\
									\
	std::unique_ptr<EventLabel> clone() const override {		\
		return std::make_unique<name>(*this);			\
	}


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

	virtual void reset() {
		pbView.clear();
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
	EmptyLabel(Event pos)
		: EventLabel(EL_Empty, pos, llvm::AtomicOrdering::NotAtomic, EventDeps()) {}

	DEFINE_CREATE_CLONE(EmptyLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Empty; }
};


/*******************************************************************************
 **                            BlockLabel Class
 ******************************************************************************/

/* A label that represents a blockage event */
class BlockLabel : public EventLabel {

public:
	BlockLabel(Event pos, BlockageType t, const EventDeps &deps = EventDeps())
		: EventLabel(EL_Block, pos, llvm::AtomicOrdering::NotAtomic, deps),
		  type(t) {}

	BlockageType getType() const { return type; }

	DEFINE_CREATE_CLONE(BlockLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Block; }

private:
	BlockageType type;
};


/*******************************************************************************
 **                            OptionalLabel Class
 ******************************************************************************/

/* A label that represents the beginning of an optional block */
class OptionalLabel : public EventLabel {

public:
	OptionalLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_Optional, pos, llvm::AtomicOrdering::NotAtomic, deps) {}

	/* Whether this block is expandable */
	bool isExpandable() const { return expandable; }
	void setExpandable(bool exp) { expandable = exp; }

	/* Whether this block has been expanded */
	bool isExpanded() const { return expanded; }
	void setExpanded(bool exp) { expanded = exp; }

	DEFINE_CREATE_CLONE(OptionalLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Optional; }

private:
	bool expandable = true;
	bool expanded = false;
};


/*******************************************************************************
 **                            LoopBeginLabel Class
 ******************************************************************************/

/* A label that marks the beginning of a spinloop. Used in DSA. */
class LoopBeginLabel : public EventLabel {

public:
	LoopBeginLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_LoopBegin, pos, llvm::AtomicOrdering::NotAtomic, deps) {}

	DEFINE_CREATE_CLONE(LoopBeginLabel)

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
	SpinStartLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_SpinStart, pos, llvm::AtomicOrdering::NotAtomic, deps) {}

	DEFINE_CREATE_CLONE(SpinStartLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_SpinStart; }
};


/*******************************************************************************
 **                            FaiZNESpinEndLabel Class
 ******************************************************************************/

/* A label that marks the end of a potential FaiZNE spinloop. If the loop turns out to be not
 * a spinloop, it is added to the graph; otherwise, it should be replaced by a BlockLabel */
class FaiZNESpinEndLabel : public EventLabel {

public:
	FaiZNESpinEndLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_FaiZNESpinEnd, pos, llvm::AtomicOrdering::NotAtomic, deps) {}

	DEFINE_CREATE_CLONE(FaiZNESpinEndLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_FaiZNESpinEnd; }
};


/*******************************************************************************
 **                            LockZNESpinEndLabel Class
 ******************************************************************************/

/* A label that marks the end of a potential LockZNE spinloop */
class LockZNESpinEndLabel : public EventLabel {

public:
	LockZNESpinEndLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_LockZNESpinEnd, pos, llvm::AtomicOrdering::NotAtomic, deps) {}

	DEFINE_CREATE_CLONE(LockZNESpinEndLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LockZNESpinEnd; }
};

class MallocLabel;

/*******************************************************************************
 **                       MemAccessLabel Class (Abstract)
 ******************************************************************************/

/* This label abstracts the common functionality that loads and stores have
 * (e.g., asking for the address of such a label) */
class MemAccessLabel : public EventLabel, public llvm::ilist_node<MemAccessLabel> {

protected:
	MemAccessLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		       SAddr loc, ASize size, AType type,
		       const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), access(loc, size, type) {}
	MemAccessLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		       AAccess a, const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), access(a) {}
public:
	/* Returns the address of this access */
	SAddr getAddr() const { return access.getAddr(); }

	/* Returns the size (in bytes) of the access */
	ASize getSize() const { return access.getSize(); }

	/* Returns the type of the access */
	AType getType() const { return access.getType(); }

	/* Returns the packed access */
	const AAccess &getAccess() const { return access; }

	/* Helper flag for maximality checks */
	bool wasAddedMax() const { return maximal; }
	void setAddedMax(bool status) { maximal = status; }

	/* Getter for allocating event */
	MallocLabel *getAlloc() const { return allocLab; }
	MallocLabel *getAlloc() { return allocLab; }

	void setAlloc(MallocLabel *lab) { allocLab = lab; }

	virtual void reset() override {
		EventLabel::reset();
		maximal = true;
		allocLab = nullptr;
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_MemAccessBegin && k <= EL_MemAccessEnd;
	}

private:
	/* The access performed */
	AAccess access;

	/* Whether was mo-maximal when added */
	bool maximal = true;

	/* The allocation event corresponding to this access */
	MallocLabel *allocLab = nullptr;
};


/*******************************************************************************
 **                         ReadLabel Class
 ******************************************************************************/

/* The label for reads. All special read types (e.g., FAI, CAS) should inherit
 * from this class */
class ReadLabel : public MemAccessLabel, public llvm::ilist_node<ReadLabel> {

public:
	using AnnotT = SExpr<ModuleID::ID>;
	using AnnotVP = value_ptr<AnnotT, SExprCloner<ModuleID::ID>>;

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	ReadLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		  SAddr loc, ASize size, AType type, EventLabel *rfLab = nullptr,
		  AnnotVP annot = nullptr, const EventDeps &deps = EventDeps())
		: MemAccessLabel(k, pos, ord, loc, size, type, deps),
		  readsFrom(rfLab), annotExpr(std::move(annot)) {}
public:
	ReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		  AType type, EventLabel *rfLab, AnnotVP annot, const EventDeps &deps = EventDeps())
		: ReadLabel(EL_Read, pos, ord, loc, size, type, rfLab, annot, deps) {}
	ReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		  AType type, EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, type, rfLab, nullptr, deps) {}
	ReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		  AType type, const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, type, nullptr, nullptr, deps) {}

	DEFINE_CREATE_CLONE(ReadLabel)

	/* Returns the position of the write this read is readinf-from */
	EventLabel *getRf() const { return readsFrom; }
	EventLabel *getRf() { return readsFrom; }

	/* Whether this read has a set RF and reads externally */
	bool readsExt() const {
		return getRf() && !getRf()->getPos().isInitializer() &&
			getRf()->getThread() != getThread();
	}

	/* Whether this read has a set RF and reads internally */
	bool readsInt() const {
		return getRf() && (getRf()->getPos().isInitializer() ||
				   getRf()->getThread() == getThread());
	}

	/* Returns true if the read was revisited in-place */
	bool isIPR() const { return ipr; }

	/* Sets the IPR status for this read */
	void setIPRStatus(bool status) { ipr = status; }

	/* SAVer: Getter/setter for the annotation expression */
	const AnnotT *getAnnot() const { return annotExpr.get(); }
	void setAnnot(std::unique_ptr<AnnotT> annot) { annotExpr = std::move(annot); }

	virtual void reset() override {
		MemAccessLabel::reset();
		setRf(nullptr);
		ipr = false;
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_Read && k <= EL_LastRead;
	}

private:
	/* Changes the reads-from edge for this label. This should only
	 * be called from the execution graph to update other relevant
	 * information as well */
	void setRf(EventLabel *rfLab) { readsFrom = rfLab; }

	/* Position of the write it is reading from in the graph */
	EventLabel *readsFrom;

	/* Whether the read has been revisited in place */
	bool ipr;

	/* SAVer: Expression for annotatable loads. This needs to have
	 * heap-value semantics so that it does not create concurrency issues */
	AnnotVP annotExpr;
};

#define READ_PURE_SUBCLASS(_class_kind)			\
class _class_kind ## ReadLabel : public ReadLabel {	\
							\
protected:						\
	friend class ExecutionGraph;			\
	friend class DepExecutionGraph;			\
							\
public:									\
	_class_kind ## ReadLabel(Event pos, llvm::AtomicOrdering ord,	\
				 SAddr loc, ASize size, AType type,	\
				 EventLabel *rfLab, AnnotVP annot,	\
				 const EventDeps &deps = EventDeps()) \
	: ReadLabel(EL_ ## _class_kind ## Read, pos, ord, loc, size,	\
		    type, rfLab, std::move(annot), deps) {}		\
	_class_kind ## ReadLabel(Event pos, llvm::AtomicOrdering ord,	\
				 SAddr loc, ASize size, AType type,	\
				 EventLabel *rfLab, const EventDeps &deps = EventDeps())	\
	: _class_kind ## ReadLabel(pos, ord, loc, size, type, rfLab,	\
				   nullptr, deps) {}		\
	_class_kind ## ReadLabel(Event pos, llvm::AtomicOrdering ord,	\
				 SAddr loc, ASize size, AType type,	\
				 const EventDeps &deps = EventDeps())	\
	: _class_kind ## ReadLabel(pos, ord, loc, size,	type,		\
				   nullptr, nullptr, deps) {}		\
									\
	DEFINE_CREATE_CLONE(_class_kind ## ReadLabel)			\
									\
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); } \
	static bool classofKind(EventLabelKind k) { return k == EL_ ## _class_kind ## Read; } \
};

READ_PURE_SUBCLASS(Speculative);
READ_PURE_SUBCLASS(Confirming);


/*******************************************************************************
 **                         BWaitReadLabel Class
 ******************************************************************************/

/* Specialization of ReadLabel for the read part of a barrier_wait() op */
class BWaitReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	BWaitReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		       AType type, EventLabel *rfLab, AnnotVP annot, const EventDeps &deps = EventDeps())
		: ReadLabel(EL_BWaitRead, pos, ord, loc, size, type, rfLab, std::move(annot), deps) {}
	BWaitReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		       AType type, EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: BWaitReadLabel(pos, ord, loc, size, type, rfLab, nullptr, deps) {}
	BWaitReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		       AType type, const EventDeps &deps = EventDeps())
		: BWaitReadLabel(pos, ord, loc, size, type, nullptr, nullptr, deps) {}

	DEFINE_CREATE_CLONE(BWaitReadLabel)

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

	FaiReadLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		     SAddr addr, ASize size, AType type, llvm::AtomicRMWInst::BinOp op,
		     SVal val, WriteAttr wattr, EventLabel *rfLab, AnnotVP annot,
		     const EventDeps &deps = EventDeps())
		: ReadLabel(k, pos, ord, addr, size, type, rfLab, std::move(annot), deps),
		  binOp(op), opValue(val), wattr(wattr) {}

public:
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,
		     EventLabel *rfLab, AnnotVP annot, const EventDeps &deps = EventDeps())
		: FaiReadLabel(EL_FaiRead, pos, ord, addr, size, type, op, val,
			       wattr, rfLab, std::move(annot), deps) {}
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,
		     EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, wattr, rfLab, nullptr, deps) {}
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,
		     const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, wattr, nullptr, deps) {}
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, WriteAttr::None, deps) {}

	DEFINE_CREATE_CLONE(FaiReadLabel)

	/* Returns the type of this RMW operation (e.g., add, sub) */
	llvm::AtomicRMWInst::BinOp getOp() const { return binOp; }

	/* Returns the other operand's value */
	SVal getOpVal() const { return opValue; }

	/* Returns/sets the attributes of the write part */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/* Checks whether the write part has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	virtual void reset() override {
		ReadLabel::reset();
		wattr &= ~(WriteAttr::RevBlocker);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_FaiRead && k <= EL_FaiReadLast; }

private:
	/* The binary operator for this RMW operation */
	const llvm::AtomicRMWInst::BinOp binOp;

	/* The other operand's value for the operation */
	SVal opValue;

	/* Attributes for the write part of the RMW */
	WriteAttr wattr;
};

#define FAIREAD_PURE_SUBCLASS(_class_kind)			\
class _class_kind ## FaiReadLabel : public FaiReadLabel {	\
								\
protected:							\
	friend class ExecutionGraph;				\
	friend class DepExecutionGraph;				\
								\
public:								\
	_class_kind ## FaiReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type, \
				    llvm::AtomicRMWInst::BinOp op, SVal val, \
				    WriteAttr wattr, EventLabel *rfLab, AnnotVP annot, \
				    const EventDeps &deps = EventDeps())		\
	: FaiReadLabel(EL_ ## _class_kind ## FaiRead, pos, ord, addr, size, \
		       type, op, val, wattr, rfLab, std::move(annot), deps) {} \
	_class_kind ## FaiReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type, \
				    llvm::AtomicRMWInst::BinOp op, SVal val, \
				    WriteAttr wattr, EventLabel *rfLab,		\
				    const EventDeps &deps = EventDeps())		\
	: _class_kind ## FaiReadLabel(pos, ord, addr, size, type, op, \
				      val, wattr, rfLab, nullptr, deps) {} \
	_class_kind ## FaiReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type,	\
				    llvm::AtomicRMWInst::BinOp op, SVal val, \
				    WriteAttr wattr,			\
				    const EventDeps &deps = EventDeps())		\
	: _class_kind ## FaiReadLabel(pos, ord, addr, size, type, op, \
				      val, wattr, nullptr, deps) {} \
	_class_kind ## FaiReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type,	\
				    llvm::AtomicRMWInst::BinOp op, SVal val, \
				    const EventDeps &deps = EventDeps())		\
	: _class_kind ## FaiReadLabel(pos, ord, addr, size, type, op,	\
				      val, WriteAttr::None, deps) {} \
									\
									\
	DEFINE_CREATE_CLONE(_class_kind ## FaiReadLabel)		\
									\
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); } \
	static bool classofKind(EventLabelKind k) { return k == EL_  ## _class_kind ## FaiRead; } \
};

FAIREAD_PURE_SUBCLASS(NoRet);
FAIREAD_PURE_SUBCLASS(BInc);


/*******************************************************************************
 **                         CasReadLabel Class
 ******************************************************************************/

/* Represents the read part of a compare-and-swap (CAS) operation */
class CasReadLabel : public ReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	CasReadLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		     SAddr addr, ASize size, AType type, SVal exp, SVal swap,
		     WriteAttr wattr, EventLabel *rfLab, AnnotVP annot,
		     const EventDeps &deps = EventDeps())
		: ReadLabel(k, pos, ord, addr, size, type, rfLab, std::move(annot), deps),
		  wattr(wattr), expected(exp), swapValue(swap) {}

public:
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		     AType type, SVal exp, SVal swap, WriteAttr wattr,
		     EventLabel *rfLab, AnnotVP annot, const EventDeps &deps = EventDeps())
		: CasReadLabel(EL_CasRead, pos, ord, addr, size, type, exp,
			       swap, wattr, rfLab, std::move(annot), deps) {}
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		     AType type, SVal exp, SVal swap, WriteAttr wattr,
		     EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp,
			       swap, wattr, rfLab, nullptr, deps) {}
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		     AType type, SVal exp, SVal swap, WriteAttr wattr,
		     const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp,
			       swap, wattr, nullptr, deps) {}
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		     AType type, SVal exp, SVal swap,
		     const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp,
			       swap, WriteAttr::None, deps) {}

	DEFINE_CREATE_CLONE(CasReadLabel)

	/* Returns the value that will make this CAS succeed */
	SVal getExpected() const { return expected; }

	/* Returns the value that will be written is the CAS succeeds */
	SVal getSwapVal() const { return swapValue; }

	/* Returns/sets the attributes of the write part */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/* Checks whether the write part has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	virtual void reset() override {
		ReadLabel::reset();
		wattr &= ~(WriteAttr::RevBlocker);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_CasRead && k <= EL_CasReadLast; }

private:
	/* The value that will make this CAS succeed */
	const SVal expected;

	/* The value that will be written if the CAS succeeds */
	const SVal swapValue;

	/* The attributes of the write part of the RMW */
	WriteAttr wattr;
};

#define CASREAD_PURE_SUBCLASS(_class_kind)			\
class _class_kind ## CasReadLabel : public CasReadLabel {	\
								\
protected:							\
	friend class ExecutionGraph;				\
	friend class DepExecutionGraph;				\
								\
public:								\
	_class_kind ## CasReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type, SVal exp, SVal swap, \
				    WriteAttr wattr, EventLabel *rfLab, AnnotVP annot, \
				    const EventDeps &deps = EventDeps()) \
	: CasReadLabel(EL_ ## _class_kind ## CasRead, pos, ord, addr, size, \
		       type, exp, swap, wattr, rfLab, std::move(annot), deps) {} \
	_class_kind ## CasReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type, SVal exp, SVal swap, \
				    WriteAttr wattr, EventLabel *rfLab,	\
				    const EventDeps &deps = EventDeps()) \
		: _class_kind ## CasReadLabel(pos, ord, addr, size, type, exp, \
					      swap, wattr, rfLab, nullptr, deps) {} \
	_class_kind ## CasReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type, SVal exp, SVal swap, \
				    WriteAttr wattr, const EventDeps &deps = EventDeps())		\
	: _class_kind ## CasReadLabel(pos, ord, addr, size, type, exp,	\
				      swap, wattr, nullptr, deps) {}	\
	_class_kind ## CasReadLabel(Event pos, llvm::AtomicOrdering ord,\
				    SAddr addr, ASize size, AType type, \
				    SVal exp, SVal swap,		\
				    const EventDeps &deps = EventDeps()) \
	: _class_kind ## CasReadLabel(pos, ord, addr, size, type, exp,	\
				      swap, WriteAttr::None, deps) {}	\
									\
									\
	DEFINE_CREATE_CLONE(_class_kind ## CasReadLabel)		\
									\
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); } \
	static bool classofKind(EventLabelKind k) { return k == EL_  ## _class_kind ## CasRead; } \
};

CASREAD_PURE_SUBCLASS(Helped);
CASREAD_PURE_SUBCLASS(Confirming);

/*******************************************************************************
 **                         LockCasReadLabel Class
 ******************************************************************************/

/* Specialization of CasReadLabel for lock CASes */
class LockCasReadLabel : public CasReadLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	LockCasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			 AType type, SVal exp, SVal swap, WriteAttr wattr,
			 EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: CasReadLabel(EL_LockCasRead, pos, ord, addr, size, type, exp,
			       swap, wattr, rfLab, nullptr, deps) {}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			 const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, llvm::AtomicOrdering::Acquire, addr, size,
				   AType::Signed, SVal(0), SVal(1), wattr,
				   rfLab, deps) {}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr,
			 const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, addr, size, wattr, nullptr, deps) {}
	LockCasReadLabel(Event pos, SAddr addr, ASize size,
			 const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, addr, size, WriteAttr::None, deps) {}

	DEFINE_CREATE_CLONE(LockCasReadLabel)

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
	TrylockCasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			    AType type, SVal exp, SVal swap, WriteAttr wattr,
			    EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: CasReadLabel(EL_TrylockCasRead, pos, ord, addr, size, type,
			       exp, swap, wattr, rfLab, nullptr, deps) {}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, llvm::AtomicOrdering::Acquire, addr, size,
				      AType::Signed, SVal(0), SVal(1), wattr,
				      rfLab, deps) {}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr,
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, addr, size, wattr, nullptr, deps) {}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size,
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, addr, size, WriteAttr::None, deps) {}

	DEFINE_CREATE_CLONE(TrylockCasReadLabel)

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
	DskReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		     AType type, EventLabel *rfLab = nullptr,
		     const EventDeps &deps = EventDeps())
		: ReadLabel(EL_DskRead, pos, ord, loc, size, type, rfLab, nullptr, deps),
		  DskAccessLabel(EL_DskRead) {}

	DskReadLabel(Event pos, SAddr loc, ASize size, AType type,
		     EventLabel *rfLab = nullptr, const EventDeps &deps = EventDeps())
		: DskReadLabel(pos, llvm::AtomicOrdering::Acquire, loc, size, type, rfLab, deps) {}

	DEFINE_CREATE_CLONE(DskReadLabel)

	virtual void reset() override {
		ReadLabel::reset();
		DskAccessLabel::reset();
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
class WriteLabel : public MemAccessLabel, public llvm::ilist_node<WriteLabel> {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	WriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		   SAddr addr, ASize size, AType type, SVal val, WriteAttr wattr,
		   const EventDeps &deps = EventDeps())
		: MemAccessLabel(k, pos, ord, addr, size, type, deps), value(val), wattr(wattr) {}
	WriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		   SAddr addr, ASize size, AType type, SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, WriteAttr::None, deps) {}

public:
	WriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		   AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: WriteLabel(EL_Write, pos, ord, addr, size, type, val, wattr, deps) {}
	WriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		   AType type, SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps) {}

	DEFINE_CREATE_CLONE(WriteLabel)

	/* Getter/setter for the write value */
	SVal getVal() const { return value; }
	void setVal(SVal v) { value = v; }

	/* Returns the attributes of the write */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/* Checks whether the write has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	/* Helpers for various write attributes */
	bool isFinal() const { return hasAttr(WriteAttr::Final); }
	bool isLocal() const { return hasAttr(WriteAttr::Local); }

	/* Iterators for readers */
	using ReaderList = CopyableIList<ReadLabel>;
	using rf_iterator = ReaderList::iterator;
	using const_rf_iterator = ReaderList::const_iterator;
	using rf_range = llvm::iterator_range<rf_iterator>;
	using const_rf_range = llvm::iterator_range<const_rf_iterator>;

	rf_iterator readers_begin() { return readerList.begin(); }
	rf_iterator readers_end() { return readerList.end(); }
	rf_range readers() {
		return rf_range(readers_begin(), readers_end());
	}
	const_rf_iterator readers_begin() const { return readerList.begin(); }
	const_rf_iterator readers_end() const { return readerList.end(); }
	const_rf_range readers() const {
		return const_rf_range(readers_begin(), readers_end());
	}

	virtual void reset() override {
		MemAccessLabel::reset();
		readerList.clear();
		wattr &= ~(WriteAttr::RevBlocker);
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) {
		return k >= EL_Write && k <= EL_LastWrite;
	}

private:
	friend class CoherenceCalculator;

	/* Adds a read to the list of reads reading from the write */
	void addReader(ReadLabel *rLab) {
		BUG_ON(std::find_if(readers_begin(), readers_end(), [rLab](ReadLabel &oLab){
			return oLab.getPos() == rLab->getPos(); }) != readers_end());
		readerList.push_back(*rLab);
	}

	/* Removes all readers that satisfy predicate F */
	template <typename F>
	void removeReader(F cond) {
		for (auto it = readers_begin(); it != readers_end(); ) {
			if (cond(*it))
				it = readerList.erase(it);
			else
				++it;
		}
	}

	/* The value written by this label */
	SVal value;

	/* List of reads reading from the write */
	ReaderList readerList;

	/* Attributes of the write */
	WriteAttr wattr;
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
	UnlockWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			 AType type, SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(EL_UnlockWrite, pos, ord, addr, size, type, val, deps) {}
	UnlockWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: UnlockWriteLabel(pos, llvm::AtomicOrdering::Release, addr, size,
				   AType::Signed, SVal(0), deps) {}

	DEFINE_CREATE_CLONE(UnlockWriteLabel)

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
	BInitWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			AType type, SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(EL_BInitWrite, pos, ord, addr, size, type, val, deps) {}

	DEFINE_CREATE_CLONE(BInitWriteLabel)

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
	BDestroyWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			   AType type, SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(EL_BDestroyWrite, pos, ord, addr, size, type, val, deps) {}

	DEFINE_CREATE_CLONE(BDestroyWriteLabel)

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

	FaiWriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		      AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, wattr, deps) {}

public:
	FaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		      AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(EL_FaiWrite, pos, ord, addr, size, type, val, wattr, deps) {}
	FaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		      AType type, SVal val, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps) {}

	DEFINE_CREATE_CLONE(FaiWriteLabel)

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
	NoRetFaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			   AType type, SVal val, WriteAttr wattr = WriteAttr::None,
			   const EventDeps &deps = EventDeps())
		: FaiWriteLabel(EL_NoRetFaiWrite, pos, ord, addr, size, type,
				val, wattr, deps) {}

	DEFINE_CREATE_CLONE(NoRetFaiWriteLabel)

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
	BIncFaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			  AType type, SVal val, WriteAttr wattr,
			  const EventDeps &deps = EventDeps())
		: FaiWriteLabel(EL_BIncFaiWrite, pos, ord, addr, size, type, val,
				wattr, deps) {}
	BIncFaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			  AType type, SVal val, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(EL_BIncFaiWrite, pos, ord, addr, size, type, val,
				WriteAttr::None, deps) {}

	DEFINE_CREATE_CLONE(BIncFaiWriteLabel)

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

	CasWriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr,
		      ASize size, AType type, SVal val, WriteAttr wattr = WriteAttr::None,
		      const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, wattr, deps) {}

public:
	CasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		      SVal val, WriteAttr wattr = WriteAttr::None,
		      const EventDeps &deps = EventDeps())
		: CasWriteLabel(EL_CasWrite, pos, ord, addr, size, type, val, wattr, deps) {}

	DEFINE_CREATE_CLONE(CasWriteLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_CasWrite && k <= EL_CasWriteLast; }
};


#define CASWRITE_PURE_SUBCLASS(_class_kind)			\
class _class_kind ## CasWriteLabel : public CasWriteLabel {	\
								\
protected:							\
	friend class ExecutionGraph;				\
	friend class DepExecutionGraph;				\
								\
public:								\
	_class_kind ## CasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, \
				     ASize size, AType type, SVal val, WriteAttr wattr = WriteAttr::None, \
				     const EventDeps &deps = EventDeps()) \
	: CasWriteLabel(EL_ ## _class_kind ## CasWrite, pos, ord, addr,	\
			size, type, val, wattr, deps) {}		\
									\
	DEFINE_CREATE_CLONE(_class_kind ## CasWriteLabel)		\
									\
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); } \
	static bool classofKind(EventLabelKind k) {			\
		return k == EL_ ## _class_kind ## CasWrite; 		\
	}								\
};

CASWRITE_PURE_SUBCLASS(Helped);
CASWRITE_PURE_SUBCLASS(Confirming);

/*******************************************************************************
 **                         LockCasWriteLabel Class
 ******************************************************************************/

/* Specialization of CasWriteLabel for lock CASes */
class LockCasWriteLabel : public CasWriteLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	LockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			  AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasWriteLabel(EL_LockCasWrite, pos, ord, addr, size, type, val,
				wattr, deps) {}
	LockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			  AType type, SVal val, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps) {}
	LockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, llvm::AtomicOrdering::Acquire, addr, size,
				    AType::Signed, SVal(1), deps) {}

	DEFINE_CREATE_CLONE(LockCasWriteLabel)

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
	TrylockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			     AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasWriteLabel(EL_TrylockCasWrite, pos, ord, addr, size, type,
				val, WriteAttr::None, deps) {}
	TrylockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			     AType type, SVal val, const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps) {}
	TrylockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, llvm::AtomicOrdering::Acquire, addr, size,
				       AType::Signed, SVal(1), deps) {}

	DEFINE_CREATE_CLONE(TrylockCasWriteLabel)

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

	DskWriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr,
		      ASize size, AType type, SVal val, void *mapping,
		      const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, deps),
		  DskAccessLabel(k), mapping(mapping) {}

public:
	DskWriteLabel(Event pos,llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		      SVal val, void *mapping, const EventDeps &deps = EventDeps())
		: DskWriteLabel(EL_DskWrite, pos, ord, addr, size, type, val, mapping, deps) {}

	DskWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val, void *mapping,
		      const EventDeps &deps = EventDeps())
		: DskWriteLabel(pos, llvm::AtomicOrdering::Release, addr, size, type,
				val, mapping, deps) {}

	/* Returns the starting offset for this write's disk mapping */
	const void *getMapping() const { return mapping; }

	DEFINE_CREATE_CLONE(DskWriteLabel)

	virtual void reset() override {
		WriteLabel::reset();
		DskAccessLabel::reset();
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
	DskMdWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			SVal val, void *mapping, std::pair<void *, void*> ordDataRange,
			const EventDeps &deps = EventDeps())
		: DskWriteLabel(EL_DskMdWrite, pos, ord, addr, size, type,
				val, mapping, deps), ordDataRange(ordDataRange) {}

	DskMdWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val,
			void *mapping, std::pair<void *, void*> ordDataRange,
			const EventDeps &deps = EventDeps())
		: DskWriteLabel(EL_DskMdWrite, pos, llvm::AtomicOrdering::Release, addr, size,
				type, val, mapping, deps), ordDataRange(ordDataRange) {}

	/* Helpers that return data with which this write is ordered */
	const void *getOrdDataBegin() const { return ordDataRange.first; }
	const void *getOrdDataEnd() const { return ordDataRange.second; }
	const std::pair<void *, void *>
	getOrdDataRange() const { return ordDataRange; }

	DEFINE_CREATE_CLONE(DskMdWriteLabel)

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
	DskDirWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			 SVal val, void *mapping, const EventDeps &deps = EventDeps())
		: DskWriteLabel(EL_DskDirWrite, pos, ord, addr, size, type, val, mapping, deps) {}

	DskDirWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val, void *mapping,
			 const EventDeps &deps = EventDeps())
		: DskWriteLabel(EL_DskDirWrite, pos, llvm::AtomicOrdering::Release, addr, size,
				type, val, mapping, deps) {}

	DEFINE_CREATE_CLONE(DskDirWriteLabel)

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
	DskJnlWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			 SVal val, void *mapping, void *inode, const EventDeps &deps = EventDeps())
		: DskWriteLabel(EL_DskJnlWrite, pos, ord, addr, size, type, val,
				mapping, deps), inode(inode) {}

	DskJnlWriteLabel(Event pos, SAddr addr, ASize size, AType type, SVal val, void *mapping,
			 void *inode, const EventDeps &deps = EventDeps())
		: DskWriteLabel(EL_DskJnlWrite, pos, llvm::AtomicOrdering::Release, addr, size,
				type, val, mapping, deps), inode(inode) {}

	/* Returns the inode on which the transaction takes place */
	const void *getTransInode() const { return inode; }

	DEFINE_CREATE_CLONE(DskJnlWriteLabel)

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

	FenceLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		   const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps) {}
public:
	FenceLabel(Event pos, llvm::AtomicOrdering ord, const EventDeps &deps = EventDeps())
		: FenceLabel(EL_Fence, pos, ord, deps) {}

	DEFINE_CREATE_CLONE(FenceLabel)

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
	DskFsyncLabel(Event pos, llvm::AtomicOrdering ord, const void *inode, unsigned int size,
		      const EventDeps &deps = EventDeps())
		: FenceLabel(EL_DskFsync, pos, ord, deps), DskAccessLabel(EL_DskFsync),
		  inode(inode), size(size) {}
	DskFsyncLabel(Event pos, const void *inode, unsigned int size,
		      const EventDeps &deps = EventDeps())
		: DskFsyncLabel(pos, llvm::AtomicOrdering::Release, inode, size, deps) {}

	/* Returns a pointer to the inode on which the fsync() took place */
	const void *getInode() const { return inode; }

	/* Returns the "size" of this fsync()'s range */
	unsigned int getSize() const { return size; }

	DEFINE_CREATE_CLONE(DskFsyncLabel)

	virtual void reset() override {
		FenceLabel::reset();
		DskAccessLabel::reset();
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
	DskSyncLabel(Event pos, llvm::AtomicOrdering ord, const EventDeps &deps = EventDeps())
		: FenceLabel(EL_DskSync, pos, ord, deps), DskAccessLabel(EL_DskSync) {}
	DskSyncLabel(Event pos, const EventDeps &deps = EventDeps())
		: DskSyncLabel(pos, llvm::AtomicOrdering::Release, deps) {}

	DEFINE_CREATE_CLONE(DskSyncLabel)

	virtual void reset() override {
		FenceLabel::reset();
		DskAccessLabel::reset();
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
	DskPbarrierLabel(Event pos, llvm::AtomicOrdering ord, const EventDeps &deps = EventDeps())
		: FenceLabel(EL_DskPbarrier, pos, ord, deps), DskAccessLabel(EL_DskPbarrier) {}

	DskPbarrierLabel(Event pos, const EventDeps &deps = EventDeps())
		: DskPbarrierLabel(pos, llvm::AtomicOrdering::Release, deps) {}

	DEFINE_CREATE_CLONE(DskPbarrierLabel)

	virtual void reset() override {
		FenceLabel::reset();
		DskAccessLabel::reset();
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
	SmpFenceLabelLKMM(Event pos, llvm::AtomicOrdering ord, SmpFenceType t,
			  const EventDeps &deps = EventDeps())
		: FenceLabel(EL_SmpFenceLKMM, pos, ::isStrong(t) ?
			     llvm::AtomicOrdering::SequentiallyConsistent :
			     llvm::AtomicOrdering::Monotonic, deps), type(t) {}

	static bool isType(const EventLabel *lab, SmpFenceType type) {
		if (auto *fLab = llvm::dyn_cast<SmpFenceLabelLKMM>(lab))
			return fLab->getType() == type;
		return false;
	}

	/* Returns the type of this fence */
	SmpFenceType getType() const { return type; }

	/* Returns true if this fence is cumulative */
	bool isCumul() const { return ::isCumul(getType()); }

	/* Returns true if this fence is a strong fence */
	bool isStrong() const { return ::isStrong(getType()); }

	DEFINE_CREATE_CLONE(SmpFenceLabelLKMM)

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
	RCUSyncLabelLKMM(Event pos, const EventDeps &deps = EventDeps())
		: FenceLabel(EL_RCUSyncLKMM, pos, llvm::AtomicOrdering::SequentiallyConsistent,
			     deps) {}

	DEFINE_CREATE_CLONE(RCUSyncLabelLKMM)

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
	ThreadCreateLabel(Event pos, llvm::AtomicOrdering ord, ThreadInfo childInfo,
			  const EventDeps &deps = EventDeps())
		: EventLabel(EL_ThreadCreate, pos, ord, deps), childInfo(childInfo) {}
	ThreadCreateLabel(Event pos, ThreadInfo childInfo, const EventDeps &deps = EventDeps())
		: ThreadCreateLabel(pos, llvm::AtomicOrdering::Release, childInfo, deps) {}

	/* Getters for the created thread's info */
	const ThreadInfo &getChildInfo() const { return childInfo; }
	ThreadInfo &getChildInfo() { return childInfo; }

	/* Getter/setter for the identifier of the created thread */
	unsigned int getChildId() const { return getChildInfo().id; }
	void setChildId(unsigned int tid) { getChildInfo().id = tid; }

	DEFINE_CREATE_CLONE(ThreadCreateLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadCreate; }

private:
	/* Information about the child thread */
	ThreadInfo childInfo;
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
	ThreadJoinLabel(Event pos, llvm::AtomicOrdering ord, unsigned int childId,
			const EventDeps &deps = EventDeps())
		: EventLabel(EL_ThreadJoin, pos, ord, deps), childId(childId) {}
	ThreadJoinLabel(Event pos, unsigned int childId, const EventDeps &deps = EventDeps())
		: ThreadJoinLabel(pos, llvm::AtomicOrdering::Acquire, childId, deps) {}

	/* Returns the identifier of the thread this join() is waiting on */
	unsigned int getChildId() const { return childId; }

	DEFINE_CREATE_CLONE(ThreadJoinLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadJoin; }

private:
	/* The identifier of the child */
	const unsigned int childId;
};


/*******************************************************************************
 **                     ThreadKillLabel Class
 ******************************************************************************/

/* Represents the abnormal termination of a thread */
class ThreadKillLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	ThreadKillLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_ThreadKill, pos, llvm::AtomicOrdering::NotAtomic, deps) {}

	DEFINE_CREATE_CLONE(ThreadKillLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadKill; }
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

	ThreadStartLabel(EventLabelKind kind, Event pos, Event pc)
		: EventLabel(kind, pos, llvm::AtomicOrdering::Acquire, EventDeps()),
		  parentCreate(pc), threadInfo() {}

public:
	ThreadStartLabel(Event pos, llvm::AtomicOrdering ord, Event pc,
			 ThreadInfo tinfo, int symm = -1)
		: EventLabel(EL_ThreadStart, pos, ord, EventDeps()),
		  parentCreate(pc), threadInfo(tinfo), symmetricTid(symm) {}
	ThreadStartLabel(Event pos, Event pc, ThreadInfo tinfo, int symm = -1)
		: ThreadStartLabel(pos, llvm::AtomicOrdering::Acquire, pc, tinfo, symm) {}

	/* Returns the position of the corresponding create operation */
	Event getParentCreate() const { return parentCreate; }

	/* Getters for the thread's info */
	const ThreadInfo &getThreadInfo() const { return threadInfo; }
	ThreadInfo &getThreadInfo() { return threadInfo; }

	/* SR: Returns the id of a symmetric thread, or -1 if no symmetric thread exists  */
	int getSymmetricTid() const { return symmetricTid; }

	DEFINE_CREATE_CLONE(ThreadStartLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_ThreadStart && k <= EL_ThreadStartEnd; }

private:
	/* The position of the corresponding create opeartion */
	Event parentCreate;

	/* Information about this thread */
	ThreadInfo threadInfo;

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
	ThreadFinishLabel(Event pos, llvm::AtomicOrdering ord, SVal retVal)
		: EventLabel(EL_ThreadFinish, pos, ord, EventDeps()), retVal(retVal) {}

	ThreadFinishLabel(Event pos, SVal retVal)
		: ThreadFinishLabel(pos, llvm::AtomicOrdering::Release, retVal) {}

	/* Returns the join() operation waiting on this thread or
	   NULL if no such operation exists (yet) */
	ThreadJoinLabel *getParentJoin() const { return parentJoin; }

	/* Sets the corresponding join() event */
	void setParentJoin(ThreadJoinLabel *jLab) { parentJoin = jLab; }

	/* Returns the return value of this thread */
	SVal getRetVal() const { return retVal; }

	DEFINE_CREATE_CLONE(ThreadFinishLabel)

	virtual void reset() override {
		EventLabel::reset();
		parentJoin = nullptr;
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_ThreadFinish; }

private:

	/* Position of corresponding join() event in the graph
	 * (NULL if such event does not exist) */
	ThreadJoinLabel *parentJoin = nullptr;

	/* Return value of the thread */
	SVal retVal;
};


/*******************************************************************************
 **                        MallocLabel Class
 ******************************************************************************/

class FreeLabel;

/* Corresponds to a memory-allocating operation (e.g., malloc()) */
class MallocLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	MallocLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, unsigned int size,
		    unsigned alignment, StorageDuration sd, StorageType stype, AddressSpace spc,
		    const NameInfo *info, const std::string &name, const EventDeps &deps = EventDeps())
		: EventLabel(EL_Malloc, pos, ord, deps),
		  allocAddr(addr), allocSize(size), alignment(alignment),
		  sdur(sd), stype(stype), spc(spc), nameInfo(info), name(name) {}
	MallocLabel(Event pos, SAddr addr, unsigned int size, unsigned alignment,
		    StorageDuration sd, StorageType stype, AddressSpace spc,
		    const NameInfo *info = nullptr, const std::string &name = {},
		    const EventDeps &deps = EventDeps())
		: MallocLabel(pos, llvm::AtomicOrdering::NotAtomic, addr, size, alignment, sd,
			      stype, spc, info, name, deps) {}
	MallocLabel(Event pos, unsigned int size, unsigned alignment, StorageDuration sd,
		    StorageType stype, AddressSpace spc, const NameInfo *info = nullptr,
		    const std::string &name = {}, const EventDeps &deps = EventDeps())
		: MallocLabel(pos, SAddr(), size, alignment, sd, stype, spc, info, name, deps) {}
	MallocLabel(Event pos, unsigned int size, unsigned alignment, StorageDuration sd,
		    StorageType stype, AddressSpace spc, const EventDeps &deps = EventDeps())
		: MallocLabel(pos, size, alignment, sd, stype, spc, nullptr, {}, deps) {}

	DEFINE_CREATE_CLONE(MallocLabel)

	/* Getter/setter for the (fresh) address returned by the allocation */
	SAddr getAllocAddr() const { return allocAddr; }
	void setAllocAddr(SAddr addr) { allocAddr = addr; }

	/* Getter/setter for the corresponding free label*/
	FreeLabel *getFree() const { return dLab; }
	FreeLabel *getFree() { return dLab; }
	void setFree(FreeLabel *lab) { dLab = lab; }

	/* Iterators for accesses */
	using AccessList = CopyableIList<MemAccessLabel>;
	using access_iterator = AccessList::iterator;
	using const_access_iterator = AccessList::const_iterator;
	using access_range = llvm::iterator_range<access_iterator>;
	using const_access_range = llvm::iterator_range<const_access_iterator>;

	access_iterator accesses_begin() { return accessList.begin(); }
	access_iterator accesses_end() { return accessList.end(); }
	access_range accesses() {
		return access_range(accesses_begin(), accesses_end());
	}
	const_access_iterator accesses_begin() const { return accessList.begin(); }
	const_access_iterator accesses_end() const { return accessList.end(); }
	const_access_range accesses() const {
		return const_access_range(accesses_begin(), accesses_end());
	}

	/* Returns the size of this allocation */
	unsigned int getAllocSize() const { return allocSize; }

	/* Returns true if ADDR is contained within the allocated block */
	bool contains(SAddr addr) const {
		return getAllocAddr() <= addr && addr < getAllocAddr() + getAllocSize();
	}

	/* Returns the alignment of this allocation */
	unsigned int getAlignment() const { return alignment; }

	/* Returns the storage duration of this allocation */
	StorageDuration getStorageDuration() const { return sdur; }

	/* Returns the storage type of this allocation */
	StorageType getStorageType() const { return stype; }

	/* Returns the address space of this allocation */
	AddressSpace getAddressSpace() const { return spc; }

	/* Returns the name of the variable allocated */
	const std::string &getName() const { return name; }

	/* Returns the naming info associated with this allocation.
	 * Returns null if no such info is found. */
	const NameInfo *getNameInfo() const { return nameInfo; }

	virtual void reset() override {
		EventLabel::reset();
		dLab = nullptr;
		accessList.clear();
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Malloc; }

private:
	void addAccess(MemAccessLabel *mLab) {
		BUG_ON(std::find_if(accesses_begin(), accesses_end(), [mLab](auto &oLab){
			return oLab.getPos() == mLab->getPos(); }) != accesses_end());
		accessList.push_back(*mLab);
	}

	template <typename F>
	void removeAccess(F cond) {
		for (auto it = accesses_begin(); it != accesses_end(); ) {
			if (cond(*it))
				it = accessList.erase(it);
			else
				++it;
		}
	}

	/* The address returned by malloc() */
	SAddr allocAddr;

	/* The corresponding free label (if it exists) */
	FreeLabel *dLab = nullptr;

	/* Accesses on the allocated location */
	AccessList accessList;

	/* The size of the requested allocation */
	unsigned int allocSize;

	/* Allocation alignment */
	unsigned int alignment;

	/* Storage duration */
	StorageDuration sdur;

	/* Storage type */
	StorageType stype;

	/* Address space */
	AddressSpace spc;

	/* Name of the variable allocated */
	std::string name;

	/* Naming information for this allocation */
	const NameInfo *nameInfo;
};


/*******************************************************************************
 **                         FreeLabel Class
 ******************************************************************************/

/* Corresponds to a memory-freeing operation (e.g., free()) */
class FreeLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	FreeLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr,
		  unsigned int size, const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), freeAddr(addr), freedSize(size) {}
	FreeLabel(EventLabelKind k, Event pos, SAddr addr, unsigned int size,
		  const EventDeps &deps = EventDeps())
		: FreeLabel(k, pos, llvm::AtomicOrdering::NotAtomic, addr, size, deps) {}
public:
	FreeLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, unsigned int size,
		  const EventDeps &deps = EventDeps())
		: FreeLabel(EL_Free, pos, ord, addr, size, deps) {}
	FreeLabel(Event pos, SAddr addr, unsigned int size, const EventDeps &deps = EventDeps())
		: FreeLabel(pos, llvm::AtomicOrdering::NotAtomic, addr, size, deps) {}
	FreeLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: FreeLabel(pos, addr, 0, deps) {}

	/* Returns the address being freed */
	SAddr getFreedAddr() const { return freeAddr; }

	/* Getter/setter for the size of the memory freed */
	unsigned int getFreedSize() const { return freedSize; }
	void setFreedSize(unsigned int size) { freedSize = size; }

	/* Getter/setter for the corresponding allocating event */
	MallocLabel *getAlloc() const { return aLab; }
	MallocLabel *getAlloc() { return aLab; }
	void setAlloc(MallocLabel *lab) { aLab = lab; }

	/* Returns true if ADDR is contained within the deallocated block */
	bool contains(SAddr addr) const {
		return getFreedAddr() <= addr && addr < getFreedAddr() + getFreedSize();
	}

	DEFINE_CREATE_CLONE(FreeLabel)

	virtual void reset() override {
		EventLabel::reset();
		aLab = nullptr;
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k >= EL_Free && k <= EL_FreeLast; }

private:
	/* The address of the memory freed */
	SAddr freeAddr;

	/* The size of the memory freed */
	unsigned int freedSize;

	/* The corresponding allocation */
	MallocLabel *aLab = nullptr;
};


/*******************************************************************************
 **                         HpRetireLabel Class
 ******************************************************************************/

/* Corresponds to a hazptr retire operation */
class HpRetireLabel : public FreeLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	HpRetireLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, unsigned int size,
		      const EventDeps &deps = EventDeps())
		: FreeLabel(EL_HpRetire, pos, ord, addr, size, deps) {}
	HpRetireLabel(Event pos, SAddr addr, unsigned int size,
		      const EventDeps &deps = EventDeps())
		: HpRetireLabel(pos, llvm::AtomicOrdering::NotAtomic, addr, size, deps) {}
	HpRetireLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: HpRetireLabel(pos, addr, 0, deps) {}

	DEFINE_CREATE_CLONE(HpRetireLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_HpRetire; }
};


/*******************************************************************************
 **                         HpProtectLabel Class
 ******************************************************************************/

/* Specialization of writes for hazptr protect events */
class HpProtectLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	HpProtectLabel(Event pos, llvm::AtomicOrdering ord, SAddr hpAddr, SAddr protAddr,
		       const EventDeps &deps = EventDeps())
		: EventLabel(EL_HpProtect, pos, ord, deps),
		  hpAddr(hpAddr), protAddr(protAddr) {}
	HpProtectLabel(Event pos, SAddr hpAddr, SAddr protAddr, const EventDeps &deps = EventDeps())
		: HpProtectLabel(pos, llvm::AtomicOrdering::Release, hpAddr, protAddr, deps) {}

	/* Getters for HP/protected address */
	SAddr getHpAddr() const { return hpAddr; }
	SAddr getProtectedAddr() const { return protAddr; }

	DEFINE_CREATE_CLONE(HpProtectLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_HpProtect; }

private:
	/* HP address */
	SAddr hpAddr;

	/* Protected address */
	SAddr protAddr;
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
	LockLabelLAPOR(Event pos, llvm::AtomicOrdering ord, SAddr addr,
		       const EventDeps &deps = EventDeps())
		: EventLabel(EL_LockLAPOR, pos, ord, deps), lockAddr(addr) {}
	LockLabelLAPOR(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: LockLabelLAPOR(pos, llvm::AtomicOrdering::Acquire, addr, deps) {}

	/* Returns the address of the acquired lock */
	SAddr getLockAddr() const { return lockAddr; }

	DEFINE_CREATE_CLONE(LockLabelLAPOR)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_LockLAPOR; }

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
	UnlockLabelLAPOR(Event pos, llvm::AtomicOrdering ord, SAddr addr,
			 const EventDeps &deps = EventDeps())
		: EventLabel(EL_UnlockLAPOR, pos, ord, deps), lockAddr(addr) {}
	UnlockLabelLAPOR(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: UnlockLabelLAPOR(pos, llvm::AtomicOrdering::Release, addr, deps) {}

	/* Returns the address of the released lock */
	SAddr getLockAddr() const { return lockAddr; }

	DEFINE_CREATE_CLONE(UnlockLabelLAPOR)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_UnlockLAPOR; }

private:
	/* The address of the released lock */
	SAddr lockAddr;
};


/*******************************************************************************
 **                         HelpingCasLabel class
 ******************************************************************************/

/* In contrast to HelpedCAS, a HelpingCAS is a dummy event*/
class HelpingCasLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	HelpingCasLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			AType type, SVal exp, SVal swap, const EventDeps &deps = EventDeps())
		: EventLabel(EL_HelpingCas, pos, ord, deps), access(AAccess(addr, size, type)),
		  expected(exp), swapValue(swap) {}

	/* Returns the address of this access */
	SAddr getAddr() const { return access.getAddr(); }

	/* Returns the size (in bytes) of the access */
	ASize getSize() const { return access.getSize(); }

	/* Returns the type of the access */
	AType getType() const { return access.getType(); }

	/* Returns the packed access */
	AAccess getAccess() const { return access; }

	/* Returns the value that makes the supposed CAS succeed */
	SVal getExpected() const { return expected; }

	/* Returns the value that the supposed CAS writes */
	SVal getSwapVal() const { return swapValue; }

	DEFINE_CREATE_CLONE(HelpingCasLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_HelpingCas; }

private:
	/* The size of the access performed (in bytes) */
	AAccess access;

	/* CAS expected value */
	const SVal expected;

	/* CAS swap value */
	const SVal swapValue;
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
	DskOpenLabel(Event pos, llvm::AtomicOrdering ord, const std::string &fileName, SVal fd,
		     const EventDeps &deps = EventDeps())
		: EventLabel(EL_DskOpen, pos, ord, deps), fileName(fileName), fd(fd) {}
	DskOpenLabel(Event pos, const std::string &fileName, SVal fd = SVal(0),
		     const EventDeps &deps = EventDeps())
		: DskOpenLabel(pos, llvm::AtomicOrdering::Release, fileName, fd, deps) {}

	/* Returns the name of the opened file */
	const std::string &getFileName() const { return fileName; }

	/* Setter/getter for the file descriptor returned by open() */
	SVal getFd() const { return fd; }
	void setFd(SVal d) { fd = d; }

	DEFINE_CREATE_CLONE(DskOpenLabel)

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
	RCULockLabelLKMM(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_RCULockLKMM, pos, llvm::AtomicOrdering::Acquire, deps) {}

	DEFINE_CREATE_CLONE(RCULockLabelLKMM)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_RCULockLKMM; }
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
	RCUUnlockLabelLKMM(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(EL_RCUUnlockLKMM, pos, llvm::AtomicOrdering::Release, deps) {}

	DEFINE_CREATE_CLONE(RCUUnlockLabelLKMM)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_RCUUnlockLKMM; }
};


/*******************************************************************************
 **                         CLFlushLabel Class
 ******************************************************************************/

/* Represents a cache line flush */
class CLFlushLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	CLFlushLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr,
		     const EventDeps &deps = EventDeps())
		: EventLabel(EL_CLFlush, pos, ord, deps), addr(addr) {}
	CLFlushLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: CLFlushLabel(pos, llvm::AtomicOrdering::Monotonic, addr, deps) {}

	/* Returns a pointer to the addr on which the flush takes place */
	SAddr getAddr() const { return addr; }

	DEFINE_CREATE_CLONE(CLFlushLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_CLFlush; }

private:
	SAddr addr;
};


/*******************************************************************************
 **                          InitLabel Class
 ******************************************************************************/

/* Represents the INIT label of the graph, modeling the initialization of all
 * memory locaitons. The first thread is special in that it does not start with
 * a ThreadStartLabel as the other threads do */
class InitLabel : public ThreadStartLabel {

private:
	using ReaderList = CopyableIList<ReadLabel>;

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	InitLabel() : ThreadStartLabel(EL_Init, Event::getInit(), Event::getInit()) {}

	using rf_iterator = ReaderList::iterator;
	using const_rf_iterator = ReaderList::const_iterator;

	rf_iterator rf_begin(SAddr addr) { return initRfs[addr].begin(); }
	const_rf_iterator rf_begin(SAddr addr) const { return initRfs.at(addr).begin(); };
	rf_iterator rf_end(SAddr addr) { return initRfs[addr].end(); }
	const_rf_iterator rf_end(SAddr addr) const { return initRfs.at(addr).end(); }

	DEFINE_CREATE_CLONE(InitLabel)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k) { return k == EL_Init; }

private:
	void addReader(ReadLabel *rLab) {
		BUG_ON(std::find_if(rf_begin(rLab->getAddr()), rf_end(rLab->getAddr()), [rLab](ReadLabel &oLab){
			return oLab.getPos() == rLab->getPos(); }) != rf_end(rLab->getAddr()));
		initRfs[rLab->getAddr()].push_back(*rLab);
	}

	/* Removes all readers that satisfy predicate F */
	template <typename F>
	void removeReader(SAddr addr, F cond) {
		for (auto it = rf_begin(addr); it != rf_end(addr); ) {
			if (cond(*it))
				it = initRfs[addr].erase(it);
			else
				++it;
		}
	}

	std::unordered_map<SAddr, ReaderList> initRfs;
};

inline bool EventLabel::isStable() const
{
	auto *mLab = llvm::dyn_cast<MemAccessLabel>(this);
	return !isRevisitable() || (mLab && !mLab->wasAddedMax());
}

/*******************************************************************************
 **                             Static methods
 *******************************************************************************/

inline EventLabel *EventLabel::castFromDskAccessLabel (const DskAccessLabel *D)
{
	EventLabel::EventLabelKind DK = D->getEventLabelKind();
	switch (DK) {
	case EventLabel::EventLabelKind::EL_DskRead:
		return static_cast<DskReadLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskWrite:
		return static_cast<DskWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskMdWrite:
		return static_cast<DskMdWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskJnlWrite:
		return static_cast<DskJnlWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskDirWrite:
		return static_cast<DskDirWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskSync:
		return static_cast<DskSyncLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskFsync:
		return static_cast<DskFsyncLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskPbarrier:
		return static_cast<DskPbarrierLabel *>(const_cast<DskAccessLabel *>(D));
	default:
		BUG();
	}
}

inline DskAccessLabel *EventLabel::castToDskAccessLabel(const EventLabel *E)
{
	EventLabel::EventLabelKind EK = E->getKind();
	switch (EK) {
	case EventLabel::EventLabelKind::EL_DskRead:
		return static_cast<DskReadLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskWrite:
		return static_cast<DskWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskMdWrite:
		return static_cast<DskMdWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskJnlWrite:
		return static_cast<DskJnlWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskDirWrite:
		return static_cast<DskDirWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskSync:
		return static_cast<DskSyncLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskFsync:
		return static_cast<DskFsyncLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskPbarrier:
		return static_cast<DskPbarrierLabel *>(const_cast<EventLabel *>(E));
	default:
		BUG();
	}
}


/*******************************************************************************
 **                             RTTI helpers
 *******************************************************************************/

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


llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
			      const llvm::AtomicOrdering o);
llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
			      const EventLabel::EventLabelKind k);
llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const SmpFenceType t);

#endif /* __EVENTLABEL_HPP__ */
