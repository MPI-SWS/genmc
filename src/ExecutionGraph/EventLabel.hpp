/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#ifndef GENMC_EVENTLABEL_HPP
#define GENMC_EVENTLABEL_HPP

#include "ADT/VSet.hpp"
#include "ADT/View.hpp"
#include "ADT/ilist.hpp"
#include "ADT/value_ptr.hpp"
#include "ExecutionGraph/DepInfo.hpp"
#include "ExecutionGraph/Event.hpp"
#include "ExecutionGraph/EventAttr.hpp"
#include "ExecutionGraph/LoadAnnotation.hpp"
#include "ExecutionGraph/Stamp.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "Support/Cast.hpp"
#include "Support/MemAccess.hpp"
#include "Support/MemOrdering.hpp"
#include "Support/NameInfo.hpp"
#include "Support/RMWOps.hpp"
#include "Support/SAddr.hpp"
#include "Support/SVal.hpp"
#include "Support/ThreadInfo.hpp"

#include <format>
#include <optional>
#include <ranges>

class ReadLabel;
class MallocLabel;
class FreeLabel;
class ThreadCreateLabel;
class ThreadJoinLabel;
class ExecutionGraph;

template <typename T, typename Tag = void> class CopyableIList : public genmc::ilist<T, Tag> {

	using BaseT = genmc::ilist<T, Tag>;

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

struct po_tag {};
struct io_tag {};

/**
 * An abstract class for modeling event labels. Contains the bare minimum
 * that all different labels (e.g., Reads, Writes, etc) should have. Although
 * hb and (po U rf) are stored as vector clocks in EventLabels, the respective
 * getter methods are private. One can obtain information about such relations
 * by querying the execution graph.
 */
class EventLabel : public genmc::ilist_node<EventLabel, io_tag>,
		   public genmc::ilist_node<EventLabel, po_tag> {

public:
	/* Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * It is public to allow clients perform a switch() on it */
	enum EventLabelKind {
#define HANDLE_LABEL(NAME) NAME,
#include "ExecutionGraph/EventLabel.def"
#define FIRST_LABEL(NAME, ARG) FIRST_##NAME = ARG,
#define LAST_LABEL(NAME, ARG) LAST_##NAME = ARG,
#include "ExecutionGraph/EventLabel.def"
	};

protected:
	EventLabel(EventLabelKind k, Event p, MemOrdering o, const EventDeps &deps = EventDeps())
		: kind(k), position(p), ordering(o), deps(deps)
	{}

public:
	virtual ~EventLabel() = default;

	/** Iterators for dependencies */
	auto data() const { return std::views::all(deps.data); }
	auto addr() const { return std::views::all(deps.addr); }
	auto ctrl() const { return std::views::all(deps.ctrl); }

	/** Returns the discriminator of this object */
	EventLabelKind getKind() const { return kind; }

	/** Returns the parent graph of this label */
	const ExecutionGraph *getParent() const { return parent; }
	ExecutionGraph *getParent() { return parent; }

	/** Sets the parent graph for this label */
	void setParent(ExecutionGraph *graph) { parent = graph; }

	/** Returns the position in the execution graph (thread, index) */
	Event getPos() const { return position; }
	Event &getPos() { return position; }

	/** Returns the index of this label within a thread */
	int getIndex() const { return position.index; }

	/** Returns the thread of this label in the execution graph */
	int getThread() const { return position.thread; }

	/** Getter for the label ordering */
	MemOrdering getOrdering() const { return ordering; }

	/** Setter for the label ordering */
	void setOrdering(MemOrdering ord) { ordering = ord; }

	/** Returns this label's dependencies */
	const EventDeps &getDeps() const { return deps; }

	/** Sets this label's dependencies */
	void setDeps(const EventDeps &ds) { deps = ds; }
	void setDeps(const EventDeps *ds) { deps = *ds; }
	void setDeps(EventDeps &&ds) { deps = std::move(ds); }

	/** Returns whether a stamp has been assigned for this label */
	bool hasStamp() const { return stamp.has_value(); }

	/** Returns the stamp of the label in a graph */
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

	/** Iterators for calculated relations */
	auto calculated(size_t i) const
	{
		return (getPos().isInitializer() || getKind() == Empty)
			       ? std::views::all(calculatedRels[0])
			       : std::views::all(calculatedRels[i]);
	}

	/** Getters for calculated views */
	const View &view(size_t i) const
	{
		return (getPos().isInitializer() || getKind() == Empty) ? calculatedViews[0]
									: calculatedViews[i];
	}

	/** Returns true if this label corresponds to a non-atomic access */
	bool isNotAtomic() const { return ordering == MemOrdering::NotAtomic; }

	/** Returns true if the ordering of this access is acquire or stronger */
	bool isAtLeastAcquire() const
	{
		return isAtLeastOrStrongerThan(ordering, MemOrdering::Acquire);
	}

	/** Returns true if the ordering of this access is release or stronger */
	bool isAtLeastRelease() const
	{
		return isAtLeastOrStrongerThan(ordering, MemOrdering::Release);
	}

	/** Returns true if this is a sequentially consistent access */
	bool isSC() const { return ordering == MemOrdering::SequentiallyConsistent; }

	/** Whether this label can have outgoing dep edges */
	bool isDependable() const { return isDependable(getKind()); }

	/** Whether this label returns a value */
	bool returnsValue() const { return returnsValue(getKind()); }

	/** Returns the value returned by the label */
	SVal getReturnValue() const;

	/** Returns whether this label accesses some value */
	bool accessesValue() const { return accessesValue(getKind()); }

	/** Returns the value from memory the label accesses.
	 * (The label needs to be a memory access.) */
	SVal getAccessValue(const AAccess &access) const;

	/** Whether this label has a location */
	bool hasLocation() const { return hasLocation(getKind()); }

	/** Returns true if this event can be revisited */
	bool isRevisitable() const { return revisitable; }

	/** Makes the relevant event revisitable/non-revisitable. The
	 * execution graph is responsible for making such changes */
	void setRevisitStatus(bool status) { revisitable = status; }

	/** Returns true if this event cannot be revisited or deleted */
	bool isStable() const;

	/** Necessary for multiple inheritance + LLVM-style RTTI to work */
	static bool classofKind(EventLabelKind K) { return true; }

	/** Returns a clone object (virtual to allow deep copying from base) */
	virtual std::unique_ptr<EventLabel> clone() const = 0;

	/** Resets all graph-related info on a label to their default values */
	virtual void reset()
	{
		parent = nullptr;
		stamp = std::nullopt;
		calculatedRels.clear();
		calculatedViews.clear();
		prefixView = nullptr;
		revisitable = true;
	}

private:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	static inline bool isDependable(EventLabelKind k);
	static inline bool returnsValue(EventLabelKind k);
	static inline bool accessesValue(EventLabelKind k);
	static inline bool hasLocation(EventLabelKind k);

	void setStamp(Stamp s) { stamp = s; }

	/** Discriminator enum for LLVM-style RTTI */
	const EventLabelKind kind;

	ExecutionGraph *parent{};

	/** Position of this label within the execution graph (thread, index) */
	Event position;

	/** Ordering of this access mode */
	MemOrdering ordering;

	/** Events on which this label depends */
	EventDeps deps;

	/** The stamp of this label in the execution graph */
	std::optional<Stamp> stamp = std::nullopt;

	mutable value_ptr<VectorClock, VectorClockCloner> prefixView = nullptr;

	/** Saved calculations */
	std::vector<VSet<Event>> calculatedRels;

	/** Saved views */
	std::vector<View> calculatedViews;

	/** Revisitability status */
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

#define DEFINE_CLASSOF_RANGE(name)                                                                 \
	static bool classof(const EventLabel *lab) { return classofKind(lab->getKind()); }         \
	static bool classofKind(EventLabelKind k) { return k >= FIRST_##name && k <= LAST_##name; }

#define DEFINE_STANDARD_MEMBERS_RANGE(name)                                                        \
	DEFINE_CREATE_CLONE(name)                                                                  \
	DEFINE_CLASSOF_RANGE(name)

/*******************************************************************************
 **                     ThreadStartLabel Class
 ******************************************************************************/

/** Represents the beginning of a thread. This label synchronizes with the
 * ThreadCreateLabel that led to the creation of this thread */
class ThreadStartLabel : public EventLabel {

protected:
	ThreadStartLabel(EventLabelKind kind, Event pos, Event createId,
			 ThreadCreateLabel *createLab)
		: EventLabel(kind, pos, MemOrdering::Acquire, EventDeps()), createId_(createId),
		  createLab_(createLab)
	{}

public:
	ThreadStartLabel(Event pos, MemOrdering ord, Event createId, ThreadCreateLabel *createLab,
			 ThreadInfo tinfo, int symmPred = -1)
		: EventLabel(ThreadStart, pos, ord, EventDeps()), createId_(createId),
		  createLab_(createLab), threadInfo(tinfo), symmPredTid(symmPred)
	{}
	ThreadStartLabel(Event pos, Event createId, ThreadCreateLabel *createLab, ThreadInfo tinfo,
			 int symmPred = -1)
		: ThreadStartLabel(pos, MemOrdering::Acquire, createId, createLab, tinfo, symmPred)
	{}

	/** Returns the position of the corresponding create operation */
	auto getCreate() const -> ThreadCreateLabel * { return createLab_; }
	auto getCreate() -> ThreadCreateLabel * { return createLab_; }
	auto setCreate(ThreadCreateLabel *lab) { createLab_ = lab; }
	auto getCreateId() const -> Event { return createId_; }

	/** Getters for the thread's info */
	auto getThreadInfo() const -> const ThreadInfo & { return threadInfo; }
	auto getThreadInfo() -> ThreadInfo & { return threadInfo; }

	/** SR: Returns the tid of the symmetric predecessor (-1 if it doesn't exist)  */
	auto getSymmPredTid() const -> int { return symmPredTid; }
	void setSymmPredTid(int tid) { symmPredTid = tid; }

	/** SR: Returns the tid of the symmetric successor (-1 if it doesn't exist) */
	auto getSymmSuccTid() const -> int { return symmSuccTid; }
	void setSymmSuccTid(int tid) { symmSuccTid = tid; }

	virtual void reset() override
	{
		EventLabel::reset();
		createLab_ = nullptr;
	}

	DEFINE_STANDARD_MEMBERS_RANGE(ThreadStart)

private:
	/** The position of the corresponding create opeartion */
	Event createId_;
	ThreadCreateLabel *createLab_;

	/** Information about this thread */
	ThreadInfo threadInfo;

	/** SR: The tid of the symmetric predecessor */
	int symmPredTid = -1;

	/** SR: The tid of the symmetric successor */
	int symmSuccTid = -1;
};

/*******************************************************************************
 **                          InitLabel Class
 ******************************************************************************/

/** Represents the INIT label of the graph, modeling the initialization of all
 * memory locations. The first thread is special in that it does not start with
 * a ThreadStartLabel as the other threads do */
class InitLabel : public ThreadStartLabel {

private:
	using ReaderList = CopyableIList<ReadLabel>;
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	InitLabel() : ThreadStartLabel(Init, Event::getInit(), Event::getInit(), nullptr) {}

	using rf_iterator = ReaderList::iterator;
	using const_rf_iterator = ReaderList::const_iterator;

	rf_iterator rf_begin(SAddr addr) { return initRfs[addr].begin(); }
	const_rf_iterator rf_begin(SAddr addr) const { return initRfs.at(addr).begin(); };
	rf_iterator rf_end(SAddr addr) { return initRfs[addr].end(); }
	const_rf_iterator rf_end(SAddr addr) const { return initRfs.at(addr).end(); }
	auto rfs(SAddr addr) const { return std::views::all(initRfs.at(addr)); }
	auto rfs(SAddr addr) { return std::views::all(initRfs.at(addr)); }

	DEFINE_STANDARD_MEMBERS(Init)

private:
	friend class ReadLabel; // FIXME: Robustify; no friendship necessary

	void addReader(ReadLabel *rLab);

	/** Removes all readers that satisfy predicate F */
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

/** Abstract class for representing the termination of a thread */
class TerminatorLabel : public EventLabel {

protected:
	TerminatorLabel(EventLabelKind k, MemOrdering ord, Event pos)
		: EventLabel(k, pos, ord, EventDeps())
	{}

public:
	DEFINE_CLASSOF_RANGE(Terminator)
};

/*******************************************************************************
 **                            BlockLabel Class
 ******************************************************************************/

/** An abstract label that represents a blockage. Subclasses denote the blockage type */
class BlockLabel : public TerminatorLabel {

protected:
	BlockLabel(EventLabelKind k, Event pos) : TerminatorLabel(k, MemOrdering::NotAtomic, pos) {}

public:
	static auto createAssumeBlock(Event pos, AssumeType type) -> std::unique_ptr<BlockLabel>;

	DEFINE_CLASSOF_RANGE(Block)
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
BLOCK_PURE_SUBCLASS(BarrierBlock);
BLOCK_PURE_SUBCLASS(ErrorBlock);
BLOCK_PURE_SUBCLASS(UserBlock);

/**
 * Represents that a thread cannot be scheduled until a child
 * one terminates (i.e., it is blocked due to a join()).
 * (Similar to ReadOptBlock below.)
 */
class JoinBlockLabel : public BlockLabel {

public:
	JoinBlockLabel(Event pos, unsigned childId) : BlockLabel(JoinBlock, pos), childId(childId)
	{}

	/** Returns the ID of the child waited on */
	const unsigned &getChildId() const { return childId; }

	DEFINE_STANDARD_MEMBERS(JoinBlock)

private:
	const unsigned int childId{}; // the child waiting on
};

/**
 * A temporary block label (mostly used to optimize IPRs).  The
 * presence of such a label indicates that the corresponding thread
 * should not be considered for scheduling (though this may be
 * reconsidered whenever events in a given address are added).
 */
class ReadOptBlockLabel : public BlockLabel {

public:
	ReadOptBlockLabel(Event pos, SAddr addr) : BlockLabel(ReadOptBlock, pos), addr(addr) {}

	/** Returns the address waited on */
	const SAddr &getAddr() const { return addr; }

	DEFINE_STANDARD_MEMBERS(ReadOptBlock)

private:
	SAddr addr;
};

/*******************************************************************************
 **                     ThreadKillLabel Class
 ******************************************************************************/

/** Represents the abnormal termination of a thread */
class ThreadKillLabel : public TerminatorLabel {

public:
	ThreadKillLabel(Event pos) : TerminatorLabel(ThreadKill, MemOrdering::NotAtomic, pos) {}

	DEFINE_STANDARD_MEMBERS(ThreadKill)
};

/*******************************************************************************
 **                     ThreadFinishLabel Class
 ******************************************************************************/

/** Represents the ending of a thread. This label synchronizes with the
 * ThreadJoinLabel that awaits for this particular thread (if any)
 *
 * FIXME: no error is reported if multiple threads are waiting on the
 * same thread */
class ThreadFinishLabel : public TerminatorLabel {

public:
	ThreadFinishLabel(Event pos, MemOrdering ord, SVal retVal)
		: TerminatorLabel(ThreadFinish, ord, pos), retVal(retVal)
	{}

	ThreadFinishLabel(Event pos, SVal retVal)
		: ThreadFinishLabel(pos, MemOrdering::Release, retVal)
	{}

	/** Returns the join() operation waiting on this thread or
	   NULL if no such operation exists (yet) */
	ThreadJoinLabel *getParentJoin() const { return parentJoin; }

	/** Sets the corresponding join() event */
	void setParentJoin(ThreadJoinLabel *jLab) { parentJoin = jLab; }

	/** Returns the return value of this thread */
	SVal getRetVal() const { return retVal; }

	virtual void reset() override
	{
		EventLabel::reset();
		parentJoin = nullptr;
	}

	DEFINE_STANDARD_MEMBERS(ThreadFinish)

private:
	/** Position of corresponding join() event in the graph
	 * (NULL if such event does not exist) */
	ThreadJoinLabel *parentJoin = nullptr;

	/** Return value of the thread */
	SVal retVal;
};

/*******************************************************************************
 **                       MemAccessLabel Class (Abstract)
 ******************************************************************************/

/** This label abstracts the common functionality that loads and stores have
 * (e.g., asking for the address of such a label) */
class MemAccessLabel : public EventLabel, public genmc::ilist_node<MemAccessLabel> {

protected:
	MemAccessLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr loc, ASize size,
		       AType type, const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), access(loc, size, type)
	{}
	MemAccessLabel(EventLabelKind k, Event pos, MemOrdering ord, AAccess a,
		       const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), access(a)
	{}

public:
	/** Returns the address of this access */
	SAddr getAddr() const { return access.getAddr(); }

	/** Returns the size (in bytes) of the access */
	ASize getSize() const { return access.getSize(); }

	/** Returns the type of the access */
	AType getType() const { return access.getType(); }

	/** Returns the packed access */
	const AAccess &getAccess() const { return access; }

	/** Helper flag for maximality checks */
	bool wasAddedMax() const { return maximal; }
	void setAddedMax(bool status) { maximal = status; }

	/** Getter for allocating event */
	MallocLabel *getAlloc() const { return allocLab; }
	MallocLabel *getAlloc() { return allocLab; }

	void setAlloc(MallocLabel *lab) { allocLab = lab; }

	virtual void reset() override
	{
		EventLabel::reset();
		maximal = true;
		allocLab = nullptr;
	}

	DEFINE_CLASSOF_RANGE(MemAccess)

private:
	/** The access performed */
	AAccess access;

	/** Whether was mo-maximal when added */
	bool maximal = true;

	/** The allocation event corresponding to this access */
	MallocLabel *allocLab = nullptr;
};

/*******************************************************************************
 **                         ReadLabel Class
 ******************************************************************************/

/** The label for reads. All special read types (e.g., FAI, CAS) should inherit
 * from this class */
class ReadLabel : public MemAccessLabel, public genmc::ilist_node<ReadLabel> {

protected:
	ReadLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,
		  EventLabel *rfLab = nullptr, std::optional<Annotation> annot = {},
		  const EventDeps &deps = EventDeps())
		: MemAccessLabel(k, pos, ord, loc, size, type, deps), readsFrom(rfLab),
		  annot_(std::move(annot))
	{}

public:
	ReadLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type, EventLabel *rfLab,
		  std::optional<Annotation> annot, const EventDeps &deps = EventDeps())
		: ReadLabel(Read, pos, ord, loc, size, type, rfLab, std::move(annot), deps)
	{}
	ReadLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type, EventLabel *rfLab,
		  const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, type, rfLab, std::nullopt, deps)
	{}
	ReadLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,
		  const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, type, nullptr, std::nullopt, deps)
	{}

	/** Returns the position of the write this read is reading-from */
	EventLabel *getRf() const { return readsFrom; }
	EventLabel *getRf() { return readsFrom; }

	/** Changes the reads-from edge for this label.
	 * Also updates reader information in the writer */
	void setRf(EventLabel *rfLab);

	/** Whether this read has a set RF and reads externally */
	bool readsExt() const
	{
		return getRf() && !getRf()->getPos().isInitializer() &&
		       getRf()->getThread() != getThread();
	}

	/** Whether this read has a set RF and reads internally */
	bool readsInt() const
	{
		return getRf() &&
		       (getRf()->getPos().isInitializer() || getRf()->getThread() == getThread());
	}

	/** Whether the read is part of an RMW operation (needs to be part of a graph) */
	bool isRMW() const;

	/** Convenience function that returns whether reading a value will create an RMW */
	bool valueMakesRMWSucceed(const SVal &val) const;

	/** Convenience function that returns whether reading a value makes the assume
	 * succeed */
	bool valueMakesAssumeSucceed(const SVal &val) const;

	/** Helper: Whether this is a confirmation read */
	bool isConfirming() const { return isConfirming(getKind()); }

	/** SAVer: Getter for the annotation expression */
	const std::optional<Annotation> &getAnnot() const { return annot_; }

	virtual void reset() override
	{
		MemAccessLabel::reset();
		setRfNoCascade(nullptr);
	}

	DEFINE_STANDARD_MEMBERS_RANGE(Read)

private:
	static inline bool isConfirming(EventLabelKind k);

	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	void setRfNoCascade(EventLabel *rfLab) { readsFrom = rfLab; }

	/** Position of the write it is reading from in the graph */
	EventLabel *readsFrom = nullptr;

	/** SAVer: Expression for annotatable loads. This needs to have
	 * heap-value semantics so that it does not create concurrency issues */
	std::optional<Annotation> annot_;
};

#define READ_PURE_SUBCLASS(name)                                                                   \
	class name##Label : public ReadLabel {                                                     \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,         \
			    EventLabel *rfLab, std::optional<Annotation> annot,                    \
			    const EventDeps &deps = EventDeps())                                   \
			: ReadLabel(name, pos, ord, loc, size, type, rfLab, std::move(annot),      \
				    deps)                                                          \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,         \
			    EventLabel *rfLab, const EventDeps &deps = EventDeps())                \
			: name##Label(pos, ord, loc, size, type, rfLab, std::nullopt, deps)        \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,         \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, loc, size, type, nullptr, std::nullopt, deps)      \
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

/** Represents the read part of a read-modify-write (RMW) (e.g., fetch-and-add,
 * fetch-and-sub, etc) operation (compare-and-exhange is excluded) */
class FaiReadLabel : public ReadLabel {

protected:
	FaiReadLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size,
		     AType type, RMWBinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab,
		     std::optional<Annotation> annot, const EventDeps &deps = EventDeps())
		: ReadLabel(k, pos, ord, addr, size, type, rfLab, std::move(annot), deps),
		  binOp(op), opValue(val), wattr(wattr)
	{}

public:
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, RMWBinOp op,
		     SVal val, WriteAttr wattr, EventLabel *rfLab, std::optional<Annotation> annot,
		     const EventDeps &deps = EventDeps())
		: FaiReadLabel(FaiRead, pos, ord, addr, size, type, op, val, wattr, rfLab,
			       std::move(annot), deps)
	{}
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, RMWBinOp op,
		     SVal val, WriteAttr wattr, EventLabel *rfLab,
		     const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, wattr, rfLab, std::nullopt,
			       deps)
	{}
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, RMWBinOp op,
		     SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, wattr, nullptr, deps)
	{}
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, RMWBinOp op,
		     SVal val, const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, type, op, val, WriteAttr::None, deps)
	{}

	/** Returns the type of this RMW operation (e.g., add, sub) */
	RMWBinOp getOp() const { return binOp; }

	/** Returns the other operand's value */
	SVal getOpVal() const { return opValue; }

	/** Returns/sets the attributes of the write part */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/** Checks whether the write part has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	virtual void reset() override { ReadLabel::reset(); }

	DEFINE_STANDARD_MEMBERS_RANGE(FaiRead)

private:
	/** The binary operator for this RMW operation */
	const RMWBinOp binOp;

	/** The other operand's value for the operation */
	SVal opValue;

	/** Attributes for the write part of the RMW */
	WriteAttr wattr = WriteAttr::None;
};

#define FAIREAD_PURE_SUBCLASS(name)                                                                \
	class name##Label : public FaiReadLabel {                                                  \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    RMWBinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab,             \
			    std::optional<Annotation> annot, const EventDeps &deps = EventDeps())  \
			: FaiReadLabel(name, pos, ord, addr, size, type, op, val, wattr, rfLab,    \
				       std::move(annot), deps)                                     \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    RMWBinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab,             \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, op, val, wattr, rfLab,           \
				      std::nullopt, deps)                                          \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    RMWBinOp op, SVal val, WriteAttr wattr,                                \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, op, val, wattr, nullptr, deps)   \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    RMWBinOp op, SVal val, const EventDeps &deps = EventDeps())            \
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

/** Represents the read part of a compare-and-swap (CAS) operation */
class CasReadLabel : public ReadLabel {

protected:
	CasReadLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size,
		     AType type, SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
		     std::optional<Annotation> annot, const EventDeps &deps = EventDeps())
		: ReadLabel(k, pos, ord, addr, size, type, rfLab, std::move(annot), deps),
		  expected(exp), swapValue(swap), wattr(wattr)
	{}

public:
	CasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal exp,
		     SVal swap, WriteAttr wattr, EventLabel *rfLab, std::optional<Annotation> annot,
		     const EventDeps &deps = EventDeps())
		: CasReadLabel(CasRead, pos, ord, addr, size, type, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	CasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal exp,
		     SVal swap, WriteAttr wattr, EventLabel *rfLab,
		     const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp, swap, wattr, rfLab, std::nullopt,
			       deps)
	{}
	CasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal exp,
		     SVal swap, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp, swap, wattr, nullptr, deps)
	{}
	CasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal exp,
		     SVal swap, const EventDeps &deps = EventDeps())
		: CasReadLabel(pos, ord, addr, size, type, exp, swap, WriteAttr::None, deps)
	{}

	/** Returns the value that will make this CAS succeed */
	SVal getExpected() const { return expected; }

	/** Returns the value that will be written is the CAS succeeds */
	SVal getSwapVal() const { return swapValue; }

	/** Returns/sets the attributes of the write part */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/** Checks whether the write part has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	virtual void reset() override { ReadLabel::reset(); }

	DEFINE_STANDARD_MEMBERS_RANGE(CasRead)

private:
	/** The value that will make this CAS succeed */
	const SVal expected;

	/** The value that will be written if the CAS succeeds */
	const SVal swapValue;

	/** The attributes of the write part of the RMW */
	WriteAttr wattr = WriteAttr::None;
};

#define CASREAD_PURE_SUBCLASS(name)                                                                \
	class name##Label : public CasReadLabel {                                                  \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,               \
			    std::optional<Annotation> annot, const EventDeps &deps = EventDeps())  \
			: CasReadLabel(name, pos, ord, addr, size, type, exp, swap, wattr, rfLab,  \
				       std::move(annot), deps)                                     \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,               \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, exp, swap, wattr, rfLab,         \
				      std::nullopt, deps)                                          \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    SVal exp, SVal swap, WriteAttr wattr,                                  \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, type, exp, swap, wattr, nullptr, deps) \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,        \
			    SVal exp, SVal swap, const EventDeps &deps = EventDeps())              \
			: name##Label(pos, ord, addr, size, type, exp, swap, WriteAttr::None,      \
				      deps)                                                        \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(name)                                                      \
	};

CASREAD_PURE_SUBCLASS(HelpedCasRead);
CASREAD_PURE_SUBCLASS(ConfirmingCasRead);

/*******************************************************************************
 **                         LockCasReadLabel Class
 ******************************************************************************/

/** Specialization of CasReadLabel for lock CASes */
class LockCasReadLabel : public CasReadLabel {

protected:
	LockCasReadLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size,
			 AType type, SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
			 std::optional<Annotation> annot = {}, const EventDeps &deps = EventDeps())
		: CasReadLabel(k, pos, ord, addr, size, type, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}

public:
	LockCasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal exp,
			 SVal swap, WriteAttr wattr, EventLabel *rfLab,
			 std::optional<Annotation> annot = {}, const EventDeps &deps = EventDeps())
		: CasReadLabel(LockCasRead, pos, ord, addr, size, type, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			 std::optional<Annotation> annot = {}, const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, MemOrdering::Acquire, addr, size, AType::Signed, SVal(0),
				   SVal(1), wattr, rfLab, std::move(annot), deps)
	{}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr,
			 std::optional<Annotation> annot = {}, const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, addr, size, wattr, nullptr, std::move(annot), deps)
	{}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, std::optional<Annotation> annot = {},
			 const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, addr, size, WriteAttr::None, std::move(annot), deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(LockCasRead)
};

/*******************************************************************************
 **                         TrylockCasReadLabel Class
 ******************************************************************************/

/** Specialization of CasReadLabel for trylock CASes */
class TrylockCasReadLabel : public CasReadLabel {

public:
	TrylockCasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,
			    SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
			    std::optional<Annotation> annot = {},
			    const EventDeps &deps = EventDeps())
		: CasReadLabel(TrylockCasRead, pos, ord, addr, size, type, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			    std::optional<Annotation> annot = {},
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, MemOrdering::Acquire, addr, size, AType::Signed, SVal(0),
				      SVal(1), wattr, rfLab, std::move(annot), deps)
	{}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr,
			    std::optional<Annotation> annot = {},
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, addr, size, wattr, nullptr, std::move(annot), deps)
	{}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, std::optional<Annotation> annot = {},
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, addr, size, WriteAttr::None, std::move(annot), deps)
	{}

	DEFINE_STANDARD_MEMBERS(TrylockCasRead)
};

/*******************************************************************************
 **                         AbstractLockCasReadLabel Class
 ******************************************************************************/

/** Special lock to enforce atomicity in the abstract specification */
class AbstractLockCasReadLabel : public LockCasReadLabel {
public:
	AbstractLockCasReadLabel(Event pos, SAddr addr, ASize size,
				 std::optional<Annotation> annot = {},
				 const EventDeps &deps = EventDeps(), EventLabel *rfLab = nullptr)
		: LockCasReadLabel(AbstractLockCasRead, pos, MemOrdering::Acquire, addr, size,
				   AType::Signed, SVal(0), SVal(1), WriteAttr::None, rfLab, annot,
				   deps)
	{}

	DEFINE_STANDARD_MEMBERS(AbstractLockCasRead)
};

/*******************************************************************************
 **                         WriteLabel Class
 ******************************************************************************/

/** Represents a write operation. All special types of writes (e.g., FAI, CAS)
 * should inherit from this class */
class WriteLabel : public MemAccessLabel, public genmc::ilist_node<WriteLabel> {

protected:
	WriteLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,
		   SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: MemAccessLabel(k, pos, ord, addr, size, type, deps), value(val), wattr(wattr)
	{}
	WriteLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,
		   SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}

public:
	WriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
		   WriteAttr wattr, const EventDeps &deps = EventDeps())
		: WriteLabel(Write, pos, ord, addr, size, type, val, wattr, deps)
	{}
	WriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
		   const EventDeps &deps = EventDeps())
		: WriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}

	/** Getter for the write value */
	SVal getVal() const { return value; }

	/** Setter for the write value */
	void setVal(SVal v) { value = v; }

	/** Returns the attributes of the write */
	WriteAttr getAttr() const { return wattr; }
	void setAttr(WriteAttr a) { wattr |= a; }

	/** Checks whether the write has the specified attributes */
	bool hasAttr(WriteAttr a) const { return !!(wattr & a); }

	/** Helpers for various write attributes */
	bool isFinal() const { return hasAttr(WriteAttr::Final); }
	bool isLocal() const { return hasAttr(WriteAttr::Local); }
	bool isComplete() const { return hasAttr(WriteAttr::Complete); }

	/** Whether this is part of an RMW operation */
	bool isRMW() const;

	/** Whether this write modifies global memory (SAVer) */
	bool isEffectful() const;

	/** Whether this write is (externally) observable */
	bool isObservable() const;

	/** Adds the store co-after LAB */
	void addCo(EventLabel *lab);

	/** Moves the store co-after LAB (has to already exist in co) */
	void moveCo(EventLabel *lab);

	/** Iterators for readers */
	using ReaderList = CopyableIList<ReadLabel>;
	using rf_iterator = ReaderList::iterator;
	using const_rf_iterator = ReaderList::const_iterator;

	auto readers() { return std::views::all(readerList); }
	auto readers() const { return std::views::all(readerList); }

	virtual void reset() override
	{
		MemAccessLabel::reset();
		readerList.clear();
	}

	DEFINE_STANDARD_MEMBERS_RANGE(Write)

private:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;
	friend class ReadLabel; // FIXME: Robustify; no friendship necessary

	/** Adds a read to the list of reads reading from the write */
	void addReader(ReadLabel *rLab)
	{
		BUG_ON(std::find_if(readerList.begin(), readerList.end(), [rLab](ReadLabel &oLab) {
			       return oLab.getPos() == rLab->getPos();
		       }) != readerList.end());
		readerList.push_back(*rLab);
	}

	/** Removes all readers that satisfy predicate F */
	template <typename F> void removeReader(F cond)
	{
		for (auto it = readerList.begin(); it != readerList.end();) {
			if (cond(*it))
				it = readerList.erase(it);
			else
				++it;
		}
	}

	/** The value written by this label */
	SVal value;

	/** List of reads reading from the write */
	ReaderList readerList;

	/** Attributes of the write */
	WriteAttr wattr = WriteAttr::None;
};

#define WRITE_PURE_SUBCLASS(_class_kind)                                                           \
	class _class_kind##Label : public WriteLabel {                                             \
                                                                                                   \
	public:                                                                                    \
		_class_kind##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,  \
				   SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps()) \
			: WriteLabel(_class_kind, pos, ord, loc, size, type, val, wattr, deps)     \
		{}                                                                                 \
		_class_kind##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type,  \
				   SVal val, const EventDeps &deps = EventDeps())                  \
			: _class_kind##Label(pos, ord, loc, size, type, val, WriteAttr::None,      \
					     deps)                                                 \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(_class_kind)                                               \
	};

WRITE_PURE_SUBCLASS(CondVarInitWrite);
WRITE_PURE_SUBCLASS(CondVarSignalWrite);
WRITE_PURE_SUBCLASS(CondVarBcastWrite);
WRITE_PURE_SUBCLASS(CondVarDestroyWrite);

/** Represents releasing a lock (e.g., pthread_mutex_unlock) */
class UnlockWriteLabel : public WriteLabel {
protected:
	UnlockWriteLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size,
			 AType type, SVal val, WriteAttr wattr = WriteAttr::None,
			 const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, wattr, deps)
	{}

public:
	UnlockWriteLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type, SVal val,
			 WriteAttr wattr, const EventDeps &deps = EventDeps())
		: UnlockWriteLabel(UnlockWrite, pos, ord, loc, size, type, val, wattr, deps)
	{}
	UnlockWriteLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, AType type, SVal val,
			 const EventDeps &deps = EventDeps())
		: UnlockWriteLabel(pos, ord, loc, size, type, val, WriteAttr::None, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(UnlockWrite)
};

/*******************************************************************************
 **                         AbstractUnlockWriteLabel Class
 ******************************************************************************/

/** Represents a call to __VERIFIER_plock_unlock */
class AbstractUnlockWriteLabel : public UnlockWriteLabel {

public:
	AbstractUnlockWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,
				 SVal val, WriteAttr wattr = WriteAttr::None,
				 const EventDeps &deps = EventDeps())
		: UnlockWriteLabel(AbstractUnlockWrite, pos, ord, addr, size, type, val, wattr,
				   deps)
	{}

	DEFINE_STANDARD_MEMBERS(AbstractUnlockWrite)
};

/*******************************************************************************
 **                         FaiWriteLabel Class
 ******************************************************************************/

/** Represents the write part of a read-modify-write (RMW) operation (e.g,
fetch-and-add, fetch-and-sub, etc) */
class FaiWriteLabel : public WriteLabel {

protected:
	FaiWriteLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size,
		      AType type, SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, wattr, deps)
	{}

public:
	FaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
		      WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(FaiWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}
	FaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
		      const EventDeps &deps = EventDeps())
		: FaiWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(FaiWrite)
};

/*******************************************************************************
 **                         NoRetFaiWriteLabel Class
 ******************************************************************************/

/** Specialization of FaiWriteLabel for non-value-returning FAIs */
class NoRetFaiWriteLabel : public FaiWriteLabel {

public:
	NoRetFaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
			   WriteAttr wattr = WriteAttr::None, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(NoRetFaiWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}

	DEFINE_STANDARD_MEMBERS(NoRetFaiWrite)
};

/*******************************************************************************
 **                         BIncFaiWriteLabel Class
 ******************************************************************************/

/** Specialization of FaiWriteLabel for barrier FAIs */
class BIncFaiWriteLabel : public FaiWriteLabel {

public:
	BIncFaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
			  WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(BIncFaiWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}
	BIncFaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
			  const EventDeps &deps = EventDeps())
		: FaiWriteLabel(BIncFaiWrite, pos, ord, addr, size, type, val, WriteAttr::None,
				deps)
	{}

	DEFINE_STANDARD_MEMBERS(BIncFaiWrite)
};

/*******************************************************************************
 **                         CasWriteLabel Class
 ******************************************************************************/

/** Represents the write part of a compare-and-swap (CAS) operation */
class CasWriteLabel : public WriteLabel {

protected:
	CasWriteLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size,
		      AType type, SVal val, WriteAttr wattr = WriteAttr::None,
		      const EventDeps &deps = EventDeps())
		: WriteLabel(k, pos, ord, addr, size, type, val, wattr, deps)
	{}

public:
	CasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
		      WriteAttr wattr = WriteAttr::None, const EventDeps &deps = EventDeps())
		: CasWriteLabel(CasWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(CasWrite)
};

#define CASWRITE_PURE_SUBCLASS(_class_kind)                                                        \
	class _class_kind##Label : public CasWriteLabel {                                          \
                                                                                                   \
	public:                                                                                    \
		_class_kind##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, \
				   SVal val, WriteAttr wattr = WriteAttr::None,                    \
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

/** Specialization of CasWriteLabel for lock CASes */
class LockCasWriteLabel : public CasWriteLabel {

protected:
	LockCasWriteLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, ASize size,
			  AType type, SVal val, WriteAttr wattr = WriteAttr::None,
			  const EventDeps &deps = EventDeps())
		: CasWriteLabel(k, pos, ord, addr, size, type, val, wattr, deps)
	{}

public:
	LockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
			  WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasWriteLabel(LockCasWrite, pos, ord, addr, size, type, val, wattr, deps)
	{}
	LockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal val,
			  const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}
	LockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, MemOrdering::Acquire, addr, size, AType::Signed, SVal(1),
				    deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(LockCasWrite)
};

/*******************************************************************************
 **                         TrylockCasWriteLabel Class
 ******************************************************************************/

/** Specialization of CasWriteLabel for trylock CASes */
class TrylockCasWriteLabel : public CasWriteLabel {

public:
	TrylockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,
			     SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasWriteLabel(TrylockCasWrite, pos, ord, addr, size, type, val, WriteAttr::None,
				deps)
	{}
	TrylockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,
			     SVal val, const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, ord, addr, size, type, val, WriteAttr::None, deps)
	{}
	TrylockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, MemOrdering::Acquire, addr, size, AType::Signed,
				       SVal(1), deps)
	{}

	DEFINE_STANDARD_MEMBERS(TrylockCasWrite)
};

/*******************************************************************************
 **                         AbstractLockCasWriteLabel Class
 ******************************************************************************/

/** Represents a call to __VERIFIER_plock_lock */
class AbstractLockCasWriteLabel : public LockCasWriteLabel {
public:
	AbstractLockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type,
				  SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(AbstractLockCasWrite, pos, ord, addr, size, type, val, wattr,
				    deps)
	{}

	AbstractLockCasWriteLabel(Event pos, SAddr addr, ASize size,
				  const EventDeps &deps = EventDeps())
		: AbstractLockCasWriteLabel(pos, MemOrdering::Acquire, addr, size, AType::Signed,
					    SVal(1), WriteAttr::None, deps)
	{}

	DEFINE_STANDARD_MEMBERS(AbstractLockCasWrite)
};

/*******************************************************************************
 **                         FenceLabel Class
 ******************************************************************************/

/** Represents a fence */
class FenceLabel : public EventLabel {

protected:
	FenceLabel(EventLabelKind k, Event pos, MemOrdering ord,
		   const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps)
	{}

public:
	FenceLabel(Event pos, MemOrdering ord, const EventDeps &deps = EventDeps())
		: FenceLabel(Fence, pos, ord, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(Fence)
};

/*******************************************************************************
 **                        MallocLabel Class
 ******************************************************************************/

/** Corresponds to a memory-allocating operation (e.g., malloc()) */
class MallocLabel : public EventLabel {

public:
	MallocLabel(Event pos, MemOrdering ord, SAddr addr, unsigned int size, unsigned alignment,
		    StorageDuration sd, StorageType stype, AddressSpace spc, const NameInfo *info,
		    const std::string &name, const EventDeps &deps = EventDeps())
		: EventLabel(Malloc, pos, ord, deps), allocAddr(addr), allocSize(size),
		  alignment(alignment), sdur(sd), stype(stype), spc(spc), name(name), nameInfo(info)
	{}
	MallocLabel(Event pos, SAddr addr, unsigned int size, unsigned alignment,
		    StorageDuration sd, StorageType stype, AddressSpace spc,
		    const NameInfo *info = nullptr, const std::string &name = {},
		    const EventDeps &deps = EventDeps())
		: MallocLabel(pos, MemOrdering::NotAtomic, addr, size, alignment, sd, stype, spc,
			      info, name, deps)
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

	/** Getter for the (fresh) address returned by the allocation */
	SAddr getAllocAddr() const { return allocAddr; }

	/** Setter for the (fresh) address returned by the allocation */
	void setAllocAddr(SAddr addr) { allocAddr = addr; }

	/** Getter for the corresponding free label */
	FreeLabel *getFree() const { return dLab; }
	FreeLabel *getFree() { return dLab; }

	/** Setter for the corresponding free label*/
	void setFree(FreeLabel *lab) { dLab = lab; }

	/** Iterators for accesses */
	using AccessList = CopyableIList<MemAccessLabel>;
	using access_iterator = AccessList::iterator;
	using const_access_iterator = AccessList::const_iterator;

	auto accesses() { return std::views::all(accessList); }
	auto accesses() const { return std::views::all(accessList); }

	/** Returns the size of this allocation */
	unsigned int getAllocSize() const { return allocSize; }

	/** Returns true if ADDR is contained within the allocated block */
	bool contains(SAddr addr) const
	{
		return getAllocAddr() <= addr && addr < getAllocAddr() + getAllocSize();
	}

	/** Returns the alignment of this allocation */
	unsigned int getAlignment() const { return alignment; }

	/** Returns the storage duration of this allocation */
	StorageDuration getStorageDuration() const { return sdur; }

	/** Returns the storage type of this allocation */
	StorageType getStorageType() const { return stype; }

	/** Returns the address space of this allocation */
	AddressSpace getAddressSpace() const { return spc; }

	/** Returns the name of the variable allocated */
	const std::string &getName() const { return name; }

	/** Returns the naming info associated with this allocation.
	 * Returns null if no such info is found. */
	const NameInfo *getNameInfo() const { return nameInfo; }

	virtual void reset() override
	{
		EventLabel::reset();
		dLab = nullptr;
		accessList.clear();
	}

	DEFINE_STANDARD_MEMBERS_RANGE(Malloc)

private:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	void addAccess(MemAccessLabel *mLab)
	{
		BUG_ON(std::find_if(accessList.begin(), accessList.end(), [mLab](auto &oLab) {
			       return oLab.getPos() == mLab->getPos();
		       }) != accessList.end());
		accessList.push_back(*mLab);
	}

	template <typename F> void removeAccess(F cond)
	{
		for (auto it = accessList.begin(); it != accessList.end();) {
			if (cond(*it))
				it = accessList.erase(it);
			else
				++it;
		}
	}

	/** The address returned by malloc() */
	SAddr allocAddr;

	/** The corresponding free label (if it exists) */
	FreeLabel *dLab = nullptr;

	/** Accesses on the allocated location */
	AccessList accessList;

	/** The size of the requested allocation */
	unsigned int allocSize{};

	/** Allocation alignment */
	unsigned int alignment{};

	/** Storage duration */
	StorageDuration sdur;

	/** Storage type */
	StorageType stype;

	/** Address space */
	AddressSpace spc;

	/** Name of the variable allocated */
	std::string name;

	/** Naming information for this allocation */
	const NameInfo *nameInfo{};
};

/*******************************************************************************
 **                         FreeLabel Class
 ******************************************************************************/

/** Corresponds to a memory-freeing operation (e.g., free()) */
class FreeLabel : public EventLabel {

protected:
	FreeLabel(EventLabelKind k, Event pos, MemOrdering ord, SAddr addr, unsigned int size,
		  const EventDeps &deps = EventDeps())
		: EventLabel(k, pos, ord, deps), freeAddr(addr), freedSize(size)
	{}
	FreeLabel(EventLabelKind k, Event pos, SAddr addr, unsigned int size,
		  const EventDeps &deps = EventDeps())
		: FreeLabel(k, pos, MemOrdering::NotAtomic, addr, size, deps)
	{}

public:
	FreeLabel(Event pos, MemOrdering ord, SAddr addr, unsigned int size,
		  const EventDeps &deps = EventDeps())
		: FreeLabel(Free, pos, ord, addr, size, deps)
	{}
	FreeLabel(Event pos, SAddr addr, unsigned int size, const EventDeps &deps = EventDeps())
		: FreeLabel(pos, MemOrdering::NotAtomic, addr, size, deps)
	{}
	FreeLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: FreeLabel(pos, addr, 0, deps)
	{}

	/** Returns the address being freed */
	SAddr getFreedAddr() const { return freeAddr; }

	/** Getter for the size of the memory freed */
	unsigned int getFreedSize() const { return freedSize; }

	/** Setter for the size of the memory freed */
	void setFreedSize(unsigned int size) { freedSize = size; }

	/** Getter for the corresponding allocating event */
	MallocLabel *getAlloc() const { return aLab; }
	MallocLabel *getAlloc() { return aLab; }

	/** Setter for the corresponding allocating event */
	void setAlloc(MallocLabel *lab) { aLab = lab; }

	/** Returns true if ADDR is contained within the deallocated block */
	bool contains(SAddr addr) const
	{
		return getFreedAddr() <= addr && addr < getFreedAddr() + getFreedSize();
	}

	virtual void reset() override
	{
		EventLabel::reset();
		aLab = nullptr;
	}

	DEFINE_STANDARD_MEMBERS_RANGE(Free)

private:
	/** The address of the memory freed */
	SAddr freeAddr;

	/** The size of the memory freed */
	unsigned int freedSize{};

	/** The corresponding allocation */
	MallocLabel *aLab{};
};

/*******************************************************************************
 **                         HpRetireLabel Class
 ******************************************************************************/

/** Corresponds to a hazptr retire operation */
class HpRetireLabel : public FreeLabel {

public:
	HpRetireLabel(Event pos, MemOrdering ord, SAddr addr, unsigned int size,
		      const EventDeps &deps = EventDeps())
		: FreeLabel(HpRetire, pos, ord, addr, size, deps)
	{}
	HpRetireLabel(Event pos, SAddr addr, unsigned int size, const EventDeps &deps = EventDeps())
		: HpRetireLabel(pos, MemOrdering::NotAtomic, addr, size, deps)
	{}
	HpRetireLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: HpRetireLabel(pos, addr, 0, deps)
	{}

	DEFINE_STANDARD_MEMBERS(HpRetire)
};

/*******************************************************************************
 **                     ThreadCreateLabel Class
 ******************************************************************************/

/** This label denotes the creation of a thread (via, e.g., pthread_create()) */
class ThreadCreateLabel : public EventLabel {

public:
	ThreadCreateLabel(Event pos, MemOrdering ord, ThreadInfo childInfo,
			  const EventDeps &deps = EventDeps())
		: EventLabel(ThreadCreate, pos, ord, deps), childInfo(childInfo)
	{}
	ThreadCreateLabel(Event pos, ThreadInfo childInfo, const EventDeps &deps = EventDeps())
		: ThreadCreateLabel(pos, MemOrdering::Release, childInfo, deps)
	{}

	/** Getters for the created thread's info */
	const ThreadInfo &getChildInfo() const { return childInfo; }
	ThreadInfo &getChildInfo() { return childInfo; }

	/** Getter for the identifier of the created thread */
	unsigned int getChildId() const { return getChildInfo().id; }

	/** Setter for the identifier of the created thread */
	void setChildId(unsigned int tid) { getChildInfo().id = tid; }

	DEFINE_STANDARD_MEMBERS(ThreadCreate)

private:
	/** Information about the child thread */
	ThreadInfo childInfo;
};

/*******************************************************************************
 **                     ThreadJoinLabel Class
 ******************************************************************************/

/** Represents a join() operation (e.g., pthread_join()) */
class ThreadJoinLabel : public EventLabel {

public:
	ThreadJoinLabel(Event pos, MemOrdering ord, unsigned int childId,
			const EventDeps &deps = EventDeps())
		: EventLabel(ThreadJoin, pos, ord, deps), childId(childId)
	{}
	ThreadJoinLabel(Event pos, unsigned int childId, const EventDeps &deps = EventDeps())
		: ThreadJoinLabel(pos, MemOrdering::Acquire, childId, deps)
	{}

	/** Returns the identifier of the thread this join() is waiting on */
	unsigned int getChildId() const { return childId; }

	DEFINE_STANDARD_MEMBERS(ThreadJoin)

private:
	/** The identifier of the child */
	const unsigned int childId{};
};

/*******************************************************************************
 **                         HpProtectLabel Class
 ******************************************************************************/

/** Specialization of writes for hazptr protect events */
class HpProtectLabel : public EventLabel {

public:
	HpProtectLabel(Event pos, MemOrdering ord, SAddr hpAddr, SAddr protAddr,
		       const EventDeps &deps = EventDeps())
		: EventLabel(HpProtect, pos, ord, deps), hpAddr(hpAddr), protAddr(protAddr)
	{}
	HpProtectLabel(Event pos, SAddr hpAddr, SAddr protAddr, const EventDeps &deps = EventDeps())
		: HpProtectLabel(pos, MemOrdering::Release, hpAddr, protAddr, deps)
	{}

	/** Getters for HP/protected address */
	SAddr getHpAddr() const { return hpAddr; }
	SAddr getProtectedAddr() const { return protAddr; }

	DEFINE_STANDARD_MEMBERS(HpProtect)

private:
	/** HP address */
	SAddr hpAddr;

	/** Protected address */
	SAddr protAddr;
};

/*******************************************************************************
 **                         MethodBeginLabel Class
 ******************************************************************************/

class MethodEndLabel;

/** Along with `MethodEndLabel` represents boundaries of a method invocation */
class MethodBeginLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	MethodBeginLabel(Event pos, std::string methodName, int32_t argValue)
		: EventLabel(MethodBegin, pos, MemOrdering::NotAtomic),
		  name_(std::move(methodName)), arg_(argValue)
	{}

	auto lin_preds() const { return std::views::all(linPreds_); }
	auto lin_preds() { return std::views::all(linPreds_); }

	/** Adds PRED as a linearization predecessor for this label.
	 * (Also updates information in the predecessor.) */
	void addPred(MethodEndLabel *pred);

	/** Removes all predecessors that satisfy predicate F,
	 * and accordingly updates successor lists of predecessors (SLOW) */
	template <typename F> void removePred(F cond);

	auto getName() const -> std::string { return name_; }
	auto getArgument() const -> int32_t { return arg_; }

	virtual void reset() override
	{
		EventLabel::reset();
		linPreds_.clear();
	}

	DEFINE_STANDARD_MEMBERS(MethodBegin)

private:
	friend class ExecutionGraph;
	friend class MethodEndLabel;

	void addPredNoCascade(MethodEndLabel *predLab)
	{
		BUG_ON(std::ranges::find(linPreds_, predLab) != linPreds_.end());
		linPreds_.push_back(predLab);
	}

	template <typename F> void removePredNoCascade(F cond)
	{
		for (auto it = linPreds_.begin(); it != linPreds_.end();) {
			if (cond(*it)) {
				it = linPreds_.erase(it);
			} else {
				++it;
			}
		}
	}

	std::string name_;
	int32_t arg_;
	std::vector<MethodEndLabel *> linPreds_;
};

/*******************************************************************************
 **                         MethodEndLabel Class
 ******************************************************************************/

/** Along with `MethodBeginLabel` represent boundaries of a method invocation and
 * also captures its result */
class MethodEndLabel : public EventLabel {

protected:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

public:
	MethodEndLabel(Event pos, std::string methodName, int32_t returnValue)
		: EventLabel(MethodEnd, pos, MemOrdering::NotAtomic), name_(std::move(methodName)),
		  result_(returnValue)
	{}

	auto lin_succs() const { return std::views::all(linSuccs_); }
	auto lin_succs() { return std::views::all(linSuccs_); }

	/** Adds SUCC as a linearization successor for this label.
	 * (Also updates predecessor information in the successor.) */
	void addSucc(MethodBeginLabel *succ);

	/** Removes all successors that satisfy predicate F, and accordingly updates the
	 * predecessor lists of the successors (SLOW) */
	template <typename F> void removeSucc(F cond)
	{
		for (auto it = linSuccs_.begin(); it != linSuccs_.end();) {
			if (cond(*it)) {
				auto *mbLab = genmc::dyn_cast<MethodBeginLabel>(*it);
				mbLab->removePredNoCascade(
					[this](auto &lab) { return lab == this; });
				it = linSuccs_.erase(it);
			} else {
				++it;
			}
		}
	}

	auto getName() const -> std::string { return name_; }
	auto getResult() const -> int32_t { return result_; }

	virtual void reset() override
	{
		EventLabel::reset();
		linSuccs_.clear();
	}

	DEFINE_STANDARD_MEMBERS(MethodEnd)

private:
	friend class MethodBeginLabel;
	friend class ExecutionGraph;

	template <typename F> void removeSuccNoCascade(F cond)
	{
		for (auto it = linSuccs_.begin(); it != linSuccs_.end();) {
			if (cond(*it)) {
				it = linSuccs_.erase(it);
			} else {
				++it;
			}
		}
	}

	void addSuccNoCascade(MethodBeginLabel *succLab)
	{
		BUG_ON(std::ranges::find(linSuccs_, succLab) != linSuccs_.end());
		linSuccs_.push_back(succLab);
	}

	std::string name_;
	int32_t result_;
	std::vector<MethodBeginLabel *> linSuccs_;
};

/*******************************************************************************
 **                           OutputLabel Class
 ******************************************************************************/

/** Prints a string in the execution graph **/
class OutputLabel : public EventLabel {

public:
	OutputLabel(Event pos, std::string msg)
		: EventLabel(Output, pos, MemOrdering::NotAtomic), msg_(std::move(msg))
	{}

	[[nodiscard]] auto getMsg() const -> const std::string & { return msg_; }

	DEFINE_STANDARD_MEMBERS(Output)
private:
	std::string msg_;
};

/*******************************************************************************
 **                           ErrorLabel Class
 ******************************************************************************/

/** Represents an error in the user program (e.g., assertion violation) **/
class ErrorLabel : public EventLabel {

public:
	ErrorLabel(Event pos, std::string msg)
		: EventLabel(Error, pos, MemOrdering::NotAtomic), msg_(std::move(msg))
	{}

	[[nodiscard]] auto getMsg() const -> const std::string & { return msg_; }

	DEFINE_STANDARD_MEMBERS(Error)
private:
	std::string msg_;
};

/*******************************************************************************
 **                         HelpingCasLabel class
 ******************************************************************************/

/** In contrast to HelpedCAS, a HelpingCAS is a dummy event*/
class HelpingCasLabel : public EventLabel {

public:
	HelpingCasLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, AType type, SVal exp,
			SVal swap, const EventDeps &deps = EventDeps())
		: EventLabel(HelpingCas, pos, ord, deps), access(AAccess(addr, size, type)),
		  expected(exp), swapValue(swap)
	{}

	/** Returns the address of this access */
	SAddr getAddr() const { return access.getAddr(); }

	/** Returns the size (in bytes) of the access */
	ASize getSize() const { return access.getSize(); }

	/** Returns the type of the access */
	AType getType() const { return access.getType(); }

	/** Returns the packed access */
	AAccess getAccess() const { return access; }

	/** Returns the value that makes the supposed CAS succeed */
	SVal getExpected() const { return expected; }

	/** Returns the value that the supposed CAS writes */
	SVal getSwapVal() const { return swapValue; }

	DEFINE_STANDARD_MEMBERS(HelpingCas)

private:
	/** The size of the access performed (in bytes) */
	AAccess access;

	/** CAS expected value */
	const SVal expected;

	/** CAS swap value */
	const SVal swapValue;
};

/*******************************************************************************
 **                            OptionalLabel Class
 ******************************************************************************/

/** A label that represents the beginning of an optional block */
class OptionalLabel : public EventLabel {

public:
	OptionalLabel(Event pos, const EventDeps &deps = EventDeps())
		: EventLabel(Optional, pos, MemOrdering::NotAtomic, deps)
	{}

	/** Whether this block is expandable */
	bool isExpandable() const { return expandable; }
	void setExpandable(bool exp) { expandable = exp; }

	/** Whether this block has been expanded */
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
			: EventLabel(_class_kind, pos, MemOrdering::NotAtomic, deps)               \
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

template <typename F> void MethodBeginLabel::removePred(F cond)
{
	for (auto it = linPreds_.begin(); it != linPreds_.end();) {
		if (cond(*it)) {
			auto *meLab = genmc::dyn_cast<MethodEndLabel>(*it);
			meLab->removeSuccNoCascade([this](auto &lab) { return lab == this; });
			it = linPreds_.erase(it);
		} else {
			++it;
		}
	}
}

/*******************************************************************************
 **                             Static methods
 *******************************************************************************/

inline bool EventLabel::isStable() const
{
	auto *mLab = genmc::dyn_cast<MemAccessLabel>(this);
	return !isRevisitable() || (mLab && !mLab->wasAddedMax());
}

inline bool EventLabel::isDependable(EventLabelKind k)
{
	return ReadLabel::classofKind(k) || k == Malloc || k == Optional;
}

inline bool EventLabel::returnsValue(EventLabelKind k)
{
	return ThreadStartLabel::classofKind(k) || ReadLabel::classofKind(k) || k == ThreadJoin ||
	       k == Optional;
}

inline bool EventLabel::accessesValue(EventLabelKind k)
{
	return InitLabel::classofKind(k) || MemAccessLabel::classofKind(k);
}

inline bool EventLabel::hasLocation(EventLabelKind k) { return MemAccessLabel::classofKind(k); }

inline bool ReadLabel::isConfirming(EventLabelKind k)
{
	return ConfirmingReadLabel::classofKind(k) || ConfirmingCasReadLabel::classofKind(k);
}

/**** Formatting ****/

/** Make `EventLabel::EventLabelKind` formattable with `std::format`. */
template <> struct std::formatter<EventLabel::EventLabelKind> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const EventLabel::EventLabelKind &k, std::format_context &ctx) const
	{
		std::string_view str;
		switch (k) {
		case EventLabel::ThreadStart:
			str = "THREAD_START";
			break;
		case EventLabel::Init:
			str = "INIT";
			break;
		case EventLabel::JoinBlock:
			str = "BLOCK[join]";
			break;
		case EventLabel::SpinloopBlock:
			str = "BLOCK[spinloop]";
			break;
		case EventLabel::FaiZNEBlock:
			str = "BLOCK[Fai-zne]";
			break;
		case EventLabel::LockZNEBlock:
			str = "BLOCK[Lock-zne]";
			break;
		case EventLabel::HelpedCASBlock:
			str = "BLOCK[helped-cas]";
			break;
		case EventLabel::ConfirmationBlock:
			str = "BLOCK[conf]";
			break;
		case EventLabel::BarrierBlock:
			str = "BLOCK[barrier]";
			break;
		case EventLabel::ErrorBlock:
			str = "BLOCK[error]";
			break;
		case EventLabel::UserBlock:
			str = "BLOCK[user]";
			break;
		case EventLabel::ReadOptBlock:
			str = "BLOCK[read-opt]";
			break;
		case EventLabel::ThreadKill:
			str = "KILL";
			break;
		case EventLabel::ThreadFinish:
			str = "THREAD_END";
			break;
		case EventLabel::Read:
		case EventLabel::BWaitRead:
		case EventLabel::CondVarWaitRead:
		case EventLabel::SpeculativeRead:
		case EventLabel::ConfirmingRead:
			str = "R";
			break;
		case EventLabel::CasRead:
		case EventLabel::LockCasRead:
		case EventLabel::TrylockCasRead:
		case EventLabel::AbstractLockCasRead:
		case EventLabel::HelpedCasRead:
		case EventLabel::ConfirmingCasRead:
			str = "CR";
			break;
		case EventLabel::FaiRead:
		case EventLabel::BIncFaiRead:
		case EventLabel::NoRetFaiRead:
			str = "UR";
			break;
		case EventLabel::Write:
		case EventLabel::CondVarInitWrite:
		case EventLabel::CondVarSignalWrite:
		case EventLabel::CondVarBcastWrite:
		case EventLabel::CondVarDestroyWrite:
		case EventLabel::UnlockWrite:
			str = "W";
			break;
		case EventLabel::CasWrite:
		case EventLabel::LockCasWrite:
		case EventLabel::TrylockCasWrite:
		case EventLabel::AbstractLockCasWrite:
		case EventLabel::HelpedCasWrite:
		case EventLabel::ConfirmingCasWrite:
			str = "CW";
			break;
		case EventLabel::FaiWrite:
		case EventLabel::BIncFaiWrite:
		case EventLabel::NoRetFaiWrite:
			str = "UW";
			break;
		case EventLabel::Fence:
			str = "F";
			break;
		case EventLabel::Malloc:
			str = "MALLOC";
			break;
		case EventLabel::Free:
			str = "FREE";
			break;
		case EventLabel::HpRetire:
			str = "HP_RETIRE";
			break;
		case EventLabel::ThreadCreate:
			str = "THREAD_CREATE";
			break;
		case EventLabel::ThreadJoin:
			str = "THREAD_JOIN";
			break;
		case EventLabel::HelpingCas:
			str = "HELPING_CAS";
			break;
		case EventLabel::HpProtect:
			str = "HP_PROTECT";
			break;
		case EventLabel::MethodBegin:
			str = "METHOD_BEGIN";
			break;
		case EventLabel::MethodEnd:
			str = "METHOD_END";
			break;
		case EventLabel::Output:
			str = "OUTPUT";
			break;
		case EventLabel::Error:
			str = "ERROR";
			break;
		case EventLabel::Optional:
			str = "OPTIONAL";
			break;
		case EventLabel::LoopBegin:
			str = "LOOP_BEGIN";
			break;
		case EventLabel::SpinStart:
			str = "SPIN_START";
			break;
		case EventLabel::FaiZNESpinEnd:
		case EventLabel::LockZNESpinEnd:
			str = "ZNE_SPIN_END";
			break;
		case EventLabel::Empty:
			str = "EMPTY";
			break;
		default:
			PRINT_BUGREPORT_INFO_ONCE("print-label-type", "Cannot print label type");
			str = "UNKNOWN";
			break;
		}
		return std::format_to(ctx.out(), "{}", str);
	}
};

/** Make any `EventLabel` (including subclasses) formattable with `std::format`. */
#include "ExecutionGraph/LabelVisitor.hpp"

template <typename T>
requires std::derived_from<T, EventLabel>
struct std::formatter<T> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const T &lab, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}", LabelPrinter().toString(lab));
	}
};

#endif /* GENMC_EVENTLABEL_HPP */
