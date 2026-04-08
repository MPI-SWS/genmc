atomic_int x;
atomic_int y;

void *thread_1(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 0, memory_order_release);
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *arg)
{
	if (atomic_load_explicit(&y, memory_order_acquire))
		atomic_store_explicit(&x, 2, memory_order_relaxed);
	return NULL;
}
