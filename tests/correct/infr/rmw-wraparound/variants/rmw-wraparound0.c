/* Check that RMW results stay within their access size, even when they carry or borrow */

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>
#include <limits.h>

atomic_int x = 5;

atomic_uint ua = 5;
atomic_uint ub = 0xFFFFFFFF;
atomic_uint uc = 0;
atomic_int ia = INT_MAX;
atomic_int ib = 0;
atomic_short s = 5;
atomic_schar c = 5;
int n = 5;
atomic_long l = 5;

void *thread_1(void *unused)
{
	atomic_fetch_add_explicit(&x, -1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_fetch_add_explicit(&x, -1, memory_order_relaxed);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	int ex = 3;
	assert(atomic_compare_exchange_strong(&x, &ex, 0));

	atomic_fetch_add(&ua, 0xFFFFFFFF);
	unsigned int eua = 4;
	assert(atomic_compare_exchange_strong(&ua, &eua, 0));

	atomic_fetch_add(&ub, 1);
	unsigned int eub = 0;
	assert(atomic_compare_exchange_strong(&ub, &eub, 1));

	atomic_fetch_sub(&uc, 1);
	unsigned int euc = 0xFFFFFFFF;
	assert(atomic_compare_exchange_strong(&uc, &euc, 0));

	atomic_fetch_add(&ia, 1);
	int eia = INT_MIN;
	assert(atomic_compare_exchange_strong(&ia, &eia, 0));

	atomic_fetch_sub(&ib, 1);
	int eib = -1;
	assert(atomic_compare_exchange_strong(&ib, &eib, 0));

	atomic_fetch_add(&s, -1);
	short es = 4;
	assert(atomic_compare_exchange_strong(&s, &es, 0));

	atomic_fetch_add(&c, -1);
	signed char ec = 4;
	assert(atomic_compare_exchange_strong(&c, &ec, 0));

	__atomic_fetch_nand(&n, 1, __ATOMIC_SEQ_CST);
	int en = ~(5 & 1);
	assert(__atomic_compare_exchange_n(&n, &en, 0, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));

	atomic_fetch_add(&l, -1);
	long el = 4;
	assert(atomic_compare_exchange_strong(&l, &el, 0));

	return 0;
}
