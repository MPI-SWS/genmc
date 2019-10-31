atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	int r0 = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&y, 1 + (r0 ^ r0), memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r0 = atomic_load_explicit(&y, memory_order_relaxed);
	int r1 = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_store_explicit(&x, 1 + (r1 ^ r1), memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store_explicit(&y, 2, memory_order_relaxed);
	return NULL;
}
/* exists (0:r0=1 /\ 1:r0=1) */
