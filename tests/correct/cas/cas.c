atomic_int x;
int idx[N];

void *thread_n(void *arg)
{
	int expected = 0;
	int new = *((int *) arg);
	atomic_compare_exchange_strong_explicit(&x, &expected, new, memory_order_relaxed,
						memory_order_relaxed);
	return NULL;
}
