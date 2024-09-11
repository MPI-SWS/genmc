atomic_int a;
atomic_int b;
atomic_int c;
atomic_int d;

void *thread_1(void *unused)
{
	c = 2;
	a = 2;
	__VERIFIER_assume(b == 2);
	return NULL;
}

void *thread_2(void *unused)
{
	c = 1;
	__VERIFIER_assume(a == 2);
	__VERIFIER_assume(b == 1);
	return NULL;
}

void *thread_3(void *unused)
{
	b = 2;
	a = 1;
	__VERIFIER_assume(c == 2);;
	return NULL;
}

void *thread_4(void *unused)
{
	b = 1;
	__VERIFIER_assume(a == 1);
	__VERIFIER_assume(c == 1);
	return NULL;
}
