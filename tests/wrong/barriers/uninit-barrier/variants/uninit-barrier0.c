#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

pthread_barrier_t b;

int main()
{
	pthread_barrier_wait(&b);
	return 0;
}
