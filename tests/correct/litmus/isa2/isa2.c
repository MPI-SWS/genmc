atomic_int a;
atomic_int b;
atomic_int c;

void *thread_1(void *unused)
{
	atomic_store_explicit(&a, 1, memory_order_relaxed);
	atomic_store_explicit(&b, 1, memory_order_release);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_load_explicit(&b, memory_order_acquire);
	atomic_store_explicit(&c, 1, memory_order_release);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_load_explicit(&c, memory_order_acquire);
	atomic_load_explicit(&a, memory_order_relaxed);
	return NULL;
}
