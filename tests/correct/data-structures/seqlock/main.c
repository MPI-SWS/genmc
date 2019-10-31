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

int shared;

void *threadR(void *unused)
{
	read_seqlock_excl(&lock);
	int data = shared;
	read_sequnlock_excl(&lock);
	return NULL;
}

void *threadW(void *unused)
{
	write_seqlock(&lock);
	shared = 42;
	write_sequnlock(&lock);
	return NULL;
}

void *threadRW(void *unused)
{
	read_seqlock_excl(&lock);
	int data = shared;
	read_sequnlock_excl(&lock);
	write_seqlock(&lock);
	shared = data + 1;
	write_sequnlock(&lock);
	return NULL;
}
