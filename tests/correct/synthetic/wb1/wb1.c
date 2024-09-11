atomic_int x;

void *thread1(void *unused)
{
	x = 1;
	int a = atomic_fetch_add(&x, 1);
	__VERIFIER_assume(a == 42);
	return NULL;
}

void *thread2(void *unused)
{
	int b = atomic_fetch_add(&x, 42);
	__VERIFIER_assume(b == 0);

	/* If all assume()s hold, this will have nowhere to read-from */
	int r = x;
	return NULL;
}
