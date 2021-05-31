#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../main.c"

int main()
{
	int i = 0;

	num_threads = readers + writers + rdwr;

	atomic_init(&x[1], 0);
	atomic_init(&x[2], 0);

	init_stack(&stack, num_threads);
	for (int j = 0; j < num_threads; j++)
		param[j] = j;
	for (int j = 0; j < writers; j++, i++)
		pthread_create(&threads[i], NULL, threadW, &param[i]);
	for (int j = 0; j < readers; j++, i++)
		pthread_create(&threads[i], NULL, threadR, &param[i]);
	for (int j = 0; j < rdwr; j++, i++)
		pthread_create(&threads[i], NULL, threadRW, &param[i]);

	for (i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	/* bool correct = false; */
	//correct |= (a == 17 || a == 37 || a == 0);
	//MODEL_ASSERT(correct);

	/* free(param); */
	/* free(threads); */
	/* free(stack); */

	return 0;
}
