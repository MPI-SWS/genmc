int x;
atomic_int y;

void *thread_1(void *arg)
{
	x = 1;
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *arg)
{
	int expected = 2;
	atomic_compare_exchange_strong_explicit(&y, &expected, 3,
						memory_order_relaxed,
						memory_order_relaxed);
	return NULL;
}

void *thread_3(void *arg)
{
	int expected = 1;
	atomic_compare_exchange_strong_explicit(&y, &expected, 2,
						memory_order_relaxed,
						memory_order_relaxed);
	return NULL;
}

void *thread_4(void *arg)
{
	if (atomic_load_explicit(&y, memory_order_relaxed) > 2)
		x = 2;
	return NULL;
}
