atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_acq_rel);
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r1 = atomic_exchange_explicit(&x, 1, memory_order_relaxed);
	int r2 = atomic_exchange_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}
/* exists 1:r1=1 /\ 1:r2=0 */
