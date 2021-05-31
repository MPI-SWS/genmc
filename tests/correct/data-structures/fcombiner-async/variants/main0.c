#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

#include "../main.c"

int main(int argc, char **argv)
{
	struct combiner cmb;

	/* initialize the set and the combiner */
	init();
	init_combiner(&cmb);

	for (int i = 1; i < num_threads; i++)
		pthread_create(&threads[i], NULL, thread_n, &cmb);
	for (int i = 1; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	return 0;
}
