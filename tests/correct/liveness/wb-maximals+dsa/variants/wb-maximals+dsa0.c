#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#include <lkmm.h>
#include <genmc.h>

#include "../wb-maximals+dsa.c"

int main()
{
	pthread_t ta, tb;

	pthread_create(&ta, NULL, alice, NULL);
	pthread_create(&tb, NULL, bob, NULL);

	pthread_join(ta, NULL);
	pthread_join(tb, NULL);
	assert(locked == 0);

	return 0;
}
