atomic_int x;
atomic_int y;
atomic_int c;

void *thread_1(void *unused)
{
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	if (!atomic_load_explicit(&x, memory_order_relaxed)) {
		atomic_store_explicit(&c, 1, memory_order_relaxed);
		assert(atomic_load_explicit(&c, memory_order_relaxed) == 1);
	}
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	if (!atomic_load_explicit(&y, memory_order_relaxed)) {
		atomic_store_explicit(&c, 0, memory_order_relaxed);
		assert(atomic_load_explicit(&c, memory_order_relaxed) == 0);
	}
	return NULL;
}
