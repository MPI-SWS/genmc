atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	int r = atomic_load_explicit(&x, memory_order_relaxed);
	if (!r)
		atomic_store_explicit(&y, 1, memory_order_relaxed);
	else
		atomic_store_explicit(&y, 2, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_store_explicit(&x, 1, memory_order_release);
	return NULL;
}
