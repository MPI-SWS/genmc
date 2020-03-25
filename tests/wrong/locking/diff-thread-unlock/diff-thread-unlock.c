#define N 2

static pthread_mutex_t lock;

static atomic_int x = 0;
static int readX = 0;

static void *runLock(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	pthread_mutex_lock(&lock);
	return NULL;
}

static void *runUnlock(void *arg)
{
	readX = atomic_load_explicit(&x, memory_order_relaxed);
	if (readX == 0)
		pthread_mutex_unlock(&lock);
	return NULL;
}
