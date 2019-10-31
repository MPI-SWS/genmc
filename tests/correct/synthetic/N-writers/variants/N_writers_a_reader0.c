#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../N_writers_a_reader.c"

int main()
{
	pthread_t tr, tw[N + 1];

	if (pthread_create(&tr, NULL, threadR, NULL))
		abort();
	for (int i = 0; i <= N; i++) {
		idx[i] = i;
		if (pthread_create(&tw[i], NULL, threadW, &idx[i]))
			abort();
	}

	return 0;
}
