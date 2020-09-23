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

void __VERIFIER_recovery_routine(void)
{
	/* printf("Nothing to do\n"); */
	return;
}

int main()
{
	char buf[8];
	buf[0] = 42;

	int fd = open("foo", O_CREAT|O_RDONLY, S_IRWXU);
	assert(fd != -1);
	close(fd);

	int ret = truncate("foo", 4242);
	assert(ret >= 0);

	return 0;
}
