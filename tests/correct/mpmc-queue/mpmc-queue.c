#include "mpmc-queue.h"

#define MAXREADERS 3
#define MAXWRITERS 3
#define MAXRDWR 3

#ifdef CONFIG_MPMC_READERS
#define DEFAULT_READERS (CONFIG_MPMC_READERS)
#else
#define DEFAULT_READERS 1
#endif

#ifdef CONFIG_MPMC_WRITERS
#define DEFAULT_WRITERS (CONFIG_MPMC_WRITERS)
#else
#define DEFAULT_WRITERS 1
#endif

#ifdef CONFIG_MPMC_RDWR
#define DEFAULT_RDWR (CONFIG_MPMC_RDWR)
#else
#define DEFAULT_RDWR 0
#endif

int readers = DEFAULT_READERS, writers = DEFAULT_WRITERS, rdwr = DEFAULT_RDWR;

struct mpmc_boundq_1_alt queue;

void *threadW(void *unused)
{
	int32_t *bin = write_prepare(&queue);
	*bin = 1;
	write_publish(&queue);
	return NULL;
}

void *threadR(void *unused)
{
	int32_t *bin;
	while ((bin = read_fetch(&queue)) != NULL) {
		// printf("Read: %d\n", load_32(bin));
		read_consume(&queue);
	}
	return NULL;
}

void *threadRW(void *unused)
{
	int32_t *bin = write_prepare(&queue);
	*bin = 1;
	write_publish(&queue);

	while ((bin = read_fetch(&queue)) != NULL) {
//		printf("Read: %d\n", load_32(bin));
		read_consume(&queue);
	}
	return NULL;
}
