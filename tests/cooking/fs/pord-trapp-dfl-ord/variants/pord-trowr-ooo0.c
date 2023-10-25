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
	char buf_f[2], buf_b[2];

	int ff = open("foo", O_RDONLY, S_IRWXU);
	int fb = open("bar", O_RDONLY, S_IRWXU);
	if (ff == -1 || fb == -1)
		return;

	int nb = read(fb, buf_b, 2);
	int nf = read(ff, buf_f, 2);

	/* Is it possible to see the app in "bar" but not the tr in "foo"? */
	assert(!(nb == 2 && nf == 2));
	return;
}

int main()
{
	char buf[2] = "00";

	int ff = creat("foo", S_IRWXU);
	int fb = creat("bar", S_IRWXU);

	pwrite(ff, buf, 2, 0);

	__VERIFIER_pbarrier();

	truncate("foo", 0);
	write(fb, buf, 2);

	close(ff);
	close(fb);

	return 0;
}
