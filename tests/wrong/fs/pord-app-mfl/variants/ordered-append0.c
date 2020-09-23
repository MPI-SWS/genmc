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
	char buff[8],bufb[8];
	buff[0] = bufb[0] = 0;
	buff[1] = bufb[1] = 0;

	int fdf = open("foo", O_RDONLY, 0666);
	int fdb = open("bar", O_RDONLY, 0666);
	assert(fdf != -1 && fdb != -1);

	int nrf = pread(fdf, buff, 2, 2);
	int nrb = pread(fdb, bufb, 2, 2);

	/* Is is possible to see the append in BAR but not in FOO? */
	assert(!(nrb == 2 && nrf == 0));
	return;
}

int main()
{
	char buf[8];

	buf[0] = 1;
	buf[1] = 1;
	buf[2] = 2;
	buf[3] = 2;

	int fdf = open("foo", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
	int fdb = open("bar", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
	write(fdf, buf, 2);
	write(fdb, buf, 2);

	__VERIFIER_pbarrier();

	write(fdf, buf + 2, 2);
	write(fdb, buf + 2, 2);

	close(fdf);
	close(fdb);

	return 0;
}
