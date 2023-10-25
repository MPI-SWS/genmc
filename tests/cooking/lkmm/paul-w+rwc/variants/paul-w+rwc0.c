#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;
int a;
int b;
spinlock_t l;

void *P0(void *unused)
{
	WRITE_ONCE(a, 1);
	smp_mb(); /* Lock acquisition for rcu_node ->lock. */
	WRITE_ONCE(x, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r3 = READ_ONCE(x);
	smp_mb(); /* Lock acquisition for rcu_node ->lock. */
	spin_lock(&l); /* Locking in complete(). */
	WRITE_ONCE(y, 1);
	spin_unlock(&l);
	return NULL;
}

void *P2(void *unused)
{
	spin_lock(&l); /* Locking in wait_for_completion. */
	int r4 = READ_ONCE(y);
	spin_unlock(&l);
	int r1 = READ_ONCE(b);
	return NULL;
}

void *P3(void *unused)
{
	WRITE_ONCE(b, 1);
	smp_mb();
	int r2 = READ_ONCE(a);
	return NULL;
}


int main()
{
	pthread_t t0, t1, t2, t3;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);
	pthread_create(&t3, NULL, P3, NULL);

	return 0;
}
