int na = 0;
atomic_int x;
atomic_int y;

void *thread_n(void *unused)
{
        int r = 1;
        __VERIFIER_assume(atomic_compare_exchange_strong(&x, &r, 1));
	return NULL;
}

void *thread_fai(void *unused)
{
        atomic_fetch_add(&x, 1);
	return NULL;
}
