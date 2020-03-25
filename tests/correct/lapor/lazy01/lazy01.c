#define __VERIFIER_error() assert(0)

pthread_mutex_t mutex;
int data = 0;

void *thread1(void *arg)
{
	pthread_mutex_lock(&mutex);
	data++;
	pthread_mutex_unlock(&mutex);
	return NULL;
}


void *thread2(void *arg)
{
	pthread_mutex_lock(&mutex);
	data += 2;
	pthread_mutex_unlock(&mutex);
	return NULL;
}


void *thread3(void *arg)
{
	pthread_mutex_lock(&mutex);
	if (data >= 3) {
	/* ERROR: __VERIFIER_error(); */
		;
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}
