#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <lkmm.h>

int x;
int r0;

void *thread_1(void *unused)
{
	WRITE_ONCE(x, 1);
	r0 = READ_ONCE(x);
	return NULL;
}

void *thread_2(void *unused)
{
	WRITE_ONCE(x, 2);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	assert(!(READ_ONCE(x) == 1 && r0 == 2));

	return 0;
}
