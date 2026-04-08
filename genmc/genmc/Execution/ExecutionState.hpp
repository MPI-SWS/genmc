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

#ifndef GENMC_EXECUTION_STATE_HPP
#define GENMC_EXECUTION_STATE_HPP

#include "genmc/ADT/AdaptiveView.hpp"
#include "genmc/ADT/IntervalMap.hpp"
#include "genmc/ADT/IntervalSet.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Support/ActionEnums.hpp"
#include "genmc/Support/MemAccess.hpp"
#include "genmc/Support/SAddrAllocator.hpp"
#include "genmc/Support/SVal.hpp"

#include <genmc/config.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <unordered_set>
#include <utility>

/** Wraps a raw pointer to static-variable initial data.
 *  Satisfies IVal so it can be stored in an IntervalMap. */
struct InitDataPtr {
	const uint8_t *ptr = nullptr;

	auto operator==(const InitDataPtr &) const -> bool = default;
	auto operator+=(const InitDataPtr &rhs) -> InitDataPtr &
	{
		ptr = rhs.ptr;
		return *this;
	}
};

class ExecutionState {
public:
	using AdaptiveViewMap = genmc::IntervalMap<SAddr, AdaptiveView>;

	/* Ctors/dtor */

	ExecutionState() = default;
	ExecutionState(const ExecutionState &) = default;
	ExecutionState(ExecutionState &&) noexcept = default;
	~ExecutionState() = default;

	auto operator=(const ExecutionState &) -> ExecutionState & = default;
	auto operator=(ExecutionState &&) noexcept -> ExecutionState & = default;

	/* State initialization */

	/** Registers a static range. Returns a fresh address */
	auto addStaticRange(ASize size, uint64_t alignment, bool persistent, bool internal)
		-> SAddr;

#if EMIT_NA_LABELS
	/** Register raw initial data for a static allocation.
	 *  The pointer must remain valid for the state's lifetime
	 *  and must point to initialized data before execution begins. */
	void addStaticInitData(SAddr base, ASize size, const void *data)
	{
		staticInitData_.add(base, base + size,
				    InitDataPtr{static_cast<const uint8_t *>(data)});
	}

	/** Read the initial value for a static memory access. */
	[[nodiscard]] auto readStaticInitValue(const AAccess &access) const -> SVal;
#endif

	/* State getters (state queries) */

	auto allocations() const { return std::ranges::subrange(allocInfoMap_); }

	/* Checker methods */
	[[nodiscard]] auto isAllocated(SAddr addr) const -> bool;
	[[nodiscard]] auto isStatic(SAddr addr) const -> bool;
	[[nodiscard]] auto isInitialized(const AAccess &access) const -> bool;
	[[nodiscard]] auto isAtomicAccessConsistent(const AAccess &access) const -> bool;
	[[nodiscard]] auto isFreed(SAddr addr) const -> bool;
	[[nodiscard]] auto isRetired(SAddr addr) const -> bool;

	[[nodiscard]] auto getAllocPos(SAddr addr) const -> Event;
	[[nodiscard]] auto getAllocAccess(SAddr addr) const -> AAccess;
	[[nodiscard]] auto getFreePos(SAddr addr) const -> std::optional<Event>;

	[[nodiscard]] auto getMaxNAReadView(const AAccess &access) const -> AdaptiveView;
	[[nodiscard]] auto getMaxNAWriteEvent(const AAccess &access) const -> AdaptiveView;
	[[nodiscard]] auto getMaxAReadView(const AAccess &access) const -> AdaptiveView;
	[[nodiscard]] auto getMaxAWriteView(const AAccess &access) const -> AdaptiveView;
	[[nodiscard]] auto getMaxUnprotectedView(SAddr addr) const -> AdaptiveView;

	/** Returns the aggregated view of the writes initializing ACCESS */
	[[nodiscard]] auto getInitView(const AAccess &access) const -> AdaptiveView;

	/** Returns the interval {StartAddr,EndAddr} of the atomic write covering ADDR.
	 * If no such write exists, returns nullopt */
	[[nodiscard]] auto getAtomicWriteInfo(SAddr addr) const
		-> std::optional<std::pair<SAddr, SAddr>>;

	[[nodiscard]] auto getLastMemAccess(int thread) const -> int;
	/* Returns the index of the previous NA read/write and a boolean indicating whether it was a read */
	[[nodiscard]] auto getPreviousNA(int thread) const -> std::pair<int, bool>;

	/* Use of this assumes NA labels are not in the graph */
	[[nodiscard]] auto getNAWriteValue(const AAccess &access) const -> SVal;

	/** Merges the value of an atomic with potential NA overwrites */
	[[nodiscard]] auto reconstructMemValue(const AAccess &access, const View &atomicView,
					       SVal atomicVal) const -> SVal;

	/* Event handlers (state updates) */

	void onNALoad(Event pos, const AAccess &access, const View &view);
	void onATLoad(Event pos, const AAccess &access, const View &view);
	void onNAStore(Event pos, const AAccess &access, const View &view,
		       const std::optional<SVal> &val);
	void onATStore(Event pos, const AAccess &access, const View &view, bool isEffectful,
		       bool isRelease);
	void onFence(Event pos, bool isRelease);
	void onLoopBegin(Event pos);
	auto onSpinStart(Event pos) -> bool;
	auto onAlloc(Event pos, ASize size, uint64_t alignment, StorageDuration sdur,
		     StorageType styp, AddressSpace spc) -> SAddr;
	void onFree(Event pos, SAddr addr, bool isRetire);
	void onProtect(SAddr hpAddr, SAddr protAddr);

	/* Use of this assumes NA labels are not in the graph */
	void onDeferredValueUpdate(const AAccess &access, SVal val);

	void clear();

private:
	/** * A wrapper around optional that supports += as an "overwrite if present"
	 * operation. This is required for aggregation logic. */
	template <typename T> class NonRevertibleOptional {
	public:
		NonRevertibleOptional() = default;
		NonRevertibleOptional(T value) : value_(std::move(value)) {}

		auto operator+=(const NonRevertibleOptional &rhs) -> NonRevertibleOptional &
		{
			if (rhs.value_)
				value_ = rhs.value_;
			return *this;
		}

		[[nodiscard]] auto value() const -> const T & { return *value_; }

		explicit operator bool() const { return value_.has_value(); }

		auto operator==(const NonRevertibleOptional &rhs) const -> bool = default;

	private:
		std::optional<T> value_;
	};

	/** Packs together information about an allocation */
	struct AllocInfo {
		Event pos;
		AAccess access;
		unsigned int alignment{};
		std::optional<Event> freePos;
		bool isRetire{};

		AllocInfo() = default;
		AllocInfo(Event e, AAccess access, unsigned int alignment)
			: pos(e), access(access), alignment(alignment)
		{}

		auto operator==(const AllocInfo &) const -> bool = default;
		auto operator+=(const AllocInfo &other) -> AllocInfo &
		{
			*this = other;
			return *this;
		}
	};

	using NASValMap = genmc::IntervalMap<SAddr, NonRevertibleOptional<SVal>>;
	using NAEventMap = genmc::IntervalMap<SAddr, NonRevertibleOptional<Event>>;
	using LoopId = Event;
	using LoopAccessMap = std::unordered_map<LoopId, genmc::IntervalSet<SAddr>>;
	using HpCountMap = genmc::IntervalMap<SAddr, int>;

	auto getAllocator() const -> const SAddrAllocator & { return alloctor_; }
	auto getAllocator() -> SAddrAllocator & { return alloctor_; }

	[[nodiscard]] auto isInLoop(int thread) const -> bool;
	[[nodiscard]] auto doesCurrLoopHaveSideEffects(int thread) const -> bool;
	[[nodiscard]] auto isLocReadInLoop(int thread, const AAccess &access) const -> bool;
	[[nodiscard]] auto isProtected(const AAccess &access) const -> bool;

	void checkLoopSideEffects(Event pos, const AAccess &access, bool isRelease);
	void updateSideEffects(Event pos);

	void updateHpCount(SAddr addr, int count);
	void recordMemAccess(AdaptiveViewMap &m, const AAccess &access, Event pos,
			     const View &view);
	void recordNAWrite(const AAccess &access, Event pos, const View &view);
	void updateLastWriteType(const AAccess &access, bool isAtomic);
	void markInitialized(const AAccess &access, Event pos, const View &view);

	/* allocation information */
	genmc::IntervalMap<SAddr, AllocInfo> allocInfoMap_;
	AdaptiveViewMap initialized_;
	SAddrAllocator alloctor_;
	VSet<std::pair<SAddr, SAddr>> staticRanges_;

#if EMIT_NA_LABELS
	/* init vals for statics (not cleared) */
	genmc::IntervalMap<SAddr, InitDataPtr> staticInitData_;
#endif

	/* non-atomic value */
	NASValMap lastNAWriteVal_;
	genmc::IntervalMap<SAddr, NonRevertibleOptional<bool>> lastIsNA_;

	/* race detection */
	NAEventMap lastNAWriteEvent_;
	AdaptiveViewMap lastNAReadView_;
	AdaptiveViewMap lastAReadView_;
	AdaptiveViewMap lastAWriteView_;

	/* spinloop detection */
	std::unordered_map<int, LoopId> currentLoop_; /* thread id -> loop id */
	LoopAccessMap loopAccess_;
	std::unordered_set<LoopId> loopSideEffects_;
	std::unordered_map<int, int> lastReleaseIdx_;

	/* symmetry reduction */
	View lastMemAccess_;

	/* moot execution */
	std::unordered_map<int, std::pair<int, bool>> previousNA_; /* thread id -> {index, isRead} */

	/* hazard pointers */
	std::unordered_map<SAddr, SAddr> hpProtectMap_; /* hp addr -> protected addr */
	HpCountMap hpProtectCount_;
	AdaptiveViewMap lastUnprotectedAccess_;
};

#endif /* GENMC_EXECUTION_STATE_HPP */
