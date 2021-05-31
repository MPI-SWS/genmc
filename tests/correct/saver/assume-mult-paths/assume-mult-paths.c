atomic_int x;

void *thread_1(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		x = i;
	return NULL;
}

void *thread_2(void *unused)
{
	int r;

	int a = x;
	if (a) {
		r = a - 42;
	} else {
		r = a - 4242;
	}
	__VERIFIER_assume(r == 0);
	return NULL;
}
