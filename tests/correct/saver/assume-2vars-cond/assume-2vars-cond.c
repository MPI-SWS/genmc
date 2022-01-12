atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	for (int i = 1u; i <= 42; i++) {
		x = i;
		y = i;
	}
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

	int b = y;
	__VERIFIER_assume(b == 42 && r == 0);
	return NULL;
}
