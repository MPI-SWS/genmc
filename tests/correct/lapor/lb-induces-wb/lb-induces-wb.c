void __VERIFIER_assume(int);

atomic_int x;
atomic_int y;
atomic_int z;

pthread_mutex_t l;

void *thread_1(void *unused)
{
	pthread_mutex_lock(&l);
	y = 1;
	pthread_mutex_unlock(&l);

	__VERIFIER_assume(x == 2);
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&l);
	x = 1;
	__VERIFIER_assume(y == 0);
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_3(void *unused)
{
	pthread_mutex_lock(&l);
	x = 2;
	__VERIFIER_assume(z == 0);
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_4(void *unused)
{
	pthread_mutex_lock(&l);
	z = 1;
	pthread_mutex_unlock(&l);

	__VERIFIER_assume(x == 1);
	return NULL;
}
