#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
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

atomic_bool done[NUM_WRITERS + 1];

#define DECLARE_WRITER(id)						\
	void *thread_##id(void *unused)					\
	{								\
		char buf[FILESIZE] = { [0 ... FILESIZE - 1] = 42};	\
		int fd = open("file" #id, O_WRONLY, 0640);		\
		int nw = write(fd, buf, FILESIZE);			\
		assert(nw == FILESIZE);					\
		close(fd);						\
		atomic_store_explicit(&done[id], 1, memory_order_release); \
		return NULL;						\
	}

/* Preprocessor trickery to only use const static strings for filenames... */
DECLARE_WRITER(1);
DECLARE_WRITER(2);
DECLARE_WRITER(3);
DECLARE_WRITER(4);

void *thread_joiner(void *unused)
{
	int f1 = open("file1", O_RDONLY, S_IRWXU);
	int f2 = open("file2", O_RDONLY, S_IRWXU);
	int f3 = open("file3", O_RDONLY, S_IRWXU);
	int f4 = open("file4", O_RDONLY, S_IRWXU);
	assert(f1 != -1 && f2 != -1 && f3 != -1 && f4 != -1);

	int fd = open("result", O_WRONLY, 0640);
	assert(fd != -1);

	/* Repeat the procedure below until all files are fully read */
	int n1, n2, n3, n4;
	do {
		/* Busy wait until any file is ready to be read */
		while (!atomic_load_explicit(&done[1], memory_order_acquire) &&
		       !atomic_load_explicit(&done[2], memory_order_acquire) &&
		       !atomic_load_explicit(&done[3], memory_order_acquire) &&
		       !atomic_load_explicit(&done[4], memory_order_acquire))
			;

		/* Read all files up to that point */
		char buf1[FILESIZE], buf2[FILESIZE], buf3[FILESIZE], buf4[FILESIZE];

		n1 = read(f1, buf1, FILESIZE);
		n2 = read(f2, buf2, FILESIZE);
		n3 = read(f3, buf3, FILESIZE);
		n4 = read(f4, buf4, FILESIZE);
		/* assert(n1 == FILESIZE && n2 == FILESIZE && n3 == FILESIZE && n4 == FILESIZE); */

		/* And glue them together */
		write(fd, buf1, n1);
		write(fd, buf2, n2);
		write(fd, buf3, n3);
		write(fd, buf4, n4);
	} while (n1 != 0 || n2 != 0 || n3 != 0 || n4 != 0); /* Repeat if there's more to do */
	return NULL;
}

void __VERIFIER_recovery_routine(void)
{
	/* Read the contents of the product file */
	int fd = open("result", O_RDONLY, 0640);
	assert(fd != -1);

	int n = lseek(fd, 0, SEEK_END);
	assert(n <= MAX_FILESIZE);
	return;
}

int main()
{
	pthread_t t1, t2, t3, t4, tj;

	int f1 = creat("file1", S_IRWXU);
	int f2 = creat("file2", S_IRWXU);
	int f3 = creat("file3", S_IRWXU);
	int f4 = creat("file4", S_IRWXU);
	assert(f1 != -1 && f2 != -1 && f3 != -1 && f4 != -1);

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
	if (pthread_create(&tj, NULL, thread_joiner, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();
	if (pthread_join(t3, NULL))
		abort();
	if (pthread_join(t4, NULL))
		abort();
	if (pthread_join(tj, NULL))
		abort();

	return 0;
}
