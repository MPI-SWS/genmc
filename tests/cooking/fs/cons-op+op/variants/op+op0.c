#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <genmc.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "../op+op.c"

int main()
{
	pthread_t t1, t2;

	/* Create the file */
	int fd = creat("foo", S_IRWXU);
	assert(fd != -1);

	__VERIFIER_pbarrier();

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
