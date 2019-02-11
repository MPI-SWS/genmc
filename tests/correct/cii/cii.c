atomic_int x;

void *thread_1(void *unused)
{
	int r = 1;

	atomic_compare_exchange_strong_explicit(&x, &r, 2, memory_order_relaxed,
						memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}
