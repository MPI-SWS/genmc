atomic_int x;
atomic_int y;
atomic_int z;

void *thread_1(void *unused)
{
	int a = atomic_load_explicit(&x, memory_order_relaxed);
	int b = atomic_compare_exchange_strong_explicit(&y, &a, 1, memory_order_relaxed,
							memory_order_relaxed);
	atomic_store_explicit(&z, a, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&y, 2, memory_order_relaxed);
	return NULL;
}
