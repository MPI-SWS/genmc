_Atomic(atomic_int *) p;

int *x[2];

void *thread1(void *unused)
{
	p = malloc(sizeof(atomic_int));
	atomic_store_explicit(p, 42, memory_order_relaxed);
	return NULL;
}

void *thread2(void *unused)
{
	atomic_int *a = atomic_load_explicit(&p, memory_order_acquire);
	atomic_load_explicit(a, memory_order_relaxed);
	return NULL;
}
