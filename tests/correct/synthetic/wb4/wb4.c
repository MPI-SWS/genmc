#define mo_rlx memory_order_relaxed

atomic_int x;

void __VERIFIER_assume(int);

void *thread1(void *arg)
{

	int a = atomic_fetch_add_explicit(&x, 1, mo_rlx);
	__VERIFIER_assume(a == 1);
	return NULL;
}

void *thread2(void *arg)
{
	atomic_store_explicit(&x, 1, mo_rlx);
	int b = atomic_fetch_add_explicit(&x, 1, mo_rlx);
	__VERIFIER_assume(b == 10);
	atomic_load_explicit(&x, mo_rlx);
	return NULL;
}

void *thread3(void *arg)
{
	atomic_store_explicit(&x, 10, mo_rlx);
	return NULL;
}
