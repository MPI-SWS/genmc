#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <genmc.h>

struct foo {
	int (*q)(int);
	int value;
};

int (*g)(int);

atomic_int a[4];

int __get_var(int i) { return a[i];}

int get_var(int i) { return __get_var(i); }

int get_indirect_local(int i)
{
	int (*p)(int);

	p = &get_var;
	return (*p)(i);
}

int get_indirect_global(int i)
{
	g = &get_var;
	return (*g)(i);
}

int get_indirect_struct(int i)
{
	struct foo s;

	s.q = &get_var;
	return s.q(i);
}

int get_indirect_recursive(int i)
{
	static bool entered = false;

	if (!entered) {
		entered = true;
		return get_indirect_recursive(i);
	}
	return get_var(i);
}

void *thread_1(void *unused)
{
	for (int i = 0; i < 4; i++)
		for (int j = 1; j <= 2; j++)
			a[i] = j;
	return NULL;
}

void *thread_2(void *unused)
{
	/* Assuming local() is inlined, we should be able to get rid
	 * of the blocked executions corresponding to it */
	__VERIFIER_assume(get_indirect_local(0) == 2 &&
			  get_indirect_global(1) == 2 &&
			  get_indirect_struct(2) == 2 &&
			  get_indirect_recursive(3) == 2);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
