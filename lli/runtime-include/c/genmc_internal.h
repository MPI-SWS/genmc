#ifndef __GENMC_INTERNAL_H__
#define __GENMC_INTERNAL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Internal declarations for GenMC -- should not be used by user programs */

#ifdef __cplusplus
extern "C"
{
#endif

/* Data types and variables */

typedef long __VERIFIER_thread_t;
typedef struct { int __private; } __VERIFIER_attr_t;


typedef struct {
	unsigned __private;
	unsigned __count;
} __VERIFIER_barrier_t;
typedef int __VERIFIER_barrierattr_t;

typedef struct { int __private; } __VERIFIER_cond_t;
typedef long __VERIFIER_condattr_t;

typedef struct { int __private; } __VERIFIER_mutex_t;
typedef long __VERIFIER_mutexattr_t;

#ifndef __VERIFIER_MAX_THREAD_NUM
 #define __VERIFIER_MAX_THREAD_NUM 8
#endif

typedef struct {
	__VERIFIER_mutex_t __private[__VERIFIER_MAX_THREAD_NUM];
} __VERIFIER_rwlock_t;
typedef long __VERIFIER_rwlockattr_t;

typedef struct __VERIFIER_plock {
	__VERIFIER_mutex_t lock;
} __VERIFIER_plock_t;

#define __VERIFIER_MUTEX_INITIALIZER { 0 }
#define __VERIFIER_COND_INITIALIZER { 0 }

typedef struct { void *__dummy; } __VERIFIER_hazptr_t;

/* assert */

extern void __VERIFIER_assert_fail(const char *, const char *, int) __attribute__ ((__nothrow__));


/* stdlib */

extern void __VERIFIER_free(void *) __attribute__ ((__nothrow__));

extern void *__VERIFIER_malloc(size_t) __attribute__ ((__nothrow__));

extern void *__VERIFIER_malloc_aligned(size_t, size_t) __attribute__ ((__nothrow__));

extern int __VERIFIER_atexit(void (*func)(void)) __attribute__ ((__nothrow__));


/* Thread functions */

extern int __VERIFIER_thread_create (const __VERIFIER_attr_t * __attr,
				     void *(*__start_routine) (void *),
				     void *__restrict __arg) __attribute__ ((__nothrow__));

extern int __VERIFIER_thread_create_symmetric (const __VERIFIER_attr_t * __attr,
					       void *(*__start_routine) (void *),
					       void *__restrict __arg,
					       __VERIFIER_thread_t __th) __attribute__ ((__nothrow__));

extern void __VERIFIER_thread_exit (void *__retval) __attribute__ ((__noreturn__)) __attribute__ ((__nothrow__));

extern void *__VERIFIER_thread_join (__VERIFIER_thread_t __th) __attribute__ ((__nothrow__));

extern __VERIFIER_thread_t __VERIFIER_thread_self (void) __attribute__ ((__nothrow__));


/* Mutex functions */

extern int __VERIFIER_mutex_init (__VERIFIER_mutex_t *__mutex,
				  const __VERIFIER_mutexattr_t *__mutexattr) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_destroy (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_trylock (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_lock (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_unlock (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));

/* Condvar functions */

extern int __VERIFIER_cond_init (__VERIFIER_cond_t *__cond,
				  const __VERIFIER_condattr_t *__condattr) __attribute__ ((__nothrow__));

extern int __VERIFIER_cond_destroy (__VERIFIER_cond_t *__cond) __attribute__ ((__nothrow__));

extern int __VERIFIER_cond_wait (__VERIFIER_cond_t *__cond) __attribute__ ((__nothrow__));

extern int __VERIFIER_cond_signal (__VERIFIER_cond_t *__cond) __attribute__ ((__nothrow__));

extern int __VERIFIER_cond_bcast (__VERIFIER_cond_t *__cond) __attribute__ ((__nothrow__));


/* barrier functions */

extern int __VERIFIER_barrier_init (__VERIFIER_barrier_t *__restrict __barrier,
				    const __VERIFIER_barrierattr_t *__restrict __attr,
				    unsigned int __count) __attribute__ ((__nothrow__));

extern int __VERIFIER_barrier_destroy (__VERIFIER_barrier_t *__barrier) __attribute__ ((__nothrow__));

extern int __VERIFIER_barrier_wait (__VERIFIER_barrier_t *__barrier) __attribute__ ((__nothrow__));

extern int __VERIFIER_barrier_destroy (__VERIFIER_barrier_t *__barrier) __attribute__ ((__nothrow__));


/* Custom opcode implementations */

#define GENMC_ATTR_LOCAL   0x00000001
#define GENMC_ATTR_FINAL   0x00000002

#define GENMC_KIND_NONVR   0x00010000
#define GENMC_KIND_HELPED  0x00020000
#define GENMC_KIND_HELPING 0x00040000
#define GENMC_KIND_SPECUL  0x00080000
#define GENMC_KIND_CONFIRM 0x00100000
#define GENMC_KIND_PLOCK   0x00200000
#define GENMC_KIND_BARRIER 0x00400000

/* Need to match Execution.cpp and GenMC's AssumeType */
typedef enum {
	GENMC_ASSUME_USER = 0 ,
	GENMC_ASSUME_BARRIER = 1,
	GENMC_ASSUME_SPINLOOP = 2,
} __VERIFIER_assume_t;

/* Should match interpreter's assume call (arg size used in annotation passes) */
extern void __VERIFIER_assume_internal(bool cond, char type) __attribute__((__nothrow__));

/*
 * Annotate a subsequent instruction with the given mask.
 */
extern void __VERIFIER_annotate_begin(int mask) __attribute__ ((__nothrow__));
extern void __VERIFIER_annotate_end(int mask) __attribute__ ((__nothrow__));

/* Marks the beginning of an optional block. */
extern int __VERIFIER_opt_begin(void) __attribute__ ((__nothrow__));

/* Hazard pointer functions */
extern __VERIFIER_hazptr_t *__VERIFIER_hazptr_alloc(void)  __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_protect(__VERIFIER_hazptr_t *hp, void *p) __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_clear(__VERIFIER_hazptr_t *hp) __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_free(__VERIFIER_hazptr_t *hp) __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_retire(void *p) __attribute__ ((__nothrow__));

/* RCU functions */
extern void __VERIFIER_rcu_read_lock(void) __attribute__ ((__nothrow__));
extern void __VERIFIER_rcu_read_unlock(void) __attribute__ ((__nothrow__));
extern void __VERIFIER_synchronize_rcu(void) __attribute__ ((__nothrow__));

#ifdef __cplusplus
}
#endif

#endif /* __GENMC_INTERNAL_H__ */
