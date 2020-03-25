pthread_mutex_t atomic_l;

#define __VERIFIER_atomic_begin() pthread_mutex_lock(&atomic_l)
#define __VERIFIER_atomic_end()   pthread_mutex_unlock(&atomic_l)

#define __VERIFIER_error() assert(0)

int i = 3;
int j = 6;

#ifndef NUM
# define NUM 5
#endif
#define LIMIT (2 * NUM + 6)

void *t1(void *unused)
{
	for (int k = 0; k < NUM; k++) {
		__VERIFIER_atomic_begin();
		i = j + 1;
		__VERIFIER_atomic_end();
	}
	return NULL;
}

void *t2(void *unused)
{
	for (int k = 0; k < NUM; k++) {
		__VERIFIER_atomic_begin();
		j = i + 1;
		__VERIFIER_atomic_end();
	}
	return NULL;
}
