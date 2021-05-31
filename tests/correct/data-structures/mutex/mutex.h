#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "futex.h"

typedef atomic_int mutex_t;

static inline void mutex_init(mutex_t *m);
static inline int mutex_lock_fastpath(mutex_t *m);
static inline int mutex_lock_try_acquire(mutex_t *m);
static inline void mutex_lock(mutex_t *m);
static inline int mutex_unlock_fastpath(mutex_t *m);
static inline void mutex_unlock(mutex_t *m);

#endif /* __MUTEX_H__ */
