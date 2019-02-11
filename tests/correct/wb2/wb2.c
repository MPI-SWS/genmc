#define mo_rlx memory_order_relaxed

atomic_int x;

void __VERIFIER_assume(int);

void *thread1(void *arg)
{
	int a = atomic_load_explicit(&x, mo_rlx);
	__VERIFIER_assume(a == 5);
	int b = atomic_load_explicit(&x, mo_rlx);
	return NULL;
}

void *thread2(void *arg)
{
	int c = atomic_load_explicit(&x, mo_rlx);
	__VERIFIER_assume(c == 4);
	atomic_store_explicit(&x, 5, mo_rlx);
	return NULL;
}

void *thread3(void *arg)
{
	atomic_store_explicit(&x, 3, mo_rlx);
	atomic_store_explicit(&x, 4, mo_rlx);
	return NULL;
}

void *thread4(void *arg)
{
	int d = atomic_fetch_add_explicit(&x, 10, mo_rlx);
	__VERIFIER_assume(d == 3);
	return NULL;
}
