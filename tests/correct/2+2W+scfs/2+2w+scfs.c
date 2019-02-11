atomic_int x;
atomic_int y;

void *thread_1(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store_explicit(&y, 2, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *arg)
{
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store_explicit(&x, 2, memory_order_relaxed);
	return NULL;
}
