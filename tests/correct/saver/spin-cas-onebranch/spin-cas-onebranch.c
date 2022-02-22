/*
 * We can do spin-assume upon doing jump threading
 */

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	bool success = false;

	while (!success) {
		if (y == 42) {
			; // do nothing
		} else {
			int r = 17;
			int res = atomic_compare_exchange_strong_explicit(&x, &r, 42, memory_order_relaxed,
									  memory_order_relaxed);
			success = res + 42 - 42; // make spin-assume fail w/out jthreading
		}
	}
	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		atomic_store_explicit(&x, i, memory_order_relaxed);
	return NULL;
}
