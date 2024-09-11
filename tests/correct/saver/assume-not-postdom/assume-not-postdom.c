#ifndef N
# define N 42
#endif

atomic_int x;

void *thread_1(void *unused)
{
	for (int i = 1u; i <= N; i++)
		atomic_store_explicit(&x, i, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int a = 0;

	int b = atomic_load_explicit(&x, memory_order_relaxed);

	if (a < N)
		__VERIFIER_assume(b == N);

	return NULL;
}
