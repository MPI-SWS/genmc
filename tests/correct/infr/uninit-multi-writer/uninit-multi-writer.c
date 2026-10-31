/* In this test, the Wx2 in T2 might be added before the Wx1 in T2,
 * but not uninit error should be reported */

_Atomic(atomic_uint *) p;

void *thread_1(void *unused)
{
	atomic_uint *x = malloc(sizeof(*x));

	atomic_store_explicit(&p, x, memory_order_release);
	atomic_store_explicit(x, 1, memory_order_relaxed);
	unsigned r = atomic_load_explicit(x, memory_order_relaxed);
	(void)r;
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_uint *x = atomic_load_explicit(&p, memory_order_acquire);
	if (x)
		atomic_store_explicit(x, 2, memory_order_relaxed);
	return NULL;
}
