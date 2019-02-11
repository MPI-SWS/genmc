atomic_int x;

void *thread_1(void *unused)
{
	int r = 0;
	atomic_compare_exchange_strong_explicit(&x, &r, 42, memory_order_relaxed,
						            memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r = 0;
	atomic_compare_exchange_strong_explicit(&x, &r, 17, memory_order_relaxed,
						            memory_order_relaxed);
	atomic_load_explicit(&x, memory_order_relaxed);
	return NULL;
}
