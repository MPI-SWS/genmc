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

#ifndef GENMC_EVENTLABEL_HPP
#define GENMC_EVENTLABEL_HPP

#include "ADT/DepView.hpp"
#include "ADT/VSet.hpp"
#include "ADT/View.hpp"
#include "ADT/value_ptr.hpp"
#include "ExecutionGraph/DepInfo.hpp"
#include "ExecutionGraph/Event.hpp"
#include "ExecutionGraph/EventAttr.hpp"
#include "ExecutionGraph/Stamp.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "Static/ModuleID.hpp"
#include "Support/MemAccess.hpp"
#include "Support/NameInfo.hpp"
#include "Support/SAddr.hpp"
#include "Support/SExpr.hpp"
#include "Support/SVal.hpp"
#include "Support/ThreadInfo.hpp"
#include <llvm/ADT/ilist_node.h>
#include <llvm/IR/Instructions.h> /* For AtomicOrdering in older LLVMs */
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <optional>

class DskAccessLabel;
class ReadLabel;
class MallocLabel;
class FreeLabel;
class ThreadJoinLabel;
class ExecutionGraph;

template <typename T, class... Options>
class CopyableIList : public llvm::simple_ilist<T, Options...> {

	using BaseT = llvm::simple_ilist<T, Options...>;

public:
	CopyableIList() = default;
	CopyableIList(const CopyableIList &other) : BaseT(BaseT()) {}
	CopyableIList(CopyableIList &&other) = default;

	CopyableIList &operator=(const CopyableIList &other) { *this = std::move(BaseT()); }
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
class EventLabel : public llvm::ilist_node<EventLabel> {

public:
	/* Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * It is public to allow clients perform a switch() on it */
	enum EventLabelKind {
#define HANDLE_LABEL(NUM, NAME) NAME = NUM,
#include "ExecutionGraph/EventLabel.def"
	};

protected:
	EventLabel(EventLabelKind k, Event p, llvm::AtomicOrdering o,
		   const EventDeps &deps = EventDeps())
		: kind(k), position(p), ordering(o), deps(deps)
	{}

	using const_dep_iterator = DepInfo::const_iterator;
	using const_dep_range = llvm::iterator_range<const_dep_iterator>;

	using calc_const_iterator = VSet<Event>::const_iterator;
	using calc_const_range = llvm::iterator_range<calc_const_iterator>;

public:
	virtual ~EventLabel() = default;

	/* Iterators for dependencies */
	const_dep_iterator data_begin() const { return deps.data.begin(); }
	const_dep_iterator data_end() const { return deps.data.end(); }
	const_dep_range data() const { return const_dep_range(deps.data.begin(), deps.data.end()); }

	const_dep_iterator addr_begin() const { return deps.addr.begin(); }
	const_dep_iterator addr_end() const { return deps.addr.end(); }
	const_dep_range addr() const { return const_dep_range(deps.addr.begin(), deps.addr.end()); }

	const_dep_iterator ctrl_begin() const { return deps.ctrl.begin(); }
	const_dep_iterator ctrl_end() const { return deps.ctrl.end(); }
	const_dep_range ctrl() const { return const_dep_range(deps.ctrl.begin(), deps.ctrl.end()); }

	/* Returns the discriminator of this object */
	EventLabelKind getKind() const { return kind; }

	/* Returns the parent graph of this label */
	const ExecutionGraph *getParent() const { return parent; }
	ExecutionGraph *getParent() { return parent; }

	/* Sets the parent graph for this label */
	void setParent(ExecutionGraph *graph) { parent = graph; }

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
	Stamp getStamp() const
	{
#ifdef ENABLE_GENMC_DEBUG
		return stamp.value();
#else
		return *stamp;
#endif
	}

	bool hasPrefixView() const { return prefixView.get() != nullptr; }
	const VectorClock &getPrefixView() const { return *prefixView; }
	VectorClock &getPrefixView() { return *prefixView; }
	void setPrefixView(std::unique_ptr<VectorClock> v) const { prefixView = std::move(v); }

	void setCalculated(std::vector<VSet<Event>> &&calc) { calculatedRels = std::move(calc); }

	void setViews(std::vector<View> &&views) { calculatedViews = std::move(views); }
	void addView(View &&view) { calculatedViews.emplace_back(view); }

	/* Iterators for calculated relations */
	calc_const_range calculated(size_t i) const
	{
		return (getPos().isInitializer() || getKind() == Empty) ? calculatedRels[0]
									: calculatedRels[i];
	}

	/* Getters for calculated views */
	const View &view(size_t i) const
	{
		return (getPos().isInitializer() || getKind() == Empty) ? calculatedViews[0]
									: calculatedViews[i];
	}

	/* Returns true if this label corresponds to a non-atomic access */
	bool isNotAtomic() const { return ordering == llvm::AtomicOrdering::NotAtomic; }

	/* Returns true if the ordering of this access is acquire or stronger */
	bool isAtLeastAcquire() const
	{
		return ordering == llvm::AtomicOrdering::Acquire ||
		       ordering == llvm::AtomicOrdering::AcquireRelease ||
		       ordering == llvm::AtomicOrdering::SequentiallyConsistent;
	}

	/* Returns true if the ordering of this access is acquire or weaker */
	bool isAtMostAcquire() const
	{
		return ordering == llvm::AtomicOrdering::NotAtomic ||
		       ordering == llvm::AtomicOrdering::Monotonic ||
		       ordering == llvm::AtomicOrdering::Acquire;
	}

	/* Returns true if the ordering of this access is release or stronger */
	bool isAtLeastRelease() const
	{
		return ordering == llvm::AtomicOrdering::Release ||
		       ordering == llvm::AtomicOrdering::AcquireRelease ||
		       ordering == llvm::AtomicOrdering::SequentiallyConsistent;
	}

	/* Returns true if the ordering of this access is release or weaker */
	bool isAtMostRelease() const
	{
		return ordering == llvm::AtomicOrdering::NotAtomic ||
		       ordering == llvm::AtomicOrdering::Monotonic ||
		       ordering == llvm::AtomicOrdering::Release;
	}

	/* Returns true if this is a sequentially consistent access */
	bool isSC() const { return ordering == llvm::AtomicOrdering::SequentiallyConsistent; }

	/* Whether this label can have outgoing dep edges */
	bool isDependable() const { return isDependable(getKind()); }

	/* Whether this label carries a value */
	bool hasValue() const { return hasValue(getKind()); }

	/* Whether this label has a location */
	bool hasLocation() const { return hasLocation(getKind()); }

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
	virtual void reset()
	{
		parent = nullptr;
		stamp = std::nullopt;
		calculatedRels.clear();
		calculatedViews.clear();
		prefixView = nullptr;
		revisitable = true;
	}

	friend llvm::raw_ostream &operator<<(llvm::raw_ostream &rhs, const EventLabel &lab);

private:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	static inline bool isDependable(EventLabelKind k);
	static inline bool hasValue(EventLabelKind k);
	static inline bool hasLocation(EventLabelKind k);

	void setStamp(Stamp s) { stamp = s; }

	/* Discriminator enum for LLVM-style RTTI */
	const EventLabelKind kind;

	ExecutionGraph *parent{};

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

#define DEFINE_CREATE_CLONE(name)                                                                  \
	template <typename... Ts> static std::unique_ptr<name##Label> create(Ts &&...params)       \
	{                                                                                          \
		return std::make_unique<name##Label>(std::forward<Ts>(params)...);                 \
	}                                                                                          \
                                                                                                   \
	std::unique_ptr<EventLabel> clone() const override                                         \
	{                                                                                          \
		return std::make_unique<name##Label>(*this);                                       \
	}

#define DEFINE_STANDARD_MEMBERS(name)                                                              \
	DEFINE_CREATE_CLONE(name)                                                                  \
                                                                                                   \
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }         \
	static bool classofKind(EventLabelKind k) { return k == name; }

/*******************************************************************************
 **                     ThreadStartLabel Class
 ******************************************************************************/

/* Represents the beginning of a thread. This label synchronizes with the
 * ThreadCreateLabel that led to the creation of this thread */
class ThreadStartLabel : public EventLabel {

protected:
	ThreadStartLabel(EventLabelKind kind, Event pos, Event pc)
		: EventLabel(kind, pos, llvm::AtomicOrdering::Acquire, EventDeps()),
		  parentCreate(pc), threadInfo()
	{}

public:
	ThreadStartLabel(Event pos, llvm::AtomicOrdering ord, Event pc, ThreadInfo tinfo,
			 int symm = -1)
		: EventLabel(ThreadStart, pos, ord, EventDeps()), parentCreate(pc),
		  threadInfo(tinfo), symmetricTid(symm)
	{}
	ThreadStartLabel(Event pos, Event pc, ThreadInfo tinfo, int symm = -1)
		: ThreadStartLabel(pos, llvm::AtomicOrdering::Acquire, pc, tinfo, symm)
	{}

	/* Returns the position of the corresponding create operation */
	Event getParentCreate() const { return parentCreate; }

	/* Getters for the thread's info */
	const ThreadInfo &getThreadInfo() const { return threadInfo; }
	ThreadInfo &getThreadInfo() { return threadInfo; }

	/* SR: Returns the id of a symmetric thread, or -1 if no symmetric thread exists  */
	int getSymmetricTid() const { return symmetricTid; }

	DEFINE_CREATE_CLONE(ThreadStart)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_BEGIN_LABEL(NUM) k >= NUM &&
#define LAST_BEGIN_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}

private:
	/* The position of the corresponding create opeartion */
	Event parentCreate;

	/* Information about this thread */
	ThreadInfo threadInfo;

	/* SR: The tid a symmetric thread (currently: minimum among all) */
	int symmetricTid = -1;
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
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	InitLabel() : ThreadStartLabel(Init, Event::getInit(), Event::getInit()) {}

	using rf_iterator = ReaderList::iterator;
	using const_rf_iterator = ReaderList::const_iterator;

	rf_iterator rf_begin(SAddr addr) { return initRfs[addr].begin(); }
	const_rf_iterator rf_begin(SAddr addr) const { return initRfs.at(addr).begin(); };
	rf_iterator rf_end(SAddr addr) { return initRfs[addr].end(); }
	const_rf_iterator rf_end(SAddr addr) const { return initRfs.at(addr).end(); }

	DEFINE_STANDARD_MEMBERS(Init)

private:
	void addReader(ReadLabel *rLab);

	/* Removes all readers that satisfy predicate F */
	template <typename F> void removeReader(SAddr addr, F cond)
	{
		for (auto it = rf_begin(addr); it != rf_end(addr);) {
			if (cond(*it))
				it = initRfs[addr].erase(it);
			else
				++it;
		}
	}

	std::unordered_map<SAddr, ReaderList> initRfs;
};

/*******************************************************************************
 **                          TerminatorLabel Class
 ******************************************************************************/

/* Abstract class for representing the termination of a thread */
class TerminatorLabel : public EventLabel {

protected:
	TerminatorLabel(EventLabelKind k, llvm::AtomicOrdering ord, Event pos)
		: EventLabel(k, pos, ord, EventDeps())
	{}

public:
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_TERM_LABEL(NUM) k >= NUM &&
#define LAST_TERM_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}
};

/*******************************************************************************
 **                            BlockLabel Class
 ******************************************************************************/

/* An abstract label that represents a blockage. Subclasses denote the blockage type */
class BlockLabel : public TerminatorLabel {

protected:
	BlockLabel(EventLabelKind k, Event pos)
		: TerminatorLabel(k, llvm::AtomicOrdering::NotAtomic, pos)
	{}

public:
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_BLOCK_LABEL(NUM) k >= NUM &&
#define LAST_BLOCK_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}
};

#define BLOCK_PURE_SUBCLASS(name)                                                                  \
	class name##Label : public BlockLabel {                                                    \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos) : BlockLabel(name, pos) {}                                  \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(name)                                                      \
	};

BLOCK_PURE_SUBCLASS(SpinloopBlock);
BLOCK_PURE_SUBCLASS(FaiZNEBlock);
BLOCK_PURE_SUBCLASS(LockZNEBlock);
BLOCK_PURE_SUBCLASS(HelpedCASBlock);
BLOCK_PURE_SUBCLASS(ConfirmationBlock);
BLOCK_PURE_SUBCLASS(LockNotAcqBlock);
BLOCK_PURE_SUBCLASS(LockNotRelBlock);
BLOCK_PURE_SUBCLASS(BarrierBlock);
BLOCK_PURE_SUBCLASS(ErrorBlock);
BLOCK_PURE_SUBCLASS(UserBlock);

/*
 * Represents that a thread cannot be scheduled until a child
 * one terminates (i.e., it is blocked due to a join()).
 * (Similar to ReadOptBlock below.)
 */
class JoinBlockLabel : public BlockLabel {

public:
	JoinBlockLabel(Event pos, unsigned childId) : BlockLabel(JoinBlock, pos), childId(childId)
	{}

	const unsigned &getChildId() const { return childId; }

	DEFINE_STANDARD_MEMBERS(JoinBlock)

private:
	const unsigned int childId{}; // the child waiting on
};

/*
 * A temporary block label (mostly used to optimize IPRs).  The
 * presence of such a label indicates that the corresponding thread
 * should not be considered for scheduling (though this may be
 * reconsidered whenever events in a given address are added).
 */
class ReadOptBlockLabel : public BlockLabel {

public:
	ReadOptBlockLabel(Event pos, SAddr addr) : BlockLabel(ReadOptBlock, pos), addr(addr) {}

	const SAddr &getAddr() const { return addr; }

	DEFINE_STANDARD_MEMBERS(ReadOptBlock)

private:
	SAddr addr; // the address waiting on
};

/*******************************************************************************
 **                     ThreadKillLabel Class
 ******************************************************************************/

/* Represents the abnormal termination of a thread */
class ThreadKillLabel : public TerminatorLabel {

public:
	ThreadKillLabel(Event pos)
		: TerminatorLabel(ThreadKill, llvm::AtomicOrdering::NotAtomic, pos)
	{}

	DEFINE_STANDARD_MEMBERS(ThreadKill)
};

/*******************************************************************************
 **                     ThreadFinishLabel Class
 ******************************************************************************/

/* Represents the ending of a thread. This label synchronizes with the
 * ThreadJoinLabel that awaits for this particular thread (if any)
 *
 * FIXME: no error is reported if multiple threads are waiting on the
 * same thread */
class ThreadFinishLabel : public TerminatorLabel {

public:
	ThreadFinishLabel(Event pos, llvm::AtomicOrdering ord, SVal retVal)
		: TerminatorLabel(ThreadFinish, ord, pos), retVal(retVal)
	{}

	ThreadFinishLabel(Event pos, SVal retVal)
		: ThreadFinishLabel(pos, llvm::AtomicOrdering::Release, retVal)
	{}

	/* Returns the join() operation waiting on this thread or
	   NULL if no such operation exists (yet) */
	ThreadJoinLabel *getParentJoin() const { return parentJoin; }

	/* Sets the corresponding join() event */
	void setParentJoin(ThreadJoinLabel *jLab) { parentJoin = jLab; }

	/* Returns the return value of this thread */
	SVal getRetVal() const { return retVal; }

	virtual void reset() override
	{
		EventLabel::reset();
		parentJoin = nullptr;
	}

	DEFINE_STANDARD_MEMBERS(ThreadFinish)

private:
	/* Position of corresponding join() event in the graph
	 * (NULL if such event does not exist) */
	ThreadJoinLabel *parentJoin = nullptr;

	/* Return value of the thread */
	SVal retVal;
};

/*******************************************************************************
 **                       MemAccessLabel Class (Abstract)
 ******************************************************************************/

/* This label abstracts the common functionality that loads and stores have
 * (e.g., asking for the address of such a label) */
class MemAccessLabel : public EventLabel, public llvm::ilist_node<MemAccessLabel> {

protected:
	MemAccessLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		       AType type, const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), access(loc, size, type)
	{}
	MemAccessLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, AAccess a,
		       const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), access(a)
	{}

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

	virtual void reset() override
	{
		EventLabel::reset();
		maximal = true;
		allocLab = nullptr;
	}

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_MEMORY_LABEL(NUM) k >= NUM &&
#define LAST_MEMORY_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
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
	ReadLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,
		  AType type, EventLabel *rfLab = nullptr, AnnotVP annot = nullptr,
		  const EventDeps &deps = EventDeps())
		: MemAccessLabel(k, pos, ord, loc, size, type, deps), readsFrom(rfLab),
		  annotExpr(std::move(annot))
	{}

public:
	ReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size, AType type,
		  EventLabel *rfLab, AnnotVP annot, const EventDeps &deps = EventDeps())
		: ReadLabel(Read, pos, ord, loc, size, type, rfLab, annot, deps)
	{}
	ReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size, AType type,
		  EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, type, rfLab, nullptr, deps)
	{}
	ReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size, AType type,
		  const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, type, nullptr, nullptr, deps)
	{}

	/* Returns the position of the write this read is readinf-from */
	EventLabel *getRf() const { return readsFrom; }
	EventLabel *getRf() { return readsFrom; }

	/* Whether this read has a set RF and reads externally */
	bool readsExt() const
	{
		return getRf() && !getRf()->getPos().isInitializer() &&
		       getRf()->getThread() != getThread();
	}

	/* Whether this read has a set RF and reads internally */
	bool readsInt() const
	{
		return getRf() &&
		       (getRf()->getPos().isInitializer() || getRf()->getThread() == getThread());
	}

	/* Returns true if the read was revisited in-place */
	bool isIPR() const { return ipr; }

	/* Sets the IPR status for this read */
	void setIPRStatus(bool status) { ipr = status; }

	/* Helper: Whether this is a confirmation read */
	bool isConfirming() const { return isConfirming(getKind()); }

	/* SAVer: Getter/setter for the annotation expression */
	const AnnotT *getAnnot() const { return annotExpr.get(); }
	void setAnnot(std::unique_ptr<AnnotT> annot) { annotExpr = std::move(annot); }

	virtual void reset() override
	{
		MemAccessLabel::reset();
		setRf(nullptr);
		ipr = false;
	}

	DEFINE_CREATE_CLONE(Read)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_READ_LABEL(NUM) k >= NUM &&
#define LAST_READ_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}

private:
	static inline bool isConfirming(EventLabelKind k);

	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	/* Changes the reads-from edge for this label. This should only
	 * be called from the execution graph to update other relevant
	 * information as well */
	void setRf(EventLabel *rfLab) { readsFrom = rfLab; }

	/* Position of the write it is reading from in the graph */
	EventLabel *readsFrom = nullptr;

	/* Whether the read has been revisited in place */
	bool ipr = false;

	/* SAVer: Expression for annotatable loads. This needs to have
	 * heap-value semantics so that it does not create concurrency issues */
	AnnotVP annotExpr = nullptr;
};

#define READ_PURE_SUBCLASS(name)                                                                   \
	class name##Label : public ReadLabel {                                                     \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,            \
			    AType type, EventLabel *rfLab, AnnotVP annot,                          \
			    const EventDeps &deps = EventDeps())                                   \
			: ReadLabel(name, pos, ord, loc, size, type, rfLab, std::move(annot),      \
				    deps)                                                          \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,            \
			    AType type, EventLabel *rfLab, const EventDeps &deps = EventDeps())    \
			: name##Label(pos, ord, loc, size, type, rfLab, nullptr, deps)             \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,            \
			    AType type, const EventDeps &deps = EventDeps())                       \
			: name##Label(pos, ord, loc, size, type, nullptr, nullptr, deps)           \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(name)                                                      \
	};

READ_PURE_SUBCLASS(SpeculativeRead);
READ_PURE_SUBCLASS(ConfirmingRead);
READ_PURE_SUBCLASS(BWaitRead);
READ_PURE_SUBCLASS(CondVarWaitRead);

/*******************************************************************************
 **                         FaiReadLabel Class
 ******************************************************************************/

/* Represents the read part of a read-modify-write (RMW) (e.g., fetch-and-add,
 * fetch-and-sub, etc) operation (compare-and-exhange is excluded) */
class FaiReadLabel : public ReadLabel {

protected:
	FaiReadLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		     AType type, llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,
		     EventLabel *rfLab, AnnotVP annot, const EventDeps &deps = EventDeps())
		: ReadLabel(k, pos, ord, addr, size, type, rfLab, std::move(annot), deps),
		  binOp(op), opValue(val), wattr(wattr)
	{}

public:
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab,
		     AnnotVP annot, const EventDeps &deps = EventDeps())
		: FaiReadLabel(FaiRead, pos, ord, addr, size, type, op, val, wattr, rfLab,
			       std::move(annot), deps)
	{}
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab,
		     const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, wattr, rfLab, nullptr, deps)
	{}
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,
		     const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, wattr, nullptr, deps)
	{}
	FaiReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     llvm::AtomicRMWInst::BinOp op, SVal val, const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, WriteAttr::None, deps)
	{}

	/* Returns the type of this RMW operation (e.g., add, sub) */
	llvm::AtomicRMWInst::BinOp getOp() const { return binOp; }

	/* Returns the other operand's value */
	SVal getOpVal() const { return opValue; }

	/* Returns/sets the attributes of the write part */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/* Checks whether the write part has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	virtual void reset() override
	{
		ReadLabel::reset();
		wattr &= ~(WriteAttr::RevBlocker);
	}

	DEFINE_CREATE_CLONE(FaiRead)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_FAI_READ_LABEL(NUM) k >= NUM &&
#define LAST_FAI_READ_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}

private:
	/* The binary operator for this RMW operation */
	const llvm::AtomicRMWInst::BinOp binOp;

	/* The other operand's value for the operation */
	SVal opValue;

	/* Attributes for the write part of the RMW */
	WriteAttr wattr = WriteAttr::None;
};

#define FAIREAD_PURE_SUBCLASS(name)                                                                \
	class name##Label : public FaiReadLabel {                                                  \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,  \
			    EventLabel *rfLab, AnnotVP annot, const EventDeps &deps = EventDeps()) \
			: FaiReadLabel(name, pos, ord, addr, size, type, op, val, wattr, rfLab,    \
				       std::move(annot), deps)                                     \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,  \
			    EventLabel *rfLab, const EventDeps &deps = EventDeps())                \
			: name##Label(pos, ord, addr, size, type, op, val, wattr, rfLab, nullptr,  \
				      deps)                                                        \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, llvm::AtomicRMWInst::BinOp op, SVal val, WriteAttr wattr,  \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, op, val, wattr, nullptr, deps)   \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, llvm::AtomicRMWInst::BinOp op, SVal val,                   \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, op, val, WriteAttr::None, deps)  \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(name)                                                      \
	};

FAIREAD_PURE_SUBCLASS(NoRetFaiRead);
FAIREAD_PURE_SUBCLASS(BIncFaiRead);

/*******************************************************************************
 **                         CasReadLabel Class
 ******************************************************************************/

/* Represents the read part of a compare-and-swap (CAS) operation */
class CasReadLabel : public ReadLabel {

protected:
	CasReadLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		     AType type, SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
		     AnnotVP annot, const EventDeps &deps = EventDeps())
		: ReadLabel(k, pos, ord, addr, size, type, rfLab, std::move(annot), deps),
		  wattr(wattr), expected(exp), swapValue(swap)
	{}

public:
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab, AnnotVP annot,
		     const EventDeps &deps = EventDeps())
		: CasReadLabel(CasRead, pos, ord, addr, size, type, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
		     const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp, swap, wattr, rfLab, nullptr, deps)
	{}
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     SVal exp, SVal swap, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp, swap, wattr, nullptr, deps)
	{}
	CasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		     SVal exp, SVal swap, const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp, swap, WriteAttr::None, deps)
	{}

	/* Returns the value that will make this CAS succeed */
	SVal getExpected() const { return expected; }

	/* Returns the value that will be written is the CAS succeeds */
	SVal getSwapVal() const { return swapValue; }

	/* Returns/sets the attributes of the write part */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/* Checks whether the write part has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	virtual void reset() override
	{
		ReadLabel::reset();
		wattr &= ~(WriteAttr::RevBlocker);
	}

	DEFINE_CREATE_CLONE(CasRead)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_CAS_READ_LABEL(NUM) k >= NUM &&
#define LAST_CAS_READ_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}

private:
	/* The value that will make this CAS succeed */
	const SVal expected;

	/* The value that will be written if the CAS succeeds */
	const SVal swapValue;

	/* The attributes of the write part of the RMW */
	WriteAttr wattr = WriteAttr::None;
};

#define CASREAD_PURE_SUBCLASS(name)                                                                \
	class name##Label : public CasReadLabel {                                                  \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,   \
			    AnnotVP annot, const EventDeps &deps = EventDeps())                    \
			: CasReadLabel(name, pos, ord, addr, size, type, exp, swap, wattr, rfLab,  \
				       std::move(annot), deps)                                     \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,   \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, exp, swap, wattr, rfLab,         \
				      nullptr, deps)                                               \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, SVal exp, SVal swap, WriteAttr wattr,                      \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, exp, swap, wattr, nullptr, deps) \
		{}                                                                                 \
		name##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,           \
			    AType type, SVal exp, SVal swap, const EventDeps &deps = EventDeps())  \
			: name                                                                     \
			  ##Label(pos, ord, addr, size, type, exp, swap, WriteAttr::None, deps)    \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(name)                                                      \
	};

CASREAD_PURE_SUBCLASS(HelpedCasRead);
CASREAD_PURE_SUBCLASS(ConfirmingCasRead);

/*******************************************************************************
 **                         LockCasReadLabel Class
 ******************************************************************************/

/* Specialization of CasReadLabel for lock CASes */
class LockCasReadLabel : public CasReadLabel {

public:
	LockCasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			 SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
			 AnnotVP annot = nullptr, const EventDeps &deps = EventDeps())
		: CasReadLabel(LockCasRead, pos, ord, addr, size, type, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			 AnnotVP annot = nullptr, const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, llvm::AtomicOrdering::Acquire, addr, size, AType::Signed,
				   SVal(0), SVal(1), wattr, rfLab, std::move(annot), deps)
	{}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr,
			 AnnotVP annot = nullptr, const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, addr, size, wattr, nullptr, std::move(annot), deps)
	{}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, AnnotVP annot = nullptr,
			 const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, addr, size, WriteAttr::None, std::move(annot), deps)
	{}

	DEFINE_STANDARD_MEMBERS(LockCasRead)
};

/*******************************************************************************
 **                         TrylockCasReadLabel Class
 ******************************************************************************/

/* Specialization of CasReadLabel for trylock CASes */
class TrylockCasReadLabel : public CasReadLabel {

public:
	TrylockCasReadLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			    SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
			    AnnotVP annot = nullptr, const EventDeps &deps = EventDeps())
		: CasReadLabel(TrylockCasRead, pos, ord, addr, size, type, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			    AnnotVP annot = nullptr, const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, llvm::AtomicOrdering::Acquire, addr, size, AType::Signed,
				      SVal(0), SVal(1), wattr, rfLab, std::move(annot), deps)
	{}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr,
			    AnnotVP annot = nullptr, const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, addr, size, wattr, nullptr, std::move(annot), deps)
	{}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, AnnotVP annot = nullptr,
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, addr, size, WriteAttr::None, std::move(annot), deps)
	{}

	DEFINE_STANDARD_MEMBERS(TrylockCasRead)
};

/*******************************************************************************
 **                         WriteLabel Class
 ******************************************************************************/

/* Represents a write operation. All special types of writes (e.g., FAI, CAS)
 * should inherit from this class */
class WriteLabel : public MemAccessLabel, public llvm::ilist_node<WriteLabel> {

protected:
	WriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		   AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: MemAccessLabel(k, pos, ord, addr, size, type, deps), value(val), wattr(wattr)
	{}
	WriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		   AType type, SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}

public:
	WriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		   SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: WriteLabel(Write, pos, ord, addr, size, type, val, wattr, deps)
	{}
	WriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		   SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}

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
	rf_range readers() { return rf_range(readers_begin(), readers_end()); }
	const_rf_iterator readers_begin() const { return readerList.begin(); }
	const_rf_iterator readers_end() const { return readerList.end(); }
	const_rf_range readers() const { return const_rf_range(readers_begin(), readers_end()); }

	virtual void reset() override
	{
		MemAccessLabel::reset();
		readerList.clear();
		wattr &= ~(WriteAttr::RevBlocker);
	}

	DEFINE_CREATE_CLONE(Write)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_WRITE_LABEL(NUM) k >= NUM &&
#define LAST_WRITE_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}

private:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	/* Adds a read to the list of reads reading from the write */
	void addReader(ReadLabel *rLab)
	{
		BUG_ON(std::find_if(readers_begin(), readers_end(), [rLab](ReadLabel &oLab) {
			       return oLab.getPos() == rLab->getPos();
		       }) != readers_end());
		readerList.push_back(*rLab);
	}

	/* Removes all readers that satisfy predicate F */
	template <typename F> void removeReader(F cond)
	{
		for (auto it = readers_begin(); it != readers_end();) {
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
	WriteAttr wattr = WriteAttr::None;
};

#define WRITE_PURE_SUBCLASS(_class_kind)                                                           \
	class _class_kind##Label : public WriteLabel {                                             \
                                                                                                   \
	public:                                                                                    \
		_class_kind##Label(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,     \
				   AType type, SVal val, WriteAttr wattr,                          \
				   const EventDeps &deps = EventDeps())                            \
			: WriteLabel(_class_kind, pos, ord, loc, size, type, val, wattr, deps)     \
		{}                                                                                 \
		_class_kind##Label(Event pos, llvm::AtomicOrdering ord, SAddr loc, ASize size,     \
				   AType type, SVal val, const EventDeps &deps = EventDeps())      \
			: _class_kind                                                              \
			  ##Label(pos, ord, loc, size, type, val, WriteAttr::None, deps)           \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(_class_kind)                                               \
	};

WRITE_PURE_SUBCLASS(UnlockWrite);
WRITE_PURE_SUBCLASS(BInitWrite);
WRITE_PURE_SUBCLASS(BDestroyWrite);
WRITE_PURE_SUBCLASS(CondVarInitWrite);
WRITE_PURE_SUBCLASS(CondVarSignalWrite);
WRITE_PURE_SUBCLASS(CondVarBcastWrite);
WRITE_PURE_SUBCLASS(CondVarDestroyWrite);

/*******************************************************************************
 **                         FaiWriteLabel Class
 ******************************************************************************/

/* Represents the write part of a read-modify-write (RMW) operation (e.g,
fetch-and-add, fetch-and-sub, etc) */
class FaiWriteLabel : public WriteLabel {

protected:
	FaiWriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		      AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, wattr, deps)
	{}

public:
	FaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		      SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(FaiWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}
	FaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		      SVal val, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}

	DEFINE_CREATE_CLONE(FaiWrite)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_FAI_WRITE_LABEL(NUM) k >= NUM &&
#define LAST_FAI_WRITE_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}
};

/*******************************************************************************
 **                         NoRetFaiWriteLabel Class
 ******************************************************************************/

/* Specialization of FaiWriteLabel for non-value-returning FAIs */
class NoRetFaiWriteLabel : public FaiWriteLabel {

public:
	NoRetFaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			   SVal val, WriteAttr wattr = WriteAttr::None,
			   const EventDeps &deps = EventDeps())
		: FaiWriteLabel(NoRetFaiWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}

	DEFINE_STANDARD_MEMBERS(NoRetFaiWrite)
};

/*******************************************************************************
 **                         BIncFaiWriteLabel Class
 ******************************************************************************/

/* Specialization of FaiWriteLabel for barrier FAIs */
class BIncFaiWriteLabel : public FaiWriteLabel {

public:
	BIncFaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			  SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(BIncFaiWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}
	BIncFaiWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			  SVal val, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(BIncFaiWrite, pos, ord, addr, size, type, val, WriteAttr::None,
				deps)
	{}

	DEFINE_STANDARD_MEMBERS(BIncFaiWrite)
};

/*******************************************************************************
 **                         CasWriteLabel Class
 ******************************************************************************/

/* Represents the write part of a compare-and-swap (CAS) operation */
class CasWriteLabel : public WriteLabel {

protected:
	CasWriteLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
		      AType type, SVal val, WriteAttr wattr = WriteAttr::None,
		      const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, wattr, deps)
	{}

public:
	CasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
		      SVal val, WriteAttr wattr = WriteAttr::None,
		      const EventDeps &deps = EventDeps())
		: CasWriteLabel(CasWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}

	DEFINE_CREATE_CLONE(CasWrite)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_CAS_WRITE_LABEL(NUM) k >= NUM &&
#define LAST_CAS_WRITE_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}
};

#define CASWRITE_PURE_SUBCLASS(_class_kind)                                                        \
	class _class_kind##Label : public CasWriteLabel {                                          \
                                                                                                   \
	public:                                                                                    \
		_class_kind##Label(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,    \
				   AType type, SVal val, WriteAttr wattr = WriteAttr::None,        \
				   const EventDeps &deps = EventDeps())                            \
			: CasWriteLabel(_class_kind, pos, ord, addr, size, type, val, wattr, deps) \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(_class_kind)                                               \
	};

CASWRITE_PURE_SUBCLASS(HelpedCasWrite);
CASWRITE_PURE_SUBCLASS(ConfirmingCasWrite);

/*******************************************************************************
 **                         LockCasWriteLabel Class
 ******************************************************************************/

/* Specialization of CasWriteLabel for lock CASes */
class LockCasWriteLabel : public CasWriteLabel {

public:
	LockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			  SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasWriteLabel(LockCasWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}
	LockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			  SVal val, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}
	LockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, llvm::AtomicOrdering::Acquire, addr, size, AType::Signed,
				    SVal(1), deps)
	{}

	DEFINE_STANDARD_MEMBERS(LockCasWrite)
};

/*******************************************************************************
 **                         TrylockCasWriteLabel Class
 ******************************************************************************/

/* Specialization of CasWriteLabel for trylock CASes */
class TrylockCasWriteLabel : public CasWriteLabel {

public:
	TrylockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			     AType type, SVal val, WriteAttr wattr,
			     const EventDeps &deps = EventDeps())
		: CasWriteLabel(TrylockCasWrite, pos, ord, addr, size, type, val, WriteAttr::None,
				deps)
	{}
	TrylockCasWriteLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size,
			     AType type, SVal val, const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}
	TrylockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, llvm::AtomicOrdering::Acquire, addr, size,
				       AType::Signed, SVal(1), deps)
	{}

	DEFINE_STANDARD_MEMBERS(TrylockCasWrite)
};

/*******************************************************************************
 **                         FenceLabel Class
 ******************************************************************************/

/* Represents a fence */
class FenceLabel : public EventLabel {

protected:
	FenceLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord,
		   const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps)
	{}

public:
	FenceLabel(Event pos, llvm::AtomicOrdering ord, const EventDeps &deps = EventDeps())
		: FenceLabel(Fence, pos, ord, deps)
	{}

	DEFINE_CREATE_CLONE(Fence)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_FENCE_LABEL(NUM) k >= NUM &&
#define LAST_FENCE_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}
};

/*******************************************************************************
 **                        MallocLabel Class
 ******************************************************************************/

/* Corresponds to a memory-allocating operation (e.g., malloc()) */
class MallocLabel : public EventLabel {

public:
	MallocLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, unsigned int size,
		    unsigned alignment, StorageDuration sd, StorageType stype, AddressSpace spc,
		    const NameInfo *info, const std::string &name,
		    const EventDeps &deps = EventDeps())
		: EventLabel(Malloc, pos, ord, deps), allocAddr(addr), allocSize(size),
		  alignment(alignment), sdur(sd), stype(stype), spc(spc), nameInfo(info), name(name)
	{}
	MallocLabel(Event pos, SAddr addr, unsigned int size, unsigned alignment,
		    StorageDuration sd, StorageType stype, AddressSpace spc,
		    const NameInfo *info = nullptr, const std::string &name = {},
		    const EventDeps &deps = EventDeps())
		: MallocLabel(pos, llvm::AtomicOrdering::NotAtomic, addr, size, alignment, sd,
			      stype, spc, info, name, deps)
	{}
	MallocLabel(Event pos, unsigned int size, unsigned alignment, StorageDuration sd,
		    StorageType stype, AddressSpace spc, const NameInfo *info = nullptr,
		    const std::string &name = {}, const EventDeps &deps = EventDeps())
		: MallocLabel(pos, SAddr(), size, alignment, sd, stype, spc, info, name, deps)
	{}
	MallocLabel(Event pos, unsigned int size, unsigned alignment, StorageDuration sd,
		    StorageType stype, AddressSpace spc, const EventDeps &deps = EventDeps())
		: MallocLabel(pos, size, alignment, sd, stype, spc, nullptr, {}, deps)
	{}

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
	access_range accesses() { return access_range(accesses_begin(), accesses_end()); }
	const_access_iterator accesses_begin() const { return accessList.begin(); }
	const_access_iterator accesses_end() const { return accessList.end(); }
	const_access_range accesses() const
	{
		return const_access_range(accesses_begin(), accesses_end());
	}

	/* Returns the size of this allocation */
	unsigned int getAllocSize() const { return allocSize; }

	/* Returns true if ADDR is contained within the allocated block */
	bool contains(SAddr addr) const
	{
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

	virtual void reset() override
	{
		EventLabel::reset();
		dLab = nullptr;
		accessList.clear();
	}

	DEFINE_CREATE_CLONE(Malloc)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_ALLOC_LABEL(NUM) k >= NUM &&
#define LAST_ALLOC_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}

private:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	void addAccess(MemAccessLabel *mLab)
	{
		BUG_ON(std::find_if(accesses_begin(), accesses_end(), [mLab](auto &oLab) {
			       return oLab.getPos() == mLab->getPos();
		       }) != accesses_end());
		accessList.push_back(*mLab);
	}

	template <typename F> void removeAccess(F cond)
	{
		for (auto it = accesses_begin(); it != accesses_end();) {
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
	unsigned int allocSize{};

	/* Allocation alignment */
	unsigned int alignment{};

	/* Storage duration */
	StorageDuration sdur;

	/* Storage type */
	StorageType stype;

	/* Address space */
	AddressSpace spc;

	/* Name of the variable allocated */
	std::string name;

	/* Naming information for this allocation */
	const NameInfo *nameInfo{};
};

/*******************************************************************************
 **                         FreeLabel Class
 ******************************************************************************/

/* Corresponds to a memory-freeing operation (e.g., free()) */
class FreeLabel : public EventLabel {

protected:
	FreeLabel(EventLabelKind k, Event pos, llvm::AtomicOrdering ord, SAddr addr,
		  unsigned int size, const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), freeAddr(addr), freedSize(size)
	{}
	FreeLabel(EventLabelKind k, Event pos, SAddr addr, unsigned int size,
		  const EventDeps &deps = EventDeps())
		: FreeLabel(k, pos, llvm::AtomicOrdering::NotAtomic, addr, size, deps)
	{}

public:
	FreeLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, unsigned int size,
		  const EventDeps &deps = EventDeps())
		: FreeLabel(Free, pos, ord, addr, size, deps)
	{}
	FreeLabel(Event pos, SAddr addr, unsigned int size, const EventDeps &deps = EventDeps())
		: FreeLabel(pos, llvm::AtomicOrdering::NotAtomic, addr, size, deps)
	{}
	FreeLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: FreeLabel(pos, addr, 0, deps)
	{}

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
	bool contains(SAddr addr) const
	{
		return getFreedAddr() <= addr && addr < getFreedAddr() + getFreedSize();
	}

	virtual void reset() override
	{
		EventLabel::reset();
		aLab = nullptr;
	}

	DEFINE_CREATE_CLONE(Free)

	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }
	static bool classofKind(EventLabelKind k)
	{
		return
#define FIRST_FREE_LABEL(NUM) k >= NUM &&
#define LAST_FREE_LABEL(NUM) k <= NUM;
#include "ExecutionGraph/EventLabel.def"
	}

private:
	/* The address of the memory freed */
	SAddr freeAddr;

	/* The size of the memory freed */
	unsigned int freedSize{};

	/* The corresponding allocation */
	MallocLabel *aLab{};
};

/*******************************************************************************
 **                         HpRetireLabel Class
 ******************************************************************************/

/* Corresponds to a hazptr retire operation */
class HpRetireLabel : public FreeLabel {

public:
	HpRetireLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, unsigned int size,
		      const EventDeps &deps = EventDeps())
		: FreeLabel(HpRetire, pos, ord, addr, size, deps)
	{}
	HpRetireLabel(Event pos, SAddr addr, unsigned int size, const EventDeps &deps = EventDeps())
		: HpRetireLabel(pos, llvm::AtomicOrdering::NotAtomic, addr, size, deps)
	{}
	HpRetireLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: HpRetireLabel(pos, addr, 0, deps)
	{}

	DEFINE_STANDARD_MEMBERS(HpRetire)
};

/*******************************************************************************
 **                     ThreadCreateLabel Class
 ******************************************************************************/

/* This label denotes the creation of a thread (via, e.g., pthread_create()) */
class ThreadCreateLabel : public EventLabel {

public:
	ThreadCreateLabel(Event pos, llvm::AtomicOrdering ord, ThreadInfo childInfo,
			  const EventDeps &deps = EventDeps())
		: EventLabel(ThreadCreate, pos, ord, deps), childInfo(childInfo)
	{}
	ThreadCreateLabel(Event pos, ThreadInfo childInfo, const EventDeps &deps = EventDeps())
		: ThreadCreateLabel(pos, llvm::AtomicOrdering::Release, childInfo, deps)
	{}

	/* Getters for the created thread's info */
	const ThreadInfo &getChildInfo() const { return childInfo; }
	ThreadInfo &getChildInfo() { return childInfo; }

	/* Getter/setter for the identifier of the created thread */
	unsigned int getChildId() const { return getChildInfo().id; }
	void setChildId(unsigned int tid) { getChildInfo().id = tid; }

	DEFINE_STANDARD_MEMBERS(ThreadCreate)

private:
	/* Information about the child thread */
	ThreadInfo childInfo;
};

/*******************************************************************************
 **                     ThreadJoinLabel Class
 ******************************************************************************/

/* Represents a join() operation (e.g., pthread_join()) */
class ThreadJoinLabel : public EventLabel {

public:
	ThreadJoinLabel(Event pos, llvm::AtomicOrdering ord, unsigned int childId,
			const EventDeps &deps = EventDeps())
		: EventLabel(ThreadJoin, pos, ord, deps), childId(childId)
	{}
	ThreadJoinLabel(Event pos, unsigned int childId, const EventDeps &deps = EventDeps())
		: ThreadJoinLabel(pos, llvm::AtomicOrdering::Acquire, childId, deps)
	{}

	/* Returns the identifier of the thread this join() is waiting on */
	unsigned int getChildId() const { return childId; }

	DEFINE_STANDARD_MEMBERS(ThreadJoin)

private:
	/* The identifier of the child */
	const unsigned int childId{};
};

/*******************************************************************************
 **                         HpProtectLabel Class
 ******************************************************************************/

/* Specialization of writes for hazptr protect events */
class HpProtectLabel : public EventLabel {

public:
	HpProtectLabel(Event pos, llvm::AtomicOrdering ord, SAddr hpAddr, SAddr protAddr,
		       const EventDeps &deps = EventDeps())
		: EventLabel(HpProtect, pos, ord, deps), hpAddr(hpAddr), protAddr(protAddr)
	{}
	HpProtectLabel(Event pos, SAddr hpAddr, SAddr protAddr, const EventDeps &deps = EventDeps())
		: HpProtectLabel(pos, llvm::AtomicOrdering::Release, hpAddr, protAddr, deps)
	{}

	/* Getters for HP/protected address */
	SAddr getHpAddr() const { return hpAddr; }
	SAddr getProtectedAddr() const { return protAddr; }

	DEFINE_STANDARD_MEMBERS(HpProtect)

private:
	/* HP address */
	SAddr hpAddr;

	/* Protected address */
	SAddr protAddr;
};

/*******************************************************************************
 **                         HelpingCasLabel class
 ******************************************************************************/

/* In contrast to HelpedCAS, a HelpingCAS is a dummy event*/
class HelpingCasLabel : public EventLabel {

public:
	HelpingCasLabel(Event pos, llvm::AtomicOrdering ord, SAddr addr, ASize size, AType type,
			SVal exp, SVal swap, const EventDeps &deps = EventDeps())
		: EventLabel(HelpingCas, pos, ord, deps), access(AAccess(addr, size, type)),
		  expected(exp), swapValue(swap)
	{}

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

	DEFINE_STANDARD_MEMBERS(HelpingCas)

private:
	/* The size of the access performed (in bytes) */
	AAccess access;

	/* CAS expected value */
	const SVal expected;

	/* CAS swap value */
	const SVal swapValue;
};

/*******************************************************************************
 **                            OptionalLabel Class
 ******************************************************************************/

/* A label that represents the beginning of an optional block */
class OptionalLabel : public EventLabel {

public:
	OptionalLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(Optional, pos, llvm::AtomicOrdering::NotAtomic, deps)
	{}

	/* Whether this block is expandable */
	bool isExpandable() const { return expandable; }
	void setExpandable(bool exp) { expandable = exp; }

	/* Whether this block has been expanded */
	bool isExpanded() const { return expanded; }
	void setExpanded(bool exp) { expanded = exp; }

	DEFINE_STANDARD_MEMBERS(Optional)

private:
	bool expandable = true;
	bool expanded = false;
};

/*******************************************************************************
 **                            Dummy subclasses
 ******************************************************************************/

#define DEFINE_DUMMY_SUBCLASS(_class_kind)                                                         \
	class _class_kind##Label : public EventLabel {                                             \
	public:                                                                                    \
		_class_kind##Label(Event pos, const EventDeps &deps = EventDeps())                 \
			: EventLabel(_class_kind, pos, llvm::AtomicOrdering::NotAtomic, deps)      \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(_class_kind)                                               \
	};

DEFINE_DUMMY_SUBCLASS(LoopBegin)
DEFINE_DUMMY_SUBCLASS(SpinStart)
DEFINE_DUMMY_SUBCLASS(FaiZNESpinEnd)
DEFINE_DUMMY_SUBCLASS(LockZNESpinEnd)
DEFINE_DUMMY_SUBCLASS(Empty)

/*******************************************************************************
 **                             Out-of-class definitions
 *******************************************************************************/

inline void InitLabel::addReader(ReadLabel *rLab)
{
	BUG_ON(std::find_if(rf_begin(rLab->getAddr()), rf_end(rLab->getAddr()),
			    [rLab](ReadLabel &oLab) { return oLab.getPos() == rLab->getPos(); }) !=
	       rf_end(rLab->getAddr()));
	initRfs[rLab->getAddr()].push_back(*rLab);
}

/*******************************************************************************
 **                             Static methods
 *******************************************************************************/

inline bool EventLabel::isStable() const
{
	auto *mLab = llvm::dyn_cast<MemAccessLabel>(this);
	return !isRevisitable() || (mLab && !mLab->wasAddedMax());
}

inline bool EventLabel::isDependable(EventLabelKind k)
{
	return ReadLabel::classofKind(k) || k == Malloc || k == Optional;
}

inline bool EventLabel::hasValue(EventLabelKind k)
{
	return ThreadStartLabel::classofKind(k) || ReadLabel::classofKind(k) || k == ThreadJoin ||
	       k == Optional;
}

inline bool EventLabel::hasLocation(EventLabelKind k) { return MemAccessLabel::classofKind(k); }

inline bool ReadLabel::isConfirming(EventLabelKind k)
{
	return ConfirmingReadLabel::classofKind(k) || ConfirmingCasReadLabel::classofKind(k);
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &rhs, const llvm::AtomicOrdering o);
llvm::raw_ostream &operator<<(llvm::raw_ostream &rhs, const EventLabel::EventLabelKind k);

#endif /* GENMC_EVENTLABEL_HPP */
