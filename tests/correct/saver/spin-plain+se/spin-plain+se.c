atomic_int x;

void *thread_1(void *unused)
{
	while (1) {
		if (atomic_load_explicit(&x, memory_order_relaxed) == 42) {
			continue;
		} else {
			atomic_store_explicit(&x, 42, memory_order_relaxed);
			continue;
		}
	}
	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 0u; i < 2; i++)
		x = i;
	return NULL;
}
