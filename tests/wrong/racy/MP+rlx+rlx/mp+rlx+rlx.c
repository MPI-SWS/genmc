int x;
atomic_int y;

void *thread_1(void *arg)
{
	x = 1;
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *arg)
{
	int r_x;

	if (atomic_load_explicit(&y, memory_order_relaxed))
		r_x = x;
	return NULL;
}
