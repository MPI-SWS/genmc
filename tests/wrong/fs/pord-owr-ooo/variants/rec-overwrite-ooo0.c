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

	buf[0] = 0;
	buf[1] = 0;

	int fd = open("foo", O_RDONLY, 0666);

	pread(fd, &buf[0], 1, 0);
	pread(fd, &buf[1], 1, 3);

	/* Is is possible for the writes to persist out of order? */
	assert(!(buf[0] == 0 && buf[1] == 42));
	return;
}

int main()
{
	char buf_a[8], buf_b[8];

	/* Initialize bufs */
	buf_a[0] = buf_a[1] = buf_a[2] = buf_a[3] = 0;
	buf_b[0] = 42;

	/* Initialize foo */
	int fd = creat("foo", 0666);
	write(fd, buf_a, 4);

	__VERIFIER_pbarrier();

	/* Write to different offsets */
	pwrite(fd, buf_b, 1, 0);
	pwrite(fd, buf_b, 1, 3);

	close(fd);

	return 0;
}
