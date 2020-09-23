#include <stdlib.h>
#include <threads.h>
#include <assert.h>

mtx_t mutex;
int x;

static void thread_1(void *unused)
{
	int r;

	mtx_lock(&mutex);
	r = x;
	x = r + 1;
	mtx_unlock(&mutex);
	return;
}

static void thread_2(void *unused)
{
	int r;

	mtx_lock(&mutex);
	r = x;
	x = r - 1;
	mtx_unlock(&mutex);
	return;
}

int main()
{
	thrd_t t1, t2;

	int res = thrd_create(&t1, (thrd_start_t)&thread_1, NULL);
	assert(res == thrd_success);
	res = thrd_create(&t1, (thrd_start_t)&thread_2, NULL);
	assert(res == thrd_success);

	return 0;
}
