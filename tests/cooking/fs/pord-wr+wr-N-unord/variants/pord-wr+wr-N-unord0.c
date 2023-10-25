#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>
#include <genmc.h>

/* Different threads concurrently writing to the same file in a completely
 * unordered fashion. Each thread apends to the file upon completing its calculation.
 *
 * After a crash, only a prefix of the file written might have persisted.
 */

#define NUM_WRITERS 4
#define WRITER_BATCHES 2
#define WRITER_BATCH_SIZE 2
#define MAX_FILESIZE NUM_WRITERS * WRITER_BATCHES * WRITER_BATCH_SIZE

struct thread_info_struct {
	pthread_t tid; /* POSIX thread id, as returned by the library */
	int thrid; /* Application-defined thread id */
	int thrcnt;
	int data;
};

struct thread_info_struct threads[NUM_WRITERS];

void *thread_n(void *arg)
{
	struct thread_info_struct *thr = arg;

	int fd = open("thefile", O_WRONLY|O_APPEND, 0640);
	assert(fd != -1);

	for (int i = 0u; i < WRITER_BATCHES; ++i) {
		/* Model a (supposedly intense) computation taking place */
		char buf[WRITER_BATCH_SIZE];
		for (int j = 0u; j < WRITER_BATCH_SIZE; j++)
			buf[j] = thr->data;

		/* Append to the file  */
		int nw = write(fd, buf, WRITER_BATCH_SIZE);
		assert(nw == WRITER_BATCH_SIZE);
	}
	return NULL;
}

void __VERIFIER_recovery_routine(void)
{
	/* Observe the outcome of the serialization */
	int fd = open("thefile", O_RDONLY, S_IRWXU);
	assert(fd != -1);

	char buf[MAX_FILESIZE];
	/* int nr = read(fd, buf, MAX_FILESIZE); */
	int nr = lseek(fd, 0, SEEK_END);
	assert(nr <= MAX_FILESIZE); /* Trivally true */
	return;
}

int main()
{
	pthread_t t[NUM_WRITERS];

	int fd = creat("thefile", 0640);
	__VERIFIER_pbarrier();

	for (int i = 0u; i < NUM_WRITERS; i++) {
		threads[i].thrid = i;
		threads[i].data = i;
		threads[i].thrcnt = NUM_WRITERS;
		if (pthread_create(&threads[i].tid, NULL, thread_n, &threads[i]))
			abort();
	}

	return 0;
}
