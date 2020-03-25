#define __VERIFIER_nondet_int() 42   /* int __VERIFIER_nondet_int(void); */
#define __VERIFIER_error() assert(0)

#ifndef SIZE
# define SIZE	(20)
#endif

#define EMPTY	(-1)
#define FULL	(-2)

typedef struct {
	int element[SIZE];
	int head;
	int tail;
	int amount;
} QType;

pthread_mutex_t m;
int stored_elements[SIZE];
bool enqueue_flag, dequeue_flag;
QType queue;

void init(QType *q)
{
	q->head = 0;
	q->tail = 0;
	q->amount = 0;
}

int empty(QType *q)
{
	if (q->head == q->tail) {
		/* printf("queue is empty\n"); */
		return EMPTY;
	} else {
		return 0;
	}
}

int full(QType *q)
{
	if (q->amount == SIZE) {
		/* printf("queue is full\n"); */
		return FULL;
	} else {
		return 0;
	}
}

int enqueue(QType *q, int x)
{
	q->element[q->tail] = x;
	q->amount++;
	if (q->tail == SIZE) {
		q->tail = 1;
	} else {
		q->tail++;
	}

	return 0;
}

int dequeue(QType *q)
{
	int x;

	x = q->element[q->head];
	q->amount--;
	if (q->head == SIZE) {
		q->head = 1;
	} else {
		q->head++;
	}

	return x;
}

void *t1(void *arg)
{
	pthread_mutex_lock(&m);
	if (enqueue_flag) {
		for (int i = 0; i < SIZE; i++) {
			int value = __VERIFIER_nondet_int();
			enqueue(&queue, value);
			stored_elements[i] = value;
		}
		enqueue_flag = false;
		dequeue_flag = true;
	}
	pthread_mutex_unlock(&m);

	return NULL;
}

void *t2(void *arg)
{
	pthread_mutex_lock(&m);
	if (dequeue_flag) {
		for(int i = 0; i < SIZE; i++) {
			if (empty(&queue) != EMPTY &&
			    dequeue(&queue) != stored_elements[i])
				__VERIFIER_error();
		}
		dequeue_flag = false;
		enqueue_flag = true;
	}
	pthread_mutex_unlock(&m);

	return NULL;
}
