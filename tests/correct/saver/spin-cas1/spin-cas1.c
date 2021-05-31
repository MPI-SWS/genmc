/*
 * spin-cas1: Simple CAS spinloop captured by the transformation.
 */

atomic_int x;

void *thread_1(void *unused)
{
	int r = 1;
	while (!atomic_compare_exchange_strong(&x, &r, 42))
		r = 1;
	return NULL;
}

void *thread_2(void *unused)
{
	x = 1;
	return NULL;
}

void *thread_3(void *unused)
{
	x = 2;
	return NULL;
}
