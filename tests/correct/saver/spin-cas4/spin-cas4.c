/*
 * spin-cas4: CAS spinloop with a join point; captured by spin-assume
 */

atomic_int x, y;

void *thread_1()
{
	int r = x;
	while (!atomic_compare_exchange_strong(&x, &r, 42)) {
		if (y == 0)
			r = x;
	}
	return NULL;
}

void *thread_2()
{
	x = 1;
	y = 1;
	return NULL;
}
