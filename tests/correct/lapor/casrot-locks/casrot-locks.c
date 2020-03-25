pthread_mutex_t l;

atomic_int x;
int idx[N];

void *thread_n(void *arg)
{
	int new = *((int *) arg);
	int exp = new - 1;

	pthread_mutex_lock(&l);

	if (atomic_load_explicit(&x, memory_order_relaxed) == exp)
		atomic_store_explicit(&x, new, memory_order_relaxed);

	pthread_mutex_unlock(&l);

	return NULL;
}
