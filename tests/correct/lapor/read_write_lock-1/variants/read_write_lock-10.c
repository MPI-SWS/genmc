#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#include "../read_write_lock-1.c"

int main()
{
	pthread_t t1, t2, t3, t4;

	if (pthread_create(&t1, NULL, writer, NULL))
		abort();
	if (pthread_create(&t2, NULL, reader, NULL))
		abort();
	if (pthread_create(&t3, NULL, writer, NULL))
		abort();
	if (pthread_create(&t4, NULL, reader, NULL))
		abort();

	/* pthread_join(t1, NULL); */
	/* pthread_join(t2, NULL); */
	/* pthread_join(t3, NULL); */
	/* pthread_join(t4, NULL); */

	return 0;
}
