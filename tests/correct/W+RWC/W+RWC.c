atomic_int x;
atomic_int y;
atomic_int z;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&z, 1, memory_order_release);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_load_explicit(&z, memory_order_acquire);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_load_explicit(&y, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_load_explicit(&x, memory_order_relaxed);
	return NULL;
}
