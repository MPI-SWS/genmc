/* Bisimilarity fails due to the optional block in the loop */

atomic_int x;

int num_slept;

void sleep()
{
	++num_slept;
}

void *thread_1(void *unused)
{
	int r = x;
	while (!r) {
		__VERIFIER_optional(sleep()); /* side-effects */
		r = x;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	x = 42;
	return NULL;
}
