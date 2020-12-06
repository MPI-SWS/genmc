#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

/* Simple test case demonstrating the effects of fsync() across files.
 *
 * If the contents of "bar" have persisted, so will have the contents of "foo",
 * due to the fsync() in thread_2.
 */

#define FILESIZE 4

void *thread_1(void *unused)
{
	int fd = creat("foo", S_IRWXU);
	assert(fd != -1);

	char buf[FILESIZE] = { [0 ... FILESIZE - 1] = 42 };

	int nw = write(fd, buf, FILESIZE);
	assert(nw == FILESIZE);
	return NULL;
}

void *thread_2(void *unused)
{
	/* Wait until foo is created... */
	int ff = open("foo", O_RDONLY, S_IRWXU);
	__VERIFIER_assume(ff != -1);

	/* Wait until foo is fully populated... */
	char buf[FILESIZE];
	int nr = read(ff, buf, FILESIZE);
	__VERIFIER_assume(nr == FILESIZE);

	/* Make sure foo has reached disk... */
	fsync(ff);

	/* Mofidy contents read and write them to bar */
	for (int i = 0u; i < nr; i++)
		++buf[i];

	int fb = creat("bar", S_IRWXU);
	int nw = write(fb, buf, FILESIZE);
	assert(nw == FILESIZE);
	return NULL;
}

void __VERIFIER_recovery_routine(void)
{
	/* Is it possible to read the contents of bar without seeing foo as well? */
	int ff = open("foo", O_RDONLY, S_IRWXU);
	int fb = open("bar", O_RDONLY, S_IRWXU);

	if (fb == -1)
		return;

	char buff[FILESIZE], bufb[FILESIZE];

	int nrb = pread(fb, bufb, FILESIZE, 0);
	if (nrb < FILESIZE)
		return;

	int nrf = read(ff, buff, FILESIZE);
	assert(nrf == FILESIZE && nrb == FILESIZE);

	for (int i = 0u; i < FILESIZE; i++)
		assert(buff[i] == 42);
	return;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
