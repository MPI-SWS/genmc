/*
 * spin-dowhile1: do-while that only contains plain paths
 */

atomic_int x;

void *thread_1(void *unused)
{
	do {
		if (atomic_load_explicit(&x, memory_order_acquire) == 42)
				break;
	} while (atomic_load_explicit(&x, memory_order_relaxed) != 0);

	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		atomic_store_explicit(&x, i, memory_order_relaxed);
	return NULL;
}
