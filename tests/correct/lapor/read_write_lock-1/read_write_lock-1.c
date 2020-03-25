/* Testcase from Threader's distribution. For details see:
   http://www.model.in.tum.de/~popeea/research/threader

   This file is adapted from the example introduced in the paper:
   Thread-Modular Verification for Shared-Memory Programs
   by Cormac Flanagan, Stephen Freund, Shaz Qadeer.
*/

#define __VERIFIER_error() assert(0)
void __VERIFIER_assume(int);

int w = 0, r = 0;
atomic_int x, y;

pthread_mutex_t atomic_l;

void __VERIFIER_atomic_take_write_lock()
{
	pthread_mutex_lock(&atomic_l);
	__VERIFIER_assume(w == 0 && r == 0);
	w = 1;
	pthread_mutex_unlock(&atomic_l);
}

void __VERIFIER_atomic_release_write_lock()
{
	pthread_mutex_lock(&atomic_l);
	w = 0;
	pthread_mutex_unlock(&atomic_l);
}

void __VERIFIER_atomic_take_read_lock()
{
	pthread_mutex_lock(&atomic_l);
	__VERIFIER_assume(w == 0);
	r = r + 1;
	pthread_mutex_unlock(&atomic_l);
}

void __VERIFIER_atomic_release_read_lock()
{
	pthread_mutex_lock(&atomic_l);
	r = r - 1;
	pthread_mutex_unlock(&atomic_l);
}

/* writer */
void *writer(void *arg)
{
	__VERIFIER_atomic_take_write_lock();
	x = 3;
	__VERIFIER_atomic_release_write_lock();
	return 0;
}

/* reader */
void *reader(void *arg)
{
	int l;

	__VERIFIER_atomic_take_read_lock();
	l = x;
	y = l;
	assert(y == x);
	__VERIFIER_atomic_release_read_lock();
	return 0;
}
