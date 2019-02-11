atomic_int x;

void *thread_0(void *unused)
{
	atomic_store_explicit(&x, 2, memory_order_release);
	return NULL;
}

void *thread_n(void *unused)
{
	int r = 2;
	atomic_compare_exchange_strong_explicit(&x, &r, 3, memory_order_relaxed,
						memory_order_relaxed);
	return NULL;
}
