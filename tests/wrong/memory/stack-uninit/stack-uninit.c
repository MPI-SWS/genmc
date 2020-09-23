_Atomic(int *) p;
atomic_bool done;

void *thread_1(void *unused)
{
	int x;

	p = &x;
	while (!done)
		;
	return NULL;
}

void *thread_2(void *unused)
{
	while (p == NULL)
		;

	int r = *p;
	done = 1;
	return NULL;
}
