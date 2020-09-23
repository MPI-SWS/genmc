#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>

atomic_int x;

atomic_bool p1;
atomic_bool p2;

void *thread_1(void *unused)
{
	p1 = false;
	while (true) {
		p1 = true;
		while (p2)
			;
		/* begin critical section */
		x = 1;
		/* end critical section */
		p1 = false;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	p2 = false;
	while (true) {
		p2 = true;
		while (p1) {
			p2 = false;
			while (p1)
				;
			p2 = true;
		}
		/* begin critical section */
		x = 2;
		/* end critical section */
		p2 = false;
	}
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
