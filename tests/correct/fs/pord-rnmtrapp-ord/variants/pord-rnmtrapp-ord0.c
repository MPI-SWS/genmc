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
	int ff = open("foo", O_RDONLY, 0640);
	int fb = open("bar", O_RDONLY, 0640);

	int sf = lseek(ff, SEEK_END, 0);

	/* Is it possible to see the new foo but not bar? */
	assert(!(ff >= 0 && fb == -1 && sf < 2));
	return;
}

int main()
{
	char buf[2];

	buf[0] = buf[1] = 42;

	int fd = open("foo", O_CREAT|O_TRUNC|O_WRONLY, 0640);
	write(fd, buf, 2);
	close(fd);

	__VERIFIER_pbarrier();

	rename("foo", "bar");

	fd = creat("foo", 0640);
	write(fd, buf, 1);
	close(fd);

	return 0;
}
