#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

#include <fcntl.h>
#include <sys/stat.h>

#define __stringify_1(x) #x
#define __stringify(x) __stringify_1(x)

#define WRITE_FILE(i)							\
do {								        \
	fd[i] = open("foo" #i, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);	\
	pwrite(fd[i], buf, 4, 0);					\
	close(fd[i]);							\
} while (0)

void __VERIFIER_recovery_routine(void)
{
	char buf[4];

	int fd = open("foo1", O_RDONLY, 0666);

	/* Just check if the first one persisted... */
	int sz = lseek(fd, SEEK_END, 0);
	if (fd > 0)
		assert(sz >= 0);
	return;
}

int main()
{


	int fd[4];
	char buf[4];

	buf[0] = 1;
	buf[1] = 1;
	buf[2] = 1;
	buf[3] = 1;

	WRITE_FILE(0);
	WRITE_FILE(1);
	WRITE_FILE(2);
	WRITE_FILE(3);

	return 0;
}
