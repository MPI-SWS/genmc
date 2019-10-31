atomic_int x = ATOMIC_VAR_INIT(3);
atomic_int y = ATOMIC_VAR_INIT(4);

_Atomic(atomic_int *) p;

void *thread_one(void *unused)
{
	int c1 = 0;
	atomic_int * t;

	atomic_store_explicit(&p, &y, memory_order_release);
	for (int i = 0; i < N; i++)
		c1 += atomic_load_explicit(&x, memory_order_acquire);
	t = atomic_load_explicit(&p, memory_order_acquire);
	atomic_store_explicit(t, atomic_load_explicit(t, memory_order_acquire) + 3,
			      memory_order_release);
	/* assert(3 <= x && x <= 9); */
	/* assert(3 <= y && y <= 9); */
	return NULL;
}

void *thread_two(void *unused)
{
	int c2 = 0;
	atomic_int * t;

	atomic_store_explicit(&p, &x, memory_order_release);
	for (int i = 0; i < N; i++)
		c2 += atomic_load_explicit(&y, memory_order_acquire);
	t = atomic_load_explicit(&p, memory_order_acquire);
	atomic_store_explicit(t, atomic_load_explicit(t, memory_order_acquire) + 3,
			      memory_order_release);
	/* assert(3 <= x && x <= 9); */
	/* assert(3 <= y && y <= 9); */
	return NULL;
}
