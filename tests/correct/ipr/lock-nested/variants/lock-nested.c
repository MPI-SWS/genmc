/*
 * Demonstrates that the ReadOpt invariant (event would block
 * by reading maximally in the **current** execution) can be broken
 * when unlocks are added.
 * Here this causes two Locks to be IPR'd by the same Unlock,
 * leading to a (false) non-atomic race.
 * Variables c and a are optional and only help to consistently hit the bug.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

int na = 0;
atomic_int c = 0;
atomic_int a = 0;
pthread_mutex_t x;
pthread_mutex_t y;

void *thread_race(void *unused)
{
	__VERIFIER_assume(atomic_fetch_add(&c, 1) < 2);
	pthread_mutex_lock(&y);
	na = 1;
	pthread_mutex_unlock(&y);
	return NULL;
}

void *thread_1(void *unused)
{
	pthread_mutex_lock(&y);
	__VERIFIER_assume(atomic_fetch_add(&c, 1) == 3);
	atomic_store(&a, 1);
	pthread_mutex_lock(&x);
	pthread_mutex_unlock(&x);
	pthread_mutex_unlock(&y);
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&x);
	__VERIFIER_assume(atomic_fetch_add(&c, 1) == 2);
	__VERIFIER_assume(atomic_load(&a) == 1);
	pthread_mutex_unlock(&x);
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3, t4;

	if (pthread_create(&t1, NULL, thread_race, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_race, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t4, NULL, thread_2, NULL))
		abort();

	return 0;
}
