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

	int fd = open("foo", O_RDONLY, 0666);
	assert(fd != -1);

	int nr = pread(fd, buf, 4, 4);
	if (nr == 0)
		return;

	/* Are (aligned) appends atomic? ? */
	assert(nr == 4);
	for (int i = 0u; i < 4; i++)
		assert(buf[i] == '2');
	return;
}

int main()
{
	char buf[8] = "11112222";

	int fd = open("foo", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
	write(fd, buf, 4);

	__VERIFIER_pbarrier();

	write(fd, buf + 4, 4);
	close(fd);

	return 0;
}
