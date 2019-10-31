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
	atomic_fetch_add_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *arg)
{
	if (atomic_load_explicit(&y, memory_order_relaxed) > 1) {
		atomic_thread_fence(memory_order_acquire);
		x = 2;
	}
	return NULL;
}
