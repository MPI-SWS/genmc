atomic_int x;

void *thread_1(void *unused)
{
	int r;
	do {
		r = atomic_load_explicit(&x, memory_order_relaxed);
		/* do work */
	} while (atomic_compare_exchange_strong_explicit(&x, &r, 100, memory_order_relaxed,
							 memory_order_relaxed) + 41 != 42);
	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 0u; i < 40; i++)
		atomic_store_explicit(&x, i, memory_order_relaxed);
	return NULL;
}
