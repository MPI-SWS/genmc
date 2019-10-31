#include "fake.h"
#include "seqlock.h"

#define MAXREADERS 3
#define MAXWRITERS 3
#define MAXRDWR 3

#ifdef CONFIG_SEQ_READERS
#define DEFAULT_READERS (CONFIG_SEQ_READERS)
#else
#define DEFAULT_READERS 1
#endif

#ifdef CONFIG_SEQ_WRITERS
#define DEFAULT_WRITERS (CONFIG_SEQ_WRITERS)
#else
#define DEFAULT_WRITERS 1
#endif

#ifdef CONFIG_SEQ_RDWR
#define DEFAULT_RDWR (CONFIG_SEQ_RDWR)
#else
#define DEFAULT_RDWR 0
#endif

int readers = DEFAULT_READERS, writers = DEFAULT_WRITERS, rdwr = DEFAULT_RDWR;

DEFINE_SEQLOCK(lock);

atomic_int shared;

void *threadR(void *unused)
{
	unsigned int seq;
	int data;

	do {
		seq = read_seqbegin(&lock);
		/* Make a copy of the data of interest */
		data = atomic_load_explicit(&shared, mo_relaxed);
	} while (read_seqretry(&lock, seq));
	return NULL;
}

void *threadW(void *unused)
{
	write_seqlock(&lock);
	atomic_store_explicit(&shared, 42, mo_relaxed);
	write_sequnlock(&lock);
	return NULL;
}

void *threadRW(void *unused)
{
	unsigned int seq;
	int data;

	do {
		seq = read_seqbegin(&lock);
		/* Make a copy of the data of interest */
		data = atomic_load_explicit(&shared, mo_relaxed);
	} while (read_seqretry(&lock, seq));

	write_seqlock(&lock);
	atomic_store_explicit(&shared, data + 1, mo_relaxed);
	write_sequnlock(&lock);
	return NULL;
}
