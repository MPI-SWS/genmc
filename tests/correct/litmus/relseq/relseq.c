atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 1, memory_order_release);
	atomic_store_explicit(&y, 2, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_exchange_explicit(&y, 3, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_load_explicit(&y, memory_order_relaxed);
	atomic_thread_fence(memory_order_acq_rel);
	atomic_load_explicit(&x, memory_order_relaxed);
	return NULL;
}
