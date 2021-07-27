/* Test case by: Diogo Behrens (see Github #23) */

#define await_while(cond)						\
({									\
	int tmp = 0;							\
	for (__VERIFIER_loop_begin();					\
	     __VERIFIER_spin_start(), tmp = (cond),			\
		     __VERIFIER_spin_end(!tmp), tmp;)			\
		;							\
})

int locked;
int *next;

void *alice(void *unused)
{
	WRITE_ONCE(locked, 1);
	smp_store_release(&next, &locked);
	await_while (READ_ONCE(locked));
	return NULL;
}
void *bob(void *unused)
{
	int *i;
	await_while((i = smp_load_acquire(&next)) == 0); // was READ_ONCE
	WRITE_ONCE(*i, 0);
	return NULL;
}
