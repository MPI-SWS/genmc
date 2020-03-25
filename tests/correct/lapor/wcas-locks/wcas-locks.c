pthread_mutex_t l;
int x;

void *thread_0(void *unused)
{
	pthread_mutex_lock(&l);
	x = 2;
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_n(void *unused)
{
	pthread_mutex_lock(&l);
	if (x == 2)
		x = 3;
	pthread_mutex_unlock(&l);
	return NULL;
}
