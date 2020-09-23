#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
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
	assert(fd != -1);

	int nr = pread(fd, buf, 2, 2);
	if (nr < 2)
		return;

	/* Is is possible to read other than {1,2} ? */
	assert(!(buf[0] == 1 && buf[1] == 0));
	return;
}

int main()
{
	char buf[8];

	buf[0] = 1;
	buf[1] = 1;
	buf[2] = 1;
	buf[3] = 2;
	buf[4] = 2;

	int fd = open("foo", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
	write(fd, buf, 3);

	__VERIFIER_pbarrier();

	write(fd, buf + 3, 2);

	close(fd);

	return 0;
}
