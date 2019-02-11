#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../lamportfm-sc.c"

int main()
{
	pthread_t t[N+1];

	myinit(&x, 0);
	myinit(&y, 0);
	for (intptr_t i = 1; i <= N; i++)
		myinit(&b[i], false);

	for (intptr_t i = N; i >= 1; i--)
		if (pthread_create(&t[i], 0, thread, (void *) i))
			abort();

	for (intptr_t i = 1; i <= N; i++)
		if (pthread_join(t[i], NULL))
			abort();

	return 0;
}
