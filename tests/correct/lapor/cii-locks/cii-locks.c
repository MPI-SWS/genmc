pthread_mutex_t l;

atomic_int x;

void *thread_1(void *unused)
{
	int r = 1;

	pthread_mutex_lock(&l);
	if (atomic_load_explicit(&x, memory_order_relaxed) == r)
		atomic_store_explicit(&x, 2, memory_order_relaxed);
	pthread_mutex_unlock(&l);

	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&l);
	int r = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&x, r + 1, memory_order_relaxed);
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_3(void *unused)
{
	pthread_mutex_lock(&l);
	int r = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&x, r + 1, memory_order_relaxed);
	pthread_mutex_unlock(&l);
	return NULL;
}
