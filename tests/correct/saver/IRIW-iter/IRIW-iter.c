atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		atomic_store_explicit(&x, i, memory_order_release);
	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		atomic_store_explicit(&y, i, memory_order_release);
	return NULL;
}

void *thread_3(void *unused)
{
	int a = atomic_load_explicit(&x, memory_order_acquire);
	__VERIFIER_assume(a == 42);
	int b = atomic_load_explicit(&y, memory_order_acquire);
	__VERIFIER_assume(b == 0);
	return NULL;
}

void *thread_4(void *unused)
{
	int a = atomic_load_explicit(&y, memory_order_acquire);
	__VERIFIER_assume(a == 0);
	int b = atomic_load_explicit(&x, memory_order_acquire);
	__VERIFIER_assume(b == 42);
	return NULL;
}
