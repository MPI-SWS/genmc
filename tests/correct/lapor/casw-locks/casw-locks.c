pthread_mutex_t l;

int x;
int idx[N];

void *thread_n(void *arg)
{
	int r = 0, val = *((int *) arg);

	pthread_mutex_lock(&l);

	if (x == r)
		x = 1;

	pthread_mutex_unlock(&l);

	pthread_mutex_lock(&l);
	x = val + 3;
	pthread_mutex_unlock(&l);
	return NULL;
}
