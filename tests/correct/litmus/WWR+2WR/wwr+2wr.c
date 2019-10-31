void __VERIFIER_assume(int);

atomic_int x;
atomic_int y;
atomic_int z;

void *thread1(void *unused)
{
	x = 1;
	y = 1;
	__VERIFIER_assume(z == 0);
	return NULL;
}

void *thread2(void *unused)
{
	z = 1;
	__VERIFIER_assume(y == 2);
	return NULL;
}

void *thread3(void *unused)
{
	y = 2;
	__VERIFIER_assume(x == 0);
	return NULL;
}
