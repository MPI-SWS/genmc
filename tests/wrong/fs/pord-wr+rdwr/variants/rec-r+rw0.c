#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <genmc.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

void *thread_1(void *fdx)
{
	char buf[8];
	int fd_x = *((int *) fdx);
	buf[0] = 42;

	pwrite(fd_x, buf, 1, 0);
	return NULL;
}

void *thread_2(void *fdx)
{
	char buf_x[8];
	char buf_y[8];
	int fd_x = *((int *) fdx);

	buf_x[0] = 0;
	buf_y[0] = 42;

	int fd_y = creat("y", S_IRWXU);

	pread(fd_x, buf_x, 1, 0);
	if (buf_x[0] == 42)
		pwrite(fd_y, buf_y, 1, 0);

	return NULL;
}

void __VERIFIER_recovery_routine(void)
{
	char buf_x[8];
	char buf_y[8];

	buf_x[0] = 0;
	buf_y[0] = 0;

	int fd_y = open("y", O_RDONLY, S_IRWXU);
	int fd_x = open("x", O_RDONLY, S_IRWXU);

	if (fd_x == -1 || fd_y == -1)
		return;

	pread(fd_y, buf_y, 1, 0);
	pread(fd_x, buf_x, 1, 0);

	assert(!(buf_y[0] == 42 && buf_x[0] == 0));
	return;
}

int main()
{
	pthread_t t1, t2;

	int fd_x = open("x", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);

	__VERIFIER_pbarrier();

	if (pthread_create(&t1, NULL, thread_1, &fd_x))
		abort();
	if (pthread_create(&t2, NULL, thread_2, &fd_x))
		abort();
	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	return 0;
}
