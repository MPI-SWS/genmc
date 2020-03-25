pthread_mutex_t l;

int x;
int y;

void *thread1(void *unused)
{
	pthread_mutex_lock(&l);
	x = 42;
	y = 42;
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread2(void *unused)
{
	pthread_mutex_lock(&l);
	int a = x;
	int b = y;
	pthread_mutex_unlock(&l);
	return NULL;
}
