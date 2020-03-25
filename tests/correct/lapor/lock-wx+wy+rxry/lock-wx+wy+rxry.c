pthread_mutex_t l;

int x, y;

void *thread1(void *unused)
{
	pthread_mutex_lock(&l);
	x = 42;
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread2(void *unused)
{
	pthread_mutex_lock(&l);
	y = 42;
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread3(void *unused)
{
	pthread_mutex_lock(&l);
	if (x == 42 && y == 42)
		; /* printf("hooray\n"); */
	pthread_mutex_unlock(&l);
	return NULL;
}
