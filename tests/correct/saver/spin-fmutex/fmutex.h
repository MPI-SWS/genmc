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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_UNIX_FUTEX_FMUTEX_H
#define PANDA_LIBPANDABASE_PBASE_OS_UNIX_FUTEX_FMUTEX_H

/* #include <unistd.h> */

#ifdef MC_ON
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#define THREAD_ID pthread_t
#define GET_CURRENT_THREAD pthread_self()
#define ATOMIC(type) _Atomic type
#define ATOMIC_INT atomic_int
#define ATOMIC_STORE(addr, val, mem) atomic_store_explicit(addr, val, mem)
#define ATOMIC_LOAD(addr, mem) atomic_load_explicit(addr, mem)
#define ATOMIC_FETCH_ADD(addr, val, mem) atomic_fetch_add_explicit(addr, val, mem)
#define ATOMIC_FETCH_SUB(addr, val, mem) atomic_fetch_sub_explicit(addr, val, mem)
#define ATOMIC_CAS_WEAK(addr, old_val, new_val, mem1, mem2) \
    atomic_compare_exchange_weak_explicit(addr, &old_val, new_val, mem1, mem2)
#define ASSERT(a) assert(a)
#define LIKELY(a) a
#define UNLIKELY(a) a
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#else
#include <limits>
#include <sys/param.h>
#include <atomic>
#include <os/thread.h>
#include <linux/futex.h>
#include <sys/syscall.h>
namespace panda::os::unix::memory::futex {
#define THREAD_ID thread::ThreadId                                         // NOLINT(cppcoreguidelines-macro-usage)
#define GET_CURRENT_THREAD os::thread::GetCurrentThreadId()                // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC(type) std::atomic<type>                                     // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC_INT ATOMIC(int)                                             // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC_STORE(addr, val, mem) (addr)->store(val, std::mem)          // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC_LOAD(addr, mem) (addr)->load(std::mem)                      // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC_FETCH_ADD(addr, val, mem) (addr)->fetch_add(val, std::mem)  // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC_FETCH_SUB(addr, val, mem) (addr)->fetch_sub(val, std::mem)  // NOLINT(cppcoreguidelines-macro-usage)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ATOMIC_CAS_WEAK(addr, old_val, new_val, mem1, mem2) \
    (addr)->compare_exchange_weak(old_val, new_val, std::mem1, std::mem2)
#endif

// Copy of mutex storage, after complete implementation should totally replace mutex::current_tid
extern __thread THREAD_ID current_tid;

void MutexInit(struct fmutex *m);
void MutexDestroy(struct fmutex *m);
bool MutexLock(struct fmutex *m, bool trylock);
bool MutexTryLockWithSpinning(struct fmutex *m);
void MutexUnlock(struct fmutex *m);
void MutexLockForOther(struct fmutex *m, THREAD_ID thread);
void MutexUnlockForOther(struct fmutex *m, THREAD_ID thread);

#ifdef MC_ON
// GenMC does not support syscalls(futex)
// Models are defined in .c file to avoid code style warnings
#else
inline int futex(volatile int *uaddr, int op, int val, const struct timespec *timeout, volatile int *uaddr2, int val3)
{
    // NOLINTNEXTLINE
    return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}
#endif

static constexpr int WAKE_ONE = 1;
static constexpr int WAKE_ALL = INT_MAX;
static constexpr int32_t HELD_MASK = 1;
static constexpr int32_t WAITER_SHIFT = 1;
// NOLINTNEXTLINE(hicpp-signed-bitwise, cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
static constexpr int32_t WAITER_INCREMENT = 1 << WAITER_SHIFT;

struct fmutex {
    // Lowest bit: 0 - unlocked, 1 - locked.
    // Other bits: Number of waiters.
    // Unified lock state and waiters count to avoid requirement of double seq_cst memory order on mutex unlock
    // as it's done in RWLock::WriteUnlock
    ATOMIC_INT state_and_waiters_;
    ATOMIC(THREAD_ID) exclusive_owner_;
    int recursiveCount;
    bool recursive_mutex_;
};

int *GetStateAddr(struct fmutex *m);
void IncrementWaiters(struct fmutex *m);
void DecrementWaiters(struct fmutex *m);
int32_t GetWaiters(struct fmutex *m);
bool IsHeld(struct fmutex *m, THREAD_ID thread);
bool MutexDoNotCheckOnTerminationLoop();
void MutexIgnoreChecksOnTerminationLoop();

#ifdef MC_ON
#else
}  // namespace panda::os::unix::memory::futex
#endif

#endif  // PANDA_LIBPANDABASE_PBASE_OS_UNIX_FUTEX_FMUTEX_H
