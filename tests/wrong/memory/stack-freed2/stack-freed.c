_Atomic(int *) p;

void exit_thread(int *loc)
{
	p = loc;
	pthread_exit(NULL);
}

void *thread1(void *unused)
{
	int x = 0;
	exit_thread(&x);
	return NULL;
}

void *thread2(void *unused)
{
	if (p != NULL)
		*p = 42;
	return NULL;
}
