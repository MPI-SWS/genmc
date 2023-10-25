#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

/* Different threads concurrently writing to different files.
 *
 * The contents of the files might persist in any order.
 */

#define FILESIZE 4

#define DECLARE_WRITER(id)						\
	void *thread_##id(void *unused)					\
	{								\
		char buf[FILESIZE] = { [0 ... FILESIZE - 1] = 42};	\
		int fd = creat("file" #id, 0640);			\
		int nw = write(fd, buf, FILESIZE);			\
		assert(nw == FILESIZE);					\
		return NULL;						\
	}

/* Preprocessor trickery to only use const static strings for filenames... */
DECLARE_WRITER(1);
DECLARE_WRITER(2);
DECLARE_WRITER(3);
DECLARE_WRITER(4);

void __VERIFIER_recovery_routine(void)
{
	/* Just observe the various sizes the files might have */
	int f1 = open("file1", O_RDONLY, S_IRWXU);
	int f2 = open("file2", O_RDONLY, S_IRWXU);
	int f3 = open("file3", O_RDONLY, S_IRWXU);
	int f4 = open("file4", O_RDONLY, S_IRWXU);

	if (f1 == -1 || f2 == -1 || f3 == -1 || f4 == -1)
		return;

	int s1 = lseek(f1, 0, SEEK_END);
	int s2 = lseek(f2, 0, SEEK_END);
	int s3 = lseek(f3, 0, SEEK_END);
	int s4 = lseek(f4, 0, SEEK_END);
	return;
}

int main()
{
	pthread_t t1, t2, t3, t4;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_3, NULL))
		abort();
	if (pthread_create(&t4, NULL, thread_4, NULL))
		abort();

	return 0;
}
