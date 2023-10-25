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
	char buf1[8], buf2[8];

	buf1[0] = buf1[1] = 0;
	buf2[0] = buf2[1] = 0;

	int fd = open("foo", O_RDONLY, 0666);
	assert(fd != -1);

	int nr1 = pread(fd, buf1, 2, 2);
	int nr2 = pread(fd, buf2, 2, 4);

	/* Is is possible to see the 2nd append but not the 1st?
	 * ...The answer is 42 (executions) ;-)  */
	assert(!(nr1 == 2 && nr2 == 2 &&
				     buf1[0] != 2 && buf1[1] != 2 &&
				     buf2[0] == 3 && buf2[1] == 3));
	return;
}

int main()
{
	char buf[8];

	buf[0] = 1;
	buf[1] = 1;
	buf[2] = 2;
	buf[3] = 2;
	buf[4] = 3;
	buf[5] = 3;

	int fd = open("foo", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
	write(fd, buf, 2);

	__VERIFIER_pbarrier();

	write(fd, buf + 2, 2);
	write(fd, buf + 4, 2);
	close(fd);

	return 0;
}
