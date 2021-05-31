atomic_int x;

void *thread_1(void *unused)
{
	for (;;) {
		int a, b = 0;

		a = atomic_load_explicit(&x, memory_order_relaxed);
		if (a == 41)
			b = a;
		b++;
		if (b == 42)
			break;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 0u; i < 42; i++)
		x = i;
	return NULL;
}
