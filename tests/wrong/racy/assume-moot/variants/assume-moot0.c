#include <assert.h>
#include <genmc.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x;
int a;

void *t0(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_release);
	return NULL;
}

void *t1(void *arg)
{
	int r_x = atomic_load_explicit(&x, memory_order_acquire);
	if (r_x == 0) {
		a = 1;
		__VERIFIER_assume(r_x == 1);
	}
	return NULL;
}

void *t2(void *arg)
{
	int r_x = atomic_load_explicit(&x, memory_order_relaxed);
	a = 2;
	return NULL;
}

int main()
{
	pthread_t t[3];
	pthread_create(&t[0], NULL, t0, NULL);
	pthread_create(&t[1], NULL, t1, NULL);
	pthread_create(&t[2], NULL, t2, NULL);
	pthread_join(t[0], NULL);
	pthread_join(t[1], NULL);
	pthread_join(t[2], NULL);

	return 0;
}