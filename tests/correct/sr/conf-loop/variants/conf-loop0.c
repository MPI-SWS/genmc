#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_n(void *unused)
{
	int r;
	do {
		r = x;
	} while (!atomic_compare_exchange_strong(&x, &r, 42));
	return NULL;
}

void *thread_1(void *unused)
{
	while (true) {
		int r = x;
		if (atomic_compare_exchange_strong(&x, &r, 0))
			break;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	int r = 42;
	atomic_compare_exchange_strong(&x, &r, 0);
	return NULL;
}

int main()
{
	__VERIFIER_spawn(thread_1, NULL);
	__VERIFIER_spawn(thread_2, NULL);

	__VERIFIER_thread_t t1 = __VERIFIER_spawn(thread_n, NULL);
	__VERIFIER_thread_t t2 = __VERIFIER_spawn_symmetric(thread_n, NULL, t1);
	__VERIFIER_thread_t t3 = __VERIFIER_spawn_symmetric(thread_n, NULL, t2);

	__VERIFIER_spawn(thread_1, NULL);
	__VERIFIER_spawn(thread_2, NULL);

	return 0;
}
