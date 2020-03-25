#define __VERIFIER_error() assert(0)

pthread_mutex_t ma, mb;
int data1, data2;

void *thread1(void *arg)
{
	pthread_mutex_lock(&ma);
	data1++;
	pthread_mutex_unlock(&ma);

	pthread_mutex_lock(&ma);
	data2++;
	pthread_mutex_unlock(&ma);

	return NULL;
}


void *thread2(void *arg)
{
	pthread_mutex_lock(&ma);
	data1 += 5;
	pthread_mutex_unlock(&ma);

	pthread_mutex_lock(&ma);
	data2 -= 6;
	pthread_mutex_unlock(&ma);

	return NULL;
}
