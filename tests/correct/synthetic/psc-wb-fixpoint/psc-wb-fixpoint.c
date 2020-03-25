/*
 * PSC/WB Fixpoint:
 *
 * A test case demonstrating why we should add WB edges not just
 * between SC writes, but also between an SC write and a non-SC
 * write. For this test, a WB edge should be added between Wx1 and
 * Wx2, which wouldn't have been added, if we were only adding edges
 * between SC writes.
 */

void __VERIFIER_assume(int);

atomic_int x;
atomic_int y;
atomic_int z;

void *thread_1(void *unused)
{
	x = 1;
	__VERIFIER_assume(y == 0);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&x, 2, memory_order_relaxed);

	atomic_thread_fence(memory_order_seq_cst);

	int c = atomic_load_explicit(&z, memory_order_relaxed);
	__VERIFIER_assume(c == 0);
	return NULL;
}

void *thread_3(void *unused)
{
	y = 1;

	int b = x;
	__VERIFIER_assume(b == 2);
	return NULL;
}

void *thread_4(void *unused)
{
	atomic_store_explicit(&z, 1, memory_order_relaxed);

	atomic_thread_fence(memory_order_seq_cst);

	int d = atomic_load_explicit(&x, memory_order_relaxed);
	__VERIFIER_assume(d == 1);
	return NULL;
}
