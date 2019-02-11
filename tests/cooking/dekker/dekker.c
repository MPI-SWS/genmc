#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#ifdef MAKE_ACCESSES_SC
# define mo_relaxed memory_order_seq_cst
# define mo_acquire memory_order_seq_sct
# define mo_release memory_order_seq_cst
# define mo_acq_rel memory_order_seq_cst
# define mo_seq_cst memory_order_seq_cst
# define smp_rmb()
# define smp_wmb()
# define smp_mb()
#else
# define mo_relaxed memory_order_relaxed
# define mo_acquire memory_order_acquire
# define mo_release memory_order_release
# define mo_acq_rel memory_order_acq_rel
# define mo_seq_cst memory_order_seq_cst
# ifdef NIDHUGG
#  define smp_rmb() __asm__ __volatile__("mfence" ::: "memory")
#  define smp_wmb() __asm__ __volatile__("mfence" ::: "memory")
#  define smp_mb()  __asm__ __volatile__("mfence" ::: "memory")
# else
#  define smp_rmb()  atomic_thread_fence(memory_order_acquire)
#  define smp_wmb()  atomic_thread_fence(memory_order_release)
#  define smp_mb()   atomic_thread_fence(memory_order_seq_cst)
# endif
#endif

atomic_bool flag0 = ATOMIC_VAR_INIT(false);
atomic_bool flag1 = ATOMIC_VAR_INIT(false);
atomic_int turn   = ATOMIC_VAR_INIT(0);

int var = 0;

void *thread_0(void *unused)
{
	for (int i = 0; i < NUM; i++) {
		atomic_store_explicit(&flag0, true, mo_relaxed);
		smp_mb();
		while (atomic_load_explicit(&flag1, mo_relaxed)) {
			if (atomic_load_explicit(&turn, mo_relaxed) != 0) {
				atomic_store_explicit(&flag0, false, mo_relaxed);
				while (atomic_load_explicit(&turn, mo_relaxed) != 0)
					;
				atomic_store_explicit(&flag0, true, mo_relaxed);
				smp_mb();
			}
		}
		smp_rmb();

		// critical section
		var = 42;

		atomic_store_explicit(&turn, 1, mo_relaxed);
		smp_wmb();
		atomic_store_explicit(&flag0, false, mo_relaxed);
	}
	return NULL;
}

void *thread_1(void *unused)
{
	for (int i = 0; i < NUM; i++) {
		atomic_store_explicit(&flag1, true, mo_relaxed);
		smp_mb();
		while (atomic_load_explicit(&flag0, mo_relaxed)) {
			if (atomic_load_explicit(&turn, mo_relaxed) != 1) {
				atomic_store_explicit(&flag1, false, mo_relaxed);
				while (atomic_load_explicit(&turn, mo_relaxed) != 1)
					;
				atomic_store_explicit(&flag1, true, mo_relaxed);
				smp_mb();
			}
		}
		smp_rmb();

		// critical section
		var = 17;

		atomic_store_explicit(&turn, 0, mo_relaxed);
		smp_wmb();
		atomic_store_explicit(&flag1, false, mo_relaxed);
	}
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, NULL, thread_0, NULL);
	pthread_create(&t2, NULL, thread_1, NULL);

	return 0;
}
