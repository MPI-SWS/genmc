atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	int r1 = atomic_load_explicit(&x, memory_order_relaxed);
	if (r1 >= 0)
		atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r2 = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_store_explicit(&x, r2, memory_order_relaxed);
	return NULL;
}
