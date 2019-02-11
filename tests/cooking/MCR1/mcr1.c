atomic_int x;

void *thread_1(void *unused)
{
	x = 1;
	return NULL;
}

void *thread_2(void *unused)
{
	x = 1;
	int r = x;
	return NULL;
}
