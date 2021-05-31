/* Simple implementation of a lock */

typedef atomic_int lock_t;

void lock(lock_t *lock)
{
	int r = 0;
	while (!atomic_compare_exchange_strong_explicit(lock, &r, 1, memory_order_acquire,
							memory_order_acquire))
		r = 0;
	return;
}

void unlock(lock_t *lock)
{
	atomic_store_explicit(lock, 0, memory_order_release);
	return;
}

/* Actual test case -- deadlock due to lock acquisition order */
lock_t lock_a;
lock_t lock_b;

int data;

void *thread_1(void *unused)
{
	lock(&lock_a);
	lock(&lock_b);

	data = 42;

	unlock(&lock_b);
	unlock(&lock_a);
	return NULL;
}

void *thread_2(void *unused)
{
	lock(&lock_b);
	lock(&lock_a);

	data = 17;

	unlock(&lock_a);
	unlock(&lock_b);
	return NULL;
}
