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

#include "genmc/Execution/ExecutionState.hpp"
#include "genmc/ADT/AdaptiveView.hpp"
#include "genmc/ADT/IntervalMap.hpp"
#include "genmc/ADT/IntervalSet.hpp"
#include "genmc/Execution/Event.hpp"
#include "genmc/Support/ASize.hpp"
#include "genmc/Support/ActionEnums.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Support/MemAccess.hpp"
#include "genmc/Support/SAddr.hpp"
#include "genmc/Support/SVal.hpp"
#include "genmc/config.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <utility>
#include <vector>

using namespace genmc;

/* Mask selecting the least-significant byte of an integer value */
static constexpr uint64_t BYTE_MASK = 0xFF;

/* Helper to create right-open intervals [addr, addr + size) */
static auto makeInterval(const AAccess &access) -> Interval<SAddr>
{
	return {access.addr, access.addr + access.size};
}

/* Helper to fetch the view matching ACCESS from a map M */
static auto getMaxView(const ExecutionState::AdaptiveViewMap &viewMap, const AAccess &access)
	-> AdaptiveView
{
	AdaptiveView result;

	auto interval = makeInterval(access);
	auto it = viewMap.lower_bound(interval);
	auto ie = viewMap.upper_bound(interval);
	while (it != ie) {
		result.join(it->second);
		++it;
	}
	return result;
}

auto ExecutionState::addStaticRange(ASize size, uint64_t alignment, bool persistent, bool internal)
	-> SAddr
{
	auto &alloctor = getAllocator();
	auto addr = alloctor.allocStatic(0, size.get(), alignment, persistent, internal);
	staticRanges_.insert(std::pair{addr, addr + size - ASize(1)});
	return addr;
}

#if EMIT_NA_LABELS
auto ExecutionState::readStaticInitValue(const AAccess &access) const -> SVal
{
	auto it = staticInitData_.find(access.addr);
	VERIFY(it != staticInitData_.end());
	auto entry = *it;
	auto offset = static_cast<size_t>(access.addr - entry.first.start);
	const std::span<const uint8_t> bytes{entry.second.ptr, offset + access.size.get()};
	uint64_t val = 0;
	for (unsigned i = 0; i < access.size.get(); i++)
		val |= static_cast<uint64_t>(bytes[offset + i]) << (i * CHAR_BIT);
	return SVal(val);
}
#endif

void ExecutionState::clear()
{
	/* allocations */
	allocInfoMap_.clear();
	initialized_.clear();
	alloctor_.restrict(View{});

	/* non-atomic value */
	lastNAWriteVal_.clear();
	lastIsNA_.clear();

	/* race detection */
	lastNAReadView_.clear();
	lastNAWriteEvent_.clear();
	lastAReadView_.clear();
	lastAWriteView_.clear();

	/* spinloop detection */
	currentLoop_.clear();
	loopAccess_.clear();
	loopSideEffects_.clear();
	lastReleaseIdx_.clear();

	/* symmetry reduction */
	lastMemAccess_.clear();

	/* moot execution */
	previousNA_.clear();

	/* hazard pointers */
	hpProtectMap_.clear();
	hpProtectCount_.clear();
	lastUnprotectedAccess_.clear();
}

/************************************************************
 * State queries
 ************************************************************/

auto ExecutionState::isAllocated(SAddr addr) const -> bool { return allocInfoMap_.contains(addr); }

auto ExecutionState::isStatic(SAddr addr) const -> bool
{
	auto addrRange = std::make_pair(addr, addr);
	auto it = std::ranges::lower_bound(
		staticRanges_, addrRange,
		[](const auto &itV, const auto &v) { return itV.second < v.first; });
	return it != staticRanges_.end() && addr >= it->first && addr <= it->second;
}

/* TODO: adjust this when we have cutting */
auto ExecutionState::isInitialized(const AAccess &access) const -> bool
{
	return !getInitView(access).empty();
}

auto ExecutionState::isAtomicAccessConsistent(const AAccess &access) const -> bool
{
	auto iv = makeInterval(access);

	/* Overlapping atomic accesses have to have the same size. Inspect
	 * *all* overlapping segments: a wide access can span several narrower
	 * ones (e.g. a 16-bit access over two 8-bit ones), which find() misses
	 * because no single segment fully contains it. */
	auto allSegmentsMatch = [&](const auto &accessMap) {
		auto it = accessMap.lower_bound(iv);
		auto ie = accessMap.upper_bound(iv);
		for (; it != ie; ++it) {
			if (first(it->first) != access.addr || length(it->first) != access.size)
				return false;
		}
		return true;
	};

	return allSegmentsMatch(lastAWriteView_) && allSegmentsMatch(lastAReadView_);
}

auto ExecutionState::isFreed(SAddr addr) const -> bool
{
	auto it = allocInfoMap_.find(addr);
	return it != allocInfoMap_.end() && it->second.freePos.has_value();
}

auto ExecutionState::isRetired(SAddr addr) const -> bool
{
	auto it = allocInfoMap_.find(addr);
	return it != allocInfoMap_.end() && it->second.freePos.has_value() && it->second.isRetire;
}

auto ExecutionState::getAllocPos(SAddr addr) const -> Event
{
	auto it = allocInfoMap_.find(addr);
	VERIFY(it != allocInfoMap_.end());
	return it->second.pos;
}

auto ExecutionState::getAllocAccess(SAddr addr) const -> AAccess
{
	auto it = allocInfoMap_.find(addr);
	VERIFY(it != allocInfoMap_.end());
	return it->second.access;
}

auto ExecutionState::getFreePos(SAddr addr) const -> std::optional<Event>
{
	auto it = allocInfoMap_.find(addr);
	VERIFY(it != allocInfoMap_.end());
	return it->second.freePos;
}

auto ExecutionState::getMaxNAReadView(const AAccess &access) const -> AdaptiveView
{
	return getMaxView(lastNAReadView_, access);
}

auto ExecutionState::getMaxNAWriteEvent(const AAccess &access) const -> AdaptiveView
{

	AdaptiveView::Events result;

	auto interval = makeInterval(access);
	auto it = lastNAWriteEvent_.lower_bound(interval);
	auto ie = lastNAWriteEvent_.upper_bound(interval);
	while (it != ie) {
		result.push_back(it->second.value());
		++it;
	}
	return result;
}

auto ExecutionState::getMaxAReadView(const AAccess &access) const -> AdaptiveView
{
	return getMaxView(lastAReadView_, access);
}

auto ExecutionState::getMaxAWriteView(const AAccess &access) const -> AdaptiveView
{
	return getMaxView(lastAWriteView_, access);
}

auto ExecutionState::getMaxUnprotectedView(SAddr addr) const -> AdaptiveView
{
	/* Assume that hazard pointers always protect a single allocation */
	auto allocAccess = getAllocAccess(addr);
	VERIFY(allocAccess.addr == addr);

	return getMaxView(lastUnprotectedAccess_, allocAccess);
}

auto ExecutionState::getInitView(const AAccess &access) const -> AdaptiveView
{
	return getMaxView(initialized_, access);
}

auto ExecutionState::getLastMemAccess(int thread) const -> int
{
	return lastMemAccess_.getMax(thread);
}

auto ExecutionState::getPreviousNA(int thread) const -> std::pair<int, bool>
{
	return previousNA_.contains(thread) ? previousNA_.at(thread) : std::make_pair(0, false);
}

auto ExecutionState::getNAWriteValue(const AAccess &access) const -> SVal
{
	uint64_t res = 0;
	std::vector<uint64_t> provenances;
	for (auto i = 0U; i < access.size; ++i) {
		auto it = lastNAWriteVal_.find(access.addr + i);
		VERIFY(it != lastNAWriteVal_.end());
		res |= (it->second.value().get() & BYTE_MASK) << (i * CHAR_BIT);
		provenances.push_back(it->second.value().getProvenance());
	}
	const bool allSameProvenance =
		std::ranges::adjacent_find(provenances, std::not_equal_to{}) == provenances.end();
	uint64_t provenance = 0;
	if (allSameProvenance && !provenances.empty())
		provenance = provenances.front();
	return {res, provenance};
}

auto ExecutionState::getAtomicWriteInfo(SAddr addr) const -> std::optional<std::pair<SAddr, SAddr>>
{
	auto it = lastAWriteView_.find(addr);
	if (it == lastAWriteView_.end())
		return {};
	return std::make_pair(it->first.start, it->first.end);
}

auto ExecutionState::reconstructMemValue(const AAccess &access, const View &atomicView,
					 SVal atomicVal) const -> SVal
{
	/* Fastpath: if the atomic view is newer than all NA writes in the range */
	if (atomicView.contains(getMaxNAWriteEvent(access)))
		return atomicVal;

	/* Slowpath: reconstruct byte-by-byte */
	auto res = atomicVal;
	std::vector<uint64_t> provenances;
	for (auto i = 0U; i < access.size; ++i) {
		/* If the atomic view knows about the last NA write to this byte, skip */
		if (atomicView.contains(getMaxNAWriteEvent({access.addr + i, 1})))
			continue;

		/* Otherwise, overwrite this byte */
		res &= SVal(~(BYTE_MASK << (i * CHAR_BIT))); /* mask */
		const SVal byteVal = getNAWriteValue({access.addr + i, 1}) << SVal(i * CHAR_BIT);
		res |= byteVal; /* byte val */
		provenances.push_back(byteVal.getProvenance());
	}
	return std::ranges::adjacent_find(provenances, std::not_equal_to{}) == provenances.end()
		       ? SVal(res.get(), provenances.empty() ? 0 : provenances.front())
		       : SVal(res.get(), 0);
}

/************************************************************
 * State updates
 ************************************************************/

auto ExecutionState::isInLoop(int thread) const -> bool
{
	auto it = currentLoop_.find(thread);
	return it != currentLoop_.end() && !it->second.isInitializer();
}

auto ExecutionState::doesCurrLoopHaveSideEffects(int thread) const -> bool
{
	if (!isInLoop(thread))
		return false;
	return loopSideEffects_.contains(currentLoop_.at(thread));
}

auto ExecutionState::isProtected(const AAccess &access) const -> bool
{
	auto interval = makeInterval(access);
	auto it = hpProtectCount_.lower_bound(interval);
	auto ie = hpProtectCount_.upper_bound(interval);
	SAddr lastUncovered = access.addr;
	while (it != ie) {
		if (it->second > 0 && lower(it->first) <= lastUncovered &&
		    upper(it->first) > lastUncovered) {
			lastUncovered = upper(it->first);
		}
		if (lower(it->first) > lastUncovered)
			return false;
		++it;
	}
	return lastUncovered >= access.addr + access.size;
}

void ExecutionState::recordMemAccess(AdaptiveViewMap &accessMap, const AAccess &access, Event pos,
				     const View &view)
{
	auto interval = makeInterval(access);

	accessMap.add(std::make_pair(interval, AdaptiveView(pos, view)));
	lastMemAccess_.updateIdx(pos);
	if (!isProtected(access))
		lastUnprotectedAccess_.add(std::make_pair(interval, AdaptiveView(pos, view)));
}

void ExecutionState::onNALoad(Event pos, const AAccess &access, const View &view)
{
	recordMemAccess(lastNAReadView_, access, pos, view);
	previousNA_[pos.thread] = std::make_pair(pos.index, true);

	/* Caveat here: some reads are not recorded for efficiency reason */
	if (isInLoop(pos.thread) && !doesCurrLoopHaveSideEffects(pos.thread))
		loopAccess_[currentLoop_[pos.thread]].add(makeInterval(access));
}

void ExecutionState::onATLoad(Event pos, const AAccess &access, const View &view)
{
	recordMemAccess(lastAReadView_, access, pos, view);

	if (isInLoop(pos.thread) && !doesCurrLoopHaveSideEffects(pos.thread))
		loopAccess_[currentLoop_[pos.thread]].add(makeInterval(access));
}

auto ExecutionState::onSpinStart(Event pos) -> bool
{
	/* If there's no loop-begin, this is a manual instrumentation(?); report to user */
	ERROR_ON(!isInLoop(pos.thread), "No loop-beginning found!");

	/* Next iteration needs to recheck for side-effects */
	auto effects = doesCurrLoopHaveSideEffects(pos.thread);
	loopSideEffects_.erase(currentLoop_.at(pos.thread));
	return effects;
}

/* FIXME: nested loop */
void ExecutionState::onLoopBegin(Event pos) { currentLoop_[pos.thread] = pos; }

/* Returns a fresh address to be used from the interpreter */
static auto getFreshAddr(int tid, ASize size, uint64_t alignment, StorageDuration sdur,
			 StorageType styp, AddressSpace spc, SAddrAllocator &alloctor) -> SAddr
{
	/* The arguments to getFreshAddr() need to be well-formed;
	 * make sure the alignment is positive and a power of 2 */
	VERIFY(alignment > 0 && (alignment & (alignment - 1)) == 0);
	switch (sdur) {
	case StorageDuration::SD_Automatic:
		return alloctor.allocAutomatic(tid, size.get(), alignment,
					       styp == StorageType::ST_Durable,
					       spc == AddressSpace::AS_Internal);
	case StorageDuration::SD_Heap:
		return alloctor.allocHeap(tid, size.get(), alignment,
					  styp == StorageType::ST_Durable,
					  spc == AddressSpace::AS_Internal);
	case StorageDuration::SD_Static: /* Cannot ask for fresh static addresses */
	default:
		UNREACHABLE();
	}
	UNREACHABLE();
	return {};
}

auto ExecutionState::onAlloc(Event pos, ASize size, uint64_t alignment, StorageDuration sdur,
			     StorageType styp, AddressSpace spc) -> SAddr
{
	auto addr = getFreshAddr(pos.thread, size, alignment, sdur, styp, spc, getAllocator());
	const AAccess access(addr, size);

	VERIFY(!allocInfoMap_.contains(access.addr));
	allocInfoMap_.add({{access.addr, access.addr + access.size},
			   {pos, access, static_cast<unsigned int>(alignment)}});
	return addr;
}

void ExecutionState::onFree(Event pos, SAddr addr, bool isRetire)
{
	auto it = allocInfoMap_.find(addr);
	VERIFY(it != allocInfoMap_.end());
	auto newVal = it->second;
	newVal.freePos = pos;
	newVal.isRetire = isRetire;

	allocInfoMap_.add({it->first, newVal});

	/* FIXME: HazptrFree - Should not be here */
	if (hpProtectMap_.contains(addr)) {
		if (hpProtectMap_.at(addr) != nullptr)
			updateHpCount(addr, -1);
		hpProtectMap_.erase(addr);
	}
}

void ExecutionState::updateHpCount(SAddr addr, int count)
{
	auto protectedAddr = hpProtectMap_.at(addr);
	auto allocAccess = getAllocAccess(protectedAddr);
	VERIFY(allocAccess.addr == protectedAddr); /* HPs protect a single allocation */

	hpProtectCount_.add(std::make_pair(makeInterval(allocAccess), count));
}

void ExecutionState::onProtect(SAddr hpAddr, SAddr protAddr)
{
	/* If the HP was protecting an address, decreate the respective count*/
	if (hpProtectMap_.contains(hpAddr) && hpProtectMap_.at(hpAddr) != nullptr)
		updateHpCount(hpAddr, -1);

	/* Protect the new address, and set an initial count*/
	hpProtectMap_[hpAddr] = protAddr;
	if (protAddr != nullptr)
		updateHpCount(hpAddr, 1);
}

auto ExecutionState::isLocReadInLoop(int thread, const AAccess &access) const -> bool
{
	if (!isInLoop(thread))
		return false;

	auto loopId = currentLoop_.at(thread);
	auto it = loopAccess_.find(loopId);
	if (it == loopAccess_.end())
		return false;

	auto interval = makeInterval(access);
	return it->second.overlaps(interval);
}

void ExecutionState::updateSideEffects(Event pos)
{
	VERIFY(isInLoop(pos.thread));
	loopSideEffects_.insert(currentLoop_[pos.thread]);
}

void ExecutionState::checkLoopSideEffects(Event pos, const AAccess &access, bool isRelease)
{
	if (isRelease)
		lastReleaseIdx_[pos.thread] = pos.index;
	if (!isInLoop(pos.thread) || doesCurrLoopHaveSideEffects(pos.thread))
		return;

	if (isRelease || !access.addr.isDynamic()) {
		updateSideEffects(pos);
		return;
	}

	auto allocPos = getAllocPos(access.addr);
	if (allocPos.thread != pos.thread) {
		updateSideEffects(pos);
		return;
	}
	auto lastRelIdx = lastReleaseIdx_[pos.thread];
	if (lastRelIdx > allocPos.index && lastRelIdx < pos.index) {
		updateSideEffects(pos);
		return;
	}

	if (isLocReadInLoop(pos.thread, access)) {
		updateSideEffects(pos);
		return;
	}
}

void ExecutionState::recordNAWrite(const AAccess &access, Event pos, const View &view)
{
	lastNAWriteEvent_.add(std::make_pair(makeInterval(access), pos));
	lastMemAccess_.updateIdx(pos);
	if (!isProtected(access)) {
		lastUnprotectedAccess_.add(
			std::make_pair(makeInterval(access), AdaptiveView(pos, view)));
	}
}

void ExecutionState::markInitialized(const AAccess &access, Event pos, const View &view)
{
	auto interval = makeInterval(access);
	if (initialized_.contains(interval))
		return;

	IntervalSet<SAddr> uninitParts;
	uninitParts.add(interval);

	auto it = initialized_.lower_bound(interval);
	auto ie = initialized_.upper_bound(interval);
	while (it != ie) {
		uninitParts.subtract(it->first & interval);
		++it;
	}

	if (!is_empty(uninitParts)) {
		const AdaptiveView initView(pos, view);
		for (const auto &iv : uninitParts)
			initialized_.add(iv, initView);
	}
}

void ExecutionState::onNAStore(Event pos, const AAccess &access, const View &view,
			       const std::optional<SVal> &val)
{
	updateLastWriteType(access, false);
	previousNA_[pos.thread] = std::make_pair(pos.index, false);
	/* Runtime sends the NA value when adding the NA Store */
	if (val)
		onDeferredValueUpdate(access, *val);

	markInitialized(access, pos, view);
	recordNAWrite(access, pos, view);
	checkLoopSideEffects(pos, access, false);
}

void ExecutionState::onATStore(Event pos, const AAccess &access, const View &view, bool isEffectful,
			       bool isRelease)
{
	recordMemAccess(lastAWriteView_, access, pos, view);
	markInitialized(access, pos, view);
	updateLastWriteType(access, true);

	if (hpProtectMap_.contains(access.addr) && hpProtectMap_.at(access.addr) != nullptr) {
		updateHpCount(access.addr, -1);
		hpProtectMap_[access.addr] = nullptr;
	}
	if (isEffectful)
		checkLoopSideEffects(pos, access, isRelease);
}

void ExecutionState::onFence(Event pos, bool isRelease)
{
	if (isRelease)
		lastReleaseIdx_[pos.thread] = pos.index;
}

/* Use of this assumes NA labels are not in the graph */
void ExecutionState::onDeferredValueUpdate(const AAccess &access, SVal val)
{
	for (auto i = 0U; i < access.size; ++i) {
		/* Extract the i-th byte from the val */
		const SVal byteVal = (val >> SVal(i * 8)) & SVal(0xFF);
		/* NOTE: This can be slow (per-byte) */
		auto it = lastIsNA_.find(access.addr + i);
		VERIFY(it != lastIsNA_.end());
		/* If NAs are overwritten by atomic writes, ignore it */
		if (!it->second.value())
			continue;
		lastNAWriteVal_.add(access.addr + i, access.addr + i + 1,
				    NonRevertibleOptional<SVal>{byteVal});
	}
}

void ExecutionState::updateLastWriteType(const AAccess &access, bool isAtomic)
{
	auto interval = makeInterval(access);
	lastIsNA_.add(std::make_pair(interval, NonRevertibleOptional<bool>{!isAtomic}));
}
