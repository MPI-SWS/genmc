#ifndef N
# define N 2
#endif

pthread_mutex_t l;
int x;

int idx[N];

void *threadn(void *arg)
{
	int i = *((int *) arg);

	pthread_mutex_lock(&l);
	int r = x;
	x = i + 1;
	pthread_mutex_unlock(&l);
	return NULL;
}
