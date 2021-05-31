#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

struct spinlock_s {
	atomic_int lock;
};

typedef struct spinlock_s spinlock_t;

static inline void spinlock_init(struct spinlock_s *l);
static inline void spinlock_acquire(struct spinlock_s *l);
static inline int spinlock_tryacquire(struct spinlock_s *l);
static inline void spinlock_release(struct spinlock_s *l);

#endif /* __SPINLOCK_H__ */
