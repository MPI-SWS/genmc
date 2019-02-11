atomic_int x;

void *thread_1(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&x, 4, memory_order_release);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_fetch_add_explicit(&x, 2, memory_order_relaxed);
	return NULL;
}
