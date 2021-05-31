#ifndef __MUTEX_MUSL_H__
#define __MUTEX_MUSL_H__

#include "futex.h"

typedef struct {
	atomic_int lock;
	atomic_int waiters;
} mutex_t;

static inline void mutex_init(mutex_t *m);
static inline int mutex_lock_fastpath(mutex_t *m);
static inline int mutex_lock_slowpath_check(mutex_t *m);
static inline void mutex_lock(mutex_t *m);
static inline void mutex_unlock(mutex_t *m);

#endif
