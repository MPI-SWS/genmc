atomic_int a;
atomic_int b;
atomic_int c;

void *thread_1(void *unused)
{
	atomic_store_explicit(&a, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store_explicit(&b, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r1 = atomic_load_explicit(&b, memory_order_acquire);
	atomic_store_explicit(&c, 1, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store_explicit(&c, 2, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_load_explicit(&a, memory_order_relaxed);
	return NULL;
}
