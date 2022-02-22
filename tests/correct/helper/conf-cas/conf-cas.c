#ifndef N
# define N 4
#endif

atomic_int x;

void *thread_n(void *unused)
{
	int r;
	do {
		r = atomic_load_explicit(&x, memory_order_relaxed);
		/* do some work */
	} while (!atomic_compare_exchange_strong_explicit(&x, &r, r + 1, memory_order_relaxed,
							  memory_order_relaxed));
	return NULL;
}
