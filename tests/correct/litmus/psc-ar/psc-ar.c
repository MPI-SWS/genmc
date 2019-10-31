void __VERIFIER_assume(int);

atomic_int x;
atomic_int y;
atomic_int z;

int res;

void *thread_1(void *unused)
{
	int a = atomic_load_explicit(&y, memory_order_relaxed);
	__VERIFIER_assume(a == 1);

	atomic_thread_fence(memory_order_seq_cst);

	int b = atomic_load_explicit(&z, memory_order_relaxed);
	__VERIFIER_assume(b == 0);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&z, 1, memory_order_relaxed);

	atomic_thread_fence(memory_order_seq_cst);

	atomic_store_explicit(&x, 1, memory_order_relaxed);
	/* __VERIFIER_assume(c == 1); */
	return NULL;
}

void *thread_3(void *unused)
{
	int d = atomic_load_explicit(&x, memory_order_relaxed);
	if (d == 1)
		atomic_store_explicit(&y, 1, memory_order_relaxed);

	__VERIFIER_assume(d == 1);
	res = d; /* Used in an assertion in main(), to make sure it's invalid */
	return NULL;
}
