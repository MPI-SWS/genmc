/* Testcase from Threader's distribution. For details see:
   http://www.model.in.tum.de/~popeea/research/threader
*/

#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#include "../scull.c"

int main() {
	pthread_t t1, t2, t3;

	if (pthread_mutex_init(&lock, FILE_WITH_LOCK_UNLOCKED))
		abort();

	if (pthread_create(&t1, NULL, loader, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread1, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread2, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();
	if (pthread_join(t3, NULL))
		abort();

	/* pthread_mutex_destroy(&lock); */

  return 0;
}
