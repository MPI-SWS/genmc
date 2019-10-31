pthread_mutex_t mutex;
atomic_int x;

void *thread_n(void *unused)
{
	int r;

	pthread_mutex_lock(&mutex);
	r = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&x, r + 1, memory_order_relaxed);
	pthread_mutex_unlock(&mutex);
	return NULL;
}
