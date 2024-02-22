#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

#ifndef N
#define N 3
#endif

#include "../ncas-fai.c"

int main()
{
	pthread_t t[N+1];

        for (int i = 0; i < N; i++)
                if (pthread_create(&t[i], NULL, thread_n, NULL))
                        abort();

        if (pthread_create(&t[N], NULL, thread_fai, NULL))
                abort();

	return 0;
}
