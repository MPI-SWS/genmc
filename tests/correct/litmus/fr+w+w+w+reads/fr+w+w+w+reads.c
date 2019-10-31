atomic_int x;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&x, 2, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store_explicit(&x, 3, memory_order_relaxed);
	return NULL;
}

void *thread_4(void *unused)
{
	int r1 = atomic_load_explicit(&x, memory_order_relaxed);
	int r2 = atomic_load_explicit(&x, memory_order_relaxed);
	int r3 = atomic_load_explicit(&x, memory_order_relaxed);
	int r4 = atomic_load_explicit(&x, memory_order_relaxed);
	return NULL;
}

/* exists */
/* (3:r1=0 /\ 3:r2=1 /\ 3:r3=2 /\ 3:r4=3) */
