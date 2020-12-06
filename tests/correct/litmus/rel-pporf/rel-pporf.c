/* Test case by: Jonas Oberhauser */

atomic_int x;
atomic_int y;
atomic_int z;

void *runA(void *p)
{
	int a = atomic_load_explicit(&x, memory_order_relaxed);
#ifdef USE_STORE
	atomic_store_explicit(&z, 1, memory_order_release);
#else
	atomic_exchange_explicit(&z, 1, memory_order_release);
#endif
	atomic_store_explicit(&y, 1, memory_order_release);
	return NULL;
}

void *runB(void *q)
{
	int b = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}
