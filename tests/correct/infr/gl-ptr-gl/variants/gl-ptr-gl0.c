/* Ensures the address-translation mechanism works properly with static pointers */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

#ifndef N
# define N 41
#endif

int v;

struct stat {
	int f;
};

struct stat __open_file_stat;

typedef struct {
	struct stat *s;
} openfilestruct;

openfilestruct __open_file = {
	.s = &__open_file_stat,
};

openfilestruct *openfile;

atomic_int x;

void *thread_1(void *unused)
{
	int r = x;

	if (openfile->s->f == r)
		;
	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 0u; i < N; i++)
		x = i;

	if (openfile->s->f == v)
		;
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	openfile = &__open_file;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
