atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	for (int i = 1u; i <= 42; i++)
		x = i;
	return NULL;
}

void *thread_2(void *unused)
{
	int r, b;

	r = atomic_load_explicit(&x, memory_order_seq_cst);
	for (;;) {
		if (r == 42) {
			if (atomic_compare_exchange_strong(&x, &r, 42))
				break;
		} else {
			if (atomic_compare_exchange_strong(&x, &r, 17))
				break;
		}
	}
	return NULL;
}
