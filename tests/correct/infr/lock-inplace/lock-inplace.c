atomic_int x;
atomic_int y;
pthread_mutex_t l;

void *thread_1(void *unused)
{
	int r = x;
	pthread_mutex_lock(&l);
	/* pthread_mutex_unlock(&l); */
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&l);
	y = 42;
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_3(void *unused)
{
	__VERIFIER_assume(y == 42);
	x = 42;
	return NULL;
}
