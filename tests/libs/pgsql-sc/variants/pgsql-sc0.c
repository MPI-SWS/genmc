#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "../pgsql-sc.c"

int main()
{
	pthread_t t1, t2;

	myinit(&latch1, true);
	myinit(&latch2, false);
	myinit(&flag1, true);
	myinit(&flag2, false);

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	/* if (pthread_join(t1, NULL)) */
	/* 	abort(); */
	/* if (pthread_join(t2, NULL)) */
	/* 	abort(); */

	return 0;
}
