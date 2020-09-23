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
	/* Create the file */
	int fd = open("foo", O_CREAT|42, S_IRWXU);
	assert(fd != -1);

	return 0;
}
