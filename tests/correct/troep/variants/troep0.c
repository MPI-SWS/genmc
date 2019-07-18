/*
 * Test case by: Carlo Wood <carlo@alinoe.com>
 *
 * This test case demonstrates why special care when reading from stores
 * following failed CASes is necessary. More specifically, it shows that
 * a store stemming from a failed CAS can have an i64 type (same as the
 * CAS, which is performed on atomics/i64), but subsequent reads in the
 * same location can read based on the actual type of the variable (in
 * this test case pointer type). Thus, special care in the interpreter
 * regarding the type is needed when reading from such stores.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>

#include "../troep.c"

int main()
{
	// Start THREADS threads that each append one node the the queue.
	for (int i = 0; i < THREADS; ++i)
		if (pthread_create(&t[i], NULL, producer_thread, &n[i]))
			abort();

	// Wait till all threads finished.
	for (int i = 0; i < THREADS; ++i)
		if (pthread_join(t[i], NULL))
			abort();

	// Count number of elements in the list.
	struct Node* l = list;
	int sum = 0;
	while (l)
	{
		++sum;
		l = l->next;
	}
	/* printf("Sum = %d\n", sum); */

	return 0;
}
