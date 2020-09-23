#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main()
{
	int fd = open("test.txt", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
	if (fd == -1)
		return -1;

	close(fd);

	int ret = truncate("test.txt", 2);
	assert(ret != -1);

	return 0;
}
