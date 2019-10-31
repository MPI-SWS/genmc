atomic_int x;
int idx[N];

void *thread_n(void *arg)
{
	int v = *((int *) arg);

	atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&x, v, memory_order_release);
	return NULL;
}
