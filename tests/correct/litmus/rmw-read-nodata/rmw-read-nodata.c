atomic_int x;
atomic_int y;

/* r0 = 1 with the CAS reading y = 0 is possible because only the CAS write depends on
 * the read of x */
void *thread_1(void *unused)
{
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r0 = atomic_load_explicit(&x, memory_order_relaxed);
	int r1 = 0;
	atomic_compare_exchange_strong_explicit(&y, &r1, r0, memory_order_relaxed,
						memory_order_relaxed);
	return NULL;
}
