/*
 * spin-cas2: Another CAS spinloop captured by the transformation.
 *            The ordering of the load and the CAS need to match.
 */

atomic_int x;

void *thread_1(void *unused) /* spawned 3 times */
{
	int r = x;
	while (!atomic_compare_exchange_strong(&x, &r, 42))
		;
	return NULL;
}

void *thread_2(void *unused)
{
	x = 1;
	return NULL;
}
