atomic_int x;
atomic_int y;

/* r0 = 1 /\ a = 1 possible under IMM; the revisit of r0 cuts around the create of t2 */
void *thread_1(void *unused)
{
	int a = atomic_load_explicit(&x, memory_order_relaxed);
	atomic_store_explicit(&y, a, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	return NULL;
}
