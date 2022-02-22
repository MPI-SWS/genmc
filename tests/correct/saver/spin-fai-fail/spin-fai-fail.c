atomic_int x;

void *thread_1(void *unused)
{
	for (;;) {
		int a = atomic_fetch_add(&x, 1);
		if (a == 0)
			break;
		atomic_fetch_add(&x, -1);
	}
	return NULL;
}

void *thread_2(void *unused)
{
	for (;;) {
		int a = atomic_fetch_add(&x, 1);
		if (a == 0)
			break;
		atomic_fetch_add(&x, -1);
	}
	return NULL;
}

void *thread_3(void *unused)
{
	int a = x;
	int b = x;
	return NULL;
}
