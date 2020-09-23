_Atomic(int *) p;

void *thread1(void *unused)
{
	int x = 0;
	p = &x;
	return NULL;
}

void *thread2(void *unused)
{
	if (p != NULL)
		*p = 42;
	return NULL;
}
