#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <genmc.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

void __VERIFIER_recovery_routine(void)
{
	/* printf("Nothing to do\n"); */
	return;
}

int main()
{
	int fd = creat("foo", S_IRWXU);
	assert(fd != -1);

	int ret = close(fd);
	assert(ret != -1);

	return 0;
}
