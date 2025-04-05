/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Pavel Golovin <pgolovin@mpi-sws.org>
 */

/* Generic test client which allows to create test for linearizability in declarative way.
 * How to use:
 * + define PRELUDE_DECL for global declarations in test
 * + define THREAD_BODY_1 thread body of the first kind and THREAD_NUM_1 for number of copies to spawn
 * Features:
 * + It defines each thread_t type as separate variable so it is not preventing symmetric reduction.
 * + FROM1, TO1 defines which threads should be synchronized (from end of FROM1 to the beginning of TO1).
 *   (Requires having all THREAD_NUM_# equal 1 :/ )
 * */

/* UTIL MACRO */
#define PLACEHOLDER 4242

#define ITER0_MACRO_3ARGS(M, x, y) ;
#define ITER1_MACRO_3ARGS(M, x, y)  M(x, y, 1);
#define ITER2_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2);
#define ITER3_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2); M(x, y, 3);
#define ITER4_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2); M(x, y, 3); M(x, y, 4);
#define ITER5_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2); M(x, y, 3); M(x, y, 4); M(x, y, 5);
#define ITER6_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2); M(x, y, 3); M(x, y, 4); M(x, y, 5); M(x, y, 6);
#define ITER7_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2); M(x, y, 3); M(x, y, 4); M(x, y, 5); M(x, y, 6); M(x, y, 7);
#define ITER8_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2); M(x, y, 3); M(x, y, 4); M(x, y, 5); M(x, y, 6); M(x, y, 7); M(x, y, 8);
#define ITER9_MACRO_3ARGS(M, x, y)  M(x, y, 1); M(x, y, 2); M(x, y, 3); M(x, y, 4); M(x, y, 5); M(x, y, 6); M(x, y, 7); M(x, y, 8); M(x, y, 9);
#define ITER10_MACRO_3ARGS(M, x, y) M(x, y, 1); M(x, y, 2); M(x, y, 3); M(x, y, 4); M(x, y, 5); M(x, y, 6); M(x, y, 7); M(x, y, 8); M(x, y, 9); M(x, y, 10);

#define ITER0_MACRO_2ARGS(M, x) ;
#define ITER1_MACRO_2ARGS(M, x)  M(x, 1);
#define ITER2_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2);
#define ITER3_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2); M(x, 3);
#define ITER4_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2); M(x, 3); M(x, 4);
#define ITER5_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2); M(x, 3); M(x, 4); M(x, 5);
#define ITER6_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2); M(x, 3); M(x, 4); M(x, 5); M(x, 6);
#define ITER7_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2); M(x, 3); M(x, 4); M(x, 5); M(x, 6); M(x, 7);
#define ITER8_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2); M(x, 3); M(x, 4); M(x, 5); M(x, 6); M(x, 7); M(x, 8);
#define ITER9_MACRO_2ARGS(M, x)  M(x, 1); M(x, 2); M(x, 3); M(x, 4); M(x, 5); M(x, 6); M(x, 7); M(x, 8); M(x, 9);
#define ITER10_MACRO_2ARGS(M, x) M(x, 1); M(x, 2); M(x, 3); M(x, 4); M(x, 5); M(x, 6); M(x, 7); M(x, 8); M(x, 9); M(x, 10);

#define ITER(M, N) ITER##N##_MACRO_2ARGS(M, PLACEHOLDER)
#define ITER_2ARGS(M, N, x) ITER##N##_MACRO_2ARGS(M, x)
#define ITER_3ARGS(M, N, x, y) ITER##N##_MACRO_3ARGS(M, x, y)
#define ITER2D(M, i, j) ITER_3ARGS(ITER_2ARGS, i, M, j)

#ifndef THREAD_NUM_1
#define THREAD_NUM_1 0
#endif
#ifndef THREAD_NUM_2
#define THREAD_NUM_2 0
#endif
#ifndef THREAD_NUM_3
#define THREAD_NUM_3 0
#endif
#ifndef THREAD_NUM_4
#define THREAD_NUM_4 0
#endif
#ifndef THREAD_NUM_5
#define THREAD_NUM_5 0
#endif
#ifndef THREAD_NUM_6
#define THREAD_NUM_6 0
#endif
#ifndef THREAD_NUM_7
#define THREAD_NUM_7 0
#endif
#ifndef THREAD_NUM_8
#define THREAD_NUM_8 0
#endif

#define NUM_THREADS (1 + THREAD_NUM_1 + THREAD_NUM_2 + THREAD_NUM_3 + THREAD_NUM_4 + THREAD_NUM_5 + THREAD_NUM_6 + THREAD_NUM_7 + THREAD_NUM_8)

#ifndef MAX_THREADS
#define MAX_THREADS 16
#endif

#include <pthread.h>
#include <assert.h>

int get_thread_num() { return pthread_self(); }
int get_thread_max_num() { return NUM_THREADS; }
static pthread_t threads[MAX_THREADS];
static int param[MAX_THREADS];

PRELUDE_DECL();

#define MAX_NUM_OF_THREAD_KINDS 6
#define MAX_NUM_OF_THREAD_COPIES 10
#ifndef THREAD_BODY_1
#define THREAD_BODY_1() assert(0 && "Bug in generic_test_client");
#endif
#ifndef THREAD_BODY_2
#define THREAD_BODY_2() assert(0 && "Bug in generic_test_client");
#endif
#ifndef THREAD_BODY_3
#define THREAD_BODY_3() assert(0 && "Bug in generic_test_client");
#endif
#ifndef THREAD_BODY_4
#define THREAD_BODY_4() assert(0 && "Bug in generic_test_client");
#endif
#ifndef THREAD_BODY_5
#define THREAD_BODY_5() assert(0 && "Bug in generic_test_client");
#endif
#ifndef THREAD_BODY_6
#define THREAD_BODY_6() assert(0 && "Bug in generic_test_client");
#endif

// External synchronization
// method in thread#1 --sync--> method in thread#2
//#define FROM1_KIND_IX 1
//#define FROM1_COPY_IX 1
//#define TO1_KIND_IX 2
//#define TO1_COPY_IX 1

#ifdef FROM6_KIND_IX
#define SYNC_ITER_3ARGS ITER6_MACRO_3ARGS
#elif defined(FROM5_KIND_IX)
#define SYNC_ITER_3ARGS ITER5_MACRO_3ARGS
#elif defined(FROM4_KIND_IX)
#define SYNC_ITER_3ARGS ITER4_MACRO_3ARGS
#elif defined(FROM3_KIND_IX)
#define SYNC_ITER_3ARGS ITER3_MACRO_3ARGS
#elif defined(FROM2_KIND_IX)
#define SYNC_ITER_3ARGS ITER2_MACRO_3ARGS
#elif defined(FROM1_KIND_IX)
#define SYNC_ITER_3ARGS ITER1_MACRO_3ARGS
#else
#define SYNC_ITER_3ARGS ITER0_MACRO_3ARGS
#endif

#define FROM_SYNC(KIND_IX, COPY_IX, SYNC_IX) \
	if (KIND_IX == FROM##SYNC_IX##_KIND_IX && \
	    COPY_IX == FROM##SYNC_IX##_COPY_IX )   \
	atomic_store_explicit(&sync_var##SYNC_IX, 1, memory_order_release);

#define TO_SYNC(KIND_IX, COPY_IX, SYNC_IX) \
	if (KIND_IX == TO##SYNC_IX##_KIND_IX &&\
	    COPY_IX == TO##SYNC_IX##_COPY_IX) \
	__VERIFIER_assume(atomic_load_explicit(&sync_var##SYNC_IX, memory_order_acquire) == 1);

#define DECL_SYNC_VAR(_x, _y, SYNC_IX) \
	_Atomic(int) sync_var##SYNC_IX;

SYNC_ITER_3ARGS(DECL_SYNC_VAR, _PLACEHOLDER, _PLACEHOLDER)

#define M_PTHREAD(KIND_IX, COPY_IX) pthread_t t##KIND_IX##COPY_IX;

#ifdef GENERATE_SYNC // define each copy individually and add synchronization

	#define DECLARE_THREAD(KIND_IX, COPY_IX) \
		void *thread##KIND_IX##COPY_IX(void *param) { \
			SYNC_ITER_3ARGS(TO_SYNC, KIND_IX, COPY_IX); \
			THREAD_BODY_##KIND_IX(); \
			SYNC_ITER_3ARGS(FROM_SYNC, KIND_IX, COPY_IX); \
			return NULL; \
		}
	ITER2D(DECLARE_THREAD, /*MAX_NUM_OF_THREAD_KINDS=*/6, /*MAX_NUM_OF_THREAD_COPIES=*/10)

	#define M_CREATE_THREAD(KIND_IX, COPY_IX)                          \
		if (COPY_IX <= THREAD_NUM_ ## KIND_IX)                           \
			pthread_create(&t##KIND_IX##COPY_IX, NULL, thread ## KIND_IX ## COPY_IX, NULL);
#else
	#define DECLARE_THREAD(_ARG, KIND_IX) \
		void *thread##KIND_IX(void *param) { \
			THREAD_BODY_##KIND_IX();      \
			return NULL;              \
		}
	ITER(DECLARE_THREAD, 6)

	#define M_CREATE_THREAD(KIND_IX, COPY_IX)                          \
		if (COPY_IX <= THREAD_NUM_ ## KIND_IX)                           \
			pthread_create(&t##KIND_IX##COPY_IX, NULL, thread ## KIND_IX, NULL);

#endif // GENERATE_SYNC

#define M_JOIN_THREAD(KIND_IX, COPY_IX)  \
	if (COPY_IX <= THREAD_NUM_ ## KIND_IX) \
		pthread_join(t##KIND_IX##COPY_IX, NULL);

int main() {
	MAIN_THREAD_BODY_INIT();

	ITER2D(M_PTHREAD, /*MAX_NUM_OF_THREAD_KINDS=*/6, /*MAX_NUM_OF_THREAD_COPIES=*/10);

	ITER2D(M_CREATE_THREAD, /*MAX_NUM_OF_THREAD_KINDS=*/6, /*MAX_NUM_OF_THREAD_COPIES=*/10);

	ITER2D(M_JOIN_THREAD, /*MAX_NUM_OF_THREAD_KINDS=*/6, /*MAX_NUM_OF_THREAD_COPIES=*/10);

	MAIN_THREAD_BODY_FINISH();

	return 0;
}
