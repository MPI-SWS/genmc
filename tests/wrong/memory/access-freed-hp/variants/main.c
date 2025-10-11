#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>
#include <genmc.h>

_Atomic(int *) ptr;

void *t0(void *arg)
{
	__VERIFIER_hp_t *hp = __VERIFIER_hp_alloc();
	int *p = __VERIFIER_hp_protect(hp, &ptr);
	__VERIFIER_hp_retire(p);
	__VERIFIER_hp_free(hp);
	return NULL;
}

void *t1(void *arg)
{
	__VERIFIER_hp_t *hp = __VERIFIER_hp_alloc();
	int *p = __VERIFIER_hp_protect(hp, &ptr);
	if (p)
		assert(*p == 42);
	__VERIFIER_hp_free(hp);
	return NULL;
}

int main()
{
	pthread_t t[2];

	int *p = malloc(sizeof(int));
	*p = 42;
	atomic_init(&ptr, p);

	pthread_create(&t[0], NULL, t0, NULL);
	pthread_create(&t[1], NULL, t1, NULL);

	pthread_join(t[0], NULL);
	pthread_join(t[1], NULL);

	return 0;
}
