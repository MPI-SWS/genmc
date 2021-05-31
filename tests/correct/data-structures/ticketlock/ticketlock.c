struct ticketlock_s {
	atomic_int next;
	atomic_int owner;
};
typedef struct ticketlock_s ticketlock_t;

static inline void ticketlock_init(struct ticketlock_s *l)
{
	atomic_init(&l->next, 0);
	atomic_init(&l->owner, 0);
}

static inline int get_next_ticket(struct ticketlock_s *l)
{
	return atomic_fetch_add_explicit(&l->next, 1, memory_order_relaxed);
}

static inline void await_for_ticket(struct ticketlock_s *l, int ticket)
{
	while (atomic_load_explicit(&l->owner, memory_order_acquire) != ticket)
		;
}

static inline void ticketlock_acquire(struct ticketlock_s *l)
{
	int ticket = get_next_ticket(l);
	await_for_ticket(l, ticket);
}

static inline int ticketlock_tryacquire(struct ticketlock_s *l)
{
	int o = atomic_load_explicit(&l->owner, memory_order_acquire);
	int n = atomic_compare_exchange_strong_explicit(&l->next, &o, o + 1,
							memory_order_relaxed,
							memory_order_relaxed);
	return n == o;
}

static inline void ticketlock_release(struct ticketlock_s *l)
{
	int owner = atomic_load_explicit(&l->owner, memory_order_relaxed);
	atomic_store_explicit(&l->owner, owner + 1, memory_order_release);
}
