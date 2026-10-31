_Atomic(atomic_uint *) p;
atomic_int done1, done2;

void *thread_1(void *unused)
{
	atomic_uint *x = malloc(sizeof(*x));

	atomic_store_explicit(&p, x, memory_order_release);
	atomic_store_explicit(x, 1, memory_order_relaxed);
	atomic_store_explicit(&done1, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_uint *x = atomic_load_explicit(&p, memory_order_acquire);
	if (x) {
		atomic_store_explicit(x, 2, memory_order_relaxed);
		atomic_store_explicit(&done2, 1, memory_order_relaxed);
	}
	return NULL;
}

void *thread_3(void *unused)
{
	if (atomic_load_explicit(&done1, memory_order_relaxed) &&
	    atomic_load_explicit(&done2, memory_order_relaxed)) {
		atomic_uint *x = atomic_load_explicit(&p, memory_order_acquire);
		if (x) {
			unsigned r = atomic_load_explicit(x, memory_order_relaxed);
			(void)r;
		}
	}
	return NULL;
}
