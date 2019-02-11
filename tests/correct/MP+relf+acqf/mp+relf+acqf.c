int x;
atomic_int y;

void *thread_1(void *arg)
{
	x = 1;
	atomic_thread_fence(memory_order_release);
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *arg)
{
	int r_x;

	if (atomic_load_explicit(&y, memory_order_relaxed)) {
		atomic_thread_fence(memory_order_acquire);
		r_x = x;
	}
	return NULL;
}
