atomic_int x;
atomic_int y;
atomic_int z;

void *thread_1(void *unused)
{
	atomic_load_explicit(&y, memory_order_acquire);
	atomic_fetch_add_explicit(&z, 1, memory_order_acq_rel);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_fetch_add_explicit(&z, 1, memory_order_acq_rel);
	atomic_load_explicit(&x, memory_order_acquire);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_release);
	atomic_store_explicit(&y, 1, memory_order_release);
	return NULL;
}
