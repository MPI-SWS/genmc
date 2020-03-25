pthread_mutex_t lx;
pthread_mutex_t ly;

int x;
int y;

void *thread_n(void *unused)
{
	pthread_mutex_lock(&lx);
	x = x + 1;
	pthread_mutex_unlock(&lx);

	pthread_mutex_lock(&ly);
	y = y + 1;
	pthread_mutex_unlock(&ly);
	return NULL;
}
