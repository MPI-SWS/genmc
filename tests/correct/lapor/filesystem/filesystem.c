#define NUMBLOCKS 26
#define NUMINODES 32

pthread_mutex_t locki[NUMINODES];
int inode[NUMINODES];

pthread_mutex_t lockb[NUMBLOCKS];
bool busy[NUMBLOCKS];

int idx[N];

void *thread_n(void *arg)
{
	int tid = *((int *) arg);
	int i = tid % NUMINODES;

	pthread_mutex_lock(&locki[i]);
	if (inode[i] == 0) {
		int b = (i * 2) % NUMBLOCKS;
		while (true) {
			pthread_mutex_lock(&lockb[b]);
			if (!busy[b]) {
				busy[b] = true;
				inode[i] = b + 1;
				pthread_mutex_unlock(&lockb[b]);
				break;
			}
			pthread_mutex_unlock(&lockb[b]);
			b = (b + 1) % NUMBLOCKS;
		}
	}
	pthread_mutex_unlock(&locki[i]);
	return NULL;
}
