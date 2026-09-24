#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#ifndef N
#define N 5
#endif

atomic_int x;
atomic_int y;
int a;

void *thread_0(void *unused)
{
    atomic_store_explicit(&y, N-1, memory_order_relaxed);
    int old_x = atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
    if (old_x == 0) {
        a = 1;
    }
    return NULL;
}

void *thread_i(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	int old_y = atomic_fetch_add_explicit(&y, 1, memory_order_relaxed);
	if (old_y == N-2) {
        a = 2;
    }
	return NULL;
}

int main()
{
	pthread_t t[N];

    pthread_create(&t[0], NULL, thread_0, NULL);
    for (int i = 1; i < N; i++) {
        pthread_create(&t[i], NULL, thread_i, NULL);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(t[i], NULL);
    }

	return 0;
}
