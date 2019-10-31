atomic_int x;
atomic_int y;
atomic_int z;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int a = atomic_load_explicit(&z, memory_order_relaxed);
	atomic_store_explicit(&x, a - 1, memory_order_relaxed);
	int b = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&y, b, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	int c = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_store_explicit(&z, c, memory_order_relaxed);
	return NULL;
}
