#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>

#include "../peterson.c"

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

#ifdef NIDHUGG_PRINT_COMPLETE_EXECS
	printf("Full execution encountered\n");
#endif
	return 0;
}
