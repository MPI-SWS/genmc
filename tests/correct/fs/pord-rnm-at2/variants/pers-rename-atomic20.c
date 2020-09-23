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
	int fq = open("qux", O_RDONLY, 0640);
	int fb = open("bar", O_RDONLY, 0640);

	/* Rename should be atomic -- <= 1 name should exist */
	assert((ff >= 0 && fb < 0  && fq < 0) ||
				   (ff < 0  && fb >= 0 && fq < 0) ||
				   (ff < 0  && fb < 0  && fq >= 0));
	return;
}

int main()
{
	char buf[3] = "111";

	int fd = open("foo", O_CREAT|O_TRUNC|O_WRONLY, 0640);
	write(fd, buf, 3);
	close(fd);

	__VERIFIER_pbarrier();

	rename("foo", "bar");
	rename("bar", "qux");

	return 0;
}
