atomic_int x;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 5, memory_order_release);
	return NULL;
}

void *thread_2(void *unused)
{
	int expected = 1;
	atomic_compare_exchange_strong_explicit(&x, &expected, 2,
						memory_order_relaxed,
						memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_4(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}
