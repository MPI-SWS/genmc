/*
 * spin-cas6: Multiple exiting blocks for the spinloop, captured by spin-assume.
 */

atomic_int x;

void *thread_1()
{
	int r = 1;
	while (!atomic_compare_exchange_strong(&x, &r, 42)) {
		if (x == 42)
			break;
		r = 1;
	}
	return NULL;
}

void *thread_2()
{
	x = 42;
	return NULL;
}
