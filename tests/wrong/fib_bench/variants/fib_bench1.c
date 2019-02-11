#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../fib_bench.c"

int main()
{
	pthread_t id1, id2, id3;

	pthread_create(&id2, NULL, t2, NULL);
	pthread_create(&id1, NULL, t1, NULL);
	pthread_create(&id3, NULL, t3, NULL);

	return 0;
}
