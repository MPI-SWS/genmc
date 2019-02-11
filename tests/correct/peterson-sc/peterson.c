atomic_bool flag1; /* Boolean flags */
atomic_bool flag2;

atomic_int turn;   /* Atomic integer that holds the ID of the thread whose turn it is */
atomic_bool x;     /* Boolean variable to test mutual exclusion */

void __VERIFIER_assume(int);

void *thread_1(void *arg)
{
	atomic_store_explicit(&flag1, 1, memory_order_seq_cst);
	atomic_store_explicit(&turn, 1, memory_order_seq_cst);

	__VERIFIER_assume(atomic_load_explicit(&flag2, memory_order_seq_cst) != 1 ||
			  atomic_load_explicit(&turn, memory_order_seq_cst) != 1);

	/* critical section beginning */
	atomic_store_explicit(&x, 0, memory_order_seq_cst);
//	assert(atomic_load_explicit(&x, memory_order_seq_cst) <= 0);
	atomic_load_explicit(&x, memory_order_seq_cst);
	/* critical section ending */

	atomic_store_explicit(&flag1, 0, memory_order_seq_cst);
	return NULL;
}

void *thread_2(void *arg)
{
	atomic_store_explicit(&flag2, 1, memory_order_seq_cst);
	atomic_store_explicit(&turn, 0, memory_order_seq_cst);

	__VERIFIER_assume(atomic_load_explicit(&flag1, memory_order_seq_cst) != 1 ||
			  atomic_load_explicit(&turn, memory_order_seq_cst) != 0);

	/* critical section beginning */
	atomic_store_explicit(&x, 1, memory_order_seq_cst);
//	assert(atomic_load_explicit(&x, memory_order_seq_cst) >= 1);
	atomic_load_explicit(&x, memory_order_seq_cst);
	/* critical section ending */

	atomic_store_explicit(&flag2, 0, memory_order_seq_cst);
	return NULL;
}
