atomic_int x;
atomic_int y;

void *thread_1(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 0, memory_order_release);
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *arg)
{
	int r_x;

	if (atomic_load_explicit(&y, memory_order_relaxed)) {
		atomic_thread_fence(memory_order_acquire);
		r_x = atomic_load_explicit(&x, memory_order_relaxed);
	}
	return NULL;
}
