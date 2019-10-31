atomic_int x, y;

_Atomic(atomic_int *) p;

void *thread1(void *unused)
{
	x = 42;
	return NULL;
}

void *thread2(void *unused)
{
	if (x == 0)
		p = malloc(sizeof(atomic_int));

	y = 17;
	return NULL;
}

void *thread3(void *unused)
{
	if (y == 17) {
		atomic_int *a = atomic_load_explicit(&p, memory_order_acquire);
		atomic_store_explicit(a, 42, memory_order_relaxed);
	}
	return NULL;
}
