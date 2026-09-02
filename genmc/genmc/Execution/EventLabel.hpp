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

#include "genmc/ADT/VSet.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/ADT/ilist.hpp"
#include "genmc/ADT/value_ptr.hpp"
#include "genmc/Execution/DepInfo.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Execution/EventAttr.hpp"
#include "genmc/Execution/LoadAnnotation.hpp"
#include "genmc/Execution/Stamp.hpp"
#include "genmc/Support/ActionEnums.hpp"
#include "genmc/Support/Cast.hpp"
#include "genmc/Support/MemAccess.hpp"
#include "genmc/Support/MemOrdering.hpp"
#include "genmc/Support/NameInfo.hpp"
#include "genmc/Support/RMWOps.hpp"
#include "genmc/Support/SAddr.hpp"
#include "genmc/Support/SVal.hpp"
#include "genmc/Support/ThreadInfo.hpp"

#include <cstdint>
#include <format>
#include <optional>
#include <ranges>
#include <utility>

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
	CopyableIList(const CopyableIList & /*other*/) : BaseT(BaseT()) {}
	CopyableIList(CopyableIList &&other) = default;

	auto operator=(const CopyableIList & /*other*/) -> CopyableIList &
	{
		*this = std::move(BaseT());
	}
	auto operator=(CopyableIList &&other) -> CopyableIList & = default;
	~CopyableIList() = default;
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
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class EventLabel : public genmc::ilist_node<EventLabel, io_tag>,
		   public genmc::ilist_node<EventLabel, po_tag> {

public:
	/* Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * It is public to allow clients perform a switch() on it */
	// NOLINTNEXTLINE(cppcoreguidelines-use-enum-class,readability-enum-initial-value)
	enum EventLabelKind : std::uint8_t {
#define HANDLE_LABEL(NAME) NAME,
#include "genmc/Execution/EventLabel.def"
#define FIRST_LABEL(NAME, ARG) FIRST_##NAME = ARG,
#define LAST_LABEL(NAME, ARG) LAST_##NAME = ARG,
#include "genmc/Execution/EventLabel.def"
	};

protected:
	EventLabel(EventLabelKind kind, Event pos, MemOrdering ord, EventDeps deps = EventDeps())
		: kind(kind), position(pos), ordering(ord), deps(std::move(deps))
	{}

public:
	virtual ~EventLabel() = default;

	/** Iterators for dependencies */
	auto data() const { return std::views::all(deps.data); }
	auto addr() const { return std::views::all(deps.addr); }
	auto ctrl() const { return std::views::all(deps.ctrl); }

	/** Returns the discriminator of this object */
	auto getKind() const -> EventLabelKind { return kind; }

	/** Returns the parent graph of this label */
	auto getParent() const -> const ExecutionGraph * { return parent; }
	auto getParent() -> ExecutionGraph * { return parent; }

	/** Sets the parent graph for this label */
	void setParent(ExecutionGraph *graph) { parent = graph; }

	/** Returns the position in the execution graph (thread, index) */
	auto getPos() const -> Event { return position; }

	/** Sets the position of the event */
	void setPos(Event pos) { position = pos; }

	/** Returns the index of this label within a thread */
	auto getIndex() const -> int { return position.index; }

	/** Returns the thread of this label in the execution graph */
	auto getThread() const -> int { return position.thread; }

	/** Getter for the label ordering */
	auto getOrdering() const -> MemOrdering { return ordering; }

	/** Setter for the label ordering */
	void setOrdering(MemOrdering ord) { ordering = ord; }

	/** Returns this label's dependencies */
	auto getDeps() const -> const EventDeps & { return deps; }

	/** Sets this label's dependencies */
	void setDeps(const EventDeps &ds) { deps = ds; }
	void setDeps(const EventDeps *ds) { deps = *ds; }
	void setDeps(EventDeps &&ds) { deps = std::move(ds); }

	/** Returns whether a stamp has been assigned for this label */
	auto hasStamp() const -> bool { return stamp.has_value(); }

	/** Returns the stamp of the label in a graph */
	auto getStamp() const -> Stamp
	{
#ifdef ENABLE_GENMC_DEBUG
		return stamp.value();
#else
		return *stamp;
#endif
	}

	auto hasPrefixView() const -> bool { return prefixView.get() != nullptr; }
	auto getPrefixView() const -> const VectorClock & { return *prefixView; }
	auto getPrefixView() -> VectorClock & { return *prefixView; }
	void setPrefixView(std::unique_ptr<VectorClock> v) const { prefixView = std::move(v); }

	void setCalculated(std::vector<VSet<Event>> &&calc) { calculatedRels = std::move(calc); }

	void setViews(std::vector<View> &&views) { calculatedViews = std::move(views); }
	void addView(View &&view) { calculatedViews.emplace_back(std::move(view)); }

	/** Iterators for calculated relations */
	auto calculated(size_t i) const
	{
		return (getPos().isInitializer() || getKind() == Empty)
			       ? std::views::all(calculatedRels[0])
			       : std::views::all(calculatedRels[i]);
	}

	/** Getters for calculated views */
	auto view(size_t i) const -> const View &
	{
		return (getPos().isInitializer() || getKind() == Empty) ? calculatedViews[0]
									: calculatedViews[i];
	}

	/** Iterator over the calculated views */
	auto views() const { return std::views::all(calculatedViews); }

	/** Returns true if this label corresponds to a non-atomic access */
	auto isNotAtomic() const -> bool { return ordering == MemOrdering::NotAtomic; }

	/** Returns true if the ordering of this access is acquire or stronger */
	auto isAtLeastAcquire() const -> bool
	{
		return isAtLeastOrStrongerThan(ordering, MemOrdering::Acquire);
	}

	/** Returns true if the ordering of this access is release or stronger */
	auto isAtLeastRelease() const -> bool
	{
		return isAtLeastOrStrongerThan(ordering, MemOrdering::Release);
	}

	/** Returns true if this is a sequentially consistent access */
	auto isSC() const -> bool { return ordering == MemOrdering::SequentiallyConsistent; }

	/** Whether this label can have outgoing dep edges */
	auto isDependable() const -> bool { return isDependable(getKind()); }

	/** Whether this label returns a value */
	auto returnsValue() const -> bool { return returnsValue(getKind()); }

	/** Returns the value returned by the label */
	auto getReturnValue() const -> SVal;

	/** Returns whether this label accesses some value */
	auto accessesValue() const -> bool { return accessesValue(getKind()); }

	/** Returns the value from memory the label accesses.
	 * (The label needs to be a memory access.) */
	auto getAccessValue(const AAccess &access) const -> SVal;

	/** Whether this label has a location */
	auto hasLocation() const -> bool { return hasLocation(getKind()); }

	/** Returns true if this event can be revisited */
	auto isRevisitable() const -> bool { return revisitable; }

	/** Makes the relevant event revisitable/non-revisitable. The
	 * execution graph is responsible for making such changes */
	void setRevisitStatus(bool status) { revisitable = status; }

	/** Returns true if this event cannot be revisited or deleted */
	auto isStable() const -> bool;

	/** Necessary for multiple inheritance + LLVM-style RTTI to work */
	static auto classofKind(EventLabelKind /*K*/) -> bool { return true; }

	/** Returns a clone object (virtual to allow deep copying from base) */
	virtual auto clone() const -> std::unique_ptr<EventLabel> = 0;

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

	static inline auto isDependable(EventLabelKind kind) -> bool;
	static inline auto returnsValue(EventLabelKind kind) -> bool;
	static inline auto accessesValue(EventLabelKind kind) -> bool;
	static inline auto hasLocation(EventLabelKind kind) -> bool;

	void setStamp(std::optional<Stamp> s) { stamp = s; }

	/** Discriminator enum for LLVM-style RTTI */
	const EventLabelKind kind; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

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
	template <typename... Ts>                                                                  \
	static auto create(Ts &&...params) -> std::unique_ptr<name##Label>                         \
	{                                                                                          \
		return std::make_unique<name##Label>(std::forward<Ts>(params)...);                 \
	}                                                                                          \
                                                                                                   \
	auto clone() const -> std::unique_ptr<EventLabel> override                                 \
	{                                                                                          \
		return std::make_unique<name##Label>(*this);                                       \
	}

#define DEFINE_STANDARD_MEMBERS(name)                                                              \
	DEFINE_CREATE_CLONE(name)                                                                  \
                                                                                                   \
	static auto classof(const EventLabel *lab) -> bool { return classofKind(lab->getKind()); } \
	static auto classofKind(EventLabelKind kind) -> bool { return kind == name; }

#define DEFINE_CLASSOF_RANGE(name)                                                                 \
	static auto classof(const EventLabel *lab) -> bool { return classofKind(lab->getKind()); } \
	static auto classofKind(EventLabelKind kind) -> bool                                       \
	{                                                                                          \
		return kind >= FIRST_##name && kind <= LAST_##name;                                \
	}

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
		  createLab_(createLab), threadInfo(std::move(tinfo)), symmPredTid(symmPred)
	{}
	ThreadStartLabel(Event pos, Event createId, ThreadCreateLabel *createLab, ThreadInfo tinfo,
			 int symmPred = -1)
		: ThreadStartLabel(pos, MemOrdering::Acquire, createId, createLab, std::move(tinfo),
				   symmPred)
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

	void reset() override
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

	auto rf_begin(SAddr addr) -> rf_iterator { return initRfs[addr].begin(); }
	auto rf_begin(SAddr addr) const -> const_rf_iterator { return initRfs.at(addr).begin(); };
	auto rf_end(SAddr addr) -> rf_iterator { return initRfs[addr].end(); }
	auto rf_end(SAddr addr) const -> const_rf_iterator { return initRfs.at(addr).end(); }
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
	TerminatorLabel(EventLabelKind kind, MemOrdering ord, Event pos)
		: EventLabel(kind, pos, ord, EventDeps())
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
	BlockLabel(EventLabelKind kind, Event pos)
		: TerminatorLabel(kind, MemOrdering::NotAtomic, pos)
	{}

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
	JoinBlockLabel(Event pos, int childId) : BlockLabel(JoinBlock, pos), childId(childId) {}

	/** Returns the ID of the child waited on */
	auto getChildId() const -> int { return childId; }

	DEFINE_STANDARD_MEMBERS(JoinBlock)

private:
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
	const int childId{}; // the child waiting on
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
	auto getAddr() const -> const SAddr & { return addr; }

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
	auto getParentJoin() const -> ThreadJoinLabel * { return parentJoin; }

	/** Sets the corresponding join() event */
	void setParentJoin(ThreadJoinLabel *jLab) { parentJoin = jLab; }

	/** Returns the return value of this thread */
	auto getRetVal() const -> SVal { return retVal; }

	void reset() override
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
 **                        MemLabel Class (Abstract)
 ******************************************************************************/

/** This label abstracts the common functionality that loads, stores, alloc
 * and free have (e.g., a base address and a size) */
class MemLabel : public EventLabel {
protected:
	MemLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr loc, ASize size,
		 const EventDeps &deps = EventDeps())
		: EventLabel(kind, pos, ord, deps), access(loc, size)
	{}
	MemLabel(EventLabelKind kind, Event pos, MemOrdering ord, AAccess acc,
		 const EventDeps &deps = EventDeps())
		: EventLabel(kind, pos, ord, deps), access(acc)
	{}

public:
	/** Returns the address of this access */
	auto getAddr() const -> SAddr { return access.addr; }

	/** Sets the address of this access */
	void setAddr(SAddr addr) { access.addr = addr; }

	/** Returns the size (in bytes) of the access */
	auto getSize() const -> ASize { return access.size; }

	/** Sets the size of this access */
	void setSize(ASize size) { access.size = size; }

	/** Returns the packed access */
	auto getAccess() const -> const AAccess & { return access; }

	/** Returns true if ADDR is contained within the address range */
	auto contains(SAddr addr) const -> bool
	{
		return getAddr() <= addr && addr < getAddr() + getSize();
	}

	void reset() override { EventLabel::reset(); }

	DEFINE_CLASSOF_RANGE(Mem)

private:
	/** The access performed */
	AAccess access;
};

/*******************************************************************************
 **                       MemAccessLabel Class (Abstract)
 ******************************************************************************/

/** This label abstracts the common functionality that loads and stores have
 * (e.g., which allocation block they're modifying) */
class MemAccessLabel : public MemLabel, public genmc::ilist_node<MemAccessLabel> {

protected:
	MemAccessLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr loc, ASize size,
		       const EventDeps &deps = EventDeps())
		: MemLabel(kind, pos, ord, loc, size, deps)
	{}
	MemAccessLabel(EventLabelKind kind, Event pos, MemOrdering ord, AAccess acc,
		       const EventDeps &deps = EventDeps())
		: MemLabel(kind, pos, ord, acc, deps)
	{}

public:
	/** Returns whether the access is maximal in the graph it was added (default),
	 * unless the status has been changed via setAddedMax() */
	auto wasAddedMax() const -> bool { return maximal; }

	/** Explicitly sets the maximality status */
	void setAddedMax(bool status) { maximal = status; }

	void reset() override
	{
		MemLabel::reset();
		maximal = true;
	}

	DEFINE_CLASSOF_RANGE(MemAccess)

private:
	/** Whether was mo-maximal when added
	 * Note: This is used in handleStore to decide whether current event is co-maximal */
	bool maximal = true;
};

/*******************************************************************************
 **                         ReadLabel Class
 ******************************************************************************/

/** The label for reads. All special read types (e.g., FAI, CAS) should inherit
 * from this class */
class ReadLabel : public MemAccessLabel, public genmc::ilist_node<ReadLabel> {

protected:
	ReadLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr loc, ASize size,
		  EventLabel *rfLab = nullptr, std::optional<Annotation> annot = {},
		  const EventDeps &deps = EventDeps())
		: MemAccessLabel(kind, pos, ord, loc, size, deps), readsFrom(rfLab),
		  annot_(std::move(annot))
	{}

public:
	ReadLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, EventLabel *rfLab,
		  std::optional<Annotation> annot, const EventDeps &deps = EventDeps())
		: ReadLabel(Read, pos, ord, loc, size, rfLab, std::move(annot), deps)
	{}
	ReadLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, EventLabel *rfLab,
		  const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, rfLab, std::nullopt, deps)
	{}
	ReadLabel(Event pos, MemOrdering ord, SAddr loc, ASize size,
		  const EventDeps &deps = EventDeps())
		: ReadLabel(pos, ord, loc, size, nullptr, std::nullopt, deps)
	{}

	/** Returns the position of the write this read is reading-from */
	auto getRf() const -> EventLabel * { return readsFrom; }
	auto getRf() -> EventLabel * { return readsFrom; }

	/** Changes the reads-from edge for this label.
	 * Also updates reader information in the writer */
	void setRf(EventLabel *rfLab);

	/** Whether this read has a set RF and reads externally */
	auto readsExt() const -> bool
	{
		return getRf() && !getRf()->getPos().isInitializer() &&
		       getRf()->getThread() != getThread();
	}

	/** Whether this read has a set RF and reads internally */
	auto readsInt() const -> bool
	{
		return getRf() &&
		       (getRf()->getPos().isInitializer() || getRf()->getThread() == getThread());
	}

	/** Whether the read is part of an RMW operation (needs to be part of a graph) */
	auto isRMW() const -> bool;

	/** Convenience function that returns whether reading a value will create an RMW */
	auto valueMakesRMWSucceed(const SVal &val) const -> bool;

	/** Convenience function that returns whether reading a value makes the assume
	 * succeed */
	auto valueMakesAssumeSucceed(const SVal &val) const -> bool;

	/** Helper: Whether this is a confirmation read */
	auto isConfirming() const -> bool { return isConfirming(getKind()); }

	/** SAVer: Getter for the annotation expression */
	auto getAnnot() const -> const std::optional<Annotation> & { return annot_; }

	void reset() override
	{
		MemAccessLabel::reset();
		setRfNoCascade(nullptr);
	}

	DEFINE_STANDARD_MEMBERS_RANGE(Read)

private:
	static inline auto isConfirming(EventLabelKind kind) -> bool;

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
		name##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, EventLabel *rfLab,  \
			    std::optional<Annotation> annot, const EventDeps &deps = EventDeps())  \
			: ReadLabel(name, pos, ord, loc, size, rfLab, std::move(annot), deps)      \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, EventLabel *rfLab,  \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, loc, size, rfLab, std::nullopt, deps)              \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr loc, ASize size,                     \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, loc, size, nullptr, std::nullopt, deps)            \
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
	FaiReadLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
		     RMWBinOp op, SVal val, WriteAttr wattr, EventLabel *rfLab,
		     std::optional<Annotation> annot, const EventDeps &deps = EventDeps())
		: ReadLabel(kind, pos, ord, addr, size, rfLab, std::move(annot), deps), binOp(op),
		  opValue(val), wattr(wattr)
	{}

public:
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op, SVal val,
		     WriteAttr wattr, EventLabel *rfLab, std::optional<Annotation> annot,
		     const EventDeps &deps = EventDeps())
		: FaiReadLabel(FaiRead, pos, ord, addr, size, op, val, wattr, rfLab,
			       std::move(annot), deps)
	{}
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op, SVal val,
		     WriteAttr wattr, EventLabel *rfLab, const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, op, val, wattr, rfLab, std::nullopt, deps)
	{}
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op, SVal val,
		     WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, op, val, wattr, nullptr, deps)
	{}
	FaiReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op, SVal val,
		     const EventDeps &deps = EventDeps())
		: FaiReadLabel(pos, ord, addr, size, op, val, WriteAttr::None, deps)
	{}

	/** Returns the type of this RMW operation (e.g., add, sub) */
	auto getOp() const -> RMWBinOp { return binOp; }

	/** Returns the other operand's value */
	auto getOpVal() const -> SVal { return opValue; }

	/** Returns/sets the attributes of the write part */
	auto getAttr() const -> WriteAttr { return wattr; }
	void setAttr(WriteAttr attr) { wattr |= attr; }

	/** Checks whether the write part has the specified attributes */
	auto hasAttr(WriteAttr attr) const -> bool { return !!(wattr & attr); }

	void reset() override { ReadLabel::reset(); }

	DEFINE_STANDARD_MEMBERS_RANGE(FaiRead)

private:
	/** The binary operator for this RMW operation */
	const RMWBinOp binOp; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	/** The other operand's value for the operation */
	SVal opValue;

	/** Attributes for the write part of the RMW */
	WriteAttr wattr = WriteAttr::None;
};

#define FAIREAD_PURE_SUBCLASS(name)                                                                \
	class name##Label : public FaiReadLabel {                                                  \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op,       \
			    SVal val, WriteAttr wattr, EventLabel *rfLab,                          \
			    std::optional<Annotation> annot, const EventDeps &deps = EventDeps())  \
			: FaiReadLabel(name, pos, ord, addr, size, op, val, wattr, rfLab,          \
				       std::move(annot), deps)                                     \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op,       \
			    SVal val, WriteAttr wattr, EventLabel *rfLab,                          \
			    const EventDeps &deps = EventDeps())                                   \
			: name##Label(pos, ord, addr, size, op, val, wattr, rfLab, std::nullopt,   \
				      deps)                                                        \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op,       \
			    SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())        \
			: name##Label(pos, ord, addr, size, op, val, wattr, nullptr, deps)         \
		{}                                                                                 \
		name##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, RMWBinOp op,       \
			    SVal val, const EventDeps &deps = EventDeps())                         \
			: name##Label(pos, ord, addr, size, op, val, WriteAttr::None, deps)        \
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
	CasReadLabel(EventLabelKind kind, Event pos, MemOrdering ord, MemOrdering failOrd,
		     SAddr addr, ASize size, SVal exp, SVal swap, WriteAttr wattr,
		     EventLabel *rfLab, std::optional<Annotation> annot,
		     const EventDeps &deps = EventDeps())
		: ReadLabel(kind, pos, ord, addr, size, rfLab, std::move(annot), deps),
		  expected(exp), swapValue(swap), successOrdering_(ord), failOrdering_(failOrd),
		  wattr(wattr)
	{}

public:
	CasReadLabel(Event pos, MemOrdering ord, MemOrdering failOrd, SAddr addr, ASize size,
		     SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
		     std::optional<Annotation> annot, const EventDeps &deps = EventDeps())
		: CasReadLabel(CasRead, pos, ord, failOrd, addr, size, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}

	/** Returns the value that will make this CAS succeed */
	auto getExpected() const -> SVal { return expected; }

	/** Returns the value that will be written is the CAS succeeds */
	auto getSwapVal() const -> SVal { return swapValue; }

	/** Returns the orderings of the successful/failed CAS, respectively */
	auto getSuccessOrdering() const -> MemOrdering { return successOrdering_; }
	auto getFailOrdering() const -> MemOrdering { return failOrdering_; }

	/** Returns/sets the attributes of the write part */
	auto getAttr() const -> WriteAttr { return wattr; }
	void setAttr(WriteAttr attr) { wattr |= attr; }

	/** Checks whether the write part has the specified attributes */
	auto hasAttr(WriteAttr attr) const -> bool { return !!(wattr & attr); }

	/** Whether this is a weak CAS (i.e., *may* fail spuriously) */
	auto isWeak() const -> bool { return weak; }
	void setWeak(bool w) { weak = w; }

	/** Whether this CAS *does* fail spuriously.
	 * Pre: `this` is a weak CAS. */
	auto failsSpuriously() const -> bool { return spuriousFail; }
	void setSpuriousFailure(bool f) { spuriousFail = f; }

	void reset() override { ReadLabel::reset(); }

	DEFINE_STANDARD_MEMBERS_RANGE(CasRead)

private:
	/** The value that will make this CAS succeed */
	const SVal expected; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	/** The value that will be written if the CAS succeeds */
	const SVal swapValue; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	/** Orderings of the CAS instruction. The inherited `ordering`
	 * caches whichever is in effect */
	const MemOrdering
		successOrdering_; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
	const MemOrdering
		failOrdering_; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	/** The attributes of the write part of the RMW */
	WriteAttr wattr = WriteAttr::None;

	/** Whether this is a weak CAS */
	bool weak = false;

	/** Whether this weak CAS fails spuriously */
	bool spuriousFail = false;
};

#define CASREAD_PURE_SUBCLASS(name)                                                                \
	class name##Label : public CasReadLabel {                                                  \
                                                                                                   \
	public:                                                                                    \
		name##Label(Event pos, MemOrdering ord, MemOrdering failOrd, SAddr addr,           \
			    ASize size, SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,   \
			    std::optional<Annotation> annot, const EventDeps &deps = EventDeps())  \
			: CasReadLabel(name, pos, ord, failOrd, addr, size, exp, swap, wattr,      \
				       rfLab, std::move(annot), deps)                              \
		{}                                                                                 \
                                                                                                   \
		DEFINE_STANDARD_MEMBERS(name)                                                      \
	};

CASREAD_PURE_SUBCLASS(HelpedCasRead);
CASREAD_PURE_SUBCLASS(ConfirmingCasRead);

/*******************************************************************************
 **                         LockCasReadLabel Class
 ******************************************************************************/

/** Specialization of CasReadLabel for lock CASes. A lock CAS has a single
 * ordering, used for both the success and the failure path. */
class LockCasReadLabel : public CasReadLabel {

protected:
	LockCasReadLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
			 SVal exp, SVal swap, WriteAttr wattr, EventLabel *rfLab,
			 std::optional<Annotation> annot = {}, const EventDeps &deps = EventDeps())
		: CasReadLabel(kind, pos, ord, ord, addr, size, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}

public:
	LockCasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal exp, SVal swap,
			 WriteAttr wattr, EventLabel *rfLab, std::optional<Annotation> annot = {},
			 const EventDeps &deps = EventDeps())
		: CasReadLabel(LockCasRead, pos, ord, ord, addr, size, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	LockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			 std::optional<Annotation> annot = {}, const EventDeps &deps = EventDeps())
		: LockCasReadLabel(pos, MemOrdering::Acquire, addr, size, SVal(0), SVal(1), wattr,
				   rfLab, std::move(annot), deps)
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

/** Specialization of CasReadLabel for trylock CASes. A trylock CAS has a single
 * ordering, used for both the success and the failure path. */
class TrylockCasReadLabel : public CasReadLabel {

public:
	TrylockCasReadLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal exp, SVal swap,
			    WriteAttr wattr, EventLabel *rfLab,
			    std::optional<Annotation> annot = {},
			    const EventDeps &deps = EventDeps())
		: CasReadLabel(TrylockCasRead, pos, ord, ord, addr, size, exp, swap, wattr, rfLab,
			       std::move(annot), deps)
	{}
	TrylockCasReadLabel(Event pos, SAddr addr, ASize size, WriteAttr wattr, EventLabel *rfLab,
			    std::optional<Annotation> annot = {},
			    const EventDeps &deps = EventDeps())
		: TrylockCasReadLabel(pos, MemOrdering::Acquire, addr, size, SVal(0), SVal(1),
				      wattr, rfLab, std::move(annot), deps)
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
				   SVal(0), SVal(1), WriteAttr::None, rfLab, std::move(annot), deps)
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
	WriteLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
		   SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: MemAccessLabel(kind, pos, ord, addr, size, deps), value(val), wattr(wattr)
	{}
	WriteLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
		   SVal val, const EventDeps &deps = EventDeps())
		: WriteLabel(kind, pos, ord, addr, size, val, WriteAttr::None, deps)
	{}

public:
	WriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val, WriteAttr wattr,
		   const EventDeps &deps = EventDeps())
		: WriteLabel(Write, pos, ord, addr, size, val, wattr, deps)
	{}
	WriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
		   const EventDeps &deps = EventDeps())
		: WriteLabel(pos, ord, addr, size, val, WriteAttr::None, deps)
	{}

	/** Getter for the write value */
	auto getVal() const -> SVal { return value; }

	/** Setter for the write value */
	void setVal(SVal v) { value = v; }

	/** Returns the attributes of the write */
	auto getAttr() const -> WriteAttr { return wattr; }
	void setAttr(WriteAttr attr) { wattr |= attr; }

	/** Checks whether the write has the specified attributes */
	auto hasAttr(WriteAttr attr) const -> bool { return !!(wattr & attr); }

	/** Helpers for various write attributes */
	auto isFinal() const -> bool { return hasAttr(WriteAttr::Final); }
	auto isLocal() const -> bool { return hasAttr(WriteAttr::Local); }
	auto isComplete() const -> bool { return hasAttr(WriteAttr::Complete); }

	/** Whether this is part of an RMW operation */
	auto isRMW() const -> bool;

	/** Whether this write modifies global memory (SAVer) */
	auto isEffectful() const -> bool;

	/** Whether this write has been inserted into co */
	auto isInCo() const -> bool { return ilist_node<WriteLabel>::is_linked(); }

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

	void reset() override
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
		ASSERT(std::ranges::find_if(readerList, [rLab](ReadLabel &oLab) {
			       return oLab.getPos() == rLab->getPos();
		       }) == readerList.end());
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
		_class_kind##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, SVal val,    \
				   WriteAttr wattr, const EventDeps &deps = EventDeps())           \
			: WriteLabel(_class_kind, pos, ord, loc, size, val, wattr, deps)           \
		{}                                                                                 \
		_class_kind##Label(Event pos, MemOrdering ord, SAddr loc, ASize size, SVal val,    \
				   const EventDeps &deps = EventDeps())                            \
			: _class_kind##Label(pos, ord, loc, size, val, WriteAttr::None, deps)      \
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
	UnlockWriteLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
			 SVal val, WriteAttr wattr = WriteAttr::None,
			 const EventDeps &deps = EventDeps())
		: WriteLabel(kind, pos, ord, addr, size, val, wattr, deps)
	{}

public:
	UnlockWriteLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, SVal val,
			 WriteAttr wattr, const EventDeps &deps = EventDeps())
		: UnlockWriteLabel(UnlockWrite, pos, ord, loc, size, val, wattr, deps)
	{}
	UnlockWriteLabel(Event pos, MemOrdering ord, SAddr loc, ASize size, SVal val,
			 const EventDeps &deps = EventDeps())
		: UnlockWriteLabel(pos, ord, loc, size, val, WriteAttr::None, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(UnlockWrite)
};

/*******************************************************************************
 **                         AbstractUnlockWriteLabel Class
 ******************************************************************************/

/** Represents a call to __VERIFIER_plock_unlock */
class AbstractUnlockWriteLabel : public UnlockWriteLabel {

public:
	AbstractUnlockWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
				 WriteAttr wattr = WriteAttr::None,
				 const EventDeps &deps = EventDeps())
		: UnlockWriteLabel(AbstractUnlockWrite, pos, ord, addr, size, val, wattr, deps)
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
	FaiWriteLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
		      SVal val, WriteAttr wattr, const EventDeps &deps = EventDeps())
		: WriteLabel(kind, pos, ord, addr, size, val, wattr, deps)
	{}

public:
	FaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val, WriteAttr wattr,
		      const EventDeps &deps = EventDeps())
		: FaiWriteLabel(FaiWrite, pos, ord, addr, size, val, wattr, deps)
	{}
	FaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
		      const EventDeps &deps = EventDeps())
		: FaiWriteLabel(pos, ord, addr, size, val, WriteAttr::None, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(FaiWrite)
};

/*******************************************************************************
 **                         NoRetFaiWriteLabel Class
 ******************************************************************************/

/** Specialization of FaiWriteLabel for non-value-returning FAIs */
class NoRetFaiWriteLabel : public FaiWriteLabel {

public:
	NoRetFaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
			   WriteAttr wattr = WriteAttr::None, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(NoRetFaiWrite, pos, ord, addr, size, val, wattr, deps)
	{}

	DEFINE_STANDARD_MEMBERS(NoRetFaiWrite)
};

/*******************************************************************************
 **                         BIncFaiWriteLabel Class
 ******************************************************************************/

/** Specialization of FaiWriteLabel for barrier FAIs */
class BIncFaiWriteLabel : public FaiWriteLabel {

public:
	BIncFaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
			  WriteAttr wattr, const EventDeps &deps = EventDeps())
		: FaiWriteLabel(BIncFaiWrite, pos, ord, addr, size, val, wattr, deps)
	{}
	BIncFaiWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
			  const EventDeps &deps = EventDeps())
		: FaiWriteLabel(BIncFaiWrite, pos, ord, addr, size, val, WriteAttr::None, deps)
	{}

	DEFINE_STANDARD_MEMBERS(BIncFaiWrite)
};

/*******************************************************************************
 **                         CasWriteLabel Class
 ******************************************************************************/

/** Represents the write part of a compare-and-swap (CAS) operation */
class CasWriteLabel : public WriteLabel {

protected:
	CasWriteLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
		      SVal val, WriteAttr wattr = WriteAttr::None,
		      const EventDeps &deps = EventDeps())
		: WriteLabel(kind, pos, ord, addr, size, val, wattr, deps)
	{}

public:
	CasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
		      WriteAttr wattr = WriteAttr::None, const EventDeps &deps = EventDeps())
		: CasWriteLabel(CasWrite, pos, ord, addr, size, val, wattr, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(CasWrite)
};

#define CASWRITE_PURE_SUBCLASS(_class_kind)                                                        \
	class _class_kind##Label : public CasWriteLabel {                                          \
                                                                                                   \
	public:                                                                                    \
		_class_kind##Label(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,   \
				   WriteAttr wattr = WriteAttr::None,                              \
				   const EventDeps &deps = EventDeps())                            \
			: CasWriteLabel(_class_kind, pos, ord, addr, size, val, wattr, deps)       \
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
	LockCasWriteLabel(EventLabelKind kind, Event pos, MemOrdering ord, SAddr addr, ASize size,
			  SVal val, WriteAttr wattr = WriteAttr::None,
			  const EventDeps &deps = EventDeps())
		: CasWriteLabel(kind, pos, ord, addr, size, val, wattr, deps)
	{}

public:
	LockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
			  WriteAttr wattr, const EventDeps &deps = EventDeps())
		: CasWriteLabel(LockCasWrite, pos, ord, addr, size, val, wattr, deps)
	{}
	LockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
			  const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, ord, addr, size, val, WriteAttr::None, deps)
	{}
	LockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(pos, MemOrdering::Acquire, addr, size, SVal(1), deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(LockCasWrite)
};

/*******************************************************************************
 **                         TrylockCasWriteLabel Class
 ******************************************************************************/

/** Specialization of CasWriteLabel for trylock CASes */
class TrylockCasWriteLabel : public CasWriteLabel {

public:
	TrylockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
			     WriteAttr /*wattr*/, const EventDeps &deps = EventDeps())
		: CasWriteLabel(TrylockCasWrite, pos, ord, addr, size, val, WriteAttr::None, deps)
	{}
	TrylockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
			     const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, ord, addr, size, val, WriteAttr::None, deps)
	{}
	TrylockCasWriteLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: TrylockCasWriteLabel(pos, MemOrdering::Acquire, addr, size, SVal(1), deps)
	{}

	DEFINE_STANDARD_MEMBERS(TrylockCasWrite)
};

/*******************************************************************************
 **                         AbstractLockCasWriteLabel Class
 ******************************************************************************/

/** Represents a call to __VERIFIER_plock_lock */
class AbstractLockCasWriteLabel : public LockCasWriteLabel {
public:
	AbstractLockCasWriteLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal val,
				  WriteAttr wattr, const EventDeps &deps = EventDeps())
		: LockCasWriteLabel(AbstractLockCasWrite, pos, ord, addr, size, val, wattr, deps)
	{}

	AbstractLockCasWriteLabel(Event pos, SAddr addr, ASize size,
				  const EventDeps &deps = EventDeps())
		: AbstractLockCasWriteLabel(pos, MemOrdering::Acquire, addr, size, SVal(1),
					    WriteAttr::None, deps)
	{}

	DEFINE_STANDARD_MEMBERS(AbstractLockCasWrite)
};

/*******************************************************************************
 **                       MemLifecycleLabel Class (Abstract)
 ******************************************************************************/

/** This label abstracts the common functionality that allocs and frees have */
class MemLifecycleLabel : public MemLabel {
protected:
	MemLifecycleLabel(EventLabelKind kind, Event pos, SAddr loc, ASize size,
			  const EventDeps &deps = EventDeps())
		: MemLabel(kind, pos, MemOrdering::NotAtomic, loc, size, deps)
	{}
	MemLifecycleLabel(EventLabelKind kind, Event pos, AAccess acc,
			  const EventDeps &deps = EventDeps())
		: MemLabel(kind, pos, MemOrdering::NotAtomic, acc, deps)
	{}

public:
	void reset() override { MemLabel::reset(); }

	DEFINE_CLASSOF_RANGE(MemLifecycle)
};

/*******************************************************************************
 **                        MallocLabel Class
 ******************************************************************************/

/** Corresponds to a memory-allocating operation (e.g., malloc()) */
class MallocLabel : public MemLifecycleLabel {

public:
	MallocLabel(Event pos, SAddr addr, uint64_t size, uint64_t alignment, StorageDuration sd,
		    StorageType stype, AddressSpace spc, const NameInfo *info = nullptr,
		    std::string name = {}, const EventDeps &deps = EventDeps())
		: MemLifecycleLabel(Malloc, pos, addr, size, deps), alignment(alignment), sdur(sd),
		  stype(stype), spc(spc), name(std::move(name)), nameInfo(info)
	{}
	MallocLabel(Event pos, uint64_t size, uint64_t alignment, StorageDuration sd,
		    StorageType stype, AddressSpace spc, const NameInfo *info = nullptr,
		    const std::string &name = {}, const EventDeps &deps = EventDeps())
		: MallocLabel(pos, SAddr(), size, alignment, sd, stype, spc, info, name, deps)
	{}
	MallocLabel(Event pos, uint64_t size, uint64_t alignment, StorageDuration sd,
		    StorageType stype, AddressSpace spc, const EventDeps &deps = EventDeps())
		: MallocLabel(pos, size, alignment, sd, stype, spc, nullptr, {}, deps)
	{}

	/** Returns the alignment of this allocation */
	auto getAlignment() const -> uint64_t { return alignment; }

	/** Returns the storage duration of this allocation */
	auto getStorageDuration() const -> StorageDuration { return sdur; }

	/** Returns the storage type of this allocation */
	auto getStorageType() const -> StorageType { return stype; }

	/** Returns the address space of this allocation */
	auto getAddressSpace() const -> AddressSpace { return spc; }

	/** Returns the name of the variable allocated */
	auto getName() const -> const std::string & { return name; }

	/** Returns the naming info associated with this allocation.
	 * Returns null if no such info is found. */
	auto getNameInfo() const -> const NameInfo * { return nameInfo; }

	void reset() override { MemLifecycleLabel::reset(); }

	DEFINE_STANDARD_MEMBERS_RANGE(Malloc)

private:
	friend class ExecutionGraph;
	friend class DepExecutionGraph;

	/** Allocation alignment */
	uint64_t alignment{};

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
class FreeLabel : public MemLifecycleLabel {

protected:
	FreeLabel(EventLabelKind kind, Event pos, SAddr addr, ASize size,
		  const EventDeps &deps = EventDeps())
		: MemLifecycleLabel(kind, pos, addr, size, deps)
	{}

public:
	FreeLabel(Event pos, SAddr addr, ASize size, const EventDeps &deps = EventDeps())
		: FreeLabel(Free, pos, addr, size, deps)
	{}

	void reset() override { MemLifecycleLabel::reset(); }

	DEFINE_STANDARD_MEMBERS_RANGE(Free)
};

/*******************************************************************************
 **                         HpRetireLabel Class
 ******************************************************************************/

/** Corresponds to a hazptr retire operation */
class HpRetireLabel : public FreeLabel {

public:
	HpRetireLabel(Event pos, SAddr addr, uint64_t size, const EventDeps &deps = EventDeps())
		: FreeLabel(HpRetire, pos, addr, size, deps)
	{}
	HpRetireLabel(Event pos, SAddr addr, const EventDeps &deps = EventDeps())
		: HpRetireLabel(pos, addr, 0, deps)
	{}

	DEFINE_STANDARD_MEMBERS(HpRetire)
};

/*******************************************************************************
 **                         FenceLabel Class
 ******************************************************************************/

/** Represents a fence */
class FenceLabel : public EventLabel {

protected:
	FenceLabel(EventLabelKind kind, Event pos, MemOrdering ord,
		   const EventDeps &deps = EventDeps())
		: EventLabel(kind, pos, ord, deps)
	{}

public:
	FenceLabel(Event pos, MemOrdering ord, const EventDeps &deps = EventDeps())
		: FenceLabel(Fence, pos, ord, deps)
	{}

	DEFINE_STANDARD_MEMBERS_RANGE(Fence)
};

/*******************************************************************************
 **                     ThreadCreateLabel Class
 ******************************************************************************/

/** This label denotes the creation of a thread (via, e.g., pthread_create()) */
class ThreadCreateLabel : public EventLabel {

public:
	ThreadCreateLabel(Event pos, MemOrdering ord, ThreadInfo childInfo,
			  const EventDeps &deps = EventDeps())
		: EventLabel(ThreadCreate, pos, ord, deps), childInfo(std::move(childInfo))
	{}
	ThreadCreateLabel(Event pos, ThreadInfo childInfo, const EventDeps &deps = EventDeps())
		: ThreadCreateLabel(pos, MemOrdering::Release, std::move(childInfo), deps)
	{}

	/** Getters for the created thread's info */
	auto getChildInfo() const -> const ThreadInfo & { return childInfo; }
	auto getChildInfo() -> ThreadInfo & { return childInfo; }

	/** Getter for the identifier of the created thread */
	auto getChildId() const -> int { return getChildInfo().id; }

	/** Setter for the identifier of the created thread */
	void setChildId(int tid) { getChildInfo().id = tid; }

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
	ThreadJoinLabel(Event pos, MemOrdering ord, int childId,
			const EventDeps &deps = EventDeps())
		: EventLabel(ThreadJoin, pos, ord, deps), childId(childId)
	{}
	ThreadJoinLabel(Event pos, int childId, const EventDeps &deps = EventDeps())
		: ThreadJoinLabel(pos, MemOrdering::Acquire, childId, deps)
	{}

	/** Returns the identifier of the thread this join() is waiting on */
	auto getChildId() const -> int { return childId; }

	DEFINE_STANDARD_MEMBERS(ThreadJoin)

private:
	/** The identifier of the child */
	const int childId{}; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
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
	auto getHpAddr() const -> SAddr { return hpAddr; }
	auto getProtectedAddr() const -> SAddr { return protAddr; }

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

	void reset() override
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
		ASSERT(std::ranges::find(linPreds_, predLab) == linPreds_.end());
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

	void reset() override
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
		ASSERT(std::ranges::find(linSuccs_, succLab) == linSuccs_.end());
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
	HelpingCasLabel(Event pos, MemOrdering ord, SAddr addr, ASize size, SVal exp, SVal swap,
			const EventDeps &deps = EventDeps())
		: EventLabel(HelpingCas, pos, ord, deps), access(AAccess(addr, size)),
		  expected(exp), swapValue(swap)
	{}

	/** Returns the address of this access */
	auto getAddr() const -> SAddr { return access.addr; }

	/** Returns the size (in bytes) of the access */
	auto getSize() const -> ASize { return access.size; }

	/** Returns the packed access */
	auto getAccess() const -> AAccess { return access; }

	/** Returns the value that makes the supposed CAS succeed */
	auto getExpected() const -> SVal { return expected; }

	/** Returns the value that the supposed CAS writes */
	auto getSwapVal() const -> SVal { return swapValue; }

	DEFINE_STANDARD_MEMBERS(HelpingCas)

private:
	/** The size of the access performed (in bytes) */
	AAccess access;

	/** CAS expected value */
	const SVal expected; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	/** CAS swap value */
	const SVal swapValue; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
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
	auto isExpandable() const -> bool { return expandable; }
	void setExpandable(bool exp) { expandable = exp; }

	/** Whether this block has been expanded */
	auto isExpanded() const -> bool { return expanded; }
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

inline auto EventLabel::isStable() const -> bool
{
	const auto *mLab = genmc::dyn_cast<MemAccessLabel>(this);
	return !isRevisitable() || (mLab && !mLab->wasAddedMax());
}

inline auto EventLabel::isDependable(EventLabelKind kind) -> bool
{
	return ReadLabel::classofKind(kind) || kind == Malloc || kind == Optional;
}

inline auto EventLabel::returnsValue(EventLabelKind kind) -> bool
{
	return ThreadStartLabel::classofKind(kind) || ReadLabel::classofKind(kind) ||
	       kind == ThreadJoin || kind == Optional;
}

inline auto EventLabel::accessesValue(EventLabelKind kind) -> bool
{
	return InitLabel::classofKind(kind) || MemAccessLabel::classofKind(kind);
}

inline auto EventLabel::hasLocation(EventLabelKind kind) -> bool
{
	return MemAccessLabel::classofKind(kind);
}

inline auto ReadLabel::isConfirming(EventLabelKind kind) -> bool
{
	return ConfirmingReadLabel::classofKind(kind) || ConfirmingCasReadLabel::classofKind(kind);
}

/**** Formatting ****/

/** Make `EventLabel::EventLabelKind` formattable with `std::format`. */
template <> struct std::formatter<EventLabel::EventLabelKind> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const EventLabel::EventLabelKind &kind, std::format_context &ctx) const
	{
		std::string_view str;
		switch (kind) {
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
#include "genmc/Execution/LabelVisitor.hpp"

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
