#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../linuxrwlocks.c"

int main()
{
	pthread_t W[MAXWRITERS], R[MAXREADERS], RW[MAXRDWR];

	atomic_init(&mylock.lock, RW_LOCK_BIAS);
	for (int i = 0; i < writers; i++)
		pthread_create(&W[i], NULL, threadW, NULL);
	for (int i = 0; i < readers; i++)
		pthread_create(&R[i], NULL, threadR, NULL);
	for (int i = 0; i < rdwr; i++)
		pthread_create(&RW[i], NULL, threadRW, NULL);

	/* for (int i = 0; i < writers; i++) */
	/* 	pthread_join(W[i], NULL); */
	/* for (int i = 0; i < readers; i++) */
	/* 	pthread_join(R[i], NULL); */
	/* for (int i = 0; i < rdwr; i++) */
	/* 	pthread_join(RW[i], NULL); */

	return 0;
}
