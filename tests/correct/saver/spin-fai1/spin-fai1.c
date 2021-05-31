atomic_int x;

void *thread_1(void *unused)
{
	while (true) {
		atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
		atomic_fetch_sub_explicit(&x, 1, memory_order_relaxed);
	}
	return NULL;
}

void *thread_2(void *unused)
{
	while (true) {
		atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
		atomic_fetch_add_explicit(&x, -1, memory_order_relaxed);
	}
	return NULL;
}

void *thread_3(void *unused)
{
	while (true) {
		atomic_fetch_sub_explicit(&x, 1, memory_order_relaxed);
		atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	}
	return NULL;
}
