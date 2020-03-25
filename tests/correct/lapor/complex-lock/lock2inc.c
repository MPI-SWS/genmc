pthread_mutex_t mutex;
pthread_mutex_t l;

int x;
int y;

void *thread_n(void *unused)
{
	int r1, r2;

	pthread_mutex_lock(&mutex);
	++x;
	/* ++y; */
	pthread_mutex_unlock(&mutex);

	/* pthread_mutex_lock(&l); */
	/* pthread_mutex_unlock(&l); */

	pthread_mutex_lock(&l);
	pthread_mutex_unlock(&l);

	pthread_mutex_lock(&mutex);
	++x;

	/* pthread_mutex_lock(&l); */
	/* pthread_mutex_unlock(&l); */

	/* ++y; */
	pthread_mutex_unlock(&mutex);
	return NULL;
}
