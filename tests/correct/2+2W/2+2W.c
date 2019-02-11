atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_release);
	atomic_store_explicit(&y, 2, memory_order_release);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&y, 1, memory_order_release);
	atomic_store_explicit(&x, 2, memory_order_release);
	return NULL;
}
