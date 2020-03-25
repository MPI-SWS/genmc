#ifndef N
# define N 2
#endif

pthread_mutex_t l;
int x;

void *threadn(void *unused)
{
	pthread_mutex_lock(&l);
	int r = x;
	pthread_mutex_unlock(&l);
	return NULL;
}
