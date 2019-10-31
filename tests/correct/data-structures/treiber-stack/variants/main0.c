#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../main.c"

int main()
{
	unsigned int in_sum = 0, out_sum = 0;

	atomic_init(&x[1], 0);
	atomic_init(&x[2], 0);

	init_stack(&stack, num_threads);
	for (int i = 0; i < num_threads; i++) {
		param[i] = i;
		pthread_create(&threads[i], NULL, main_task, &param[i]);
	}

	for (int i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	/* bool correct = false; */
	//correct |= (a == 17 || a == 37 || a == 0);
	//MODEL_ASSERT(correct);

	/* free(param); */
	/* free(threads); */
	/* free(stack); */

	return 0;
}
