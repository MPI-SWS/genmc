_Atomic(int *) p;

void *thread1(void *unused)
{
	p = malloc(sizeof(int));
	*p = 42;
	free(p);
	return NULL;
}

void *thread2(void *unused)
{
	p = malloc(sizeof(int));
	return NULL;
}

void *thread3(void *unused)
{
	int *r = p;
	if (r)
		*r = 18;
	return NULL;
}
