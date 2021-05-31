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
	__VERIFIER_assume(x == 42 && y == 42);
	return NULL;
}
