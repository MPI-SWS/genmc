atomic_bool flag1; /* Boolean flags */
atomic_bool flag2;

atomic_int turn;   /* Atomic integer that holds the ID of the thread whose turn it is */
atomic_bool x;     /* Boolean variable to test mutual exclusion */

void *thread_1(void *arg)
{
	atomic_store_explicit(&flag1, 1, memory_order_release);
	atomic_store_explicit(&turn, 1, memory_order_release);

	__VERIFIER_assume(atomic_load_explicit(&flag2, memory_order_acquire) != 1 ||
			  atomic_load_explicit(&turn, memory_order_acquire) != 1);

	/* critical section beginning */
	atomic_store_explicit(&x, 0, memory_order_release);
//	assert(atomic_load_explicit(&x, memory_order_acquire) <= 0);
	atomic_load_explicit(&x, memory_order_acquire);
	/* critical section ending */

	atomic_store_explicit(&flag1, 0, memory_order_release);
	return NULL;
}

void *thread_2(void *arg)
{
	atomic_store_explicit(&flag2, 1, memory_order_release);
	atomic_store_explicit(&turn, 0, memory_order_release);

	__VERIFIER_assume(atomic_load_explicit(&flag1, memory_order_acquire) != 1 ||
			  atomic_load_explicit(&turn, memory_order_acquire) != 0);

	/* critical section beginning */
	atomic_store_explicit(&x, 1, memory_order_release);
//	assert(atomic_load_explicit(&x, memory_order_acquire) >= 1);
	atomic_load_explicit(&x, memory_order_acquire);
	/* critical section ending */

	atomic_store_explicit(&flag2, 0, memory_order_release);
	return NULL;
}
