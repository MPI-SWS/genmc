atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	int r = y;
	return NULL;
}

void *thread_2(void *unused)
{
        int r = 0;
        __VERIFIER_assume(atomic_compare_exchange_strong(&x, &r, 1));
	return NULL;
}

void *thread_3(void *unused)
{
        int r = 0;
        if (!atomic_compare_exchange_strong(&x, &r, 1))
                return NULL;
        atomic_fetch_add(&x, 1);
        atomic_fetch_add(&x, 1);
	return NULL;
}

void *thread_4(void *unused)
{
        int r = 2;
        if (!atomic_compare_exchange_strong(&x, &r, 0))
                return NULL;
	y = 1;
	return NULL;
}
