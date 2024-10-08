#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>
#include <stdatomic.h>
#include <genmc.h>

#include "../main.c"

int main()
{
	int i = 0;
	unsigned int in_sum = 0, out_sum = 0;

	queue = &myqueue;
	num_threads = readers + writers + rdwr;

	init_queue(queue, num_threads);
	for (int j = 0; j < num_threads; j++)
		param[j] = j;
	for (int j = 0; j < writers; j++, i++)
		pthread_create(&threads[i], NULL, threadW, &param[i]);
	for (int j = 0; j < readers; j++, i++)
		pthread_create(&threads[i], NULL, threadR, &param[i]);
	for (int j = 0; j < rdwr; j++, i++)
		pthread_create(&threads[i], NULL, threadRW, &param[i]);

	/* for (i = 0; i < num_threads; i++) { */
	/* 	param[i] = i; */
	/* 	pthread_create(&threads[i], NULL, main_task, &param[i]); */
	/* } */

	/* for (i = 0; i < num_threads; i++) { */
	/* 	in_sum += input[i]; */
	/* 	out_sum += output[i]; */
	/* } */
	/* for (i = 0; i < num_threads; i++) */
	/* 	printf("input[%d] = %u\n", i, input[i]); */
	/* for (i = 0; i < num_threads; i++) */
	/* 	printf("output[%d] = %u\n", i, output[i]); */
	/* if (succ1 && succ2) */
	/* 	assert(in_sum == out_sum); */
	/* else */
	/* 	assert(0); */

	return 0;
}
