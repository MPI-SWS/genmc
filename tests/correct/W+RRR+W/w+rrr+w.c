#define mo_rlx memory_order_relaxed

atomic_int x;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, mo_rlx);
	return NULL;
}

void *thread_2(void *unused)
{
	int a = atomic_load_explicit(&x, mo_rlx);
	int b = atomic_load_explicit(&x, mo_rlx);
	int c = atomic_load_explicit(&x, mo_rlx);
	assert(!(a == 1 && b == 2 && c == 1));
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store_explicit(&x, 2, mo_rlx);
	return NULL;
}
