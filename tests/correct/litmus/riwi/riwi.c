atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	atomic_load_explicit(&y, memory_order_relaxed);
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 1, memory_order_release);
	return NULL;
}
