atomic_int x;

void *thread_0(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_release);
	return NULL;
}

void *thread_n(void *arg)
{
	int r = *((int *) arg);
	atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&x, r, memory_order_release);
	return NULL;
}
