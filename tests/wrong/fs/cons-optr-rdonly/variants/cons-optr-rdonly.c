#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <genmc.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

int main()
{
	/* Create the file */
	int fd = creat("foo", S_IRWXU);
	close(fd);

	/* Open the file with O_RDONLY|O_TRUNC */
	fd = open("foo", O_RDONLY|O_TRUNC, 0640);
	assert(fd != -1);
	close(fd);

	return 0;
}
