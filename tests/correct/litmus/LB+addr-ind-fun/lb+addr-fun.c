atomic_int x;
atomic_int y[2];

void load_store(int a)
{
	int b = atomic_load_explicit(y + a, memory_order_relaxed);
	atomic_store_explicit(y, 1, memory_order_relaxed);
}

void *thread_1(void *unused)
{
	void (*fp)(int) = &load_store;

	int a = atomic_load_explicit(&x, memory_order_relaxed);
	fp(a);
	return NULL;
}

void *thread_2(void *unused)
{
	int r = atomic_load_explicit(y, memory_order_relaxed);
	atomic_store_explicit(&x, r, memory_order_release);
	return NULL;
}
