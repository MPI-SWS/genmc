#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>

#include "../par_length.c"

int main(int argc, char **argv)
{
	pthread_t tid[T];

	for (int i = 0; i < T; ++i) {
		pthread_create(tid + i, NULL, thread_n, (void*)(intptr_t) i);
	}

	return 0;
}
