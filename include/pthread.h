/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _PTHREAD_H
#define _PTHREAD_H	1

#ifdef __cplusplus
extern "C"
{
#endif

//#include <features.h>
//#include <endian.h>
/* #include <sched.h> */
/* #include <time.h> */
#include <genmc_internal.h>

typedef __VERIFIER_attr_t pthread_attr_t;
typedef __VERIFIER_barrier_t pthread_barrier_t;
typedef __VERIFIER_barrierattr_t pthread_barrierattr_t;
/* typedef struct { int __private; } pthread_cond_t; */
/* typedef long pthread_condattr_t; */
/* typedef int pthread_key_t; */
typedef __VERIFIER_mutex_t pthread_mutex_t;
typedef __VERIFIER_mutexattr_t pthread_mutexattr_t;
/* typedef int pthread_once_t; */
/* typedef struct { int __private; } pthread_rwlock_t; */
/* typedef struct { int __private; } pthread_rwlockattr_t; */
/* typedef struct { int __private; } pthread_spinlock_t; */
typedef __VERIFIER_thread_t pthread_t;

#define PTHREAD_MUTEX_INITIALIZER __VERIFIER_MUTEX_INITIALIZER

/* Detach state.  */
enum
{
  PTHREAD_CREATE_JOINABLE,
#define PTHREAD_CREATE_JOINABLE	PTHREAD_CREATE_JOINABLE
  PTHREAD_CREATE_DETACHED
#define PTHREAD_CREATE_DETACHED	PTHREAD_CREATE_DETACHED
};


/* Mutex types.  */
enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
#ifdef __USE_GNU
  /* For compatibility.  */
  , PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_TIMED_NP
#endif
};


/* Mutex protocols.  */
enum
{
  PTHREAD_PRIO_NONE,
  PTHREAD_PRIO_INHERIT,
  PTHREAD_PRIO_PROTECT
};


/* /\* Read-write lock types.  *\/ */
/* enum */
/* { */
/*   PTHREAD_RWLOCK_PREFER_READER_NP, */
/*   PTHREAD_RWLOCK_PREFER_WRITER_NP, */
/*   PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP, */
/*   PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_READER_NP */
/* }; */

/* /\* Scheduler inheritance.  *\/ */
/* enum */
/* { */
/*   PTHREAD_INHERIT_SCHED, */
/* #define PTHREAD_INHERIT_SCHED   PTHREAD_INHERIT_SCHED */
/*   PTHREAD_EXPLICIT_SCHED */
/* #define PTHREAD_EXPLICIT_SCHED  PTHREAD_EXPLICIT_SCHED */
/* }; */


/* Scope handling.  */
enum
{
  PTHREAD_SCOPE_SYSTEM,
#define PTHREAD_SCOPE_SYSTEM    PTHREAD_SCOPE_SYSTEM
  PTHREAD_SCOPE_PROCESS
#define PTHREAD_SCOPE_PROCESS   PTHREAD_SCOPE_PROCESS
};


/* Process shared or private flag.  */
enum
{
  PTHREAD_PROCESS_PRIVATE,
#define PTHREAD_PROCESS_PRIVATE PTHREAD_PROCESS_PRIVATE
  PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_SHARED  PTHREAD_PROCESS_SHARED
};


/* Single execution handling.  */
#define PTHREAD_ONCE_INIT 0

/* Value returned by 'pthread_barrier_wait' for one of the threads after
   the required number of threads have called this function.
   -1 is distinct from 0 and all errno constants */
#define PTHREAD_BARRIER_SERIAL_THREAD -1


/* Create a new thread, starting with execution of START-ROUTINE
   getting passed ARG.  Creation attributed come from ATTR.  The new
   handle is stored in *NEWTHREAD.  */
__attribute__ ((always_inline)) static inline
int pthread_create(pthread_t *__restrict __newthread,
		   const pthread_attr_t *__restrict __attr,
		   void *(*__start_routine) (void *),
		   void *__restrict __arg)
{
	(*__newthread) = __VERIFIER_thread_create(__attr, __start_routine, __arg);
	return 0;
}

/* Terminate calling thread.  */
__attribute__ ((always_inline)) static inline
void pthread_exit(void *__retval)
{
	__VERIFIER_thread_exit(__retval);
}

/* Make calling thread wait for termination of the thread TH.  The
   exit status of the thread is stored in *THREAD_RETURN, if THREAD_RETURN
   is not NULL. */
__attribute__ ((always_inline)) static inline
int pthread_join(pthread_t __th, void **__thread_return)
{
	void *__retval = __VERIFIER_thread_join(__th);
	if (__thread_return != NULL)
		*(__thread_return) = __retval;
	return 0;
}

/* #ifdef __USE_GNU */
/* /\* Check whether thread TH has terminated.  If yes return the status of */
/*    the thread in *THREAD_RETURN, if THREAD_RETURN is not NULL.  *\/ */
/* extern int pthread_tryjoin_np (pthread_t __th, void **__thread_return); */

/* /\* Make calling thread wait for termination of the thread TH, but only */
/*    until TIMEOUT.  The exit status of the thread is stored in */
/*    *THREAD_RETURN, if THREAD_RETURN is not NULL.  *\/ */
/* extern int pthread_timedjoin_np (pthread_t __th, void **__thread_return, */
/* 				 const struct timespec *__abstime); */
/* #endif */

/* /\* Indicate that the thread TH is never to be joined with PTHREAD_JOIN. */
/*    The resources of TH will therefore be freed immediately when it */
/*    terminates, instead of waiting for another thread to perform PTHREAD_JOIN */
/*    on it.  *\/ */
/* extern int pthread_detach (pthread_t __th); */


/* Obtain the identifier of the current thread.  */
/* extern pthread_t pthread_self (void); */
__attribute__ ((always_inline)) static inline
pthread_t pthread_self(void)
{
	return __VERIFIER_thread_self();
}

/* Compare two thread identifiers.  */
/* extern int pthread_equal (pthread_t __thread1, pthread_t __thread2); */


/* /\* Thread attribute handling.  *\/ */

/* /\* Initialize thread attribute *ATTR with default attributes */
/*    (detachstate is PTHREAD_JOINABLE, scheduling policy is SCHED_OTHER, */
/*     no user-provided stack).  *\/ */
/* extern int pthread_attr_init (pthread_attr_t *__attr); */

/* /\* Destroy thread attribute *ATTR.  *\/ */
/* extern int pthread_attr_destroy (pthread_attr_t *__attr); */

/* /\* Get detach state attribute.  *\/ */
/* extern int pthread_attr_getdetachstate (const pthread_attr_t *__attr, */
/* 					int *__detachstate); */

/* /\* Set detach state attribute.  *\/ */
/* extern int pthread_attr_setdetachstate (pthread_attr_t *__attr, */
/* 					int __detachstate); */


/* /\* Get the size of the guard area created for stack overflow protection.  *\/ */
/* extern int pthread_attr_getguardsize (const pthread_attr_t *__attr, */
/* 				      size_t *__guardsize); */

/* /\* Set the size of the guard area created for stack overflow protection.  *\/ */
/* extern int pthread_attr_setguardsize (pthread_attr_t *__attr, */
/* 				      size_t __guardsize); */


/* /\* Return in *PARAM the scheduling parameters of *ATTR.  *\/ */
/* extern int pthread_attr_getschedparam (const pthread_attr_t *__restrict __attr, */
/* 				       struct sched_param *__restrict __param); */

/* /\* Set scheduling parameters (priority, etc) in *ATTR according to PARAM.  *\/ */
/* extern int pthread_attr_setschedparam (pthread_attr_t *__restrict __attr, */
/* 				       const struct sched_param *__restrict __param); */

/* /\* Return in *POLICY the scheduling policy of *ATTR.  *\/ */
/* extern int pthread_attr_getschedpolicy (const pthread_attr_t *__restrict */
/* 					__attr, int *__restrict __policy); */

/* /\* Set scheduling policy in *ATTR according to POLICY.  *\/ */
/* extern int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy); */

/* /\* Return in *INHERIT the scheduling inheritance mode of *ATTR.  *\/ */
/* extern int pthread_attr_getinheritsched (const pthread_attr_t *__restrict */
/* 					 __attr, int *__restrict __inherit); */

/* /\* Set scheduling inheritance mode in *ATTR according to INHERIT.  *\/ */
/* extern int pthread_attr_setinheritsched (pthread_attr_t *__attr, */
/* 					 int __inherit); */


/* /\* Return in *SCOPE the scheduling contention scope of *ATTR.  *\/ */
/* extern int pthread_attr_getscope (const pthread_attr_t *__restrict __attr, */
/* 				  int *__restrict __scope); */

/* /\* Set scheduling contention scope in *ATTR according to SCOPE.  *\/ */
/* extern int pthread_attr_setscope (pthread_attr_t *__attr, int __scope); */

/* /\* Return the previously set address for the stack.  *\/ */
/* extern int pthread_attr_getstackaddr (const pthread_attr_t *__restrict */
/* 				      __attr, void **__restrict __stackaddr); */

/* /\* Set the starting address of the stack of the thread to be created. */
/*    Depending on whether the stack grows up or down the value must either */
/*    be higher or lower than all the address in the memory block.  The */
/*    minimal size of the block must be PTHREAD_STACK_MIN.  *\/ */
/* extern int pthread_attr_setstackaddr (pthread_attr_t *__attr, void *__stackaddr); */

/* /\* Return the currently used minimal stack size.  *\/ */
/* extern int pthread_attr_getstacksize (const pthread_attr_t *__restrict */
/* 				      __attr, size_t *__restrict __stacksize); */

/* /\* Add information about the minimum stack size needed for the thread */
/*    to be started.  This size must never be less than PTHREAD_STACK_MIN */
/*    and must also not exceed the system limits.  *\/ */
/* extern int pthread_attr_setstacksize (pthread_attr_t *__attr, size_t __stacksize); */

/* /\* Functions for scheduling control.  *\/ */

/* /\* Set the scheduling parameters for TARGET_THREAD according to POLICY */
/*    and *PARAM.  *\/ */
/* extern int pthread_setschedparam (pthread_t __target_thread, int __policy, */
/* 				  const struct sched_param *__param) */
/*      ; */

/* /\* Return in *POLICY and *PARAM the scheduling parameters for TARGET_THREAD. *\/ */
/* extern int pthread_getschedparam (pthread_t __target_thread, */
/* 				  int *__restrict __policy, */
/* 				  struct sched_param *__restrict __param) */
/*      ; */

/* /\* Set the scheduling priority for TARGET_THREAD.  *\/ */
/* extern int pthread_setschedprio (pthread_t __target_thread, int __prio) */
/*      ; */


/* /\* Determine level of concurrency.  *\/ */
/* extern int pthread_getconcurrency (void) ; */

/* /\* Set new concurrency level to LEVEL.  *\/ */
/* extern int pthread_setconcurrency (int __level) ; */

/* /\* Yield the processor to another thread or process. */
/*    This function is similar to the POSIX `sched_yield' function but */
/*    might be differently implemented in the case of a m-on-n thread */
/*    implementation.  *\/ */
/* extern int pthread_yield (void) ; */


/* /\* Functions for handling initialization.  *\/ */

/* /\* Guarantee that the initialization function INIT_ROUTINE will be called */
/*    only once, even if pthread_once is executed several times with the */
/*    same ONCE_CONTROL argument. ONCE_CONTROL must point to a static or */
/*    extern variable initialized to PTHREAD_ONCE_INIT. */
/*  *\/ */
/* extern int pthread_once (pthread_once_t *__once_control, */
/* 			 void (*__init_routine) (void)) ; */


/* /\* Functions for handling cancellation. */

/*    Note that these functions are explicitly not marked to not throw an */
/*    exception in C++ code.  If cancellation is implemented by unwinding */
/*    this is necessary to have the compiler generate the unwind information.  *\/ */

/* /\* Set cancelability state of current thread to STATE, returning old */
/*    state in *OLDSTATE if OLDSTATE is not NULL.  *\/ */
/* extern int pthread_setcancelstate (int __state, int *__oldstate); */

/* /\* Set cancellation state of current thread to TYPE, returning the old */
/*    type in *OLDTYPE if OLDTYPE is not NULL.  *\/ */
/* extern int pthread_setcanceltype (int __type, int *__oldtype); */

/* /\* Cancel THREAD immediately or at the next possibility.  *\/ */
/* extern int pthread_cancel (pthread_t __th); */

/* /\* Test for pending cancellation for the current thread and terminate */
/*    the thread as per pthread_exit(PTHREAD_CANCELED) if it has been */
/*    cancelled.  *\/ */
/* extern void pthread_testcancel (void); */


/* Mutex handling.  */

/* Initialize a mutex.  */
__attribute__ ((always_inline)) static inline
int pthread_mutex_init(pthread_mutex_t *__mutex,
		       const pthread_mutexattr_t *__mutexattr)
{
	return __VERIFIER_mutex_init(__mutex, __mutexattr);
}

/* Destroy a mutex.  */
/* extern int pthread_mutex_destroy (pthread_mutex_t *__mutex); */
__attribute__ ((always_inline)) static inline
int pthread_mutex_destroy(pthread_mutex_t *__mutex)
{
	return __VERIFIER_mutex_destroy(__mutex);
}

/* Try locking a mutex.  */
/* extern int pthread_mutex_trylock (pthread_mutex_t *__mutex); */
__attribute__ ((always_inline)) static inline
int pthread_mutex_trylock(pthread_mutex_t *__mutex)
{
	return __VERIFIER_mutex_trylock(__mutex);
}

/* Lock a mutex.  */
/* extern int pthread_mutex_lock (pthread_mutex_t *__mutex); */
__attribute__ ((always_inline)) static inline
int pthread_mutex_lock(pthread_mutex_t *__mutex)
{
	return __VERIFIER_mutex_lock(__mutex);
}

/* Unlock a mutex.  */
/* extern int pthread_mutex_unlock (pthread_mutex_t *__mutex); */
__attribute__ ((always_inline)) static inline
int pthread_mutex_unlock(pthread_mutex_t *__mutex)
{
	return __VERIFIER_mutex_unlock(__mutex);
}

/* /\* Get the priority ceiling of MUTEX.  *\/ */
/* extern int pthread_mutex_getprioceiling (const pthread_mutex_t * */
/* 					 __restrict __mutex, */
/* 					 int *__restrict __prioceiling); */

/* /\* Set the priority ceiling of MUTEX to PRIOCEILING, return old */
/*    priority ceiling value in *OLD_CEILING.  *\/ */
/* extern int pthread_mutex_setprioceiling (pthread_mutex_t *__restrict __mutex, */
/* 					 int __prioceiling, */
/* 					 int *__restrict __old_ceiling); */


/* /\* Functions for handling mutex attributes.  *\/ */

/* /\* Initialize mutex attribute object ATTR with default attributes */
/*    (kind is PTHREAD_MUTEX_TIMED_NP).  *\/ */
/* extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr); */

/* /\* Destroy mutex attribute object ATTR.  *\/ */
/* extern int pthread_mutexattr_destroy (pthread_mutexattr_t *__attr); */

/* /\* Get the process-shared flag of the mutex attribute ATTR.  *\/ */
/* extern int pthread_mutexattr_getpshared (const pthread_mutexattr_t * */
/* 					 __restrict __attr, */
/* 					 int *__restrict __pshared); */

/* /\* Set the process-shared flag of the mutex attribute ATTR.  *\/ */
/* extern int pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr, */
/* 					 int __pshared); */

/* /\* Return in *KIND the mutex kind attribute in *ATTR.  *\/ */
/* extern int pthread_mutexattr_gettype (const pthread_mutexattr_t *__restrict */
/* 				      __attr, int *__restrict __kind) ; */

/* /\* Set the mutex kind attribute in *ATTR to KIND (either PTHREAD_MUTEX_NORMAL, */
/*    PTHREAD_MUTEX_RECURSIVE, PTHREAD_MUTEX_ERRORCHECK, or */
/*    PTHREAD_MUTEX_DEFAULT).  *\/ */
/* extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind) ; */

/* /\* Return in *PROTOCOL the mutex protocol attribute in *ATTR.  *\/ */
/* extern int pthread_mutexattr_getprotocol (const pthread_mutexattr_t * */
/* 					  __restrict __attr, */
/* 					  int *__restrict __protocol) ; */

/* /\* Set the mutex protocol attribute in *ATTR to PROTOCOL (either */
/*    PTHREAD_PRIO_NONE, PTHREAD_PRIO_INHERIT, or PTHREAD_PRIO_PROTECT).  *\/ */
/* extern int pthread_mutexattr_setprotocol (pthread_mutexattr_t *__attr, */
/* 					  int __protocol) */
/*      ; */

/* /\* Return in *PRIOCEILING the mutex prioceiling attribute in *ATTR.  *\/ */
/* extern int pthread_mutexattr_getprioceiling (const pthread_mutexattr_t * */
/* 					     __restrict __attr, */
/* 					     int *__restrict __prioceiling) ; */

/* /\* Set the mutex prioceiling attribute in *ATTR to PRIOCEILING.  *\/ */
/* extern int pthread_mutexattr_setprioceiling (pthread_mutexattr_t *__attr, */
/* 					     int __prioceiling) ; */


/* /\* Functions for handling read-write locks.  *\/ */

/* /\* Initialize read-write lock RWLOCK using attributes ATTR, or use */
/*    the default values if later is NULL.  *\/ */
/* extern int pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock, */
/* 				const pthread_rwlockattr_t *__restrict */
/* 				__attr) ; */

/* /\* Destroy read-write lock RWLOCK.  *\/ */
/* extern int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock) */
/*      ; */

/* /\* Acquire read lock for RWLOCK.  *\/ */
/* extern int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock) */
/*      ; */

/* /\* Try to acquire read lock for RWLOCK.  *\/ */
/* extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock) */
/*   ; */

/* /\* Acquire write lock for RWLOCK.  *\/ */
/* extern int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock) */
/*      ; */

/* /\* Try to acquire write lock for RWLOCK.  *\/ */
/* extern int pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock) */
/*      ; */

/* /\* Unlock RWLOCK.  *\/ */
/* extern int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock); */


/* /\* Functions for handling read-write lock attributes.  *\/ */

/* /\* Initialize attribute object ATTR with default values.  *\/ */
/* extern int pthread_rwlockattr_init (pthread_rwlockattr_t *__attr); */

/* /\* Destroy attribute object ATTR.  *\/ */
/* extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr); */

/* /\* Return current setting of process-shared attribute of ATTR in PSHARED.  *\/ */
/* extern int pthread_rwlockattr_getpshared (const pthread_rwlockattr_t * */
/* 					  __restrict __attr, */
/* 					  int *__restrict __pshared); */

/* /\* Set process-shared attribute of ATTR to PSHARED.  *\/ */
/* extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *__attr, */
/* 					  int __pshared); */

/* /\* Return current setting of reader/writer preference.  *\/ */
/* extern int pthread_rwlockattr_getkind_np (const pthread_rwlockattr_t * */
/* 					  __restrict __attr, */
/* 					  int *__restrict __pref); */

/* /\* Set reader/write preference.  *\/ */
/* extern int pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *__attr, */
/* 					  int __pref) ; */


/* /\* Functions for handling conditional variables.  *\/ */

/* /\* Initialize condition variable COND using attributes ATTR, or use */
/*    the default values if later is NULL.  *\/ */
/* extern int pthread_cond_init (pthread_cond_t *__restrict __cond, */
/* 			      const pthread_condattr_t *__restrict __cond_attr) */
/*      ; */

/* /\* Destroy condition variable COND.  *\/ */
/* extern int pthread_cond_destroy (pthread_cond_t *__cond) */
/*      ; */

/* /\* Wake up one thread waiting for condition variable COND.  *\/ */
/* extern int pthread_cond_signal (pthread_cond_t *__cond) */
/*      ; */

/* /\* Wake up all threads waiting for condition variables COND.  *\/ */
/* extern int pthread_cond_broadcast (pthread_cond_t *__cond) */
/*      ; */

/* /\* Wait for condition variable COND to be signaled or broadcast. */
/*    MUTEX is assumed to be locked before. *\/ */
/* extern int pthread_cond_wait (pthread_cond_t *__restrict __cond, */
/* 			      pthread_mutex_t *__restrict __mutex) */
/*      ; */

/* /\* Wait for condition variable COND to be signaled or broadcast until */
/*    ABSTIME.  MUTEX is assumed to be locked before.  ABSTIME is an */
/*    absolute time specification; zero is the beginning of the epoch */
/*    (00:00:00 GMT, January 1, 1970). *\/ */
/* extern int pthread_cond_timedwait (pthread_cond_t *__restrict __cond, */
/* 				   pthread_mutex_t *__restrict __mutex, */
/* 				   const struct timespec *__restrict __abstime) */
/*      ; */

/* /\* Functions for handling condition variable attributes.  *\/ */

/* /\* Initialize condition variable attribute ATTR.  *\/ */
/* extern int pthread_condattr_init (pthread_condattr_t *__attr) */
/*      ; */

/* /\* Destroy condition variable attribute ATTR.  *\/ */
/* extern int pthread_condattr_destroy (pthread_condattr_t *__attr) */
/*      ; */

/* /\* Get the process-shared flag of the condition variable attribute ATTR.  *\/ */
/* extern int pthread_condattr_getpshared (const pthread_condattr_t * */
/* 					__restrict __attr, */
/* 					int *__restrict __pshared) */
/*      ; */

/* /\* Set the process-shared flag of the condition variable attribute ATTR.  *\/ */
/* extern int pthread_condattr_setpshared (pthread_condattr_t *__attr, */
/* 					int __pshared) ; */



/* /\* Functions for handling thread-specific data.  *\/ */

/* /\* Create a key value identifying a location in the thread-specific */
/*    data area.  Each thread maintains a distinct thread-specific data */
/*    area.  DESTR_FUNCTION, if non-NULL, is called with the value */
/*    associated to that key when the key is destroyed. */
/*    DESTR_FUNCTION is not called if the value associated is NULL when */
/*    the key is destroyed.  *\/ */
/* extern int pthread_key_create (pthread_key_t *__key, */
/* 			       void (*__destr_function) (void *)) ; */

/* /\* Destroy KEY.  *\/ */
/* extern int pthread_key_delete (pthread_key_t __key) ; */

/* /\* Return current value of the thread-specific data slot identified by KEY.  *\/ */
/* extern void *pthread_getspecific (pthread_key_t __key) ; */

/* /\* Store POINTER in the thread-specific data slot identified by KEY. *\/ */
/* extern int pthread_setspecific (pthread_key_t __key, */
/* 				const void *__pointer) ; */


/* /\* Install handlers to be called when a new process is created with FORK. */
/*    The PREPARE handler is called in the parent process just before performing */
/*    FORK. The PARENT handler is called in the parent process just after FORK. */
/*    The CHILD handler is called in the child process.  Each of the three */
/*    handlers can be NULL, meaning that no handler needs to be called at that */
/*    point. */
/*    PTHREAD_ATFORK can be called several times, in which case the PREPARE */
/*    handlers are called in LIFO order (last added with PTHREAD_ATFORK, */
/*    first called before FORK), and the PARENT and CHILD handlers are called */
/*    in FIFO (first added, first called).  *\/ */

/* extern int pthread_atfork (void (*__prepare) (void), */
/* 			   void (*__parent) (void), */
/* 			   void (*__child) (void)) ; */


/* Functions to handle barriers.  */

/* Initialize BARRIER with the attributes in ATTR.  The barrier is
   opened when COUNT waiters arrived.  */
__attribute__ ((always_inline)) static inline
int pthread_barrier_init(pthread_barrier_t *__restrict __barrier,
			 const pthread_barrierattr_t *__restrict __attr,
			 unsigned int __count)
{
	return __VERIFIER_barrier_init(__barrier, __attr, __count);
}

/* Wait on barrier BARRIER.  */
__attribute__ ((always_inline)) static inline
int pthread_barrier_wait(pthread_barrier_t *__barrier)
{
	return __VERIFIER_barrier_wait(__barrier);
}

/* Destroy a previously dynamically initialized barrier BARRIER.  */
__attribute__ ((always_inline)) static inline
int pthread_barrier_destroy(pthread_barrier_t *__barrier)
{
	return __VERIFIER_barrier_destroy(__barrier);
}

#ifdef __cplusplus
}
#endif

#endif	/* pthread.h */
