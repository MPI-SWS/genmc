#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../time_var_mutex.c"

int main()
{
	pthread_t t1, t2;

	__VERIFIER_assume(inode == busy);

	if (pthread_mutex_init(&m_inode, NULL))
		abort();
	if (pthread_mutex_init(&m_busy, NULL))
		abort();

	if (pthread_create(&t1, NULL, allocator, NULL))
		abort();
	if (pthread_create(&t2, NULL, de_allocator, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	/* pthread_mutex_destroy(&m_inode); */
	/* pthread_mutex_destroy(&m_busy); */
	return 0;
}
