#define mo_rlx memory_order_relaxed

atomic_int x;

int a, b, c, d;

void *thread_1(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, mo_rlx);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, mo_rlx);
	return NULL;
}

void *thread_3(void *unused)
{
	a = atomic_load_explicit(&x, mo_rlx);
	b = atomic_load_explicit(&x, mo_rlx);
	return NULL;
}

void *thread_4(void *unused)
{
	atomic_store_explicit(&x, 42, mo_rlx);
	return NULL;
}

void *thread_5(void *unused)
{
	c = atomic_load_explicit(&x, mo_rlx);
	d = atomic_load_explicit(&x, mo_rlx);
	return NULL;
}
