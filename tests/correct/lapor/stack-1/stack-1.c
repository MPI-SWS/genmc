#define __VERIFIER_error() assert(0)
#define __VERIFIER_nondet_uint() 1u

extern void __VERIFIER_assume(int);

#ifndef SIZE
# define SIZE	  (5)
#endif
#define OVERFLOW  (-1)
#define UNDERFLOW (-2)

static int top = 0;
static unsigned int arr[SIZE];

pthread_mutex_t m;

void error(void)
{
	__VERIFIER_error();
	return;
}

void inc_top(void)
{
	top++;
}

void dec_top(void)
{
	top--;
}

int get_top(void)
{
	return top;
}

int stack_empty(void)
{
	return (top == 0) ? true : false;
}

int push(unsigned int *stack, int x)
{
	if (top == SIZE) {
		return OVERFLOW;
	} else {
		stack[get_top()] = x;
		inc_top();
	}
	return 0;
}

int pop(unsigned int *stack)
{
	if (get_top() == 0) {
		return UNDERFLOW;
	} else {
		dec_top();
		return stack[get_top()];
	}
	return 0;
}

void *t1(void *arg)
{
	for(int i = 0; i < SIZE; i++) {
		pthread_mutex_lock(&m);
		/* tmp = __VERIFIER_nondet_uint(); */
		/* __VERIFIER_assume(tmp < SIZE); */
		unsigned int tmp = i;
		if (push(arr,tmp) == OVERFLOW)
			error();
		pthread_mutex_unlock(&m);
	}
	return NULL;
}

void *t2(void *arg)
{
	for(int i = 0; i < SIZE; i++) {
		pthread_mutex_lock(&m);
		if (top > 0) {
			if (pop(arr) == UNDERFLOW)
				error();
		}
		pthread_mutex_unlock(&m);
	}
	return NULL;
}
