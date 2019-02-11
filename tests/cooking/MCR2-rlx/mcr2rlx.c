atomic_int x;
atomic_int y;

pthread_mutex_t l;

void *thread_1(void *ptr)
{
	for (int i = 0; i < 2; i++) {
		pthread_mutex_lock(&l);
		atomic_store_explicit(&x, 0, memory_order_relaxed);
		pthread_mutex_unlock(&l);
		if (atomic_load_explicit(&x, memory_order_relaxed) > 0) {
			atomic_fetch_add_explicit(&y, 1, memory_order_relaxed);
			atomic_store_explicit(&x, 2, memory_order_relaxed);
		}
	}
	return NULL;
}

void *thread_2(void *ptr)
{
	if (atomic_load_explicit(&x, memory_order_relaxed) > 1) {
		if (atomic_load_explicit(&y, memory_order_relaxed) == 3) {
			/* assert(0); */
			/* printf("*****error*****\n"); */
			;
		} else {
			atomic_store_explicit(&y, 2, memory_order_relaxed);
		}
	}
	return NULL;
}

void *thread_3(void *unused)
{
	for (int i = 0; i < 2; i++) {
		pthread_mutex_lock(&l);
		atomic_store_explicit(&x, 1, memory_order_relaxed);
		atomic_store_explicit(&y, 1, memory_order_relaxed);
		pthread_mutex_unlock(&l);
	}
	return NULL;
}
