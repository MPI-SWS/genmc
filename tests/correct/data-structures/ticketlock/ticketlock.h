#ifndef __TICKETLOCK_H__
#define __TICKETLOCK_H__

struct ticketlock_s {
	atomic_int next;
	atomic_int owner;
};
typedef struct ticketlock_s ticketlock_t;

static inline void ticketlock_init(struct ticketlock_s *l);
static inline void ticketlock_acquire(struct ticketlock_s *l);
static inline int ticketlock_tryacquire(struct ticketlock_s *l);
static inline void ticketlock_release(struct ticketlock_s *l);

#endif /* __TICKETLOCK_H__ */
