#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

#include <fcntl.h>
#include <sys/stat.h>

void __VERIFIER_recovery_routine(void)
{
	char buf[8];

	int fd = open("foo", O_RDONLY, 0666);
	assert(fd != -1);

	int nr = pread(fd, buf, 5, 0);
	if (nr < 5)
		return;

	/* Is is possible to read new data for the 2nd block w/out the 1st? */
	assert(!(buf[3] == 2 && buf[4] == 2 &&
				     buf[0] == 1 && buf[1] == 1));
	return;
}

int main()
{
	char buf[8];

	buf[0] = 1;
	buf[1] = 1;
	buf[2] = 2;
	buf[3] = 2;

	int fd = open("foo", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
	pwrite(fd, buf, 2, 0);

	__VERIFIER_pbarrier();

	pwrite(fd, buf + 2, 2, 0); /* 1) overwrite */
	pwrite(fd, buf + 2, 2, 3); /* 2) append    */

	close(fd);

	return 0;
}
