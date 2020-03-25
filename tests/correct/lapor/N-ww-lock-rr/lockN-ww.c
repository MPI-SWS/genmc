#ifndef N
# define N 2
#endif

pthread_mutex_t l;
int x;
int y;

int idx[N];

void *threadn(void *arg)
{
	int i = *((int *) arg);

	pthread_mutex_lock(&l);
	x = i;
	y = i;
	pthread_mutex_unlock(&l);
	return NULL;
}
