/* Testcase from Threader's distribution. For details see:
   http://www.model.in.tum.de/~popeea/research/threader
   This file is adapted from the example introduced in the paper:
   Thread-Modular Verification for Shared-Memory Programs
   by Cormac Flanagan, Stephen Freund, Shaz Qadeer.
*/

void __VERIFIER_assume(int);
#define __VERIFIER_error() assert(0)

/* #define assert(e) if (!(e)) ERROR: __VERIFIER_error() */

int block;
int busy; // boolean flag indicating whether the block has been allocated to an inode
int inode;
pthread_mutex_t m_inode; // protects the inode
pthread_mutex_t m_busy; // protects the busy flag

void *allocator(void *_)
{
	pthread_mutex_lock(&m_inode);
	if (inode == 0) {
		pthread_mutex_lock(&m_busy);
		busy = 1;
		pthread_mutex_unlock(&m_busy);
		inode = 1;
	}
	block = 1;
	assert(block == 1);
	pthread_mutex_unlock(&m_inode);
	return NULL;
}

void *de_allocator(void *_)
{
	pthread_mutex_lock(&m_busy);
	if (busy == 0) {
		block = 0;
		assert(block == 0);
	}
	pthread_mutex_unlock(&m_busy);
	return ((void *)0);
}
