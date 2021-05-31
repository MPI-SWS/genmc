#include "mutex_musl.h"

static inline void mutex_init(mutex_t *m)
{
	atomic_init(&m->lock, 0);
	atomic_init(&m->waiters, 0);
}

static inline int mutex_lock_fastpath(mutex_t *m)
{
	int r = 0;
	return atomic_compare_exchange_strong_explicit(&m->lock, &r, 1,
						       memory_order_acquire,
						       memory_order_acquire);
}

static inline int mutex_lock_slowpath_check(mutex_t *m)
{
	int r = 0;
	return atomic_compare_exchange_strong_explicit(&m->lock, &r, 1,
						       memory_order_acquire,
						       memory_order_acquire);
}

static inline void mutex_lock(mutex_t *m)
{

	if (mutex_lock_fastpath(m))
		return;
	atomic_thread_fence(memory_order_relaxed);

	while (!mutex_lock_slowpath_check(m)) {
		atomic_thread_fence(memory_order_relaxed);
		atomic_fetch_add_explicit(&m->waiters, 1, memory_order_relaxed);
		int r = 1;
		if (!atomic_compare_exchange_strong_explicit(&m->lock, &r, 2,
							     memory_order_relaxed,
							     memory_order_relaxed))
			atomic_thread_fence(memory_order_relaxed);
		__futex_wait(&m->lock, 2);
		atomic_fetch_sub_explicit(&m->waiters, 1, memory_order_relaxed);
	}
}

static inline void mutex_unlock(mutex_t *m)
{
	int old = atomic_exchange_explicit(&m->lock, 0, memory_order_release);
	if (atomic_load_explicit(&m->waiters, memory_order_relaxed) > 0 || old != 1)
		__futex_wake(&m->lock, 1);
}
