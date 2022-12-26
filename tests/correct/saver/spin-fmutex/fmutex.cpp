/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fmutex.h"

#ifdef MC_ON
// NOLINTNEXTLINE(C_RULE_ID_DEFINE_MULTILINE)
#define FAIL_WITH_MESSAGE(m)			\
	ASSERT(0);				\
	return
#define LOG_MESSAGE(l, m)
#define HELPERS_TO_UNSIGNED(m) m
#else
#include "utils/logger.h"
#include "utils/type_helpers.h"
#define LOG_MESSAGE(l, m) LOG(l, COMMON) << (m)        // NOLINT(cppcoreguidelines-macro-usage)
#define FAIL_WITH_MESSAGE(m) LOG_MESSAGE(FATAL, m)     // NOLINT(cppcoreguidelines-macro-usage)
#define HELPERS_TO_UNSIGNED(m) helpers::ToUnsigned(m)  // NOLINT(cppcoreguidelines-macro-usage)
namespace panda::os::unix::memory::futex {
#endif

// This field is set to false in case of deadlock with daemon threads (only daemon threads
// are not finished and they have state IS_BLOCKED). In this case we should terminate
// those threads ignoring failures on lock structures destructors.
	static ATOMIC(bool) g_deadlockFlag = false;

#ifdef MC_ON
// GenMC does not support syscalls(futex)
	static ATOMIC_INT g_futexSignal;

	static inline void FutexWait(ATOMIC_INT *m, int v)
	{
		// Atomic with acquire order reason: mutex synchronization
		int s = ATOMIC_LOAD(&g_futexSignal, memory_order_acquire);
		// Atomic with acquire order reason: mutex synchronization
		if (ATOMIC_LOAD(m, memory_order_acquire) != v) {
			return;
		}
		// Atomic with acquire order reason: mutex synchronization
		while (ATOMIC_LOAD(&g_futexSignal, memory_order_acquire) == s) {
		}
	}

	static inline void FutexWake()
	{
		// Atomic with release order reason: mutex synchronization
		ATOMIC_FETCH_ADD(&g_futexSignal, 1, memory_order_release);
	}
#else
// futex() is defined in header, as it is still included in different places
#endif

	bool MutexDoNotCheckOnTerminationLoop()
	{
		return g_deadlockFlag;
	}

	void MutexIgnoreChecksOnTerminationLoop()
	{
		g_deadlockFlag = true;
	}

	int *GetStateAddr(struct fmutex *const m)
	{
		return reinterpret_cast<int *>(&m->state_and_waiters_);
	}

	void IncrementWaiters(struct fmutex *const m)
	{
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_FETCH_ADD(&m->state_and_waiters_, WAITER_INCREMENT, memory_order_relaxed);
	}
	void DecrementWaiters(struct fmutex *const m)
	{
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_FETCH_SUB(&m->state_and_waiters_, WAITER_INCREMENT, memory_order_relaxed);
	}

	int32_t GetWaiters(struct fmutex *const m)
	{
		// Atomic with relaxed order reason: mutex synchronization
		return static_cast<int32_t>(static_cast<uint32_t>(ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed)) >>
					    static_cast<uint32_t>(WAITER_SHIFT));
	}

	bool IsHeld(struct fmutex *const m, THREAD_ID thread)
	{
		// Atomic with relaxed order reason: mutex synchronization
		return ATOMIC_LOAD(&m->exclusive_owner_, memory_order_relaxed) == thread;
	}

// Spin for small arguments and yield for longer ones.
	static void BackOff(uint32_t i)
	{
#ifndef MC_ON
		static constexpr uint32_t SPIN_MAX = 10;
		if (i <= SPIN_MAX) {
			volatile uint32_t x = 0;  // Volatile to make sure loop is not optimized out.
			const uint32_t spinCount = 10 * i;
			for (uint32_t spin = 0; spin < spinCount; spin++) {
				++x;
			}
		} else {
			thread::Yield();
		}
#endif
	}

// Wait until pred is true, or until timeout is reached.
// Return true if the predicate test succeeded, false if we timed out.
	static inline bool WaitBrieflyFor(ATOMIC_INT *addr)
	{
		// We probably don't want to do syscall (switch context) when we use WaitBrieflyFor
		static constexpr uint32_t MAX_BACK_OFF = 10;
		static constexpr uint32_t maxIter = 50;
		for (uint32_t i = 1; i <= maxIter; i++) {
			BackOff(MIN(i, MAX_BACK_OFF));
			// Atomic with relaxed order reason: mutex synchronization
			int state = ATOMIC_LOAD(addr, memory_order_relaxed);
			if ((HELPERS_TO_UNSIGNED(state) & HELPERS_TO_UNSIGNED(HELD_MASK)) == 0) {
				return true;
			}
		}
		return false;
	}

	void MutexInit(struct fmutex *const m)
	{
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_STORE(&m->exclusive_owner_, 0, memory_order_relaxed);
		m->recursiveCount = 0;
		m->recursive_mutex_ = false;
		// Atomic with release order reason: mutex synchronization
		ATOMIC_STORE(&m->state_and_waiters_, 0, memory_order_release);
	}

	void MutexDestroy(struct fmutex *const m)
	{
#ifndef PANDA_TARGET_MOBILE
		// We can ignore these checks on devices because we use zygote and do not destroy runtime.
		if (!MutexDoNotCheckOnTerminationLoop()) {
#endif  // PANDA_TARGET_MOBILE
        // Atomic with relaxed order reason: mutex synchronization
			if (ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed) != 0) {
				FAIL_WITH_MESSAGE("Mutex destruction failed; state_and_waiters_ is non zero!");
				// Atomic with relaxed order reason: mutex synchronization
			} else if (ATOMIC_LOAD(&m->exclusive_owner_, memory_order_relaxed) != 0) {
				FAIL_WITH_MESSAGE("Mutex destruction failed; mutex has an owner!");
			}
#ifndef PANDA_TARGET_MOBILE
		} else {
			LOG_MESSAGE(WARNING, "Termination loop detected, ignoring Mutex");
		}
#endif  // PANDA_TARGET_MOBILE
	}

	bool MutexLock(struct fmutex *const m, bool trylock)
	{
		if (current_tid == 0) {
			current_tid = GET_CURRENT_THREAD;
		}
		if (m->recursive_mutex_) {
			if (IsHeld(m, current_tid)) {
				m->recursiveCount++;
				return true;
			}
		}

		ASSERT(!IsHeld(m, current_tid));
		bool done = false;
		while (!done) {
			// Atomic with relaxed order reason: mutex synchronization
			auto curState = ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed);
			if (LIKELY((HELPERS_TO_UNSIGNED(curState) & HELPERS_TO_UNSIGNED(HELD_MASK)) == 0)) {
				// Lock not held, try acquiring it.
				auto newState = HELPERS_TO_UNSIGNED(curState) | HELPERS_TO_UNSIGNED(HELD_MASK);
				done =
					ATOMIC_CAS_WEAK(&m->state_and_waiters_, curState, newState, memory_order_acquire, memory_order_relaxed);
				continue;
			} else {
				if (trylock) {
					return false;
				}
				// Failed to acquire, wait for unlock
				// NOLINTNEXTLINE(hicpp-signed-bitwise)
				if (!WaitBrieflyFor(&m->state_and_waiters_)) {
					// WaitBrieflyFor failed, go to futex wait
					// Increment waiters count.
					IncrementWaiters(m);
					// Update curState to be equal to current expected state_and_waiters_.
					curState += WAITER_INCREMENT;
					// Retry wait until lock not held. In heavy contention situations curState check can fail
					// immediately due to repeatedly decrementing and incrementing waiters.
					// NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
					while ((HELPERS_TO_UNSIGNED(curState) & HELPERS_TO_UNSIGNED(HELD_MASK)) != 0) {
#ifdef MC_ON
						FutexWait(&m->state_and_waiters_, curState);
#else
						// NOLINTNEXTLINE(hicpp-signed-bitwise), NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
						if (futex(GetStateAddr(m), FUTEX_WAIT_PRIVATE, curState, nullptr, nullptr, 0) != 0) {
							// NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
							if ((errno != EAGAIN) && (errno != EINTR)) {
								LOG(FATAL, COMMON) << "Futex wait failed!";
							}
						}
#endif
						// Atomic with relaxed order reason: mutex synchronization
						curState = ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed);
					}
					DecrementWaiters(m);
				}
			}
			continue;
		}
		// Mutex is held now
		// Atomic with relaxed order reason: mutex synchronization
		ASSERT((HELPERS_TO_UNSIGNED(ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed)) &
			HELPERS_TO_UNSIGNED(HELD_MASK)) != 0);
		// Atomic with relaxed order reason: mutex synchronization
		ASSERT(ATOMIC_LOAD(&m->exclusive_owner_, memory_order_relaxed) == 0);
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_STORE(&m->exclusive_owner_, current_tid, memory_order_relaxed);
		m->recursiveCount++;
		ASSERT(m->recursiveCount == 1);  // should be 1 here, there's a separate path for recursive mutex above
		return true;
	}

	bool MutexTryLockWithSpinning(struct fmutex *const m)
	{
		const int maxIter = 10;
		for (int i = 0; i < maxIter; i++) {
			if (MutexLock(m, true)) {
				return true;
			}
			// NOLINTNEXTLINE(hicpp-signed-bitwise)
			if (!WaitBrieflyFor(&m->state_and_waiters_)) {
				// WaitBrieflyFor failed, means lock is held
				return false;
			}
		}
		return false;
	}

	void MutexUnlock(struct fmutex *const m)
	{
		if (current_tid == 0) {
			current_tid = GET_CURRENT_THREAD;
		}
		if (!IsHeld(m, current_tid)) {
			FAIL_WITH_MESSAGE("Trying to unlock mutex which is not held by current thread");
		}
		m->recursiveCount--;
		if (m->recursive_mutex_) {
			if (m->recursiveCount > 0) {
				return;
			}
		}

		ASSERT(m->recursiveCount == 0);  // should be 0 here, there's a separate path for recursive mutex above
		bool done = false;
		// Atomic with relaxed order reason: mutex synchronization
		auto curState = ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed);
		// Retry CAS until succeess
		while (!done) {
			auto newState = HELPERS_TO_UNSIGNED(curState) & ~HELPERS_TO_UNSIGNED(HELD_MASK);  // State without holding bit
			if ((HELPERS_TO_UNSIGNED(curState) & HELPERS_TO_UNSIGNED(HELD_MASK)) == 0) {
				FAIL_WITH_MESSAGE("Mutex unlock got unexpected state, mutex is unlocked?");
			}
			// Reset exclusive owner before changing state to avoid check failures if other thread sees UNLOCKED
			// Atomic with relaxed order reason: mutex synchronization
			ATOMIC_STORE(&m->exclusive_owner_, 0, memory_order_relaxed);
			// curState should be updated with fetched value on fail
			done = ATOMIC_CAS_WEAK(&m->state_and_waiters_, curState, newState, memory_order_release, memory_order_relaxed);
			if (LIKELY(done)) {
				// If we had waiters, we need to do futex call
				if (UNLIKELY(newState != 0)) {
#ifdef MC_ON
					FutexWake();
#else
					// NOLINTNEXTLINE(hicpp-signed-bitwise)
					futex(GetStateAddr(m), FUTEX_WAKE_PRIVATE, WAKE_ONE, nullptr, nullptr, 0);
#endif
				}
			}
		}
	}

	void MutexLockForOther(struct fmutex *const m, THREAD_ID thread)
	{
		// Atomic with relaxed order reason: mutex synchronization
		ASSERT(ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed) == 0);
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_STORE(&m->state_and_waiters_, HELD_MASK, memory_order_relaxed);
		m->recursiveCount = 1;
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_STORE(&m->exclusive_owner_, thread, memory_order_relaxed);
	}

	void MutexUnlockForOther(struct fmutex *const m, THREAD_ID thread)
	{
		if (!IsHeld(m, thread)) {
			FAIL_WITH_MESSAGE("Unlocking for thread which doesn't own this mutex");
		}
		// Atomic with relaxed order reason: mutex synchronization
		ASSERT(ATOMIC_LOAD(&m->state_and_waiters_, memory_order_relaxed) == HELD_MASK);
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_STORE(&m->state_and_waiters_, 0, memory_order_relaxed);
		m->recursiveCount = 0;
		// Atomic with relaxed order reason: mutex synchronization
		ATOMIC_STORE(&m->exclusive_owner_, 0, memory_order_relaxed);
	}

#ifdef MC_ON
#else
}  // namespace panda::os::unix::memory::futex
#endif
