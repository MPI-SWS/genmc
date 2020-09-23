#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

/* Make sure that CASes on unallocated memory doesn't confuse the interpreter */

int main()
{
	pthread_mutex_lock((pthread_mutex_t *) 0xdeadbeef);
	return 0;
}
