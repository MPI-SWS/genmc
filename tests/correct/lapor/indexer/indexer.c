#define __VERIFIER_error() assert(0)

#define SIZE  128
#define MAX   4
#define N  13

int table[SIZE];
pthread_mutex_t  cas_mutex[SIZE];

pthread_t  tids[N];
int idx[N];

bool cas(int *tab, int h, int val, int new_val)
{
	int ret_val = false;

	pthread_mutex_lock(&cas_mutex[h]);

	if (tab[h] == val) {
		tab[h] = new_val;
		ret_val = true;
	}

	pthread_mutex_unlock(&cas_mutex[h]);

	return ret_val;
}



void *thread_routine(void *arg)
{
	int m = 0, w, h;
	int tid = *((int *) arg);

	while (true) {
		if (m < MAX) {
			w = (++m) * 11 + tid;
		} else {
			return NULL;
		}

		h = (w * 7) % SIZE;

		if (h < 0) {
		ERROR: __VERIFIER_error();
		}

		while (cas(table, h, 0, w) == false) {
			h = (h + 1) % SIZE;
		}
	}

}
