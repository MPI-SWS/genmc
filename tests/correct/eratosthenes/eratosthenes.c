
atomic_int naturals[N];

void sieve()
{
	int i = 0;
	int j = 0;
	while (i < N) {
		if (!atomic_load_explicit(&naturals[i], memory_order_acquire)) {
			atomic_store_explicit(&naturals[i], 1, memory_order_release);
			j = 2 * i + 2;
			while (j < N) {
				atomic_store_explicit(&naturals[j], 2, memory_order_release);
				j = j + i + 2;
			}
		}
		++i;
	}
	return;
}

void *thread_1(void *arg)
{
	sieve();
	return NULL;
}

void *thread_2(void *arg)
{
	sieve();
	return NULL;
}
