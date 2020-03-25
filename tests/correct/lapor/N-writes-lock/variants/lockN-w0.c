#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>

#include "../lockN-w.c"

int main()
{
	pthread_t t[N];

	for (int i = 0; i < N; i++) {
		if (pthread_create(&t[i], NULL, threadn, NULL))
			abort();
	}

	return 0;
}
