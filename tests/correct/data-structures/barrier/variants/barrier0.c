#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "../main.c"

int main()
{
	pthread_t W, R[NUMREADERS];

	if (pthread_create(&W, NULL, threadW, NULL))
		abort();
	for (int i = 0; i < NUMREADERS; i++)
		if (pthread_create(&R[i], NULL, threadR, NULL))
			abort();

	for (int i = 0; i < NUMREADERS; i++)
		if (pthread_join(R[i], NULL))
			abort();
	if (pthread_join(W, NULL))
		abort();

	return 0;
}
