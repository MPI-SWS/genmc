atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 1, memory_order_release);
	return NULL;
}

void *thread_2(void *unused)
{
	int r = 0;
	atomic_compare_exchange_strong_explicit(&x, &r, 1, memory_order_acquire,
						memory_order_acquire);
	return NULL;
}

void *thread_3(void *unused)
{
	if (atomic_load_explicit(&y, memory_order_relaxed))
		atomic_store_explicit(&x, 4, memory_order_release);
	return NULL;
}
