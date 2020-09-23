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
	char buf[8];
	int fd = open("foo", O_RDONLY, S_IRWXU);

	if (fd == -1)
		return;

	int nr = read(fd, buf, 1);

	assert(nr == 1);
	return;
}

int main()
{
	int fd = creat("foo", S_IRWXU);
	return 0;
}
