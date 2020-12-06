#include <stdatomic.h>
#include <pthread.h>

#include "../rel-pporf.c"

int main()
{
	pthread_t t[2];

	pthread_create(&t[0], NULL, &runA, NULL);
	pthread_create(&t[1], NULL, &runB, NULL);

	return 0;
}
