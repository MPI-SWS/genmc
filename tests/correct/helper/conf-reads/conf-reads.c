#ifndef N
# define N 2
#endif

atomic_int x;

void *thread_writer(void *unused)
{
	for (int i = 0u; i < 10; i++)
		atomic_store_explicit(&x, i, memory_order_relaxed);
	return NULL;
}

void *thread_n(void *unused)
{
	int a, b;
	do {
		a = atomic_load_explicit(&x, memory_order_relaxed);
		/* do some work */
		b = atomic_load_explicit(&x, memory_order_relaxed);
	} while (a != b);
	return NULL;
}
