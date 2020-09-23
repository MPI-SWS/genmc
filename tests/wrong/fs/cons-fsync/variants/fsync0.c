#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <genmc.h>
#include <assert.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

atomic_int x;
atomic_int y;

void __VERIFIER_recovery_routine(void)
{
	/* printf("Nothing to do\n"); */
	return;
}

int main()
{
	int ret = fsync(42);
	assert(ret != 1);

	return 0;
}
