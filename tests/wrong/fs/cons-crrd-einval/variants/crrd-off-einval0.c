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

int main()
{
	char buf[8];
	buf[0] = 42;

	int fd = open("foo", O_CREAT|O_RDONLY, S_IRWXU);
	assert(fd != -1);

	int nr = pread(fd, buf, 1, -42);
	assert(nr >= 0);

	return 0;
}
