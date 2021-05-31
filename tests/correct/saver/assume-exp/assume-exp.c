#ifndef NUM
# define NUM 4
#endif

atomic_int x;
atomic_int a[8];

void *thread_1(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		x = i;
	return NULL;
}

void *thread_2(void *unused)
{
	__VERIFIER_assume(x == 42);
	return NULL;
}

void *thread_3(void *unused)
{
	for (int i = 0u; i < NUM; i++) {
		for (int j = 0u; j < 8; j++)
			atomic_store_explicit(&a[j], i, memory_order_relaxed);
	}
	return NULL;
}

void *thread_4(void *unused)
{
	for (int i = 0u; i < 8; i++)
		atomic_load_explicit(&a[i], memory_order_relaxed);
	return NULL;
}
