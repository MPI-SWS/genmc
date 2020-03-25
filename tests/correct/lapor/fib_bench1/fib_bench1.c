#define __VERIFIER_error() assert(0)

pthread_mutex_t atomic_l;

#define LOCK(l)   pthread_mutex_lock(&(l))
#define UNLOCK(l) pthread_mutex_unlock(&(l))

#define __VERIFIER_atomic_begin()   LOCK(atomic_l)
#define __VERIFIER_atomic_end()     UNLOCK(atomic_l)

int i = 1;
int j = 1;

#define NUM 5

void *t1(void *unused)
{
	for (int k = 0; k < NUM; k++) {
		__VERIFIER_atomic_begin();
		i += j;
		__VERIFIER_atomic_end();
	}
	return NULL;
}

void *t2(void *unused)
{
	for (int k = 0; k < NUM; k++) {
		__VERIFIER_atomic_begin();
		j += i;
		__VERIFIER_atomic_end();
	}
	return NULL;
}
