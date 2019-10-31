atomic_int x;
atomic_int y;
atomic_int z1;
atomic_int z2;

void *thread_1(void *unused)
{
	atomic_store_explicit(&y, 1, memory_order_seq_cst);
	if (!atomic_load_explicit(&x, memory_order_seq_cst)) {
		atomic_store_explicit(&z1, 1, memory_order_relaxed);
	}
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_seq_cst);
	if (!atomic_load_explicit(&y, memory_order_seq_cst)) {
		atomic_store_explicit(&z2, 1, memory_order_relaxed);
	}
	return NULL;
}

void *thread_3(void *unused)
{
	if (atomic_load_explicit(&z1, memory_order_relaxed) &&
	    atomic_load_explicit(&z2, memory_order_relaxed)) {
		for (int i = 0; i < N; i++) {
			atomic_store_explicit(&x, i, memory_order_relaxed);
		}
	}
	return NULL;
}

void *thread_4(void *unused)
{
	if (atomic_load_explicit(&z1, memory_order_relaxed) &&
	    atomic_load_explicit(&z2, memory_order_relaxed)) {
		for (int i = 0; i < N; i++) {
			atomic_load_explicit(&x, memory_order_relaxed);
		}
	}
	return NULL;
}
