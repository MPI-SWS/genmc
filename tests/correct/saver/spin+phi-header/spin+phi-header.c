/*
 * A spinloop with a PHI node in its header for which we have
 * to perform jump threading in order to transform it.
 */

atomic_int x;

void *thread_1(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		atomic_store_explicit(&x, i, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int success = 0;

	while (!success)
		success = x;
	return NULL;
}
