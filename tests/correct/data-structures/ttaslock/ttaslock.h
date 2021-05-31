#ifndef __TTASLOCK_H__
#define __TTASLOCK_H__

struct ttaslock_s {
	atomic_int state;
};
typedef struct ttaslock_s ttaslock_t;

static inline void ttaslock_init(struct ttaslock_s *l);
static inline void ttaslock_acquire(struct ttaslock_s *l);
static inline void ttaslock_release(struct ttaslock_s *l);

#endif /* __TTASLOCK_H__ */
