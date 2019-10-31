_Atomic(pthread_t) pid;

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 42, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	if (pthread_join(pid, NULL))
		abort();

	assert(atomic_load_explicit(&x, memory_order_relaxed) == 42);
	atomic_store_explicit(&y, 42, memory_order_relaxed);
	return NULL;
}
