#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../main.c"

int main()
{
	pthread_t t1, t2, t3;

	init_qrcu_struct(&qs);

	if (pthread_create(&t1, NULL, qrcu_reader, NULL))
		abort();
	if (pthread_create(&t2, NULL, qrcu_reader, NULL))
		abort();
	if (pthread_create(&t3, NULL, qrcu_updater, NULL))
		abort();

	return 0;
}
