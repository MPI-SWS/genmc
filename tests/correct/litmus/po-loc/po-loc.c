atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	int r0 = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_thread_fence(memory_order_acq_rel);
	int r1 = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r2 = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&y, r2, memory_order_relaxed);
	return NULL;
}
