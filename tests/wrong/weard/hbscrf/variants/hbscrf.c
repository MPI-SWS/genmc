#include <pthread.h>
#include <stdatomic.h>

atomic_int x;
atomic_int y;
int a;

void *thread_1(void *unused)
{
	int r_y = atomic_load_explicit(&y, memory_order_relaxed);
	int r_x = atomic_load_explicit(&x, memory_order_seq_cst);
	if (r_x == 0 && r_y == 1) {
		a = 2;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	a = 1;
	atomic_store_explicit(&x, 1, memory_order_seq_cst);
	return NULL;
}

void *thread_3(void *unused)
{
	int r_x = atomic_load_explicit(&x, memory_order_acquire);
	if (r_x == 1) {
		atomic_thread_fence(memory_order_seq_cst);
		atomic_store_explicit(&y, 1, memory_order_relaxed);
	}
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3;

	pthread_create(&t1, NULL, thread_1, NULL);
	pthread_create(&t2, NULL, thread_2, NULL);
	pthread_create(&t3, NULL, thread_3, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	return 0;
}
