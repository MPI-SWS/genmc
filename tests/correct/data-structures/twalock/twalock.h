/* Adapted from "Ticket Locks Augmented with a Waiting Array" @ EuroPar'19 */

#ifndef __TWALOCK_H__
#define __TWALOCK_H__

struct twalock_s {
	atomic_int ticket;
	atomic_int grant;
};
typedef struct twalock_s twalock_t;

#define TWA_L 1
#define TWA_A 4096
#define TWA_DIFF(a,b) (a-b)
#define TWA_HASH(t) ((t) % TWA_A)

struct twa_counter_s {
	atomic_int val;
};
typedef struct twa_counter_s twa_counter_t;

/* waiting array shared among lock instances */
extern twa_counter_t __twa_array[TWA_A];

/* declares the waiting array */
#define TWALOCK_ARRAY_DECL twa_counter_t __twa_array[TWA_A];

/* Initializes the lock */
#define TWALOCK_INIT() {               \
	.ticket = ATOMIC_VAR_INIT(0),  \
	.grant = ATOMIC_VAR_INIT(0) }

static inline void twalock_init(struct twalock_s *l);
static inline void twalock_acquire(struct twalock_s *l);
static inline void twalock_release(struct twalock_s *l);
static inline int twalock_tryacquire(struct twalock_s *l);

#endif /* __TWALOCK_H__ */
