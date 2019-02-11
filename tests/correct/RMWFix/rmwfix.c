atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	if (y) {
		atomic_fetch_add(&x, 1);
		atomic_fetch_add(&x, 1);
	}
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_fetch_add(&x, 1);
	return NULL;
}

void *thread_3(void *unused)
{
	y = 42;
	return NULL;
}
