#include "spinlock.h"

static inline void spinlock_init(struct spinlock_s *l)
{
	atomic_init(&l->lock, 0);
}

static inline void await_for_lock(struct spinlock_s *l)
{
	while (atomic_load_explicit(&l->lock, memory_order_relaxed) != 0)
		;
	return;
}

static inline int try_get_lock(struct spinlock_s *l)
{
	int val = 0;
	return atomic_compare_exchange_strong_explicit(&l->lock, &val, 1,
						       memory_order_acquire,
						       memory_order_acquire);
}

static inline void spinlock_acquire(struct spinlock_s *l)
{
	do {
		await_for_lock(l);
	} while(!try_get_lock(l));
	return;
}

static inline int spinlock_tryacquire(struct spinlock_s *l)
{
	return try_get_lock(l);
}

static inline void spinlock_release(struct spinlock_s *l)
{
	atomic_store_explicit(&l->lock, 0, memory_order_release);
}
