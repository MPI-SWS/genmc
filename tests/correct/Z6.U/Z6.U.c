atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	atomic_store(&x, 1);
	atomic_store_explicit(&y, 1, memory_order_release);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_fetch_add(&y, 1);
	atomic_load_explicit(&y, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store(&y, 3);
	atomic_load(&x);
	return NULL;
}
