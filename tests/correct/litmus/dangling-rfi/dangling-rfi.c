atomic_int x;
atomic_int y;
atomic_int z;

/* a = 2 /\ b = 3 possible under IMM */
void *thread_1(void *unused)
{
	int a = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&y, a + 1, memory_order_relaxed);
	int b = atomic_load_explicit(&y, memory_order_acquire);
	atomic_store_explicit(&z, 1, memory_order_relaxed);
	__VERIFIER_assume(a == 2 && b == 3);
	return NULL;
}

void *thread_2(void *unused)
{
	int c = atomic_load_explicit(&z, memory_order_relaxed);
	atomic_store_explicit(&x, c + 1, memory_order_relaxed);
	return NULL;
}
