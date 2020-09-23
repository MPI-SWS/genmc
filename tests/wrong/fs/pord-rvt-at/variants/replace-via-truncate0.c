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

	int fdf = open("foo", O_RDONLY, 0666);
	int fdb = open("bar", O_RDONLY, 0666);

	int nrf = read(fdf, buf, 1);
	int nrb = read(fdb, buf, 1);

	/* Is is possible for foo to be empty and for bar not to be? */
	assert(!(nrb > 0 && nrf <= 0));
	return;
}

int main()
{
	char buf[8];
	buf[0] = 42;

	int fd = creat("foo", 0666);
	write(fd, buf, 1);
	truncate("foo", 0); // should set flag
	write(fd, buf, 1);
	close(fd);

	int fdb = open("bar", O_CREAT|O_RDWR, 0666);
	write(fdb, buf, 1);
	close(fdb);

	return 0;
}
