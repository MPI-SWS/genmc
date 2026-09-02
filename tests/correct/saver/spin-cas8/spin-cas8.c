/*
 * spin-cas8: Different memory orders between the read before the CAS and the CAS.
 * Transformation should fail. As such, CAS needs to be strong.
 */

atomic_int x;

void *thread_1()
{
	int r = x;
	while (!atomic_compare_exchange_strong_explicit(&x, &r, 42, memory_order_acq_rel,
							memory_order_acq_rel))
		;
	return NULL;
}

void *thread_2()
{
	x = 1;
	return NULL;
}
