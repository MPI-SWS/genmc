#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>

#include "../dsa-live.c"

int main()
{
	pthread_t t1;

	pthread_create(&t1, 0, run_t1, 0);
	run_t0(NULL);
	pthread_join(t1, NULL);
	return 0;
}
