/*
 * This test case exposes a bug in the implementation where
 * a PSC edge from Wx1 to Ry0 was added in the following program,
 * despite the fact that this should not be the case:
 *
 *    Wx1 (SC) || Rx1 (ACQ)
 *    Wy1 (SC) || Ry0 (SC)
 *
 * An sb_(<>loc);hb;sb_(<>loc) edge was erroneously added because
 * it was checked whether Wx1 is hb-before Rx1, instead of
 * checking whether Wy1 is hb-before Rx1.
 *
 * (Print PSC to see the erroneously added edge.)
 */

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	x = 1;
	y = 1;
	return NULL;
}

void *thread_2(void *unused)
{
	int a = atomic_load_explicit(&x, memory_order_acquire);
	__VERIFIER_assume(a == 1);

	int b = y;
	__VERIFIER_assume(b == 0);
	return NULL;
}
