void __VERIFIER_assume(int);

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	int a = atomic_load_explicit(&x, memory_order_relaxed);
	__VERIFIER_assume(a == 2);

	y = 1;
	return NULL;
}

void *thread_2(void *unused)
{
	y = 2;
	return NULL;
}

void *thread_3(void *unused)
{
	int b = atomic_load_explicit(&y, memory_order_relaxed);
	__VERIFIER_assume(b == 2);

	atomic_store_explicit(&x, b, memory_order_relaxed);
	return NULL;
}
