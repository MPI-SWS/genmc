#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>
#include <genmc.h>

/* Different threads concurrently writing to the same file in a
 * round-robin fashion. Each thread is appending to the file when its
 * turn arrives. (Alternatively, each thread could write to a specific
 * offset, provided that the file is large enough to begin with.)
 *
 * After a crash, only a prefix of the file written might have persisted,
 * but the prefix should be in accordance with the order the file
 * was written.
 */

#define NUM_WRITERS 4
#define NUM_BATCHES 8
#define WRITER_BATCH_SIZE 2

struct thread_info_struct {
	pthread_t tid; /* POSIX thread id, as returned by the library */
	int thrid; /* Application-defined thread id */
	int thrcnt;
	int data;
};

atomic_bool condition[NUM_WRITERS];
struct thread_info_struct threads[NUM_WRITERS];

void *thread_n(void *arg)
{
	struct thread_info_struct *thr = arg;

	int fd = open("thefile", O_WRONLY, 0640);
	assert(fd != -1);

	for (int i = thr->thrid; i < NUM_BATCHES * WRITER_BATCH_SIZE; i += thr->thrcnt) {
		/* Model a (supposedly intense) computation taking place */
		char buf[WRITER_BATCH_SIZE];
		for (int j = 0u; j < WRITER_BATCH_SIZE; j++)
			buf[j] = thr->data;

		/* Wait to be signaled */
		__VERIFIER_assume(atomic_load_explicit(&condition[thr->thrid], memory_order_acquire));

		/* Append to the file */
		int npos = lseek(fd, 0, SEEK_END);
		int nw = write(fd, buf, WRITER_BATCH_SIZE);
		assert(nw == WRITER_BATCH_SIZE);

		/* Reset condition and signal successor */
		atomic_store_explicit(&condition[thr->thrid], 0, memory_order_release);
		atomic_store_explicit(&condition[(thr->thrid + 1) % (thr->thrcnt)], 1, memory_order_release);
	}
	return NULL;
}

void __VERIFIER_recovery_routine(void)
{
	/* Observe the outcome of the serialization */
	int fd = open("thefile", O_RDONLY, S_IRWXU);
	assert(fd != -1);

	char buf[NUM_BATCHES * WRITER_BATCH_SIZE];
	int nr = read(fd, buf, NUM_BATCHES * WRITER_BATCH_SIZE);

	for (int i = 0; i < nr - 1; i++)
		if (i % NUM_WRITERS <= (i + 1) % NUM_WRITERS)
			assert(buf[i] <= buf[i + 1]);
	return;
}

int main()
{
	pthread_t t[NUM_WRITERS];

	int fd = creat("thefile", 0640);
	__VERIFIER_pbarrier();

	atomic_store_explicit(&condition[0], 1, memory_order_relaxed);
	for (int i = 1u; i < NUM_WRITERS; i++)
		atomic_store_explicit(&condition[i], 0, memory_order_relaxed);

	for (int i = 0u; i < NUM_WRITERS; i++) {
		threads[i].thrid = i;
		threads[i].data = i;
		threads[i].thrcnt = NUM_WRITERS;
		if (pthread_create(&threads[i].tid, NULL, thread_n, &threads[i]))
			abort();
	}

	return 0;
}
