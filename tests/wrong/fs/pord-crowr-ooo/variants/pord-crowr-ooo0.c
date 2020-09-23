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
	char buf[2];
	int fd = open("foo", O_RDONLY, S_IRWXU);
	if (fd == -1)
		return;

	int fb = open("bar", O_RDONLY, 0640);
	int nr = read(fd, buf, 2);

	/* Is it possible to see the overwrite but not "bar"? */
	assert(!(nr == 2 && fb == -1 &&
				     buf[1] == '1' && buf[0] == '1'));
	return;
}

int main()
{
	char buf_init[2] = "00";
	char buf[2] = "11";

	int fd = creat("foo", S_IRWXU);
	write(fd, buf_init, 2);
	__VERIFIER_pbarrier();

	int fd2 = creat("bar", 0640);
	/* We won't do any operations on bar */

	pwrite(fd, buf, 2, 0);
	close(fd);
	close(fd2);

	return 0;
}
