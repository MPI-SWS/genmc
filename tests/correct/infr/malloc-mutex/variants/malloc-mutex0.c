#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
	pthread_mutex_t m;
} lock;

int main()
{
	lock *t = malloc(sizeof(lock));

	pthread_mutex_init(&t->m, NULL);
	pthread_mutex_lock(&t->m);

	return 0;
}
