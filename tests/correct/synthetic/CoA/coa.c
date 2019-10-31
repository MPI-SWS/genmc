atomic_int x;
atomic_int y;
atomic_int z;

int idx[N];

void *thread_one(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_release);
	atomic_load_explicit(&y, memory_order_acquire);
	atomic_load_explicit(&x, memory_order_acquire);
	return NULL;
}

void *thread_two(void *unused)
{
	atomic_store_explicit(&x, 2, memory_order_release);
	atomic_load_explicit(&z, memory_order_acquire);
	atomic_load_explicit(&x, memory_order_acquire);
	return NULL;
}

void *thread_n(void *arg)
{
	int i = *((int *) arg);

	atomic_store_explicit(&y, i, memory_order_release);
	atomic_store_explicit(&z, i, memory_order_release);
	return NULL;
}
