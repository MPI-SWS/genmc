#include <cstdlib>
#include <genmc.h>
#include <pthread.h>
#include <stdatomic.h>

/*
 * This test is for ensuring that we detect errors in exit handlers.
 * (Previous versions did not properly handle non-inlined function calls in exit handlers.)
 */

volatile int x;

void *thread_1(void *unused)
{
	x = 42;
	return NULL;
}

__attribute__((noinline)) void recursiveRace(unsigned int n)
{
	if (n == 0) {
		x = 1234;
		return;
	}
	return recursiveRace(n - 1);
}

int main()
{
	pthread_t t1;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		std::abort();

	std::atexit([](void) {
		recursiveRace(3);
		return;
	});

	return 0;
}
