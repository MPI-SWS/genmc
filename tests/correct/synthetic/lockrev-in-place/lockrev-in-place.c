pthread_mutex_t l;
atomic_int x;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 42, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&l);
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_3(void *unused)
{
	pthread_mutex_lock(&l);
	atomic_store_explicit(&x, 2, memory_order_relaxed);
	atomic_load_explicit(&x, memory_order_relaxed);
	pthread_mutex_unlock(&l);
	return NULL;
}
