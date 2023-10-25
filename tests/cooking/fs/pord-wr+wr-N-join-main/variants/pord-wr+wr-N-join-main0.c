#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

/* Different threads concurrently writing to different files, which
 * are subsequently joined by the main() function.
 *
 * The contents of the files might persist in any order.
 */

#define FILESIZE 4
#define NUM_WRITERS 4
#define MAX_FILESIZE FILESIZE * NUM_WRITERS

#define DECLARE_WRITER(id)						\
	void *thread_##id(void *unused)					\
	{								\
		char buf[FILESIZE] = { [0 ... FILESIZE - 1] = 42};	\
		int fd = creat("file" #id, 0640);			\
		int nw = write(fd, buf, FILESIZE);			\
		assert(nw == FILESIZE);					\
		close(fd);						\
		return NULL;						\
	}

/* Ugly trickery to only use const static strings for filenames... */
DECLARE_WRITER(1);
DECLARE_WRITER(2);
DECLARE_WRITER(3);
DECLARE_WRITER(4);

void __VERIFIER_recovery_routine(void)
{
	/* Read the contents of the product file */
	int fd = open("result", O_RDONLY, 0640);
	assert(fd != -1);

	char buf[MAX_FILESIZE];
	int nr = read(fd, buf, MAX_FILESIZE);
	return;
}

int main()
{
	pthread_t t1, t2, t3, t4;

	int fd = creat("result", 0640);
	assert(fd != -1);
	__VERIFIER_pbarrier();

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_3, NULL))
		abort();
	if (pthread_create(&t4, NULL, thread_4, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();
	if (pthread_join(t3, NULL))
		abort();
	if (pthread_join(t4, NULL))
		abort();

	/* Read the files and stitch them together */
	int f1 = open("file1", O_RDONLY, S_IRWXU);
	int f2 = open("file2", O_RDONLY, S_IRWXU);
	int f3 = open("file3", O_RDONLY, S_IRWXU);
	int f4 = open("file4", O_RDONLY, S_IRWXU);

	assert(f1 != -1 && f2 != -1 && f3 != -1 && f4 != -1);

	char buf1[FILESIZE], buf2[FILESIZE], buf3[FILESIZE], buf4[FILESIZE];

	int n1 = read(f1, buf1, FILESIZE);
	int n2 = read(f2, buf2, FILESIZE);
	int n3 = read(f3, buf3, FILESIZE);
	int n4 = read(f4, buf4, FILESIZE);
	assert(n1 == FILESIZE && n2 == FILESIZE && n3 == FILESIZE && n4 == FILESIZE);

	write(fd, buf1, n1);
	write(fd, buf2, n2);
	write(fd, buf3, n3);
	write(fd, buf4, n4);

	return 0;
}
