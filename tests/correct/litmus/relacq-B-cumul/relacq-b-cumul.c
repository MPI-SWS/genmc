atomic_int x;
atomic_int y;
atomic_int z;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 1, memory_order_release);
}

void *thread_2(void *unused)
{
	int r1 = atomic_load_explicit(&y, memory_order_acquire);
	atomic_store_explicit(&z, 1, memory_order_relaxed);
}

void *thread_3(void *unused)
{
	int r2 = atomic_load_explicit(&z, memory_order_relaxed);
	atomic_thread_fence(memory_order_acq_rel);
	int r3 = atomic_load_explicit(&x, memory_order_relaxed);
}
